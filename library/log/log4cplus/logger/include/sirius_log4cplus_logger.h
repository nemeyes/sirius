#ifndef _CAP_LOG4CPLUS_LOGGER_H_
#define _CAP_LOG4CPLUS_LOGGER_H_
#include <string>

#if defined(_WIN32)
# include <windows.h>
# if defined(EXPORT_LOG4CPLUS_LOGGER_LIB)
#  define EXP_LOG4CPLUS_LOGGER_DLL __declspec(dllexport)
# else
#  define EXP_LOG4CPLUS_LOGGER_DLL __declspec(dllimport)
# endif
#else
# define EXP_LOG4CPLUS_LOGGER_DLL
#endif

/*
#define SBE		"sirius.base.error"
#define SAC		"sirius.app.coordinator"
#define SAL		"sirius.app.loadrunner"
#define SAE		"sirius.app.emulator"

#define SLNC	"sirius.library.net.controller"
#define SLNS	"sirius.library.net.streamer"
*/

typedef enum _logger_type_t 
{
	error = 0,
	arbitrator = 1,
	stressor,
	client,
	controller,
	streamer,
	streamer_create,
	video_source,
	watchdog,
	logger_type_end
}logger_type_t;
typedef bool(*fpn_file_changed)();

#if !defined(WITH_DISABLE)
namespace log4cplus
{
	class ConfigureAndWatchThread;
};
#endif

namespace sirius
{
	namespace library
	{
		namespace log
		{
			namespace log4cplus
			{
				class EXP_LOG4CPLUS_LOGGER_DLL logger
				{
				public:
					class EXP_LOG4CPLUS_LOGGER_DLL critical_section
					{
					public:
						critical_section(void);
						~critical_section(void);
						void lock(void);
						void unlock(void);
					private:
						CRITICAL_SECTION _cs;
					};

					class EXP_LOG4CPLUS_LOGGER_DLL scopped_lock
					{
					public:
						scopped_lock(critical_section * cs);
						~scopped_lock(void);
					private:
						critical_section * _cs;
					};

					static const int max_message_size = 10000;

					class EXP_LOG4CPLUS_LOGGER_DLL trace_logger
					{
					public:
						trace_logger(const char * secion, const char * fmt, ...);
						~trace_logger(void);
					private:
						char _section[MAX_PATH];
						char _log[max_message_size];
					};


					static void create(const char * configuration_path, const char * log_type, char * device_id);
					static void destroy(void);
					static bool first_log_directory_check(const char * log_type);
					static void log_directory_check(const char * secion);
					static void log_configuration(const char * secion);
					static void make_fatal_log(const char * secion, const char * fmt, ...);
					static void make_error_log(const char * secion, const char * fmt, ...);
					static void make_warn_log(const char * secion, const char * fmt, ...);
					static void make_info_log(const char * secion, const char * fmt, ...);
					static void make_debug_log(const char * secion, const char * fmt, ...);
					static void make_trace_log(const char * secion, const char * fmt, ...);
					static void test_log(const char * fmt, ...);

					static void set_section(const char * s) { strcpy_s(_default_section, s); };
					static const char * get_section() { return _default_section; };
					static char *replace_all(char *s, const char *olds, const char *news);
					static bool set_file_monitor(std::string file_path, fpn_file_changed pfn);
					static bool file_changed();
					static bool file_monitor_start();
					static bool file_monitor_stop();
					static void log_level_change(int log_level, int log_type);
					static void streamer_log_init(const char * device_id, const char * log_type);
					static const char * get_device_id() { return _device_id; };
				private:
					logger(const char * configuration_path, const char * log_type, char * device_id);
					logger(const logger & clone);
					virtual ~logger(void);
					static DWORD WINAPI monitor_Thread_proc(__inout LPVOID lpParam);

				private:
					static sirius::library::log::log4cplus::logger * _instance;
					static char _default_section[MAX_PATH];
					static char _pattern[MAX_PATH];
					static char _dir_path[MAX_PATH];
					static char _file_path[MAX_PATH];
					static int  _log_type;
					static int  _change_log_type;
					static int  _diff_log_type;
					static char	_device_id[MAX_PATH];
					static char	_log_name[MAX_PATH];
					static char	_device_log_name[MAX_PATH];
					static char _module_path[MAX_PATH];
					static TCHAR _wc_last_file_name[MAX_PATH];
					static TCHAR wc_last_error_file_name[MAX_PATH];
					static TCHAR wc_last_attendant_file_name[MAX_PATH];
					static TCHAR wc_last_attendant_error_file_name[MAX_PATH];
					static SYSTEMTIME _sys_time;
					static int	_section_log_update[logger_type_end];
					static _WIN32_FILE_ATTRIBUTE_DATA _file_att_data;
					static fpn_file_changed			  _pfn_file_changed;
					static HANDLE					  _change_handles[2];
					static HINSTANCE _self;
#if !defined(WITH_DISABLE)
					::log4cplus::ConfigureAndWatchThread * _configure_thread;
#endif
				};
			};
		};
	};
};


static sirius::library::log::log4cplus::logger::critical_section g_log4cplus_critical_section;

#define LOGGER sirius::library::log::log4cplus::logger

#define SBE		"sirius.base.error"
#define SAA		"sirius.app.arbitrator"
#define SAS		"sirius.app.stressor"
#define SAC		"sirius.app.client"
#define SAW		"sirius.app.watchdog"

#define SLNS	"sirius.library.net.streamer"
#define SLNSC		"sirius.library.net.streamer.create"

#define log_create(theProperties,default_section) \
sirius::library::log::log4cplus::logger::create(#theProperties) ; \
sirius::library::log::log4cplus::logger::set_section((const char *) #default_section) ; \

#define log_destroy() sirius::library::log::log4cplus::logger::destroy() 
#define log_streamer_init() sirius::library::log::log4cplus::logger::streamer_log_init()

#define log_fatal(_fmt_,...) sirius::library::log::log4cplus::logger::make_fatal_log(sirius::library::log::log4cplus::logger::get_section(), _fmt_,__VA_ARGS__)
#define log_error(_fmt_,...) sirius::library::log::log4cplus::logger::make_error_log(sirius::library::log::log4cplus::logger::get_section(), _fmt_,__VA_ARGS__)
#define log_warn(_fmt_,...) sirius::library::log::log4cplus::logger::make_warn_log(sirius::library::log::log4cplus::logger::get_section(), _fmt_,__VA_ARGS__)
#define log_info(_fmt_,...) sirius::library::log::log4cplus::logger::make_info_log(sirius::library::log::log4cplus::logger::get_section(), _fmt_,__VA_ARGS__)
#define log_debug(_fmt_,...) sirius::library::log::log4cplus::logger::make_debug_log(sirius::library::log::log4cplus::logger::get_section(), _fmt_,__VA_ARGS__)
#define log_trace(_fmt_,...) sirius::library::log::log4cplus::logger::make_trace_log(sirius::library::log::log4cplus::logger::get_section(), _fmt_,__VA_ARGS__)

#define log_fatal_sec(_section_,_fmt_,...) sirius::library::log::log4cplus::logger::make_error_log(#_section_, _fmt_,__VA_ARGS__)
#define log_error_sec(_section_,_fmt_,...) sirius::library::log::log4cplus::logger::make_error_log(#_section_, _fmt_,__VA_ARGS__)
#define log_warn_sec(_section_,_fmt_,...) sirius::library::log::log4cplus::logger::make_warn_log(#_section_, _fmt_,__VA_ARGS__)
#define log_info_sec(_section_,_fmt_,...) sirius::library::log::log4cplus::logger::make_error_info(#_section_, _fmt_,__VA_ARGS__)
#define log_debug_sec(_section_,_fmt_,...) sirius::library::log::log4cplus::logger::make_debug_log(#_section_, _fmt_,__VA_ARGS__)
#define log_trace_sec(_section_,_fmt_,...) sirius::library::log::log4cplus::logger::make_trace_log(#_section_, _fmt_,__VA_ARGS__)

#endif