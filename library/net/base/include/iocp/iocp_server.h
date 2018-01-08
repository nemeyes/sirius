#ifndef _IOCP_SERVER_H_
#define _IOCP_SERVER_H_

#include <sirius.h>
#include <platform.h>
#include <iocp_session.h>
#include <iocp_handler.h>

struct CRYPTO_dynlock_value
{
	CRITICAL_SECTION lock;
};

namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace iocp
			{
				class server : public sirius::base
				{
					friend class sirius::library::net::iocp::handler;
				public:
					server(int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, BOOL tls = FALSE);
					virtual ~server(void);

					int32_t		initialize(void);
					int32_t		release(void);

					int32_t		start(char * address, int32_t port_number, int32_t io_thread_pool_count = 0);
					int32_t		stop(void);

					void		accept_session(std::shared_ptr<sirius::library::net::iocp::session> session);
					std::shared_ptr<sirius::library::net::iocp::session> accept_session(void);

					BOOL		active(void) const;
					BOOL		associate(SOCKET socket, ULONG_PTR key, int32_t * err_code);
					void		data_request(std::shared_ptr<sirius::library::net::iocp::session> session, const char * packet, int32_t packet_size);

					void		on_session_connect(std::shared_ptr<sirius::library::net::iocp::session> session);
					void		on_session_close(std::shared_ptr<sirius::library::net::iocp::session> session);

					virtual void	on_app_session_connect(std::shared_ptr<sirius::library::net::iocp::session> session)	= 0;
					virtual void	on_app_session_close(std::shared_ptr<sirius::library::net::iocp::session> session)		= 0;
					virtual std::shared_ptr<sirius::library::net::iocp::session>	create_session(int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, BOOL tls = FALSE, SSL_CTX * ssl_ctx = NULL, BOOL reconnection = FALSE) = 0;
					virtual void													destroy_session(std::shared_ptr<sirius::library::net::iocp::session> session) = 0;
					virtual void	on_start(void)		= 0;
					virtual void	on_running(void)	= 0;
					virtual void	on_stop(void)		= 0;
					
				private:
					void							execute(void);

					void							initialization_tls(void);
					void							release_tls(void);
					void							set_certificate(void);

					static void						ssl_lock_callback(int mode, int n, const char * file, int line);
					static CRYPTO_dynlock_value *	ssl_lock_dyn_create_callback(const char * file, int line);
					static void						ssl_lock_dyn_callback(int mode, CRYPTO_dynlock_value * l, const char * file, int line);
					static void						ssl_lock_dyn_destroy_callback(CRYPTO_dynlock_value * l, const char * file, int line);

					SOCKET							create_listen_socket(int32_t portnumber);
					static unsigned __stdcall		process_cb(void * param);
					virtual void					process(void);
					
				protected:
					sirius::library::net::iocp::handler *					_iocp;
					int32_t													_so_recv_buffer_size;
					int32_t													_so_send_buffer_size;
					int32_t													_recv_buffer_size;
					int32_t													_send_buffer_size;
					
					char													_address[MAX_PATH];
					int32_t													_portnumber;
					int32_t													_io_thread_pool_count;
					HANDLE													_thread;
					BOOL													_run;
					std::shared_ptr<sirius::library::net::iocp::session>	_accept_session;

					BOOL													_tls;
					static int32_t											_nssl_locks;
					static CRITICAL_SECTION *								_ssl_locks;
					static SSL_CTX *										_ssl_ctx;

					static const char *										_ca_cert_key_pem;
					static const char *										_server_cert_key_pem;
				};
			};
		};
	};
};





#endif