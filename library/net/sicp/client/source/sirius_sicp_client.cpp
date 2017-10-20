#include "sirius_sicp_client.h"
#include <sicp_command.h>
#include "sicp_client.h"

sirius::library::net::sicp::client::client(int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size, int32_t recv_buffer_size, int32_t command_thread_pool_count, int32_t io_thread_pool_count , bool use_keep_alive , bool dynamic_alloc, int32_t type, bool multicast)
{
	_client = new sirius::library::net::sicp::client::core(this, mtu, so_rcvbuf_size, so_sndbuf_size,recv_buffer_size, command_thread_pool_count, io_thread_pool_count, use_keep_alive, dynamic_alloc, type, multicast);
}

sirius::library::net::sicp::client::client(const char * uuid, int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size,int32_t recv_buffer_size, int32_t command_thread_pool_count, int32_t io_thread_pool_count, bool use_keep_alive, bool dynamic_alloc, int32_t type, bool multicast)
{
	_client = new sirius::library::net::sicp::client::core(this, uuid, mtu, so_rcvbuf_size, so_sndbuf_size,recv_buffer_size, command_thread_pool_count, io_thread_pool_count, use_keep_alive, dynamic_alloc, type, multicast);
}

sirius::library::net::sicp::client::~client(void)
{
	if (_client)
	{
		delete _client;
	}
	_client = nullptr;
}

bool sirius::library::net::sicp::client::connect(const char * address, int32_t port_number, bool reconnection)
{
	return _client->connect(address, port_number, reconnection);
}

bool sirius::library::net::sicp::client::disconnect(void)
{
	return _client->disconnect();
}

void sirius::library::net::sicp::client::data_request(char * dst, int32_t command_id, char * msg, int32_t length)
{
	_client->data_request(dst, command_id, msg, length);
}

void sirius::library::net::sicp::client::add_command(abstract_command * command)
{
	_client->add_command(command);
}

void sirius::library::net::sicp::client::add_command(int32_t forword_message_id)
{
	_client->add_forarded_command(forword_message_id);
}