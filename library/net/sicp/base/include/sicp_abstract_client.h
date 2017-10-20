#ifndef _SIRIUS_ABSTRACT_SICP_CLIENT_H_
#define _SIRIUS_ABSTRACT_SICP_CLIENT_H_

#include <abstract_client.h>
#include <sicp_base.h>
#include <sicp_session.h>
#include <sicp_command.h>

namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace sicp
			{
				class abstract_client
					: public sirius::library::net::client
					, public sirius::library::net::sicp::base
				{
				public:
					abstract_client(int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size,int32_t recv_buffer_size, int32_t command_thread_pool_count, bool use_keep_alive, bool dynamic_alloc, int32_t type, bool multicast);
					abstract_client(const char * uuid, int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size,int32_t recv_buffer_size, int32_t command_thread_pool_count, bool use_keep_alive, bool dynamic_alloc, int32_t type, bool multicast);
					
					virtual ~abstract_client(void);

					bool connect(const char * address, int32_t port_number, int32_t io_thread_pool_count, bool reconnection = true);
					bool disconnect(void);

					bool is_run(void) const;

					const char * uuid(void);
					void uuid(const char * uuid);

					void data_indication_callback(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * msg, size_t length, std::shared_ptr<sirius::library::net::sicp::session> session);
					void data_request(char * dst, int32_t command_id, char * msg, int32_t length);
					void data_request(char * dst, char * src, int32_t command_id, char * msg, int32_t length);

					std::shared_ptr<sirius::library::net::session> create_session_callback(SOCKET client_socket, int32_t mtu, int32_t recv_buffer_size,bool dynamic_alloc = false);
					void destroy_session_callback(std::shared_ptr<sirius::library::net::session> session);

					void add_command(abstract_command * command);
					void wait_command_thread_end(void);
					void remove_command(int32_t command_id);

					void create_session_completion_callback(std::shared_ptr<sirius::library::net::sicp::session> session);
					void destroy_session_completion_callback(std::shared_ptr<sirius::library::net::sicp::session> session);

					virtual void create_session_callback(void) = 0;
					virtual void destroy_session_callback(void) = 0;

				protected:
					void clear_command_list(void);
					void process(void);

				protected:
					bool				_use_keep_alive;
					char				_uuid[64];
					CRITICAL_SECTION	_session_cs;
					CRITICAL_SECTION	_commands_cs;
					std::map<int32_t, sirius::library::net::sicp::abstract_command*> _commands;

				private:
					abstract_client(sirius::library::net::sicp::abstract_client & clone);
					std::shared_ptr<sirius::library::net::session> _session;
				};
			};
		};
	};
};

#endif
