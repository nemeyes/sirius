#include <process.h>

#include <commands_payload.h>
#include <commands_client.h>
#include "scsp_client.h"
#include <json/json.h>

#include <commands_client.h>
#include <sirius_log4cplus_logger.h>
#include <sirius_locks.h>

#define USE_MINIMIZED_VIDEO_PACKET

sirius::library::net::scsp::client::core::core(sirius::library::net::scsp::client * front)
	: sirius::library::net::sicp::client(MTU_SIZE, 1024 * 1024 * 2, 1024 * 1024 * 2, 1024 * 1024 * 4,COMMAND_THREAD_POOL_COUNT, IO_THREAD_POOL_COUNT, false, false, sirius::library::net::scsp::client::ethernet_type_t::tcp, false)
	, _front(front)
	, _video_codec(sirius::library::net::scsp::client::video_submedia_type_t::unknown)
	, _video_width(1280)
	, _video_height(720)
	, _video_fps(30)
	, _receive_option(sirius::library::net::scsp::client::media_type_t::unknown)
	, _play(false)
	, _rcv_first_video(false)
	, _state(sirius::library::net::scsp::client::state_t::disconnected)
	, _video_frame_size(0)
	, _video_frame_timestamp(0)
	, _video_frame_data(NULL)
{
	::InitializeCriticalSection(&_state_cs);

	add_command(new sirius::library::net::scsp::play_res_cmd(this));
	add_command(new sirius::library::net::scsp::video_stream_data_cmd(this));

	_s_video_interval = 0;
	_e_video_interval = 0;
}

sirius::library::net::scsp::client::core::~core(void)
{
	if (_video_frame_data)
		delete[] _video_frame_data;

	::DeleteCriticalSection(&_state_cs);
}

int32_t sirius::library::net::scsp::client::core::play(const char * url, int32_t port, int32_t recv_option, bool repeat)
{
	_receive_option = recv_option;
	_play = true;

	{
		sirius::autolock mutex(&_state_cs);
		if (_state != sirius::library::net::scsp::client::state_t::disconnected)
			return sirius::library::net::scsp::client::err_code_t::success;
	}

	_state = sirius::library::net::scsp::client::state_t::connecting;
	sirius::library::net::sicp::client::connect((char*)url, port, repeat);
	return _state;
}

void sirius::library::net::scsp::client::core::request_play(int32_t recv_option)
{
	if (recv_option & sirius::library::net::scsp::client::media_type_t::video)
	{
		if (_video_frame_data == NULL)
			_video_frame_data = new uint8_t[VIDEO_DATA_SIZE];
		request_play_video();
	}

	_state = sirius::library::net::scsp::client::state_t::streaming;
}

int32_t sirius::library::net::scsp::client::core::stop(void)
{
	_state = sirius::library::net::scsp::client::state_t::disconnecting;

	int32_t status = sirius::library::net::sicp::client::disconnect();
	_rcv_first_video = false;

	return status;
}

void sirius::library::net::scsp::client::core::create_session_callback(void)
{
	request_play(_receive_option);
	_state = sirius::library::net::scsp::client::state_t::connected;
}

void sirius::library::net::scsp::client::core::destroy_session_callback(void)
{
	_state = sirius::library::net::scsp::client::state_t::disconnected;
	if (_receive_option & sirius::library::net::scsp::client::media_type_t::video)
	{
		if (_front)
			_front->on_end_video();
	}
	_rcv_first_video = false;
}

void sirius::library::net::scsp::client::core::av_stream_callback(const char * msg, int32_t length)
{
	Json::Value packet;
	Json::Reader reader;
	reader.parse(msg, packet);

	int32_t res_code = sirius::library::net::scsp::client::err_code_t::fail;
	if (packet["rcode"].isInt())
		res_code = packet["rcode"].asInt();

	if (res_code == sirius::library::net::scsp::client::err_code_t::success)
	{
		int32_t stream_type = sirius::library::net::scsp::client::media_type_t::unknown;
		if (packet["type"].isInt())
			stream_type = packet.get("type", 0).asInt();

		if (stream_type == sirius::library::net::scsp::client::media_type_t::video)
		{
			if (packet["codec"].isInt())
				_video_codec = packet.get("codec", sirius::library::net::scsp::client::video_submedia_type_t::unknown).asInt();
			if (packet["video_width"].isInt())
				_video_width = packet.get("video_width", 1280).asInt();
			if (packet["video_height"].isInt())
				_video_height = packet.get("video_height", 720).asInt();
			if (packet["video_fps"].isInt())
				_video_fps = packet.get("video_fps", 30).asInt();

			if(_front)
				_front->on_recv_video_info(_video_codec, _video_width, _video_height, _video_fps);
		}

		_state = sirius::library::net::scsp::client::state_t::streaming;
	}
}

void sirius::library::net::scsp::client::core::request_play_video(void)
{
	_play = true;

	Json::Value packet;
	Json::StyledWriter writer;
	packet["type"] = sirius::base::media_type_t::video;
	std::string req_msg = writer.write(packet);
	data_request(SERVER_UUID, CMD_PLAY_REQ, (char*)req_msg.c_str(), req_msg.size() + 1);
}

void sirius::library::net::scsp::client::core::push_video_packet(sirius::library::net::scsp::cmd_stream_data_t * packet, uint8_t * data, int32_t length)
{
	{
		sirius::autolock mutex(&_state_cs);
		if (_state != sirius::library::net::scsp::client::state_t::streaming)
			return;
	}

	uint8_t es_count = packet->count;
	if (_s_video_interval == 0)
	{
		_s_video_interval = clock();
	}
	else
	{
		_e_video_interval = clock();
		_s_video_interval = _e_video_interval;
	}

	long long k = ntohl((u_long)packet->data.timestamp);
	for (int i = 0; i < es_count; i++)
	{
		uint32_t data_length = ntohl(packet->data.length);
		uint32_t data_index = ntohl(packet->data.index);
		uint64_t data_timestamp = ntohll(packet->data.timestamp);
		if (!data || data_length < 1)
			continue;

		if (_video_codec == sirius::library::net::scsp::client::video_submedia_type_t::png)
		{
			if (!_rcv_first_video)
			{
				_front->on_begin_video(_video_codec, data, data_length, data_timestamp, data_timestamp);
			}
			else
			{
				_front->on_recv_video(_video_codec, data, data_length, data_timestamp, data_timestamp);
			}
		}
	}
}
