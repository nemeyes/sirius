#include <memory>
#include <string>
#include <process.h>
#include <commands_payload.h>
#include <commands_client.h>
#include "client_proxy.h"
#include "json/json.h"
#include <sirius_stringhelper.h>
#include <sirius_client_framework.h>

#define MTU_SIZE					1500
#define IO_THREAD_POOL_COUNT		1
#define COMMAND_THREAD_POOL_COUNT	1

sirius::app::client::proxy::core::core(sirius::app::client::proxy * front, bool keepalive, bool tls, HINSTANCE instance, HWND hwnd)
	: sirius::library::net::sicp::client(MTU_SIZE, MTU_SIZE, MTU_SIZE, MTU_SIZE, IO_THREAD_POOL_COUNT, COMMAND_THREAD_POOL_COUNT, keepalive?TRUE:FALSE, tls?TRUE:FALSE)
	, _state(sirius::app::client::proxy::state_t::disconnected)
	, _front(front)
	, _reconnection(false)
	, _recv_attendant_info(false)
	, _streamer_portnumber(-1)
{
	add_command(new sirius::app::client::connect_client_res(this));
	add_command(new sirius::app::client::disconnect_client_res(this));
	add_command(new sirius::app::client::attendant_info_noti(this));
	add_command(new sirius::app::client::xml_noti(this));
	add_command(new sirius::app::client::error_noti(this));

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


	memset(_szattendant_uuid, 0x00, sizeof(_szattendant_uuid));
}

sirius::app::client::proxy::core::~core(void)
{
	disconnect();
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

int32_t sirius::app::client::proxy::core::connect(wchar_t * address, int32_t portnumber, bool reconnection)
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
		if (sirius::library::net::sicp::client::connect(_address, portnumber, _reconnection))
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

int32_t sirius::app::client::proxy::core::connect_client(wchar_t * id)
{
	char * client_id = nullptr;

	sirius::stringhelper::convert_wide2multibyte((LPWSTR)(LPCWSTR)id, &client_id);
	if (client_id && strlen(client_id) > 0)
	{
		Json::Value wpacket;
		Json::StyledWriter writer;

		wpacket["id"] = client_id;
		std::string request = writer.write(wpacket);
		if (request.size() > 0)
			data_request(SERVER_UUID, CMD_CONNECT_CLIENT_REQ, (char*)request.c_str(), request.size() + 1);

		free(client_id);
		client_id = nullptr;
	}
	_recv_attendant_info = false;

	return sirius::app::client::proxy::err_code_t::success;
}

int32_t sirius::app::client::proxy::core::disconnect_client(void)
{
	data_request(SERVER_UUID, CMD_DISCONNECT_CLIENT_REQ, NULL, 0);
	_recv_attendant_info = false;

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
void sirius::app::client::proxy::core::on_create_session(void)
{
	if (_front)
		_front->on_pre_create_session();

	if (_front)
		_front->on_create_session();

	_state = sirius::app::client::proxy::state_t::connected;

	if (_front)
		_front->on_post_create_session();
}

void sirius::app::client::proxy::core::on_destroy_session(void)
{
	if (_front)
		_front->on_pre_destroy_session();

	if (_front)
		_front->on_destroy_session();

	_state = sirius::app::client::proxy::state_t::disconnected;

	if (_front)
		_front->on_stop_streaming();

	if (_front)
		_front->on_post_destroy_session();
}

void sirius::app::client::proxy::core::connect_client_callback(int32_t code, const char * msg)
{
	wchar_t * wmsg;
	sirius::stringhelper::convert_multibyte2wide((char*)msg, &wmsg);

	if (_front)
		_front->on_pre_connect_client(code, wmsg);

	if (code == 0)
	{
		if (_front)
			_front->on_connect_client(code, wmsg);
	}
	else
	{
		disconnect_client();
	}

	if (_front)
		_front->on_post_connect_client(code, wmsg);

	SysFreeString(wmsg);
}

void sirius::app::client::proxy::core::disconnect_client_callback(int32_t code)
{
	if (_front)
		_front->on_pre_disconnect_client(code);

	if (_front)
		_front->on_disconnect_client(code);

	disconnect();

	if (_front)
		_front->on_post_disconnect_client(code);
}

void sirius::app::client::proxy::core::attendant_info_callback(int32_t code, const char * attendant_uuid, int32_t streamer_portnumber, int32_t video_width, int32_t video_height)
{
	memset(_szattendant_uuid, 0x00, sizeof(_szattendant_uuid));
	strncpy_s(_szattendant_uuid, sizeof(_szattendant_uuid), attendant_uuid, strlen(attendant_uuid) + 1);

	wchar_t * wattendant_uuid = nullptr;
	sirius::stringhelper::convert_multibyte2wide(_szattendant_uuid, &wattendant_uuid);
	if (wattendant_uuid)
	{
		memset(_szwattendant_uuid, 0x00, sizeof(_szwattendant_uuid));
		wcsncpy_s(_szwattendant_uuid, wattendant_uuid, wcslen(wattendant_uuid) + 1);
		::SysFreeString(wattendant_uuid);
		wattendant_uuid = nullptr;
	}
	_recv_attendant_info = true;

	if (_front)
		_front->on_pre_attendant_info(code, _szwattendant_uuid, streamer_portnumber, video_width, video_height);

	if ((code == sirius::app::client::proxy::err_code_t::success) && _recv_attendant_info && (streamer_portnumber>0))
	{
		_streamer_portnumber = streamer_portnumber;
		if (_front)
		{
			_front->on_stop_streaming();
			_front->on_open_streaming(_szwattendant_uuid, streamer_portnumber, _reconnection);
			_front->on_play_streaming();
		}
	}

	if (_front)
		_front->on_post_attendant_info(code, _szwattendant_uuid, streamer_portnumber, video_width, video_height);
}

void sirius::app::client::proxy::core::xml_callback(const char * msg, size_t length)
{
	if (_front)
		_front->on_pre_xml(msg, length);

	if (_front)
		_front->on_xml(msg, length);

	if (_front)
		_front->on_post_xml(msg, length);
}

void sirius::app::client::proxy::core::error_callback(int32_t code)
{
	if (_front)
		_front->on_pre_error(code);

	if (_front)
		_front->on_error(code);

	if (_front)
		_front->on_post_error(code);
}