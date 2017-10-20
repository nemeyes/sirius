#ifndef _IOCP_TCP_CLIENT_H_
#define _IOCP_TCP_CLIENT_H_
#include "iocp_client.h"
namespace amadeus
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
						: public amadeus::library::net::iocp_client
					{
					public:
						client(amadeus::library::net::client * processor, int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size, int32_t recv_buffer_size, int32_t transport_type, bool dynamic_alloc = false);
						virtual ~client(void);

						std::shared_ptr<amadeus::library::net::session> connect(const char * address, int32_t port_number, int32_t io_thread_pool_count);
						void execute(void);
					};
				}
			}
		}
	}
}
#endif