#include <platform.h>
#pragma comment(lib,"ws2_32.lib")

#include <abstract_client.h>
#include <iocp_client.h>

amadeus::library::net::iocp_client::iocp_client(amadeus::library::net::client * processor, int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size, int32_t recv_buffer_size,bool dynamic_alloc)
	: _processor(processor)
	, _mtu(mtu)
	, _so_rcvbuf_size(so_rcvbuf_size)
	, _so_sndbuf_size(so_sndbuf_size)
	, _recv_buffer_size(recv_buffer_size)
	, _dynamic_alloc(dynamic_alloc)
{
	_iocp = new iocp_handler(this);
}

amadeus::library::net::iocp_client::~iocp_client(void)
{
	if (_iocp)
		delete _iocp;
	_iocp = nullptr;
}

bool amadeus::library::net::iocp_client::initialize(void)
{
	WSADATA wsd;
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
		return false;
	return true;
}

void amadeus::library::net::iocp_client::release(void)
{
	WSACleanup();
}
//
//std::shared_ptr<amadeus::library::net::session> amadeus::library::net::iocp_client::connect(const char * address, int32_t port_number, int32_t io_thread_pool_count)
//{
//	SOCKADDR_IN	serv_addr;
//	SOCKET socket = INVALID_SOCKET;
//
//	int32_t err_code;
//	if (!_iocp->create(io_thread_pool_count, &err_code))
//		return std::shared_ptr<amadeus::library::net::session>();
//	_iocp->create_thread_pool();
//
//
//	int32_t value = -1;
//	int32_t length = -1;
//	int32_t zero = 1;
//
//	//if (transport_type == transport_type_t::tcp)
//	//	socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
//	//else
//		socket = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
//
//	if (socket == INVALID_SOCKET)
//		return std::shared_ptr<amadeus::library::net::session>();
//
//	memset(&serv_addr, 0x00, sizeof(serv_addr));
//	serv_addr.sin_family = AF_INET;
//	serv_addr.sin_addr.s_addr = inet_addr(address);
//	serv_addr.sin_port = htons(port_number);
//
//	value = WSAConnect(socket, (SOCKADDR*)(&serv_addr), sizeof(serv_addr), NULL, NULL, NULL, NULL);
//	if ((value == SOCKET_ERROR) && (WSAGetLastError() != WSAEWOULDBLOCK))
//	{
//		closesocket(socket);
//		return std::shared_ptr<amadeus::library::net::session>();
//	}
//
//	if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&zero, sizeof(int)) == SOCKET_ERROR)
//	{
//		closesocket(socket);
//		return std::shared_ptr<amadeus::library::net::session>();
//	}
//
//	zero = _so_rcvbuf_size;
//	if (setsockopt(socket, SOL_SOCKET, SO_RCVBUF, (const char*)&zero, sizeof(int)) == SOCKET_ERROR)
//	{
//		closesocket(socket);
//		return std::shared_ptr<amadeus::library::net::session>();
//	}
//	zero = _so_sndbuf_size;
//	if (setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (const char*)&zero, sizeof(int)) == SOCKET_ERROR)
//	{
//		closesocket(socket);
//		return std::shared_ptr<amadeus::library::net::session>();
//	}
//
//	zero = 0;
//	if (setsockopt(socket, SOL_SOCKET, TCP_NODELAY, (char *)&zero, sizeof(int)) == SOCKET_ERROR) //disable nagle algorithm
//	{
//		closesocket(socket);
//		return std::shared_ptr<amadeus::library::net::session>();
//	}
//
//	std::shared_ptr<amadeus::library::net::session> session = allocate_session(socket);
//	if (!_iocp->associate((session)->fd(), reinterpret_cast<ULONG_PTR>(session.get()), &err_code))
//	{
//		session->close();
//		return std::shared_ptr<amadeus::library::net::session>();
//	}
//
//	if (!post_recv(session, session->get_first_recv_packet_size()))
//	{
//		session->close();
//		return std::shared_ptr<amadeus::library::net::session>();
//	}
//
//	//if (transport_type == transport_type_t::udp)
//	{
//		int flags = 0;
//		send(socket, NULL, 0, flags);
//	}
//	_processor->enable_close_waiting_flag(false);
//	return session;
//	
//}

bool amadeus::library::net::iocp_client::disconnect(void)
{
	_iocp->close_thread_pool();
	return true;
}

void amadeus::library::net::iocp_client::close(std::shared_ptr<amadeus::library::net::session> session)
{
	if (session->fd() != INVALID_SOCKET)
		session->close();
	_processor->destroy_session_callback(session);
}

bool amadeus::library::net::iocp_client::recv_completion_callback(std::shared_ptr<amadeus::library::net::session> session, std::shared_ptr<amadeus::library::net::session::recv_buffer_t> context, int32_t nbytes)
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

bool amadeus::library::net::iocp_client::send_completion_callback(std::shared_ptr<amadeus::library::net::session> session, std::shared_ptr<amadeus::library::net::session::send_buffer_t> context, int32_t nbytes)
{
	session->pop_send_buffer(context);
	return true;
}

bool amadeus::library::net::iocp_client::other_completion_callback(std::shared_ptr<amadeus::library::net::session> session, int32_t nbytes)
{
	// 현재 여기로 오면 Recv , Send 이외의 이상한 동작을 가리킴 소켓 끊어버리자.
	//session->shutdown_fd();
	return true;
}

bool amadeus::library::net::iocp_client::post_recv(std::shared_ptr<amadeus::library::net::session> session, int32_t size)
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

bool amadeus::library::net::iocp_client::post_send(std::shared_ptr<amadeus::library::net::session> session, std::vector<std::shared_ptr<amadeus::library::net::session::send_buffer_t>> sendQ)
{
	DWORD nbytes = 0;
	DWORD flags = 0;

	bool status = true;
	if ((session->fd() != INVALID_SOCKET))
	{
		std::vector<std::shared_ptr<amadeus::library::net::session::send_buffer_t>>::iterator iter;
		for (iter = sendQ.begin(); iter != sendQ.end(); iter++)
		{
			std::shared_ptr<amadeus::library::net::session::send_buffer_t> send_buffer = *iter;
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

std::shared_ptr<amadeus::library::net::session> amadeus::library::net::iocp_client::allocate_session(SOCKET client_socket)
{
	std::shared_ptr<amadeus::library::net::session> session;
	if (_processor)
		session = _processor->create_session_callback(client_socket, _mtu,_recv_buffer_size, _dynamic_alloc);
	return session;
}
//
//void amadeus::library::net::iocp_client::execute(void)
//{
//	amadeus::library::net::session * session = NULL;
//	amadeus::library::net::io_context_t * io_context = NULL;
//	DWORD nbytes = 0;
//	int32_t err_code = 0;
//
//	while (1)
//	{
//		// IO Completion Packet 얻어온다.
//		bool value = _iocp->get_completion_status(reinterpret_cast<ULONG_PTR*>(&session), &nbytes, reinterpret_cast<LPOVERLAPPED*>(&io_context), &err_code);
//		if (value)
//		{
//			if (((int32_t)session) == KILL_THREAD)
//				break;
//		}
//
//		do
//		{
//			session->update_last_access_tm();
//			if (session->fd() == INVALID_SOCKET)
//				break;
//
//			if (nbytes == 0)
//			{
//				_processor->enable_close_waiting_flag(false);
//				close(session->shared_from_this());
//				break;
//			}
//
//			if (err_code)
//			{
//				switch (err_code)
//				{
//				case ERROR_MORE_DATA: break;
//				}
//			
//				//_processor->enable_disconnect_flag(true);
//				_processor->enable_close_waiting_flag(true);
//				session->close();
//				break;
//			}
//
//			// IO 성격에 따라 그에 따른 처리
//			if (io_context->operation == amadeus::library::net::io_context_t::operation_t::recv)
//			{
//				amadeus::library::net::session::recv_buffer_t * recv_buffer = static_cast<amadeus::library::net::session::recv_buffer_t*>(io_context);
//#if 0 //CKOH_FILE_EXPORT
//				static HANDLE file = NULL;
//				if (file == NULL)
//					file = ::CreateFileA("amadeus_udp_ts_dst.ts", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
//				if (file != INVALID_HANDLE_VALUE)
//				{
//					if (recv_buffer->shared_from_this()->packet && nbytes > 0)
//					{
//						uint32_t bytes2write = nbytes;
//						uint32_t bytes_written = 0;
//						do
//						{
//							uint32_t nb_write = 0;
//							::WriteFile(file, recv_buffer->shared_from_this()->packet, bytes2write, (LPDWORD)&nb_write, 0);
//							bytes_written += nb_write;
//							if (bytes2write == bytes_written)
//								break;
//						} while (1);
//					}
//				}
//#endif
//				
//				if (!recv_completion_callback(session->shared_from_this(), recv_buffer->shared_from_this(), nbytes))
//				{
//					//_processor->enable_disconnect_flag(true);
//					_processor->enable_close_waiting_flag(true);
//					session->close();
//				}
//			}
//			else if (io_context->operation == amadeus::library::net::io_context_t::operation_t::send)
//			{
//				amadeus::library::net::session::send_buffer_t * send_buffer = static_cast<amadeus::library::net::session::send_buffer_t*>(io_context);
//				if (!send_completion_callback(session->shared_from_this(), send_buffer->shared_from_this(), nbytes))
//				{
//					//_processor->enable_disconnect_flag(true);
//					_processor->enable_close_waiting_flag(true);
//					session->close();
//				}
//			}
//			else
//			{
//				//_processor->enable_disconnect_flag(true);
//				_processor->enable_close_waiting_flag(true);
//				session->close();
//			}
//
//		} while (false);
//	}
//}