//#include "sirius_casp_server.h"
//#include "casp_server.h"
//#include "sirius_stringhelper.h"
//#include "sirius_log4cplus_logger.h"
//#include "sirius_locks.h"

#include <platform.h>
#include "sirius_casp_server.h"
#include "casp_server.h"
#include <sirius_stringhelper.h>
#include <sirius_log4cplus_logger.h>
#include <sirius_locks.h>
#include <commands_server.h>

#define IS_NOT_NULL( x )        ( x != NULL )
#define SAFE_DELETE_POINTS( x )	if( IS_NOT_NULL( x ) ){ delete [] x; x = NULL; }
sirius::library::net::casp::server::server(void)
	: _frame_index_video(0)
	, _video_data(nullptr)
	, _core(nullptr)
	, _audio_configstr_size(0)
{
#ifdef ES_WRITE
	_pFile = ::CreateFileA("00_server_after_encode.video", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#endif
#ifdef CELT_WRITE
	_pFile = ::CreateFileA("00_celt_data.celt", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#endif
	_video_data = new uint8_t[MAX_VIDEO_ES_SIZE];

	_core = new sirius::library::net::casp::server::core(SERVER_UUID, this);
}

sirius::library::net::casp::server::~server(void)
{
#ifdef ES_WRITE
	if (_pFile != NULL && _pFile != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(_pFile);
		_pFile = INVALID_HANDLE_VALUE;
	}
#endif
#ifdef CELT_WRITE
	if (_pFile != NULL && _pFile != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(_pFile);
		_pFile = INVALID_HANDLE_VALUE;
	}
#endif
	if (_core)
	{
		_core->publish_end();
		delete _core;
		_core = nullptr;
	}

	SAFE_DELETE_POINTS(_video_data);
}

int32_t sirius::library::net::casp::server::publish_begin(int32_t vsmt, int32_t asmt, const char * address, int32_t slot_number, const char * uuid, proxy * sc)
{
#ifndef WITH_CASP
	if (!address || strlen(address) < 1)
		return sirius::library::net::casp::server::err_code_t::fail;
#endif

	_vsmt = vsmt;
	_asmt = asmt;
	_slot_number = slot_number;
	_port_number = _slot_number /*+ port_number_base*/;
	if (address && strlen(address)>1)
		strncpy_s(_address, address, sizeof(_address));
	if (uuid && strlen(uuid)>1)
		strncpy_s(_uuid, uuid, sizeof(_uuid));
	_controller = sc;

	int32_t status = sirius::library::net::casp::server::err_code_t::success;
	if (_video_data == nullptr)
		_video_data = new uint8_t[MAX_VIDEO_ES_SIZE];

	LOGGER::make_trace_log(SLNS, "%s(), %d : vsmt=%d, asmt=%d, address=%s, slot_number=%d, uuid=%s", __FUNCTION__, __LINE__, _vsmt, _asmt, _address, _port_number, _uuid);

	_core->publish_end();
	_core->publish_begin(_vsmt, _asmt, address, _port_number, _uuid, _controller);

	LOGGER::make_info_log(SLNSC, "%s, %d port_number=%d, device_id=%s, slot_number=%d", __FUNCTION__, __LINE__, _port_number, LOGGER::get_device_id(), _slot_number);

	return sirius::library::net::casp::server::err_code_t::success;
}

int32_t sirius::library::net::casp::server::publish_end(void)
{
	int32_t status = sirius::library::net::casp::server::err_code_t::success;

	status = _core->publish_end();

	return status;
}

int32_t sirius::library::net::casp::server::publish_video(uint8_t * vps, size_t vps_size, uint8_t * sps, size_t sps_size, uint8_t * pps, size_t pps_size, uint8_t * bitstream, size_t nb, long long timestamp)
{
	const uint8_t contents_count = 1;
	int32_t data_pos = 0;

	if (MAX_VIDEO_ES_SIZE < nb)
		LOGGER::make_error_log(SLNS, "%s(), %d [MINOR] Streaming error(error_code:%d) MAX_VIDEO_ES_SIZE_OVER MAX=%d, size=%d, SlotNum=%d", __FUNCTION__, __LINE__, sirius::library::net::casp::server::err_code_t::max_es_size_over, MAX_VIDEO_ES_SIZE, nb, _slot_number);

	//if (_frame_index_video > 2)
	{
		if (_core != nullptr)
		{
			if ((_video_data != nullptr) && (_core->state_start(sirius::library::net::casp::server::server::media_type_t::video) != sirius::library::net::casp::server::stream_state_t::state_stopped))
			{
				int32_t stream_type = sirius::library::net::casp::server::media_type_t::video;
				data_pos = set_casp_payload(_video_data, stream_type, data_pos, contents_count, vps_size + sps_size + pps_size, nb, timestamp);
				data_pos = set_sps_pps(data_pos, vps, vps_size, sps, sps_size, pps, pps_size);
				data_pos = set_es_stream(_video_data, stream_type, data_pos, bitstream, nb);
				if (_core)
				{
					_core->publish_video(_video_data, data_pos, timestamp);
					_network_usage.video_transferred_bytes += data_pos;
				}
#ifdef ES_WRITE
				do
				{

					uint32_t bytes2write = data_pos - 17;
					uint32_t bytes_written = 0;
					uint32_t nb_write = 0;
					::WriteFile(_pFile, _video_data + 17, bytes2write, (LPDWORD)&nb_write, 0);
					bytes_written += nb_write;
					if (bytes2write == bytes_written)
						break;
				} while (1);
#endif
			}
		}
	}
	_frame_index_video++;
	return 0;
}

int32_t sirius::library::net::casp::server::publish_video(int32_t count, int32_t * index, uint8_t ** compressed, int32_t * size, long long timestamp)
{
	const uint8_t contents_count = count;
	int32_t data_pos = 0;
	if ((_video_data != nullptr) && (_core->state_start(sirius::library::net::casp::server::server::media_type_t::video) != sirius::library::net::casp::server::stream_state_t::state_stopped))
	{
		uint8_t * video_data = _video_data;
		int32_t stream_type = sirius::library::net::casp::server::media_type_t::video;
		
		for (int32_t x = 0; x < count; x++)
		{
			sirius::library::net::casp::stream_packet_t single_packet_header;
			single_packet_header.stream_data.count = count;
			single_packet_header.stream_data.data.length = htonl(size[x]);
			single_packet_header.stream_data.data.index = htonl(_frame_index_video);
			single_packet_header.stream_data.data.timestamp = htonll(timestamp);
			memmove(video_data, &single_packet_header, sizeof(single_packet_header));
			video_data += sizeof(sirius::library::net::casp::stream_packet_t);

			memmove(video_data, compressed[x], size[x]);
			video_data += size[x];
			_frame_index_video++;
		}
		if (_core)
		{
			_core->publish_video(_video_data, video_data - _video_data, timestamp);
			_network_usage.video_transferred_bytes += (video_data - _video_data);
		}
	}
	return 0;
}

int32_t sirius::library::net::casp::server::set_casp_payload(uint8_t * data, uint8_t stream_type, int32_t & data_pos, uint8_t contents_count, size_t es_header_size, size_t nb, long long timestamp)
{
	uint32_t max_data_size, frame_index;
#if defined(STREAMING_BLOCKING_MODE)
	ic::stream_packet_t stream_packet;
	int32_t packet_header_size = sizeof(ic::casp_packet_header_t);
#endif

	if (stream_type == sirius::library::net::casp::server::media_type_t::video)
	{
		max_data_size = MAX_VIDEO_ES_SIZE;
		frame_index = _frame_index_video;
#if defined(STREAMING_BLOCKING_MODE)
		stream_packet.header.command = CMD_SC_VIDEO_DATA;
#endif
	}
	else
		return sirius::library::net::casp::server::err_code_t::fail;

#if defined(STREAMING_BLOCKING_MODE)
	stream_packet.header.length = sizeof(ic::stream_packet_t) - 1 + es_header_size + nb;
#endif

	sirius::library::net::casp::stream_packet_t* packet = (sirius::library::net::casp::stream_packet_t*)data;

#if defined(STREAMING_BLOCKING_MODE)
	//바이트 오더 변경. 
	packet->header.command = htons(stream_packet.header.command);
	packet->header.length = htonl(stream_packet.header.length);
	packet->header.version = 0x00;
	packet->header.type = 0x00;
#endif

	packet->stream_data.count = contents_count;
	packet->stream_data.data.length = htonl(nb + es_header_size);
	packet->stream_data.data.index = htonl(frame_index);
	packet->stream_data.data.timestamp = htonll(timestamp);
	data_pos += sizeof(sirius::library::net::casp::stream_packet_t);
	return data_pos;
}

int32_t sirius::library::net::casp::server::set_sps_pps(int32_t data_pos, uint8_t * vps, size_t vps_size, uint8_t * sps, size_t sps_size, uint8_t * pps, size_t pps_size)
{
	if ((vps_size > 0) && (vps != nullptr))
	{
		//LOGGER::make_trace_log(CAPS, "%s(), %d : vps_size=%d", __FUNCTION__, __LINE__, vps_size);
		memcpy(_video_data + data_pos, vps, vps_size);
		data_pos += vps_size;
	}
	if ((sps_size > 0) && (sps != nullptr))
	{
		//LOGGER::make_trace_log(CAPS, "%s(), %d : sps_size=%d", __FUNCTION__, __LINE__, sps_size);
		memcpy(_video_data + data_pos, sps, sps_size);
		data_pos += sps_size;
	}
	if ((pps_size > 0) && (pps != nullptr))
	{
		//LOGGER::make_trace_log(CAPS, "%s(), %d : pps_size=%d", __FUNCTION__, __LINE__, pps_size);
		memcpy(_video_data + data_pos, pps, pps_size);
		data_pos += pps_size;
	}

	return data_pos;
}

int32_t sirius::library::net::casp::server::set_es_stream(uint8_t *data, int32_t stream_type, int32_t data_pos, uint8_t * bitstream, size_t nb)
{
	if (stream_type == sirius::library::net::casp::server::media_type_t::video)
		memcpy(data + data_pos, bitstream, nb);
	else if (stream_type == sirius::library::net::casp::server::media_type_t::audio)
		memcpy(data + data_pos, bitstream, nb);
	else
		return sirius::library::net::casp::server::err_code_t::fail;

	data_pos += nb;
	return data_pos;
}