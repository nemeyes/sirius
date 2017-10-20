#ifndef _SIRIUS_SICP_CLIENT_H_
#define _SIRIUS_SICP_CLIENT_H_

#include <cstdint>

namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace sicp
			{
				class abstract_command;
				class client
				{
				public:
					class core;
				public:
/*
					client(int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size,int32_t recv_buffer_size, int32_t command_thread_pool_count = 3, int32_t io_thread_pool_count = 1, bool use_keep_alive = false, bool dynamic_alloc = false, int32_t type = sirius::library::net::sicp::client::ethernet_type_t::tcp, bool multicast = false);
					client(const char * uuid, int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size, int32_t recv_buffer_size,int32_t command_thread_pool_count = 3, int32_t io_thread_pool_count = 1, bool use_keep_alive = false, bool dynamic_alloc = false, int32_t type = sirius::library::net::sicp::client::ethernet_type_t::tcp, bool multicast = false);
*/
					client(int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size,int32_t recv_buffer_size, int32_t command_thread_pool_count, int32_t io_thread_pool_count, bool use_keep_alive, bool dynamic_alloc, int32_t type, bool multicast);
					client(const char * uuid, int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size, int32_t recv_buffer_size,int32_t command_thread_pool_count, int32_t io_thread_pool_count, bool use_keep_alive, bool dynamic_alloc, int32_t type, bool multicast);
					virtual ~client(void);

					bool connect(const char * address, int32_t port_number, bool reconnection = true);
					bool disconnect(void);

					void data_request(char * dst, int32_t command_id, char * msg, int32_t length);
					void add_command(abstract_command * command);
					void add_command(int32_t forword_message_id);

					virtual void create_session_callback(void) = 0;
					virtual void destroy_session_callback(void) = 0;

				private:
					sirius::library::net::sicp::client::core * _client;
				};
			};
		};
	};
};




#endif
