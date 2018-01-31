#ifndef _SIRIUS_ABSTRACT_SICP_CLIENT_H_
#define _SIRIUS_ABSTRACT_SICP_CLIENT_H_

#include <iocp_session.h>
#include <iocp_client.h>
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
					: public sirius::library::net::iocp::client
					, public sirius::library::net::sicp::base
				{
				public:
					static const int32_t KEEPALIVE_INTERVAL			= 5000;
					static const int32_t KEEPALIVE_INTERVAL_MARGIN	= 2000;


				public:
					abstract_client(int32_t command_thread_pool_count, BOOL keepalive, int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, BOOL tls);
					abstract_client(const char * uuid, int32_t command_thread_pool_count, BOOL keepalive, int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, BOOL tls);
					virtual ~abstract_client(void);

					int32_t			initialize(void);
					int32_t			release(void);

					int32_t			connect(const char * address, int32_t portnumber, int32_t io_thread_pool_count, BOOL reconnection = TRUE);
					int32_t			disconnect(void);
					void			disconnect(BOOL enable);
					BOOL			active(void) const;

					const char *	uuid(void);
					void			uuid(const char * uuid);

					void			on_data_indication(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * packet, int32_t packet_size, std::shared_ptr<sirius::library::net::sicp::session> session);
					void			data_request(const char * dst, int32_t command_id, const char * packet, int32_t packet_size);
					void			data_request(const char * dst, const char * src, int32_t command_id, const char * packet, int32_t packet_size);

					void			add_command(abstract_command * command);
					void			wait_command_thread_end(void);
					void			remove_command(int32_t command_id);

					virtual void	on_create_session(std::shared_ptr<sirius::library::net::sicp::session> session);
					virtual void	on_destroy_session(std::shared_ptr<sirius::library::net::sicp::session> session);
					virtual void	on_create_session(void)		= 0;
					virtual void	on_destroy_session(void)	= 0;

					virtual std::shared_ptr<sirius::library::net::iocp::session> create_session(int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, BOOL tls = FALSE, SSL_CTX * ssl_ctx = NULL, BOOL reconnection = FALSE);
					virtual void												 destroy_session(std::shared_ptr<sirius::library::net::iocp::session> session);
					virtual void	on_start(void);
					virtual void	on_stop(void);
					virtual void	on_running(void);


				protected:
					void			clear_command_list(void);

					virtual void	on_app_session_handshaking(std::shared_ptr<sirius::library::net::iocp::session> session);
					virtual void	on_app_session_connect(std::shared_ptr<sirius::library::net::iocp::session> session);
					virtual void	on_app_session_close(std::shared_ptr<sirius::library::net::iocp::session> session);

				protected:
					BOOL				_keepalive;
					char				_uuid[64];
					CRITICAL_SECTION	_slock;
					std::map<int32_t, sirius::library::net::sicp::abstract_command*> _commands;

				private:
					abstract_client(sirius::library::net::sicp::abstract_client & clone);
					std::shared_ptr<sirius::library::net::sicp::session> _session;
				};
			};
		};
	};
};

#endif
