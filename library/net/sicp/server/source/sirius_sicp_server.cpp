#include "sirius_sicp_server.h"
#include <sicp_command.h>
#include "sicp_server.h"

sirius::library::net::sicp::server::server(const char * uuid, int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, int32_t io_thread_pool_count, int32_t command_thread_pool_count, BOOL keepalive, int32_t keepalive_timeout, BOOL tls, int32_t max_sessions)
	: _io_thread_pool_count(io_thread_pool_count)
{
	_server = new sirius::library::net::sicp::server::core(this, so_recv_buffer_size, so_send_buffer_size, recv_buffer_size, send_buffer_size, uuid, command_thread_pool_count, keepalive, keepalive_timeout, tls, max_sessions);
	_server->initialize();
}

sirius::library::net::sicp::server::~server(void)
{
	if (_server)
	{
		_server->release();
		delete _server;
	}
	_server = nullptr;
}

void sirius::library::net::sicp::server::uuid(const char * uuid)
{
	return _server->uuid(uuid);
}

int32_t sirius::library::net::sicp::server::start(char * address, int32_t port_number)
{
	return _server->start(address, port_number, _io_thread_pool_count);
}

int32_t sirius::library::net::sicp::server::stop(void)
{
	return _server->stop();
}

bool sirius::library::net::sicp::server::is_valid(const char * uuid)
{
	return _server->check_activate_session(uuid);
}

void sirius::library::net::sicp::server::data_request(char * dst, int32_t command_id, const char * packet, int32_t packet_size)
{
	_server->data_request(dst, command_id, packet, packet_size);
}

void sirius::library::net::sicp::server::add_command(sirius::library::net::sicp::abstract_command * command)
{
	_server->add_command(command);
}

void sirius::library::net::sicp::server::add_command(int32_t forworded_message_id)
{
	_server->add_forarded_command(forworded_message_id);
}