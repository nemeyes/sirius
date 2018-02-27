#include <commands_arbitrator.h>
#include "arbitrator_proxy.h"
#include "configuration_dao.h"
#include "attendant_dao.h"
#include <sirius_log4cplus_logger.h>
#include <sirius_locks.h>
#include <process.h>
#include <tlhelp32.h>
#include "sirius_version.h"

sirius::app::server::arbitrator::proxy::core::core(const char * uuid, sirius::app::server::arbitrator::proxy * front, bool use_keepliave, bool use_tls)
	: sirius::library::net::sicp::server(uuid, MTU_SIZE, MTU_SIZE, MTU_SIZE, MTU_SIZE, IO_THREAD_POOL_COUNT, COMMAND_THREAD_POOL_COUNT, use_keepliave?TRUE:FALSE, use_tls?TRUE:FALSE)
	, _front(front)
	, _monitor(nullptr)
	, _run(false)
	, _thread(INVALID_HANDLE_VALUE)
	, _system_monitor_run(false)
	, _system_monitor_thread(INVALID_HANDLE_VALUE)
	, _use_count(NULL)
	, _max_attendant_instance_count(0)
	, _last_alloc_session_id(-1)
{
	::InitializeCriticalSection(&_attendant_cs);
	::InitializeCriticalSection(&_closed_attendant_cs);
	sirius::library::log::log4cplus::logger::create("configuration\\sirius_log_configuration.ini", SAA, "");
	LOGGER::make_info_log(SAA, "%s, ======================= ", __FUNCTION__);
	LOGGER::make_info_log(SAA, "%s, %d Sirius Start", __FUNCTION__, __LINE__);
	LOGGER::make_info_log(SAA, "%s, ======================= ", __FUNCTION__);

	_monitor = new sirius::library::misc::performance::monitor();

	add_command(new sirius::app::server::arbitrator::connect_client_req(_front));
	add_command(new sirius::app::server::arbitrator::disconnect_client_req(_front));
	add_command(new sirius::app::server::arbitrator::connect_attendant_req(_front));
	add_command(new sirius::app::server::arbitrator::disconnect_attendant_res(_front));
	add_command(new sirius::app::server::arbitrator::start_attendant_res(_front));
	add_command(new sirius::app::server::arbitrator::stop_attendant_res(_front));
	//add_command(new sirius::app::server::arbitrator::keepalivecheck_res(_front));
	
	add_command(CMD_ATTENDANT_INFO_IND);
	add_command(CMD_END2END_DATA_IND);
	add_command(CMD_ERROR_IND);

	add_command(CMD_KEY_DOWN_IND);
	add_command(CMD_KEY_UP_IND);
	add_command(CMD_MOUSE_LBD_IND);
	add_command(CMD_MOUSE_LBU_IND);
	add_command(CMD_MOUSE_RBD_IND);
	add_command(CMD_MOUSE_RBU_IND);
	add_command(CMD_MOUSE_MOVE_IND);
	add_command(CMD_MOUSE_LB_DCLICK_IND);
	add_command(CMD_MOUSE_RB_DCLICK_IND);
	add_command(CMD_MOUSE_WHEEL_IND);
	add_command(CMD_KEEPALIVE_REQUEST);
}

sirius::app::server::arbitrator::proxy::core::~core(void)
{
	if (_monitor)
	{
		delete _monitor;
		_monitor = nullptr;
	}

	sirius::library::log::log4cplus::logger::destroy();

	::DeleteCriticalSection(&_attendant_cs);
	::DeleteCriticalSection(&_closed_attendant_cs);	
}

int32_t sirius::app::server::arbitrator::proxy::core::initialize(sirius::app::server::arbitrator::proxy::context_t * context)
{
	if (!context)
		return sirius::app::server::arbitrator::proxy::err_code_t::fail;
	_context = context;

	int32_t status = sirius::app::server::arbitrator::proxy::err_code_t::fail;

	status = _monitor->initialize();
	if (status != sirius::app::server::arbitrator::proxy::err_code_t::success)
		return status;

	sirius::app::server::arbitrator::entity::configuration_t confentity;
	{
		sirius::app::server::arbitrator::db::configuration_dao confdao(context->db_path);
		status = confdao.retrieve(&confentity);
	}

	if (_context && _context->handler)
	{	
		_context->handler->on_initialize(confentity.uuid, confentity.url, confentity.max_attendant_instance, confentity.attendant_creation_delay, confentity.controller_portnumber, confentity.streamer_portnumber, confentity.video_codec, confentity.video_width, confentity.video_height, confentity.video_fps, confentity.video_block_width, confentity.video_block_height, confentity.video_compression_level, confentity.video_quantization_colors, confentity.enable_tls, confentity.enable_keepalive, confentity.enable_present, confentity.enable_auto_start, confentity.enable_caching, _monitor->cpu_info(), _monitor->mem_info(), confentity.log_level, confentity.idle_time, confentity.log_root_path, confentity.app_session_app);
		unsigned int thrdaddr;
		_system_monitor_run = true;
		_system_monitor_thread = (HANDLE)::_beginthreadex(NULL, 0, sirius::app::server::arbitrator::proxy::core::system_monitor_process_cb, this, 0, &thrdaddr);

	}
	return sirius::app::server::arbitrator::proxy::err_code_t::success;
}

int32_t sirius::app::server::arbitrator::proxy::core::release(void)
{
	if (_context && _context->handler)
	{
		if (_system_monitor_thread != NULL && _system_monitor_thread != INVALID_HANDLE_VALUE)
		{
			_system_monitor_run = false;
			if (::WaitForSingleObject(_system_monitor_thread, INFINITE) == WAIT_OBJECT_0)
			{
				::CloseHandle(_system_monitor_thread);
				_system_monitor_thread = INVALID_HANDLE_VALUE;
			}
		}

		_context->handler->on_release();
	}

	_monitor->release();
	return sirius::app::server::arbitrator::proxy::err_code_t::success;
}

int32_t sirius::app::server::arbitrator::proxy::core::start(void)
{
	uint32_t thrdaddr;
	_run = true;
	_cluster = new sirius::library::net::backend::cluster();
	_cluster->backend_init(GEN_VER_VERSION_STRING);
	_thread = (HANDLE)::_beginthreadex(NULL, 0, sirius::app::server::arbitrator::proxy::core::process_cb, this, 0, &thrdaddr);
	if(_thread)
		return sirius::app::server::arbitrator::proxy::err_code_t::success;
	else
		return sirius::app::server::arbitrator::proxy::err_code_t::fail;
}

int32_t sirius::app::server::arbitrator::proxy::core::stop(void)
{
	_run = false;
	if (_thread != NULL && _thread != INVALID_HANDLE_VALUE)
	{
		if (::WaitForSingleObject(_thread, INFINITE) == WAIT_OBJECT_0)
		{
			::CloseHandle(_thread);
			_thread = INVALID_HANDLE_VALUE;
		}
	}
	::Sleep(20);
	_cluster->backend_stop();
	if (_cluster)
	{
		delete _cluster;
		_cluster = nullptr;
	}
	return sirius::app::server::arbitrator::proxy::err_code_t::success;
}

int32_t sirius::app::server::arbitrator::proxy::core::update(const char * uuid, const char * url, int32_t max_attendant_instance, int32_t attendant_creation_delay, int32_t controller_portnumber, int32_t streamer_portnumber, int32_t video_codec, int32_t video_width, int32_t video_height, int32_t video_fps, int32_t video_block_width, int32_t video_block_height, int32_t video_compression_level, int32_t video_quantization_colors, bool enable_tls, bool enable_keepalive, bool enable_present, bool enable_auto_start, bool enable_caching, int32_t log_level, int32_t idle_time, const char * log_root_path, const char * app_session_app)
{
	int32_t status = sirius::app::server::arbitrator::proxy::err_code_t::fail;

	sirius::app::server::arbitrator::entity::configuration_t configuration;
	sirius::app::server::arbitrator::db::configuration_dao dao(_context->db_path);

	strncpy_s(configuration.uuid, uuid, sizeof(configuration.uuid) - 1);
	strncpy_s(configuration.url, url, sizeof(configuration.url) - 1);
	configuration.max_attendant_instance = max_attendant_instance;
	configuration.attendant_creation_delay = attendant_creation_delay;
	configuration.controller_portnumber = controller_portnumber;
	configuration.streamer_portnumber = streamer_portnumber;
	configuration.video_codec = video_codec;
	configuration.video_width = video_width;
	configuration.video_height = video_height;
	configuration.video_fps = video_fps;
	configuration.video_block_width = video_block_width;
	configuration.video_block_height = video_block_height;
	configuration.video_compression_level = video_compression_level;
	configuration.video_quantization_colors = video_quantization_colors;
	configuration.enable_tls = enable_tls;
	configuration.enable_keepalive = enable_keepalive;
	configuration.enable_present = enable_present;
	configuration.enable_auto_start = enable_auto_start;
	configuration.enable_caching = enable_caching;
	configuration.log_level = log_level;
	configuration.idle_time = idle_time;
	strncpy_s(configuration.log_root_path, log_root_path, sizeof(configuration.log_root_path) - 1);
	strncpy_s(configuration.app_session_app, app_session_app, sizeof(configuration.app_session_app) - 1);	

	status = dao.update(&configuration);
	return status;
}

int32_t	sirius::app::server::arbitrator::proxy::core::connect_client(const char * uuid, const char * client_id)
{	
	sirius::autolock lock(&_attendant_cs);

	LOGGER::make_info_log(SAA, "%s, %d, client_uuid=%s", __FUNCTION__, __LINE__, uuid);
	
	int32_t status = sirius::app::server::arbitrator::proxy::err_code_t::fail;
	bool alloc_session = false;
	std::string attendant_uuid;	

	if (_sessions.size() == 1)
		_last_alloc_session_id = -1;
				
	for (int32_t id = _last_alloc_session_id + 1; id < _sessions.size(); id++)
	{
		sirius::app::server::arbitrator::session * session = _sessions.find(id)->second;

		if (!session) continue;

		if (session->state() == sirius::app::server::arbitrator::proxy::core::attendant_state_t::available)
		{
			alloc_session = true;
			_last_alloc_session_id = id;

			session->client_id(client_id);
			session->client_uuid(uuid);
			session->state(sirius::app::server::arbitrator::proxy::core::attendant_state_t::starting);

			attendant_uuid = session->attendant_uuid();
			status = sirius::app::server::arbitrator::proxy::err_code_t::success;
			
			_use_count = _sessions.size() - get_available_attendant_count();
			_cluster->backend_client_connect((char*)session->client_id(), _use_count, session->id());		
			break;
		}
	}
	
	if (!alloc_session)
	{
		for (int32_t id = 0; id < _last_alloc_session_id; id++)
		{
			sirius::app::server::arbitrator::session * session = _sessions.find(id)->second;

			if (!session) continue;

			if (session->state() == sirius::app::server::arbitrator::proxy::core::attendant_state_t::available)
			{
				alloc_session = true;
				_last_alloc_session_id = id;

				session->client_id(client_id);
				session->client_uuid(uuid);
				session->state(sirius::app::server::arbitrator::proxy::core::attendant_state_t::starting);

				attendant_uuid = session->attendant_uuid();
				status = sirius::app::server::arbitrator::proxy::err_code_t::success;			

				_use_count = _sessions.size() - get_available_attendant_count();
				_cluster->backend_client_connect((char*)session->client_id(), _use_count, session->id());
				
				break;
			}
		}
	}	

	if (alloc_session)
	{
		Json::Value wpacket;
		Json::StyledWriter writer;
		wpacket["client_uuid"] = uuid;
		wpacket["client_id"] = client_id;
		std::string request = writer.write(wpacket);
		if (request.size() > 0)
		{
			LOGGER::make_info_log(SAA, "%s, %d, [CMD_START_ATTENDANT_REQ] attendant_uuid=%s, request_msg=%s", __FUNCTION__, __LINE__, attendant_uuid.c_str(), (char*)request.c_str());
			data_request((char*)attendant_uuid.c_str(), CMD_START_ATTENDANT_REQ, (char*)request.c_str(), request.size() + 1);
		}
	}
	else
	{
		return sirius::app::server::arbitrator::proxy::err_code_t::attendant_full;
	}
	return status;
}

int32_t sirius::app::server::arbitrator::proxy::core::disconnect_client(const char * uuid)
{
	return sirius::app::server::arbitrator::proxy::err_code_t::success;
}

int32_t sirius::app::server::arbitrator::proxy::core::get_available_attendant_count()
{
	int32_t available_count = 0;
	std::map<int32_t, sirius::app::server::arbitrator::session * >::iterator iter;
	{
		for (iter = _sessions.begin(); iter != _sessions.end(); iter++)
		{
			sirius::app::server::arbitrator::session * session = iter->second;
			if (session->state() == sirius::app::server::arbitrator::proxy::core::attendant_state_t::available)
			{
				available_count++;								
			}
		}
	}
	return available_count;
}

int32_t	sirius::app::server::arbitrator::proxy::core::connect_attendant_callback(const char * uuid, int32_t id, int32_t pid)
{
	sirius::autolock lock(&_attendant_cs);
		
	sirius::app::server::arbitrator::session * session = nullptr;
	
	std::map<int32_t, sirius::app::server::arbitrator::session *>::iterator iter;
	iter = _sessions.find(id);
	if (iter != _sessions.end())
		session = iter->second;
		
	if (session)
	{
		session->pid(pid);
		session->attendant_uuid(uuid);
		session->state(attendant_state_t::available);
	}
	else
	{
		sirius::app::server::arbitrator::session * new_session(new sirius::app::server::arbitrator::session(id));
		new_session->pid(pid);
		new_session->attendant_uuid(uuid);
		new_session->state(attendant_state_t::available);
		_sessions.insert(std::make_pair(id, new_session));
	}	
	return sirius::app::server::arbitrator::proxy::err_code_t::success;
}

void sirius::app::server::arbitrator::proxy::core::disconnect_attendant_callback(const char * uuid)
{

}

void sirius::app::server::arbitrator::proxy::core::start_attendant_callback(const char * uuid, int32_t id, const char * client_id, const char * client_uuid, int32_t code)
{
	LOGGER::make_info_log(SAA, "%s, %d, attendant_uuid=%s", __FUNCTION__, __LINE__, uuid);
	
	sirius::app::server::arbitrator::session * session = nullptr;
	{
		std::map<int32_t, sirius::app::server::arbitrator::session *>::iterator iter;
		iter = _sessions.find(id);
		if (iter != _sessions.end())
			session = iter->second;

		if (session)
			session->state(sirius::app::server::arbitrator::proxy::core::attendant_state_t::running);
	}
}

void sirius::app::server::arbitrator::proxy::core::stop_attendant_callback(const char * uuid, int32_t code)
{
	LOGGER::make_info_log(SAA, "%s, %d, [CMD_STOP_ATTENDANT_RES] attendant_uuid=%s,code=%d", __FUNCTION__, __LINE__, uuid, code);
#ifdef WITH_RESTART
#else
	sirius::app::server::arbitrator::session * session = nullptr;
	std::map<int32_t, sirius::app::server::arbitrator::session * >::iterator iter;
	{
		sirius::autolock lock(&_attendant_cs);
		for (iter = _sessions.begin(); iter != _sessions.end(); iter++)
		{
			session = iter->second;
			if (strcmp(session->attendant_uuid(), uuid) == 0)
			{
				session->state(sirius::app::server::arbitrator::proxy::core::attendant_state_t::available);
				break;
			}
		}
	}
#endif
}

void sirius::app::server::arbitrator::proxy::core::retrieve_db_path(char * path)
{
	HINSTANCE module_handle = ::GetModuleHandleA("sirius_arbitrator_proxy.dll");
	char module_path[MAX_PATH] = { 0 };
	char * module_name = module_path;
	module_name += GetModuleFileNameA(module_handle, module_name, (sizeof(module_path) / sizeof(*module_path)) - (module_name - module_path));
	if (module_name != module_path)
	{
		CHAR * slash = strrchr(module_path, '\\');
		if (slash != NULL)
		{
			module_name = slash + 1;
			_strset_s(module_name, strlen(module_name) + 1, 0);
		}
		else
		{
			_strset_s(module_path, strlen(module_path) + 1, 0);
		}
	}

	_snprintf_s(path, MAX_PATH, MAX_PATH, "%sdb\\sirius.db", module_path);
}

void sirius::app::server::arbitrator::proxy::core::on_create_session(const char * uuid)
{
	LOGGER::make_info_log(SAA, "%s, %d, uuid=%s", __FUNCTION__, __LINE__, uuid);
}

void sirius::app::server::arbitrator::proxy::core::on_destroy_session(const char * uuid)
{
	sirius::autolock lock(&_attendant_cs);

	LOGGER::make_info_log(SAA, "%s, %d, uuid=%s", __FUNCTION__, __LINE__, uuid);
	
	sirius::app::server::arbitrator::process::controller proc;
	sirius::app::server::arbitrator::session * session = nullptr;
	std::map<int32_t, sirius::app::server::arbitrator::session * >::iterator iter;
	{
		for (iter = _sessions.begin(); iter != _sessions.end(); iter++)
		{
			session = iter->second;
			if (strcmp(session->client_uuid(), uuid) == 0)
			{				
				session->state(sirius::app::server::arbitrator::proxy::core::attendant_state_t::stopping);

				_use_count = _max_attendant_instance_count - get_available_attendant_count();
				_cluster->backend_client_disconnect((char*)session->client_id(), _use_count, session->id());
#ifdef WITH_RESTART
				data_request((char*)session->attendant_uuid(), CMD_DESTROY_SESSION_INDICATION, NULL, 0);				
#else
				Json::Value wpacket;
				Json::StyledWriter writer;
				wpacket["client_uuid"] = uuid;
				std::string request = writer.write(wpacket);
				if (request.size() > 0)
					data_request((char*)session->attendant_uuid(), CMD_STOP_ATTENDANT_REQ, (char*)request.c_str(), request.size() + 1);
#endif
				break;
			}

			if (strcmp(session->attendant_uuid(), uuid) == 0)
			{
				create_attendant(session->id());
				break;
			}
		}
	}
}

int32_t sirius::app::server::arbitrator::proxy::core::get_attendant_count(void)
{
	DWORD dwCount = 0;
	HANDLE hSnap = NULL;
	PROCESSENTRY32 proc32;

	if ((hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)) == INVALID_HANDLE_VALUE)
		return -1;

	proc32.dwSize = sizeof(PROCESSENTRY32);
	while ((Process32Next(hSnap, &proc32)) == TRUE)
		if (_wcsicmp(proc32.szExeFile, L"sirius_web_attendant.exe") == 0)
			++dwCount;

	CloseHandle(hSnap);
	return dwCount;
}

int32_t sirius::app::server::arbitrator::proxy::core::get_launcher_count(void)
{
	DWORD dwCount = 0;
	HANDLE hSnap = NULL;
	PROCESSENTRY32 proc32;

	if ((hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)) == INVALID_HANDLE_VALUE)
		return -1;

	proc32.dwSize = sizeof(PROCESSENTRY32);
	while ((Process32Next(hSnap, &proc32)) == TRUE)
		if (_wcsicmp(proc32.szExeFile, L"sirius_arbitrator_launcher.exe") == 0)
			++dwCount;

	CloseHandle(hSnap);
	return dwCount;
}

void sirius::app::server::arbitrator::proxy::core::check_alive_attendant(void)
{	
	sirius::autolock lock(&_attendant_cs);

	sirius::app::server::arbitrator::process::controller proc;
	std::map<int32_t, sirius::app::server::arbitrator::session *>::iterator iter;
	for (iter = _sessions.begin(); iter != _sessions.end(); iter++)
	{
		sirius::app::server::arbitrator::session * session = iter->second;
		if(is_valid(session->attendant_uuid()) == false)
		{
			sirius::autolock lock(&_closed_attendant_cs);
		}			
	}	
}

void sirius::app::server::arbitrator::proxy::core::close_disconnected_attendant(void)
{	
	sirius::autolock lock(&_attendant_cs);

	sirius::app::server::arbitrator::process::controller proc;
	
	DWORD dwCount = 0;
	HANDLE hSnap = NULL;
	PROCESSENTRY32 proc32;

	if ((hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)) == INVALID_HANDLE_VALUE)
		return;

	proc32.dwSize = sizeof(PROCESSENTRY32);
	while ((Process32Next(hSnap, &proc32)) == TRUE)
	{
		if (_wcsicmp(proc32.szExeFile, L"sirius_web_attendant.exe") == 0)
		{
			bool is_vaild =false;
			std::map<int32_t, sirius::app::server::arbitrator::session *>::iterator iter;
			for (iter = _sessions.begin(); iter != _sessions.end(); iter++)
			{
				sirius::app::server::arbitrator::session * session = iter->second;
				if (session->pid() == proc32.th32ProcessID || session->pid() == proc32.th32ParentProcessID)
				{
					is_vaild = true;
					break;
				}						
			}

			if (is_vaild == false)				
				proc.kill(proc32.th32ProcessID);				
		}			
	}
	CloseHandle(hSnap);	
}

void sirius::app::server::arbitrator::proxy::core::update_available_attendant(void)
{
	std::map<int32_t, sirius::app::server::arbitrator::session *>::iterator iter;
	{
		for (iter = _sessions.begin(); iter != _sessions.end(); iter++)
		{
			sirius::app::server::arbitrator::session * session = iter->second;
			if (session->state() > sirius::app::server::arbitrator::proxy::core::attendant_state_t::available && 
				is_valid(session->client_uuid()) == false &&
				is_valid(session->attendant_uuid() ) == true)
			{
				session->state(sirius::app::server::arbitrator::proxy::core::attendant_state_t::available);
			}
		}
	}
}

void sirius::app::server::arbitrator::proxy::core::create_attendant(int32_t id)
{
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	memset(&si, 0x00, sizeof(si));
	memset(&pi, 0x00, sizeof(pi));
	si.cb = sizeof(STARTUPINFO);

	char module_path[MAX_PATH] = { 0 };
	char * module_name = module_path;
	module_name += GetModuleFileNameA(NULL, module_name, (sizeof(module_path) / sizeof(*module_path)) - (module_name - module_path));
	if (module_name != module_path)
	{
		CHAR * slash = strrchr(module_path, '\\');
		if (slash != NULL)
		{
			module_name = slash + 1;
			_strset_s(module_name, strlen(module_name) + 1, 0);
		}
		else
		{
			_strset_s(module_path, strlen(module_path) + 1, 0);
		}
	}
	char command_line[MAX_PATH] = { 0 };
	sprintf_s(command_line, "sirius_arbitrator_launcher.exe --id=%d", id);

	BOOL result = ::CreateProcessA(NULL, command_line, NULL, NULL, FALSE, CREATE_NEW_PROCESS_GROUP, NULL, module_path, &si, &pi);
	if (result)
	{
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
}


unsigned sirius::app::server::arbitrator::proxy::core::process_cb(void * param)
{
	sirius::app::server::arbitrator::proxy::core * self = static_cast<sirius::app::server::arbitrator::proxy::core*>(param);
	self->process();
	return 0;
}

void sirius::app::server::arbitrator::proxy::core::process(void)
{	
	_max_attendant_instance_count = 0;
	if (_context && _context->handler)
	{
		unsigned long pid = 0;
		int32_t status = sirius::app::server::arbitrator::proxy::err_code_t::fail;

		STARTUPINFOA si;
		PROCESS_INFORMATION pi;
		memset(&si, 0x00, sizeof(si));
		memset(&pi, 0x00, sizeof(pi));
		si.cb = sizeof(STARTUPINFO);

		char module_path[MAX_PATH] = { 0 };
		char * module_name = module_path;
		module_name += GetModuleFileNameA(NULL, module_name, (sizeof(module_path) / sizeof(*module_path)) - (module_name - module_path));
		if (module_name != module_path)
		{
			CHAR * slash = strrchr(module_path, '\\');
			if (slash != NULL)
			{
				module_name = slash + 1;
				_strset_s(module_name, strlen(module_name) + 1, 0);
			}
			else
			{
				_strset_s(module_path, strlen(module_path) + 1, 0);
			}
		}

		sirius::app::server::arbitrator::db::attendant_dao dao(_context->db_path);
		dao.remove();

		sirius::app::server::arbitrator::entity::configuration_t confentity;
		{
			sirius::app::server::arbitrator::db::configuration_dao confdao(_context->db_path);
			status = confdao.retrieve(&confentity);
		}

		if (status == sirius::app::server::arbitrator::proxy::err_code_t::success)
		{
			_max_attendant_instance_count = confentity.max_attendant_instance;
			sirius::library::net::sicp::server::start(nullptr, confentity.controller_portnumber);
			if (_context && _context->handler)
				_context->handler->on_start();		
		
			BOOL result = ::CreateProcessA(NULL, "sirius_arbitrator_launcher.exe", NULL, NULL, FALSE, CREATE_NEW_PROCESS_GROUP, NULL, module_path, &si, &pi);
			if (result /*&& ::WaitForSingleObject(pi.hProcess, INFINITE) == WAIT_OBJECT_0*/)
			{
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
			}
			else
			{
				_run = false;
			}
		}
		else
		{
			_run = false;
		}
	}
	else
	{
		_run = false;
	}

	bool attendant_created = false;

	const unsigned long msleep = 100;
	const unsigned long onesec = 1000;
	long long elapsed_millisec = 0;

	while (_run)
	{		
		if (!attendant_created)
		{
			if (_max_attendant_instance_count > 0)
			{
				int32_t count = (get_attendant_count()) >> 1;
				int32_t percent = ((float)count / (float)_max_attendant_instance_count) * 100;

				if (percent < 100)
				{
					if (_context && _context->handler)
						_context->handler->on_attendant_create(percent);			
				}
				else
				{
					int32_t count = get_launcher_count();
					if (count == 0)
					{
						if (_context && _context->handler)
							_context->handler->on_attendant_create(100);

						attendant_created = true;
						_cluster->ssm_service_info("START", _max_attendant_instance_count);
					}
				}
			}
			else
			{
				_context->handler->on_attendant_create(100);
				attendant_created = true;
			}
		}
		else
		{		
			//if (elapsed_millisec % (onesec * 3) == 0)			
			//	check_alive_attendant();

			//if (elapsed_millisec % (onesec * 10) == 0)
			//	update_available_attendant();				

			//if (elapsed_millisec % (onesec * 30) == 0 && elapsed_millisec > 0)
			//	close_disconnected_attendant();
		}	
		::Sleep(msleep);
		elapsed_millisec += msleep;
	}

	std::map<int32_t, sirius::app::server::arbitrator::session * >::iterator iter;
	for (iter = _sessions.begin(); iter != _sessions.end(); iter++)
	{
		sirius::app::server::arbitrator::session * session = iter->second;
		if (session)
			delete session;
	}
	_sessions.clear();
			
	_cluster->ssm_service_info("STOP", _max_attendant_instance_count);
	if (_context && _context->handler)
		_context->handler->on_stop();
	sirius::library::net::sicp::server::stop();

	sirius::app::server::arbitrator::db::attendant_dao contdao(_context->db_path);
	contdao.remove();
}

unsigned sirius::app::server::arbitrator::proxy::core::system_monitor_process_cb(void * param)
{
	sirius::app::server::arbitrator::proxy::core * self = static_cast<sirius::app::server::arbitrator::proxy::core*>(param);
	self->system_monitor_process();
	return 0;
}

void sirius::app::server::arbitrator::proxy::core::system_monitor_process(void)
{
	while (_system_monitor_run)
	{
		if (_context && _context->handler)
		{
			double cpu_usage = _monitor->total_cpu_usage();
			double memory_usage = _monitor->total_mem_usage();
			if (_context && _context->handler)
			{
				_context->handler->on_system_monitor_info(cpu_usage, memory_usage);
			}
		}
		::Sleep(1000);
	}
}