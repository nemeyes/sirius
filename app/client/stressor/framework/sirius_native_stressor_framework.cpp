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

int32_t sirius::library::framework::stressor::native::open(wchar_t * url, int32_t port, int32_t recv_option, bool repeat)
{
	return _core->open(url, port, recv_option, repeat);
}

int32_t sirius::library::framework::stressor::native::play(HWND hwnd)
{
	return _core->play(hwnd);
}

int32_t sirius::library::framework::stressor::native::stop(void)
{
	return _core->stop();
}

void sirius::library::framework::stressor::native::stream_connect_callback()
{
	if (_front)
		_front->stream_connect_callback();
}

void sirius::library::framework::stressor::native::stream_disconnect_callback()
{
	if (_front)
		_front->stream_disconnect_callback();
}