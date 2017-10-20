#include "sirius_unified_server.h"
#include "unified_server.h"

sirius::library::unified::server & sirius::library::unified::server::instance(void)
{
	static sirius::library::unified::server _instance;
	return _instance;
}

sirius::library::unified::server::server(void)
{
	_core = new sirius::library::unified::server::core();
}

sirius::library::unified::server::~server(void)
{
	release();
	release_video_compressor();

	if (_core)
		delete _core;
	_core = nullptr;
}

int32_t sirius::library::unified::server::initialize(sirius::library::unified::server::context_t * context)
{
	release();
	return _core->initialize(context);
}

int32_t sirius::library::unified::server::release(void)
{
	return _core->release();
}

bool sirius::library::unified::server::is_video_compressor_initialized(void)
{
	return _core->is_video_compressor_initialized();
}

int32_t sirius::library::unified::server::initialize_video_compressor(sirius::library::unified::server::video_compressor_context_t * context)
{
	release_video_compressor();
	return _core->initialize_video_compressor(context);
}

int32_t sirius::library::unified::server::release_video_compressor(void)
{
	return _core->release_video_compressor();
}

int32_t sirius::library::unified::server::compress(sirius::library::video::transform::codec::compressor::entity_t * input)
{
	return _core->compress(input);
}

sirius::library::unified::server::network_usage_t & sirius::library::unified::server::get_network_usage(void)
{
	return _core->get_network_usage();
}
