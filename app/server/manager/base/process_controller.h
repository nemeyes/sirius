#ifndef _PROCESS_CONTRLLER_H_
#define _PROCESS_CONTRLLER_H_

namespace sirius
{
	namespace app
	{
		namespace server
		{
			namespace manager
			{
				class controller
				{
				public:
					controller(void);
					virtual ~controller(void);

					bool run(const char * path, const char* arguments, unsigned long & pid);
					bool process_kill(DWORD process_id);
					bool process_kill(const char* process_name);
					bool process_find(DWORD process_id);
					bool process_find(char *process_name);
					DWORD find_process_id(char *process_name);
					void add_cmd_option(char* srcCmd, char *addCmd, ...);
					bool safe_terminate_process(DWORD process_id, UINT exit_code);
					bool launch_app_different_session(char* path, char* arguments, int session_id);
					DWORD WINAPI terminate_app(DWORD process_id, DWORD time_out);
					static BOOL CALLBACK terminate_app_enum(HWND hwnd, LPARAM lParam);

				private:
					void retrieve_absolute_module_path(const char * fileName, char ** path);

				};
			};
		};
	};
};

#endif //_PROCESS_CONTRLLER_H_

