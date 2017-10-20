#ifndef _IOCP_UDP_SERVER_H_
#define _IOCP_UDP_SERVER_H_
#include "iocp_server.h"

namespace amadeus
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
						: public amadeus::library::net::iocp_server
					{
					public:
						
						server(amadeus::library::net::server * processor, int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size, int32_t recv_buffer_size, int32_t transport_type, bool dynamic_alloc = false);
						virtual ~server();

						virtual bool start(char * address, int32_t port_number, int32_t thread_pool_count = 0);
						virtual bool post_recv(std::shared_ptr<amadeus::library::net::session> session, int32_t size);
						virtual bool post_send(std::shared_ptr<amadeus::library::net::session> session, std::vector<std::shared_ptr<amadeus::library::net::session::send_buffer_t>> sendQ);
						virtual void execute(void);

					private:
						int32_t _transport_type;
						SOCKADDR_IN _listen_socket_addr;
					};
				}
			}
		}
	}
}
#endif