#ifndef _IOCP_UDP_SERVER_H_
#define _IOCP_UDP_SERVER_H_
#include "iocp_server.h"

namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace iocp
			{
				namespace udp
				{
					class server
						: public sirius::library::net::iocp::server
					{
					public:
						server(sirius::library::net::server * processor, int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size, int32_t recv_buffer_size, bool multicast = false, bool dynamic_alloc = false);
						virtual ~server(void);

						virtual int32_t start(char * address, int32_t port_number, int32_t thread_pool_count = 0);
						virtual int32_t stop(void);

						virtual void	close(std::shared_ptr<sirius::library::net::session> session);

						virtual bool	recv_completion_callback(std::shared_ptr<sirius::library::net::session> session, std::shared_ptr<sirius::library::net::session::recv_buffer_t> context, int32_t nbytes);
						virtual bool	send_completion_callback(std::shared_ptr<sirius::library::net::session> session, std::shared_ptr<sirius::library::net::session::send_buffer_t> context, int32_t nbytes);
						virtual bool	other_completion_callback(std::shared_ptr<sirius::library::net::session> session, int32_t nbytes);

						virtual bool	post_recv(std::shared_ptr<sirius::library::net::session> session, int32_t size);
						virtual bool	post_send(std::shared_ptr<sirius::library::net::session> session, std::vector<std::shared_ptr<sirius::library::net::session::send_buffer_t>> sendQ);
						virtual void	execute(void);

					private:
						bool		_multicast;
						SOCKADDR_IN _listen_socket_addr;
					};
				}
			}
		}
	}
}
#endif