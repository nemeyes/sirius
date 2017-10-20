#include "stdafx.h"
#include "sirius_injector.h"

#include "userenv.h"
#include "wtsapi32.h"
#include "winnt.h"

#define NUM_OF_WAITING_CHILD 2

bool sirius::misc::injector::inject_dx11_gpu_process(DWORD dwProcessID)
{
	HANDLE			hProcessSnapshot;
	PROCESSENTRY32	processInfo;
	int nWaitingChild = NUM_OF_WAITING_CHILD;
	std::map<DWORD, bool> childMap;
	int nTryCount = 0;
	bool bFound = false;
	std::pair<std::map<DWORD, bool>::iterator, bool> ret;

	do
	{
		hProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hProcessSnapshot != INVALID_HANDLE_VALUE)
		{
			processInfo.dwSize = sizeof(PROCESSENTRY32);
			if (Process32First(hProcessSnapshot, &processInfo))
			{
				do
				{
					if (dwProcessID == processInfo.th32ParentProcessID)
					{
						DWORD dwChildPID = processInfo.th32ProcessID;
						ret = childMap.insert(std::pair<DWORD, bool>(dwChildPID, false));
						if (ret.second == false)
						{
							trace(TEXT("GetChildProcessList() Already found!! child process id is %d\n"), dwChildPID);
							continue;
						}
						trace(TEXT("GetChildProcessList() child process id is %d\n"), dwChildPID);

						if ((bFound = get_process_handle(dwChildPID, true, TEXT("cap_video_capture"))) ||
							--nWaitingChild == 0)
						{
							break;
						}
					}
				} while (Process32Next(hProcessSnapshot, &processInfo));

				CloseHandle(hProcessSnapshot);
			}
		}
		if (!bFound)
		{
			trace(TEXT("Try Count = %d\n"), ++nTryCount);
			if (nTryCount > 10)
			{
				trace(TEXT("Cannot find child process!!\n"));
				break;
			}
			Sleep(100);
		}
	} while (!bFound);

	return bFound;
}

BOOL WINAPI sirius::misc::injector::inject_library(HANDLE hProcess, const wchar_t *pDLL, DWORD dwLen)
{
	DWORD   dwTemp, dwSize, lastError;
	BOOL    bSuccess, bRet = 0;
	HANDLE  hThread = NULL;
	LPVOID  pStr = NULL;
	UPARAM  procAddress;
	SIZE_T  writtenSize;

	WPMPROC pWriteProcessMemory;
	CRTPROC pCreateRemoteThread;
	VAEPROC pVirtualAllocEx;
	VFEPROC pVirtualFreeEx;
	HMODULE hK32;
	char pWPMStr[19], pCRTStr[19], pVAEStr[15], pVFEStr[14], pLLStr[13];
	int obfSize = 12;
	int i;

	/*--------------------------------------------------------*/

	if (!hProcess) return 0;

	dwSize = (dwLen + 1) * sizeof(wchar_t);

	/*--------------------------------------------------------*/


	memcpy(pWPMStr, "RvnrdPqmni|}Dmfegm", 19); //WriteProcessMemory with each character obfuscated
	memcpy(pCRTStr, "FvbgueQg`c{k]`yotp", 19); //CreateRemoteThread with each character obfuscated
	memcpy(pVAEStr, "WiqvpekGeddiHt", 15);     //VirtualAllocEx with each character obfuscated
	memcpy(pVFEStr, "Wiqvpek@{mnOu", 14);      //VirtualFreeEx with each character obfuscated
	memcpy(pLLStr, "MobfImethzr", 12);         //LoadLibrary with each character obfuscated

#ifdef UNICODE
	pLLStr[11] = 'W';
#else
	pLLStr[11] = 'A';
#endif
	pLLStr[12] = 0;

	obfSize += 6;
	for (i = 0; i < obfSize; i++) pWPMStr[i] ^= i ^ 5;
	for (i = 0; i < obfSize; i++) pCRTStr[i] ^= i ^ 5;

	obfSize -= 4;
	for (i = 0; i < obfSize; i++) pVAEStr[i] ^= i ^ 1;

	obfSize -= 1;
	for (i = 0; i < obfSize; i++) pVFEStr[i] ^= i ^ 1;

	obfSize -= 2;
	for (i = 0; i < obfSize; i++) pLLStr[i] ^= i ^ 1;

	hK32 = GetModuleHandle(TEXT("KERNEL32"));
	pWriteProcessMemory = (WPMPROC)GetProcAddress(hK32, pWPMStr);
	pCreateRemoteThread = (CRTPROC)GetProcAddress(hK32, pCRTStr);
	pVirtualAllocEx = (VAEPROC)GetProcAddress(hK32, pVAEStr);
	pVirtualFreeEx = (VFEPROC)GetProcAddress(hK32, pVFEStr);

	/*--------------------------------------------------------*/

	pStr = (LPVOID)(*pVirtualAllocEx)(hProcess, NULL, dwSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (!pStr) goto end;

	bSuccess = (*pWriteProcessMemory)(hProcess, pStr, (LPVOID)pDLL, dwSize, &writtenSize);
	if (!bSuccess) goto end;

	procAddress = (UPARAM)GetProcAddress(hK32, pLLStr);
	if (!procAddress) goto end;

	hThread = (*pCreateRemoteThread)(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)procAddress,
		pStr, 0, &dwTemp);
	if (!hThread) goto end;

	if (WaitForSingleObject(hThread, 200) == WAIT_OBJECT_0)
	{
		DWORD dw;
		GetExitCodeThread(hThread, &dw);
		bRet = dw != 0;

		SetLastError(0);
	}

end:
	if (!bRet)
		lastError = GetLastError();

	if (hThread)
		CloseHandle(hThread);

	if (pStr)
		(*pVirtualFreeEx)(hProcess, pStr, 0, MEM_RELEASE);

	if (!bRet)
		SetLastError(lastError);

	return bRet;
}

BOOL sirius::misc::injector::check_file_integrity(LPCTSTR strDLL)
{
	HANDLE hFileTest = CreateFile(strDLL, GENERIC_READ | GENERIC_EXECUTE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFileTest == INVALID_HANDLE_VALUE)
	{
		std::wstring strWarning;

		DWORD err = GetLastError();
		if (err == ERROR_FILE_NOT_FOUND)
			strWarning = TEXT("Important capture files have been deleted.");
		else if (err == ERROR_ACCESS_DENIED)
			strWarning = TEXT("Important capture files can not be loaded. ");
		else
			strWarning = TEXT("Important game capture files can not be loaded (error ") + err;
		trace(TEXT("inject_helper::check_file_integrity: Error %d while accessing %s"), err, strDLL);

		return FALSE;
	}
	else
	{
		CloseHandle(hFileTest);
		return TRUE;
	}
}

PVOID sirius::misc::injector::get_peb_address(HANDLE ProcessHandle)
{
	_NtQueryInformationProcess NtQueryInformationProcess =
		(_NtQueryInformationProcess)GetProcAddress(
			GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess");
	PROCESS_BASIC_INFORMATION pbi;

	NTSTATUS status = NtQueryInformationProcess(ProcessHandle, 0, &pbi, sizeof(pbi), NULL);

	return pbi.PebBaseAddress;
}

bool sirius::misc::injector::get_cmd_line(HANDLE hProcess)
{
	bool ret = false;
	PVOID pebAddress;
	PVOID rtlUserProcParamsAddress;
	UNICODE_STRING commandLine;
	WCHAR *commandLineContents;

	pebAddress = get_peb_address(hProcess);

	/* get the address of ProcessParameters */
	if (!ReadProcessMemory(hProcess, (PCHAR)pebAddress + 0x10,
		&rtlUserProcParamsAddress, sizeof(PVOID), NULL))
	{
		trace(TEXT("Could not read the address of ProcessParameters!\n"));
		return ret;
	}

	/* read the CommandLine UNICODE_STRING structure */
	if (!ReadProcessMemory(hProcess, (PCHAR)rtlUserProcParamsAddress + 0x40,
		&commandLine, sizeof(commandLine), NULL))
	{
		trace(TEXT("Could not read CommandLine!\n"));
		return ret;
	}

	/* allocate memory to hold the command line */
	commandLineContents = (WCHAR *)malloc(commandLine.MaximumLength + 1);
	ZeroMemory(commandLineContents, commandLine.MaximumLength + 1);

	/* read the command line */
	if (!ReadProcessMemory(hProcess, commandLine.Buffer,
		commandLineContents, commandLine.Length, NULL))
	{
		trace(TEXT("Could not read the command line string!\n"));
		free(commandLineContents);
		return ret;
	}
	if (wcsstr(commandLineContents, TEXT("--type=gpu-process")) > 0)
	{
		ret = true;
	}
	//CloseHandle(hProcess);
	free(commandLineContents);

	return ret;
}

std::wstring sirius::misc::injector::get_current_directory_on_windows()
{
	WCHAR currentDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, currentDir);
	return std::wstring(currentDir);
}

int sirius::misc::injector::get_process_id_by_name(std::string &process_name)
{
	HANDLE hProcessSnap;
	//HANDLE hProcess;
	PROCESSENTRY32 pe32;
	//DWORD dwPriorityClass;
	int ret_process_id = -1;
	char exec_process_name[MAX_PATH] = { 0 };

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		trace(TEXT("CreateToolhelp32Snapshot (of processes) is null."));
		return -1;
	}

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof(PROCESSENTRY32);

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if (!Process32First(hProcessSnap, &pe32))
	{
		trace(TEXT("Process32First is false %d"),GetLastError()); // show cause of failure
		CloseHandle(hProcessSnap);          // clean the snapshot object
		return -1;
	}

	// Now walk the snapshot of processes, and
	// display information about each process in turn
	do
	{
		trace(TEXT("=====================================================\n"));
		trace(TEXT("PROCESS NAME:  %s[%u]\n"), pe32.szExeFile,pe32.th32ProcessID);
		trace(TEXT("-------------------------------------------------------\n"));

		UINT32 len = WideCharToMultiByte(CP_ACP, 0, pe32.szExeFile, wcslen(pe32.szExeFile), NULL, NULL, NULL, NULL);
		::ZeroMemory(exec_process_name, (len + 1) * sizeof(char));
		if (len > MAX_PATH) {
			trace(TEXT("Process name is over MAX_PATH."));
			break;
		}

		WideCharToMultiByte(CP_ACP, 0, pe32.szExeFile, -1, exec_process_name, len, NULL, NULL);
		if (process_name.compare(exec_process_name) == 0)
		{
			ret_process_id = pe32.th32ProcessID;
			break;
		}
	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return ret_process_id;
}

bool sirius::misc::injector::get_process_handle(DWORD dwprocess_id, bool bgpu_process, std::wstring dll_name)
{
	bool ret = false;
	int retVal = 0;
	bool useSafeHook = false;
	bool berror = false;
	char pOPStr[12];
	memcpy(pOPStr, "NpflUvhel{x", 12);
	for (int i = 0; i < 11; i++) pOPStr[i] ^= i ^ 1;

	OPPROC pOpenProcess = (OPPROC)GetProcAddress(GetModuleHandle(TEXT("KERNEL32")), pOPStr);

	DWORD permission = useSafeHook ? (PROCESS_QUERY_INFORMATION | PROCESS_VM_READ) : (PROCESS_ALL_ACCESS);

	HANDLE hProcess = (*pOpenProcess)(permission, FALSE, dwprocess_id);
	if (hProcess)
	{
		if (!bgpu_process || get_cmd_line(hProcess))
		{
			BOOL bEqualBit = TRUE;
			BOOL b32bit = TRUE;
			//std::vector<std::wstring> vdll_names;

			if (is_64bit_windows())
			{
				BOOL bWow64, bTargetWow64;
				IsWow64Process(GetCurrentProcess(), &bWow64);
				IsWow64Process(hProcess, &bTargetWow64);

				bEqualBit = (bWow64 == bTargetWow64);
				bTargetWow64 = b32bit;
			}

			std::wstring strDLL = get_current_directory_on_windows();
			strDLL += TEXT("\\") + dll_name;
			//strDLL += vdll_names.back();
			//vdll_names.pop_back();

			if (!b32bit)
				strDLL += TEXT("64");

			strDLL += TEXT(".dll");
			if (!check_file_integrity(strDLL.c_str()))
			{
				trace(L"Error: capture file wrong\n");
				berror = true;
			}
			else
			{

				if (bEqualBit && !useSafeHook)
				{
					if (!inject_library(hProcess, strDLL.c_str(), strDLL.length() + 1))
					{
						retVal = GetLastError();
						if (!retVal)
							retVal = -5;
					}
					else
					{
						ret = true;
					}
				}
				else
				{
					retVal = -3;
				}
			}
		}
		CloseHandle(hProcess);
	}
	else
	{
		trace(TEXT("AttemptCapture: OpenProcess failed, GetLastError = %u\n"), GetLastError());
	}

	return ret;
}

char * sirius::misc::injector::get_process_command_line(int pid)
{
	HANDLE process_handle = nullptr;
	PVOID peb_address = nullptr;;
	PVOID rtl_user_proc_params_address = nullptr;;
	UNICODE_STRING command_line;
	WCHAR *command_line_contents = nullptr;
	char *out_command_line_contents = nullptr;

	if ((process_handle = OpenProcess(
		PROCESS_QUERY_INFORMATION | /* required for NtQueryInformationProcess */
		PROCESS_VM_READ, /* required for ReadProcessMemory */
		FALSE, pid)) == 0)
	{
		trace(TEXT("Could not open process pid=(%d)! Error (%d) \n"), pid, GetLastError());
		return out_command_line_contents;
	}

	peb_address = get_peb_address(process_handle);

	/* get the address of ProcessParameters */
	if (!ReadProcessMemory(process_handle, (PCHAR)peb_address + 0x10,
		&rtl_user_proc_params_address, sizeof(PVOID), NULL))
	{
		trace(TEXT("Could not read the address of ProcessParameters Error (%d) !\n"), GetLastError());
		return out_command_line_contents;
	}

	/* read the CommandLine UNICODE_STRING structure */
	if (!ReadProcessMemory(process_handle, (PCHAR)rtl_user_proc_params_address + 0x40,
		&command_line, sizeof(command_line), NULL))
	{
		trace(TEXT("Could not read CommandLine Error (%d) !\n"),GetLastError());
		return out_command_line_contents;
	}

	/* allocate memory to hold the command line */
	command_line_contents = (WCHAR *)malloc(command_line.Length);
	ZeroMemory(command_line_contents, command_line.Length);

	/* read the command line */
	if (!ReadProcessMemory(process_handle, command_line.Buffer,
		command_line_contents, command_line.Length, NULL))
	{
		trace(TEXT("Could not read command line string Error (%d) !\n"),GetLastError());
		if (command_line_contents != nullptr)
			free(command_line_contents);
		return out_command_line_contents;
	}

	trace(TEXT("process command size:%d, %s"), wcslen(command_line_contents), command_line_contents);

	//cap_string_helper::convert_wide2multibyte(command_line_contents, &out_command_line_contents);
	UINT32 len = WideCharToMultiByte(CP_ACP, 0, command_line_contents, wcslen(command_line_contents), NULL, NULL, NULL, NULL);
	out_command_line_contents = new char[NULL, len + 1];
	::ZeroMemory(out_command_line_contents, (len + 1) * sizeof(char));
	WideCharToMultiByte(CP_ACP, 0, command_line_contents, -1, out_command_line_contents, len, NULL, NULL);

	CloseHandle(process_handle);

	if (command_line_contents != nullptr)
		free(command_line_contents);

	return out_command_line_contents;
}

int32_t sirius::misc::injector::get_parent_pid(int32_t pid)
{
	DWORD cur_pid = -1, parent_pid = -1;
	HANDLE         process_snap = NULL;
	PROCESSENTRY32 pe32 = { 0 };
	
	cur_pid = (pid == -1 ? GetCurrentProcessId() : -1);

	process_snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (process_snap == INVALID_HANDLE_VALUE)
		return -1;

	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(process_snap, &pe32)) {
		do {
			if (pe32.th32ProcessID == cur_pid) {
				parent_pid = pe32.th32ParentProcessID;
				break;
			}
		} while (Process32Next(process_snap, &pe32));
	}
	CloseHandle(process_snap);

	return parent_pid;
}

bool sirius::misc::injector::is_gpu_process(int pid)
{
	char command_line[1024] = { '\0' };
	//if (false == GetProcessCommandLine(pid, command_line, 1024))
	if(get_process_command_line(pid) == nullptr)
		return false;

	std::string cmd = command_line;
	if (std::string::npos != cmd.find("--type=gpu-process")) {
		return true;
	}

	return false;
}

int sirius::misc::injector::get_gpu_process_id(int parent_processid)
{
	DWORD curPID = parent_processid;
	HANDLE         process_snap = NULL;
	PROCESSENTRY32 pe32 = { 0 };
	int   gpu_PID = 0;

	process_snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (process_snap == INVALID_HANDLE_VALUE)
		return 0;

	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(process_snap, &pe32)) {
		do {
			if (pe32.th32ParentProcessID == curPID) {
				if (is_gpu_process(pe32.th32ProcessID)) { // check the type=gpu-process of command line of process
					trace(TEXT("GPU PID(%d), Exe(%s)"), pe32.th32ProcessID, pe32.szExeFile);
					gpu_PID = pe32.th32ProcessID;
					break;
				}
			}
		} while (Process32Next(process_snap, &pe32));
	}
	CloseHandle(process_snap);

	return gpu_PID;
}

