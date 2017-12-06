#include "sirius_client_proxy.h"
#include "client_proxy.h"

sirius::app::client::proxy::handler::handler(void)
{

}

sirius::app::client::proxy::handler::~handler(void)
{

}

void sirius::app::client::proxy::handler::set_proxy(sirius::app::client::proxy * prxy)
{
	_proxy = prxy;
}

int32_t sirius::app::client::proxy::handler::state(void)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_proxy)
		status = _proxy->state();

	return status;
}

bool sirius::app::client::proxy::handler::recv_attendant_info(void)
{
	bool status = false;

	if (_proxy)
		status = _proxy->recv_attendant_info();

	return status;
}

const wchar_t * sirius::app::client::proxy::handler::attendant_uuid(void)
{
	if (_proxy)
		return _proxy->attendant_uuid();
	return nullptr;
}

const wchar_t *	sirius::app::client::proxy::handler::address(void)
{
	if (_proxy)
		return _proxy->address();
	return nullptr;
}

const int32_t sirius::app::client::proxy::handler::streamer_portnumber(void)
{
	int32_t portnumber = -1;
	if (_proxy)
		portnumber = _proxy->streamer_portnumber();
	return portnumber;
}

int32_t sirius::app::client::proxy::handler::set_using_mouse(BOOL value)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_proxy)
		status = _proxy->set_using_mouse(value);

	return status;
}

int32_t sirius::app::client::proxy::handler::set_key_stroke(int32_t interval)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_proxy)
		status = _proxy->set_key_stroke(interval);

	return status;
}

int32_t sirius::app::client::proxy::handler::connect(wchar_t * address, int32_t portnumber, BOOL reconnection)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_proxy)
		status = _proxy->connect(address, portnumber, reconnection);

	return status;
}

int32_t sirius::app::client::proxy::handler::disconnect(void)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_proxy)
		status = _proxy->disconnect();

	return status;
}

int32_t sirius::app::client::proxy::handler::connect_attendant(wchar_t * appid, wchar_t * deviceid, wchar_t * devicetype, wchar_t * envtype, wchar_t * modeltype, int32_t width, int32_t height, int32_t gpuindex)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_proxy)
		status = _proxy->connect_attendant(appid, deviceid, devicetype, envtype, modeltype, width, height, gpuindex);

	return status;
}

int32_t sirius::app::client::proxy::handler::disconnect_attendant(void)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_proxy)
		status = _proxy->disconnect_attendant();

	return status;
}

int32_t sirius::app::client::proxy::handler::key_up(int32_t value)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_proxy)
		status = _proxy->key_up(value);

	return status;
}

int32_t	sirius::app::client::proxy::handler::key_down(int32_t value)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_proxy)
		status = _proxy->key_down(value);

	return status;
}

int32_t sirius::app::client::proxy::handler::mouse_move(int32_t pos_x, int32_t pos_y)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_proxy)
		status = _proxy->mouse_move(pos_x, pos_y);

	return status;
}

int32_t sirius::app::client::proxy::handler::mouse_wheel(int32_t pos_x, int32_t pos_y, int32_t wheel_delta)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_proxy)
		status = _proxy->mouse_wheel(pos_x, pos_y, wheel_delta);

	return status;
}

int32_t sirius::app::client::proxy::handler::mouse_lb_double(int32_t pos_x, int32_t pos_y)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_proxy)
		status = _proxy->mouse_lb_double(pos_x, pos_y);

	return status;
}

int32_t sirius::app::client::proxy::handler::mouse_lb_down(int32_t pos_x, int32_t pos_y)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_proxy)
		status = _proxy->mouse_lb_down(pos_x, pos_y);

	return status;
}

int32_t sirius::app::client::proxy::handler::mouse_lb_up(int32_t pos_x, int32_t pos_y)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_proxy)
		status = _proxy->mouse_lb_up(pos_x, pos_y);

	return status;
}

int32_t sirius::app::client::proxy::handler::mouse_rb_double(int32_t pos_x, int32_t pos_y)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_proxy)
		status = _proxy->mouse_rb_double(pos_x, pos_y);

	return status;
}

int32_t sirius::app::client::proxy::handler::mouse_rb_down(int32_t pos_x, int32_t pos_y)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_proxy)
		status = _proxy->mouse_rb_down(pos_x, pos_y);

	return status;
}

int32_t sirius::app::client::proxy::handler::mouse_rb_up(int32_t pos_x, int32_t pos_y)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_proxy)
		status = _proxy->mouse_rb_up(pos_x, pos_y);

	return status;
}


sirius::app::client::proxy::proxy(sirius::app::client::proxy::handler * hndlr, HINSTANCE instance, HWND hwnd)
	: _handler(hndlr)
{
	_core = new sirius::app::client::proxy::core(this, instance, hwnd);

	if (_handler)
		_handler->set_proxy(this);
}

sirius::app::client::proxy::~proxy(void)
{
	if (_handler)
		_handler->set_proxy(NULL);

	_handler = NULL;

	if (_core)
	{
		_core->disconnect();
		delete _core;
		_core = NULL;
	}
}

int32_t sirius::app::client::proxy::state(void)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_core)
		status = _core->state();

	return status;
}

bool sirius::app::client::proxy::recv_attendant_info(void)
{
	bool status = FALSE;

	if (_core)
		status = _core->recv_attendant_info();

	return status;
}

const wchar_t * sirius::app::client::proxy::attendant_uuid(void)
{
	if (_core)
		return _core->attendant_uuid();
	return nullptr;
}

const wchar_t *	sirius::app::client::proxy::address(void)
{
	if (_core)
		return _core->address();
	return nullptr;
}

const int32_t sirius::app::client::proxy::streamer_portnumber(void)
{
	int32_t streamingPortNumber = -1;
	if (_core)
		streamingPortNumber = _core->streamer_portnumber();
	return streamingPortNumber;
}

int32_t sirius::app::client::proxy::set_using_mouse(BOOL value)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_core)
		status = _core->set_using_mouse(value);

	return status;
}

int32_t sirius::app::client::proxy::set_key_stroke(int32_t interval)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_core)
		status = _core->set_key_stroke(interval);

	return status;
}

int32_t sirius::app::client::proxy::connect(wchar_t * address, int32_t portnumber, BOOL reconnection)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_core)
		status = _core->connect(address, portnumber, reconnection);

	return status;
}

int32_t sirius::app::client::proxy::disconnect(void)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_core)
		status = _core->disconnect();

	return status;
}

int32_t sirius::app::client::proxy::connect_attendant(wchar_t * appid, wchar_t * deviceid, wchar_t * devicetype, wchar_t * envtype, wchar_t * modeltype, int32_t width, int32_t height, int32_t gpuindex)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_core)
		status = _core->connect_attendant(appid, deviceid, devicetype, envtype, modeltype, width, height, gpuindex);

	return status;
}

int32_t sirius::app::client::proxy::disconnect_attendant(void)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_core)
		status = _core->disconnect_attendant();

	return status;
}

int32_t	sirius::app::client::proxy::key_up(int32_t value)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_core)
		status = _core->key_up(value);

	return status;
}

int32_t	sirius::app::client::proxy::key_down(int32_t value)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_core)
		status = _core->key_down(value);

	return status;
}

int32_t sirius::app::client::proxy::mouse_move(int32_t pos_x, int32_t pos_y)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_core)
		status = _core->mouse_move(pos_x, pos_y);

	return status;
}

int32_t sirius::app::client::proxy::mouse_wheel(int32_t pos_x, int32_t pos_y, int32_t wheel_delta)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_core)
		status = _core->mouse_wheel(pos_x, pos_y, wheel_delta);

	return status;
}

int32_t sirius::app::client::proxy::mouse_lb_double(int32_t pos_x, int32_t pos_y)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_core)
		status = _core->mouse_lb_double(pos_x, pos_y);

	return status;
}

int32_t sirius::app::client::proxy::mouse_lb_down(int32_t pos_x, int32_t pos_y)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_core)
		status = _core->mouse_lb_down(pos_x, pos_y);

	return status;
}

int32_t sirius::app::client::proxy::mouse_lb_up(int32_t pos_x, int32_t pos_y)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_core)
		status = _core->mouse_lb_up(pos_x, pos_y);

	return status;
}

int32_t sirius::app::client::proxy::mouse_rb_double(int32_t pos_x, int32_t pos_y)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_core)
		status = _core->mouse_rb_double(pos_x, pos_y);

	return status;
}

int32_t sirius::app::client::proxy::mouse_rb_down(int32_t pos_x, int32_t pos_y)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_core)
		status = _core->mouse_rb_down(pos_x, pos_y);

	return status;
}

int32_t sirius::app::client::proxy::mouse_rb_up(int32_t pos_x, int32_t pos_y)
{
	int32_t status = sirius::app::client::proxy::err_code_t::fail;

	if (_core)
		status = _core->mouse_rb_up(pos_x, pos_y);

	return status;
}

//CALLBACK
void sirius::app::client::proxy::on_pre_connect(wchar_t * address, int32_t portNumber, BOOL reconnection)
{
	if (_handler)
		_handler->on_pre_connect(address, portNumber, reconnection);
}

void sirius::app::client::proxy::on_post_connect(wchar_t * address, int32_t portNumber, BOOL reconnection)
{
	if (_handler)
		_handler->on_post_connect(address, portNumber, reconnection);
}

void sirius::app::client::proxy::on_pre_disconnect(void)
{
	if (_handler)
		_handler->on_pre_disconnect();
}

void sirius::app::client::proxy::on_post_disconnect(void)
{
	if (_handler)
		_handler->on_post_disconnect();
}

void sirius::app::client::proxy::on_pre_create_session(void)
{
	if (_handler)
		_handler->on_pre_create_session();
}

void sirius::app::client::proxy::on_create_session(void)
{
	if (_handler)
		_handler->on_create_session();
}

void sirius::app::client::proxy::on_post_create_session(void)
{
	if (_handler)
		_handler->on_post_create_session();
}

void sirius::app::client::proxy::on_pre_keepalive(void)
{
	if (_handler)
		_handler->on_pre_keepalive();
}

void sirius::app::client::proxy::on_keepalive(void)
{
	if (_handler)
		_handler->on_keepalive();
}

void sirius::app::client::proxy::on_post_keepalive(void)
{
	if (_handler)
		_handler->on_post_keepalive();
}

void sirius::app::client::proxy::on_pre_destroy_session(void)
{
	if (_handler)
		_handler->on_pre_destroy_session();
}

void sirius::app::client::proxy::on_destroy_session(void)
{
	if (_handler)
		_handler->on_destroy_session();
}

void sirius::app::client::proxy::on_post_destroy_session(void)
{
	if (_handler)
		_handler->on_post_destroy_session();
}

void sirius::app::client::proxy::on_pre_connect_attendant(int32_t code, wchar_t * msg)
{
	if (_handler)
		_handler->on_pre_connect_attendant(code, msg);
}

void sirius::app::client::proxy::on_connect_attendant(int32_t code, wchar_t * msg)
{
	if (_handler)
		_handler->on_connect_attendant(code, msg);
}

void sirius::app::client::proxy::on_post_connect_attendant(int32_t code, wchar_t * msg)
{
	if (_handler)
		_handler->on_post_connect_attendant(code, msg);
}

void sirius::app::client::proxy::on_pre_disconnect_attendant(void)
{
	if (_handler)
		_handler->on_pre_disconnect_attendant();
}

void sirius::app::client::proxy::on_disconnect_attendant(void)
{
	if (_handler)
		_handler->on_disconnect_attendant();
}

void sirius::app::client::proxy::on_post_disconnect_attendant(void)
{
	if (_handler)
		_handler->on_post_disconnect_attendant();
}

void sirius::app::client::proxy::on_pre_attendant_info(int32_t code, wchar_t * attendant_uuid, wchar_t * streamer_address, int32_t streamer_portnumber)
{
	if (_handler)
		_handler->on_pre_attendant_info(code, attendant_uuid, streamer_address, streamer_portnumber);
}

void sirius::app::client::proxy::on_post_attendant_info(int32_t code, wchar_t * attendant_uuid, wchar_t * streamer_address, int32_t streamer_portnumber)
{
	if (_handler)
		_handler->on_post_attendant_info(code, attendant_uuid, streamer_address, streamer_portnumber);
}

void sirius::app::client::proxy::on_open_streaming(wchar_t * attendant_uuid, wchar_t * streamer_address, int32_t streamer_portnumber, BOOL reconnection)
{
	if (_handler)
		_handler->on_open_streaming(attendant_uuid, streamer_address, streamer_portnumber, reconnection);
}

void sirius::app::client::proxy::on_play_streaming(void)
{
	if (_handler)
		_handler->on_play_streaming();
}

void sirius::app::client::proxy::on_stop_streaming(void)
{
	if (_handler)
		_handler->on_stop_streaming();
}

void sirius::app::client::proxy::on_pre_xml(const char * msg, size_t length)
{
	if (_handler)
		_handler->on_pre_xml(msg, length);
}

void sirius::app::client::proxy::on_xml(const char * msg, size_t length)
{
	if (_handler)
		_handler->on_xml(msg, length);
}

void sirius::app::client::proxy::on_post_xml(const char * msg, size_t length)
{
	if (_handler)
		_handler->on_post_xml(msg, length);
}

void sirius::app::client::proxy::on_pre_error(int32_t error_code)
{
	if (_handler)
		_handler->on_pre_error(error_code);
}

void sirius::app::client::proxy::on_error(int32_t error_code)
{
	if (_handler)
		_handler->on_error(error_code);
}

void sirius::app::client::proxy::on_post_error(int32_t error_code)
{
	if (_handler)
		_handler->on_post_error(error_code);
}
