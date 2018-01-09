#include "sirius_native_stressor_framework.h"
#include "native_stressor_framework.h"

sirius::library::framework::stressor::native::native(void)
{
	_core = new sirius::library::framework::stressor::native::core();
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
