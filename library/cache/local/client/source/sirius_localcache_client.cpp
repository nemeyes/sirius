#include "sirius_localcache_client.h"
#include <localcache_command.h>
#include "localcache_client.h"

sirius::library::cache::local::client::client(void)
{
	_client = new sirius::library::cache::local::client::core;
}

sirius::library::cache::local::client::~client(void)
{
	if (_client)
	{
		delete _client;
	}
	_client = nullptr;
}

int32_t sirius::library::cache::local::client::connect(const char * address, int32_t portnumber, BOOL reconnection)
{
	return _client->connect(address, portnumber, 5, reconnection);
}

int32_t sirius::library::cache::local::client::disconnect(void)
{
	return _client->disconnect();
}

void sirius::library::cache::local::client::disconnect(BOOL enable)
{
	_client->disconnect(enable);
}

int32_t sirius::library::cache::local::client::upload(const char * hash, const char * image, int32_t size, int32_t width, int32_t height)
{
	return _client->upload(hash, image, size, width, height);
}

int32_t sirius::library::cache::local::client::download(const char * hash, char * image, int32_t capacity, int32_t & size)
{
	return _client->download(hash, image, capacity, size);
}

int32_t sirius::library::cache::local::client::ftell(const char * hash, int32_t fsize)
{
	return _client->ftell(hash, fsize);
}