#include "sirius_sicp_client.h"
#include <sicp_command.h>
#include "sicp_client.h"

sirius::library::net::sicp::client::client(int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, int32_t io_thread_pool_count, int32_t command_thread_pool_count, BOOL keepliave, BOOL tls)
{
	_client = new sirius::library::net::sicp::client::core(this, so_recv_buffer_size, so_send_buffer_size, recv_buffer_size, send_buffer_size, io_thread_pool_count, command_thread_pool_count, keepliave, tls);
}

sirius::library::net::sicp::client::client(const char * uuid, int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, int32_t io_thread_pool_count, int32_t command_thread_pool_count, BOOL keepalive, BOOL tls)
{
	_client = new sirius::library::net::sicp::client::core(this, uuid, so_recv_buffer_size, so_send_buffer_size, recv_buffer_size, send_buffer_size, io_thread_pool_count, command_thread_pool_count, keepalive, tls);
}

sirius::library::net::sicp::client::~client(void)
{
	if (_client)
	{
		delete _client;
	}
	_client = nullptr;
}

int32_t sirius::library::net::sicp::client::connect(const char * address, int32_t portnumber, BOOL reconnection)
{
	return _client->connect(address, portnumber, reconnection);
}

int32_t sirius::library::net::sicp::client::disconnect(void)
{
	return _client->disconnect();
}

void sirius::library::net::sicp::client::data_request(const char * dst, int32_t command_id, const char * packet, int32_t packet_size)
{
	_client->data_request(dst, command_id, packet, packet_size);
}

void sirius::library::net::sicp::client::add_command(abstract_command * command)
{
	_client->add_command(command);
}

void sirius::library::net::sicp::client::add_command(int32_t forword_message_id)
{
	_client->add_forarded_command(forword_message_id);
}