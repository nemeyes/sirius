#include <sirius_string.h>
#include <sirius_scsp_server.h>
#include <sirius_log4cplus_logger.h>
#include <sirius_stringhelper.h>
#include <sirius_locks.h>

#include "unified_server.h"

sirius::library::unified::server::core::core(void)
	: _streamer(nullptr)
	, _streamer_context(nullptr)
	, _vcmprs_initialized(false)
{
	::InitializeCriticalSection(&_vcs);
}

sirius::library::unified::server::core::~core(void)
{
	::DeleteCriticalSection(&_vcs);
}

int32_t sirius::library::unified::server::core::initialize(sirius::library::unified::server::context_t * context)
{
	int32_t code = sirius::library::unified::server::err_code_t::fail;

	if (context == nullptr)
		return code;
	_context = context;

	sirius::string uuid = _context->uuid;
	sirius::string address = _context->address;

	sirius::library::net::scsp::server * streamer = new sirius::library::net::scsp::server();
	sirius::library::net::scsp::server::context_t * streamer_context = new sirius::library::net::scsp::server::context_t();

	strncpy_s(streamer_context->uuid, uuid.wtoa().c_str(), uuid.size() + 1);
	strncpy_s(streamer_context->address, address.wtoa().c_str(), address.size() + 1);
	streamer_context->portnumber = _context->portnumber;
	streamer_context->video_codec = _context->video_codec;
	streamer_context->video_width = _context->video_width;
	streamer_context->video_height = _context->video_height;
	streamer_context->video_fps = _context->video_fps;
	streamer_context->video_block_width = _context->video_block_width;
	streamer_context->video_block_height = _context->video_block_height;
	streamer_context->controller = this;

	code = streamer->start(streamer_context);
	if (code != sirius::library::unified::server::err_code_t::success)
		return code;

	_streamer_context = streamer_context;
	_streamer = streamer;

	_unified_compressor = new sirius::library::unified::compressor(this);

	return code;
}

int32_t sirius::library::unified::server::core::release(void)
{
	int32_t code = sirius::library::unified::server::err_code_t::fail;
	if (!_context)
		return sirius::library::unified::server::err_code_t::success;

	if (_streamer)
	{
		sirius::library::net::scsp::server * streamer = static_cast<sirius::library::net::scsp::server*>(_streamer);
		sirius::library::net::scsp::server::context_t * streamer_context = static_cast<sirius::library::net::scsp::server::context_t*>(_streamer_context);
		code = streamer->stop();

		delete streamer;
		streamer = nullptr;
		_streamer = nullptr;

		delete streamer_context;
		streamer_context = nullptr;
		_streamer_context = nullptr;

		if (code != sirius::library::unified::server::err_code_t::success)
			return code;
	}

	_context = nullptr;

	if (_unified_compressor)
	{
		delete _unified_compressor;
		_unified_compressor = nullptr;
	}

	return code;
}

bool sirius::library::unified::server::core::is_video_compressor_initialized(void)
{
	return _vcmprs_initialized;
}

int32_t sirius::library::unified::server::core::initialize_video_compressor(sirius::library::unified::server::video_compressor_context_t * context)
{
	sirius::autolock mutex(&_vcs);
	if (_vcmprs_initialized)
		return sirius::library::unified::server::err_code_t::success;

	int32_t status = _unified_compressor->initialize_video_compressor(context);
	if (status != sirius::library::unified::server::err_code_t::success)
	{
		if (_context)
			LOGGER::make_error_log(SLVSC, "%s(), %d initialize_video_compressor fail, Slot_Num:%d", __FUNCTION__, __LINE__, _context->portnumber);
		else
			LOGGER::make_error_log(SLVSC, "%s(), %d initialize_video_compressor fail, Slot_Num:Not Found Slot Num", __FUNCTION__, __LINE__);

		return status;
	}

	_vcmprs_initialized = true;

	LOGGER::make_info_log(SLVSC, "%s(), %d initialize_video_compressor success", __FUNCTION__, __LINE__);

	return status;
}

int32_t sirius::library::unified::server::core::release_video_compressor(void)
{
	sirius::autolock mutex(&_vcs);
	if (!_vcmprs_initialized)
		return sirius::library::unified::server::err_code_t::success;

	int32_t status = _unified_compressor->release_video_compressor();
	_vcmprs_initialized = false;
	return status;
}

int32_t sirius::library::unified::server::core::compress(sirius::library::video::transform::codec::compressor::entity_t * input)
{
	return _unified_compressor->compress(input);
}

sirius::library::unified::server::network_usage_t & sirius::library::unified::server::core::get_network_usage(void)
{
	if (_streamer)
	{
		sirius::library::net::scsp::server * streamer = static_cast<sirius::library::net::scsp::server*>(_streamer);
		_network_usage.video_transferred_bytes = streamer->get_network_usage().video_transferred_bytes;
	}
	return _network_usage;
}


int32_t sirius::library::unified::server::core::play(int32_t flags)
{
	return _unified_compressor->play(flags);
}

int32_t sirius::library::unified::server::core::pause(int32_t flags)
{
	return _unified_compressor->pause(flags);
}

int32_t sirius::library::unified::server::core::stop(int32_t flags)
{
	return _unified_compressor->stop(flags);
}

int32_t sirius::library::unified::server::core::publish_video(uint8_t * bytes, int32_t nbytes, long long before_encode_timestamp, long long after_encode_timestamp)
{
	if (!_context)
		return sirius::library::unified::server::err_code_t::success;

	int32_t code = sirius::library::unified::server::err_code_t::fail;
	if (_streamer)
	{
		sirius::library::net::scsp::server * streamer = static_cast<sirius::library::net::scsp::server*>(_streamer);
		code = streamer->post_video(bytes, nbytes, before_encode_timestamp);
		if (code != sirius::library::unified::server::err_code_t::success)
			return code;
	}
	return code;
}

int32_t sirius::library::unified::server::core::publish_video(int32_t count, int32_t * index, uint8_t ** compressed, int32_t * size, long long before_compress_timestamp, long long after_compress_timestamp)
{
	if (!_context)
		return sirius::library::unified::server::err_code_t::success;

	int32_t code = sirius::library::unified::server::err_code_t::fail;
	if (_streamer)
	{
		sirius::library::net::scsp::server * streamer = static_cast<sirius::library::net::scsp::server*>(_streamer);
		code = streamer->post_video(count, index, compressed, size, before_compress_timestamp);
		if (code != sirius::library::unified::server::err_code_t::success)
			return code;
	}
	return code;
}

void sirius::library::unified::server::core::after_video_compressing_callback(uint8_t * data, size_t size, long long before_encode_timestamp, long long after_encode_timestamp)
{
	publish_video(data, size, before_encode_timestamp, after_encode_timestamp);
}

void sirius::library::unified::server::core::after_video_compressing_callback(int32_t count, int32_t * index, uint8_t ** compressed, int32_t * size, long long before_compress_timestamp, long long after_compress_timestamp)
{
	publish_video(count, index, compressed, size, before_compress_timestamp, after_compress_timestamp);
}
