
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <crtdbg.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <conio.h>
#include <intrin.h>
#include <iphlpapi.h>
#include <iptypes.h>
#include <iprtrmib.h>
#include <tlhelp32.h>

#include "casp_server.h"
#include <commands_server.h>
#include <sirius_log4cplus_logger.h>
#include <json/json.h>
#include <sirius_locks.h>

#include <commands_payload.h>

#define ALIVE_CHECK_TIME			1000 * 10 
#define ARGUMENT_SIZE				1024

#define IO_THREAD_POOL_COUNT		4
#define COMMAND_THREAD_POOL_COUNT	2

//#define MTU_SIZE					1500
#define MTU_SIZE					0

sirius::library::net::casp::server::core::core(const char * uuid, sirius::library::net::casp::server * front)
	: sirius::library::net::sicp::server(uuid, MTU_SIZE, 1024*1024*2, 1024*1024*2, 1024 * 1024 * 4,IO_THREAD_POOL_COUNT, COMMAND_THREAD_POOL_COUNT, false, false)
	, _front(front)
	, _vsmt(sirius::library::net::casp::server::video_submedia_type_t::png)
	, _controller(nullptr)
	, _video_index(0)
	, _slot_number(0)
{ 

	allocate_send_buffer(MAX_STREAM_QUEUE);
	add_command(new sirius::library::net::casp::play_req_cmd(this));

#if defined(WITH_STREAMING_BLOCKING_MODE)
	_video_conf.send_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	_video_conf.thread_run = false;
	_video_conf.stream_type = sirius::library::net::casp::server::media_type_t::video;
	_video_conf.thread = INVALID_HANDLE_VALUE;

	_audio_conf.send_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	_audio_conf.thread_run = false;
	_audio_conf.stream_type = sirius::library::net::casp::server::media_type_t::audio;
	_audio_conf.thread = INVALID_HANDLE_VALUE;
#endif
	
	_mtu = 200000;
}

sirius::library::net::casp::server::core::~core(void)
{
	publish_end();	
}


int32_t sirius::library::net::casp::server::core::publish_begin(int32_t vsmt, int32_t asmt, const char * address, int32_t port_number, const char * uuid, sirius::library::net::casp::server::proxy * sc)
{
	_vsmt = vsmt;
	_asmt = asmt;
	_port_number = port_number;
	if (address && strlen(address)>1)
		strncpy_s(_address, address, sizeof(_address));
	if (uuid && strlen(uuid)>1)
		strncpy_s(_uuid, uuid, sizeof(_uuid));
	_controller = sc;

	bool status = sirius::library::net::sicp::server::start(nullptr, _port_number);
	_slot_number = _port_number - sirius::library::net::casp::server::port_number_base;


#if defined(WITH_STREAMING_BLOCKING_MODE)
	if (status)
	{
		_video_conf.thread_run = true;
		_video_conf.thread = (HANDLE)_beginthreadex(NULL, THREAD_STACK_SIZE, casp_server::process_cb, this, STACK_SIZE_PARAM_IS_A_RESERVATION, &_video_conf.thread_id);
		SetThreadPriority(_video_conf.thread, THREAD_PRIORITY_HIGHEST);

		_audio_conf.thread_run = true;
		_audio_conf.thread = (HANDLE)_beginthreadex(NULL, 0, casp_server::process_cb, this, 0, &_audio_conf.thread_id);
		//		SetThreadPriority(_audio_conf.thread, THREAD_PRIORITY_HIGHEST);
		return sirius::library::net::casp::server::err_code_t::success;
	}
	else
	{
		return sirius::library::net::casp::server::err_code_t::fail;
	}
#else
	return sirius::library::net::casp::server::err_code_t::success;
#endif
}

int32_t sirius::library::net::casp::server::core::deinit_stream(sirius::library::net::casp::server::core::stream_conf_t * stream_conf)
{
#if defined(WITH_STREAMING_BLOCKING_MODE)
	stream_conf->thread_run = false;
	if (stream_conf->send_event != INVALID_HANDLE_VALUE)
	{
		::SetEvent(stream_conf->send_event);
	}

	if (stream_conf->thread != INVALID_HANDLE_VALUE)
	{
		::WaitForSingleObject(stream_conf->thread, INFINITE);
		::CloseHandle(stream_conf->thread);
		stream_conf->thread = INVALID_HANDLE_VALUE;
	}

	if (stream_conf->send_event != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(stream_conf->send_event);
		stream_conf->send_event = INVALID_HANDLE_VALUE;
	}

	std::vector<SOCKET>::iterator fd_iter;
	for (fd_iter = stream_conf->fds.begin(); fd_iter != stream_conf->fds.end(); )
	{
		stream_conf->fds.erase(fd_iter);
                ++fd_iter;
		//SOCKET fd = *fd_iter;
		//if (fd != INVALID_SOCKET)
		//{
		//	closesocket(fd);
		//	fd = INVALID_SOCKET;
		//}
	}
#endif
	return sirius::base::err_code_t::success;
}

int32_t sirius::library::net::casp::server::core::publish_end(void)
{
	deinit_stream(&_video_conf);

	release_send_buffer();

	bool status = sirius::library::net::sicp::server::stop();
	
	if (status)
	{
		status = sirius::base::err_code_t::success;
		LOGGER::make_info_log(SLNS, "%s, %d publish_end success", __FUNCTION__, __LINE__);
	}
	else 
	{
		status = sirius::base::err_code_t::fail;
		LOGGER::make_error_log(SLNS, "%s, %d publish_end fail, SlotNum=%d", __FUNCTION__, __LINE__, _slot_number);
	}

	return status;
}

int32_t sirius::library::net::casp::server::core::publish_video(uint8_t * bitstream, size_t nb, long long timestamp)
{
	int32_t res_code = sirius::library::net::casp::server::err_code_t::success;
	if (state_start(sirius::library::net::casp::server::media_type_t::video) != sirius::library::net::casp::server::stream_state_t::state_stopped)
	{
		if (state_start(sirius::library::net::casp::server::media_type_t::video) == sirius::library::net::casp::server::stream_state_t::state_begin_publishing)
			_video_conf.state = sirius::library::net::casp::server::stream_state_t::state_publishing;

		if ((_video_conf._send_queue.get_count() + 1) >= MAX_STREAM_QUEUE)
		{
			LOGGER::make_warn_log(SLNS, "%s()_%d : _video_conf._send_queue.get_count()=%d", __FUNCTION__, __LINE__, _video_conf._send_queue.get_count());
			for (int i = 0; i < 1000; i++)
			{
				if ((_video_conf._send_queue.get_count() + 1) >= MAX_STREAM_QUEUE)
				{
					::Sleep(1);
				}
				else
				{
					break;
				}
			}
			LOGGER::make_warn_log(SLNS, "%s()_%d : _video_conf._send_queue.get_count()=%d", __FUNCTION__, __LINE__, _video_conf._send_queue.get_count());
		}

#if defined(WITH_STREAMING_BLOCKING_MODE)
		if (_video_conf.fds.size() > 0)
		{
			int32_t result = sirius::base::err_code_t::success;
			result = set_stream_buffer(sirius::library::net::casp::server::media_type_t::video, bitstream, nb);
			::SetEvent(_video_conf.send_event);
			/*
			if (result == sirius::base::err_code_t::fail)
				LOGGER::make_error_log(SLNS, "%s()_%d :set_stream_buffer uuid=%s, casp_stream_type=%d", __FUNCTION__, __LINE__, _video_conf.uuid, _video_conf.stream_type);
			*/
		}
#else
		if (_video_conf.uuids.size() > 0)
		{
			sirius::autolock mutex(&_video_conf.cs);

			std::vector<std::string>::iterator dst_uuid_iter;
			for (dst_uuid_iter = _video_conf.uuids.begin(); dst_uuid_iter != _video_conf.uuids.end(); dst_uuid_iter++)
			{
				std::string uuid = *dst_uuid_iter;
				data_request((char*)uuid.c_str(), CMD_VIDEO_STREAM_DATA, reinterpret_cast<char*>(bitstream), nb);
				if(_video_index <= 4)
					LOGGER::make_info_log(SLNS, "%s()_%d Video Data Request (video_index:%d)", __FUNCTION__, __LINE__, _video_index);
			}
		}
#endif
		else
		{
			_video_conf.state = sirius::library::net::casp::server::stream_state_t::state_stopped;
			LOGGER::make_info_log(SLNS, "%s()_%d video_conf(state_stopped) ", __FUNCTION__, __LINE__);
		}
	}
	else
	{
		res_code = sirius::library::net::casp::server::err_code_t::not_implemented;
		_video_conf._send_queue.pop_pending();
	}
	_video_index++;
	return res_code;
}

int32_t sirius::library::net::casp::server::core::play(int32_t flags)
{
	if (_controller)
	{
		LOGGER::make_trace_log(SLNS, "%s()_%d : flags=%d", __FUNCTION__, __LINE__, flags);
		return _controller->play(flags);
	}
	return sirius::library::net::casp::server::err_code_t::fail;
}

int32_t sirius::library::net::casp::server::core::pause(int32_t flags)
{
	if (_controller)
		return _controller->pause(flags);
	return sirius::library::net::casp::server::err_code_t::fail;
}

int32_t sirius::library::net::casp::server::core::stop(int32_t flags)
{
	if (_controller)
		return _controller->stop(flags);
	return sirius::library::net::casp::server::err_code_t::fail;
}

void sirius::library::net::casp::server::core::create_session_callback(const char * uuid)
{
	LOGGER::make_trace_log(SLNS, "cloud.app.platform.caspstream", "%s(), connect [src=%s]" , __FUNCTION__, uuid);
}

void sirius::library::net::casp::server::core::destroy_session_callback(const char * uuid)
{	
	LOGGER::make_trace_log(SLNS, "cloud.app.platform.caspstream", "%s(), disconnect [src=%s]", __FUNCTION__, uuid);

	std::vector<std::string>::iterator dst_uuid_iter;
	{
		sirius::autolock mutex(&_video_conf.cs);

		dst_uuid_iter = std::find(_video_conf.uuids.begin(), _video_conf.uuids.end(), uuid);
		if (dst_uuid_iter != _video_conf.uuids.end())
		{
			_video_conf.uuids.erase(dst_uuid_iter);
			if (_video_conf.uuids.size() < 1)
			{
				LOGGER::make_info_log(SLNS, "%s()_%d", __FUNCTION__, __LINE__);
				_video_conf.state = sirius::library::net::casp::server::stream_state_t::state_stopped;
			}
		}
	}
}

int32_t sirius::library::net::casp::server::core::client_request_stream_res(const char * client_uuid, const char * msg, int length, SOCKET clientsocket)
{	
	char str_ptr[MAX_PATH] = { 0, };
	sprintf_s(str_ptr, "aa:bb:cc");
	sirius::library::log::log4cplus::logger::streamer_log_init(str_ptr, SLNS);

	int32_t res_code = sirius::base::err_code_t::fail;
	sirius::library::net::casp::server::context_t	context;
	LOGGER::make_trace_log(SLNS, "%s(), %d : client_uuid=%s, clientsocket=%d, msg=%s, length=%d", __FUNCTION__, __LINE__, client_uuid, clientsocket, msg, length);
	
	Json::Value root;
	Json::Reader reader;
	Json::StyledWriter writer;
	std::string control_uuid;

	reader.parse(msg, root);

	int32_t stream_type = 0;
	int32_t stream_subtype = 0;
	if(root["type"].isInt())	
		stream_type = root.get("type", 0).asInt();
	
	LOGGER::make_info_log(SLNS, "[Stream Request] - %s(), %d,	Command:%d, type:%d", __FUNCTION__, __LINE__, CMD_PLAY_REQ, stream_type);
	control_uuid = root.get("attendant_uuid", _uuid).asString();
	root.clear();
	if ( (control_uuid.compare(_uuid) == 0) && (stream_type == sirius::library::net::casp::server::media_type_t::video))
	{
		if (stream_type == sirius::library::net::casp::server::media_type_t::video)
		{
			stream_subtype = _vsmt;
			root["type"] = stream_type;
			root["codec"] = stream_subtype;
			root["video_width"] = context.video_width;
			root["video_height"] = context.video_height;
			root["video_fps"] = context.video_fps;
			root["video_block_width"] = context.video_block_width;
			root["video_block_height"] = context.video_block_height;
		}
		
		res_code = sirius::base::err_code_t::success;
	}
	else
	{
		if (!(stream_type == sirius::library::net::casp::server::media_type_t::video))
		{
			res_code = sirius::base::err_code_t::fail + 1;
			LOGGER::make_error_log(SLNS, "%s(),%d, streamType does not match SlotNum=%d", __FUNCTION__, __LINE__, _slot_number);
		}
		else
		{
			res_code = sirius::base::err_code_t::fail;
			LOGGER::make_error_log(SLNS, "%s(),%d, uuid does not match, SlotNum=%d", __FUNCTION__, __LINE__, _slot_number);
		}	
	}

	root["rcode"] = res_code;
	std::string res_json = writer.write(root);

	LOGGER::make_trace_log(SLNS, "%s(), %d : client uuid=%s, client uuid=%s, msg=%s, length=%d", __FUNCTION__, __LINE__, client_uuid, control_uuid.c_str(), res_json.c_str(), res_json.size());

	data_request((char*)client_uuid, CMD_PLAY_RES, (char*)res_json.c_str(), res_json.size());
	LOGGER::make_info_log(SLNS, "[Stream Response] - %s(), %d, Command:%d, type:%d, codec:%d, video_width:%d, video_height:%d, video_fps:%d, video_block_width:%d, video_block_height:%d, rcode:%d",
		__FUNCTION__, __LINE__, CMD_PLAY_RES, stream_type, stream_subtype, context.video_width, context.video_height, context.video_fps, context.video_block_width, context.video_block_height, res_code);
	if (res_code == sirius::base::err_code_t::success)
	{
		if ((stream_type == sirius::library::net::casp::server::media_type_t::video))
		{
			{
				sirius::autolock mutex(&_video_conf.cs);
#if defined(WITH_STREAMING_BLOCKING_MODE)
				_video_conf.fds.push_back(clientsocket);
#endif
				_video_conf.uuids.push_back(client_uuid);
				if (_video_conf.state != sirius::library::net::casp::server::stream_state_t::state_begin_publishing)
				{
					_video_conf.state = sirius::library::net::casp::server::stream_state_t::state_begin_publishing;
					play(sirius::library::net::casp::server::media_type_t::video);
				}
			}

		}
		else
		{
			LOGGER::make_error_log("cloud.app.platform.caspstream", "%s(),%d : client uuid = %s msg=%s, res_msg=%s", __FUNCTION__, __LINE__, client_uuid, msg, res_json.c_str());
			res_code = -1;
		}
	}
	else
	{
		LOGGER::make_error_log("cloud.app.platform.caspstream", "%s(),%d : client_uuid=%s, slot_uuid=%s, msg=%s, res_msg=%s", __FUNCTION__, __LINE__, client_uuid, _uuid, msg, res_json.c_str());
	}
	LOGGER::make_info_log(SLNS, "%s()_%d _video_conf.state:%d", __FUNCTION__, __LINE__, _video_conf.state);
	return res_code;
}

int32_t sirius::library::net::casp::server::core::state_start(int32_t type)
{
	int32_t stat;
	if (type == sirius::library::net::casp::server::media_type_t::video)
		stat = _video_conf.state;

	return stat;
}

#if defined(WITH_STREAMING_BLOCKING_MODE)
unsigned __stdcall casp_server::process_cb(void * param)
{
	casp_server * self = static_cast<casp_server*>(param);
	self->process();
	return 0;
}

int32_t casp_server::send_packet(SOCKET fd, uint8_t * data, size_t data_length)
{
	int32_t number_of_byte = 0;
	int32_t retry = 0;

	int32_t pkt_payload_size = _mtu;
	int32_t pkt_totalsize = data_length;
	int32_t pkt_count = pkt_totalsize / _mtu;
	int32_t last_pkt_size = pkt_totalsize % _mtu;
	int32_t packet_length = 0;
	size_t pos = 0;

	if (last_pkt_size>0)
		pkt_count++;
	if (pkt_count<1)
		return sirius::base::err_code_t::fail;
	
	ic::stream_packet_t *packet = (ic::stream_packet_t *)data;

//	LOGGER::make_info_log(SLNS, "%s()_%d : pkt_count=%d, pkt_totalsize=%d, last_pkt_size=%d", __FUNCTION__, __LINE__, pkt_count, pkt_totalsize, last_pkt_size);
//	LOGGER::make_info_log(SLNS, "%s()_%d : command=%d, index=%d, pk_len=%d, con_len=%d, _mtu=%d", 
//		__FUNCTION__, __LINE__, ntohs(packet->header.command), ntohl(packet->stream_data.data.index), ntohl(packet->header.length), ntohl(packet->stream_data.data.length), _mtu);

	for (int i = 0; i<pkt_count; i++)
	{
		if (i == (pkt_count - 1))
		{
			pkt_payload_size = last_pkt_size;
			if (pkt_count == 1)	// 한번에 다 보낼때
			{
				packet_length = pkt_totalsize;
				pos = 0;
			}
			else	// 마지막 packet
			{
				if (last_pkt_size == 0)				
					packet_length = _mtu;
				else 
					packet_length = last_pkt_size;
			}
		}
		else
		{
			if (i == 0)	// 여러개의 packet 으로 보낼때 첫번째
			{
				pos = 0;
			}
			packet_length = _mtu;
		}
			
		retry = 0;
		do
		{
			int32_t last_error;
			number_of_byte = send(fd, reinterpret_cast<char*>(data+pos), packet_length, 0);
			//LOGGER::make_error_log(SLNS, "%s()_%d : pos=%d, number_of_byte=%d, packet_length=%d", __FUNCTION__, __LINE__, pos, number_of_byte, packet_length);
			if (number_of_byte != packet_length)
			{
				LOGGER::make_error_log(SLNS, "%s()_%d : pos=%d, number_of_byte=%d, packet_length=%d", __FUNCTION__, __LINE__, pos, number_of_byte, packet_length);
			}
			if (number_of_byte == SOCKET_ERROR)
			{
				last_error = WSAGetLastError();
				if (last_error != WSAEWOULDBLOCK)
				{
					LOGGER::make_info_log(SLNS, "%s()_%d : socket=%d, WSAGetLastError = %d", __FUNCTION__, __LINE__, fd, last_error);
					return sirius::base::err_code_t::fail;
				}
				else
				{
					LOGGER::make_info_log(SLNS, "%s()_%d : WSAEWOULDBLOCK pos=%d, packet_length=%d", __FUNCTION__, __LINE__, pos, packet_length);
					retry++;
					Sleep(1);
				}
			}
			else
			{
				retry = 0;
				//pos += packet_length;
				pos += number_of_byte;
			}
		} while ((retry < 10000) && (retry != 0));
	}
	return sirius::base::err_code_t::success;
}


void casp_server::process(void)
{
	casp_server::stream_conf_t * stream_conf = nullptr;
	size_t packet_length=0;

	uint32_t thread_id = GetCurrentThreadId();

	if (thread_id == _video_conf.thread_id)
		stream_conf = &_video_conf;
	else if (thread_id == _audio_conf.thread_id)
		stream_conf = &_audio_conf;
	else
		return;

	while (stream_conf->thread_run)
	{
		DWORD result = ::WaitForSingleObject(stream_conf->send_event, INFINITE);
		if (result == WAIT_FAILED)
		{
			LOGGER::make_info_log(SLNS, "%s()_%d : stream_type(%d), WAIT_FAILED ", __FUNCTION__, __LINE__, stream_conf->stream_type);
			//stream_conf->fd = INVALID_SOCKET;
			break;
		}
		else if (result == WAIT_ABANDONED)
		{
			LOGGER::make_info_log(SLNS, "%s()_%d : stream_type(%d), WAIT_ABANDONED ", __FUNCTION__, __LINE__, stream_conf->stream_type);
			continue;
		}
		else if (result == WAIT_TIMEOUT)
		{
			//LOGGER::make_info_log(SLNS, "%s()_%d : stream_type(%d), WAIT_TIMEOUT ", __FUNCTION__, __LINE__, stream_conf->stream_type);
		}
		//else
		//{
		//	stream_conf->thread_run = false;
		//	//stream_conf->fd = INVALID_SOCKET;
		//	break;
		//}

		for (uint32_t i=0;i < stream_conf->_send_queue.get_count();i++)
		{
			send_buffer_t * send_buffer = nullptr;
			send_buffer = stream_conf->_send_queue.pop_pending();
			if (send_buffer == nullptr)
				break;

			sirius::autolock mutex(&stream_conf->cs);
			std::vector<SOCKET>::iterator fd_iter;
			for (fd_iter = stream_conf->fds.begin(); fd_iter != stream_conf->fds.end(); fd_iter++)
			{
				SOCKET fd = *fd_iter;
				result = send_packet(fd, send_buffer->packet, send_buffer->size);
				if (result != sirius::base::err_code_t::success)
				{
					closesocket(fd);
					fd = INVALID_SOCKET;
					::CloseHandle(stream_conf->send_event);
					stream_conf->send_event = INVALID_HANDLE_VALUE;
				}
			}
		}
	}
}
#endif

void sirius::library::net::casp::server::core::allocate_send_buffer(int32_t size)
{
	_video_conf._send_queue_size = size;
	_video_conf._send_queue.initialize(_video_conf._send_buffers, _video_conf._send_queue_size);

	for (uint32_t i = 0; i < _video_conf._send_queue_size; i++)
	{
		_video_conf._send_buffers[i].size = 0;
		_video_conf._send_buffers[i].packet = new uint8_t[MAX_VIDEO_ES_SIZE];
	}
}

void sirius::library::net::casp::server::core::release_send_buffer(void)
{
	for (uint32_t i = 0; i < _video_conf._send_queue_size; i++)
	{
		if (_video_conf._send_buffers[i].packet)
		{
			delete [] _video_conf._send_buffers[i].packet;
			_video_conf._send_buffers[i].packet = nullptr;
			_video_conf._send_buffers[i].size = 0;
		}
	}
	_video_conf._send_queue_size = 0;
}

sirius::library::net::casp::server::core::send_buffer_t* sirius::library::net::casp::server::core::get_stream_buffer(int32_t stream_type)
{
	if (stream_type == sirius::library::net::casp::server::media_type_t::video)
	{
		send_buffer_t * send_buffer = nullptr;

		send_buffer = _video_conf._send_queue.get_available();
		if (send_buffer == nullptr)
		{
			LOGGER::make_error_log(SLNS, "%s(), %d : stream queue not avaliable, SlotNum=%d", __FUNCTION__, __LINE__, _slot_number);
		}
		return send_buffer;
	}
}

int32_t sirius::library::net::casp::server::core::set_stream_buffer(int32_t stream_type, uint8_t *bitstream, size_t size)
{
	int32_t status = sirius::base::err_code_t::success;
	if (stream_type == sirius::library::net::casp::server::media_type_t::video)
	{
		_video_conf._send_queue.set_available(bitstream, size);

	}
	else
	{
		status = sirius::base::err_code_t::fail;
	}
	return status;
}

template<class T>
sirius::library::net::casp::server::core::stream_queue<T>::stream_queue(void)
	: _buffer(NULL)
	, _size(0)
	, _pending_count(0)
	, _available_index(0)
	, _pending_index(0)
{}

template<class T>
sirius::library::net::casp::server::core::stream_queue<T>::~stream_queue(void)
{
	delete[] _buffer;
}

template<class T>
bool sirius::library::net::casp::server::core::stream_queue<T>::initialize(T *pItems, uint32_t size)
{
	_size = size;
	_pending_count = 0;
	_available_index = 0;
	_pending_index = 0;

	_buffer = new T *[_size];
	for (uint32_t i = 0; i < _size; i++)
	{
		_buffer[i] = &pItems[i];
	}

	return true;
}
template<class T>
T * sirius::library::net::casp::server::core::stream_queue<T>::set_available(uint8_t *bitstream, size_t size)
{
	T *pItem = nullptr;
	sirius::library::net::casp::server::core::send_buffer_t *buffer = nullptr;
	if (_pending_count == _size)
	{
		LOGGER::make_error_log(SLNS, "%s(), %d : Max stream buffer available_index=%d, pending_index=%d, pending_count=%d, ", __FUNCTION__, __LINE__, _available_index, _pending_index, _pending_count);
		_pending_count = 0;
		_available_index = 0;
		_pending_index = 0;
	}
	pItem = _buffer[_available_index];
	buffer = pItem;

	memcpy(buffer->packet, bitstream, size);
	buffer->size = size;

	_available_index = (_available_index + 1) % _size;
	_pending_count += 1;

	return pItem;
}

template<class T>
T * sirius::library::net::casp::server::core::stream_queue<T>::get_available(void)
{
	T *pItem = nullptr;
	if (_pending_count == _size)
	{
		LOGGER::make_error_log(SLNS, "%s(), %d : Max stream buffer available_index=%d, pending_index=%d, pending_count=%d, ", __FUNCTION__, __LINE__, _available_index, _pending_index, _pending_count);
		return nullptr;
	}
	pItem = _buffer[_available_index];
	_available_index = (_available_index + 1) % _size;
	_pending_count += 1;

	return pItem;
}

template<class T>
T* sirius::library::net::casp::server::core::stream_queue<T>::get_pending(void)
{
	if (_pending_count == 0)
	{
		return nullptr;
	}

	T *pItem = _buffer[_pending_index];

	return pItem;
}

template<class T>
T* sirius::library::net::casp::server::core::stream_queue<T>::pop_pending(void)
{
	if (_pending_count == 0)
	{
		return nullptr;
	}

	T *pItem = _buffer[_pending_index];
	_pending_index = (_pending_index + 1) % _size;
	_pending_count -= 1;
	//LOGGER::make_trace_log(SLNS, "%s(), %d : _available_index=%d, _pending_count=%d, ", __FUNCTION__, __LINE__, _available_index, _pending_count);
	return pItem;
}

template<class T>
uint32_t sirius::library::net::casp::server::core::stream_queue<T>::get_count(void)
{
	return _pending_count;
}