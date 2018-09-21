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
					core(sirius::library::net::sicp::server * front, int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, const char * uuid, int32_t command_thread_pool_count, BOOL keepalive = FALSE, int32_t keepalive_timeout = 5000, BOOL tls = FALSE, int32_t max_sessions = 2100);
					virtual ~core(void);

					void on_create_session(const char * uui);
					void on_destroy_session(const char * uuid);

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