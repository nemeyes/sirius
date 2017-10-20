#ifndef _IOCP_SERVER_H_
#define _IOCP_SERVER_H_

#include <sirius.h>
#include <platform.h>
#include <abstract_session.h>
#include <iocp_handler.h>

namespace sirius
{
	namespace library
	{
		namespace net
		{
			class server;
			namespace iocp
			{
				class server
					: public sirius::base
				{
				public:
					server(sirius::library::net::server * processor, int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size, int32_t recv_buffer_size, bool dynamic_alloc = false);
					virtual ~server(void);

					int32_t initialize(void);
					int32_t release(void);
				
					virtual int32_t start(char * address, int32_t port_number, int32_t thread_pool_count = 0) = 0;
					virtual int32_t stop(void) = 0;
					virtual void	close(std::shared_ptr<sirius::library::net::session> session) = 0;

					virtual bool	recv_completion_callback(std::shared_ptr<sirius::library::net::session> session, std::shared_ptr<sirius::library::net::session::recv_buffer_t> context, int32_t nbytes) = 0;
					virtual bool	send_completion_callback(std::shared_ptr<sirius::library::net::session> session, std::shared_ptr<sirius::library::net::session::send_buffer_t> context, int32_t nbytes) = 0;
					virtual bool	other_completion_callback(std::shared_ptr<sirius::library::net::session> session, int32_t nbytes) = 0;

					virtual bool	post_recv(std::shared_ptr<sirius::library::net::session> session, int32_t size) = 0;
					virtual bool	post_send(std::shared_ptr<sirius::library::net::session> session, std::vector<std::shared_ptr<sirius::library::net::session::send_buffer_t>> sendQ) = 0;
					virtual void	execute(void) = 0;
					
					//void start_streaming(void);
				protected:
					std::shared_ptr<sirius::library::net::session> allocate_session(SOCKET client_socket);
					
				protected:
					//Mandatory
					SOCKET									_listen_socket;
					sirius::library::net::server *			_processor;
					sirius::library::net::iocp::handler *	_iocp;

					int32_t		_mtu;
					int32_t		_so_rcvbuf_size;
					int32_t		_so_sndbuf_size;
					int32_t		_recv_buffer_size;
					bool		_dynamic_alloc;
				};
			};
		};
	};
};





#endif