#ifndef _IOCP_TCP_CLIENT_H_
#define _IOCP_TCP_CLIENT_H_

#include "iocp_client.h"

namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace iocp
			{
				namespace tcp
				{
					class client
						: public sirius::library::net::iocp::client
					{
					public:
						client(sirius::library::net::client * processor, int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size, int32_t recv_buffer_size, int32_t transport_type, bool dynamic_alloc = false);
						virtual ~client(void);

						virtual std::shared_ptr<sirius::library::net::session> connect(const char * address, int32_t port_number, int32_t io_thread_pool_count);
						virtual int32_t disconnect(void);
						virtual void close(std::shared_ptr<sirius::library::net::session> session);

						virtual bool recv_completion_callback(std::shared_ptr<sirius::library::net::session> session, std::shared_ptr<sirius::library::net::session::recv_buffer_t> context, int32_t nbytes);
						virtual bool send_completion_callback(std::shared_ptr<sirius::library::net::session> session, std::shared_ptr<sirius::library::net::session::send_buffer_t> context, int32_t nbytes);
						virtual bool other_completion_callback(std::shared_ptr<sirius::library::net::session> session, int32_t nbytes);

						virtual bool post_recv(std::shared_ptr<sirius::library::net::session> session, int32_t size);
						virtual bool post_send(std::shared_ptr<sirius::library::net::session> session, std::vector<std::shared_ptr<sirius::library::net::session::send_buffer_t>> sendQ);
						virtual void execute(void);
					};
				}
			}
		}
	}
}
#endif