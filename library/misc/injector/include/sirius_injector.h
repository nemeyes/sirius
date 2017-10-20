#pragma once

#include <winbase.h>
#include <string>
#include <winternl.h>
#include <psapi.h>


#include <tlhelp32.h>
#include <map>
#include <vector>

#ifdef C64
typedef long long           PARAM;
typedef unsigned long long  UPARAM;
#else
typedef long                PARAM;
typedef unsigned long       UPARAM;
#endif


inline BOOL is_64bit_windows()
{
#if defined(_WIN64)
	return TRUE;
#elif defined(_WIN32)
	BOOL f64 = FALSE;
	return IsWow64Process(GetCurrentProcess(), &f64) && f64;
#endif
}
typedef HANDLE(WINAPI *CRTPROC)(HANDLE, LPSECURITY_ATTRIBUTES, SIZE_T, LPTHREAD_START_ROUTINE, LPVOID, DWORD, LPDWORD);
typedef BOOL(WINAPI *WPMPROC)(HANDLE, LPVOID, LPCVOID, SIZE_T, SIZE_T*);
typedef LPVOID(WINAPI *VAEPROC)(HANDLE, LPVOID, SIZE_T, DWORD, DWORD);
typedef BOOL(WINAPI *VFEPROC)(HANDLE, LPVOID, SIZE_T, DWORD);
typedef HANDLE(WINAPI *OPPROC) (DWORD, BOOL, DWORD);
typedef HANDLE(WINAPI *OPPROC) (DWORD, BOOL, DWORD);

typedef NTSTATUS(NTAPI *_NtQueryInformationProcess)(
	HANDLE ProcessHandle,
	DWORD ProcessInformationClass,
	PVOID ProcessInformation,
	DWORD ProcessInformationLength,
	PDWORD ReturnLength
	);

namespace sirius
{
	namespace misc
	{
		class injector
		{
		public:
			static bool			inject_dx11_gpu_process(DWORD parent_pid);
			static bool			get_process_handle(DWORD process_id, bool bgpu_process, std::wstring dll);
			static int32_t		get_gpu_process_id(int32_t parent_process_id);
			static int32_t		get_process_id_by_name(std::string & process_name);
			static DWORD_PTR	get_process_base_address(DWORD process_id);
			static int32_t		get_x64_process_id_by_name(std::string & process_name);
			static char *		get_process_command_line(int32_t pid);
			static int32_t		get_parent_pid(int32_t pid = -1);

		private:
			static BOOL WINAPI	inject_library(HANDLE process, const wchar_t * dll, DWORD len);
			static BOOL			check_file_integrity(LPCTSTR str_dll);
			static PVOID		get_peb_address(HANDLE process);
			static bool			get_cmd_line(HANDLE process);
			static std::wstring get_current_directory_on_windows(void);
			static bool			is_gpu_process(int pid);
		};

	};
};

