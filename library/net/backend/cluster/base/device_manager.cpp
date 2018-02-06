#include "device_manager.h"
#define IS_ALNUM(ch) \
            ( ch >= 'a' && ch <= 'z' ) || \
            ( ch >= 'A' && ch <= 'Z' ) || \
            ( ch >= '0' && ch <= '9' ) || \
            ( ch >= '-' && ch <= '.' ) 
sirius::library::net::backend::device_manager::device_manager()
{
}

sirius::library::net::backend::device_manager::~device_manager()
{
}

char * sirius::library::net::backend::device_manager::get_cpu_name()
{
	char* cpu_name = NULL;
	wchar_t wc_cpu_name[100];
	HKEY hkey;
	int i = 0;
	long result = 0;
	DWORD size = sizeof(wc_cpu_name);
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Hardware\\Description\\System\\CentralProcessor\\0", 0, KEY_QUERY_VALUE, &hkey);
	RegQueryValueEx(hkey, L"ProcessorNameString", NULL, NULL, (LPBYTE)wc_cpu_name, &size);
	RegCloseKey(hkey);

	int wc_size = WideCharToMultiByte(CP_ACP, 0, wc_cpu_name, -1, NULL, 0, NULL, NULL);
	cpu_name = new char[wc_size];
	WideCharToMultiByte(CP_ACP, 0, wc_cpu_name, -1, cpu_name, wc_size, 0, 0);
	cpu_name = url_encode(cpu_name);
	return cpu_name;
}

char* sirius::library::net::backend::device_manager::url_encode(const char* str)
{
	int i, j = 0, len;
	char* tmp;
	len = strlen(str);
	tmp = (char*)malloc((sizeof(char) * 3 * len) + 1);
	for (i = 0; i < len; i++)
	{
		if (IS_ALNUM(str[i]))
			tmp[j] = str[i];
		else {
			snprintf(&tmp[j], 4, "%%%02X\n", (unsigned char)str[i]);
			j += 2;
		}
		j++;
	}
	tmp[j] = 0;

	return tmp;
}

char * sirius::library::net::backend::device_manager::get_operatingsystem_name()
{
	char* os = NULL;
	wchar_t product_name[100];
	wchar_t csd_version[100];
	std::wstring operatingsystem_info;
	HKEY hkey;
	int i = 0;
	DWORD size = 100;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\", 0, KEY_QUERY_VALUE, &hkey) != ERROR_SUCCESS)
	{
		return NULL;
	}
	if (RegQueryValueEx(hkey, L"ProductName", NULL, NULL, (LPBYTE)product_name, &size) != ERROR_SUCCESS)
	{
		return NULL;
	}
	if (RegQueryValueEx(hkey, L"CSDVersion", NULL, NULL, (LPBYTE)csd_version, &size) != ERROR_SUCCESS)
	{
		RegCloseKey(hkey);
		int str_size = WideCharToMultiByte(CP_ACP, 0, product_name, -1, NULL, 0, NULL, NULL);
		os = new char[str_size];
		WideCharToMultiByte(CP_ACP, 0, product_name, -1, os, str_size, 0, 0);
		os = url_encode(os);
		return os;
	}

	operatingsystem_info = product_name;
	operatingsystem_info = L" ";
	operatingsystem_info = csd_version;
	RegCloseKey(hkey);

	const wchar_t* wstr = operatingsystem_info.c_str();
	int str_size = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	os = new char[str_size];
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, os, str_size, 0, 0);
	os = url_encode(os);
	os = url_encode(os);
	return os;
}

int sirius::library::net::backend::device_manager::get_memory_info()
{
	MEMORYSTATUSEX memory;
	memory.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memory);
	char memory_str[128];
	sprintf_s(memory_str, 128, "%I64d", memory.ullTotalPhys / 1024 / 1024 / 1000);
	return atoi(memory_str);
}