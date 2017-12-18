#include "performance_monitor.h"
#include <process.h>
#include <tchar.h>

sirius::library::misc::performance::monitor::core::core(sirius::library::misc::performance::monitor * front)
	: _front(front)
	, _thread(INVALID_HANDLE_VALUE)
	, _run(false)
{

}

sirius::library::misc::performance::monitor::core::~core(void)
{

}

int32_t sirius::library::misc::performance::monitor::core::initialize(void)
{
	unsigned int thread_id = 0;
	_run = true;
	_thread = (HANDLE)_beginthreadex(NULL, 0, sirius::library::misc::performance::monitor::core::process_cb, this, 0, &thread_id);

	PDH_STATUS status = PdhOpenQuery(0, 0, &_cpu_query);
	if (status != ERROR_SUCCESS)
		return sirius::library::misc::performance::monitor::err_code_t::fail;

	status = PdhAddCounter(_cpu_query, _T("\\Processor(_Total)\\% Processor Time"), 0, &_cpu_total);

	if (status != ERROR_SUCCESS)
		return sirius::library::misc::performance::monitor::err_code_t::fail;

	status = PdhCollectQueryData(_cpu_query);

	if (status != ERROR_SUCCESS)
	{
		return sirius::library::misc::performance::monitor::err_code_t::fail;
	}

	return sirius::library::misc::performance::monitor::err_code_t::success;
}

int32_t sirius::library::misc::performance::monitor::core::release(void)
{
	_run = false;
	if (::WaitForSingleObject(_thread, INFINITE) == WAIT_OBJECT_0)
	{
		::CloseHandle(_thread);
		_thread = INVALID_HANDLE_VALUE;
	}

	if (_cpu_query)
		PdhCloseQuery(_cpu_query);
	_cpu_query = 0;

	return sirius::library::misc::performance::monitor::err_code_t::success;
}

char * sirius::library::misc::performance::monitor::core::cpu_info(void)
{
	HKEY key;
	TCHAR buffer[128] = { 0, };
	DWORD buf_size = 128;
	DWORD type = REG_SZ;
	memset(buffer, 0, sizeof(TCHAR) * 128);

	long result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"), 0, KEY_READ, &key);
	if (result == ERROR_SUCCESS)
	{
		RegQueryValueExA(key, "ProcessorNameString", NULL, &type, (LPBYTE)_cpu_info, &buf_size);
	}
	return _cpu_info;
}

char * sirius::library::misc::performance::monitor::core::mem_info(void)
{
	uint64_t memory_size = 0;
	::GetPhysicallyInstalledSystemMemory(&memory_size);
	_snprintf_s(_mem_info, sizeof(_mem_info), "%d MB", memory_size / 1000);
	return _mem_info;
}

double sirius::library::misc::performance::monitor::core::total_cpu_usage(void)
{
	double usage = 0.f;
	PDH_STATUS status = PdhCollectQueryData(_cpu_query);

	if (status != ERROR_SUCCESS)
		return usage;

	PDH_FMT_COUNTERVALUE value;
	status = PdhGetFormattedCounterValue(_cpu_total, PDH_FMT_DOUBLE, 0, &value);
	if (status != ERROR_SUCCESS)
		return usage;
		
	usage = value.doubleValue;
	return usage;
}

double sirius::library::misc::performance::monitor::core::process_cpu_usage(int32_t container_number)
{
	double usage = 0.00;
	if (_cpu_stats.size() > 0)
	{
		if (_cpu_stats[container_number].cpu_usage > 100 || _cpu_stats[container_number].cpu_usage < 0)
		{
			usage = 0.00;
			return usage;
		}
		else
		{
			usage = _cpu_stats[container_number].cpu_usage;
			return usage;
		}
	}
	return usage;
}

double sirius::library::misc::performance::monitor::core::total_mem_usage(void)
{
	double usage = 0.f;
	MEMORYSTATUSEX ms;
	ms.dwLength = sizeof(MEMORYSTATUSEX);
	::GlobalMemoryStatusEx(&ms);

	double dwTotalPhys = ms.ullTotalPhys / (1024 * 1024);
	double dwAvailPhys = ms.ullAvailPhys / (1024 * 1024);
	double dwUsedPhys = dwTotalPhys - dwAvailPhys;
	double Mem_per = (dwUsedPhys / dwTotalPhys) * 100;

	usage = Mem_per;
	return usage;
}

unsigned __stdcall sirius::library::misc::performance::monitor::core::process_cb(void * param)
{
	sirius::library::misc::performance::monitor::core * self = static_cast<sirius::library::misc::performance::monitor::core*>(param);
	self->process();
	return 0;
}

void sirius::library::misc::performance::monitor::core::process(void)
{
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	int32_t processor_count = sysInfo.dwNumberOfProcessors;

	/*
	for (int i = 0; i < Config.get_max_slot(); i++)
	{
		proc_cpu_info_t proc_cpu_info;
		memset(&proc_cpu_info, 0x00, sizeof(proc_cpu_info_t));
		_vec_proc_cpu.push_back(proc_cpu_info);
	}

	while (!_process_cpu_usage_thread_stop)
	{

		for (int i = 0; i < Config.get_max_slot(); i++)
		{
			slot_stateinfo *slot_info = SLTMGR.find(i);
			if (slot_info)
			{
				int32_t pid = slot_info->get_slot_pid();
				if (pid <= 0)
				{
					memset(&_vec_proc_cpu[i], 0x00, sizeof(proc_cpu_info_t));
					continue;
				}

				HANDLE h_process = OpenProcess(MAXIMUM_ALLOWED, TRUE, pid);

				FILETIME ftime, fsys, fuser;
				ULARGE_INTEGER now, sys, user;
				double percent;

				GetSystemTimeAsFileTime(&ftime);
				memcpy(&now, &ftime, sizeof(FILETIME));

				GetProcessTimes(h_process, &ftime, &ftime, &fsys, &fuser);
				memcpy(&sys, &fsys, sizeof(FILETIME));
				memcpy(&user, &fuser, sizeof(FILETIME));
				percent = (sys.QuadPart - _vec_proc_cpu[i].last_sys_cpu.QuadPart) + (user.QuadPart - _vec_proc_cpu[i].last_user_cpu.QuadPart);
				percent /= (now.QuadPart - _vec_proc_cpu[i].last_cpu.QuadPart);
				percent /= processor_count;

				_vec_proc_cpu[i].last_cpu = now;
				_vec_proc_cpu[i].last_user_cpu = user;
				_vec_proc_cpu[i].last_sys_cpu = sys;
				_vec_proc_cpu[i].process_id = pid;
				_vec_proc_cpu[i].cpu_usage = percent * 100;

				CloseHandle(h_process);

			}
		}
		Sleep(1000 * 1);
	}
	*/
	_cpu_stats.clear();
}