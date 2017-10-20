#include "sirius_sicp_server.h"
#include <sicp_command.h>
#include "sicp_server.h"

sirius::library::net::sicp::server::server(const char * uuid, int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size, int32_t recv_buffer_size,int32_t io_thread_pool_count, int32_t command_thread_pool_count, bool use_keep_alive, bool dynamic_alloc)
	: _io_thread_pool_count(io_thread_pool_count)
{
	_server = new sirius::library::net::sicp::server::core(this, mtu, so_rcvbuf_size, so_sndbuf_size,recv_buffer_size, uuid, command_thread_pool_count, use_keep_alive, dynamic_alloc);
}

sirius::library::net::sicp::server::~server(void)
{
	if (_server)
	{
		delete _server;
	}
	_server = nullptr;
}

void sirius::library::net::sicp::server::uuid(const char * uuid)
{
	return _server->uuid(uuid);
}

bool sirius::library::net::sicp::server::is_valid(const char * uuid)
{
	return _server->check_alive_session(uuid);
}

bool sirius::library::net::sicp::server::start(char * address, int32_t port_number)
{
	return _server->start(address, port_number, _io_thread_pool_count);
}

bool sirius::library::net::sicp::server::stop(void)
{
	return _server->stop();
}

void sirius::library::net::sicp::server::data_request(char * dst, int32_t command_id, char * msg, int32_t length)
{
	_server->data_request(dst, command_id, msg, length);
}

void sirius::library::net::sicp::server::add_command(sirius::library::net::sicp::abstract_command * command)
{
	_server->add_command(command);
}

void sirius::library::net::sicp::server::add_command(int32_t forworded_message_id)
{
	_server->add_forarded_command(forworded_message_id);
}