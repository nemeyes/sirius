#include "sirius_native_stressor_framework.h"
#include "native_stressor_framework.h"

sirius::library::framework::stressor::native::native(stressor_controller * front)
	: _front(front)
{
	_core = new sirius::library::framework::stressor::native::core(this);
}

sirius::library::framework::stressor::native::~native(void)
{
	if (_core)
	{
		delete _core;
		_core = nullptr;
	}
}

int32_t sirius::library::framework::stressor::native::state(void)
{
	return _core->state();
}

int32_t sirius::library::framework::stressor::native::open(wchar_t * url, int32_t port, int32_t recv_option, bool reconnect, bool keepalive, int32_t keepalive_timeout)
{
	return _core->open(url, port, recv_option, reconnect, keepalive, keepalive_timeout);
}

int32_t sirius::library::framework::stressor::native::play(HWND hwnd)
{
	return _core->play(hwnd);
}

int32_t sirius::library::framework::stressor::native::stop(void)
{
	return _core->stop();
}

void sirius::library::framework::stressor::native::on_connect_stream(void)
{
	if (_front)
		_front->on_connect_stream();
}

void sirius::library::framework::stressor::native::on_disconnect_stream(void)
{
	if (_front)
		_front->on_disconnect_stream();
}

void sirius::library::framework::stressor::native::on_recv_stream(void)
{
	if (_front)
		_front->on_recv_stream();
}