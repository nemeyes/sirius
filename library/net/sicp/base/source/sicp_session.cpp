#include <sicp_session.h>
#if defined(WITH_WORKING_AS_SERVER)
#include <sicp_server.h>
#else
#include <sicp_client.h>
#endif

#include <sicp_packet.h>

#include <sirius_uuid.h>
#include <sirius_locks.h>
#include <sirius_log4cplus_logger.h>

sirius::library::net::sicp::session::session(sirius::library::net::sicp::processor * prcsr, SOCKET fd, int32_t mtu, int32_t recv_buffer_size, bool dynamic_alloc)
	: sirius::library::net::session(fd, mtu,recv_buffer_size, dynamic_alloc)
	, _processor(prcsr)
	, _heart_beat(0)
	, _hb_retry_count(0)

{
	memset(_ip, 0x00, sizeof(_ip));
	memset(_uuid, 0x00, sizeof(_uuid));
	memset(_name, 0x00, sizeof(_name));

	update_hb_start_time();
	update_hb_end_time();
}

sirius::library::net::sicp::session::~session(void)
{

}

void sirius::library::net::sicp::session::push_send_packet(const char * dst, const char * src, int32_t cmd, char * msg, int32_t length)
{
	if (fd() == INVALID_SOCKET)
	{
		//LOGGER::make_trace_log(CAPS, "%s()_%d %d : already close %s ", __FUNCTION__, __LINE__, GetCurrentThreadId(), _uuid);
		return;
	}

	if (_mtu == 0)
	{
		char * pkt_payload = msg;
		int32_t pkt_header_size = sizeof(sirius::library::net::sicp::packet_header_t);
		int32_t pkt_payload_size = length;

		sirius::uuid src_uuid = std::string(src);
		sirius::uuid dst_uuid = std::string(dst);

		std::vector<std::shared_ptr<sirius::library::net::sicp::session::send_buffer_t>> send_queue;
		std::shared_ptr<sirius::library::net::sicp::session::send_buffer_t> send_buffer;
		if (_dynamic_alloc)
			send_buffer = std::shared_ptr<sirius::library::net::sicp::session::send_buffer_t>(new sirius::library::net::sicp::session::send_buffer_t(pkt_header_size + pkt_payload_size));
		else
		{
			bool empty = true;
			for (int32_t index = 0; index < 100; index++)
			{
				{
					sirius::autolock mutex(&_ready_send_cs_lock);
					empty = _send_ready_queue.empty();
				}

				if (!empty)
				{
					{
						sirius::autolock mutex(&_ready_send_cs_lock);
						send_buffer = _send_ready_queue.front();
						if (send_buffer)
						{
							_send_ready_queue.pop_front();
							if ((pkt_header_size + pkt_payload_size) > send_buffer->packet_size)
								send_buffer->resize(pkt_header_size + pkt_payload_size);
							break;
						}
					}
					::Sleep(10);
				}
				else
				{
					::Sleep(10);
				}
			}
		}

		if (send_buffer)
		{
#if defined(WITH_NEW_PROTOCOL)
			sirius::library::net::sicp::packet_header_t header;
			header.pid = 'S';
			memcpy(header.dst, (void *)&dst_uuid.hton(), sizeof(header.dst));
			memcpy(header.src, (void *)&src_uuid.hton(), sizeof(header.src));
			header.version = sirius::library::net::sicp::session::protocol_version;
			header.command = htons(cmd);
			header.length = htonl(pkt_payload_size);
#else
			sirius::library::net::sicp::packet_header_t header;
			header.command = htons(cmd);
			header.version = sirius::library::net::sicp::session::protocol_version;
			header.type = msg_type;
			header.length = htonl(pkt_header_size + pkt_payload_size);
			memcpy(header.dst, (void *)&dst_uuid.hton(), sizeof(header.dst));
			memcpy(header.src, (void *)&src_uuid.hton(), sizeof(header.src));
#endif
			memmove(send_buffer->packet, &header, pkt_header_size);
			if (pkt_payload_size > 0)
				memmove(send_buffer->packet + pkt_header_size, pkt_payload, pkt_payload_size);
			send_buffer->send_size = pkt_header_size + pkt_payload_size;

			if (fd() != INVALID_SOCKET)
				send_queue.push_back(send_buffer);

			if (send_queue.size()>0)
				static_cast<sirius::library::net::base_processor*>(_processor)->data_request(shared_from_this(), send_queue);
		}
	}
	else
	{
		char * pkt_payload = msg;
		int32_t pkt_header_size = sizeof(sirius::library::net::sicp::packet_header_t);

		int32_t pkt_payload_size = _mtu - pkt_header_size;
		int32_t pkt_totalsize = pkt_header_size + length;
		int32_t pkt_count = pkt_totalsize / _mtu;
		int32_t last_pkt_size = pkt_totalsize % _mtu;

		sirius::uuid src_uuid = std::string(src);
		sirius::uuid dst_uuid = std::string(dst);

		if (last_pkt_size > 0)
			pkt_count++;
		if (pkt_count < 1)
			return;

		std::vector<std::shared_ptr<sirius::library::net::sicp::session::send_buffer_t>> send_queue;
		for (int i = 0; i < pkt_count; i++)
		{
			std::shared_ptr<sirius::library::net::sicp::session::send_buffer_t> send_buffer;
			if (_dynamic_alloc)
				send_buffer = std::shared_ptr<sirius::library::net::sicp::session::send_buffer_t>(new sirius::library::net::sicp::session::send_buffer_t(_mtu));
			else
			{
				bool empty = true;
				for (int32_t index = 0; index < 100; index++)
				{
					{
						sirius::autolock mutex(&_ready_send_cs_lock);
						empty = _send_ready_queue.empty();
					}

					if (!empty)
					{
						{
							sirius::autolock mutex(&_ready_send_cs_lock);
							send_buffer = _send_ready_queue.front();
							if (send_buffer)
							{
								_send_ready_queue.pop_front();
								break;
							}
						}
						::Sleep(10);
					}
					else
					{
						::Sleep(10);
					}
				}
			}

			if (send_buffer)
			{
				if (i == (pkt_count - 1))//last packet	or pkt_totalsize is less than mtu size. 		
				{
					if (pkt_count == 1)
					{
						pkt_payload_size = pkt_totalsize;

#if defined(WITH_NEW_PROTOCOL)
						sirius::library::net::sicp::packet_header_t header;
						header.pid = 'S';
						memcpy(header.dst, (void *)&dst_uuid.hton(), sizeof(header.dst));
						memcpy(header.src, (void *)&src_uuid.hton(), sizeof(header.src));
						header.version = sirius::library::net::sicp::session::protocol_version;
						header.command = htons(cmd);
						header.length = htonl(pkt_payload_size);
#else
						sirius::library::net::sicp::packet_header_t header;
						header.command = htons(cmd);
						header.version = sirius::library::net::sicp::session::protocol_version;
						header.type = msg_type;
						header.length = htonl(pkt_payload_size);
						memcpy(header.dst, (void *)&dst_uuid.hton(), sizeof(header.dst));
						memcpy(header.src, (void *)&src_uuid.hton(), sizeof(header.src));
#endif
						memmove(send_buffer->packet, &header, pkt_header_size);
						if (length > 0)
							memmove(send_buffer->packet + pkt_header_size, pkt_payload, length);
						send_buffer->send_size = pkt_payload_size;

						//LOGGER::make_trace_log(CAPS, "%s()_%d : header.length=%d, length=%d ", __FUNCTION__, __LINE__, pkt_payload_size, send_buffer->size);
					}
					else
					{
						if (last_pkt_size > 0)
							pkt_payload_size = last_pkt_size;
						else
							pkt_payload_size = _mtu;

						memmove(send_buffer->packet, pkt_payload, pkt_payload_size);
						send_buffer->send_size = pkt_payload_size;
						//LOGGER::make_trace_log(CAPS, "%s()_%d : length=%d ", __FUNCTION__, __LINE__, send_buffer->size);
					}
				}
				else
				{
					if (i == 0)
					{
						pkt_payload_size = _mtu - pkt_header_size;

#if defined(WITH_NEW_PROTOCOL)
						sirius::library::net::sicp::packet_header_t header;
						header.pid = 'S';
						memcpy(header.dst, (void *)&dst_uuid.hton(), sizeof(header.dst));
						memcpy(header.src, (void *)&src_uuid.hton(), sizeof(header.src));
						header.version = sirius::library::net::sicp::session::protocol_version;
						header.command = htons(cmd);
						header.length = htonl(pkt_totalsize);
#else
						sirius::library::net::sicp::packet_header_t header;
						header.command = htons(cmd);
						header.version = sirius::library::net::sicp::session::protocol_version;
						header.type = msg_type;
						header.length = htonl(pkt_totalsize);
						memcpy(header.dst, (void *)&dst_uuid.hton(), sizeof(header.dst));
						memcpy(header.src, (void *)&src_uuid.hton(), sizeof(header.src));
#endif
						memmove(send_buffer->packet, &header, pkt_header_size);
						memmove(send_buffer->packet + pkt_header_size, pkt_payload, pkt_payload_size);
						pkt_payload += pkt_payload_size;
						send_buffer->send_size = pkt_header_size + pkt_payload_size;
						//LOGGER::make_trace_log(CAPS, "%s()_%d : header.length=%d, length=%d ", __FUNCTION__, __LINE__, pkt_totalsize, send_buffer->size);
					}
					else
					{
						pkt_payload_size = _mtu;

						memmove(send_buffer->packet, pkt_payload, pkt_payload_size);
						pkt_payload += pkt_payload_size;
						send_buffer->send_size = pkt_payload_size;
						//LOGGER::make_trace_log(CAPS, "%s()_%d : length=%d ", __FUNCTION__, __LINE__, send_buffer->size);
					}
				}

				if (fd() != INVALID_SOCKET)
				{
					send_queue.push_back(send_buffer);
				}
			}
		}
		if (send_queue.size() > 0)
			static_cast<sirius::library::net::base_processor*>(_processor)->data_request(shared_from_this(), send_queue);
	}
}

void sirius::library::net::sicp::session::push_send_packet(const char * msg, int32_t length)
{

}
int32_t sirius::library::net::sicp::session::push_recv_packet(const char * msg, int32_t length)
{
	sirius::autolock lock(&_recv_cs_lock);
	uint32_t pkt_header_size = sizeof(sirius::library::net::sicp::packet_header_t);

	if (_precv_buffer == nullptr)
	{
		LOGGER::make_error_log(SAA, "%s, %d, _precv_buffer_nullptr_error recv_packet=%d", __FUNCTION__, __LINE__, this->fd());
		LOGGER::make_error_log(SLNS, "%s, %d, _precv_buffer_nullptr_error recv_packet=%d", __FUNCTION__, __LINE__, this->fd());
		//printf("recv_packet %d", this->fd());
		return -1;
	}

	char debug_msg[1000] = { 0 };
	//snprintf(debug_msg, sizeof(debug_msg), "%s()_%d : recv_buffer_size[%d], recv_buffer_index[%d], length[%d]  \n", __FUNCTION__, __LINE__, _recv_buffer_size, _recv_buffer_index, length);
	//::OutputDebugStringA(debug_msg);

	if ((_recv_buffer_index + length) > _recv_buffer_size)
	{
		_precv_buffer = static_cast<char*>(realloc(_precv_buffer, _recv_buffer_index + length));
		_recv_buffer_size = _recv_buffer_index + length;
	}
	memcpy(_precv_buffer + _recv_buffer_index, msg, length);
	_recv_buffer_index += length;

	do
	{
		if (_recv_buffer_index < pkt_header_size)
			return (pkt_header_size - _recv_buffer_index);

		//read hedaer
#if defined(WITH_NEW_PROTOCOL)
		sirius::library::net::sicp::packet_header_t header;
		memcpy(&header, (sirius::library::net::sicp::packet_header_t *)_precv_buffer, pkt_header_size);

		if (header.pid != 'S')
			return -1;

		uint16_t command = header.command;
		header.command = ntohs(header.command);
		header.length = ntohl(header.length);
#else
		sirius::library::net::sicp::packet_header_t header;
		memcpy(&header, (sirius::library::net::sicp::packet_header_t *)_precv_buffer, pkt_header_size);
		uint16_t command = header.command;
		header.command = ntohs(header.command);
		header.length = ntohl(header.length);
#endif
		//snprintf(debug_msg, sizeof(debug_msg), "%s()_%d : recv_buffer_size[%d], recv_buffer_index[%d], length[%d], command_id[%d], header.length[%d]  \n", __FUNCTION__, __LINE__, _recv_buffer_size, _recv_buffer_index, length, header.command, header.length);
		//::OutputDebugStringA(debug_msg);

		if (_processor && !_processor->check_valid_command_id(header.command))
		{
			snprintf(debug_msg, sizeof(debug_msg), "%s, %d, recv_buffer_size[%d], recv_buffer_index[%d], length[%d], command_id[%d], header.length[%d], header.Version[%u]", __FUNCTION__, __LINE__, _recv_buffer_size, _recv_buffer_index, length, header.command, header.length, header.version);
			LOGGER::make_error_log(SAA, "%s, %d, invalid_packet_error recv_packet=%s", __FUNCTION__, __LINE__, debug_msg);
			LOGGER::make_error_log(SLNS, "%s, %d, invalid_packet_error recv_packet=%s", __FUNCTION__, __LINE__, debug_msg);
			return -1;  // invalid packet
		}

		if (header.version > sirius::library::net::sicp::session::protocol_version)    // 프로토콜 버전 체크
		{
			snprintf(debug_msg, sizeof(debug_msg), "%s, %d, recv_buffer_size[%d], recv_buffer_index[%d], length[%d], command_id[%d], header.length[%d], header.Version[%u]", __FUNCTION__, __LINE__, _recv_buffer_size, _recv_buffer_index, length, header.command, header.length, header.version);
			LOGGER::make_error_log(SAA, "%s, %d, protocol_version_error recv_packet=%s", __FUNCTION__, __LINE__, debug_msg);
			LOGGER::make_error_log(SLNS, "%s, %d, protocol_version_error recv_packet=%s", __FUNCTION__, __LINE__, debug_msg);
			return -2;
		}

		/*
		if (header.type >= msg_type_t::max_type)  // 메시지 타입 체크
		{
			//snprintf(debug_msg, sizeof(debug_msg), "%s, %d, recv_buffer_size[%d], recv_buffer_index[%d], length[%d], command_id[%d], header.length[%d], header.Version[%u], header.PayloadType[%u]", __FUNCTION__, __LINE__, _recv_buffer_size, _recv_buffer_index, length, header.command, header.length, header.version, header.type);
			LOGGER::make_error_log(SAC, "%s, %d, msg_type_error recv_packet=%s", __FUNCTION__, __LINE__, debug_msg);
			LOGGER::make_error_log(SLNS, "%s, %d, msg_type_error recv_packet=%s", __FUNCTION__, __LINE__, debug_msg);
			return -3;
		}
		*/

		if (_recv_buffer_index < (pkt_header_size + header.length))
			return ((pkt_header_size + header.length) - _recv_buffer_index);

		// 메시지 수신 완료
		sirius::uuid src_uuid((uint8_t*)header.src, 16);
		sirius::uuid dst_uuid((uint8_t*)header.dst, 16);


#if defined(WITH_NEW_PROTOCOL)
		data_indication_callback((const char *)dst_uuid.to_string_ntoh().c_str(), (const char *)src_uuid.to_string_ntoh().c_str(),
								 header.command, header.version, _precv_buffer + pkt_header_size, header.length, std::static_pointer_cast<sirius::library::net::sicp::session>(shared_from_this()));
#else
		data_indication_callback((const char *)dst_uuid.to_string_ntoh().c_str(), (const char *)src_uuid.to_string_ntoh().c_str(),
			header.command, header.version, _precv_buffer + pkt_header_size, header.length - pkt_header_size,
			sirius::library::net::session::downcasted_shared_from_this<sirius::library::net::sicp::session>());
#endif


		if (_recv_buffer_index >= (pkt_header_size + header.length))
		{
			memmove(_precv_buffer, _precv_buffer + (pkt_header_size + header.length), _recv_buffer_index - (pkt_header_size + header.length));
			_recv_buffer_index -= (pkt_header_size + header.length);
		}

	} while (_recv_buffer_index >= pkt_header_size);

	return pkt_header_size;
}

int32_t sirius::library::net::sicp::session::get_first_recv_packet_size(void)
{
	return sizeof(sirius::library::net::sicp::packet_header_t);
}

void sirius::library::net::sicp::session::update_heart_beat(void)
{
	_heart_beat = ::GetTickCount64();
}

const char * sirius::library::net::sicp::session::ip(void)
{
	return _ip;
}

const char * sirius::library::net::sicp::session::uuid(void)
{
	return _uuid;
}

const char * sirius::library::net::sicp::session::name(void)
{
	return _name;
}

void sirius::library::net::sicp::session::ip(const char * ip)
{
	strncpy_s(_ip, ip, sizeof(_ip));
}

void sirius::library::net::sicp::session::uuid(const char * uuid)
{
	strncpy_s(_uuid, uuid, sizeof(_uuid));
}

void sirius::library::net::sicp::session::name(const char * name)
{
	strncpy_s(_name, name, sizeof(_name));
}

bool sirius::library::net::sicp::session::disconnect_flag(void) const
{
	return _disconnect;
}

void sirius::library::net::sicp::session::disconnect_flag(bool flag)
{
	_disconnect = flag;
}

bool sirius::library::net::sicp::session::connected_flag(void) const
{
	return _connected;
}

void sirius::library::net::sicp::session::connected_flag(bool flag)
{
	_connected = flag;
}

bool sirius::library::net::sicp::session::assoc_flag(void) const
{
	return _associated;
}

void sirius::library::net::sicp::session::assoc_flag(bool flag)
{
	_associated = flag;
}

int32_t sirius::library::net::sicp::session::get_hb_period_sec(void) const
{
#if defined(WIN32)
	return _hb_period / 1000;
#else
	return _hb_period;
#endif
}

void sirius::library::net::sicp::session::set_hb_period_sec(int32_t sec)
{
#if defined(WIN32)
	_hb_period = sec * 1000;
#else
	_hb_period = sec;
#endif
}

void sirius::library::net::sicp::session::update_hb_start_time(void)
{
	_hb_start = ::GetTickCount64();
}

void sirius::library::net::sicp::session::update_hb_end_time(void)
{
	_hb_end = ::GetTickCount64();
}

bool sirius::library::net::sicp::session::check_hb(void)
{
#if defined(WIN32)
	uint64_t hb_diff = _hb_end - _hb_start;
	if (hb_diff >= _hb_period)
		return true;
	else
		return false;

#else
	struct timeval hb_diff;
	struct timeval block_diff;
	timersub(&_hb_end, &_hb_start, &hb_diff);
	timersub(&_hb_end, &_hb_block, &block_diff);

	if (block_diff.tv_sec > 5) //prevent more than 1packet
	{
		gettimeofday(&_hb_block, 0);
		if (hb_diff.tv_sec >= _hb_period)
			return true;
		else
			return false;
	}
	else
		return false;
#endif
}
void sirius::library::net::sicp::session::data_indication_callback(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, size_t length, std::shared_ptr<sirius::library::net::sicp::session> session)
{
	_processor->data_indication_callback(dst, src, command_id, version, msg, length, session);
}
