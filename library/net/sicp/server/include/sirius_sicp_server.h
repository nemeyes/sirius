#ifndef _SIRIUS_SICP_SERVER_H_
#define _SIRIUS_SICP_SERVER_H_

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
					server(const char * uuid, int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size,int32_t recv_buffer_size, int32_t io_thread_pool_count = 0, int32_t command_thread_pool_count = 0, bool use_keep_alive = true, bool dynamic_alloc = false);
					virtual ~server(void);

					void uuid(const char * uuid);
					bool start(char * address, int32_t port_number);
					bool stop(void);
					bool is_valid(const char * uuid);

					void data_request(char * dst, int32_t command_id, char * msg, int32_t length);
					void add_command(sirius::library::net::sicp::abstract_command * command);
					void add_command(int32_t forworded_message_id);

					virtual void create_session_callback(const char * uuid) = 0;
					virtual void destroy_session_callback(const char * uuid) = 0;

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