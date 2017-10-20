#include <platform.h>
#include "sirius_scsp_server.h"
#include "scsp_server.h"
#include <sirius_stringhelper.h>
#include <sirius_log4cplus_logger.h>
#include <sirius_locks.h>
#include <commands_server.h>

sirius::library::net::scsp::server::server(void)
	: _frame_index_video(0)
	, _video_data(nullptr)
	, _core(nullptr)
{

}

sirius::library::net::scsp::server::~server(void)
{
	stop();
}

int32_t sirius::library::net::scsp::server::start(sirius::library::net::scsp::server::context_t * context)
{
	if (!context || !context->address || strlen(context->address) < 1)
		return sirius::library::net::scsp::server::err_code_t::fail;

	int32_t status = sirius::library::net::scsp::server::err_code_t::success;

	if (_video_data == nullptr)
		_video_data = new uint8_t[MAX_VIDEO_ES_SIZE];

	LOGGER::make_trace_log(SLNS, "%s(), %d : vsmt=%d, address=%s, Attendant_number=%d, uuid=%s", __FUNCTION__, __LINE__, context->video_codec, context->address, context->portnumber, context->uuid);

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
	LOGGER::make_info_log(SLNS, "%s, %d port_number=%d, device_id=%s, attendant_number=%d", __FUNCTION__, __LINE__, _context->portnumber, LOGGER::get_device_id(), _context->portnumber - port_number_base);

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

int32_t sirius::library::net::scsp::server::post_video(uint8_t * bytes, size_t nbytes, long long timestamp)
{
	const uint8_t contents_count = 1;
	int32_t data_pos = 0;

	if (MAX_VIDEO_ES_SIZE < nbytes)
		LOGGER::make_error_log(SLNS, "%s(), %d [MINOR] Streaming error(error_code:%d) MAX_VIDEO_ES_SIZE_OVER MAX=%d, size=%d)", __FUNCTION__, __LINE__, sirius::library::net::scsp::server::err_code_t::max_es_size_over, MAX_VIDEO_ES_SIZE, nbytes);

	if ((_video_data != nullptr) && (_core->state(sirius::library::net::scsp::server::media_type_t::video) != sirius::library::net::scsp::server::state_t::stopped))
	{
		int32_t stream_type = sirius::library::net::scsp::server::media_type_t::video;

#if defined(WITH_SAVE_OUTPUT_STREAM)

		char filename[MAX_PATH];
		_snprintf_s(filename, sizeof(filename) - 1, "post_video_%d_%d.png", framenumber, nbytes);
		DWORD nwritten = 0;
		HANDLE f = ::CreateFileA(filename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		::WriteFile(f, bytes, nbytes, &nwritten, NULL);
		::CloseHandle(f);

		framenumber++;
#endif
		sirius::library::net::scsp::stream_packet_t * packet = (sirius::library::net::scsp::stream_packet_t*)(_video_data);
		packet->stream_data.count = contents_count;
		packet->stream_data.data.length = htonl(nbytes);
		packet->stream_data.data.index = htonl(_frame_index_video);
		packet->stream_data.data.timestamp = htonll(timestamp);

		memcpy(_video_data + sizeof(sirius::library::net::scsp::stream_packet_t), bytes, nbytes);

		if (_core)
		{
			_core->post_video(_video_data, sizeof(sirius::library::net::scsp::stream_packet_t) + nbytes, timestamp);
			_network_usage.video_transferred_bytes += data_pos;
		}
	}
	_frame_index_video++;
	return 0;
}