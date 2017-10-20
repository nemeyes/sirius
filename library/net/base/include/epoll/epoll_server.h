#ifndef _IOCP_SERVER_H_
#define _IOCP_SERVER_H_

#include <platform.h>
#include <abstract_session.h>
#include <iocp_handler.h>

namespace amadeus
{
	namespace library
	{
		namespace net
		{
			class server;
			class iocp_server
			{
			public:
				iocp_server(amadeus::library::net::server * processor, int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size, int32_t recv_buffer_size,bool dynamic_alloc = false);
				virtual ~iocp_server(void);

				//To be implemented
				virtual bool start(char * address, int32_t port_number, int32_t thread_pool_count = 0) = 0;
				virtual bool post_recv(std::shared_ptr<amadeus::library::net::session> session, int32_t size) = 0;
				virtual bool post_send(std::shared_ptr<amadeus::library::net::session> session, std::vector<std::shared_ptr<amadeus::library::net::session::send_buffer_t>> sendQ) = 0;
				virtual void execute(void) = 0; 
				
				//Common
				bool initialize(void);
				void release(void);
				virtual void close(std::shared_ptr<amadeus::library::net::session> session);
				bool recv_completion_callback(std::shared_ptr<amadeus::library::net::session> session, std::shared_ptr<amadeus::library::net::session::recv_buffer_t> context, int32_t nbytes);
				bool send_completion_callback(std::shared_ptr<amadeus::library::net::session> session, std::shared_ptr<amadeus::library::net::session::send_buffer_t> context, int32_t nbytes);
				bool other_completion_callback(std::shared_ptr<amadeus::library::net::session> session, int32_t nbytes);
				std::shared_ptr<amadeus::library::net::session> allocate_session(SOCKET client_socket);
				void start_streaming(void);
				
				virtual bool stop(void);
			protected:
				//Mandatory
				SOCKET	_listen_socket;
				amadeus::library::net::server *	_processor;
				amadeus::library::net::iocp_handler *		_iocp;

				int32_t	_mtu;
				int32_t	_so_rcvbuf_size;
				int32_t	_so_sndbuf_size;
				int32_t _recv_buffer_size;
				bool	_dynamic_alloc;
			};
		};
	};
};





#endif