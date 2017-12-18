#include <process.h>

#include "commands_payload.h"
#include "commands_client.h"
#include "casp_client.h"
#include "sirius_sicp_client.h"
#include <json/json.h>
#include "sirius_log4cplus_logger.h"

sirius::library::net::casp::client::core::core(sirius::library::net::casp::client * front)
	: sirius::library::net::sicp::client(MTU_SIZE, 1024 * 1024 * 2, 1024 * 1024 * 2, 1024 * 1024 * 4, COMMAND_THREAD_POOL_COUNT, IO_THREAD_POOL_COUNT, false, false, sirius::library::net::casp::client::ethernet_type_t::tcp, false)
	, _front(front)
	, _video_codec(sirius::library::net::casp::client::video_submedia_type_t::unknown)
	, _video_width(1280)
	, _video_height(720)
	, _video_fps(30)
	, _receive_option(sirius::library::net::casp::client::media_type_t::unknown)
	, _play(false)
	, _rcv_first_video(false)
	, _state(sirius::library::net::casp::client::state_t::disconnected)
	, _video_frame_size(0)
	, _video_frame_timestamp(0)
	, _video_frame_data(NULL)
{
	::InitializeCriticalSection(&_state_cs);

	add_command(new sirius::library::net::casp::play_res_cmd(this));
	add_command(new sirius::library::net::casp::video_stream_data_cmd(this));

	_s_video_interval = 0;
	_e_video_interval = 0;
}

sirius::library::net::casp::client::core::~core(void)
{
	if (_video_frame_data)
		delete[] _video_frame_data;

	::DeleteCriticalSection(&_state_cs);
}

int32_t sirius::library::net::casp::client::core::play(const char * url, int32_t port, int32_t recv_option, bool repeat)
{
	_receive_option = recv_option;
	_play = true;

	{
		sirius::autolock mutex(&_state_cs);
		if (_state != sirius::library::net::casp::client::state_t::disconnected)
			return sirius::library::net::casp::client::err_code_t::success;
	}

	_state = sirius::library::net::casp::client::state_t::connecting;
	sirius::library::net::sicp::client::connect((char*)url, port, repeat);
	return _state;
}

void sirius::library::net::casp::client::core::request_play(int32_t recv_option)
{
	if (recv_option & sirius::library::net::casp::client::media_type_t::video)
	{
		if (_video_frame_data == NULL)
			_video_frame_data = new uint8_t[VIDEO_DATA_SIZE];
		request_play_video();
	}

	_state = sirius::library::net::casp::client::state_t::streaming;
}

int32_t sirius::library::net::casp::client::core::stop(void)
{
	_state = sirius::library::net::casp::client::state_t::disconnecting;

	int32_t status = sirius::library::net::sicp::client::disconnect();
	_rcv_first_video = false;

	return status;
}

void sirius::library::net::casp::client::core::create_session_callback(void)
{
	request_play(_receive_option);
	_state = sirius::library::net::casp::client::state_t::connected;
}

void sirius::library::net::casp::client::core::destroy_session_callback(void)
{
	_state = sirius::library::net::casp::client::state_t::disconnected;
	if (_receive_option & sirius::library::net::casp::client::media_type_t::video)
	{
		if (_front)
			_front->on_end_video();
	}
	_rcv_first_video = false;
}

void sirius::library::net::casp::client::core::av_stream_callback(const char * msg, int32_t length)
{
	Json::Value packet;
	Json::Reader reader;
	reader.parse(msg, packet);

	int32_t res_code = sirius::library::net::casp::client::err_code_t::fail;
	if (packet["rcode"].isInt())
		res_code = packet["rcode"].asInt();

	if (res_code == sirius::library::net::casp::client::err_code_t::success)
	{
		int32_t stream_type = sirius::library::net::casp::client::media_type_t::unknown;
		if (packet["type"].isInt())
			stream_type = packet.get("type", 0).asInt();

		if (stream_type == sirius::library::net::casp::client::media_type_t::video)
		{
			if (packet["codec"].isInt())
				_video_codec = packet.get("codec", sirius::library::net::casp::client::video_submedia_type_t::unknown).asInt();
			if (packet["video_width"].isInt())
				_video_width = packet.get("video_width", 1280).asInt();
			if (packet["video_height"].isInt())
				_video_height = packet.get("video_height", 720).asInt();
			if (packet["video_fps"].isInt())
				_video_fps = packet.get("video_fps", 30).asInt();

			if (packet["video_block_width"].isInt())
				_video_block_width = packet.get("video_block_width", 128).asInt();
			if (packet["video_block_height"].isInt())
				_video_block_height = packet.get("video_block_height", 72).asInt();
		}
		_state = sirius::library::net::casp::client::state_t::streaming;
	}
}

void sirius::library::net::casp::client::core::request_play_video(void)
{
	_play = true;

	Json::Value packet;
	Json::StyledWriter writer;
	packet["type"] = sirius::base::media_type_t::video;
	std::string req_msg = writer.write(packet);
	data_request(SERVER_UUID, CMD_PLAY_REQ, (char*)req_msg.c_str(), req_msg.size() + 1);
}

void sirius::library::net::casp::client::core::push_video_packet(sirius::library::net::casp::cmd_stream_data_t * header, uint8_t * data, int32_t length)
{
	{
		sirius::autolock mutex(&_state_cs);
		if (_state != sirius::library::net::casp::client::state_t::streaming)
			return;
	}

	if (_s_video_interval == 0)
	{
		_s_video_interval = clock();
	}
	else
	{
		_e_video_interval = clock();
		_s_video_interval = _e_video_interval;
	}

	int32_t count = header->count;
	uint8_t * packet = data;
	
	if (count == 1)
	{
		sirius::library::net::casp::stream_packet_t single_packet_header;
		memmove(&single_packet_header, packet, sizeof(single_packet_header));

		int32_t index = ntohl(single_packet_header.stream_data.data.index);
		int32_t legnth = ntohl(single_packet_header.stream_data.data.length);
		packet += sizeof(single_packet_header);

		if (_video_codec == sirius::library::net::casp::client::video_submedia_type_t::png)
		{
			if (!_rcv_first_video)
			{
				_front->on_begin_video(_video_codec, _video_width, _video_height, _video_block_width, _video_block_height);
				_rcv_first_video = true;
			}
			_front->on_recv_video(_video_codec, packet, legnth, 0, 0);
		}
	}
	else
	{
		int32_t * mindex = new int32_t[count];
		uint8_t ** mdata = new uint8_t*[count];
		int32_t * mlength = new int32_t[count];

		for (int32_t x = 0; x < count; x++)
		{
			sirius::library::net::casp::stream_packet_t single_packet_header;
			memmove(&single_packet_header, packet, sizeof(single_packet_header));

			int32_t sindex = ntohl(single_packet_header.stream_data.data.index);
			int32_t slength = ntohl(single_packet_header.stream_data.data.length);
			packet += sizeof(single_packet_header);

			mindex[x] = sindex;
			mdata[x] = packet;
			mlength[x] = slength;
			packet += slength;
		};

		if (_video_codec == sirius::library::net::casp::client::video_submedia_type_t::png)
		{
			if (!_rcv_first_video)
			{
				_front->on_begin_video(_video_codec, _video_width, _video_height, _video_block_width, _video_block_height);
				_rcv_first_video = true;
			}

			_front->on_recv_video(_video_codec, count, mindex, mdata, mlength, 0, 0);
		}

		if (mindex)
			delete mindex;
		if (mdata)
			delete mdata;
		if (mlength)
			delete mlength;
	}
}