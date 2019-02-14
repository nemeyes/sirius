#include <commands_arbitrator.h>
#include "arbitrator_proxy.h"
#include "configuration_dao.h"
//#include "attendant_dao.h"
#include <sirius_log4cplus_logger.h>
#include <sirius_locks.h>
#include <process.h>
#include <tlhelp32.h>
#include <wincrypt.h>
#include "sirius_version.h"

sirius::app::server::arbitrator::proxy::core::core(const char * uuid, sirius::app::server::arbitrator::proxy * front, bool use_keepliave, int32_t keepalive_timeout, bool use_tls)
	: sirius::library::net::sicp::server(uuid, MTU_SIZE, MTU_SIZE, MTU_SIZE, MTU_SIZE, IO_THREAD_POOL_COUNT, COMMAND_THREAD_POOL_COUNT, use_keepliave?TRUE:FALSE, keepalive_timeout, use_tls?TRUE:FALSE)
	, _front(front)
	, _monitor(nullptr)
	, _run(false)
	, _thread(INVALID_HANDLE_VALUE)
	, _system_monitor_run(false)
	, _system_monitor_thread(INVALID_HANDLE_VALUE)
	, _use_count(NULL)	
	, _last_alloc_session_id(-1)
	, _state(sirius::app::server::arbitrator::proxy::core::arbitrator_state_t::stop)
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
		_context->handler->on_initialize(confentity.uuid, confentity.url, confentity.max_attendant_instance, confentity.attendant_creation_delay, confentity.min_attendant_restart_threshold, confentity.max_attendant_restart_threshold, confentity.controller_portnumber, confentity.streamer_portnumber,
			confentity.video_codec, confentity.video_width, confentity.video_height, confentity.video_fps, confentity.video_buffer_count, confentity.video_block_width, confentity.video_block_height, 
			confentity.png.video_compression_level, confentity.png.video_quantization_posterization, confentity.png.video_quantization_dither_map, confentity.png.video_quantization_contrast_maps, confentity.png.video_quantization_colors, 
			confentity.webp.video_quality, confentity.webp.video_method, 
			confentity.invalidate4client, confentity.indexed_mode, confentity.nthread, 
			confentity.double_reloading_on_creating, confentity.reloading_on_disconnecting,
			confentity.enable_tls, confentity.enable_keepalive, confentity.keepalive_timeout, confentity.enable_streamer_keepalive, confentity.streamer_keepalive_timeout, confentity.enable_present, confentity.enable_auto_start, confentity.enable_caching, confentity.clean_attendant, _monitor->cpu_info(), _monitor->mem_info(), confentity.app_session_app, confentity.caching_directory, confentity.caching_expire_time);
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
	sirius::library::log::log4cplus::logger::file_save();
	if (_cluster)
	{
		delete _cluster;
		_cluster = nullptr;
	}
	return sirius::app::server::arbitrator::proxy::err_code_t::success;
}

int32_t sirius::app::server::arbitrator::proxy::core::update(const char * uuid, const char * url, int32_t max_attendant_instance, int32_t attendant_creation_delay, int32_t min_attendant_restart_threshold, int32_t max_attendant_restart_threshold, int32_t controller_portnumber, int32_t streamer_portnumber,
	int32_t video_codec, int32_t video_width, int32_t video_height, int32_t video_fps, int32_t video_buffer_count, int32_t video_block_width, int32_t video_block_height, 
	int32_t video_png_compression_level, bool video_png_quantization_posterization, bool video_png_quantization_dither_map, bool video_png_quantization_contrast_maps, int32_t video_png_quantization_colors, 
	float video_webp_quality, int32_t video_webp_method, 
	bool invalidate4client, bool indexed_mode, int32_t nthread, 
	bool double_reloading_on_creating, bool reloading_on_disconnecting,
	bool enable_tls, bool enable_keepalive, int32_t keepalive_timeout, bool enable_streamer_keepalive, int32_t streamer_keepalive_timeout, bool enable_present, bool enable_auto_start, bool enable_caching, bool clean_attendant, const char * app_session_app, const char * caching_directory, int32_t caching_expire_time)
{
	int32_t status = sirius::app::server::arbitrator::proxy::err_code_t::fail;

	sirius::app::server::arbitrator::entity::configuration_t configuration;
	sirius::app::server::arbitrator::db::configuration_dao dao(_context->db_path);

	strncpy_s(configuration.uuid, uuid, sizeof(configuration.uuid) - 1);
	strncpy_s(configuration.url, url, sizeof(configuration.url) - 1);
	configuration.max_attendant_instance = max_attendant_instance;
	configuration.attendant_creation_delay = attendant_creation_delay;
	configuration.min_attendant_restart_threshold = min_attendant_restart_threshold;
	configuration.max_attendant_restart_threshold = max_attendant_restart_threshold;
	configuration.controller_portnumber = controller_portnumber;
	configuration.streamer_portnumber = streamer_portnumber;
	configuration.video_codec = video_codec;
	configuration.video_width = video_width;
	configuration.video_height = video_height;
	configuration.video_fps = video_fps;
	configuration.video_buffer_count = video_buffer_count;
	configuration.video_block_width = video_block_width;
	configuration.video_block_height = video_block_height;

	configuration.png.video_compression_level = video_png_compression_level;
	configuration.png.video_quantization_posterization = video_png_quantization_posterization;
	configuration.png.video_quantization_dither_map = video_png_quantization_dither_map;
	configuration.png.video_quantization_contrast_maps = video_png_quantization_contrast_maps;
	configuration.png.video_quantization_colors = video_png_quantization_colors;
	
	configuration.webp.video_quality = video_webp_quality;
	configuration.webp.video_method = video_webp_method;

	configuration.invalidate4client = invalidate4client;
	configuration.indexed_mode = indexed_mode;
	configuration.nthread = nthread;

	configuration.double_reloading_on_creating = double_reloading_on_creating;
	configuration.reloading_on_disconnecting = reloading_on_disconnecting;

	configuration.enable_tls = enable_tls;
	configuration.enable_keepalive = enable_keepalive;
	configuration.keepalive_timeout = keepalive_timeout;
	configuration.enable_streamer_keepalive = enable_streamer_keepalive;
	configuration.streamer_keepalive_timeout = streamer_keepalive_timeout;
	configuration.enable_present = enable_present;
	configuration.enable_auto_start = enable_auto_start;
	configuration.enable_caching = enable_caching;
	configuration.clean_attendant = clean_attendant;
	strncpy_s(configuration.app_session_app, app_session_app, sizeof(configuration.app_session_app) - 1);
	strncpy_s(configuration.caching_directory, caching_directory, sizeof(configuration.caching_directory) - 1);
	configuration.caching_expire_time = caching_expire_time;

	status = dao.update(&configuration);
	return status;
}

int32_t	sirius::app::server::arbitrator::proxy::core::connect_client(const char * uuid, const char * client_id)
{	
	sirius::autolock lock(&_attendant_cs);
	LOGGER::make_info_log(SAA, "%s, %d, client_uuid=%s", __FUNCTION__, __LINE__, uuid);	

	int32_t status = sirius::app::server::arbitrator::proxy::err_code_t::fail;

	if (_state != sirius::app::server::arbitrator::proxy::core::arbitrator_state_t::start || !is_valid(uuid))
		return status;

	bool alloc_session = false;
	std::string attendant_uuid;	
			
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
			session->connect_timestamp(::GetTickCount64());

			attendant_uuid = session->attendant_uuid();
			status = sirius::app::server::arbitrator::proxy::err_code_t::success;
			
			_use_count = _sessions.size() - get_available_attendant_count();
			_cluster->backend_client_connect((char*)session->client_id(), _use_count, session->id());		
			break;
		}
	}
	
	if (!alloc_session)
	{
		for (int32_t id = 0; id < _last_alloc_session_id + 1; id++)
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
				session->connect_timestamp(::GetTickCount64());

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
	sirius::app::server::arbitrator::process::controller proc;
	sirius::app::server::arbitrator::session * session = nullptr;
	LOGGER::make_info_log(SAA, "%s, %d, uuid=%s",  __FUNCTION__, __LINE__, uuid);
	std::map<int32_t, sirius::app::server::arbitrator::session *>::iterator iter;
	iter = _sessions.find(id);
	if (iter != _sessions.end())
		session = iter->second;
		
	if (session)
	{
		if (strcmp(session->attendant_uuid(), UNDEFINED_UUID) != 0)
		{
			LOGGER::make_info_log(SAA, "%s, %d, uuid=%s", __FUNCTION__, __LINE__, uuid);
			proc.kill(pid);
			return sirius::app::server::arbitrator::proxy::err_code_t::fail;
		}
		LOGGER::make_info_log(SAA, "%s, %d, session->attendant_uuid()=%s, uuid=%s", __FUNCTION__, __LINE__, session->attendant_uuid(), uuid);
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
		LOGGER::make_info_log(SAA, "%s, %d, new_session->attendant_uuid()=%s, uuid=%s", __FUNCTION__, __LINE__, new_session->attendant_uuid(), uuid);
	}	
	return sirius::app::server::arbitrator::proxy::err_code_t::success;
}

void sirius::app::server::arbitrator::proxy::core::disconnect_attendant_callback(const char * uuid)
{

}

void sirius::app::server::arbitrator::proxy::core::start_attendant_callback(const char * uuid, int32_t id, const char * client_id, const char * client_uuid, int32_t code)
{
	sirius::autolock lock(&_attendant_cs);	
	LOGGER::make_info_log(SAA, "%s, %d, [CMD_START_ATTENDANT_RES] attendant_uuid=%s, code=%d", __FUNCTION__, __LINE__, uuid, code);
	
	sirius::app::server::arbitrator::session * session = nullptr;
	{
		std::map<int32_t, sirius::app::server::arbitrator::session *>::iterator iter;
		iter = _sessions.find(id);
		if (iter != _sessions.end())
			session = iter->second;

		if (session && session->state() == sirius::app::server::arbitrator::proxy::core::attendant_state_t::starting)
			session->state(sirius::app::server::arbitrator::proxy::core::attendant_state_t::running);
	}
}

void sirius::app::server::arbitrator::proxy::core::stop_attendant_callback(const char * uuid, int32_t code)
{
	sirius::autolock lock(&_attendant_cs);
	LOGGER::make_info_log(SAA, "%s, %d, [CMD_STOP_ATTENDANT_RES] attendant_uuid=%s, code=%d", __FUNCTION__, __LINE__, uuid, code);
#ifdef WITH_RESTART
#else
	sirius::app::server::arbitrator::session * session = nullptr;
	std::map<int32_t, sirius::app::server::arbitrator::session * >::iterator iter;
	{
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

/*
int32_t sirius::app::server::arbitrator::proxy::core::register_certificate(const wchar_t * url, const wchar_t * certificate_path, const wchar_t * certificate_password)
{
	HANDLE hfile = INVALID_HANDLE_VALUE;
	HANDLE hsection = 0;
	void* pfx = 0;
	HCERTSTORE pfxStore = 0;
	HCERTSTORE myStore = 0;
	HCERTSTORE rootStore = 0;

	hfile = ::CreateFile((LPCWSTR)certificate_path, FILE_READ_DATA, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (hfile ==INVALID_HANDLE_VALUE)
	{
		goto cleanup;
	}

	hsection = CreateFileMapping(hfile, 0, PAGE_READONLY, 0, 0, 0);
	if (!hsection)
	{
		goto cleanup;
	}

	pfx = MapViewOfFile(hsection, FILE_MAP_READ, 0, 0, 0);
	if (!pfx)
	{
		goto cleanup;
	}

	CRYPT_DATA_BLOB blob;
	blob.cbData = GetFileSize(hfile, 0);
	blob.pbData = (BYTE*)pfx;
	if (!PFXIsPFXBlob(&blob))
	{
		goto cleanup;
	}

	DWORD importFlags = CRYPT_MACHINE_KEYSET;
	pfxStore = PFXImportCertStore(&blob, certificate_password, importFlags);
	if (!pfxStore)
	{
		goto cleanup;
	}

	myStore = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, 0, CERT_STORE_OPEN_EXISTING_FLAG | CERT_SYSTEM_STORE_LOCAL_MACHINE, L"Sirius");
	if (!myStore)
	{
		goto cleanup;
	}

	rootStore = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, 0, CERT_STORE_OPEN_EXISTING_FLAG | CERT_SYSTEM_STORE_LOCAL_MACHINE, L"Root");
	if (!rootStore)
	{
		goto cleanup;
	}
	unsigned long counter = 0;

	PCCERT_CONTEXT pctx = 0;
	while (0 != (pctx = CertEnumCertificatesInStore(pfxStore, pctx)))
	{
		wchar_t name[128];
		if (CertGetNameString(pctx, CERT_NAME_FRIENDLY_DISPLAY_TYPE, 0, 0, name, sizeof name / sizeof *name))
		{
			::OutputDebugString(L"Found a certificate in the PFX file\n");
		}
		else
		{
			::OutputDebugString(L"CertGetNameString");
		}
		::OutputDebugString(L"Attempting to import certificate into machine store...\n");

		if (CertAddCertificateContextToStore(counter == 0 ? rootStore : myStore, pctx, CERT_STORE_ADD_NEW, 0))
		{
			::OutputDebugString(L"Import succeeded.\n");
		}
		else
		{
			DWORD err = GetLastError();
			if (CRYPT_E_EXISTS == err)
			{
				::OutputDebugString(L"\nAn equivalent certificate already exists. Overwrite? (y/n) ");
				wchar_t yesOrNo;
				scanf_s("%lc", &yesOrNo);
				scanf_s("%lc", &yesOrNo);
				if (L'y' == yesOrNo)
				{
					if (CertAddCertificateContextToStore(counter == 0 ? rootStore : myStore, pctx, CERT_STORE_ADD_REPLACE_EXISTING, 0))
					{
						::OutputDebugString(L"Import succeeded.\n");
					}
					else
					{
						::OutputDebugString(L"CertAddCertificateContextToStore");
						goto cleanup;
					}
				}
				else
				{
					::OutputDebugString(L"Skipped.\n");
				}
			}
			else
			{
				::OutputDebugString(L"CertAddCertificateContextToStore");
				goto cleanup;
			}
		}
		++counter;
	}

cleanup:
	if (myStore)
	{
		CertCloseStore(myStore, 0);
	}
	if (pfxStore)
	{
		CertCloseStore(pfxStore, CERT_CLOSE_STORE_FORCE_FLAG);
	}
	if (pfx)
	{
		UnmapViewOfFile(pfx);
	}
	if (hsection)
	{
		CloseHandle(hsection);
	}
	if (INVALID_HANDLE_VALUE != hfile)
	{
		CloseHandle(hfile);
	}
	return 0;
}
*/

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
	
	sirius::app::server::arbitrator::session * session = nullptr;
	std::map<int32_t, sirius::app::server::arbitrator::session * >::iterator iter;
	{
		for (iter = _sessions.begin(); iter != _sessions.end(); iter++)
		{
			session = iter->second;
			if (strcmp(session->client_uuid(), uuid) == 0)
			{	
				session->state(sirius::app::server::arbitrator::proxy::core::attendant_state_t::stopping);
				session->disconnect_timestamp(::GetTickCount64());
				session->client_uuid(UNDEFINED_UUID);

				_use_count = get_running_attendant_count();
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
				LOGGER::make_info_log(SAA, "%s, %d, destroy attendant [uuid=%s, id = %d]", __FUNCTION__, __LINE__, uuid, session->id());
				session->state(sirius::app::server::arbitrator::proxy::core::attendant_state_t::idle);
				session->attendant_uuid(UNDEFINED_UUID);
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

int32_t sirius::app::server::arbitrator::proxy::core::get_running_attendant_count(void)
{
	int32_t running_count = 0;
	std::map<int32_t, sirius::app::server::arbitrator::session * >::iterator iter;
	{
		for (iter = _sessions.begin(); iter != _sessions.end(); iter++)
		{
			sirius::app::server::arbitrator::session * session = iter->second;
			if (session->state() == sirius::app::server::arbitrator::proxy::core::attendant_state_t::running)
			{
				running_count++;
			}
		}
	}
	return running_count;
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
	std::map<int32_t, sirius::app::server::arbitrator::session *>::iterator iter;
	for (iter = _sessions.begin(); iter != _sessions.end(); iter++)
	{
		sirius::app::server::arbitrator::session * session = iter->second;
		if(session->state() == sirius::app::server::arbitrator::proxy::core::attendant_state_t::stopping)
		{
			if(::GetTickCount64() - session->disconnect_timestamp() > 1000 * 10)
			{					
				proc.kill(session->pid());
				session->attendant_uuid(UNDEFINED_UUID);
				create_attendant(session->id());
			}			
		}

		if (session->state() == sirius::app::server::arbitrator::proxy::core::attendant_state_t::starting)
		{
			if (::GetTickCount64() - session->connect_timestamp() > 1000 * 10)
			{
				proc.kill(session->pid());
				session->attendant_uuid(UNDEFINED_UUID);
				create_attendant(session->id());
			}
		}
	}	
}

void sirius::app::server::arbitrator::proxy::core::check_expire_cache(char * caching_directory, int32_t caching_expire_time)
{
	__int64 WEEK =	(__int64)10000000 * 60 * 60 * 24 * 7;
	__int64 DAY =	(__int64)10000000 * 60 * 60 * 24;
	__int64 HOUR =	(__int64)10000000 * 60 * 60;
	__int64 MIN =	(__int64)10000000 * 60;
	__int64 SEC =	(__int64)10000000;
	
	ULARGE_INTEGER current_time;
	SYSTEMTIME stm;
	FILETIME ftm;
	GetSystemTime(&stm);
	SystemTimeToFileTime(&stm, &ftm);

	memcpy(&current_time, &ftm, sizeof(FILETIME));
	current_time.QuadPart -= HOUR * caching_expire_time;
	memcpy(&ftm, &current_time, sizeof(FILETIME));
		
	HANDLE dir;
	WIN32_FIND_DATAA file_data;		
	char dir_files[MAX_PATH] = { 0 };
	sprintf_s(dir_files, "%s\\%s", caching_directory,"*.png");
	if ((dir = FindFirstFileA(dir_files, &file_data)) == INVALID_HANDLE_VALUE)
		return; /* No files found */
	do 
	{
		if (CompareFileTime(&ftm, &file_data.ftLastAccessTime) == 1)
		{			
			char filepath[MAX_PATH] = { 0 };
			sprintf_s(filepath, "%s\\%s", caching_directory, file_data.cFileName);
			DeleteFileA(filepath);				
		}
	} while (FindNextFileA(dir, &file_data));
	FindClose(dir);
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
	sirius::app::server::arbitrator::entity::configuration_t confentity;
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
/*
		sirius::app::server::arbitrator::db::attendant_dao dao(_context->db_path);
		dao.remove();
*/
			
		sirius::app::server::arbitrator::db::configuration_dao confdao(_context->db_path);
		status = confdao.retrieve(&confentity);
		
		if (status == sirius::app::server::arbitrator::proxy::err_code_t::success)
		{		
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
	_state = sirius::app::server::arbitrator::proxy::core::arbitrator_state_t::starting;

	const unsigned long msleep = 100;
	const unsigned long onesec = 1000;
	long long elapsed_millisec = 0;

	while (_run)
	{		
		if (!attendant_created)
		{
			if (confentity.max_attendant_instance > 0)
			{
				int32_t count = get_available_attendant_count();
				int32_t percent = ((float)count / (float)confentity.max_attendant_instance) * 100;

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
			if (_state == sirius::app::server::arbitrator::proxy::core::arbitrator_state_t::starting)
			{
				double cpu_usage = _monitor->total_cpu_usage();
				LOGGER::make_info_log(SAA, "%s, %d, [WAIT_FOR_SSM_SERVICE_START] cpu usage = %0.2f", __FUNCTION__, __LINE__, cpu_usage);
				if (cpu_usage < 80)
				{
					_cluster->ssm_service_info("START", confentity.max_attendant_instance);
					_state = sirius::app::server::arbitrator::proxy::core::arbitrator_state_t::start;
					LOGGER::make_info_log(SAA, "%s, %d, [SSM_SERVICE_START]", __FUNCTION__, __LINE__);
					_cluster->alive_start();
				}
			}
			if (_state == sirius::app::server::arbitrator::proxy::core::arbitrator_state_t::start)
			{
				if (elapsed_millisec % (onesec * 10) == 0)			
					check_alive_attendant();

				if (elapsed_millisec % (onesec * 60) == 0)
					check_expire_cache(confentity.caching_directory ,confentity.caching_expire_time);

				//if (elapsed_millisec % (onesec * 10) == 0 && elapsed_millisec > 0)
				//	close_disconnected_attendant();
			}
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
			
	_cluster->ssm_service_info("STOP", confentity.max_attendant_instance);
	_state = sirius::app::server::arbitrator::proxy::core::arbitrator_state_t::stop;
	if (_context && _context->handler)
		_context->handler->on_stop();
	sirius::library::net::sicp::server::stop();

/*
	sirius::app::server::arbitrator::db::attendant_dao contdao(_context->db_path);
	contdao.remove();
*/
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