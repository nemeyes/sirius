#ifndef _SICP_CLIENT_H_
#define _SICP_CLIENT_H_

#include <sicp_abstract_client.h>
#include <sicp_session.h>
#include <sicp_command.h>

#include "sirius_sicp_client.h"

namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace sicp
			{
				class client::core
					: public sirius::library::net::sicp::abstract_client
				{
				public:
					core(sirius::library::net::sicp::client * front, int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size,int32_t recv_buffer_size, int32_t command_thread_pool_count = 3, int32_t io_thread_pool_count = 1, bool use_keep_alive = true, bool dynamic_alloc = false, int32_t type = sirius::library::net::client::ethernet_type_t::tcp, bool multicast = false);
					core(sirius::library::net::sicp::client * front, const char * uuid, int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size, int32_t recv_buffer_size, int32_t command_thread_pool_count = 3, int32_t io_thread_pool_count = 1, bool use_keep_alive = true, bool dynamic_alloc = false, int32_t type = sirius::library::net::client::ethernet_type_t::tcp, bool multicast = false);
					virtual ~core(void);

					bool connect(const char * address, int32_t port_number, bool reconnection = true);

					void create_session_callback(void);
					void destroy_session_callback(void);

				private:
					sirius::library::net::sicp::client *	_front;
					int32_t									_io_thread_pool_count;
				};
			};
		};
	};
};









#endif
