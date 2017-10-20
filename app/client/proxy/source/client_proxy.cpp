#include <memory>
#include <string>
#include <process.h>
#include <commands_payload.h>
#include <commands_client.h>
#include "client_proxy.h"
#include "json/json.h"
#include <sirius_stringhelper.h>
#include <sirius_client_framework.h>

#define MTU_SIZE 0
#define IO_THREAD_POOL_COUNT 1
#define COMMAND_THREAD_POOL_COUNT 1

sirius::app::client::proxy::core::core(sirius::app::client::proxy * front, HINSTANCE instance, HWND hwnd)
	: sirius::library::net::sicp::client(MTU_SIZE, MTU_SIZE, MTU_SIZE, 1500, COMMAND_THREAD_POOL_COUNT, IO_THREAD_POOL_COUNT, true, true, sirius::app::client::proxy::ethernet_type_t::tcp, false)
	, _state(sirius::app::client::proxy::state_t::disconnected)
	, _front(front)
	, _instance(instance)
	, _hwnd(hwnd)
	, _dxkey_input_initialized(FALSE)
	, _reconnection(FALSE)
	, _recv_attendant_info(FALSE)
	, _streamer_portnumber(-1)
{
	add_command(new sirius::app::client::connect_attendant_res_cmd(this));
	add_command(new sirius::app::client::disconnect_attendant_res_cmd(this));
	add_command(new sirius::app::client::attendant_info_ind_cmd(this));
	add_command(new sirius::app::client::playback_end_ind_cmd(this));
	add_command(new sirius::app::client::playback_totaltime_ind_cmd(this));
	add_command(new sirius::app::client::playback_currenttime_ind_cmd(this));
	add_command(new sirius::app::client::playback_currentrate_ind_cmd(this));
	add_command(new sirius::app::client::xml_ind_cmd(this));
	add_command(new sirius::app::client::error_ind_cmd(this));

	add_command(CMD_PLAYBACK_TOTALTIME_IND);
	add_command(CMD_PLAYBACK_CURRENTTIME_IND);
	add_command(CMD_PLAYBACK_END_IND);
	add_command(CMD_PLAYBACK_CURRENTRATE_IND);

	add_command(CMD_PLAY_RES);
	add_command(CMD_VIDEO_STREAM_DATA);

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
	add_command(CMD_SEEK_KEY_DOWN);
	add_command(CMD_SEEK_KEY_UP);
	add_command(CMD_SEEK_POS);
	add_command(CMD_PLAY_TOGGLE);
	add_command(CMD_BACKWARD);
	add_command(CMD_FORWARD);
	add_command(CMD_REVERSE);

	add_command(CMD_GYRO_IND);
	add_command(CMD_PINCH_ZOOM_IND);
	//	add_command(CMD_GYRO_ROT_IND);
	add_command(CMD_GYRO_ATTITUDE);
	add_command(CMD_GYRO_GRAVITY);
	add_command(CMD_GYRO_ROTATION_RATE);
	add_command(CMD_GYRO_ROTATION_RATE_UNBIASED);
	add_command(CMD_GYRO_ACCELERATION);
	add_command(CMD_GYRO_ENABLED_ATTITUDE);
	add_command(CMD_GYRO_ENABLED_GRAVITY);
	add_command(CMD_GYRO_ENABLED_ROTATION_RATE);
	add_command(CMD_GYRO_ENABLED_ROTATION_RATE_UNBIASED);
	add_command(CMD_GYRO_ENABLED_USER_ACCELERATION);
	add_command(CMD_GYRO_UPDATEINTERVAL);


	memset(_szattendant_uuid, 0x00, sizeof(_szattendant_uuid));
}

sirius::app::client::proxy::core::~core(void)
{
	disconnect();
	release();
}

int32_t sirius::app::client::proxy::core::state(void)
{
	return _state;
}

bool sirius::app::client::proxy::core::recv_attendant_info(void)
{
	return _recv_attendant_info;
}

const wchar_t * sirius::app::client::proxy::core::attendant_uuid(void)
{
	return _szwattendant_uuid;
}

const wchar_t *	sirius::app::client::proxy::core::address(void)
{
	return _waddress;
}

const int32_t sirius::app::client::proxy::core::streamer_portnumber(void)
{
	return _streamer_portnumber;
}

int32_t sirius::app::client::proxy::core::set_using_mouse(BOOL value)
{
	set_using_mouse(value ? true : false);
	return sirius::app::client::proxy::err_code_t::success;
}

int32_t sirius::app::client::proxy::core::set_key_stroke(int32_t interval)
{
	set_keystroke(interval);
	return sirius::app::client::proxy::err_code_t::success;
}

int32_t sirius::app::client::proxy::core::connect(wchar_t * address, int32_t portnumber, BOOL reconnection)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;
	if (_front)
		_front->on_pre_connect(address, portnumber, reconnection);

	memset(_address, 0x00, sizeof(_address));
	memset(_waddress, 0x00, sizeof(_waddress));
	char * ascii_address = nullptr;
	sirius::stringhelper::convert_wide2multibyte(address, &ascii_address);
	if (ascii_address && strlen(ascii_address) > 0)
	{
		strncpy_s(_address, ascii_address, sizeof(_address));
		wcsncpy_s(_waddress, address, sizeof(_waddress));
		_reconnection = reconnection;

		_state = sirius::app::client::proxy::state_t::connecting;
		if (sirius::library::net::sicp::client::connect(_address, portnumber, _reconnection ? true : false))
			status = sirius::app::client::proxy::err_code_t::success;

		free(ascii_address);
		ascii_address = nullptr;
	}
	else
	{
		status = sirius::app::client::proxy::err_code_t::success;
	}


	if (_front)
		_front->on_post_connect(address, portnumber, reconnection);

	return status;
}

int32_t sirius::app::client::proxy::core::disconnect(void)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;
	if (_front)
		_front->on_pre_disconnect();

	_state = sirius::app::client::proxy::state_t::disconnecting;
	if (sirius::library::net::sicp::client::disconnect())
		status = sirius::app::client::proxy::err_code_t::success;

	if (_front)
		_front->on_post_disconnect();

	return status;
}

int32_t sirius::app::client::proxy::core::connect_attendant(wchar_t * appid, wchar_t * deviceid, wchar_t * devicetype, wchar_t * envtype, wchar_t * modeltype, int32_t width, int32_t height, int32_t gpuindex)
{
	char * app_id = nullptr;
	char * device_id = nullptr;
	char * device_t = nullptr;
	char * envir_t = nullptr;
	char * model = nullptr;

	sirius::stringhelper::convert_wide2multibyte((LPWSTR)(LPCWSTR)appid, &app_id);
	sirius::stringhelper::convert_wide2multibyte((LPWSTR)(LPCWSTR)deviceid, &device_id);
	sirius::stringhelper::convert_wide2multibyte((LPWSTR)(LPCWSTR)devicetype, &device_t);
	sirius::stringhelper::convert_wide2multibyte((LPWSTR)(LPCWSTR)envtype, &envir_t);
	sirius::stringhelper::convert_wide2multibyte((LPWSTR)(LPCWSTR)modeltype, &model);
	if (app_id && device_id && strlen(app_id) > 0 && strlen(device_id) > 0)
	{
		Json::Value packet;
		Json::StyledWriter writer;

		packet["appid"] = app_id;
		packet["deviceid"] = device_id;
		packet["devicetype"] = device_t;
		packet["environmenttype"] = envir_t;
		if (strcmp(model, "") != 0)
			packet["model"] = model;

		packet["width"] = width;
		packet["height"] = height;

		if (gpuindex >= 0)
			packet["gpuindex"] = gpuindex;

		std::string json = writer.write(packet);
		if (json.size() > 0)
			data_request(SERVER_UUID, CMD_CONNECT_ATTENDANT_REQ, (char*)json.c_str(), json.size() + 1);

		free(app_id);
		free(device_id);
		free(device_t);
		free(envir_t);
		free(model);
		app_id = nullptr;
		device_id = nullptr;
		device_t = nullptr;
		envir_t = nullptr;
		model = nullptr;
	}
	_recv_attendant_info = FALSE;

	return sirius::app::client::proxy::err_code_t::success;
}

int32_t sirius::app::client::proxy::core::disconnect_attendant(void)
{
	Json::Value packet;
	Json::StyledWriter writer;
	packet["message"] = "disconnect_attendant";
	std::string req_msg = writer.write(packet);
	if (req_msg.size() > 0)
		data_request(SERVER_UUID, CMD_DISCONNECT_ATTENDANT_REQ, (char*)req_msg.c_str(), req_msg.size() + 1);
	_recv_attendant_info = FALSE;

	return sirius::app::client::proxy::err_code_t::success;
}

int32_t sirius::app::client::proxy::core::seek_to(int32_t second)
{
	if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
	{
		CMD_SEEK_POS_T noti;
		memset(&noti, 0x00, sizeof(CMD_SEEK_POS_T));
		noti.second = htonl(second);
		data_request(_szattendant_uuid, CMD_SEEK_POS, (char*)&noti, sizeof(CMD_SEEK_POS_T));
	}
	return sirius::app::client::proxy::err_code_t::success;
}

int32_t sirius::app::client::proxy::core::key_up_play_toggle(void)
{
	if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
		data_request(_szattendant_uuid, CMD_PLAY_TOGGLE, NULL, 0);

	return sirius::app::client::proxy::err_code_t::success;
}

int32_t sirius::app::client::proxy::core::key_up_backward(void)
{
	if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
		data_request(_szattendant_uuid, CMD_BACKWARD, NULL, 0);

	return sirius::app::client::proxy::err_code_t::success;
}

int32_t sirius::app::client::proxy::core::key_up_forward(void)
{
	if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
		data_request(_szattendant_uuid, CMD_FORWARD, NULL, 0);

	return sirius::app::client::proxy::err_code_t::success;
}

int32_t sirius::app::client::proxy::core::key_up_reverse(void)
{
	if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
		data_request(_szattendant_uuid, CMD_REVERSE, NULL, 0);

	return sirius::app::client::proxy::err_code_t::success;
}

int32_t sirius::app::client::proxy::core::key_down_seek(int32_t diff)
{
	if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
	{
		CMD_SEEK_KEY_DOWN_T noti;
		memset(&noti, 0x00, sizeof(CMD_SEEK_KEY_DOWN_T));
		noti.diff = htonl(diff);
		data_request(_szattendant_uuid, CMD_SEEK_KEY_DOWN, (char*)&noti, sizeof(CMD_SEEK_KEY_DOWN_T));
	}
	return sirius::app::client::proxy::err_code_t::success;
}

int32_t	sirius::app::client::proxy::core::key_up_seek(void)
{
	if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
	{
		data_request(_szattendant_uuid, CMD_SEEK_KEY_UP, NULL, 0);
	}
	return sirius::app::client::proxy::err_code_t::success;
}

int32_t	sirius::app::client::proxy::core::key_up(int32_t value)
{
	if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
	{
		CMD_KEY_UP_IND_T noti;
		memset(&noti, 0x00, sizeof(CMD_KEY_UP_IND_T));
		noti.input_type = 0;
		noti.key_code = htonl(value);//ntohl(value);
		data_request(_szattendant_uuid, CMD_KEY_UP_IND, (char*)&noti, sizeof(CMD_KEY_UP_IND_T));
	}
	return sirius::app::client::proxy::err_code_t::success;
}

int32_t sirius::app::client::proxy::core::key_down(int32_t value)
{
	if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
	{
		CMD_KEY_DOWN_IND_T noti;
		memset(&noti, 0x00, sizeof(CMD_KEY_DOWN_IND_T));
		noti.input_type = 0;
		noti.key_code = htonl(value);//ntohl(value);
		data_request(_szattendant_uuid, CMD_KEY_DOWN_IND, (char*)&noti, sizeof(CMD_KEY_DOWN_IND_T));
	}
	return sirius::app::client::proxy::err_code_t::success;
}

int32_t sirius::app::client::proxy::core::mouse_move(int32_t pos_x, int32_t pos_y)
{
	if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
	{
		CMD_MOUSE_MOVE_IND_T noti;
		memset(&noti, 0x00, sizeof(CMD_MOUSE_MOVE_IND_T));
		noti.x_translation = htonl(pos_x);
		noti.y_translation = htonl(pos_y);
		data_request(_szattendant_uuid, CMD_MOUSE_MOVE_IND, (char*)&noti, sizeof(CMD_MOUSE_MOVE_IND_T));
	}
	return sirius::app::client::proxy::err_code_t::success;
}

int32_t sirius::app::client::proxy::core::mouse_wheel(int32_t pos_x, int32_t pos_y, int32_t wheel_delta)
{
	if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
	{
		CMD_MOUSE_WHEEL_IND_T noti;
		noti.z_delta = htonl(wheel_delta);
		noti.x = htonl(pos_x);
		noti.y = htonl(pos_y);
		data_request(_szattendant_uuid, CMD_MOUSE_WHEEL_IND, (char*)&noti, sizeof(CMD_MOUSE_WHEEL_IND_T));
	}
	return sirius::app::client::proxy::err_code_t::success;
}

int32_t sirius::app::client::proxy::core::mouse_lb_double(int32_t pos_x, int32_t pos_y)
{
	if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
	{
		CMD_MOUSE_LB_DCLICK_IND_T noti;
		noti.x = htonl(pos_x);
		noti.y = htonl(pos_y);
		data_request(_szattendant_uuid, CMD_MOUSE_LB_DCLICK_IND, (char*)&noti, sizeof(CMD_MOUSE_LB_DCLICK_IND_T));
	}
	return sirius::app::client::proxy::err_code_t::success;
}

int32_t sirius::app::client::proxy::core::mouse_lb_down(int32_t pos_x, int32_t pos_y)
{
	if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
	{
		CMD_MOUSE_LBD_IND_T noti;
		noti.x = htonl(pos_x);
		noti.y = htonl(pos_y);
		data_request(_szattendant_uuid, CMD_MOUSE_LBD_IND, (char*)&noti, sizeof(CMD_MOUSE_LBD_IND_T));
	}
	return sirius::app::client::proxy::err_code_t::success;
}

int32_t sirius::app::client::proxy::core::mouse_lb_up(int32_t pos_x, int32_t pos_y)
{
	if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
	{
		CMD_MOUSE_LBU_IND_T noti;
		noti.x = htonl(pos_x);
		noti.y = htonl(pos_y);
		data_request(_szattendant_uuid, CMD_MOUSE_LBU_IND, (char*)&noti, sizeof(CMD_MOUSE_LBU_IND_T));
	}
	return sirius::app::client::proxy::err_code_t::success;
}

int32_t sirius::app::client::proxy::core::mouse_rb_double(int32_t pos_x, int32_t pos_y)
{
	if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
	{
		CMD_MOUSE_RB_DCLICK_IND_T noti;
		noti.x = htonl(pos_x);
		noti.y = htonl(pos_y);
		data_request(_szattendant_uuid, CMD_MOUSE_RB_DCLICK_IND, (char*)&noti, sizeof(CMD_MOUSE_RB_DCLICK_IND_T));
	}
	return sirius::app::client::proxy::err_code_t::success;
}

int32_t sirius::app::client::proxy::core::mouse_rb_down(int32_t pos_x, int32_t pos_y)
{
	if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
	{
		CMD_MOUSE_RBD_IND_T noti;
		noti.x = htonl(pos_x);
		noti.y = htonl(pos_y);
		data_request(_szattendant_uuid, CMD_MOUSE_RBD_IND, (char*)&noti, sizeof(CMD_MOUSE_RBD_IND_T));
	}
	return sirius::app::client::proxy::err_code_t::success;
}

int32_t sirius::app::client::proxy::core::mouse_rb_up(int32_t pos_x, int32_t pos_y)
{
	if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
	{
		CMD_MOUSE_RBU_IND_T noti;
		noti.x = htonl(pos_x);
		noti.y = htonl(pos_y);
		data_request(_szattendant_uuid, CMD_MOUSE_RBU_IND, (char*)&noti, sizeof(CMD_MOUSE_RBU_IND_T));
	}
	return sirius::app::client::proxy::err_code_t::success;
}

//IPCClient CALLBACK
void sirius::app::client::proxy::core::create_session_callback(void)
{
	if (_front)
		_front->on_pre_create_session();


	if (!_dxkey_input_initialized && (_instance != NULL) && (_hwnd != INVALID_HANDLE_VALUE) && (_hwnd != NULL))
	{
		sirius::library::user::event::dinput::receiver::initialize(_instance, _hwnd);
		_dxkey_input_initialized = TRUE;
	}

	if (_front)
		_front->on_create_session();

	_state = sirius::app::client::proxy::state_t::connected;

	if (_front)
		_front->on_post_create_session();
}

void sirius::app::client::proxy::core::destroy_session_callback(void)
{
	if (_front)
		_front->on_pre_destroy_session();

	if (_front)
		_front->on_destroy_session();

	if (_dxkey_input_initialized)
	{
		sirius::library::user::event::dinput::receiver::release();
		_dxkey_input_initialized = FALSE;
	}

	_state = sirius::app::client::proxy::state_t::disconnected;

	if (_front)
		_front->on_stop_streaming();

	if (_front)
		_front->on_post_destroy_session();
}

void sirius::app::client::proxy::core::connect_attendant_callback(const char * dst, const char * src, const char * msg, size_t length)
{
	Json::Value packet;
	Json::Reader reader;
	reader.parse(msg, packet);

	int32_t rcode = -1;
	if (packet["rcode"].isInt())
		rcode = packet["rcode"].asInt();

	std::string multibyte_rcode = packet["msg"].asString();

	wchar_t * resMsg;
	sirius::stringhelper::convert_multibyte2wide((char*)multibyte_rcode.c_str(), &resMsg);

	if (_front)
		_front->on_pre_connect_attendant(rcode, resMsg);

	if (rcode == 0)
	{
		if (_front)
			_front->on_connect_attendant(rcode, resMsg);
	}
	else
	{
		disconnect_attendant();
	}

	if (_front)
		_front->on_post_connect_attendant(rcode, resMsg);

	SysFreeString(resMsg);
}

void sirius::app::client::proxy::core::disconnect_attendant_callback(const char * dst, const char * src, const char * msg, size_t length)
{
	Json::Value packet;
	Json::Reader reader;
	reader.parse(msg, packet);

	if (_front)
		_front->on_pre_disconnect_attendant();

	if (_front)
		_front->on_disconnect_attendant();

	disconnect();

	if (_front)
		_front->on_post_disconnect_attendant();
}

void sirius::app::client::proxy::core::attendant_info_callback(const char * dst, const char * src, const char * msg, size_t length)
{
	int32_t rcode = -1;
	int32_t streamer_portnumber = -1;
	Json::Value packet;
	Json::Reader reader;
	reader.parse(msg, packet);

	if (packet["rcode"].isInt())
	{
		rcode = packet.get("rcode", 1).asInt();
		if (rcode != 0)
		{
			disconnect_attendant();
		}
	}

	std::string slot_uuid = packet.get("attendant_uuid", -1).asString();
	if (slot_uuid.size()>0)
	{
		memset(_szattendant_uuid, 0x00, sizeof(_szattendant_uuid));
		strncpy_s(_szattendant_uuid, sizeof(_szattendant_uuid), slot_uuid.c_str(), slot_uuid.size() + 1);

		wchar_t * attendant_uuid = nullptr;
		sirius::stringhelper::convert_multibyte2wide(_szattendant_uuid, &attendant_uuid);
		if (attendant_uuid)
		{
			memset(_szwattendant_uuid, 0x00, sizeof(_szwattendant_uuid));
			wcsncpy_s(_szwattendant_uuid, attendant_uuid, wcslen(attendant_uuid) + 1);
			::SysFreeString(attendant_uuid);
			attendant_uuid = nullptr;
		}

		_recv_attendant_info = TRUE;
	}

	std::string streamer_addr = packet.get("address", -1).asString();
	if (packet["port"].isInt())
		streamer_portnumber = packet.get("port", -1).asInt();

	wchar_t * attendant_uuid = nullptr;
	wchar_t * address = nullptr;
	sirius::stringhelper::convert_multibyte2wide((char*)slot_uuid.c_str(), &attendant_uuid);
	sirius::stringhelper::convert_multibyte2wide((char*)streamer_addr.c_str(), &address);

	if (_front)
		_front->on_pre_attendant_info(rcode, attendant_uuid, address, streamer_portnumber);

	if (rcode != -1 && _recv_attendant_info && streamer_portnumber>0 && address && wcslen(address) > 0)
	{
		_streamer_portnumber = streamer_portnumber;
		if (_front)
		{
			_front->on_stop_streaming();
			_front->on_open_streaming(attendant_uuid, address, streamer_portnumber, _reconnection);
			_front->on_play_streaming();
		}
	}

	if (_front)
		_front->on_post_attendant_info(rcode, attendant_uuid, address, streamer_portnumber);

	if (attendant_uuid)
		::SysFreeString(attendant_uuid);

	if (address)
		::SysFreeString(address);
}

void sirius::app::client::proxy::core::playback_end_callback(const char * dst, const char * src, const char * msg, size_t length)
{
	if (_front)
		_front->on_pre_playback_end();

	if (_front)
		_front->on_playback_end();

	if (_front)
		_front->on_post_playback_end();
}

void sirius::app::client::proxy::core::playback_totaltime_callback(const char * dst, const char * src, const char * msg, size_t length)
{
	char* total_time_msg = (char*)malloc(length + 1);
	strncpy_s(total_time_msg, length + 1, msg, length);

	int32_t total_time = 0;
	Json::Value root;
	Json::Reader reader;
	reader.parse(total_time_msg, root);
	if (root["value"].isInt())
		total_time = root.get("value", 0).asInt();

	free(total_time_msg);
	total_time_msg = nullptr;

	if (_front)
		_front->on_pre_playback_totaltime(total_time);

	if (_front)
		_front->on_playback_totaltime(total_time);

	if (_front)
		_front->on_post_playback_totaltime(total_time);
}

void sirius::app::client::proxy::core::playback_currenttime_callback(const char * dst, const char * src, const char * msg, size_t length)
{
	char* current_time_msg = (char*)malloc(length + 1);
	strncpy_s(current_time_msg, length + 1, msg, length);

	int32_t current_time = 0;
	Json::Value root;
	Json::Reader reader;
	reader.parse(current_time_msg, root);
	if (root["value"].isInt())
		current_time = root.get("value", 0).asInt();

	free(current_time_msg);
	current_time_msg = nullptr;

	if (_front)
		_front->on_pre_playback_currenttime(current_time);

	if (_front)
		_front->on_playback_currenttime(current_time);

	if (_front)
		_front->on_post_playback_currenttime(current_time);
}

void sirius::app::client::proxy::core::playback_currentrate_callback(const char * dst, const char * src, const char * msg, size_t length)
{
	char* current_rate_msg = (char*)malloc(length + 1);
	strncpy_s(current_rate_msg, length + 1, msg, length);

	float current_rate = 0.f;
	Json::Value root;
	Json::Reader reader;
	reader.parse(current_rate_msg, root);
	if (root["value"].isInt())
		current_rate = root.get("value", 0).asFloat();

	free(current_rate_msg);
	current_rate_msg = nullptr;

	if (_front)
		_front->on_pre_playback_currentrate(current_rate);

	if (_front)
		_front->on_playback_currentrate(current_rate);

	if (_front)
		_front->on_post_playback_currentrate(current_rate);
}

void sirius::app::client::proxy::core::xml_callback(const char * dst, const char * src, const char * msg, size_t length)
{
	if (_front)
		_front->on_pre_xml(msg, length);

	if (_front)
		_front->on_xml(msg, length);

	if (_front)
		_front->on_post_xml(msg, length);
}

void sirius::app::client::proxy::core::error_callback(const char * dst, const char * src, const char * msg, size_t length)
{
	char * error_msg = (char*)malloc(length + 1);
	strncpy_s(error_msg, length + 1, msg, length);

	int32_t error_code = 0;
	Json::Value root;
	Json::Reader reader;
	reader.parse(error_msg, root);
	if (root["value"].isInt())
		error_code = root.get("value", 0).asInt();

	free(error_msg);
	error_msg = nullptr;

	if (_front)
		_front->on_pre_error(error_code);

	if (_front)
		_front->on_error(error_code);

	if (_front)
		_front->on_post_error(error_code);
}

void sirius::app::client::proxy::core::keyup_callback(int32_t value)
{
	if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
	{
		CMD_KEY_UP_IND_T noti;
		memset(&noti, 0x00, sizeof(CMD_KEY_UP_IND_T));
		noti.input_type = 0;
		noti.key_code = htonl(value);//ntohl(value);
		data_request(_szattendant_uuid, CMD_KEY_UP_IND, (char*)&noti, sizeof(CMD_KEY_UP_IND_T));
	}
}

void sirius::app::client::proxy::core::keydown_callback(int32_t value)
{
	if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
	{
		CMD_KEY_DOWN_IND_T noti;
		memset(&noti, 0x00, sizeof(CMD_KEY_DOWN_IND_T));
		noti.input_type = 0;
		noti.key_code = htonl(value);//ntohl(value);
		data_request(_szattendant_uuid, CMD_KEY_DOWN_IND, (char*)&noti, sizeof(CMD_KEY_DOWN_IND_T));
	}
}

void sirius::app::client::proxy::core::keydown_seek_callback(int32_t diff)
{
	if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
	{
		CMD_SEEK_KEY_DOWN_T noti;
		memset(&noti, 0x00, sizeof(CMD_SEEK_KEY_DOWN_T));
		noti.diff = htonl(diff);
		data_request(_szattendant_uuid, CMD_SEEK_KEY_DOWN, (char*)&noti, sizeof(CMD_SEEK_KEY_DOWN_T));
	}
}

void sirius::app::client::proxy::core::keyup_seek_callback(void)
{
	if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
		data_request(_szattendant_uuid, CMD_SEEK_KEY_UP, NULL, 0);
}

void sirius::app::client::proxy::core::mouse_move_callback(int32_t pos_x, int32_t pos_y)
{
	if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
	{
		CMD_MOUSE_MOVE_IND_T noti;
		memset(&noti, 0x00, sizeof(CMD_MOUSE_MOVE_IND_T));
		noti.x_translation = htonl(pos_x);
		noti.y_translation = htonl(pos_y);
		data_request(_szattendant_uuid, CMD_MOUSE_MOVE_IND, (char*)&noti, sizeof(CMD_MOUSE_MOVE_IND_T));
	}
}

void sirius::app::client::proxy::core::mouse_left_button_down_callback(int32_t pos_x, int32_t pos_y)
{
	if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
	{
		CMD_MOUSE_LBD_IND_T noti;
		noti.x = htonl(pos_x);
		noti.y = htonl(pos_y);
		data_request(_szattendant_uuid, CMD_MOUSE_LBD_IND, (char*)&noti, sizeof(CMD_MOUSE_LBD_IND_T));
	}
}

void sirius::app::client::proxy::core::mouse_left_button_up_callback(int32_t pos_x, int32_t pos_y)
{
	if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
	{
		CMD_MOUSE_LBU_IND_T noti;
		noti.x = htonl(pos_x);
		noti.y = htonl(pos_y);
		data_request(_szattendant_uuid, CMD_MOUSE_LBU_IND, (char*)&noti, sizeof(CMD_MOUSE_LBU_IND_T));
	}
}

void sirius::app::client::proxy::core::mouse_right_button_down_callback(int32_t pos_x, int32_t pos_y)
{
	if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
	{
		CMD_MOUSE_RBD_IND_T noti;
		noti.x = htonl(pos_x);
		noti.y = htonl(pos_y);
		data_request(_szattendant_uuid, CMD_MOUSE_RBD_IND, (char*)&noti, sizeof(CMD_MOUSE_RBD_IND_T));
	}
}

void sirius::app::client::proxy::core::mouse_right_button_up_callback(int32_t pos_x, int32_t pos_y)
{
	if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
	{
		CMD_MOUSE_RBU_IND_T noti;
		noti.x = htonl(pos_x);
		noti.y = htonl(pos_y);
		data_request(_szattendant_uuid, CMD_MOUSE_RBU_IND, (char*)&noti, sizeof(CMD_MOUSE_RBU_IND_T));
	}
}

/*
void sirius::app::client::proxy::core::mouse_left_button_down_move_callback(int32_t pos_x, int32_t pos_y)
{
if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
{
CMD_MOUSE_LBD_MOVE_IND_T noti;
noti.x_translation = htonl(pos_x);
noti.x_translation = htonl(pos_y);
data_request(_szattendant_uuid, CMD_MOUSE_LBD_MOVE_IND, (char*)&noti, sizeof(CMD_MOUSE_LBD_MOVE_IND_T));

}
}

void sirius::app::client::proxy::core::mouse_right_button_down_move_callback(int32_t pos_x, int32_t pos_y)
{
if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
{
CMD_MOUSE_RBD_MOVE_IND_T noti;
noti.x_translation = htonl(pos_x);
noti.y_translation = htonl(pos_y);
data_request(_szattendant_uuid, CMD_MOUSE_RBD_MOVE_IND, (char*)&noti, sizeof(CMD_MOUSE_RBD_MOVE_IND_T));
}
}
*/

void sirius::app::client::proxy::core::mouse_left_button_double_callback(int32_t pos_x, int32_t pos_y)
{
	if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
	{
		CMD_MOUSE_LB_DCLICK_IND_T noti;
		noti.x = htonl(pos_x);
		noti.y = htonl(pos_y);
		data_request(_szattendant_uuid, CMD_MOUSE_LB_DCLICK_IND, (char*)&noti, sizeof(CMD_MOUSE_LB_DCLICK_IND_T));
	}
}

void sirius::app::client::proxy::core::mosue_right_button_double_callback(int32_t pos_x, int32_t pos_y)
{
	if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
	{
		CMD_MOUSE_RB_DCLICK_IND_T noti;
		noti.x = htonl(pos_x);
		noti.y = htonl(pos_y);
		data_request(_szattendant_uuid, CMD_MOUSE_RB_DCLICK_IND, (char*)&noti, sizeof(CMD_MOUSE_RB_DCLICK_IND_T));
	}
}

void sirius::app::client::proxy::core::mouse_wheel_callback(int32_t pos_x, int32_t pos_y, int32_t wheel_delta)
{
	if (strlen(_szattendant_uuid) > 0 && _recv_attendant_info)
	{
		CMD_MOUSE_WHEEL_IND_T noti;
		noti.z_delta = htonl(wheel_delta);
		noti.x = htonl(pos_x);
		noti.y = htonl(pos_y);
		data_request(_szattendant_uuid, CMD_MOUSE_WHEEL_IND, (char*)&noti, sizeof(CMD_MOUSE_WHEEL_IND_T));
	}
}

