#ifndef _SICP_SERVER_H_
#define _SICP_SERVER_H_

#include <sicp_server.h>
#include <sicp_session.h>
#include <sicp_command.h>

#include "sirius_sicp_server.h"

namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace sicp
			{
				class server::core
					: public sirius::library::net::sicp::abstract_server
				{
				public:
					core(sirius::library::net::sicp::server * front, int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size, int32_t recv_buffer_size,const char * uuid, int32_t command_thread_pool_count, bool use_keep_alive = true, bool dynamic_alloc = false, int32_t type = sirius::library::net::server::ethernet_type_t::tcp, bool multicast = false);
					virtual ~core(void);

					void create_session_callback(const char * uui);
					void destroy_session_callback(const char * uuid);

				protected:
					sirius::library::net::sicp::server * _front;

				private:
					core(sirius::library::net::sicp::server::core & clone);
				};
			};
		};
	};
};














#endif