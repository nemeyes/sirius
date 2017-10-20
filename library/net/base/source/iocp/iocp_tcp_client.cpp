#include "abstract_client.h"
#include "iocp_tcp_client.h"

sirius::library::net::iocp::tcp::client::client(sirius::library::net::client * processor, int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size, int32_t recv_buffer_size, int32_t transport_type, bool dynamic_alloc)
	: sirius::library::net::iocp::client(processor, mtu, so_rcvbuf_size, so_sndbuf_size, recv_buffer_size, dynamic_alloc)
{

}
sirius::library::net::iocp::tcp::client::~client(void)
{

}

std::shared_ptr<sirius::library::net::session> sirius::library::net::iocp::tcp::client::connect(const char * address, int32_t port_number, int32_t io_thread_pool_count)
{
	SOCKADDR_IN	serv_addr;
	SOCKET socket = INVALID_SOCKET;

	int32_t err_code;
	if (!_iocp->create(io_thread_pool_count, &err_code))
		return std::shared_ptr<sirius::library::net::session>();
	_iocp->create_thread_pool();

	int32_t value = -1;
	int32_t zero = 1;

	socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED); 

	if (socket == INVALID_SOCKET)
		return std::shared_ptr<sirius::library::net::session>();

	memset(&serv_addr, 0x00, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(address);
	serv_addr.sin_port = htons(port_number);

	value = WSAConnect(socket, (SOCKADDR*)(&serv_addr), sizeof(serv_addr), NULL, NULL, NULL, NULL);
	if ((value == SOCKET_ERROR) && (WSAGetLastError() != WSAEWOULDBLOCK))
	{
		closesocket(socket);
		return std::shared_ptr<sirius::library::net::session>();
	}

	if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&zero, sizeof(int)) == SOCKET_ERROR)
	{
		closesocket(socket);
		return std::shared_ptr<sirius::library::net::session>();
	}

	zero = _so_rcvbuf_size;
	if (setsockopt(socket, SOL_SOCKET, SO_RCVBUF, (const char*)&zero, sizeof(int)) == SOCKET_ERROR)
	{
		closesocket(socket);
		return std::shared_ptr<sirius::library::net::session>();
	}
	zero = _so_sndbuf_size;
	if (setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (const char*)&zero, sizeof(int)) == SOCKET_ERROR)
	{
		closesocket(socket);
		return std::shared_ptr<sirius::library::net::session>();
	}

	zero = 0;
	if (setsockopt(socket, SOL_SOCKET, TCP_NODELAY, (char *)&zero, sizeof(int)) == SOCKET_ERROR) //disable nagle algorithm
	{
		closesocket(socket);
		return std::shared_ptr<sirius::library::net::session>();
	}

	std::shared_ptr<sirius::library::net::session> session = allocate_session(socket);
	if (!_iocp->associate((session)->fd(), reinterpret_cast<ULONG_PTR>(session.get()), &err_code))
	{
		session->close();
		return std::shared_ptr<sirius::library::net::session>();
	}

	if (!post_recv(session, session->get_first_recv_packet_size()))
	{
		session->close();
		return std::shared_ptr<sirius::library::net::session>();
	}

	_processor->enable_close_waiting_flag(false);
	return session;
}

int32_t sirius::library::net::iocp::tcp::client::disconnect(void)
{
	_iocp->close_thread_pool();
	return true;
}

void sirius::library::net::iocp::tcp::client::close(std::shared_ptr<sirius::library::net::session> session)
{
	if (session->fd() != INVALID_SOCKET)
		session->close();
	_processor->destroy_session_callback(session);
}


bool sirius::library::net::iocp::tcp::client::recv_completion_callback(std::shared_ptr<sirius::library::net::session> session, std::shared_ptr<sirius::library::net::session::recv_buffer_t> context, int32_t nbytes)
{
	int32_t length = session->push_recv_packet(context->packet, nbytes);
	if (length < 0)
		return false;

	if (session->fd() != INVALID_SOCKET && context.get() != NULL)
	{
		bool value = post_recv(session, length);
		if (!value)
			return false;
	}
	return true;
}

bool sirius::library::net::iocp::tcp::client::send_completion_callback(std::shared_ptr<sirius::library::net::session> session, std::shared_ptr<sirius::library::net::session::send_buffer_t> context, int32_t nbytes)
{
	session->pop_send_buffer(context);
	return true;
}

bool sirius::library::net::iocp::tcp::client::other_completion_callback(std::shared_ptr<sirius::library::net::session> session, int32_t nbytes)
{
	// ���� ����� ���� Recv , Send �̿��� �̻��� ������ ����Ŵ ���� ���������.
	//session->shutdown_fd();
	return true;
}

bool sirius::library::net::iocp::tcp::client::post_recv(std::shared_ptr<sirius::library::net::session> session, int32_t size)
{
	DWORD nbytes = 0;
	DWORD flags = 0;

	if ((session->fd() != INVALID_SOCKET))
	{
		if (size > session->recv_context()->packet_size)
		{
			session->recv_context()->resize(size);
		}

		session->recv_context()->wsabuf.buf = session->recv_context()->packet;
		session->recv_context()->wsabuf.len = size;
		int value = WSARecv(session->fd(), &(session->recv_context()->wsabuf), 1, &nbytes, &flags, &(session->recv_context()->overlapped), NULL);
		if (SOCKET_ERROR == value)
		{
			int err_code = WSAGetLastError();
			if (err_code != WSA_IO_PENDING)
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

bool sirius::library::net::iocp::tcp::client::post_send(std::shared_ptr<sirius::library::net::session> session, std::vector<std::shared_ptr<sirius::library::net::session::send_buffer_t>> sendQ)
{
	DWORD nbytes = 0;
	DWORD flags = 0;

	bool status = true;
	if ((session->fd() != INVALID_SOCKET))
	{
		std::vector<std::shared_ptr<sirius::library::net::session::send_buffer_t>>::iterator iter;
		for (iter = sendQ.begin(); iter != sendQ.end(); iter++)
		{
			std::shared_ptr<sirius::library::net::session::send_buffer_t> send_buffer = *iter;
			if (send_buffer)
			{
				send_buffer->wsabuf.buf = (CHAR*)send_buffer->packet;
				send_buffer->wsabuf.len = send_buffer->send_size;
				int32_t value = WSASend(session->fd(), &(send_buffer->wsabuf), 1, &nbytes, flags, &(send_buffer->overlapped), NULL);
				if (SOCKET_ERROR == value)
				{
					int32_t err_code = WSAGetLastError();
					if (err_code != WSA_IO_PENDING)
					{
						status = false;
					}
				}
			}
		}
		session->push_back_send_buffer(sendQ);
	}
	return status;
}

void sirius::library::net::iocp::tcp::client::execute(void)
{
	sirius::library::net::session * session = NULL;
	sirius::library::net::iocp::io_context_t * io_context = NULL;
	DWORD nbytes = 0;
	int32_t err_code = 0;

	while (1)
	{
		// IO Completion Packet ���´�.
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

			// IO ���ݿ� ���� �׿� ���� ó��
			if (io_context->operation == sirius::library::net::iocp::io_context_t::operation_t::recv)
			{
				sirius::library::net::session::recv_buffer_t * recv_buffer = static_cast<sirius::library::net::session::recv_buffer_t*>(io_context);
				if (!recv_completion_callback(session->shared_from_this(), recv_buffer->shared_from_this(), nbytes))
				{
					//_processor->enable_disconnect_flag(true);
					_processor->enable_close_waiting_flag(true);
					session->close();
				}
			}
			else if (io_context->operation == sirius::library::net::iocp::io_context_t::operation_t::send)
			{
				sirius::library::net::session::send_buffer_t * send_buffer = static_cast<sirius::library::net::session::send_buffer_t*>(io_context);
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