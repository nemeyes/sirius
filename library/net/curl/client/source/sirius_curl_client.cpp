#include "sirius_curl_client.h"
#include <memory.h>

#include "curl_client.h"

sirius::library::net::curl::client::client(int sending_timeout)
	: _core(nullptr)
{
	_core = new sirius::library::net::curl::client::core(sending_timeout);
}

sirius::library::net::curl::client::~client(void)
{
	if (_core)
		delete _core;
	_core = nullptr;
}

bool sirius::library::net::curl::client::set_url(char * url, int len)
{
	return _core->set_url(url, len);
}

void sirius::library::net::curl::client::set_method(HTTP_METHOD_T method)
{
	_core->set_method(method);
}

void sirius::library::net::curl::client::set_callback_function(void(*func) (int message_id, int status_code, void* data, int length))
{
	_core->set_callback_function(func);
}

bool sirius::library::net::curl::client::set_post_data(char* data)
{
	if (_core)
		return _core->set_post_data(data);
	else
		return false;
}

void sirius::library::net::curl::client::set_get_data(char* parameters, int stat_type)
{
	if (_core)
		_core->set_get_data(parameters, stat_type);
}

bool sirius::library::net::curl::client::send(void)
{
	if (_core)
		return _core->send();
	return false;
}

int sirius::library::net::curl::client::get_send_err()
{
	int err = _core->get_send_err();
	return err;
}

int sirius::library::net::curl::client::get_sending_timeout()
{
	int time_out = _core->get_sending_timeout();
	return time_out;
}
