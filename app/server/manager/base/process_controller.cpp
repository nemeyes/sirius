#include "sirius_server_manager.h"

#include <wtsapi32.h>
#include <tlhelp32.h> 
#include <Shlwapi.h>
#include <string>
#include <Userenv.h>
#include <windows.h>
#include <psapi.h>
#include "process_controller.h"
#include "sirius_stringhelper.h"
#include "sirius_log4cplus_logger.h"

#define FIXED_COMMAND_OPTION_SIZE	1024

#define TA_FAILED 0
#define TA_SUCCESS_CLEAN 1
#define TA_SUCCESS_KILL 2
#define MAX_PROCESS_LIST_COUNT 1024

sirius::app::server::manager::controller::controller(void)
{
}

sirius::app::server::manager::controller::~controller(void)
{
}

void sirius::app::server::manager::controller::add_cmd_option(char* srcCmd, char *addCmd, ...)
{
	char tmpAddCmd[MAX_PATH] = { 0 };
	va_list ap;
	va_start(ap, addCmd);
	vsprintf_s(tmpAddCmd, addCmd, ap);
	va_end(ap);

	if (strlen(srcCmd) != 0)
	{
		strcat_s(srcCmd, FIXED_COMMAND_OPTION_SIZE, " ");
	}
	strcat_s(srcCmd, FIXED_COMMAND_OPTION_SIZE, tmpAddCmd);
}

bool sirius::app::server::manager::controller::safe_terminate_process(DWORD process_Id, UINT exit_code)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_Id);

	DWORD dwTID, dwCode, dwErr = 0;
	HANDLE hProcessDup = INVALID_HANDLE_VALUE;
	HANDLE hRT = NULL;
	HINSTANCE hKernel = GetModuleHandle(_T("Kernel32"));

	bool bSuccess = false;
	BOOL bDup = DuplicateHandle(GetCurrentProcess(),
		hProcess,
		GetCurrentProcess(),
		&hProcessDup,
		PROCESS_ALL_ACCESS,
		FALSE,
		0);
	if (GetExitCodeProcess((bDup) ? hProcessDup : hProcess, &dwCode)
		&& (dwCode == STILL_ACTIVE))
	{
		FARPROC pfnExitProc;
		pfnExitProc = GetProcAddress(hKernel, "ExitProcess");
		hRT = CreateRemoteThread((bDup) ? hProcessDup : hProcess,
			NULL,
			0,
			(LPTHREAD_START_ROUTINE)pfnExitProc,
			(PVOID)exit_code, 0, &dwTID);
		if (hRT == NULL) dwErr = GetLastError();
	}
	else
	{
		dwErr = ERROR_PROCESS_ABORTED;
	}
	if (hRT)
	{
		WaitForSingleObject((bDup) ? hProcessDup : hProcess, INFINITE);
		CloseHandle(hRT);
		bSuccess = true;
	}
	if (bDup)
		CloseHandle(hProcessDup);
	if (!bSuccess)
		SetLastError(dwErr);
	if (hProcess)
		CloseHandle(hProcess);

	return bSuccess;
}


bool sirius::app::server::manager::controller::run(const char * path, const char * arguments, unsigned long & pid)
{
#if 0
	std::string process_path = path;
	std::string process_arguments = arguments;

	PHANDLE primaryToken = get_current_user_token();
	if (primaryToken == 0)
		return FALSE;

	STARTUPINFOA StartupInfo;
	PROCESS_INFORMATION processInfo;
	StartupInfo.cb = sizeof(STARTUPINFO);

	SECURITY_ATTRIBUTES Security1;
	SECURITY_ATTRIBUTES Security2;

	std::string command = "\"" + process_path + "\"";
	if (process_arguments.length() > 0)
	{
		command += " " + process_arguments;
	}

	void* lpEnvironment = NULL;
	BOOL resultEnv = CreateEnvironmentBlock(&lpEnvironment, primaryToken, FALSE);
	if (resultEnv == 0)
	{
		long nError = GetLastError();
	}

	BOOL result = CreateProcessAsUserA(primaryToken, 0, (LPSTR)(command.c_str()), &Security1, &Security2, FALSE, NORMAL_PRIORITY_CLASS | CREATE_UNICODE_ENVIRONMENT, lpEnvironment, 0, &StartupInfo, &processInfo);
	::OutputDebugStringA(command.c_str());

	DestroyEnvironmentBlock(lpEnvironment);
	CloseHandle(primaryToken);
	return result ? true : false;
#else
#if 0
	std::string process_path = path;
	std::string process_arguments = arguments;

	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	memset(&si, 0x00, sizeof(si));
	memset(&pi, 0x00, sizeof(pi));
	si.cb = sizeof(STARTUPINFO);

	std::string command = process_path;
	if (process_arguments.length() > 0)
	{
		command += " " + process_arguments;
	}

	char tmp_process_path[260] = { 0 };
	strncpy_s(tmp_process_path, process_path.c_str(), sizeof(tmp_process_path) - 1);

	char working_directory[MAX_PATH] = { 0 };
	wchar_t _working_directroy[MAX_PATH] = { 0 };

	size_t converted = 0;
	mbstowcs_s(&converted, _working_directroy, strlen(path) + 1, path, _TRUNCATE);
	::PathRemoveFileSpec(_working_directroy);
	wcstombs_s(&converted, working_directory, wcslen(_working_directroy) + 1, _working_directroy, _TRUNCATE);
	strcat_s(working_directory, sizeof(working_directory), "\\");


	BOOL result = ::CreateProcessA(NULL, (LPSTR)command.c_str(), NULL, NULL, FALSE, 0, NULL, working_directory, &si, &pi);
	if (!result)
	{
		DWORD err = GetLastError();
	}
	pid = pi.dwProcessId;

	return result ? true : false;
#else
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	memset(&si, 0x00, sizeof(si));
	memset(&pi, 0x00, sizeof(pi));
	si.cb = sizeof(STARTUPINFO);

	BOOL result = FALSE;
	std::string command = path;
	if (arguments && strlen(arguments)> 0)
	{
		command += " ";
		command += arguments;
	}

	char * working_directory = nullptr;
	retrieve_absolute_module_path(path, &working_directory);
	if (working_directory && strlen(working_directory) > 0)
	{
		result = ::CreateProcessA(NULL, (LPSTR)(LPCSTR)command.c_str(), NULL, NULL, FALSE, 0, NULL, working_directory, &si, &pi);
		if (!result)
		{
			DWORD err = GetLastError();
		}
		pid = pi.dwProcessId;
		if (working_directory)
			free(working_directory);
	}
	return result ? true : false;
#endif


#endif
}

void sirius::app::server::manager::controller::retrieve_absolute_module_path(const char * fileName, char ** path)
{
	HINSTANCE module_handle = ::GetModuleHandleA(fileName);
	char module_path[260] = { 0 };
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

	size_t length = strlen(module_path) + 1;
	(*path) = static_cast<char*>(malloc(length));
	memset((*path), 0x00, length);
	memcpy((*path), &module_path[0], length);
}

bool sirius::app::server::manager::controller::process_kill(DWORD process_id)
{
	DWORD dwProcessArray[MAX_PROCESS_LIST_COUNT];
	DWORD dwBytesReturned;

	if (!EnumProcesses(dwProcessArray, sizeof(dwProcessArray), &dwBytesReturned))
	{
		return false;
	}

	int nCount = dwBytesReturned / sizeof(DWORD);

	for (int i = 0; i < nCount; ++i)
	{
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_TERMINATE, FALSE, dwProcessArray[i]);

		if (hProcess)
		{
			for (int i = 0; i < nCount; ++i)
			{
				if (dwProcessArray[i] == process_id)
					TerminateProcess(hProcess, 0);
			}
			
			CloseHandle(hProcess);
		}
	}
	return true;
}

bool sirius::app::server::manager::controller::process_kill(const char* process_name)
{

	DWORD dwProcessArray[MAX_PROCESS_LIST_COUNT];
	DWORD dwBytesReturned;

	if (!EnumProcesses(dwProcessArray, sizeof(dwProcessArray), &dwBytesReturned))
	{
		return false;
	}

	int nCount = dwBytesReturned / sizeof(DWORD);

	for (int i = 0; i < nCount; ++i)
	{
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_TERMINATE, FALSE, dwProcessArray[i]);

		if (hProcess)
		{
			DWORD cbNeeded;
			HMODULE hModule;

			if (EnumProcessModules(hProcess, &hModule, sizeof(hModule), &cbNeeded))
			{
				TCHAR szImagePath[MAX_PATH];
				GetModuleFileNameEx(hProcess, hModule, szImagePath, sizeof(szImagePath));

				char* pname = nullptr;
				sirius::stringhelper::convert_wide2multibyte(szImagePath, &pname);
				std::string proc_path = pname;
				std::string proc_name = proc_path.substr(proc_path.find_last_of("/\\") + 1);

				if (proc_name.compare(process_name) == 0)
					TerminateProcess(hProcess, 0);

				if (pname)
				{
					free(pname);
					pname = nullptr;
				}
			}
			CloseHandle(hProcess);
		}
	}
	return true;
}

bool sirius::app::server::manager::controller::process_find(DWORD process_id)
{
	DWORD dwProcessArray[MAX_PROCESS_LIST_COUNT];
	DWORD dwBytesReturned;

	if (!EnumProcesses(dwProcessArray, sizeof(dwProcessArray), &dwBytesReturned))
		return false;

	int nCount = dwBytesReturned / sizeof(DWORD);

	for (int i = 0; i < nCount; ++i)
	{
		if (dwProcessArray[i] == process_id)
			return true;
	}
	return false;
}

DWORD WINAPI sirius::app::server::manager::controller::terminate_app(DWORD process_id, DWORD time_out)
{
	HANDLE	hProc;
	DWORD	dwRet;

	// If we can't open the process with PROCESS_TERMINATE rights,
	// then we give up immediately.
	hProc = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, FALSE, process_id);

	if (hProc == NULL)
	{
		return TA_FAILED;
	}

	// terminate_app_enum() posts WM_CLOSE to all windows whose PID
	// matches your process's.
	EnumWindows((WNDENUMPROC)terminate_app_enum, (LPARAM)process_id);

	// Wait on the handle. If it signals, great. If it times out,
	// then you kill it.
	if (WaitForSingleObject(hProc, time_out) != WAIT_OBJECT_0)
		dwRet = (TerminateProcess(hProc, 0) ? TA_SUCCESS_KILL : TA_FAILED);
	else
		dwRet = TA_SUCCESS_CLEAN;

	CloseHandle(hProc);

	return dwRet;
}


BOOL CALLBACK sirius::app::server::manager::controller::terminate_app_enum(HWND hwnd, LPARAM lParam)
{
	DWORD dwID;

	GetWindowThreadProcessId(hwnd, &dwID);

	if (dwID == (DWORD)lParam)
		PostMessage(hwnd, WM_CLOSE, 0, 0);

	return TRUE;
}


bool sirius::app::server::manager::controller::launch_app_different_session(char* path, char* arguments, int session_id)
{
	
	wchar_t *unicode_path;
	sirius::stringhelper::convert_multibyte2wide(path, &unicode_path);

	wchar_t *unicode_arguments;
	sirius::stringhelper::convert_multibyte2wide(arguments, &unicode_arguments);
	
	HMODULE hInstWtsapi32 = NULL;
	typedef BOOL(WINAPI *WTSQueryUserTokenPROC)(ULONG SessionId, PHANDLE phToken);
	WTSQueryUserTokenPROC WTSQueryUserToken = NULL;
	LUID luid;
	BOOL bResult = FALSE;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	DWORD dwCreationFlags = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE;
	LPVOID pEnv = NULL;
	HANDLE hToken = NULL;

	ZeroMemory(&si, sizeof(STARTUPINFO));
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

	si.wShowWindow = SW_SHOW;
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.lpDesktop = _T("WinSta0\\Default");
	si.wShowWindow = SW_SHOW;
				
	hInstWtsapi32 = LoadLibrary(_T("Wtsapi32.dll"));

	if (!hInstWtsapi32)
	{		
		// Wtsapi32.dll Load Failed"
		return S_FALSE;
	}

	if (!LookupPrivilegeValue(NULL, SE_TCB_NAME, &luid))
	{
		//LookupPrivilegeValue Failed"
		return S_FALSE;
	}

	WTSQueryUserToken = (WTSQueryUserTokenPROC)GetProcAddress(hInstWtsapi32, ("WTSQueryUserToken"));

	if (!WTSQueryUserToken(session_id, &hToken))
	{		
		LOGGER::make_error_log(SAW, "%s(), WTSQueryUserToken Failed!! ErrorCode = {}", __FUNCTION__, GetLastError());
		return S_FALSE;
	}
	
	if (hToken == NULL)
	{
		//hToken is NULL
		return S_FALSE;
	}
			
	//CreateProcessAsUser call
	bResult = ::CreateProcessAsUser(hToken, unicode_path, unicode_arguments, NULL, NULL, FALSE, dwCreationFlags, pEnv, NULL, &si, &pi);
	
	if (!bResult)
	{		
		LOGGER::make_error_log(SAW, "%s(), CreateProcessAsUser Failed!! ErrorCode=%d", __FUNCTION__, GetLastError());
	}

	if (unicode_path)
		SysFreeString(unicode_path);

	if (unicode_arguments)
		SysFreeString(unicode_arguments);


	if ((bResult) && (pi.hProcess != INVALID_HANDLE_VALUE))
		CloseHandle(pi.hProcess);
	if ((bResult) && (pi.hThread != INVALID_HANDLE_VALUE))
		CloseHandle(pi.hThread);

	if (hInstWtsapi32)
		FreeLibrary(hInstWtsapi32);
	if (hToken)
		CloseHandle(hToken);

	return S_OK;

}

bool sirius::app::server::manager::controller::process_find(char *process_name)
{
	bool result = false;

	DWORD dwProcessArray[MAX_PROCESS_LIST_COUNT];
	DWORD dwBytesReturned;

	if (!EnumProcesses(dwProcessArray, sizeof(dwProcessArray), &dwBytesReturned))
	{
		return false;
	}

	int nCount = dwBytesReturned / sizeof(DWORD);

	for (int i = 0; i < nCount; ++i)
	{
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_ALL_ACCESS, FALSE, dwProcessArray[i]);

		if (hProcess)
		{
			TCHAR szImagePath[MAX_PATH] = { 0 };
			if (GetProcessImageFileName(hProcess, szImagePath, sizeof(szImagePath)))
			{
				char* pname = nullptr;
				sirius::stringhelper::convert_wide2multibyte(szImagePath, &pname);
				std::string proc_path = pname;
				std::string proc_name = proc_path.substr(proc_path.find_last_of("/\\") + 1);

				if (proc_name.compare(process_name) == 0)
					result = true;

				if (pname)
				{
					free(pname);
					pname = nullptr;
				}
			}
			CloseHandle(hProcess);
		}
		if (result)	break;
	}
	return result;
}

DWORD sirius::app::server::manager::controller::find_process_id(char *process_name)
{
	PROCESSENTRY32 procEntry;

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE)
	{
		return 1;
	}

	procEntry.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hSnap, &procEntry))
	{
		return 1;
	}

	do
	{
		char strProcessName[MAX_PATH] = { 0, };
		WideCharToMultiByte(CP_ACP, 0, procEntry.szExeFile, MAX_PATH, strProcessName, MAX_PATH, NULL, NULL);

		if (_stricmp(strProcessName, process_name) == 0)
		{
				return procEntry.th32ProcessID;
				break;
		}

	} while (Process32Next(hSnap, &procEntry));

	return -1;
}