#include "sirius_native_client_framework.h"
#include "native_client_framework.h"

sirius::library::framework::client::native::native(void)
{
	_core = new sirius::library::framework::client::native::core();
}

sirius::library::framework::client::native::~native(void)
{
	if (_core)
	{
		delete _core;
		_core = nullptr;
	}
}

int32_t sirius::library::framework::client::native::state(void)
{
	return _core->state();
}

int32_t sirius::library::framework::client::native::open(wchar_t * url, int32_t port, int32_t recv_option, bool repeat)
{
	return _core->open(url, port, recv_option, repeat);
}

int32_t sirius::library::framework::client::native::play(HWND hwnd)
{
	return _core->play(hwnd);
}

int32_t sirius::library::framework::client::native::stop(void)
{
	return _core->stop();
}

sirius::library::framework::client::base * create_client_framework(void)
{
	return new sirius::library::framework::client::native();
}

void destroy_client_framework(sirius::library::framework::client::base ** client_framework)
{
	sirius::library::framework::client::native * self = dynamic_cast<sirius::library::framework::client::native*>((*client_framework));
	delete self;
	(*client_framework) = 0;
}
