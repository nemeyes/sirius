#include "abstract_server.h"
#include "iocp_tcp_server.h"

amadeus::library::net::iocp::tcp::server::server(amadeus::library::net::server * processor, int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size, int32_t recv_buffer_size, bool dynamic_alloc)
	: amadeus::library::net::iocp_server(processor, mtu, so_rcvbuf_size, so_sndbuf_size, recv_buffer_size, dynamic_alloc)
	, _baccept(false)
{

}

amadeus::library::net::iocp::tcp::server::~server()
{

}

bool amadeus::library::net::iocp::tcp::server::start(char * address, int32_t port_number, int32_t thread_pool_count)
{
	_listen_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (_listen_socket == INVALID_SOCKET)
		return false;

	int resueaddr_enable = 1;
	if (setsockopt(_listen_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&resueaddr_enable, sizeof(resueaddr_enable)) < 0)
	{
		closesocket(_listen_socket);
		return false;
	}

	SOCKADDR_IN sock_addr;
	if (!address || strlen(address)<1)
		sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	else
		sock_addr.sin_addr.s_addr = inet_addr(address);
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons((short)port_number);
	if (bind(_listen_socket, (LPSOCKADDR)&sock_addr, sizeof(sock_addr)))
	{
		closesocket(_listen_socket);
		_listen_socket = INVALID_SOCKET;
		return false;
	}

	if (SOCKET_ERROR == listen(_listen_socket, SOMAXCONN))
	{
		closesocket(_listen_socket);
		_listen_socket = INVALID_SOCKET;
		return false;
	}

	int err_code;
	if (!_iocp->create(thread_pool_count, &err_code))
	{
		closesocket(_listen_socket);
		_listen_socket = INVALID_SOCKET;
		return false;
	}

	_iocp->create_thread_pool();

	if (_listen_socket == INVALID_SOCKET)
		return false;

	unsigned int thread_id;
	_thread = (HANDLE)_beginthreadex(NULL, 0, process_cb, this, 0, &thread_id);

	return true;
}

unsigned __stdcall amadeus::library::net::iocp::tcp::server::process_cb(void * param)
{
	server * self = static_cast<server*>(param);
	self->process();
	return 0;
}

void amadeus::library::net::iocp::tcp::server::process(void)
{
	int err_code = 0;
	int sockaddr_size = sizeof(SOCKADDR_IN);
	SOCKADDR_IN client_sockaddr;

	SOCKET client_socket = INVALID_SOCKET;
	_baccept = true;

	struct timeval tv;
	fd_set rfds_original;
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(_listen_socket, &rfds);
	rfds_original = rfds;

	while (_baccept)
	{
		std::shared_ptr<amadeus::library::net::session> session = NULL;

		rfds = rfds_original;
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		int32_t ret = select(_listen_socket + 1, &rfds, (fd_set *)0, (fd_set *)0, &tv);
		
		if (ret > 0)
			client_socket = accept(_listen_socket, (LPSOCKADDR)&client_sockaddr, &sockaddr_size);
		else
			continue;

		if (client_socket == INVALID_SOCKET)
		{
			// 리슨 소켓을 클로즈 하면 이 에러가 나오므로
			// 이 에러시에 Accept 루프를 빠져나간다.
			if (WSAGetLastError() == WSAEINTR)
				return;
		}

		int32_t resueaddr_enable = 1;
		if (setsockopt(client_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&resueaddr_enable, sizeof(int)) == SOCKET_ERROR)
		{
			closesocket(client_socket);
			continue;
		}

		if (setsockopt(client_socket, SOL_SOCKET, SO_RCVBUF, (const char*)&_so_rcvbuf_size, sizeof(int)) == SOCKET_ERROR)
		{
			closesocket(client_socket);
			continue;
		}

		if (setsockopt(client_socket, SOL_SOCKET, SO_SNDBUF, (const char*)&_so_sndbuf_size, sizeof(int)) == SOCKET_ERROR)
		{
			closesocket(client_socket);
			continue;
		}

		bool tcp_nodelay_enable = false;
		if (setsockopt(client_socket, SOL_SOCKET, TCP_NODELAY, (char *)&tcp_nodelay_enable, sizeof(int)) == SOCKET_ERROR) //disable nagle algorithm
		{
			closesocket(client_socket);
			continue;
		}

		// 소켓 컨텍스트 할당 -> Completion Key
		session = allocate_session(client_socket);
		if (!session)
		{
			session->close();
			continue;
		}

		// IOCP 커널 객체와 연결
		if (!_iocp->associate(client_socket, reinterpret_cast<ULONG_PTR>(session.get()), &err_code))
		{
			session->close();
			continue;
		}

		// 초기 Recv 요청
		bool value = post_recv(session, session->get_first_recv_packet_size());
		if (value == false)
		{
			session->close();
			continue;
		}

		if (_processor)
			_processor->register_conn_client(session);
	}
}

bool amadeus::library::net::iocp::tcp::server::stop(void)
{
	_baccept = false;
	if (::WaitForSingleObject(_thread, INFINITE) == WAIT_OBJECT_0)
		::CloseHandle(_thread);

	return amadeus::library::net::iocp_server::stop();
}

bool amadeus::library::net::iocp::tcp::server::post_recv(std::shared_ptr<amadeus::library::net::session> session, int32_t size)
{
	DWORD nbytes = 0;
	DWORD flags = 0;
	bool status = true;
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
				//close(session);
				status = false;
			}
		}
	}

	return status;
}
bool amadeus::library::net::iocp::tcp::server::post_send(std::shared_ptr<amadeus::library::net::session> session, std::vector<std::shared_ptr<amadeus::library::net::session::send_buffer_t>> sendQ)
{
	DWORD nbytes = 0;
	DWORD flags = 0;
	int32_t length = 0;


	bool status = true;

	if ((session->fd() != INVALID_SOCKET))
	{
		session->push_back_send_buffer(sendQ);

		std::vector<std::shared_ptr<amadeus::library::net::session::send_buffer_t>>::iterator iter;
		for (iter = sendQ.begin(); iter != sendQ.end(); iter++)
		{
			std::shared_ptr<amadeus::library::net::session::send_buffer_t> send_buffer = *iter;
			if (send_buffer)
			{
				send_buffer->wsabuf.buf = send_buffer->packet;
				send_buffer->wsabuf.len = send_buffer->send_size;
				int32_t value = WSASend(session->fd(), &(send_buffer->wsabuf), 1, &nbytes, flags, &(send_buffer->overlapped), NULL);
				if (value == SOCKET_ERROR)
				{
					int32_t err_code = ::WSAGetLastError();
					if (err_code != WSA_IO_PENDING)
					{
						status = false;
						break;
					}
				}
			}
		}
		if (!status) {
			std::shared_ptr<amadeus::library::net::session::send_buffer_t> send_error_buffer = *iter;
			if (send_error_buffer) {
				std::vector<std::shared_ptr<amadeus::library::net::session::send_buffer_t>>::iterator iter_find = std::find(sendQ.begin(), sendQ.end(), send_error_buffer);
				if (iter_find != sendQ.end())
					sendQ.erase(iter_find);
			}
		}
		session->update_last_access_tm();
	}
	return status;
}
void amadeus::library::net::iocp::tcp::server::execute(void)
{
	amadeus::library::net::session * session = nullptr;
	amadeus::library::net::io_context_t * io_context = nullptr;
	DWORD nbytes = 0;
	int32_t err_code = 0;

	while (1)
	{
		// IO Completion Packet 얻어온다.
		io_context = nullptr;
		session = nullptr;
		nbytes = 0;
		err_code = 0;
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
				close(session->shared_from_this());
				break;
			}

			if (err_code)
			{
				session->close();
				break;
			}

			// IO 성격에 따라 그에 따른 처리
			if (io_context->operation == amadeus::library::net::io_context_t::operation_t::recv)
			{
				amadeus::library::net::session::recv_buffer_t * recv_buffer = static_cast<amadeus::library::net::session::recv_buffer_t*>(io_context);
				if (!recv_completion_callback(session->shared_from_this(), recv_buffer->shared_from_this(), nbytes))
					session->close();
			}
			else if (io_context->operation == amadeus::library::net::io_context_t::operation_t::send)
			{
				amadeus::library::net::session::send_buffer_t * send_buffer = static_cast<amadeus::library::net::session::send_buffer_t*>(io_context);
				if (!send_completion_callback(session->shared_from_this(), send_buffer->shared_from_this(), nbytes))
					session->close();
			}
			else
			{
				//if (!other_completion_callback(session->shared_from_this(), nbytes))
				session->close();
			}

		} while (false);
	}
}