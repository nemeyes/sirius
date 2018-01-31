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
					core(sirius::library::net::sicp::client * front, int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, int32_t io_thread_pool_count = 1, int32_t command_thread_pool_count = 3, BOOL keepalive = FALSE, BOOL tls = FALSE);
					core(sirius::library::net::sicp::client * front, const char * uuid, int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, int32_t io_thread_pool_count = 1, int32_t command_thread_pool_count = 3, BOOL keepalive = FALSE, BOOL tls = FALSE);
					virtual ~core(void);

					int32_t connect(const char * address, int32_t portnumber, BOOL reconnection = TRUE);


					void on_create_session(void);
					void on_destroy_session(void);

				private:
					sirius::library::net::sicp::client *	_front;
					int32_t									_io_thread_pool_count;
				};
			};
		};
	};
};









#endif
