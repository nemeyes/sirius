#include "sicp_server.h"
#include <sirius_locks.h>
#include <sirius_commands.h>
#include "sirius_sicp_server.h"

sirius::library::net::sicp::server::core::core(sirius::library::net::sicp::server * front, int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, const char * uuid, int32_t command_thread_pool_count, BOOL keepalive, int32_t keepalive_timeout, BOOL tls, int32_t max_sessions)
	: sirius::library::net::sicp::abstract_server(uuid, command_thread_pool_count, keepalive, keepalive_timeout, so_recv_buffer_size, so_send_buffer_size, recv_buffer_size, send_buffer_size, tls, max_sessions)
	, _front(front)
{
	sirius::library::net::sicp::abstract_server::initialize();
}

sirius::library::net::sicp::server::core::~core(void)
{
	sirius::library::net::sicp::abstract_server::release();
}

void sirius::library::net::sicp::server::core::on_create_session(const char * uuid)
{
	if (_front)
		_front->on_create_session(uuid);
}

void sirius::library::net::sicp::server::core::on_destroy_session(const char * uuid)
{
	if (_front)
		_front->on_destroy_session(uuid);
}
