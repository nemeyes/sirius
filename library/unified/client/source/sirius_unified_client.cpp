#include "sirius_unified_client.h"
#include "unified_client.h"

sirius::library::unified::client::client(void)
{
	_core = new sirius::library::unified::client::core(this);
}

sirius::library::unified::client::~client(void)
{
	if (_core)
	{
		//_core->stop();
		delete _core;
		_core = nullptr;
	}
}

int32_t sirius::library::unified::client::state(void)
{
	return _core->state();
}

int32_t sirius::library::unified::client::open(wchar_t * url, int32_t port, int32_t recv_option, bool reconnect, bool keepalive, int32_t keepalive_timeout)
{
	return _core->open(url, port, recv_option, reconnect, keepalive, keepalive_timeout);
}

int32_t sirius::library::unified::client::play(void)
{
	return _core->play();
}

int32_t sirius::library::unified::client::stop(void)
{
	return _core->stop();
}