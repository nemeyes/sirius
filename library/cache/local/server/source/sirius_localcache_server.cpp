#include "sirius_localcache_server.h"
#include <localcache_command.h>
#include "localcache_server.h"

sirius::library::cache::local::server::server(void)
	: _server(NULL)
	, _context(NULL)
{}

sirius::library::cache::local::server::~server(void)
{}

int32_t	sirius::library::cache::local::server::initialize(sirius::library::cache::local::server::context_t * context)
{
	_context = context;
	_server = new sirius::library::cache::local::server::core(this, _context->nsessions);
	return _server->initialize(_context);
}

int32_t	sirius::library::cache::local::server::release(void)
{
	if (_server)
	{
		_server->release();
		delete _server;
		_server = NULL;
	}
	
	return sirius::library::cache::local::server::core::err_code_t::success;
}

int32_t sirius::library::cache::local::server::start(void)
{
	if (_server)
		return _server->start(_context->portnumber, _context->nthread_pool);
	else
		return sirius::library::cache::local::server::core::err_code_t::fail;
}

int32_t sirius::library::cache::local::server::stop(void)
{
	if (_server)
		return _server->stop();
	else
		return sirius::library::cache::local::server::core::err_code_t::fail;
}
