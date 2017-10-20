#ifndef _ABSTRACT_SERVER_H_
#define _ABSTRACT_SERVER_H_

#include <platform.h>
#include <sirius.h>
#if defined(WIN32)
# include <iocp_server.h>
#elif defined(__linux__)
# include <epoll_server.h>
#elif defined(__MACOS__)
# include <kqeue_server.h>
#endif
#include <abstract_session.h>

namespace sirius
{
	namespace library
	{
		namespace net
		{
#if defined(WIN32)
			namespace iocp
			{
				class server;
			};
#elif defined(__linux__)
			namespace epoll
			{
				class server;
			};
#elif defined(__MACOS__) || defined(__FreeBSD__)
			namespace kqueue
			{
				class server;
			};
#endif
			class server
				: public sirius::base
			{
			public:
				server(int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size,int32_t recv_buffer_size, bool dynamic_alloc, int32_t type, bool multicast);
				virtual ~server(void);

				bool	start(char * address, int32_t port_number, int32_t io_thread_pool_count);
				bool	stop(void);

				bool	is_run(void) const;

				void	data_request(std::shared_ptr<sirius::library::net::session> session, std::vector<std::shared_ptr<sirius::library::net::session::send_buffer_t>> sendQ);
				

				virtual std::shared_ptr<sirius::library::net::session> create_session_callback(SOCKET client_socket, int32_t mtu, int32_t recv_buffer_size,bool dynamic_alloc = false) = 0;
				virtual void											destroy_session_callback(std::shared_ptr<sirius::library::net::session> session) = 0;

			protected:
				static unsigned __stdcall	process_cb(void * param);
				virtual void				process(void) = 0;

			protected:
				char				_address[128];
				int32_t				_port_number;
				int32_t				_io_thread_pool_count;
				HANDLE				_thread;
				bool				_run;
				CRITICAL_SECTION	_conn_sessions_cs;

#if defined(WIN32)
				sirius::library::net::iocp::server *	_server;
#elif defined(__linux__)
				sirius::library::net::epoll::server *	_server;
#elif defined(__MACOS__) || defined(__FreeBSD__)
				sirius::library::net::kqueue::server *	_server;
#endif
			public:
				void	register_conn_client(std::shared_ptr<sirius::library::net::session> session);
				void	unregister_conn_client(std::shared_ptr<sirius::library::net::session> session);
				void	clear_conn_client(void);

				std::vector<std::shared_ptr<sirius::library::net::session>>	_conn_sessions;
			};
		};
	};
};



#endif