#include <localcache_session.h>
#if defined(WITH_WORKING_AS_SERVER)
#include <localcache_server.h>
#else
#include <localcache_client.h>
#endif

#include <localcache_packet.h>

#include <sirius_uuid.h>
#include <sirius_locks.h>

sirius::library::cache::local::session::session(sirius::library::cache::local::localcache_processor * prcsr, int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, BOOL tls, SSL_CTX * ssl_ctx, BOOL reconnection)
	: sirius::library::net::iocp::session(static_cast<sirius::library::net::iocp::processor*>(prcsr), so_recv_buffer_size, so_send_buffer_size, recv_buffer_size, send_buffer_size, tls, ssl_ctx, reconnection)
	, _localcache_processor(prcsr)
	, _disconnect(FALSE)
	, _recv_buffer_index(0)
{
	::InitializeCriticalSection(&_send_lock);
	::InitializeCriticalSection(&_recv_lock);

	_recv_buffer = static_cast<char*>(malloc(_recv_buffer_size));
	memset(_recv_buffer, 0x0, _recv_buffer_size);

	_send_buffer = static_cast<char*>(malloc(_send_buffer_size));
	memset(_send_buffer, 0x0, _send_buffer_size);
}

sirius::library::cache::local::session::~session(void)
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

void sirius::library::cache::local::session::send(int32_t cmd, const char * payload, int32_t payload_size)
{
	sirius::autolock lock(&_send_lock);

	const char *	pkt_payload = payload;
	uint32_t		pkt_header_size = sizeof(sirius::library::cache::local::packet_header_t);
	uint32_t		pkt_payload_size = payload_size;
	uint32_t		pkt_size = pkt_header_size + pkt_payload_size;

	if (pkt_size > _send_buffer_size)
	{
		_send_buffer = static_cast<char*>(realloc(_send_buffer, pkt_size));
		_send_buffer_size = pkt_size;
	}

	sirius::library::cache::local::packet_header_t header;
	header.command = htons(cmd);
	header.length = htonl(pkt_payload_size);
		
	memmove(_send_buffer, &header, pkt_header_size);
	if (pkt_payload_size > 0)
		memmove(_send_buffer + pkt_header_size, pkt_payload, pkt_payload_size);
	
	sirius::library::net::iocp::session::send(_send_buffer, pkt_size);
}

int32_t	sirius::library::cache::local::session::on_recv(const char * packet, int32_t packet_size)
{
	sirius::autolock lock(&_recv_lock);
	uint32_t pkt_header_size = sizeof(sirius::library::cache::local::packet_header_t);

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

		sirius::library::cache::local::packet_header_t header;
		memcpy(&header, (sirius::library::cache::local::packet_header_t *)_recv_buffer, pkt_header_size);

		uint16_t command = header.command;
		header.command = ntohs(header.command);
		header.length = ntohl(header.length);

		if (_recv_buffer_index < (pkt_header_size + header.length))
			return ((pkt_header_size + header.length) - _recv_buffer_index);

		on_data_indication(header.command, _recv_buffer + pkt_header_size, header.length, std::static_pointer_cast<sirius::library::cache::local::session>(shared_from_this()));

		if (_recv_buffer_index >= (pkt_header_size + header.length))
		{
			memmove(_recv_buffer, _recv_buffer + (pkt_header_size + header.length), _recv_buffer_index - (pkt_header_size + header.length));
			_recv_buffer_index -= (pkt_header_size + header.length);
		}

	} while (_recv_buffer_index >= pkt_header_size);
	return pkt_header_size;
}

int32_t sirius::library::cache::local::session::packet_header_size(void)
{
	return sizeof(sirius::library::cache::local::packet_header_t);
}

BOOL sirius::library::cache::local::session::disconnect_flag(void) const
{
	return _disconnect;
}

void sirius::library::cache::local::session::disconnect_flag(BOOL flag)
{
	_disconnect = flag;
}

void sirius::library::cache::local::session::on_data_indication(int32_t command_id, const char * payload, int32_t payload_size, std::shared_ptr<sirius::library::cache::local::session> session)
{
	_localcache_processor->on_data_indication(command_id, payload, payload_size, session);
}
