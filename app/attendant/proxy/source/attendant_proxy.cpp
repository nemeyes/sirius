#include <tchar.h>
#include <commands_attendant.h>
#include "attendant_proxy.h"
#include <sirius_stringhelper.h>
#include <json/json.h>
#include <sirius_dinput_receiver.h>
#include <sirius_log4cplus_logger.h>
#include <sirius_xml_parser.h>

typedef sirius::library::framework::server::base * (*fpn_create_server_framework)();
typedef void(*fpn_destory_server_framework)(sirius::library::framework::server::base ** server_framework);

#define IO_THREAD_POOL_COUNT		1
#define COMMAND_THREAD_POOL_COUNT	1
#define MTU_SIZE					1500

sirius::app::attendant::proxy::core::core(sirius::app::attendant::proxy * front, const char * uuid, const char * client_uuid)
	: sirius::library::net::sicp::client(uuid, MTU_SIZE, MTU_SIZE, MTU_SIZE, MTU_SIZE, COMMAND_THREAD_POOL_COUNT, IO_THREAD_POOL_COUNT, false, true, ethernet_type_t::tcp, false)
	, _front(front)
	, _netstate(sirius::app::attendant::proxy::netstate_t::disconnected)
	, _mediastate(sirius::app::attendant::proxy::mediastate_t::stopped)
	, _unifiedstate(sirius::app::attendant::proxy::err_code_t::fail)
	, _framework_context(NULL)
	, _framework(NULL)
	, _hmodule(NULL)
	, callback(nullptr)
	, _key_event_count(NULL)
{
	if (client_uuid && strlen(client_uuid) > 0)
		strcpy_s(_client_uuid, client_uuid);
	else
		memset(_client_uuid, 0x00, sizeof(_client_uuid));

	add_command(new sirius::app::attendant::stop_attendant_req_cmd(front));

	add_command(new sirius::app::attendant::keyup_noti_cmd(front));
	add_command(new sirius::app::attendant::keydown_noti_cmd(front));

	add_command(new sirius::app::attendant::mouse_move_noti_cmd(front));
	add_command(new sirius::app::attendant::mouse_lbd_noti_cmd(front));
	add_command(new sirius::app::attendant::mouse_lbu_noti_cmd(front));
	add_command(new sirius::app::attendant::mouse_rbd_noti_cmd(front));
	add_command(new sirius::app::attendant::mouse_rbu_noti_cmd(front));
	add_command(new sirius::app::attendant::mouse_lb_dclick_noti_cmd(front));
	add_command(new sirius::app::attendant::mouse_rb_dclick_noti_cmd(front));
	add_command(new sirius::app::attendant::mouse_wheel_noti_cmd(front));

	add_command(new sirius::app::attendant::seek_key_down_noti_cmd(front));
	add_command(new sirius::app::attendant::seek_key_up_noti_cmd(front));
	//add_command(new sirius::app::attendant::seek_pos_cmd(front));

	add_command(new sirius::app::attendant::play_toggle_cmd(front));
	add_command(new sirius::app::attendant::backward_cmd(front));
	add_command(new sirius::app::attendant::forward_cmd(front));
	add_command(new sirius::app::attendant::reverse_cmd(front));
	add_command(new sirius::app::attendant::gyro_noti_cmd(front));
	add_command(new sirius::app::attendant::pinch_zoom_noti_cmd(front));
	//add_command(new ic::slot_alive_check_res_cmd(front));
	//add_command(new ic::slot_create_slot_res_cmd(front));

	add_command(new sirius::app::attendant::infoxml_noti_cmd(front));

	add_command(new sirius::app::attendant::gyro_attitude_noti_cmd(front));
	add_command(new sirius::app::attendant::gyro_gravity_noti_cmd(front));
	add_command(new sirius::app::attendant::gyro_rotation_rate_noti_cmd(front));
	add_command(new sirius::app::attendant::gyro_rotation_rate_unbiased_noti_cmd(front));
	add_command(new sirius::app::attendant::gyro_user_acceleration_noti_cmd(front));

	add_command(new sirius::app::attendant::gyro_enabled_attitude_cmd(front));
	add_command(new sirius::app::attendant::gyro_enabled_gravity_cmd(front));
	add_command(new sirius::app::attendant::gyro_enabled_rotation_rate_cmd(front));
	add_command(new sirius::app::attendant::gyro_enabled_rotation_rate_unbiased_cmd(front));
	add_command(new sirius::app::attendant::gyro_enabled_user_acceleration_cmd(front));
	add_command(new sirius::app::attendant::gyro_updateinterval_cmd(front));
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
				if (_framework && strlen(_client_uuid) > 0)
					_framework->set_notification_callee(this);
				_front->_initialized = TRUE;
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
	
	if ((_context != nullptr) && (_unifiedstate == sirius::app::attendant::proxy::err_code_t::fail))
	{
		_unified_context.video_codec = _context->video_codec;
		_unified_context.video_width = _context->video_width;
		_unified_context.video_height = _context->video_height;
		_unified_context.video_fps = _context->video_fps;
		wcsncpy_s(_unified_context.uuid, _context->uuid, sizeof(_unified_context.uuid));
		_unified_context.portnumber = _context->streamer_portnumber;

		if(_unified_context.video_codec!= sirius::library::unified::server::video_submedia_type_t::unknown)
		{
			status = sirius::library::unified::server::instance().initialize(&_unified_context);
			if (status != sirius::app::attendant::proxy::err_code_t::success)
			{
				sirius::library::unified::server::instance().release();
				return status;
			}
			_unifiedstate = sirius::app::attendant::proxy::err_code_t::success;
		}
	}

	bool ret = sirius::library::net::sicp::client::connect("127.0.0.1", _context->controller_portnumber, _context->reconnect ? true : false);
	if (ret)
		status = sirius::app::attendant::proxy::err_code_t::success;
	else
		status = sirius::app::attendant::proxy::err_code_t::fail;

	return status;
}

int32_t sirius::app::attendant::proxy::core::disconnect(void)
{
	if(_netstate >= sirius::app::attendant::proxy::netstate_t::disconnecting)
		return sirius::app::attendant::proxy::err_code_t::success;

	_netstate = sirius::app::attendant::proxy::netstate_t::disconnecting;

	bool ret = sirius::library::net::sicp::client::disconnect();
	if (ret)
	{
		_netstate = sirius::app::attendant::proxy::netstate_t::disconnected;
		LOGGER::make_info_log(SLNS, "%s, %d, sicp client disconnect success", __FUNCTION__, __LINE__);
		return sirius::app::attendant::proxy::err_code_t::success;
	}
	else
	{
		LOGGER::make_info_log(SLNS, "%s, %d, sicp client disconnect fail", __FUNCTION__, __LINE__);
		return sirius::app::attendant::proxy::err_code_t::fail;
	}
}

int32_t sirius::app::attendant::proxy::core::play(void)
{
	int32_t code = sirius::app::attendant::proxy::err_code_t::fail;

	if (!_framework_context || !_framework)
		return code;

	if (_mediastate == sirius::app::attendant::proxy::mediastate_t::stopped)
	{
		wcsncpy_s(_framework_context->url, _context->url, sizeof(_framework_context->url));
		_framework_context->video_codec		= _context->video_codec;
		_framework_context->video_width		= _context->video_width;
		_framework_context->video_height	= _context->video_height;
		_framework_context->video_fps		= _context->video_fps;
		_framework_context->video_nbuffer	= _context->video_nbuffer;
		_framework_context->video_process_type = _context->video_process_type;
		_framework_context->video_block_width = _context->video_block_width;
		_framework_context->video_block_height = _context->video_block_height;

		_framework_context->gpuindex		= _context->gpuindex;
		_framework_context->present			= _context->present;
		_framework_context->hwnd			= _context->hwnd;
		_framework_context->type			= _context->type;

		_framework_context->user_data		= _context->user_data;
		wcsncpy_s(_framework_context->uuid, _context->uuid, sizeof(_framework_context->uuid));
		_framework_context->portnumber		= _context->controller_portnumber;

		if (_unifiedstate == sirius::app::attendant::proxy::err_code_t::fail)
		{
			wcsncpy_s(_unified_context.uuid, _context->uuid, sizeof(_unified_context.uuid));
			_unified_context.portnumber		= _context->streamer_portnumber;

			_unified_context.video_codec	= _context->video_codec;
			_unified_context.video_width	= _context->video_width;
			_unified_context.video_height	= _context->video_height;
			_unified_context.video_fps		= _context->video_fps;
			_unified_context.video_block_width = _context->video_block_width;
			_unified_context.video_block_height = _context->video_block_height;

			if (_unified_context.video_codec != sirius::library::unified::server::video_submedia_type_t::unknown)
			{
				code = sirius::library::unified::server::instance().initialize(&_unified_context);
				if (code != sirius::app::attendant::proxy::err_code_t::success)
				{
					sirius::library::unified::server::instance().release();
					return code;
				}
				_unifiedstate = sirius::app::attendant::proxy::err_code_t::success;
			}
		}
		code = _framework->open(_framework_context);
		if (code != sirius::app::attendant::proxy::err_code_t::success)
		{
			LOGGER::make_error_log(SAA, "%s(),%d, unsupported_media_file url : %S, slot_uuid : %S", __FUNCTION__, __LINE__, _framework_context->url[0], _framework_context->uuid);
			return code;
		}

		_mediastate = sirius::app::attendant::proxy::mediastate_t::playing;
	}
	return code;
}

int32_t sirius::app::attendant::proxy::core::stop(void)
{
	if (_mediastate == sirius::app::attendant::proxy::mediastate_t::paused)
		return sirius::app::attendant::proxy::err_code_t::success;

	int32_t code = sirius::app::attendant::proxy::err_code_t::fail;
	if (!_framework)
	{
		LOGGER::make_error_log(SLNS, "%s, %d, attendant_not_found", __FUNCTION__, __LINE__);
		return code;
	}

	code = _framework->stop();
	if (code != sirius::app::attendant::proxy::err_code_t::success)
		return code;

	code = _framework->close();
	if (code != sirius::app::attendant::proxy::err_code_t::success)
		return code;

	if (_unified_context.video_codec != sirius::library::unified::server::video_submedia_type_t::unknown)
	{
		code = sirius::library::unified::server::instance().release();
		if (code != sirius::app::attendant::proxy::err_code_t::success)
		{
			LOGGER::make_error_log(SLNS, "%s, %d, Release Network Sink Fail", __FUNCTION__, __LINE__);
			return sirius::app::attendant::proxy::err_code_t::fail;
		}
	}
	_mediastate = sirius::app::attendant::proxy::mediastate_t::paused;
	return code;
}

int32_t sirius::app::attendant::proxy::core::forward(void)
{
	int32_t code = sirius::app::attendant::proxy::err_code_t::fail;
	if (!_framework)
		return code;

	if (_mediastate == sirius::app::attendant::proxy::mediastate_t::playing)
		code = _framework->forward();

	return code;
}

int32_t sirius::app::attendant::proxy::core::backward(void)
{
	int32_t code = sirius::app::attendant::proxy::err_code_t::fail;
	if (!_framework)
		return code;

	if (_mediastate == sirius::app::attendant::proxy::mediastate_t::playing)
		code = _framework->backward();
	return code;
}

int32_t sirius::app::attendant::proxy::core::reverse(void)
{
	int32_t code = sirius::app::attendant::proxy::err_code_t::fail;
	if (!_framework)
		return code;

	if (_mediastate == sirius::app::attendant::proxy::mediastate_t::playing)
		code = _framework->reverse();
	return code;
}

int32_t sirius::app::attendant::proxy::core::play_toggle(void)
{
	int32_t code = sirius::app::attendant::proxy::err_code_t::fail;
	if (!_framework)
		return code;

	if (_mediastate == sirius::app::attendant::proxy::mediastate_t::playing)
		code = _framework->play_toggle();

	return code;
}

void sirius::app::attendant::proxy::core::on_destroy(void)
{	
	if (sirius::library::unified::server::instance().is_video_compressor_initialized())
	{
		stop();
		disconnect();
	}
}

void sirius::app::attendant::proxy::core::on_key_up(int8_t type, int32_t key)
{
	if (type == sirius::library::user::event::dinput::receiver::type_t::keyboard)
	{
		on_key_board_up(key);
		LOGGER::make_debug_log(SLNS, "[Key Up Noti] - %s(), %d, Command:%d, Key:%d", __FUNCTION__, __LINE__, CMD_KEY_UP_IND, key);
	}
	else
	{

	}
}

void sirius::app::attendant::proxy::core::on_key_down(int8_t type, int32_t key)
{
	if (type == sirius::library::user::event::dinput::receiver::type_t::keyboard)
	{
		on_key_board_down(key);
		if (_key_event_count == NULL)
		{
			LOGGER::make_info_log(SLNS, "[Key Down Noti] - %s(), %d, Command:%d, Key:%d", __FUNCTION__, __LINE__, CMD_KEY_DOWN_IND, key);
			_key_event_count++;
		}
		LOGGER::make_debug_log(SLNS, "[Key Down Noti] - %s(), %d, Command:%d,  Key:%d", __FUNCTION__, __LINE__, CMD_KEY_DOWN_IND, key);
	}
	else
	{

	}
}

void sirius::app::attendant::proxy::core::on_mouse_move(int32_t pos_x, int32_t pos_y)
{
	if (!_framework)
		return;

	_framework->on_mouse_move(pos_x, pos_y);
}

void sirius::app::attendant::proxy::core::on_mouse_lbd(int32_t pos_x, int32_t pos_y)
{
	if (!_framework)
		return;

	_framework->on_L_mouse_down(pos_x, pos_y);
}

void sirius::app::attendant::proxy::core::on_mouse_lbu(int32_t pos_x, int32_t pos_y)
{
	if (!_framework)
		return;

	_framework->on_L_mouse_up(pos_x, pos_y);
}

void sirius::app::attendant::proxy::core::on_mouse_rbd(int32_t pos_x, int32_t pos_y)
{
	if (!_framework)
		return;

	_framework->on_R_mouse_down(pos_x, pos_y);
}

void sirius::app::attendant::proxy::core::on_mouse_rbu(int32_t pos_x, int32_t pos_y)
{
	if (!_framework)
		return;

	_framework->on_R_mouse_up(pos_x, pos_y);
}

void sirius::app::attendant::proxy::core::on_mouse_lb_dclick(int32_t pos_x, int32_t pos_y)
{
	if (!_framework)
		return;

	_framework->on_L_mouse_dclick(pos_x, pos_y);
}

void sirius::app::attendant::proxy::core::on_mouse_rb_dclick(int32_t pos_x, int32_t pos_y)
{
	if (!_framework)
		return;

	_framework->on_R_mouse_dclick(pos_x, pos_y);
}

void sirius::app::attendant::proxy::core::on_mouse_wheel(int32_t pos_x, int32_t pos_y, int32_t wheel_z)
{
	if (!_framework)
		return;

	_framework->on_mouse_wheel(pos_x, pos_y, wheel_z);
}

void sirius::app::attendant::proxy::core::on_gyro(float x, float y, float z)
{
	if (!_framework)
		return;

	_framework->on_gyro(x, y, z);
}

void sirius::app::attendant::proxy::core::on_pinch_zoom(float delta)
{
	if (!_framework)
		return;

	_framework->on_pinch_zoom(delta);
}

void sirius::app::attendant::proxy::core::on_gyro_attitude(float x, float y, float z, float w)
{
	if (!_framework)
		return;

	_framework->on_gyro_attitude(x, y, z, w);
}

void sirius::app::attendant::proxy::core::on_gyro_gravity(float x, float y, float z)
{
	if (!_framework)
		return;

	_framework->on_gyro_gravity(x, y, z);
}

void sirius::app::attendant::proxy::core::on_gyro_rotation_rate(float x, float y, float z)
{
	if (!_framework)
		return;

	_framework->on_gyro_rotation_rate(x, y, z);
}

void sirius::app::attendant::proxy::core::on_gyro_rotation_rate_unbiased(float x, float y, float z)
{
	if (!_framework)
		return;

	_framework->on_gyro_rotation_rate_unbiased(x, y, z);
}

void sirius::app::attendant::proxy::core::on_gyro_user_acceleration(float x, float y, float z)
{
	if (!_framework)
		return;

	_framework->on_gyro_user_acceleration(x, y, z);
}

void sirius::app::attendant::proxy::core::on_gyro_enabled_attitude(bool state)
{
	if (!_framework)
		return;

	_framework->on_gyro_enabled_attitude(state);
}

void sirius::app::attendant::proxy::core::on_gyro_enabled_gravity(bool state)
{
	if (!_framework)
		return;

	_framework->on_gyro_enabled_gravity(state);
}

void sirius::app::attendant::proxy::core::on_gyro_enabled_rotation_rate(bool state)
{
	if (!_framework)
		return;

	_framework->on_gyro_enabled_rotation_rate(state);
}

void sirius::app::attendant::proxy::core::on_gyro_enabled_rotation_rate_unbiased(bool state)
{
	if (!_framework)
		return;

	_framework->on_gyro_enabled_rotation_rate_unbiased(state);
}

void sirius::app::attendant::proxy::core::on_gyro_enabled_user_acceleration(bool state)
{
	if (!_framework)
		return;

	_framework->on_gyro_enabled_user_acceleration(state);
}

void sirius::app::attendant::proxy::core::on_gyro_update_interval(float interval)
{
	if (!_framework)
		return;

	_framework->on_gyro_updateinterval(interval);
}

void sirius::app::attendant::proxy::core::on_ar_view_mat(float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23,
	float m30, float m31, float m32, float m33)
{
	if (!_framework)
		return;

	_framework->on_ar_view_mat(m00, m01, m02, m03,
		m10, m11, m12, m13,
		m20, m21, m22, m23,
		m30, m31, m32, m33);
}

void sirius::app::attendant::proxy::core::on_ar_proj_mat(float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23,
	float m30, float m31, float m32, float m33)
{
	if (!_framework)
		return;

	_framework->on_ar_proj_mat(m00, m01, m02, m03,
		m10, m11, m12, m13,
		m20, m21, m22, m23,
		m30, m31, m32, m33);
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

void sirius::app::attendant::proxy::core::create_session_callback(void)
{
	//::timeBeginPeriod(1);
	Json::Value noti_packet;
	Json::StyledWriter writer;
	size_t noti_size = 0;
	noti_packet["attendant_number"] = _context->streamer_portnumber;
	noti_packet["attendant_uuid"] = _context->uuid;
	std::string noti_string = writer.write(noti_packet);

	noti_size = noti_string.size();
	if (noti_size > 0)
	{
		char * noti_msg = nullptr;
		noti_msg = static_cast<char*>(malloc(noti_size + 1));
		memcpy((void*)(noti_msg), noti_string.c_str(), noti_size + 1);
		data_request(SERVER_UUID, CMD_START_ATTENDANT_REQ, noti_msg, noti_size);
		LOGGER::make_info_log(SLNS, "[Slot Connect Request] - %s(), %d, Command:%d, slotNum %d, slotUuid:%S", __FUNCTION__, __LINE__, CMD_START_ATTENDANT_REQ, _context->streamer_portnumber, _context->uuid);
		free(noti_msg);
		noti_msg = nullptr;
	}
	play();
	_netstate = sirius::app::attendant::proxy::netstate_t::connected;
}

// ipc client 모듈 자체가 접속해지시(disconnect), 서버의 접속해제 확인 요청을 받지 않기 때문에 해당 함수는 타지 않음
void sirius::app::attendant::proxy::core::destroy_session_callback(void)
{
	LOGGER::make_info_log(SLNS, "%s, %d, ENTER", __FUNCTION__, __LINE__);
	
	stop();
	if (_framework)
		_framework->close();

	_netstate = sirius::app::attendant::proxy::netstate_t::disconnected;
}

void __stdcall sirius::app::attendant::proxy::core::alive_timer_callback(LPVOID args, DWORD low, DWORD high)
{
	if (args == nullptr)
		return;
	sirius::app::attendant::proxy::core * self = static_cast<sirius::app::attendant::proxy::core*>(args);
}

void sirius::app::attendant::proxy::core::on_app_to_container_xml(char * packet, int len)
{
	/*Json::Value jsonpacket;
	Json::StyledWriter writer;
	int32_t size = 0;
	std::string xml_str = packet;
	jsonpacket["xml"] = xml_str.c_str();
	std::string json_str = writer.write(jsonpacket);*/
	data_request(_client_uuid, CMD_CLIENT_INFO_XML_IND, packet, len);
}

void sirius::app::attendant::proxy::core::on_container_to_app(char * packet, int len)
{
	if (callback)
		callback(packet, len);
	_framework->on_infoxml(packet, len);
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
		data_request(_client_uuid, cmd, msg, size);
		break;
	case sirius::library::misc::notification::internal::notifier::type_t::total_time:
		cmd = CMD_PLAYBACK_TOTALTIME_IND;
		data_request(_client_uuid, cmd, msg, size);
		break;
	case sirius::library::misc::notification::internal::notifier::type_t::current_time:
		cmd = CMD_PLAYBACK_CURRENTTIME_IND;
		data_request(_client_uuid, cmd, msg, size);
		break;
	case sirius::library::misc::notification::internal::notifier::type_t::current_rate:
		cmd = CMD_PLAYBACK_CURRENTRATE_IND;
		data_request(_client_uuid, cmd, msg, size);
		break;
	case sirius::library::misc::notification::internal::notifier::type_t::gyro_enabled_attitude:
		data_request(_client_uuid, CMD_GYRO_ENABLED_ATTITUDE, msg, size);
		break;
	case sirius::library::misc::notification::internal::notifier::type_t::gyro_enabled_gravity:
		data_request(_client_uuid, CMD_GYRO_ENABLED_GRAVITY, msg, size);
		break;
	case sirius::library::misc::notification::internal::notifier::type_t::gyro_enabled_rotation_rate:
		data_request(_client_uuid, CMD_GYRO_ENABLED_ROTATION_RATE, msg, size);
		break;
	case sirius::library::misc::notification::internal::notifier::type_t::gyro_enabled_rotation_rate_unbiased:
		data_request(_client_uuid, CMD_GYRO_ENABLED_ROTATION_RATE_UNBIASED, msg, size);
		break;
	case sirius::library::misc::notification::internal::notifier::type_t::gyro_enabled_user_acceleration:
		data_request(_client_uuid, CMD_GYRO_ENABLED_USER_ACCELERATION, msg, size);
		break;
	case sirius::library::misc::notification::internal::notifier::type_t::gyro_interval:
		data_request(_client_uuid, CMD_GYRO_UPDATEINTERVAL, msg, size);
		break;
	case sirius::library::misc::notification::internal::notifier::type_t::info_xml:
		data_request(_client_uuid, CMD_CLIENT_INFO_XML_IND, msg, size);
		break;
	case sirius::library::misc::notification::internal::notifier::type_t::info_json:
		data_request(_client_uuid, CMD_CLIENT_INFO_XML_IND, msg, size);
		break;
	case sirius::library::misc::notification::internal::notifier::type_t::error :
		data_request(_client_uuid, CMD_ERROR_IND, msg, size);
		HWND hwnd = (HWND)_context->user_data;
		::PostMessage(hwnd, WM_CLOSE, 0, 0);
		break;
	}
	//data_request
}
