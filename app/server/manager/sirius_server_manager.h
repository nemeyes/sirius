#pragma once

#ifndef STRICT
#define STRICT
#endif

#include "targetver.h"
#include "resource.h"
#include <atlbase.h>
#include <atltime.h>
#include <Tlhelp32.h>
#include <atlcom.h>
#include <atlctl.h>

#define _ATL_FREE_THREADED
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	
#define ATL_NO_ASSERT_ON_DESTROY_NONEXISTENT_WINDOW
#define LOGWRITE 0

namespace sirius
{
	namespace app
	{
		namespace server
		{
			namespace manager
			{
				class manager : public CComModule
				{
				public:
					HRESULT enrollment_server(BOOL reg_type_lib, BOOL service);
					HRESULT unenrollment_server();
					void init(_ATL_OBJMAP_ENTRY* p, HINSTANCE h, UINT service_name, const GUID* lib_id = NULL);
					void service_start();
					void service_main(DWORD dw_argc, LPTSTR* lpstr_argv);
					void control_handler(DWORD code);
					void run();
					BOOL is_installation();
					BOOL installation();
					BOOL un_installation();
					LONG un_lock();
					void set_service_condition(DWORD state);

				private:
					static void WINAPI _service_main(DWORD dw_argc, LPTSTR* lpstr_argv);
					static void WINAPI _control_handler(DWORD code);

				public:
					TCHAR					        _service_name[MAX_PATH];
					SERVICE_STATUS_HANDLE	_service_status;
					SERVICE_STATUS				_status;
					DWORD							_thread_id;
					BOOL								_is_service;
				};
			};
		};
	};
};
static sirius::app::server::manager::manager _proc;
using namespace ATL;