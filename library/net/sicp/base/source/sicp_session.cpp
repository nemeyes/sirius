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

sirius::library::net::sicp::session::session(sirius::library::net::sicp::sicp_processor * prcsr, int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, BOOL tls, SSL_CTX * ssl_ctx, BOOL reconnection)
	: sirius::library::net::iocp::session(static_cast<sirius::library::net::iocp::processor*>(prcsr), so_recv_buffer_size, so_send_buffer_size, recv_buffer_size, send_buffer_size, tls, ssl_ctx, reconnection)
	, _sicp_processor(prcsr)
	, _disconnect(FALSE)
	, _registerd(FALSE)
	, _recv_buffer_index(0)
{
	::InitializeCriticalSection(&_send_lock);
	::InitializeCriticalSection(&_recv_lock);

	memset(_ip, 0x00, sizeof(_ip));
	memset(_uuid, 0x00, sizeof(_uuid));
	memset(_name, 0x00, sizeof(_name));

	//_recv_buffer_size = MTU;
	_recv_buffer = static_cast<char*>(malloc(_recv_buffer_size));
	memset(_recv_buffer, 0x0, _recv_buffer_size);

	_send_buffer = static_cast<char*>(malloc(_send_buffer_size));
	memset(_send_buffer, 0x0, _send_buffer_size);
}

sirius::library::net::sicp::session::~session(void)
{
	if (_recv_buffer)
	{
		free(_recv_buffer);
		_recv_buffer = NULL;
	}

	if (_send_buffer)
	{
		free(_send_buffer);
		_send_buffer = NULL;
	}

	::DeleteCriticalSection(&_send_lock);
	::DeleteCriticalSection(&_recv_lock);
}

void sirius::library::net::sicp::session::send(const char * dst, const char * src, int32_t cmd, const char * payload, int32_t payload_size)
{
	sirius::autolock lock(&_send_lock);

	const char *	pkt_payload = payload;
	uint32_t		pkt_header_size = sizeof(sirius::library::net::sicp::packet_header_t);
	uint32_t		pkt_payload_size = payload_size;
	uint32_t		pkt_size = pkt_header_size + pkt_payload_size;

	sirius::uuid src_uuid = std::string(src);
	sirius::uuid dst_uuid = std::string(dst);

	if (pkt_size > _send_buffer_size)
	{
		_send_buffer = static_cast<char*>(realloc(_send_buffer, pkt_size));
		_send_buffer_size = pkt_size;
	}

	sirius::library::net::sicp::packet_header_t header;
	header.pid = 'S';
	memcpy(header.dst, (void *)&dst_uuid.hton(), sizeof(header.dst));
	memcpy(header.src, (void *)&src_uuid.hton(), sizeof(header.src));
	header.version = sirius::library::net::sicp::session::protocol_version;
	header.command = htons(cmd);
	header.length = htonl(pkt_payload_size);
		
	memmove(_send_buffer, &header, pkt_header_size);
	if (pkt_payload_size > 0)
		memmove(_send_buffer + pkt_header_size, pkt_payload, pkt_payload_size);

	sirius::library::net::iocp::session::send(_send_buffer, pkt_size);
}

int32_t	sirius::library::net::sicp::session::on_recv(const char * packet, int32_t packet_size)
{
	sirius::autolock lock(&_recv_lock);
	uint32_t pkt_header_size = sizeof(sirius::library::net::sicp::packet_header_t);

	if ((_recv_buffer_index + packet_size) > _recv_buffer_size)
	{
		_recv_buffer = static_cast<char*>(realloc(_recv_buffer, _recv_buffer_index + packet_size));
		_recv_buffer_size = _recv_buffer_index + packet_size;
	}

	memcpy(_recv_buffer + _recv_buffer_index, packet, packet_size);
	_recv_buffer_index += packet_size;

	do
	{
		if (_recv_buffer_index < pkt_header_size)
			return (pkt_header_size - _recv_buffer_index);

		//read hedaer
		sirius::library::net::sicp::packet_header_t header;
		memcpy(&header, (sirius::library::net::sicp::packet_header_t *)_recv_buffer, pkt_header_size);
		if (header.pid != 'S')
			return -1;

		//uint16_t command = header.command;
		header.command = ntohs(header.command);
		header.length = ntohl(header.length);

		if (header.version > sirius::library::net::sicp::session::protocol_version)    // 프로토콜 버전 체크
			return -1;

		if (_recv_buffer_index < (pkt_header_size + header.length))
			return ((pkt_header_size + header.length) - _recv_buffer_index);

		// 메시지 수신 완료
		sirius::uuid src_uuid((uint8_t*)header.src, 16);
		sirius::uuid dst_uuid((uint8_t*)header.dst, 16);

		on_data_indication((const char *)dst_uuid.to_string_ntoh().c_str(), (const char *)src_uuid.to_string_ntoh().c_str(),
			header.command, header.version, _recv_buffer + pkt_header_size, header.length, std::static_pointer_cast<sirius::library::net::sicp::session>(shared_from_this()));

		if (_recv_buffer_index >= (pkt_header_size + header.length))
		{
			memmove(_recv_buffer, _recv_buffer + (pkt_header_size + header.length), _recv_buffer_index - (pkt_header_size + header.length));
			_recv_buffer_index -= (pkt_header_size + header.length);
		}

	} while (_recv_buffer_index >= pkt_header_size);

	return pkt_header_size;
}

int32_t sirius::library::net::sicp::session::packet_header_size(void)
{
	return sizeof(sirius::library::net::sicp::packet_header_t);
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

BOOL sirius::library::net::sicp::session::disconnect_flag(void) const
{
	return _disconnect;
}

void sirius::library::net::sicp::session::disconnect_flag(BOOL flag)
{
	_disconnect = flag;
}

BOOL sirius::library::net::sicp::session::register_flag(void) const
{
	return _registerd;
}

void sirius::library::net::sicp::session::register_flag(BOOL flag)
{
	_registerd = flag;
}

void sirius::library::net::sicp::session::on_data_indication(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * payload, int32_t payload_size, std::shared_ptr<sirius::library::net::sicp::session> session)
{
	_sicp_processor->on_data_indication(dst, src, command_id, version, payload, payload_size, session);
}
