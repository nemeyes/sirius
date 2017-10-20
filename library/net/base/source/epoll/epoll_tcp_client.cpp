#include "abstract_client.h"
#include "iocp_tcp_client.h"

amadeus::library::net::iocp::tcp::client::client(amadeus::library::net::client * processor, int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size, int32_t recv_buffer_size, int32_t transport_type, bool dynamic_alloc)
	: iocp_client(processor, mtu, so_rcvbuf_size, so_sndbuf_size, recv_buffer_size, dynamic_alloc)
{

}
amadeus::library::net::iocp::tcp::client::~client(void)
{

}

std::shared_ptr<amadeus::library::net::session> amadeus::library::net::iocp::tcp::client::connect(const char * address, int32_t port_number, int32_t io_thread_pool_count)
{
	SOCKADDR_IN	serv_addr;
	SOCKET socket = INVALID_SOCKET;

	int32_t err_code;
	if (!_iocp->create(io_thread_pool_count, &err_code))
		return std::shared_ptr<amadeus::library::net::session>();
	_iocp->create_thread_pool();

	int32_t value = -1;
	int32_t zero = 1;

	socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED); 

	if (socket == INVALID_SOCKET)
		return std::shared_ptr<amadeus::library::net::session>();

	memset(&serv_addr, 0x00, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(address);
	serv_addr.sin_port = htons(port_number);

	value = WSAConnect(socket, (SOCKADDR*)(&serv_addr), sizeof(serv_addr), NULL, NULL, NULL, NULL);
	if ((value == SOCKET_ERROR) && (WSAGetLastError() != WSAEWOULDBLOCK))
	{
		closesocket(socket);
		return std::shared_ptr<amadeus::library::net::session>();
	}

	if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&zero, sizeof(int)) == SOCKET_ERROR)
	{
		closesocket(socket);
		return std::shared_ptr<amadeus::library::net::session>();
	}

	zero = _so_rcvbuf_size;
	if (setsockopt(socket, SOL_SOCKET, SO_RCVBUF, (const char*)&zero, sizeof(int)) == SOCKET_ERROR)
	{
		closesocket(socket);
		return std::shared_ptr<amadeus::library::net::session>();
	}
	zero = _so_sndbuf_size;
	if (setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (const char*)&zero, sizeof(int)) == SOCKET_ERROR)
	{
		closesocket(socket);
		return std::shared_ptr<amadeus::library::net::session>();
	}

	zero = 0;
	if (setsockopt(socket, SOL_SOCKET, TCP_NODELAY, (char *)&zero, sizeof(int)) == SOCKET_ERROR) //disable nagle algorithm
	{
		closesocket(socket);
		return std::shared_ptr<amadeus::library::net::session>();
	}

	std::shared_ptr<amadeus::library::net::session> session = allocate_session(socket);
	if (!_iocp->associate((session)->fd(), reinterpret_cast<ULONG_PTR>(session.get()), &err_code))
	{
		session->close();
		return std::shared_ptr<amadeus::library::net::session>();
	}

	if (!post_recv(session, session->get_first_recv_packet_size()))
	{
		session->close();
		return std::shared_ptr<amadeus::library::net::session>();
	}

	_processor->enable_close_waiting_flag(false);
	return session;
}

void amadeus::library::net::iocp::tcp::client::execute(void)
{
	amadeus::library::net::session * session = NULL;
	amadeus::library::net::io_context_t * io_context = NULL;
	DWORD nbytes = 0;
	int32_t err_code = 0;

	while (1)
	{
		// IO Completion Packet 얻어온다.
		bool value = _iocp->get_completion_status(reinterpret_cast<ULONG_PTR*>(&session), &nbytes, reinterpret_cast<LPOVERLAPPED*>(&io_context), &err_code);
		if (value)
		{
			if (((int32_t)session) == KILL_THREAD)
				break;
		}

		do
		{
			session->update_last_access_tm();
			if (session->fd() == INVALID_SOCKET)
				break;

			if (nbytes == 0)
			{
				_processor->enable_close_waiting_flag(false);
				close(session->shared_from_this());
				break;
			}

			if (err_code)
			{
				_processor->enable_close_waiting_flag(true);
				session->close();
				break;
			}

			// IO 성격에 따라 그에 따른 처리
			if (io_context->operation == amadeus::library::net::io_context_t::operation_t::recv)
			{
				amadeus::library::net::session::recv_buffer_t * recv_buffer = static_cast<amadeus::library::net::session::recv_buffer_t*>(io_context);
				if (!recv_completion_callback(session->shared_from_this(), recv_buffer->shared_from_this(), nbytes))
				{
					//_processor->enable_disconnect_flag(true);
					_processor->enable_close_waiting_flag(true);
					session->close();
				}
			}
			else if (io_context->operation == amadeus::library::net::io_context_t::operation_t::send)
			{
				amadeus::library::net::session::send_buffer_t * send_buffer = static_cast<amadeus::library::net::session::send_buffer_t*>(io_context);
				if (!send_completion_callback(session->shared_from_this(), send_buffer->shared_from_this(), nbytes))
				{
					//_processor->enable_disconnect_flag(true);
					_processor->enable_close_waiting_flag(true);
					session->close();
				}
			}
			else
			{
				//_processor->enable_disconnect_flag(true);
				_processor->enable_close_waiting_flag(true);
				session->close();
			}

		} while (false);
	}
}