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
{
	::InitializeCriticalSection(&_attendant_cs);
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
		_context->handler->on_initialize(confentity.uuid, confentity.url, confentity.max_attendant_instance, confentity.attendant_creation_delay, confentity.controller_portnumber, confentity.streamer_portnumber, confentity.video_codec, confentity.video_width, confentity.video_height, confentity.video_fps, confentity.video_block_width, confentity.video_block_height, confentity.video_compression_level, confentity.video_quantization_colors, confentity.enable_tls, confentity.enable_keepalive, confentity.enable_present, confentity.enable_auto_start, confentity.enable_caching, _monitor->cpu_info(), _monitor->mem_info());
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

int32_t sirius::app::server::arbitrator::proxy::core::update(const char * uuid, const char * url, int32_t max_attendant_instance, int32_t attendant_creation_delay, int32_t controller_portnumber, int32_t streamer_portnumber, int32_t video_codec, int32_t video_width, int32_t video_height, int32_t video_fps, int32_t video_block_width, int32_t video_block_height, int32_t video_compression_level, int32_t video_quantization_colors, bool enable_tls, bool enable_keepalive, bool enable_present, bool enable_auto_start, bool enable_caching)
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

	status = dao.update(&configuration);
	return status;
}

int32_t	sirius::app::server::arbitrator::proxy::core::connect_client(const char * uuid, const char * id)
{
	sirius::autolock lock(&_attendant_cs);

	int32_t status = sirius::app::server::arbitrator::proxy::err_code_t::fail;

	sirius::app::server::arbitrator::db::attendant_dao dao(_context->db_path);
	sirius::app::server::arbitrator::entity::attendant_t ** attendant = nullptr;
	int32_t count = 0;
	{	
		status = dao.retrieve(sirius::app::server::arbitrator::proxy::core::attendant_state_t::available, &attendant, count);
	}
	if (status == sirius::app::server::arbitrator::proxy::err_code_t::success && count>0)
	{
		Json::Value wpacket;
		Json::StyledWriter writer;
		wpacket["client_uuid"] = uuid;
		wpacket["client_id"] = id;
		std::string request = writer.write(wpacket);
		if (request.size() > 0)
		{
			strncpy_s(attendant[count - 1]->client_uuid, uuid, sizeof(attendant[count - 1]->client_uuid)-1);
			strncpy_s(attendant[count - 1]->client_id, id, sizeof(attendant[count - 1]->client_id) - 1);
			attendant[count - 1]->state = sirius::app::server::arbitrator::proxy::core::attendant_state_t::starting;
			{
				dao.update(attendant[count - 1]);			
				_use_count++;
				_cluster->backend_client_connect(attendant[count - 1]->client_id, _use_count, attendant[count - 1] ->id);
			}
			data_request(attendant[count - 1]->uuid, CMD_START_ATTENDANT_REQ, (char*)request.c_str(), request.size() + 1);
		}

		for (int32_t index = 0; index < count; index++)
		{
			free(attendant[index]);
			attendant[index] = nullptr;
		}
		free(attendant);
		attendant = nullptr;
	}
	if (count < 1)
		return sirius::app::server::arbitrator::proxy::err_code_t::attendant_full;
	else
		return status;
}

int32_t sirius::app::server::arbitrator::proxy::core::disconnect_client(const char * uuid)
{
	int32_t status = sirius::app::server::arbitrator::proxy::err_code_t::fail;
	sirius::app::server::arbitrator::db::attendant_dao dao(_context->db_path);
	sirius::app::server::arbitrator::entity::attendant_t attendant;
	{
		sirius::autolock lock(&_attendant_cs);
		status = dao.retrieve(uuid, &attendant);
	}
	if (status == sirius::app::server::arbitrator::proxy::err_code_t::success)
	{
		Json::Value wpacket;
		Json::StyledWriter writer;
		wpacket["client_uuid"] = uuid;
		std::string request = writer.write(wpacket);
		if (request.size() > 0)
		{
			data_request(attendant.uuid, CMD_STOP_ATTENDANT_REQ, (char*)request.c_str(), request.size() + 1);
			{
				sirius::autolock lock(&_attendant_cs);
				dao.update(sirius::app::server::arbitrator::proxy::core::attendant_state_t::stopping, sirius::app::server::arbitrator::db::attendant_dao::type_t::client, uuid);
				_use_count--;
				_cluster->backend_client_disconnect(attendant.client_id, _use_count, attendant.id);
			}
		}
	}
	return status;
}

int32_t sirius::app::server::arbitrator::proxy::core::get_available_attendant_count()
{
	sirius::app::server::arbitrator::db::attendant_dao dao(_context->db_path);	
	return dao.retrieve_count(sirius::app::server::arbitrator::proxy::core::attendant_state_t::available);
}

int32_t	sirius::app::server::arbitrator::proxy::core::connect_attendant_callback(const char * uuid, int32_t id, int32_t pid)
{
	int32_t status = sirius::app::server::arbitrator::proxy::err_code_t::success;
	sirius::app::server::arbitrator::entity::attendant_t contenity;
	memmove(contenity.uuid, uuid, strlen(uuid) + 1);
	memmove(contenity.client_uuid, UNDEFINED_UUID, strlen(UNDEFINED_UUID) + 1);
	contenity.id = id;
	contenity.pid = pid;
	memset(contenity.client_id, 0x00, sizeof(contenity.client_id));
	contenity.state = attendant_state_t::available;
	contenity.total_bandwidth_bytes = 0;
	{
		sirius::autolock lock(&_attendant_cs);
		sirius::app::server::arbitrator::db::attendant_dao dao(_context->db_path);
		dao.add(&contenity);
	}
	return status;
}

void sirius::app::server::arbitrator::proxy::core::disconnect_attendant_callback(const char * uuid)
{
	sirius::autolock lock(&_attendant_cs);
	sirius::app::server::arbitrator::db::attendant_dao dao(_context->db_path);
	dao.update(sirius::app::server::arbitrator::proxy::core::attendant_state_t::idle, sirius::app::server::arbitrator::db::attendant_dao::type_t::attendant, uuid);
}

void sirius::app::server::arbitrator::proxy::core::start_attendant_callback(const char * uuid, const char * id, int32_t code)
{
	if (code == sirius::app::server::arbitrator::proxy::err_code_t::success)
	{
		sirius::autolock lock(&_attendant_cs);
		sirius::app::server::arbitrator::db::attendant_dao dao(_context->db_path);
		dao.update(sirius::app::server::arbitrator::proxy::core::attendant_state_t::running, sirius::app::server::arbitrator::db::attendant_dao::type_t::attendant, uuid);
	}
}

void sirius::app::server::arbitrator::proxy::core::stop_attendant_callback(const char * uuid, int32_t code)
{
	if (code == sirius::app::server::arbitrator::proxy::err_code_t::success)
	{
		sirius::autolock lock(&_attendant_cs);
		sirius::app::server::arbitrator::db::attendant_dao dao(_context->db_path);
		dao.update(sirius::app::server::arbitrator::proxy::core::attendant_state_t::available, sirius::app::server::arbitrator::db::attendant_dao::type_t::attendant, uuid);
	}
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
	sirius::library::log::log4cplus::logger::make_debug_log(SAA, "create_session_callback");
}

void sirius::app::server::arbitrator::proxy::core::on_destroy_session(const char * uuid)
{
	sirius::library::log::log4cplus::logger::make_debug_log(SAA, "destroy_session_callback");

	sirius::autolock lock(&_attendant_cs);
	sirius::app::server::arbitrator::db::attendant_dao dao(_context->db_path);
	dao.update(sirius::app::server::arbitrator::proxy::core::attendant_state_t::available, sirius::app::server::arbitrator::db::attendant_dao::type_t::client, uuid);
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
	sirius::app::server::arbitrator::process::controller proc;
	int32_t status = sirius::app::server::arbitrator::proxy::err_code_t::fail;
	sirius::app::server::arbitrator::entity::configuration_t confentity;
	{
		sirius::app::server::arbitrator::db::configuration_dao confdao(_context->db_path);
		status = confdao.retrieve(&confentity);
	}

	if (status == sirius::app::server::arbitrator::proxy::err_code_t::success)
	{		
		int32_t max_attendant_instance_count = confentity.max_attendant_instance;			
		int32_t current_attendant_instance_count = (get_attendant_count()) >> 1;

		if (current_attendant_instance_count < max_attendant_instance_count && get_launcher_count() == 0)
		{
			sirius::app::server::arbitrator::db::attendant_dao dao(_context->db_path);
			sirius::app::server::arbitrator::entity::attendant_t ** attendant = nullptr;
			int32_t count = 0;
			{
				sirius::autolock lock(&_attendant_cs);
				status = dao.retrieve(&attendant, count);
			}
			if (status == sirius::app::server::arbitrator::proxy::err_code_t::success && count > 0)
			{
				if (count == max_attendant_instance_count)
				{
					for (int index = 0; index < count; index++)
					{
						if (proc.find(attendant[index]->pid) == sirius::app::server::arbitrator::process::controller::err_code_t::fail)
						{
							{
								sirius::autolock lock(&_attendant_cs);
								dao.remove(attendant[index]);
							}
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
							sprintf_s(command_line, "sirius_arbitrator_launcher.exe --uuid=%s --id=%d", attendant[index]->uuid, attendant[index]->id);

							BOOL result = ::CreateProcessA(NULL, command_line, NULL, NULL, FALSE, CREATE_NEW_PROCESS_GROUP, NULL, module_path, &si, &pi);
							if (result)
							{
								CloseHandle(pi.hProcess);
								CloseHandle(pi.hThread);
							}
							::Sleep(100);
						}
					}
				}
				else
				{						
					for(int id =0; id < max_attendant_instance_count;id++)
					{
						bool is_vaild = false;
						for (int index = 0; index < count; index++)
						{
							if (attendant[index]->id == id)
							{
								is_vaild = true;
								break;
							}
						}

						if (is_vaild == false)
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
					}									
				}
			}
			for (int32_t index = 0; index < count; index++)
			{
				free(attendant[index]);
				attendant[index] = nullptr;
			}
			free(attendant);
			attendant = nullptr;
		}
	}	
}


void sirius::app::server::arbitrator::proxy::core::close_unconnected_attendant(void)
{
	sirius::app::server::arbitrator::process::controller proc;
	int32_t status = sirius::app::server::arbitrator::proxy::err_code_t::fail;
	sirius::app::server::arbitrator::db::attendant_dao dao(_context->db_path);
	sirius::app::server::arbitrator::entity::attendant_t ** attendant = nullptr;
	int32_t count = 0;
	{
		sirius::autolock lock(&_attendant_cs);
		status = dao.retrieve(&attendant, count);
	}
	if (status == sirius::app::server::arbitrator::proxy::err_code_t::success && count > 0)
	{
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
				for (int index = 0; index < count; index++)
				{
					if (attendant[index]->pid == proc32.th32ProcessID || 
						attendant[index]->pid == proc32.th32ParentProcessID)
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
		for (int32_t index = 0; index < count; index++)
		{
			free(attendant[index]);
			attendant[index] = nullptr;
		}
		free(attendant);
		attendant = nullptr;
	}	
}

void sirius::app::server::arbitrator::proxy::core::update_available_attendant(void)
{
	int32_t status = sirius::app::server::arbitrator::proxy::err_code_t::fail;
	sirius::app::server::arbitrator::db::attendant_dao dao(_context->db_path);
	sirius::app::server::arbitrator::entity::attendant_t ** attendant = nullptr;
	int32_t count = 0;
	{
		sirius::autolock lock(&_attendant_cs);
		status = dao.retrieve(&attendant, count);
	}
	if (status == sirius::app::server::arbitrator::proxy::err_code_t::success && count > 0)
	{
		for (int32_t index = 0; index < count; index++)
		{
			if (attendant[index]->state != sirius::app::server::arbitrator::proxy::core::attendant_state_t::available
				&& is_valid(attendant[index]->client_uuid) == false)
			{
				sirius::autolock lock(&_attendant_cs);
				sirius::app::server::arbitrator::db::attendant_dao dao(_context->db_path);
				dao.update(sirius::app::server::arbitrator::proxy::core::attendant_state_t::available, sirius::app::server::arbitrator::db::attendant_dao::type_t::client, attendant[index]->client_uuid);
			}		
		}
	}
	for (int32_t index = 0; index < count; index++)
	{
		free(attendant[index]);
		attendant[index] = nullptr;
	}
	free(attendant);
	attendant = nullptr;
}

unsigned sirius::app::server::arbitrator::proxy::core::process_cb(void * param)
{
	sirius::app::server::arbitrator::proxy::core * self = static_cast<sirius::app::server::arbitrator::proxy::core*>(param);
	self->process();
	return 0;
}

void sirius::app::server::arbitrator::proxy::core::process(void)
{
	int32_t max_attendant_instance_count = 0;
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
			max_attendant_instance_count = confentity.max_attendant_instance;
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

	int32_t msleep = 0;
	while (_run)
	{		
		if (!attendant_created)
		{
			if (max_attendant_instance_count > 0)
			{
				int32_t count = (get_attendant_count()) >> 1;
				int32_t percent = ((float)count / (float)max_attendant_instance_count) * 100;

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
							_context->handler->on_attendant_create(percent);

						attendant_created = true;
						_cluster->ssm_service_info("START", max_attendant_instance_count);
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
			check_alive_attendant();		

			if (msleep > 1000 * 10)
			{		
				update_available_attendant();
				close_unconnected_attendant();
				msleep = 0;
			}
			::Sleep(500);
			msleep += 500;
		}		
	}
	_cluster->ssm_service_info("STOP", max_attendant_instance_count);
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