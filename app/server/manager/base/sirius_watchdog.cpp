#include "sirius_server_manager.h"
#include <process.h>
#include "sirius_watchdog.h"
#include "process_controller.h"
#include "sirius_log4cplus_logger.h"
#include "sirius_stringhelper.h"

sirius::app::server::manager::monitor::monitor():
	_run(false)
{
}

sirius::app::server::manager::monitor::~monitor()
{
	uninit();
}

bool sirius::app::server::manager::monitor::init()
{
	LOGGER::make_info_log(SAW, "%s(), %d, =======sirius server manager start =======", __FUNCTION__, __LINE__);
	_run = true;
	DWORD id;
	_thread = (HANDLE)_beginthreadex(NULL, 0, process_cb, (void*)this, 0, (unsigned*)&id);

	if (!_thread)
	{		
		LOGGER::make_error_log(SAW, "%s(), %d, Failed to create monitoring thread", __FUNCTION__, __LINE__);
		return true;
	}
	else
	{		
		LOGGER::make_info_log(SAW, "%s(), %d, Success to create monitoring thread. (thread_id=%d)", __FUNCTION__, __LINE__, id);
	}
	return false;
}


bool sirius::app::server::manager::monitor::uninit()
{
	_run = false;
	if (_thread != INVALID_HANDLE_VALUE)
	{
		DWORD result = ::WaitForSingleObject(_thread, 1000 * 3);
		::CloseHandle(_thread);
		_thread = INVALID_HANDLE_VALUE;
	}
	LOGGER::make_info_log(SAW, "%s(), %d, Finish the thread", __FUNCTION__, __LINE__);
	
	return true;
}

unsigned __stdcall WINAPI sirius::app::server::manager::monitor::process_cb(void* param)
{ 
	sirius::app::server::manager::monitor *core = static_cast<sirius::app::server::manager::monitor*>(param);
	core->process();

	return TRUE;
}


void sirius::app::server::manager::monitor::process()
{	
	sirius::app::server::manager::controller controller;
	DWORD id = 0;
	
	wchar_t file_name[MAX_PATH] = {0};
	GetModuleFileName(NULL, file_name, MAX_PATH);
	PathRemoveFileSpec(file_name);

	char* mb_file_name;
	sirius::stringhelper::convert_wide2multibyte(file_name, &mb_file_name);

	char sirius_path[MAX_PATH] = { 0 };
	sprintf_s(sirius_path, "%s\\sirius_arbitrator.exe", mb_file_name);

	char resize_path[MAX_PATH] = { 0 };
	sprintf_s(resize_path, "%s\\resize.exe", mb_file_name);

	char system_user_path[MAX_PATH] = { 0 };
	sprintf_s(system_user_path, "sirius_arbitrator.exe --manager");
	if (mb_file_name)
		free(mb_file_name);

	if(ProcessIdToSessionId(controller.find_process_id("explorer.exe"), &id))
		LOGGER::make_info_log(SAW, "%s(), %d, get sesion_id=%d", __FUNCTION__, __LINE__, id);
	else
		LOGGER::make_error_log(SAW, "%s(), %d, invalid session_id", __FUNCTION__, __LINE__);

	while (_run)
	{		
		if (!controller.process_find("sirius_arbitrator.exe"))
		{
			LOGGER::make_info_log(SAW, "%s(), %d, Unable to find sirius_arbitrator.exe in process list.", __FUNCTION__, __LINE__);
			if (id > 0)
			{					
				controller.process_kill("sirius_web_attendant.exe");

				LOGGER::make_info_log(SAW, "%s(), %d, process = session_id:%d, path:%s.", __FUNCTION__, __LINE__, id, sirius_path);
				controller.launch_app_different_session(resize_path, nullptr, id);
				::Sleep(1000 * 5);
				controller.launch_app_different_session(sirius_path, system_user_path, id);
			}
			else
			{					
				LOGGER::make_info_log(SAW, "%s(), %d, Retry session_id", __FUNCTION__, __LINE__);
				if(ProcessIdToSessionId(controller.find_process_id("explorer.exe"), &id))
					LOGGER::make_info_log(SAW, "%s(), %d, get session id = %d", __FUNCTION__, __LINE__, id);
				else
					LOGGER::make_error_log(SAW, "%s(), invalid session_id", __FUNCTION__);
			}
	
		}
		Sleep(1000 * 3);
	}
	_run = false;
}