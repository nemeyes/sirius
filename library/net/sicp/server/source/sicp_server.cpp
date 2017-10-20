#include "sicp_server.h"
#include <sirius_locks.h>
#include <sirius_commands.h>
#include "sirius_sicp_server.h"

sirius::library::net::sicp::server::core::core(sirius::library::net::sicp::server * front, int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size, int32_t recv_buffer_size, const char * uuid, int32_t command_thread_pool_count, bool use_keep_alive, bool dynamic_alloc, int32_t type, bool multicast)
	: sirius::library::net::sicp::abstract_server(mtu, so_rcvbuf_size, so_sndbuf_size, recv_buffer_size,uuid, command_thread_pool_count, use_keep_alive, dynamic_alloc, type, multicast)
	, _front(front)
{

}

sirius::library::net::sicp::server::core::~core(void)
{

}

void sirius::library::net::sicp::server::core::create_session_callback(const char * uuid)
{
	if (_front)
		_front->create_session_callback(uuid);
}

void sirius::library::net::sicp::server::core::destroy_session_callback(const char * uuid)
{
	if (_front)
		_front->destroy_session_callback(uuid);
}
