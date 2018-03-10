#include <platform.h>
#include "sirius_scsp_server.h"
#include "scsp_server.h"
#include <sirius_stringhelper.h>
#include <sirius_log4cplus_logger.h>
#include <sirius_locks.h>
#include <commands_server.h>

sirius::library::net::scsp::server::server(void)
	: _video_data(nullptr)
	, _core(nullptr)
{

}

sirius::library::net::scsp::server::~server(void)
{

}

int32_t sirius::library::net::scsp::server::start(sirius::library::net::scsp::server::context_t * context)
{
	if (!context)
		return sirius::library::net::scsp::server::err_code_t::fail;

	int32_t status = sirius::library::net::scsp::server::err_code_t::success;

	if (_video_data == nullptr)
		_video_data = new uint8_t[MAX_VIDEO_ES_SIZE];

	//LOGGER::make_trace_log(SLNS, "%s(), %d : vsmt=%d, attendant_number=%d, uuid=%s", __FUNCTION__, __LINE__, context->video_codec, context->portnumber, context->uuid);

	_context = context;
	_context->portnumber = _context->portnumber/* + port_number_base */;

	if (_core)
	{
		_core->stop();
		delete _core;
		_core = nullptr;
	}

	_core = new sirius::library::net::scsp::server::core(SERVER_UUID, this);
	_core->start(context);
	LOGGER::make_info_log(SLNSC, "%s(), %d, port_number=%d, attendant_number=%d", __FUNCTION__, __LINE__, _context->portnumber, _context->portnumber - port_number_base);

	return sirius::library::net::scsp::server::err_code_t::success;
}

int32_t sirius::library::net::scsp::server::stop(void)
{
	int32_t status = sirius::library::net::scsp::server::err_code_t::success;
	if (_core)
	{
		status = _core->stop();
		delete _core;
		_core = nullptr;
	}

	if (_video_data)
	{
		delete[] _video_data;
		_video_data = nullptr;
	}

	return status;
}

#if defined(WITH_SAVE_OUTPUT_STREAM)
int32_t framenumber = 0;
#endif

int32_t sirius::library::net::scsp::server::post_video(int32_t count, int32_t * index, uint8_t ** compressed, int32_t * size, long long timestamp)
{
	if ((_video_data != nullptr) && (_core->state(sirius::library::net::scsp::server::media_type_t::video) != sirius::library::net::scsp::server::state_t::stopped))
	{

		uint8_t * video_data = _video_data;

		int32_t pkt_count = htonl(count);
		memmove(video_data, &pkt_count, sizeof(pkt_count));
		video_data += sizeof(pkt_count);

		for (int32_t x = 0; x < count; x++)
		{
			sirius::library::net::scsp::index_header_t header;
			header.index = htonl(index[x]);
			header.length = htonl(size[x]);

			memmove(video_data, &header, sizeof(header));
			video_data += sizeof(header);
			memmove(video_data, compressed[x], size[x]);
			video_data += size[x];
		}

		if (_core)
		{
			_core->post_indexed_video(_video_data, video_data - _video_data, timestamp);
			_network_usage.video_transferred_bytes += (video_data - _video_data);
		}
	}
	return sirius::library::net::scsp::server::err_code_t::success;
}

int32_t sirius::library::net::scsp::server::post_video(int32_t index, uint8_t * compressed, int32_t size, long long timestamp)
{
	if ((_video_data != nullptr) && (_core->state(sirius::library::net::scsp::server::media_type_t::video) != sirius::library::net::scsp::server::state_t::stopped))
	{

		uint8_t * video_data = _video_data;

		int32_t pkt_count = htonl(1);
		memmove(video_data, &pkt_count, sizeof(pkt_count));
		video_data += sizeof(pkt_count);

		sirius::library::net::scsp::index_header_t header;
		header.index = htonl(index);
		header.length = htonl(size);

		memmove(video_data, &header, sizeof(header));
		video_data += sizeof(header);

		memmove(video_data, compressed, size);
		video_data += size;

		if (_core)
		{
			_core->post_indexed_video(_video_data, video_data - _video_data, timestamp);
			_network_usage.video_transferred_bytes += (video_data - _video_data);
		}
	}
	return sirius::library::net::scsp::server::err_code_t::success;
}

int32_t sirius::library::net::scsp::server::post_video(int32_t count, int16_t * x, int16_t * y, int16_t * width, int16_t * height, uint8_t ** compressed, int32_t * size, long long timestamp)
{
	if ((_video_data != nullptr) && (_core->state(sirius::library::net::scsp::server::media_type_t::video) != sirius::library::net::scsp::server::state_t::stopped))
	{

		uint8_t * video_data = _video_data;

		int32_t pkt_count = htonl(count);
		memmove(video_data, &pkt_count, sizeof(pkt_count));
		video_data += sizeof(pkt_count);

		for (int32_t i = 0; i < count; i++)
		{
			sirius::library::net::scsp::coordinates_header_t header;
			header.x = htons(x[i]);
			header.y = htons(y[i]);
			header.width = htons(width[i]);
			header.height = htons(height[i]);
			header.length = htonl(size[i]);

			memmove(video_data, &header, sizeof(header));
			video_data += sizeof(header);
			memmove(video_data, compressed[i], size[i]);
			video_data += size[i];
		}

		if (_core)
		{
			_core->post_coordinates_video(_video_data, video_data - _video_data, timestamp);
			_network_usage.video_transferred_bytes += (video_data - _video_data);
		}
	}
	return sirius::library::net::scsp::server::err_code_t::success;
}

int32_t sirius::library::net::scsp::server::post_video(int16_t x, int16_t y, int16_t width, int16_t height, uint8_t * compressed, int32_t size, long long timestamp)
{
	if ((_video_data != nullptr) && (_core->state(sirius::library::net::scsp::server::media_type_t::video) != sirius::library::net::scsp::server::state_t::stopped))
	{

		uint8_t * video_data = _video_data;

		int32_t pkt_count = htonl(1);
		memmove(video_data, &pkt_count, sizeof(pkt_count));
		video_data += sizeof(pkt_count);

		sirius::library::net::scsp::coordinates_header_t header;
		header.x = htons(x);
		header.y = htons(y);
		header.width = htons(width);
		header.height = htons(height);
		header.length = htonl(size);

		memmove(video_data, &header, sizeof(header));
		video_data += sizeof(header);

		memmove(video_data, compressed, size);
		video_data += size;

		if (_core)
		{
			_core->post_coordinates_video(_video_data, video_data - _video_data, timestamp);
			_network_usage.video_transferred_bytes += (video_data - _video_data);
		}
	}
	return sirius::library::net::scsp::server::err_code_t::success;
}