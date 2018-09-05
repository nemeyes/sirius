#ifndef _SIRIUS_ABSTRACT_SICP_SERVER_H_
#define _SIRIUS_ABSTRACT_SICP_SERVER_H_

#include <iocp_session.h>
#include <iocp_server.h>
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
				class abstract_server
					: public sirius::library::net::iocp::server
					, public sirius::library::net::sicp::base
				{
				public:
					static const int32_t MAXIUM_CLOSING_SESSION_WAITING_INTERVAL	= 100;
					static const int32_t MAXIUM_REGISTING_SESSION_WAITING_INTERVAL	= 1000;
					static const int32_t MINIMUM_KEEPALIVE_INTERVAL					= 5000;
					static const int32_t KEEPALIVE_MARGIN							= 1000;

				public:
					abstract_server(const char * uuid, int32_t command_thread_pool_count, BOOL keepalive, int32_t keepalive_timeout, int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, BOOL tls, int32_t max_sessions);
					virtual ~abstract_server(void);

					int32_t			initialize(void);
					int32_t			release(void);

					const char *	uuid(void);
					void			uuid(const char * uuid);

					std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>> activated_sessions(void);

					void			on_data_indication(const char * dst, const char * src, int32_t command_id, uint8_t version, const char * packet, int32_t packet_size, std::shared_ptr<sirius::library::net::sicp::session> session);
					void			data_request(const char * dst, int32_t command_id, const char * packet, int32_t packet_size);
					void			data_request(const char * dst, const char * src, int32_t command_id, const char * packet, int32_t packet_size);


					int32_t			clean_handshaking_session(BOOL force_clean);
					int32_t			clean_connected_session(BOOL force_clean);
					int32_t			clean_activated_session(BOOL force_clean);
					int32_t			clean_closing_session(BOOL force_clean);

					bool			activate_session(/*const char * uuid, */std::shared_ptr<sirius::library::net::sicp::session> session);
					bool			deactivate_session(std::shared_ptr<sirius::library::net::sicp::session> session);
					bool			check_activate_session(const char * uuid);

					void			add_command(sirius::library::net::sicp::abstract_command * command);
					void			remove_command(int32_t command_id);

					void			on_create_session(const char * uuid, std::shared_ptr<sirius::library::net::sicp::session> session);
					void			on_destroy_session(const char * uuid, std::shared_ptr<sirius::library::net::sicp::session> session);

					virtual void	on_create_session(const char * uuid) = 0;
					virtual void	on_destroy_session(const char * uuid) = 0;

					virtual std::shared_ptr<sirius::library::net::iocp::session>	create_session(int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, BOOL tls = FALSE, SSL_CTX * ssl_ctx = NULL, BOOL reconnection = FALSE);
					virtual void													destroy_session(std::shared_ptr<sirius::library::net::iocp::session> session);
					virtual void	on_start(void);
					virtual void	on_stop(void);
					virtual void	on_running(void);


				protected:
					void			clean_command_list(void);

					virtual void	on_app_session_handshaking(std::shared_ptr<sirius::library::net::iocp::session> session);
					virtual void	on_app_session_connect(std::shared_ptr<sirius::library::net::iocp::session> session);
					virtual void	on_app_session_close(std::shared_ptr<sirius::library::net::iocp::session> session);


				protected:
					BOOL																		_keepalive;
					int32_t																		_keepalive_timeout;
					char																		_uuid[64];
					int32_t																		_sequence;

					CRITICAL_SECTION															_handshaking_slock;
					CRITICAL_SECTION															_connected_slock;
					CRITICAL_SECTION															_closing_slock;
					CRITICAL_SECTION															_active_slock;

					std::vector<std::shared_ptr<sirius::library::net::sicp::session>>			_handshaking_sessions;
					std::vector<std::shared_ptr<sirius::library::net::sicp::session>>			_connected_sessions;
					std::vector<std::shared_ptr<sirius::library::net::sicp::session>>			_closing_sessions;
					std::map<std::string, std::shared_ptr<sirius::library::net::sicp::session>>	_activated_sessions;
					std::map<int32_t, sirius::library::net::sicp::abstract_command*>			_commands;

					int32_t																		_max_sessions;

					

				private:
					abstract_server(sirius::library::net::sicp::abstract_server & clone);
				};	
			};
		};
	};
};







#endif
