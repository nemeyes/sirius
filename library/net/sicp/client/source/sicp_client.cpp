#include "sicp_client.h"
#include <sirius_locks.h>
#include <sicp_command.h>

sirius::library::net::sicp::client::core::core(sirius::library::net::sicp::client * front, int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size,int32_t recv_buffer_size, int32_t command_thread_pool_count, int32_t io_thread_pool_count, bool use_keep_alive, bool dynamic_alloc, int32_t type, bool multicast)
	: sirius::library::net::sicp::abstract_client(mtu, so_rcvbuf_size, so_sndbuf_size,recv_buffer_size, command_thread_pool_count, use_keep_alive, dynamic_alloc, type, multicast)
	, _front(front)
	, _io_thread_pool_count(io_thread_pool_count)
{

}

sirius::library::net::sicp::client::core::core(sirius::library::net::sicp::client * front, const char * uuid, int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size, int32_t recv_buffer_size, int32_t command_thread_pool_count, int32_t io_thread_pool_count, bool use_keep_alive, bool dynamic_alloc, int32_t type, bool multicast)
	: sirius::library::net::sicp::abstract_client(uuid, mtu, so_rcvbuf_size, so_sndbuf_size,recv_buffer_size, command_thread_pool_count, use_keep_alive, dynamic_alloc, type, multicast)
	, _front(front)
	, _io_thread_pool_count(io_thread_pool_count)
{

}

sirius::library::net::sicp::client::core::~core(void)
{

}

bool sirius::library::net::sicp::client::core::connect(const char * address, int32_t port_number, bool reconnection)
{
	return sirius::library::net::sicp::abstract_client::connect(address, port_number, _io_thread_pool_count, reconnection);
}

void sirius::library::net::sicp::client::core::create_session_callback(void)
{
	if (_front)
		_front->create_session_callback();
}

void sirius::library::net::sicp::client::core::destroy_session_callback(void)
{
	if (_front)
		_front->destroy_session_callback();
}
