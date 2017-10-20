#ifndef _ABSTRACT_CLIENT_H_
#define _ABSTRACT_CLIENT_H_

#include <platform.h>
#include <sirius.h>
#if defined(WIN32)
# include <iocp_udp_client.h>
# include <iocp_tcp_client.h>
#elif defined(__linux__)
# include <epoll_client.h>
#elif defined(__MAXOS__) || defined(__FreeBSD__)
# include <kqueue_client.h>
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
				class client;
			};
#elif defined(__linux__)
			namespace epoll
			{
				class client;
			};
#elif defined(__MACOS__) || defined(__FreeBSD__)
			namespace kqueue
			{
				class client;
			};
#endif
			class client
				: public sirius::base
			{
			public:
				client(int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size,int32_t recv_buffer_size, bool dynamic_alloc, int32_t type, bool multicast);
				virtual ~client(void);

				bool	connect(const char * address, int32_t port_number, int32_t io_thread_pool_count, bool reconnection = true);
				bool	disconnect(void);

				void	enable_disconnect_flag(bool enable);
				void	enable_close_waiting_flag(bool enable);


				void	data_request(std::shared_ptr<sirius::library::net::session> session, std::vector<std::shared_ptr<sirius::library::net::session::send_buffer_t>> sendQ);

				virtual std::shared_ptr<sirius::library::net::session> create_session_callback(SOCKET client_socket, int32_t mtu, int32_t recv_buffer_size, bool dynamic_alloc = false) = 0;
				virtual void destroy_session_callback(std::shared_ptr<sirius::library::net::session> session) = 0;

			protected:
				static unsigned __stdcall process_cb(void * param);
				virtual void process(void) = 0;

			protected:
#if defined(WIN32)
				sirius::library::net::iocp::client *	_client;
#elif defined(__linux__)
				sirius::library::net::epoll::client *	_client;
#elif defined(__MACOS__) || defined(__FreeBSD__)
				sirius::library::net::kqueue::client *	_client;
#endif
				char				_address[128];
				int32_t				_port_number;
				bool				_reconnection;
				HANDLE				_thread;
				bool				_run;
				CRITICAL_SECTION	_cs;
				std::shared_ptr<sirius::library::net::session>	_session;
				bool				_bwaiting;
				int32_t				_io_thread_pool_count;
			};
		};
	};
};

#endif
