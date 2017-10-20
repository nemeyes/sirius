#ifndef _EPOLL_CLIENT_H_
#define _EPOLL_CLIENT_H_

#include <platform.h>
#include <abstract_session.h>
#include <epoll_handler.h>

namespace amadeus
{
	namespace library
	{
		namespace net
		{
			class client;
            namespace epoll
            {
                class client
                {
                public:

                    client(amadeus::library::net::client * processor, int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size,int32_t recv_buffer_size, bool dynamic_alloc);
                    virtual ~client(void);

                    //Common
                    bool initialize(void);
                    void release(void);
                    bool disconnect(void);
                    void close(std::shared_ptr<amadeus::library::net::session> session);
                    bool recv_completion_callback(std::shared_ptr<amadeus::library::net::session> session, std::shared_ptr<amadeus::library::net::session::recv_buffer_t> context, int32_t nbytes);
                    bool send_completion_callback(std::shared_ptr<amadeus::library::net::session> session, std::shared_ptr<amadeus::library::net::session::send_buffer_t> context, int32_t nbytes);
                    bool other_completion_callback(std::shared_ptr<amadeus::library::net::session> session, int32_t nbytes);
                    bool post_recv(std::shared_ptr<amadeus::library::net::session> session, int32_t size);
                    bool post_send(std::shared_ptr<amadeus::library::net::session> session, std::vector<std::shared_ptr<amadeus::library::net::session::send_buffer_t>> sendQ);

                    //To be implemented
                    virtual std::shared_ptr<amadeus::library::net::session> connect(const char * address, int32_t port_number, int32_t io_thread_pool_count ) = 0;
                    virtual void execute(void) = 0;

                protected:
                    std::shared_ptr<amadeus::library::net::session> allocate_session(SOCKET socket);

                    amadeus::library::net::client *	_processor;
                    int32_t					_mtu;
                    int32_t					_so_rcvbuf_size;
                    int32_t					_so_sndbuf_size;
                    int32_t					_recv_buffer_size;
                    bool					_dynamic_alloc;
                    iocp_handler *			_iocp;
                };
            };
		};
	};
};

#endif
