#include "sicp_client.h"
#include <sirius_locks.h>
#include <sicp_command.h>

sirius::library::net::sicp::client::core::core(sirius::library::net::sicp::client * front, int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, int32_t io_thread_pool_count, int32_t command_thread_pool_count, BOOL keepalive, int32_t keepalive_timeout, BOOL tls)
	: sirius::library::net::sicp::abstract_client(command_thread_pool_count, keepalive, keepalive_timeout, so_recv_buffer_size, so_send_buffer_size, recv_buffer_size, send_buffer_size, tls)
	, _front(front)
	, _io_thread_pool_count(io_thread_pool_count)
{
	sirius::library::net::sicp::abstract_client::initialize();
}

sirius::library::net::sicp::client::core::core(sirius::library::net::sicp::client * front, const char * uuid, int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, int32_t io_thread_pool_count, int32_t command_thread_pool_count, BOOL keepalive, int32_t keepalive_timeout, BOOL tls)
	: sirius::library::net::sicp::abstract_client(uuid, command_thread_pool_count, keepalive, keepalive_timeout, so_recv_buffer_size, so_send_buffer_size, recv_buffer_size, send_buffer_size, tls)
	, _front(front)
	, _io_thread_pool_count(io_thread_pool_count)
{
	sirius::library::net::sicp::abstract_client::initialize();
}

sirius::library::net::sicp::client::core::~core(void)
{
	sirius::library::net::sicp::abstract_client::release();
}

int32_t sirius::library::net::sicp::client::core::connect(const char * address, int32_t portnumber, BOOL reconnection)
{
	return sirius::library::net::sicp::abstract_client::connect(address, portnumber, _io_thread_pool_count, reconnection);
}

void sirius::library::net::sicp::client::core::on_create_session(void)
{
	if (_front)
		_front->on_create_session();
}

void sirius::library::net::sicp::client::core::on_destroy_session(void)
{
	if (_front)
		_front->on_destroy_session();
}
