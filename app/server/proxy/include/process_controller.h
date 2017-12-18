#ifndef _PROCESS_CONTROLLER_H_
#define _PROCESS_CONTROLLER_H_

#include <sirius.h>

namespace sirius
{
	namespace app
	{
		namespace server
		{
			namespace arbitrator
			{
				namespace process
				{
					class controller
						: public sirius::base
					{
					public:
						static const int32_t FIXED_COMMAND_OPTION_SIZE = 1024;
						static const int32_t TA_FAILED = 0;
						static const int32_t TA_SUCCESS_CLEAN = 1;
						static const int32_t TA_SUCCESS_KILL = 2;
						static const int32_t MAX_PROCESS_LIST_COUNT = 1024;

					public:
						controller(void);
						~controller(void);

						int32_t fork(const char * executable, const char * path, const char * arguments, unsigned long * pid);
						int32_t kill(unsigned long pid);
						int32_t kill(const char * name);
						int32_t find(unsigned long pid);
						int32_t find(const char * name);
						void	set_cmdline(const char* src_cmd, const char * add_cmd, ...);

					private:
						void retrieve_absolute_module_path(const char * fileName, char ** path);
						HWND hwnd_from_pid(unsigned long pid);
						unsigned long pid_from_hwnd(HWND hwnd);

					};
				};
			};
		};
	};
};










#endif