#ifndef _SIRIUS_SICP_CLIENT_H_
#define _SIRIUS_SICP_CLIENT_H_

#include <winsock2.h>
#include <windows.h>
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
					client(int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, int32_t io_thread_pool_count = 1, int32_t command_thread_pool_count = 3, BOOL keepalive = FALSE, BOOL tls = FALSE);
					client(const char * uuid, int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, int32_t io_thread_pool_count = 1, int32_t command_thread_pool_count = 3, BOOL keepalive = FALSE, BOOL tls = FALSE);
					virtual ~client(void);

					int32_t connect(const char * address, int32_t portnumber, BOOL reconnection = TRUE);
					int32_t disconnect(void);

					void data_request(const char * dst, int32_t command_id, const char * packet, int32_t packet_size);
					void add_command(abstract_command * command);
					void add_command(int32_t forword_message_id);

					virtual void on_create_session(void)	= 0;
					virtual void on_destroy_session(void)	= 0;

				private:
					sirius::library::net::sicp::client::core * _client;
				};
			};
		};
	};
};




#endif
