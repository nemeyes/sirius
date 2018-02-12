#include <tchar.h>
#include "attendant_proxy.h"
#include <commands_attendant.h>
#include <sirius_stringhelper.h>
#include <json/json.h>
#include <sirius_dinput_receiver.h>
#include <sirius_log4cplus_logger.h>
#include <sirius_xml_parser.h>

typedef sirius::library::framework::server::base * (*fpn_create_server_framework)();
typedef void(*fpn_destory_server_framework)(sirius::library::framework::server::base ** server_framework);

#define IO_THREAD_POOL_COUNT		2
#define COMMAND_THREAD_POOL_COUNT	2
#define MTU_SIZE					1500

sirius::app::attendant::proxy::core::core(sirius::app::attendant::proxy * front, const char * uuid, bool keepalive, bool tls)
	: sirius::library::net::sicp::client(uuid, MTU_SIZE, MTU_SIZE, MTU_SIZE, MTU_SIZE, IO_THREAD_POOL_COUNT, COMMAND_THREAD_POOL_COUNT, keepalive?TRUE:FALSE, tls?TRUE:FALSE)
	, _front(front)
	, _framework_context(NULL)
	, _framework(NULL)
	, _hmodule(NULL)
	, _callback(nullptr)
	, _key_event_count(NULL)
	, _alloc(FALSE)
{
	if (uuid && strlen(uuid) > 0)
		strcpy_s(_client_uuid, uuid);
	else
		memset(_client_uuid, 0x00, sizeof(_client_uuid));

	add_command(new sirius::app::attendant::connect_attendant_res(front));
	add_command(new sirius::app::attendant::disconnect_attendant_req(front));
	add_command(new sirius::app::attendant::start_attendant_req(front));
	add_command(new sirius::app::attendant::stop_attendant_req(front));

	add_command(new sirius::app::attendant::keyup_noti(front));
	add_command(new sirius::app::attendant::keydown_noti(front));

	add_command(new sirius::app::attendant::mouse_move_noti(front));
	add_command(new sirius::app::attendant::mouse_lbd_noti(front));
	add_command(new sirius::app::attendant::mouse_lbu_noti(front));
	add_command(new sirius::app::attendant::mouse_rbd_noti(front));
	add_command(new sirius::app::attendant::mouse_rbu_noti(front));
	add_command(new sirius::app::attendant::mouse_lb_dclick_noti(front));
	add_command(new sirius::app::attendant::mouse_rb_dclick_noti(front));
	add_command(new sirius::app::attendant::mouse_wheel_noti(front));
	add_command(new sirius::app::attendant::end2end_data_noti(front));
}

sirius::app::attendant::proxy::core::~core(void)
{
	release();
}

int32_t sirius::app::attendant::proxy::core::initialize(sirius::app::attendant::proxy::context_t * context)
{
	OutputDebugString(_T("sirius::app::attendant::proxy::core Initialize. enter"));
	_context = context;
	switch (_context->type)
	{
		case sirius::app::attendant::proxy::attendant_type_t::web :
		{
			if (!_framework_context && !_framework)
			{
				HINSTANCE module_handle = ::GetModuleHandleA("sirius_attendant_proxy.dll");
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
				if (strlen(module_path)>0)
				{
					SetDllDirectoryA(module_path);
					_hmodule = ::LoadLibraryA("sirius_web_server_framework.dll");
					if (_hmodule)
					{
						fpn_create_server_framework pfn_create = (fpn_create_server_framework)::GetProcAddress(_hmodule, "create_server_framework");
						if (pfn_create)
						{
							_framework = (pfn_create)();
							if (!_framework)
							{
								FreeLibrary(_hmodule);
								_hmodule = NULL;
							}
						}
						else
						{
							FreeLibrary(_hmodule);
							_hmodule = NULL;
						}
					}
				}

				_framework_context = new (std::nothrow) sirius::library::framework::server::base::context_t();

				/* TODO
				if (_framework && strlen(_client_uuid) > 0)
					_framework->set_notification_callee(this);
				*/
				_front->_initialized = true;
			}
			break;
		}
	}
	_pid = ::GetCurrentProcessId();
	return sirius::app::attendant::proxy::err_code_t::success;
}

int32_t sirius::app::attendant::proxy::core::release(void)
{
	if (_hmodule)
	{
		fpn_destory_server_framework pfn_destroy = (fpn_destory_server_framework)::GetProcAddress(_hmodule, "destroy_server_framework");
		if (_framework)
		{
			(pfn_destroy)(&_framework);
			_framework = nullptr;
		}
		FreeLibrary(_hmodule);
		_hmodule = NULL;
	}

	if (_framework_context)
	{
		delete _framework_context;
		_framework_context = nullptr;
	}

	_front->_initialized = FALSE;
	return sirius::app::attendant::proxy::err_code_t::success;
}

int32_t sirius::app::attendant::proxy::core::connect(void)
{
	int32_t status = sirius::app::attendant::proxy::err_code_t::fail;
	if (!_context)
		return status;

	wcsncpy_s(_framework_context->url, _context->url, sizeof(_framework_context->url));
	_framework_context->video_codec = _context->video_codec;
	_framework_context->video_width = _context->video_width;
	_framework_context->video_height = _context->video_height;
	_framework_context->video_fps = _context->video_fps;
	_framework_context->video_nbuffer = _context->video_nbuffer;
	_framework_context->video_process_type = _context->video_process_type;
	_framework_context->video_block_width = _context->video_block_width;
	_framework_context->video_block_height = _context->video_block_height;
	_framework_context->video_compression_level = _context->video_compression_level;
	_framework_context->video_qauntization_colors = _context->video_quantization_colors;

	_framework_context->present = _context->present;
	_framework_context->keepalive = _context->keepalive;
	_framework_context->tls = _context->tls;
	_framework_context->hwnd = _context->hwnd;
	_framework_context->type = _context->type;

	_framework_context->user_data = _context->user_data;
	wcsncpy_s(_framework_context->uuid, _context->uuid, sizeof(_framework_context->uuid));
	_framework_context->portnumber = _context->streamer_portnumber + _context->id;

	status = _framework->initialize(_framework_context);
	if (status != sirius::app::attendant::proxy::err_code_t::success)
	{
		return status;
	}

	status = sirius::library::net::sicp::client::connect("127.0.0.1", _context->controller_portnumber, _context->reconnect ? true : false);

	return status;
}

int32_t sirius::app::attendant::proxy::core::disconnect(void)
{
	int32_t status = sirius::app::attendant::proxy::err_code_t::fail;
	status = _framework->release();
	if (status != sirius::app::attendant::proxy::err_code_t::success)
	{
		return status;
	}

	status = sirius::library::net::sicp::client::disconnect();
	return status;
}

int32_t sirius::app::attendant::proxy::core::play(void)
{
	int32_t status = sirius::app::attendant::proxy::err_code_t::fail;

	if (!_framework_context || !_framework)
		return status;

	if (!_context->play_after_connect)
	{
		wcsncpy_s(_framework_context->url, _context->url, sizeof(_framework_context->url));
		_framework_context->video_codec = _context->video_codec;
		_framework_context->video_width = _context->video_width;
		_framework_context->video_height = _context->video_height;
		_framework_context->video_fps = _context->video_fps;
		_framework_context->video_nbuffer = _context->video_nbuffer;
		_framework_context->video_process_type = _context->video_process_type;
		_framework_context->video_block_width = _context->video_block_width;
		_framework_context->video_block_height = _context->video_block_height;
		_framework_context->video_compression_level = _context->video_compression_level;
		_framework_context->video_qauntization_colors = _context->video_quantization_colors;

		_framework_context->present = _context->present;
		_framework_context->keepalive = _context->keepalive;
		_framework_context->tls = _context->tls;
		_framework_context->hwnd = _context->hwnd;
		_framework_context->type = _context->type;

		_framework_context->user_data = _context->user_data;
		wcsncpy_s(_framework_context->uuid, _context->uuid, sizeof(_framework_context->uuid));
		_framework_context->portnumber = _context->streamer_portnumber + _context->id;

		status = _framework->initialize(_framework_context);
		if (status != sirius::app::attendant::proxy::err_code_t::success)
		{
			return status;
		}
	}

	status = _framework->play();
	if (status != sirius::app::attendant::proxy::err_code_t::success)
	{
		return status;
	}
	return status;
}

int32_t sirius::app::attendant::proxy::core::stop(void)
{
	int32_t status = sirius::app::attendant::proxy::err_code_t::fail;
	if (!_framework)
	{
		return status;
	}

	status = _framework->stop();
	if (status != sirius::app::attendant::proxy::err_code_t::success)
		return status;

	if (!_context->play_after_connect)
	{
		status = _framework->release();
		if (status != sirius::app::attendant::proxy::err_code_t::success)
			return status;
	}

	return status;
}

void sirius::app::attendant::proxy::core::on_create_session(void)
{
	Json::Value wpacket;
	Json::StyledWriter writer;
	size_t noti_size = 0;
	wpacket["id"] = _context->id;

	char * uuid = nullptr;
	sirius::stringhelper::convert_wide2multibyte(_context->uuid, &uuid);
	
	if (uuid && strlen(uuid) > 0)
	{
		wpacket["pid"] = _pid;
		wpacket["uuid"] = uuid;
		std::string request = writer.write(wpacket);

		if (request.size() > 0)
		{
			data_request(SERVER_UUID, CMD_CONNECT_ATTENDANT_REQ, (char*)request.c_str(), request.size() + 1);
		}

		free(uuid);
		uuid = nullptr;
	}
}

// ipc client 모듈 자체가 접속해지시(disconnect), 서버의 접속해제 확인 요청을 받지 않기 때문에 해당 함수는 타지 않음
void sirius::app::attendant::proxy::core::on_destroy_session(void)
{
	if (_context->hwnd)
		::SendMessage(_context->hwnd, WM_CLOSE, NULL, NULL);
}

void sirius::app::attendant::proxy::core::connect_attendant_callback(int32_t code)
{
	if (code == sirius::app::attendant::proxy::err_code_t::success)
	{
		play();
	}
}

void sirius::app::attendant::proxy::core::disconnect_attendant_callback(void)
{
	int32_t status = sirius::app::attendant::proxy::err_code_t::success;

	Json::Value wpacket;
	Json::StreamWriterBuilder wbuilder;
	wpacket["rcode"] = status;
	std::string request = Json::writeString(wbuilder, wpacket);
	if (request.size() > 0)
	{
		data_request(SERVER_UUID, CMD_DISCONNECT_ATTENDANT_RES, (char*)request.c_str(), request.size() + 1);
	}
	stop();
	disconnect();
}

void sirius::app::attendant::proxy::core::start_attendant_callback(const char * client_uuid, const char * client_id)
{
	sirius::library::log::log4cplus::logger::streamer_log_init(client_id, SLNS);
			
	Json::Value wpacket;
	Json::StyledWriter writer;		
	wpacket["id"] = _context->id;
	wpacket["client_id"] = client_id;
	wpacket["client_uuid"] = client_uuid;	
	if (!_alloc)
		wpacket["rcode"] = sirius::app::attendant::proxy::err_code_t::success;
	else
		wpacket["rcode"] = sirius::app::attendant::proxy::err_code_t::fail;
	std::string response = writer.write(wpacket);
	if (response.size() > 0)
	{
		data_request((char*)SERVER_UUID, CMD_START_ATTENDANT_RES, (char*)response.c_str(), response.size() + 1);
		LOGGER::make_info_log(SLNS, "[CMD_START_ATTENDANT_RES] - %s(), %d,	Command:%d, id:%d, rcode:%d", __FUNCTION__, __LINE__, CMD_START_ATTENDANT_RES, _context->id, sirius::app::attendant::proxy::err_code_t::success);
	}	

	if (_alloc)
		return;
	
	memcpy(&_client_uuid, client_uuid, strlen(client_uuid));
	Json::Value npacket;
	Json::StyledWriter nbuilder;

	char * uuid = nullptr;
	sirius::stringhelper::convert_wide2multibyte(_context->uuid, &uuid);

	if (uuid && strlen(uuid) > 0)
	{
		_alloc = true;

		npacket["attendant_uuid"] = uuid;
		npacket["streamer_portnumber"] = _context->streamer_portnumber + _context->id;
		npacket["video_width"] = _context->video_width;
		npacket["video_height"] = _context->video_height;
		npacket["rcode"] = sirius::app::attendant::proxy::err_code_t::success;
		std::string noti = writer.write(npacket);
		if (noti.size() > 0)
		{
			data_request((char*)client_uuid, CMD_ATTENDANT_INFO_IND, (char*)noti.c_str(), noti.size() + 1);
			LOGGER::make_info_log(SLNS, "[attendant info notification] - %s(), %d,	Command:%d, attendant_uuid:%s, client_uuid:%s, streamer_portnumber:%d, video_width:%d, video_height;%d, rcode:%d", __FUNCTION__, __LINE__, CMD_ATTENDANT_INFO_IND, uuid, client_uuid, _context->streamer_portnumber + _context->id, _context->video_width, _context->video_height,  sirius::app::attendant::proxy::err_code_t::success);
		}
		free(uuid);
		uuid = nullptr;
	}
}

void sirius::app::attendant::proxy::core::stop_attendant_callback(const char * client_uuid)
{
	_alloc = false;

	Json::Value wpacket;
	Json::StyledWriter writer;
	wpacket["rcode"] = sirius::app::attendant::proxy::err_code_t::success;
	std::string response = writer.write(wpacket);
	if (response.size() > 0)
	{
		data_request((char*)SERVER_UUID, CMD_STOP_ATTENDANT_RES, (char*)response.c_str(), response.size() + 1);
	}
}

void sirius::app::attendant::proxy::core::destroy_callback(void)
{	

}

void sirius::app::attendant::proxy::core::key_up_callback(int8_t type, int32_t key)
{
	if (type == sirius::library::user::event::dinput::receiver::type_t::keyboard)
	{
		on_key_board_up(key);
	}
	else
	{

	}
}

void sirius::app::attendant::proxy::core::key_down_callback(int8_t type, int32_t key)
{
	if (type == sirius::library::user::event::dinput::receiver::type_t::keyboard)
	{
		on_key_board_down(key);
		if (_key_event_count == NULL)
		{
			_key_event_count++;
		}
	}
	else
	{

	}
}

void sirius::app::attendant::proxy::core::mouse_move_callback(int32_t pos_x, int32_t pos_y)
{
	if (!_framework)
		return;

	_framework->on_mouse_move(pos_x, pos_y);
}

void sirius::app::attendant::proxy::core::mouse_lbd_callback(int32_t pos_x, int32_t pos_y)
{
	if (!_framework)
		return;

	_framework->on_L_mouse_down(pos_x, pos_y);
}

void sirius::app::attendant::proxy::core::mouse_lbu_callback(int32_t pos_x, int32_t pos_y)
{
	if (!_framework)
		return;

	_framework->on_L_mouse_up(pos_x, pos_y);
}

void sirius::app::attendant::proxy::core::mouse_rbd_callback(int32_t pos_x, int32_t pos_y)
{
	if (!_framework)
		return;

	_framework->on_R_mouse_down(pos_x, pos_y);
}

void sirius::app::attendant::proxy::core::mouse_rbu_callback(int32_t pos_x, int32_t pos_y)
{
	if (!_framework)
		return;

	_framework->on_R_mouse_up(pos_x, pos_y);
}

void sirius::app::attendant::proxy::core::mouse_lb_dclick_callback(int32_t pos_x, int32_t pos_y)
{
	if (!_framework)
		return;

	_framework->on_L_mouse_dclick(pos_x, pos_y);
}

void sirius::app::attendant::proxy::core::mouse_rb_dclick_callback(int32_t pos_x, int32_t pos_y)
{
	if (!_framework)
		return;

	_framework->on_R_mouse_dclick(pos_x, pos_y);
}

void sirius::app::attendant::proxy::core::mouse_wheel_callback(int32_t pos_x, int32_t pos_y, int32_t wheel_z)
{
	if (!_framework)
		return;

	_framework->on_mouse_wheel(pos_x, pos_y, wheel_z);
}

void sirius::app::attendant::proxy::core::on_key_board_up(int32_t key)
{
	if (!_framework)
		return;

	_framework->on_keyup(key);
}

void sirius::app::attendant::proxy::core::on_key_board_down(int32_t key)
{
	if (!_framework)
		return;

	_framework->on_keydown(key);
}

void sirius::app::attendant::proxy::core::app_to_attendant(uint8_t * packet, int32_t len)
{
	/*Json::Value jsonpacket;
	Json::StyledWriter writer;
	int32_t size = 0;
	std::string xml_str = std::string((char *)packet);
	jsonpacket["xml"] = xml_str.c_str();
	std::string json_str = writer.write(jsonpacket);*/
	data_request(_client_uuid, CMD_END2END_DATA_IND, (char*)packet, len);
	LOGGER::make_info_log(SLNS, "%s, %d app_to_attendant data=%s", __FUNCTION__, __LINE__, packet);
}

void sirius::app::attendant::proxy::core::attendant_to_app_callback(uint8_t * packet, int32_t len)
{
	sirius::library::log::log4cplus::logger::make_info_log(SLNS, "%s, %d packet=%s, len=%d", __FUNCTION__, __LINE__, packet, len);
	if (_callback)
	{
		_callback(packet, len);
		sirius::library::log::log4cplus::logger::make_info_log(SLNS, "%s, %d", __FUNCTION__, __LINE__);
	}

	if (strcmp((const char *)packet, ATTENDANT_RELOAD) != 0)
	{
		sirius::library::log::log4cplus::logger::make_info_log(SLNS, "%s, %d ", __FUNCTION__, __LINE__);
		_framework->on_end2end_data(packet, len);
	}
}

void sirius::app::attendant::proxy::core::on_recv_notification(int32_t type, char * msg, int32_t size)
{
	if (_context->play_after_connect == false)
		return;

	int32_t cmd;
	switch (type)
	{
	case sirius::library::misc::notification::internal::notifier::type_t::presentation_end:
		cmd = CMD_PLAYBACK_END_IND;
		//data_request(_client_uuid, cmd, msg, size);
		break;
	case sirius::library::misc::notification::internal::notifier::type_t::total_time:
		cmd = CMD_PLAYBACK_TOTALTIME_IND;
		//data_request(_client_uuid, cmd, msg, size);
		break;
	case sirius::library::misc::notification::internal::notifier::type_t::current_time:
		cmd = CMD_PLAYBACK_CURRENTTIME_IND;
		//data_request(_client_uuid, cmd, msg, size);
		break;
	case sirius::library::misc::notification::internal::notifier::type_t::current_rate:
		cmd = CMD_PLAYBACK_CURRENTRATE_IND;
		//data_request(_client_uuid, cmd, msg, size);
		break;
	case sirius::library::misc::notification::internal::notifier::type_t::gyro_enabled_attitude:
		//data_request(_client_uuid, CMD_GYRO_ENABLED_ATTITUDE, msg, size);
		break;
	case sirius::library::misc::notification::internal::notifier::type_t::gyro_enabled_gravity:
		//data_request(_client_uuid, CMD_GYRO_ENABLED_GRAVITY, msg, size);
		break;
	case sirius::library::misc::notification::internal::notifier::type_t::gyro_enabled_rotation_rate:
		//data_request(_client_uuid, CMD_GYRO_ENABLED_ROTATION_RATE, msg, size);
		break;
	case sirius::library::misc::notification::internal::notifier::type_t::gyro_enabled_rotation_rate_unbiased:
		//data_request(_client_uuid, CMD_GYRO_ENABLED_ROTATION_RATE_UNBIASED, msg, size);
		break;
	case sirius::library::misc::notification::internal::notifier::type_t::gyro_enabled_user_acceleration:
		//data_request(_client_uuid, CMD_GYRO_ENABLED_USER_ACCELERATION, msg, size);
		break;
	case sirius::library::misc::notification::internal::notifier::type_t::gyro_interval:
		//data_request(_client_uuid, CMD_GYRO_UPDATEINTERVAL, msg, size);
		break;
	case sirius::library::misc::notification::internal::notifier::type_t::end2end_data:
		//data_request(_client_uuid, CMD_CLIENT_INFO_XML_IND, msg, size);
		break;
	case sirius::library::misc::notification::internal::notifier::type_t::error :
		//data_request(_client_uuid, CMD_ERROR_IND, msg, size);
		HWND hwnd = (HWND)_context->user_data;
		::PostMessage(hwnd, WM_CLOSE, 0, 0);
		break;
	}
}
