#include "process_controller.h"
#include <tchar.h>
#include <wtsapi32.h>
#include <tlhelp32.h> 
#include <Shlwapi.h>
#include <string>
#include <psapi.h>
#include <sirius_stringhelper.h>

#pragma comment(lib,"wtsapi32.lib")

sirius::app::server::arbitrator::process::controller::controller(void)
{

}

sirius::app::server::arbitrator::process::controller::~controller(void)
{

}

int32_t sirius::app::server::arbitrator::process::controller::fork(const char * executable, const char * path, const char * arguments, unsigned long * pid)
{
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	memset(&si, 0x00, sizeof(si));
	memset(&pi, 0x00, sizeof(pi));
	si.cb = sizeof(STARTUPINFO);

	char module_path[MAX_PATH] = { 0 };
	char * module_name = module_path;
	module_name += GetModuleFileNameA(NULL, module_name, (sizeof(module_path) / sizeof(*module_path)) - (module_name - module_path));
	if (module_name != module_path)
	{
		CHAR * slash = strrchr(module_path, '\\');
		if (slash != NULL)
		{
			module_name = slash + 1;
			_strset_s(module_name, strlen(module_name) + 1, 0);
		}
		else
		{
			_strset_s(module_path, strlen(module_path) + 1, 0);
		}
	}

	char real_executable[MAX_PATH] = { 0 };
	char real_path[MAX_PATH] = { 0 };
	_snprintf_s(real_executable, sizeof(real_executable), sizeof(real_executable), "%s%s", module_path, executable);
	_snprintf_s(real_path, sizeof(real_path), sizeof(real_path), "%s%s", module_path, path);

	BOOL result = FALSE;
	std::string command = real_executable;
	if (arguments && strlen(arguments)> 0)
	{
		command += " ";
		command += arguments;
	}

	result = ::CreateProcessA(NULL, (LPSTR)(LPCSTR)command.c_str(), NULL, NULL, FALSE, CREATE_NEW_PROCESS_GROUP, NULL, real_path, &si, &pi);
	if (!result)
	{
		DWORD err = GetLastError();
	}
	*pid = pi.dwProcessId;

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return result ? sirius::app::server::arbitrator::process::controller::err_code_t::success : sirius::app::server::arbitrator::process::controller::err_code_t::fail;
}

int32_t sirius::app::server::arbitrator::process::controller::kill(unsigned long pid)
{
#if 1
	int32_t status = sirius::app::server::arbitrator::process::controller::err_code_t::fail;
	HANDLE	hprocess = NULL;
	hprocess = ::OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, FALSE, pid);
	if(hprocess == NULL)
		return sirius::app::server::arbitrator::process::controller::err_code_t::fail;

	HWND hwnd = hwnd_from_pid(pid);
	::PostMessage(hwnd, WM_CLOSE, 0, 0);
	if (::WaitForSingleObject(hprocess, 3000) != WAIT_OBJECT_0)
	{
		if (TerminateProcess(hprocess, 0))
		{
			::WaitForSingleObject(hprocess, 3000);
			status = sirius::app::server::arbitrator::process::controller::err_code_t::success;
		}
		else
		{
			status = sirius::app::server::arbitrator::process::controller::err_code_t::fail;
		}
	}
	else
	{
		status = sirius::app::server::arbitrator::process::controller::err_code_t::success;
	}

	::CloseHandle(hprocess);
	return status;
#else
	DWORD processes[MAX_PROCESS_LIST_COUNT];
	DWORD rbytes;

	if (!EnumProcesses(processes, sizeof(processes), &rbytes))
		return sirius::app::server::arbitrator::process::controller::err_code_t::fail;

	int32_t count = rbytes / sizeof(DWORD);
	for (int32_t i = 0; i < count; ++i)
	{
		HANDLE process = OpenProcess(PROCESS_TERMINATE, FALSE, processes[i]);
		if (process)
		{
			for (int i = 0; i < count; ++i)
			{
				if (processes[i] == pid)
					TerminateProcess(process, 0);
			}
			CloseHandle(process);
		}
	}
#endif
	return sirius::app::server::arbitrator::process::controller::err_code_t::success;
}

int32_t sirius::app::server::arbitrator::process::controller::kill(const char * name)
{
	int32_t status = sirius::app::server::arbitrator::process::controller::err_code_t::fail;
	DWORD processes[MAX_PROCESS_LIST_COUNT];
	DWORD rbytes;

	if (!EnumProcesses(processes, sizeof(processes), &rbytes))
		return sirius::app::server::arbitrator::process::controller::err_code_t::fail;

	int32_t count = rbytes / sizeof(DWORD);
	for (int32_t i = 0; i < count; ++i)
	{
		HANDLE hprocess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_TERMINATE, FALSE, processes[i]);
		if (hprocess)
		{
			DWORD needed;
			HMODULE hmodule;

			if (EnumProcessModules(hprocess, &hmodule, sizeof(hmodule), &needed))
			{
				TCHAR module_path[MAX_PATH];
				GetModuleFileNameEx(hprocess, hmodule, module_path, sizeof(module_path));

				char* pname = nullptr;
				sirius::stringhelper::convert_wide2multibyte(module_path, &pname);
				std::string proc_path = pname;
				std::string proc_name = proc_path.substr(proc_path.find_last_of("/\\") + 1);

				if (proc_name.compare(name) == 0)
				{
					unsigned long pid = ::GetProcessId(hprocess);
					HWND hwnd = hwnd_from_pid(pid);
					::PostMessage(hwnd, WM_CLOSE, 0, 0);
					if (::WaitForSingleObject(hprocess, 3000) != WAIT_OBJECT_0)
					{
						if (TerminateProcess(hprocess, 0))
						{
							::WaitForSingleObject(hprocess, 3000);
							status = sirius::app::server::arbitrator::process::controller::err_code_t::success;
						}
						else
						{
							status = sirius::app::server::arbitrator::process::controller::err_code_t::fail;
						}
					}
					else
					{
						status = sirius::app::server::arbitrator::process::controller::err_code_t::success;
					}
				}
				if (pname)
				{
					free(pname);
					pname = nullptr;
				}
			}
			CloseHandle(hprocess);
		}
	}
	return status;
}

int32_t sirius::app::server::arbitrator::process::controller::find(unsigned long pid)
{
	DWORD processes[MAX_PROCESS_LIST_COUNT];
	DWORD rbytes;

	if (!EnumProcesses(processes, sizeof(processes), &rbytes))
		return sirius::app::server::arbitrator::process::controller::err_code_t::fail;

	int32_t count = rbytes / sizeof(DWORD);
	for (int32_t i = 0; i < count; ++i)
	{
		if (processes[i] == pid)
			return sirius::app::server::arbitrator::process::controller::err_code_t::success;
	}
	return sirius::app::server::arbitrator::process::controller::err_code_t::fail;
}

int32_t sirius::app::server::arbitrator::process::controller::find(const char * name)
{
	int32_t status = sirius::app::server::arbitrator::process::controller::err_code_t::fail;
	DWORD processes[MAX_PROCESS_LIST_COUNT];
	DWORD rbytes;

	if (!EnumProcesses(processes, sizeof(processes), &rbytes))
		return sirius::app::server::arbitrator::process::controller::err_code_t::fail;

	int32_t count = rbytes / sizeof(DWORD);
	for (int32_t i = 0; i < count; ++i)
	{
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_ALL_ACCESS, FALSE, processes[i]);

		if (hProcess)
		{
			TCHAR szImagePath[MAX_PATH] = { 0 };
			if (GetProcessImageFileName(hProcess, szImagePath, sizeof(szImagePath)))
			{
				char* pname = nullptr;
				sirius::stringhelper::convert_wide2multibyte(szImagePath, &pname);
				std::string proc_path = pname;
				std::string proc_name = proc_path.substr(proc_path.find_last_of("/\\") + 1);

				if (proc_name.compare(name) == 0)
					status = sirius::app::server::arbitrator::process::controller::err_code_t::success;

				if (pname)
				{
					free(pname);
					pname = nullptr;
				}
			}
			CloseHandle(hProcess);
		}
		if (status == sirius::app::server::arbitrator::process::controller::err_code_t::success)
			break;
	}
	return status;
}

void sirius::app::server::arbitrator::process::controller::set_cmdline(const char* src_cmd, const char * add_cmd, ...)
{
	char tmp_cmd[MAX_PATH] = { 0 };
	va_list ap;
	va_start(ap, add_cmd);
	vsprintf_s(tmp_cmd, add_cmd, ap);
	va_end(ap);

	if (strlen(src_cmd) != 0)
	{
		strcat_s((char *)src_cmd, FIXED_COMMAND_OPTION_SIZE, " ");
	}
	strcat_s((char *)src_cmd, FIXED_COMMAND_OPTION_SIZE, tmp_cmd);
}

void sirius::app::server::arbitrator::process::controller::retrieve_absolute_module_path(const char * fileName, char ** path)
{
	HINSTANCE module_handle = ::GetModuleHandleA(fileName);
	char module_path[MAX_PATH] = { 0 };
	char * module_name = module_path;
	module_name += ::GetModuleFileNameA(module_handle, module_name, (sizeof(module_path) / sizeof(*module_path)) - (module_name - module_path));
	if (module_name != module_path)
	{
		char * slash = strrchr(module_path, '\\');
		if (slash != NULL)
		{
			module_name = slash + 1;
			_strset_s(module_name, strlen(module_path), 0);
		}
		else
		{
			_strset_s(module_path, strlen(module_path), 0);
		}
	}

	int32_t pathlen = strlen(module_path) + 1;
	(*path) = static_cast<char*>(malloc(pathlen));
	memset((*path), 0x00, pathlen);
	strncpy_s((*path), pathlen, module_path, pathlen);
}

HWND sirius::app::server::arbitrator::process::controller::hwnd_from_pid(unsigned long pid)
{
	HWND temp_hwnd = ::FindWindow(NULL, NULL);

	while (temp_hwnd != NULL)
	{
		if (::GetParent(temp_hwnd) == NULL)
			if (pid == pid_from_hwnd(temp_hwnd))
				return temp_hwnd;
		temp_hwnd = GetWindow(temp_hwnd, GW_HWNDNEXT);
	}
	return NULL;
}

unsigned long sirius::app::server::arbitrator::process::controller::pid_from_hwnd(HWND hwnd)
{
	unsigned long pid;
	GetWindowThreadProcessId(hwnd, &pid);
	return pid;
}