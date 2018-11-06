#include "sirius_log4cplus_logger.h"
#include <sirius_version.h>
#include <shlwapi.h>
#if defined(WITH_DISABLE)
#include <stdio.h>
#include <stdarg.h>
#else
#include <codecvt>
#include <locale.h>
#include <log4cplus/logger.h>
#include <log4cplus/appender.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/configurator.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/helpers/sleep.h>
#include <log4cplus/loggingmacros.h>
#include <tchar.h>
#include <process.h>
#include <userenv.h>
#include <wtsapi32.h>
#include <io.h>
#include <direct.h>
#include <atlstr.h>
#include <atltime.h>

static log4cplus::SharedAppenderPtr _append;
static log4cplus::SharedAppenderPtr _append_error;
static log4cplus::SharedAppenderPtr _append_attendant;
static log4cplus::SharedAppenderPtr _append_attendant_error;
static log4cplus::Logger			_logger_coordinator;
static log4cplus::Logger			_logger_error;
static log4cplus::Logger			_logger_attendant;
static log4cplus::Logger			_logger_attendant_error;
#endif

sirius::library::log::log4cplus::logger::critical_section::critical_section(void)
{
	::InitializeCriticalSection(&_cs);
}

sirius::library::log::log4cplus::logger::critical_section::~critical_section(void)
{
	::DeleteCriticalSection(&_cs);
}

void sirius::library::log::log4cplus::logger::critical_section::lock(void)
{
	::EnterCriticalSection(&_cs);
}

void sirius::library::log::log4cplus::logger::critical_section::unlock(void)
{
	::LeaveCriticalSection(&_cs);
}

sirius::library::log::log4cplus::logger::scopped_lock::scopped_lock(sirius::library::log::log4cplus::logger::critical_section * cs)
	: _cs(cs)
{
	_cs->lock();
}

sirius::library::log::log4cplus::logger::scopped_lock::~scopped_lock(void)
{
	_cs->unlock();
}

sirius::library::log::log4cplus::logger::trace_logger::trace_logger(const char * section, const char * fmt, ...)
{
#if defined(WITH_DISABLE)
	memset(_log, 0x00, sizeof(_log));
	memcpy(_log, "TRACE : ", strlen("TRACE :"));

	int index = strlen(_log);
	char * rlog = &_log[index];

	va_list args;
	va_start(args, fmt);
	vsnprintf_s(rlog, sizeof(_log) - index, sizeof(_log) - index, fmt, args);
	va_end(args);

	index = strlen(_log);
	if (index > (sizeof(_log) - 1))
	{
		_log[max_message_size - 1] = 0;
		_log[max_message_size - 2] = '\n';
		_log[max_message_size - 3] = '\r';
	}
	else
	{
		_log[index] = '\r';
		_log[index + 1] = '\n';
		_log[index + 2] = 0;
	}

	char log[max_message_size] = { "ENTER :" };
	index = strlen(log);
	rlog = &log[index];
	strncpy_s(rlog, max_message_size - index, _log, strlen(_log));
	::OutputDebugStringA(log);
#else
	strncpy_s(_section, section, strlen(section));
	memset(_log, 0x00, sizeof(_log));
	va_list args;
	va_start(args, fmt);
	vsnprintf_s(_log, sizeof(_log), fmt, args);
	va_end(args);

	char log[max_message_size] = { "ENTER :" };
	int32_t index = strlen(log);
	char * rlog = &log[index];
	strncpy_s(rlog, max_message_size - index, _log, strlen(_log));
	sirius::library::log::log4cplus::logger::make_trace_log(_section, log);
#endif
}

sirius::library::log::log4cplus::logger::trace_logger::~trace_logger(void)
{
#if defined(WITH_DISABLE)
	char log[max_message_size] = { "LEAVE :" };
	int index = strlen(log);
	char * rlog = &log[index];
	strncpy_s(rlog, max_message_size - index, _log, strlen(_log));
	::OutputDebugStringA(log);
#else
	char log[max_message_size] = { "LEAVE :" };
	int32_t index = strlen(log);
	char * rlog = &log[index];
	strncpy_s(rlog, max_message_size - index, _log, strlen(_log));
	sirius::library::log::log4cplus::logger::make_trace_log(_section, log);
#endif
}

sirius::library::log::log4cplus::logger * sirius::library::log::log4cplus::logger::_instance = nullptr;
char	sirius::library::log::log4cplus::logger::_default_section[] = "root";
char	sirius::library::log::log4cplus::logger::_pattern[MAX_PATH];
char	sirius::library::log::log4cplus::logger::_dir_path[MAX_PATH];
char	sirius::library::log::log4cplus::logger::_file_path[MAX_PATH];
int		sirius::library::log::log4cplus::logger::_log_type;
int		sirius::library::log::log4cplus::logger::_diff_log_type;
int		sirius::library::log::log4cplus::logger::_change_log_type;
char	sirius::library::log::log4cplus::logger::_device_id[MAX_PATH];
char	sirius::library::log::log4cplus::logger::_log_name[MAX_PATH];
char	sirius::library::log::log4cplus::logger::_device_log_name[MAX_PATH];
char	sirius::library::log::log4cplus::logger::_module_path[MAX_PATH];
TCHAR	sirius::library::log::log4cplus::logger::_wc_last_file_name[MAX_PATH];
TCHAR	sirius::library::log::log4cplus::logger::wc_last_error_file_name[MAX_PATH];
TCHAR	sirius::library::log::log4cplus::logger::wc_last_attendant_file_name[MAX_PATH];
TCHAR	sirius::library::log::log4cplus::logger::wc_last_attendant_error_file_name[MAX_PATH];
SYSTEMTIME					sirius::library::log::log4cplus::logger::_sys_time;
_WIN32_FILE_ATTRIBUTE_DATA  sirius::library::log::log4cplus::logger::_file_att_data;
fpn_file_changed			sirius::library::log::log4cplus::logger::_pfn_file_changed;
HANDLE						sirius::library::log::log4cplus::logger::_change_handles[2];
HINSTANCE					sirius::library::log::log4cplus::logger::_self;
void sirius::library::log::log4cplus::logger::create(const char * configuration_path, const char * log_type, char * device_id)
{
	scopped_lock mutex(&g_log4cplus_critical_section);
	if (!_instance)
	{
		_instance = new sirius::library::log::log4cplus::logger(configuration_path, log_type, device_id);
		file_monitor_start();
	}
}

void sirius::library::log::log4cplus::logger::destroy(void)
{
	scopped_lock mutex(&g_log4cplus_critical_section);
	if (_instance)
	{
		file_monitor_stop();
		delete _instance;
		_instance = nullptr;
	}
}

bool sirius::library::log::log4cplus::logger::first_log_directory_check(const char * log_type)
{
#if !defined(WITH_DISABLE)
	char _directory_name[MAX_PATH] = { 0, };
	TCHAR file_name[MAX_PATH] = { 0, };
	char folder_path[MAX_PATH] = { 0, };
	char file_name_final[MAX_PATH] = { 0, };
	if (::PathFileExists(L"D:\\Log\\"))
		sprintf_s(folder_path, "D:\\Log");
	else
	{
		_self = ::GetModuleHandleA("sirius_log4cplus_logger.dll");

		::GetModuleFileName(_self, file_name, MAX_PATH);
		::PathRemoveFileSpec(file_name);
		int file_name_size = WideCharToMultiByte(CP_ACP, 0, file_name, -1, NULL, 0, NULL, NULL);
		char* str_ptr = new char[file_name_size];
		WideCharToMultiByte(CP_ACP, 0, file_name, -1, str_ptr, file_name_size, 0, 0);
		char *ptr = strstr(str_ptr, "apps");
		int len = strlen(ptr) - 3;
		if (ptr != NULL)
		{
			strncpy_s(file_name_final, sizeof(file_name_final), str_ptr, file_name_size - len);
			sprintf_s(folder_path, "%s\\Log", file_name_final);
		}
	}

	SYSTEMTIME _time;
	GetLocalTime(&_time);
	SYSTEMTIME *diff_time = &(_sys_time);
	if (_time.wYear != diff_time->wYear || _time.wMonth != diff_time->wMonth || _time.wDay != diff_time->wDay)
	{
		diff_time->wYear = _time.wYear;
		diff_time->wMonth = _time.wMonth;
		diff_time->wDay = _time.wDay;

		test_log("%s_%d: %d/%d/%d, diff=%d/%d/%d",
			__FUNCTION__, __LINE__, _time.wYear, _time.wMonth, _time.wDay, diff_time->wYear, diff_time->wMonth, diff_time->wDay);

		if (strcmp(log_type, SAC) == 0)
		{
			sprintf_s(_directory_name, MAX_PATH, "%s\\sirius\\%04d-%02d-%02d\\", folder_path, _time.wYear, _time.wMonth, _time.wDay);
			sprintf_s(_log_name, MAX_PATH, "%s", _directory_name);
		}
		else
		{
			sprintf_s(_directory_name, MAX_PATH, "%s\\sirius\\%04d-%02d-%02d\\", folder_path, _time.wYear, _time.wMonth, _time.wDay);
			sprintf_s(_log_name, MAX_PATH, "%s", _directory_name);
			sprintf_s(_directory_name, MAX_PATH, "%s\\sirius\\%04d-%02d-%02d\\stb\\", folder_path, _time.wYear, _time.wMonth, _time.wDay);
			sprintf_s(_device_log_name, MAX_PATH, "%s", _directory_name);
		}
		wchar_t _wtext[MAX_PATH];
		size_t ret_val = 0;
		mbstowcs_s(&ret_val,_wtext,MAX_PATH, _directory_name, strlen(_directory_name) + 1);
		LPWSTR ptr = _wtext;
		TCHAR tmb_folder_name[MAX_PATH] = { 0, };
		MultiByteToWideChar(CP_ACP, 0, folder_path, strlen(folder_path), tmb_folder_name, MAX_PATH);
		if (::PathFileExists(L"D:\\Log") || ::PathFileExists(tmb_folder_name))
		{
			TCHAR szPathBuffer[MAX_PATH] = { 0, };
			size_t len = _tcslen(ptr);
			for (size_t i = 0; i < len; i++)
			{
				szPathBuffer[i] = *(ptr + i);
				if (szPathBuffer[i] == _T('\\') || szPathBuffer[i] == _T('/'))
				{
					szPathBuffer[i + 1] = NULL;
					if (!::PathFileExists(szPathBuffer))
						CreateDirectory(szPathBuffer, NULL);
				}
			}
			if (strcmp(log_type, SLNS) == 0 || strcmp(log_type, SLNSC) == 0)
			{
				log_configuration(SLNSC);
				streamer_log_init(_device_id, SLNS);
			}
			else
			log_configuration(log_type);

			return true;
		}
	}
#endif
	return false;
}

void sirius::library::log::log4cplus::logger::log_directory_check(const char * secion)
{
#if !defined(WITH_DISABLE)
	scopped_lock mutex(&g_log4cplus_critical_section);
	char _directory_name[MAX_PATH] = { 0, };
	TCHAR file_name[MAX_PATH] = { 0, };
	char folder_path[MAX_PATH] = { 0, };
	char file_name_final[MAX_PATH] = { 0, };

	SYSTEMTIME _time;
	GetLocalTime(&_time);

	test_log("%s_%d: now_t=%d/%d/%d, _sys_time=%d/%d/%d, secion=%s", __FUNCTION__, __LINE__, 
		_time.wYear, _time.wMonth, _time.wDay, _sys_time.wYear, _sys_time.wMonth, _sys_time.wDay, secion);

	if (_time.wYear != _sys_time.wYear || _time.wMonth != _sys_time.wMonth || _time.wDay != _sys_time.wDay )
	{
	if (::PathFileExists(L"D:\\Log\\"))
		sprintf_s(folder_path, "D:\\Log");
	else
	{
		_self = ::GetModuleHandleA("sirius_log4cplus_logger.dll");

		::GetModuleFileName(_self, file_name, MAX_PATH);
		::PathRemoveFileSpec(file_name);
			int file_name_size = WideCharToMultiByte(CP_ACP, 0, file_name, -1, NULL, 0, NULL, NULL);
			char* str_ptr = new char[file_name_size];
			WideCharToMultiByte(CP_ACP, 0, file_name, -1, str_ptr, file_name_size, 0, 0);
			char *ptr = strstr(str_ptr, "apps");
			int len = strlen(ptr) - 3;
			if (ptr != NULL)
			{
				strncpy_s(file_name_final, sizeof(file_name_final), str_ptr, file_name_size - len);
				sprintf_s(folder_path, "%s\\Log", file_name_final);
			}
	}
		memcpy(&_sys_time, &_time, sizeof(SYSTEMTIME));
	
		test_log("in , %d/%d/%d, _sys_time=%d/%d/%d, secion=%s", 
			_time.wYear, _time.wMonth, _time.wDay, _sys_time.wYear, _sys_time.wMonth, _sys_time.wDay, secion);

		if (strcmp(secion, SAC) == 0)
		{
			sprintf_s(_directory_name, MAX_PATH, "%s\\sirius\\%04d-%02d-%02d\\", folder_path, _time.wYear, _time.wMonth, _time.wDay);
			sprintf_s(_log_name, MAX_PATH, "%s", _directory_name);
			//_diff_log_type = app_manager;
		}
		else
		{
			sprintf_s(_directory_name, MAX_PATH, "%s\\sirius\\%04d-%02d-%02d\\", folder_path, _time.wYear, _time.wMonth, _time.wDay);
			sprintf_s(_log_name, MAX_PATH, "%s", _directory_name);
			sprintf_s(_directory_name, MAX_PATH, "%s\\sirius\\%04d-%02d-%02d\\stb\\", folder_path, _time.wYear, _time.wMonth, _time.wDay);
			sprintf_s(_device_log_name, MAX_PATH, "%s", _directory_name);
		}

		wchar_t _wtext[MAX_PATH];
		size_t ret_val = 0;
		mbstowcs_s(&ret_val,_wtext,_directory_name, strlen(_directory_name) + 1);
		LPWSTR ptr = _wtext;
		TCHAR tmb_folder_name[MAX_PATH] = { 0, };
		MultiByteToWideChar(CP_ACP, 0, folder_path, strlen(folder_path), tmb_folder_name, MAX_PATH);
		if (::PathFileExists(L"D:\\Log") || ::PathFileExists(tmb_folder_name))
		{
			TCHAR szPathBuffer[MAX_PATH] = { 0, };
			size_t len = _tcslen(ptr);
			for (size_t i = 0; i < len; i++)
			{
				szPathBuffer[i] = *(ptr + i);
				if (szPathBuffer[i] == _T('\\') || szPathBuffer[i] == _T('/'))
				{
					szPathBuffer[i + 1] = NULL;
					if (!PathFileExists(szPathBuffer))
						CreateDirectory(szPathBuffer, NULL);
				}
			}
		}
		if (strcmp(secion, SLNS) == 0 || strcmp(secion, SLNSC) == 0)
		{
			log_configuration(SLNSC);
			streamer_log_init(_device_id, SLNS);
		}
		else
		log_configuration(secion);
	}
#endif
}

void sirius::library::log::log4cplus::logger::log_configuration(const char * secion)
{
#if !defined(WITH_DISABLE)
	scopped_lock mutex(&g_log4cplus_critical_section);
	CHAR mb_file_name[MAX_PATH] = { 0 };
	CHAR mb_error_file_name[MAX_PATH] = { 0 };
	WCHAR wc_file_name[MAX_PATH] = { 0 };
	WCHAR wc_buffer[MAX_PATH] = { 0 };
	WCHAR wc_error_file_name[MAX_PATH] = { 0 };
	WCHAR wc_error_buffer[MAX_PATH] = { 0 };
	WCHAR wstr[MAX_PATH] = { 0 };
	WCHAR wstr_error[MAX_PATH] = { 0 };

	int str_len = strlen((char *)_wc_last_file_name);
	if (str_len > 0)
		_append->close();

	::GetModuleFileName(NULL, _wc_last_file_name, MAX_PATH);
	::PathRemoveFileSpec(_wc_last_file_name);
	::PathAppend(_wc_last_file_name, wc_file_name);

		if (strcmp(secion, SAC) == 0)
		{
			sprintf_s(mb_file_name, sizeof(mb_file_name), "%s\\emulator.log", _log_name);
			MultiByteToWideChar(CP_ACP, 0, SAC, -1, wc_buffer, MAX_PATH);
			_log_type = client;
		}
		else if (strcmp(secion, SAS) == 0)
		{
			sprintf_s(mb_file_name, sizeof(mb_file_name), "%s\\stressor.log", _log_name);
			MultiByteToWideChar(CP_ACP, 0, SAS, -1, wc_buffer, MAX_PATH);
			_log_type = stressor;
		}
		else if (strcmp(secion, SAW) == 0)
		{
			sprintf_s(mb_file_name, sizeof(mb_file_name), "%s\\watchdog.log", _log_name);
		MultiByteToWideChar(CP_ACP, 0, SAW, -1, wc_buffer, MAX_PATH);
			_log_type = watchdog;
		}
	else if (strcmp(secion, SLNSC) == 0)
	{
		sprintf_s(mb_file_name, sizeof(mb_file_name), "%s\\attendant.log", _log_name);
		MultiByteToWideChar(CP_ACP, 0, SLNSC, -1, wc_buffer, MAX_PATH);
		_log_type = streamer_create;
	}
	else if (strcmp(secion, SLNS) == 0)
	{

	}
		else
		{
			sprintf_s(mb_file_name, sizeof(mb_file_name), "%s\\sirius_%s.log", _log_name, GEN_VER_VERSION_STRING);
		MultiByteToWideChar(CP_ACP, 0, SAA, -1, wc_buffer, MAX_PATH);
			_log_type = arbitrator;
		}

	test_log("%s_%d: log_filename=%s", __FUNCTION__, __LINE__, mb_file_name);

	MultiByteToWideChar(CP_ACP, 0, mb_file_name, -1, wc_file_name, MAX_PATH);
	_append = new ::log4cplus::RollingFileAppender(wc_file_name, 50 * 1024 * 1024, 100);
	//_append = (new ::log4cplus::DailyRollingFileAppender(wc_file_name, MINUTELY, true, 10));
	sprintf_s(_pattern, MAX_PATH, "%s", "%-8p %D{%y/%m/%d  %H:%M:%S:%q} - %m%n");

	MultiByteToWideChar(CP_ACP, 0, _pattern, strlen(_pattern), wstr, MAX_PATH);
	std::auto_ptr<::log4cplus::Layout> _layout(new ::log4cplus::PatternLayout(wstr));
	_append->setName(wc_file_name);
	_append->setLayout(_layout);

	_logger_coordinator = ::log4cplus::Logger::getInstance(wc_buffer);
	_logger_coordinator.addAppender(_append);
	bool ret = configure_load();
	if(ret == false)
		_logger_coordinator.setLogLevel(::log4cplus::INFO_LOG_LEVEL);

	/*if (strlen(change_device_id) > 0)
	{
		MultiByteToWideChar(CP_ACP, 0, SLVSC, -1, wc_buffer, MAX_PATH);
		_logger_coordinator = ::log4cplus::Logger::getInstance(wc_buffer);
		_logger_coordinator.addAppender(_append);
		_logger_coordinator.setLogLevel(::log4cplus::INFO_LOG_LEVEL);

		
		MultiByteToWideChar(CP_ACP, 0, CAPAC, -1, wc_buffer, MAX_PATH);
		_logger_coordinator = ::log4cplus::Logger::getInstance(wc_buffer);
		_logger_coordinator.addAppender(_append);
		_logger_coordinator.setLogLevel(::log4cplus::INFO_LOG_LEVEL);
		
	}*/

	int str_last_len = strlen((char *)wc_last_error_file_name);
	if (str_last_len > 0)
		_append_error->close();

	::GetModuleFileName(NULL, wc_last_error_file_name, MAX_PATH);
	::PathRemoveFileSpec(wc_last_error_file_name);
	::PathAppend(wc_last_error_file_name, wc_error_file_name);

	sprintf_s(mb_error_file_name, sizeof(mb_error_file_name), "%s\\sirius_error.log", _log_name);
	MultiByteToWideChar(CP_ACP, 0, mb_error_file_name, -1, wc_error_file_name, MAX_PATH);

	_append_error = new ::log4cplus::RollingFileAppender(wc_error_file_name, 50 * 1024 * 1024, 50);
	sprintf_s(_pattern, MAX_PATH, "%s", "%-8p %D{%y/%m/%d  %H:%M:%S:%q} - %m%n");

	MultiByteToWideChar(CP_ACP, 0, _pattern, strlen(_pattern), wstr_error, MAX_PATH);
	std::auto_ptr<::log4cplus::Layout> _layout_error(new ::log4cplus::PatternLayout(wstr_error));
	_append_error->setName(wc_error_file_name);
	MultiByteToWideChar(CP_ACP, 0, SBE, -1, wc_error_buffer, MAX_PATH);
	_logger_error = ::log4cplus::Logger::getInstance(wc_error_buffer);
	_logger_error.addAppender(_append_error);
	_logger_error.setLogLevel(::log4cplus::ERROR_LOG_LEVEL);
	_append_error->setLayout(_layout_error);
#endif
}

sirius::library::log::log4cplus::logger::logger(const char * configuration_path, const char * log_type, char * device_id)
{
#if !defined(WITH_DISABLE)
	std::locale::global(     // set global locale
		std::locale(         // using std::locale constructed from
			std::locale(),   // global locale
							 // and codecvt_utf8 facet
			new std::codecvt_utf8<wchar_t, 0x10FFFF, static_cast<std::codecvt_mode>(std::consume_header | std::little_endian)>));

	::log4cplus::initialize();
	try
	{
		HINSTANCE self;
		self = ::GetModuleHandleA("sirius_log4cplus_logger.dll");
		CHAR szModuleName[MAX_PATH] = { 0 };
		CHAR szModuleFindPath[MAX_PATH] = { 0 };

		CHAR *pszModuleName = _module_path;
		pszModuleName += GetModuleFileNameA(self, pszModuleName, (sizeof(_module_path) / sizeof(*_module_path)) - (pszModuleName - _module_path));
		if (pszModuleName != _module_path)
		{
			CHAR *slash = strrchr(_module_path, '\\');
			if (slash != NULL)
			{
				pszModuleName = slash + 1;
				_strset_s(pszModuleName, strlen(pszModuleName) + 1, 0);
			}
			else
			{
				_strset_s(_module_path, strlen(pszModuleName) + 1, 0);
			}
		}

		_snprintf_s(_module_path, sizeof(_module_path), "%s%s", _module_path, configuration_path);
		set_file_monitor(_module_path, configure_load);
		memcpy(_device_id, device_id, MAX_PATH);
		bool result = first_log_directory_check(log_type);
		if (result == false) { log_configuration(log_type); }
	}
	catch (...)
	{
		_configure_thread = nullptr;
	}
#endif
}

sirius::library::log::log4cplus::logger::~logger(void)
{
#if !defined(WITH_DISABLE)
	if (_configure_thread)
	{
		delete _configure_thread;
		_configure_thread = nullptr;
	}
#endif
}

DWORD WINAPI sirius::library::log::log4cplus::logger::monitor_Thread_proc(LPVOID lpParam)
{
#if !defined(WITH_DISABLE)
	sirius::library::log::log4cplus::logger * _monitor = (sirius::library::log::log4cplus::logger*)lpParam;
	if (_monitor == NULL)
	{
		return 0;
	}
	WCHAR wc_buffer[MAX_PATH] = { 0 };
	DWORD wait_status;
	_WIN32_FILE_ATTRIBUTE_DATA file_att_data;
	MultiByteToWideChar(CP_ACP, 0, _dir_path, -1, wc_buffer, MAX_PATH);
	_monitor->_change_handles[0] = FindFirstChangeNotification(wc_buffer, FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);
	if (_monitor->_change_handles[0] == INVALID_HANDLE_VALUE)
	{
		DWORD dError = GetLastError();
		ExitProcess(GetLastError());
	}
	_monitor->_change_handles[1] = CreateSemaphore(NULL, 0, 1, NULL);
	if (_monitor->_change_handles[1] == NULL)
		return 0;

	while (TRUE)
	{
		wait_status = WaitForMultipleObjects(2, _monitor->_change_handles, FALSE, INFINITE);
		switch (wait_status)
		{
		case WAIT_OBJECT_0:
			MultiByteToWideChar(CP_ACP, 0, _monitor->_file_path, -1, wc_buffer, MAX_PATH);
			GetFileAttributesEx(wc_buffer, GetFileExInfoStandard, (void*)&file_att_data);
			if (CompareFileTime(
				&file_att_data.ftLastWriteTime,
				&_monitor->_file_att_data.ftLastWriteTime) != 0)
			{
				int size;
				FILE *fp = fopen(_monitor->_file_path, "r");   
				fseek(fp, 0, SEEK_END); 
				size = ftell(fp);
				fclose(fp);

				if (size <= 0)
					file_save();

				(*_monitor->_pfn_file_changed)();
				_monitor->_file_att_data = file_att_data;
			}
			if (FindNextChangeNotification(_change_handles[0]) == FALSE)
				return 0;
			break;
		case WAIT_OBJECT_0 + 1:
			return 0;
			break;

		default:
			return 0;
		}
	}
#endif
	return 0;
}

void sirius::library::log::log4cplus::logger::make_fatal_log(const char * secion, const char * fmt, ...)
{
	if (_instance)
	{
#if defined(WITH_DISABLE)
		char log[max_message_size] = { "FATAL :" };
		int index = strlen(log);
		char * rlog = &log[index];

		va_list args;
		va_start(args, fmt);
		vsnprintf_s(rlog, sizeof(log) - index, sizeof(log) - index, fmt, args);
		va_end(args);

		index = strlen(log);
		if (index > (sizeof(log) - 1))
		{
			log[max_message_size - 1] = 0;
			log[max_message_size - 2] = '\n';
			log[max_message_size - 3] = '\r';
		}
		else
		{
			log[index] = '\r';
			log[index + 1] = '\n';
			log[index + 2] = 0;
		}
		::OutputDebugStringA(log);
#else
		log_directory_check(secion);
		char log[max_message_size] = { 0 };
		va_list args;
		va_start(args, fmt);
		vsnprintf_s(log, sizeof(log), fmt, args);
		va_end(args);
		LOG4CPLUS_FATAL(::log4cplus::Logger::getInstance(LOG4CPLUS_STRING_TO_TSTRING(secion)), LOG4CPLUS_STRING_TO_TSTRING(log));
#endif
	}
}

void sirius::library::log::log4cplus::logger::make_error_log(const char * secion, const char * fmt, ...)
{
	if (_instance)
	{
#if defined(WITH_DISABLE)
		char log[max_message_size] = { "ERROR :" };
		int index = strlen(log);
		char * rlog = &log[index];

		va_list args;
		va_start(args, fmt);
		vsnprintf_s(rlog, sizeof(log) - index, sizeof(log) - index, fmt, args);
		va_end(args);

		index = strlen(log);
		if (index > (sizeof(log) - 1))
		{
			log[max_message_size - 1] = 0;
			log[max_message_size - 2] = '\n';
			log[max_message_size - 3] = '\r';
		}
		else
		{
			log[index] = '\r';
			log[index + 1] = '\n';
			log[index + 2] = 0;
		}
		::OutputDebugStringA(log);
#else
		log_directory_check(secion);
		char log[max_message_size] = { 0 };
		va_list args;
		va_start(args, fmt);
		vsnprintf_s(log, sizeof(log), fmt, args);
		va_end(args);
		LOG4CPLUS_ERROR(::log4cplus::Logger::getInstance(LOG4CPLUS_STRING_TO_TSTRING(secion)), LOG4CPLUS_STRING_TO_TSTRING(log));
		LOG4CPLUS_ERROR(::log4cplus::Logger::getInstance(LOG4CPLUS_STRING_TO_TSTRING(SBE)), LOG4CPLUS_STRING_TO_TSTRING(log));
#endif
	}
}

void sirius::library::log::log4cplus::logger::make_warn_log(const char * secion, const char * fmt, ...)
{
	if (_instance)
	{
#if defined(WITH_DISABLE)
		char log[max_message_size] = { "WARN :" };
		int index = strlen(log);
		char * rlog = &log[index];

		va_list args;
		va_start(args, fmt);
		vsnprintf_s(rlog, sizeof(log) - index, sizeof(log) - index, fmt, args);
		va_end(args);

		index = strlen(log);
		if (index > (sizeof(log) - 1))
		{
			log[max_message_size - 1] = 0;
			log[max_message_size - 2] = '\n';
			log[max_message_size - 3] = '\r';
		}
		else
		{
			log[index] = '\r';
			log[index + 1] = '\n';
			log[index + 2] = 0;
		}
		::OutputDebugStringA(log);
#else
		log_directory_check(secion);
		char log[max_message_size] = { 0 };
		va_list args;
		va_start(args, fmt);
		vsnprintf_s(log, sizeof(log), fmt, args);
		va_end(args);
		LOG4CPLUS_WARN(::log4cplus::Logger::getInstance(LOG4CPLUS_STRING_TO_TSTRING(secion)), LOG4CPLUS_STRING_TO_TSTRING(log));
#endif
	}
}

void sirius::library::log::log4cplus::logger::make_info_log(const char * secion, const char * fmt, ...)
{
	if (_instance)
	{
#if defined(WITH_DISABLE)
		char log[max_message_size] = { "INFO :" };
		int index = strlen(log);
		char * rlog = &log[index];

		va_list args;
		va_start(args, fmt);
		vsnprintf_s(rlog, sizeof(log) - index, sizeof(log) - index, fmt, args);
		va_end(args);

		index = strlen(log);
		if (index > (sizeof(log) - 1))
		{
			log[max_message_size - 1] = 0;
			log[max_message_size - 2] = '\n';
			log[max_message_size - 3] = '\r';
		}
		else
		{
			log[index] = '\r';
			log[index + 1] = '\n';
			log[index + 2] = 0;
		}
		::OutputDebugStringA(log);
#else
		log_directory_check(secion);
		char log[max_message_size] = { 0 };
		va_list args;
		va_start(args, fmt);
		vsnprintf_s(log, sizeof(log), fmt, args);
		va_end(args);
		LOG4CPLUS_INFO(::log4cplus::Logger::getInstance(LOG4CPLUS_STRING_TO_TSTRING(secion)), LOG4CPLUS_STRING_TO_TSTRING(log));
#endif
	}
}

void sirius::library::log::log4cplus::logger::make_debug_log(const char * secion, const char * fmt, ...)
{
	if (_instance)
	{
#if defined(WITH_DISABLE)
		char log[max_message_size] = { "DEBUG :" };
		int index = strlen(log);
		char * rlog = &log[index];

		va_list args;
		va_start(args, fmt);
		vsnprintf_s(rlog, sizeof(log) - index, sizeof(log) - index, fmt, args);
		va_end(args);

		index = strlen(log);
		if (index > (sizeof(log) - 1))
		{
			log[max_message_size - 1] = 0;
			log[max_message_size - 2] = '\n';
			log[max_message_size - 3] = '\r';
		}
		else
		{
			log[index] = '\r';
			log[index + 1] = '\n';
			log[index + 2] = 0;
		}
		::OutputDebugStringA(log);
#else
		log_directory_check(secion);
		char log[max_message_size] = { 0 };
		va_list args;
		va_start(args, fmt);
		vsnprintf_s(log, sizeof(log), fmt, args);
		va_end(args);
		LOG4CPLUS_DEBUG(::log4cplus::Logger::getInstance(LOG4CPLUS_STRING_TO_TSTRING(secion)), LOG4CPLUS_STRING_TO_TSTRING(log));
#endif
	}
}

void sirius::library::log::log4cplus::logger::make_trace_log(const char * secion, const char * fmt, ...)
{
	if (_instance)
	{
#if defined(WITH_DISABLE)
		char log[max_message_size] = { "TRACE :" };
		int index = strlen(log);
		char * rlog = &log[index];

		va_list args;
		va_start(args, fmt);
		vsnprintf_s(rlog, sizeof(log) - index, sizeof(log) - index, fmt, args);
		va_end(args);

		index = strlen(log);
		if (index > (sizeof(log) - 1))
		{
			log[max_message_size - 1] = 0;
			log[max_message_size - 2] = '\n';
			log[max_message_size - 3] = '\r';
		}
		else
		{
			log[index] = '\r';
			log[index + 1] = '\n';
			log[index + 2] = 0;
		}
		::OutputDebugStringA(log);
#else
		log_directory_check(secion);
		char log[max_message_size] = { 0 };
		va_list args;
		va_start(args, fmt);
		vsnprintf_s(log, sizeof(log), fmt, args);
		va_end(args);
		LOG4CPLUS_TRACE(::log4cplus::Logger::getInstance(LOG4CPLUS_STRING_TO_TSTRING(secion)), LOG4CPLUS_STRING_TO_TSTRING(log));
#endif
	}
}
char * sirius::library::log::log4cplus::logger::replace_all(char * s, const char * olds, const char * news)
{
	char *result, *sr;
	size_t i, count = 0;
	size_t old_len = strlen(olds); if (old_len < 1) return s;
	size_t new_len = strlen(news);

	if (new_len != old_len) {
		for (i = 0; s[i] != '\0';) {
			if (memcmp(&s[i], olds, old_len) == 0) count++, i += old_len;
			else i++;
		}
	}
	else i = strlen(s);

	result = (char *)malloc(i + 1 + count * (new_len - old_len));
	if (result == NULL) return NULL;

	sr = result;
	while (*s) {
		if (memcmp(s, olds, old_len) == 0) {
			memcpy(sr, news, new_len);
			sr += new_len;
			s += old_len;
		}
		else *sr++ = *s++;
	}
	*sr = '\0';

	return result;
}

bool sirius::library::log::log4cplus::logger::set_file_monitor(std::string file_path, fpn_file_changed pfn)
{
#if !defined(WITH_DISABLE)
	WCHAR wc_buffer[MAX_PATH] = { 0 };
	std::string dir_path;
	if (file_path.size() == 0 || pfn == NULL)
		return false;

	int pos = file_path.rfind("\\");
	if (pos == -1)
	{
		pos = file_path.rfind("/");
		if (pos == -1)
			return false;
	}
	dir_path = file_path.substr(0, pos + 1);
	sprintf_s(_dir_path, MAX_PATH, "%s", dir_path.c_str());
	sprintf_s(_file_path, MAX_PATH, "%s", file_path.c_str());
	MultiByteToWideChar(CP_ACP, 0, _file_path, -1, wc_buffer, MAX_PATH);
	GetFileAttributesEx(
		wc_buffer,
		GetFileExInfoStandard,
		(void*)&_file_att_data);
	_pfn_file_changed = pfn;
#endif
	return true;
}

bool sirius::library::log::log4cplus::logger::configure_load()
{
#if !defined(WITH_DISABLE)
	scopped_lock mutex(&g_log4cplus_critical_section);
	TCHAR file_name[MAX_PATH];
	wchar_t pw_module_path[MAX_PATH];
	int len = (int)strlen(_module_path) + 1;
	size_t ret_val = 0;
	mbstowcs_s(&ret_val,pw_module_path, _module_path, len);
	_change_log_type = 0;
	::GetModuleFileName(NULL, file_name, MAX_PATH);
	::PathRemoveFileSpec(file_name);
	::PathAppend(file_name, pw_module_path);
	if (!::PathFileExists(file_name))
		return false;

	TCHAR value[MAX_PATH] = { 0 };
	if (_log_type == arbitrator)
	{
		::GetPrivateProfileString(_T("CONFIGURATION"), _T("LOGLEVEL_SAA"), _T("20000"), value, MAX_PATH, file_name);
		_change_log_type = _ttoi(value);
		log_level_change(_change_log_type, arbitrator);
	}
	else if (_log_type == streamer_create)
	{
		::GetPrivateProfileString(_T("CONFIGURATION"), _T("LOGLEVEL_SLNSC"), _T("20000"), value, MAX_PATH, file_name);
		_change_log_type = _ttoi(value);
		log_level_change(_change_log_type, streamer_create);
	}
	else if (_log_type == streamer)
	{
		::GetPrivateProfileString(_T("CONFIGURATION"), _T("LOGLEVEL_SLNS"), _T("20000"), value, MAX_PATH, file_name);
		_change_log_type = _ttoi(value);
		log_level_change(_change_log_type, streamer);
	}
	else if (_log_type == client)
	{
		::GetPrivateProfileString(_T("CONFIGURATION"), _T("LOGLEVEL_SAC"), _T("20000"), value, MAX_PATH, file_name);
		_change_log_type = _ttoi(value);
		log_level_change(_change_log_type, client);
	}
	else if (_log_type == stressor)
	{
		::GetPrivateProfileString(_T("CONFIGURATION"), _T("LOGLEVEL_SAS"), _T("20000"), value, MAX_PATH, file_name);
		_change_log_type = _ttoi(value);
		log_level_change(_change_log_type, stressor);
	}
	else if (_log_type == watchdog)
	{
		::GetPrivateProfileString(_T("CONFIGURATION"), _T("LOGLEVEL_SAW"), _T("20000"), value, MAX_PATH, file_name);
		_change_log_type = _ttoi(value);
		log_level_change(_change_log_type, watchdog);
	}
#endif
	return true;
}

bool sirius::library::log::log4cplus::logger::file_monitor_start()
{
#if !defined(WITH_DISABLE)
	DWORD thread_id = 0;
	HANDLE hThread = CreateThread(NULL, 0, monitor_Thread_proc, _instance, 0, &thread_id);
#endif
	return true;
}

bool sirius::library::log::log4cplus::logger::file_monitor_stop()
{
#if !defined(WITH_DISABLE)
	if (!ReleaseSemaphore(_change_handles[1], 1, NULL))
	{
		return false;
	}

	::Sleep(100);
#endif
	return true;
}

bool sirius::library::log::log4cplus::logger::file_save()
{
	scopped_lock mutex(&g_log4cplus_critical_section);
	TCHAR file_name[MAX_PATH];
	wchar_t pw_module_path[MAX_PATH];
	int len = (int)strlen(_module_path) + 1;
	size_t ret_val = 0;
	mbstowcs_s(&ret_val, pw_module_path, _module_path, len);
	::GetModuleFileName(NULL, file_name, MAX_PATH);
	::PathRemoveFileSpec(file_name);
	::PathAppend(file_name, pw_module_path);
	if (!::PathFileExists(file_name))
		return false;

	wchar_t log_level[MAX_PATH];
	wsprintfW(log_level, L"%d", _change_log_type);

	::WritePrivateProfileString(_T("CONFIGURATION"), _T("LOGLEVEL_SAA"), log_level, file_name);
	::WritePrivateProfileString(_T("CONFIGURATION"), _T("LOGLEVEL_SLNS"), log_level, file_name);
	::WritePrivateProfileString(_T("CONFIGURATION"), _T("LOGLEVEL_SLNSC"), log_level, file_name);
	::WritePrivateProfileString(_T("CONFIGURATION"), _T("ALL_LOG_LEVEL or TRACE_LOG_LEVEL"), L"0, DEBUG_LOG_LEVEL=10000, INFO_LOG_LEVEL=20000, WARN_LOG_LEVEL=3000, ERROR_LOG_LEVEL=40000, FATAL_LOG_LEVEL=50000, OFF_LOG_LEVEL=60000", file_name);

	return true;
}

void sirius::library::log::log4cplus::logger::log_level_change(int log_level, int log_type)
{
#if !defined(WITH_DISABLE)
	scopped_lock mutex(&g_log4cplus_critical_section);
	if(log_type == streamer)
		_logger_attendant.setLogLevel(log_level);
	else
	_logger_coordinator.setLogLevel(log_level);
#endif
}

void sirius::library::log::log4cplus::logger::streamer_log_init(const char * device_id, const char * log_type)
{
#if !defined(WITH_DISABLE)
	char * change_device_id;
	WCHAR wc_attendant_file_name[MAX_PATH] = { 0 };
	WCHAR wc_attendant_buffer[MAX_PATH] = { 0 };
	WCHAR wstr_attendant[MAX_PATH] = { 0 };
	WCHAR wc_error_file_name[MAX_PATH] = { 0 };
	WCHAR wc_error_buffer[MAX_PATH] = { 0 };
	WCHAR wstr_error[MAX_PATH] = { 0 };
	CHAR mb_attendant_file_name[MAX_PATH] = { 0 };
	CHAR mb_error_file_name[MAX_PATH] = { 0 };
	memcpy(_device_id, device_id, MAX_PATH);
	if (strlen(_device_id) > 0)
	{
		change_device_id = replace_all(_device_id, ":", "-");
		int str__len = strlen((char *)wc_last_attendant_file_name);
		if (str__len > 0)
			_append_attendant->close();

		::GetModuleFileName(NULL, wc_last_attendant_file_name, MAX_PATH);
		::PathRemoveFileSpec(wc_last_attendant_file_name);
		::PathAppend(wc_last_attendant_file_name, wc_attendant_file_name);

		sprintf_s(mb_attendant_file_name, sizeof(mb_attendant_file_name), "%s\\sirius_%s.log", _device_log_name, change_device_id);
		MultiByteToWideChar(CP_ACP, 0, mb_attendant_file_name, -1, wc_attendant_file_name, MAX_PATH);

		_append_attendant = new ::log4cplus::RollingFileAppender(wc_attendant_file_name, 50 * 1024 * 1024, 50);
		sprintf_s(_pattern, MAX_PATH, "%s", "%-8p %D{%y/%m/%d  %H:%M:%S:%q} - %m%n");

		MultiByteToWideChar(CP_ACP, 0, _pattern, strlen(_pattern), wstr_attendant, MAX_PATH);
		std::auto_ptr<::log4cplus::Layout> _layout_error(new ::log4cplus::PatternLayout(wstr_attendant));
		_append_attendant->setName(wc_attendant_file_name);
		MultiByteToWideChar(CP_ACP, 0, SLNS, -1, wc_attendant_buffer, MAX_PATH);
		_logger_attendant = ::log4cplus::Logger::getInstance(wc_attendant_buffer);
		_logger_attendant.addAppender(_append_attendant);
		_log_type = streamer;
		bool ret = configure_load();
		if (ret == false)
			_logger_attendant.setLogLevel(::log4cplus::INFO_LOG_LEVEL);

		_append_attendant->setLayout(_layout_error);
		
		int str_last_len = strlen((char *)wc_last_attendant_error_file_name);
		if (str_last_len > 0)
			_append_attendant_error->close();

		::GetModuleFileName(NULL, wc_last_attendant_error_file_name, MAX_PATH);
		::PathRemoveFileSpec(wc_last_attendant_error_file_name);
		::PathAppend(wc_last_attendant_error_file_name, wc_error_file_name);

		sprintf_s(mb_error_file_name, sizeof(mb_error_file_name), "%s\\sirius_error.log", _log_name);
		MultiByteToWideChar(CP_ACP, 0, mb_error_file_name, -1, wc_error_file_name, MAX_PATH);

		_append_attendant_error = new ::log4cplus::RollingFileAppender(wc_error_file_name, 50 * 1024 * 1024, 50);
		sprintf_s(_pattern, MAX_PATH, "%s", "%-8p %D{%y/%m/%d  %H:%M:%S:%q} - %m%n");

		MultiByteToWideChar(CP_ACP, 0, _pattern, strlen(_pattern), wstr_error, MAX_PATH);
		std::auto_ptr<::log4cplus::Layout> _layout_err(new ::log4cplus::PatternLayout(wstr_error));
		_append_attendant_error->setName(wc_error_file_name);
		MultiByteToWideChar(CP_ACP, 0, SBE, -1, wc_error_buffer, MAX_PATH);
		_logger_attendant_error = ::log4cplus::Logger::getInstance(wc_error_buffer);
		_logger_attendant_error.addAppender(_append_attendant_error);
		_logger_attendant_error.setLogLevel(::log4cplus::ERROR_LOG_LEVEL);
		_append_attendant_error->setLayout(_layout_err);

		if (change_device_id)
			free(change_device_id);
		change_device_id = nullptr;
	}
#endif
}

void sirius::library::log::log4cplus::logger::test_log(const char * fmt, ...)
{
#if 0
	char log[max_message_size] = { "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" };
	int index = strlen(log);
	char * rlog = &log[index];

	va_list args;
	va_start(args, fmt);
	vsnprintf_s(rlog, sizeof(log) - index, sizeof(log) - index, fmt, args);
	va_end(args);

	index = strlen(log);
	if (index > (sizeof(log) - 1))
	{
		log[max_message_size - 1] = 0;
		log[max_message_size - 2] = '\n';
		log[max_message_size - 3] = '\r';
	}
	else
	{
		log[index] = '\r';
		log[index + 1] = '\n';
		log[index + 2] = 0;
	}
	::OutputDebugStringA(log);
#endif
}
