#ifndef _SIRIUS_SICP_SERVER_H_
#define _SIRIUS_SICP_SERVER_H_

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
				class server
				{
				public:
					class core;
				public:
					server(const char * uuid, int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, int32_t io_thread_pool_count = 0, int32_t command_thread_pool_count = 0, BOOL keepalive = FALSE, int32_t keepalive_timeout = 5000, BOOL tls = FALSE, int32_t max_sessions = 1000);
					virtual ~server(void);

					void	uuid(const char * uuid);
					int32_t start(char * address, int32_t port_number);
					int32_t stop(void);
					bool is_valid(const char * uuid);

					void	data_request(char * dst, int32_t command_id, const char * packet, int32_t packet_size);
					void	add_command(sirius::library::net::sicp::abstract_command * command);
					void	add_command(int32_t forworded_message_id);

					virtual void on_create_session(const char * uuid) = 0;
					virtual void on_destroy_session(const char * uuid) = 0;

				private:
					sirius::library::net::sicp::server::core *	_server;
					int32_t		_io_thread_pool_count;
					bool		_dynamic_alloc;
				};
			};
		};
	};
};

#endif