
#include <WS2tcpip.h>
#include "abstract_server.h"
#include "iocp_udp_server.h"
#include "sirius_log4cplus_logger.h"

sirius::library::net::iocp::udp::server::server(sirius::library::net::server * processor, int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size, int32_t recv_buffer_size, bool multicast, bool dynamic_alloc)
	: sirius::library::net::iocp::server(processor, mtu, so_rcvbuf_size, so_sndbuf_size, recv_buffer_size, dynamic_alloc)
	, _multicast(multicast)
{

}

sirius::library::net::iocp::udp::server::~server(void)
{

}

int32_t sirius::library::net::iocp::udp::server::start(char * address, int32_t port_number, int32_t thread_pool_count)
{
	_listen_socket = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (_listen_socket == INVALID_SOCKET)
		return sirius::library::net::iocp::udp::server::err_code_t::fail;

	int resueaddr_enable = 1;
	if (setsockopt(_listen_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&resueaddr_enable, sizeof(resueaddr_enable)) < 0)
	{
		closesocket(_listen_socket);
		return sirius::library::net::iocp::udp::server::err_code_t::fail;
	}
	SOCKADDR_IN sock_addr;
	if (!address || strlen(address)<1 || _multicast)
		sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	else
		sock_addr.sin_addr.s_addr = inet_addr(address);
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons((short)port_number);
	if (bind(_listen_socket, (LPSOCKADDR)&sock_addr, sizeof(sock_addr)))
	{
		closesocket(_listen_socket);
		return sirius::library::net::iocp::udp::server::err_code_t::fail;
	}

	_listen_socket_addr.sin_family = AF_INET;
	_listen_socket_addr.sin_addr.s_addr = inet_addr(address);
	_listen_socket_addr.sin_port = htons(port_number);

	int err_code;
	if (!_iocp->create(thread_pool_count, &err_code))
	{ 
		closesocket(_listen_socket);
		return sirius::library::net::iocp::udp::server::err_code_t::fail;
	}
		
	_iocp->create_thread_pool();


	if (setsockopt(_listen_socket, SOL_SOCKET, SO_RCVBUF, (const char*)&_so_rcvbuf_size, sizeof(int)) == SOCKET_ERROR)
	{
		closesocket(_listen_socket);
		return false;
	}

	if (setsockopt(_listen_socket, SOL_SOCKET, SO_SNDBUF, (const char*)&_so_sndbuf_size, sizeof(int)) == SOCKET_ERROR)
	{
		closesocket(_listen_socket);
		return false;
	}
	if (_multicast)
	{
		int timeLive = 0;
		setsockopt(_listen_socket, IPPROTO_IP, IP_MULTICAST_TTL, (const char*)&timeLive, sizeof(timeLive));
	}
	std::shared_ptr<sirius::library::net::session> session = NULL;
	session = allocate_session(_listen_socket);
	if (!session)
		session->close();

	if (!_iocp->associate(_listen_socket, reinterpret_cast<ULONG_PTR>(session.get()), &err_code))
		session->close();

	post_recv(session, session->get_first_recv_packet_size());

	if (_processor)
		_processor->register_conn_client(session);
	
	//if(_multicast)
	//	start_streaming();

	return sirius::library::net::iocp::udp::server::err_code_t::success;
}

int32_t sirius::library::net::iocp::udp::server::stop(void)
{
	if (_processor)
		_processor->clear_conn_client();

	if (_listen_socket && _listen_socket != INVALID_SOCKET)
	{
		closesocket(_listen_socket);
		_listen_socket = INVALID_SOCKET;

		if (_iocp)
			_iocp->close_thread_pool();
	}
	return true;
}

void sirius::library::net::iocp::udp::server::close(std::shared_ptr<sirius::library::net::session> session)
{
	if (session->fd() != INVALID_SOCKET)
		session->close();
	_processor->unregister_conn_client(session);
	_processor->destroy_session_callback(session);
	//LOGGER::make_info_log(CAPS, "%s, %d, Streamer close", __FUNCTION__, __LINE__);
}

bool sirius::library::net::iocp::udp::server::recv_completion_callback(std::shared_ptr<sirius::library::net::session> session, std::shared_ptr<sirius::library::net::session::recv_buffer_t> context, int32_t nbytes)
{
	int32_t length = session->push_recv_packet(session->recv_context()->packet, nbytes);
	if (length < 0)
	{
		//LOGGER::make_error_log(CAPS, "%s, %d : recv_completion_callback=%d , errono=%d", __FUNCTION__, __LINE__, session->fd(), length);
		return false;
	}

	if (session->fd() != INVALID_SOCKET && session->recv_context().get() != NULL)
	{
		bool value = post_recv(session, length);
		if (!value)
			return false;
	}
	return true;
}

bool sirius::library::net::iocp::udp::server::send_completion_callback(std::shared_ptr<sirius::library::net::session> session, std::shared_ptr<sirius::library::net::session::send_buffer_t> context, int32_t nbytes)
{
	if (session->fd() == INVALID_SOCKET)
		return false;

	session->pop_send_buffer(context);
	return true;
}

bool sirius::library::net::iocp::udp::server::other_completion_callback(std::shared_ptr<sirius::library::net::session> session, int32_t nbytes)
{
	// 현재 여기로 오면 Recv , Send 이외의 이상한 동작을 가리킴 소켓 끊어버리자.
	//close(session);
	return false;
}

bool sirius::library::net::iocp::udp::server::post_recv(std::shared_ptr<sirius::library::net::session> session, int32_t size)
{
	DWORD nbytes = 0;
	DWORD flags = 0;

	bool status = true;
	{
		if ((session->fd() != INVALID_SOCKET))
		{
			//int clnt_addr_size = sizeof(_udp_clnt_addr);
			if (size > session->recv_context()->packet_size)
			{
				session->recv_context()->resize(size);
			}
			session->recv_context()->wsabuf.buf = session->recv_context()->packet;
			session->recv_context()->wsabuf.len = size;
			int addr_size = sizeof(SOCKADDR_IN);

			int value = WSARecvFrom(session->fd(), &(session->recv_context()->wsabuf), 1, &nbytes, &flags, (SOCKADDR*)&session->recv_context()->peer, &addr_size, &(session->recv_context()->overlapped), NULL);
			if (SOCKET_ERROR == value)
			{
				int err_code = WSAGetLastError();
				if (err_code != WSA_IO_PENDING)
				{
					status = false;
				}
			}
		}
	}
	return status;
}

bool sirius::library::net::iocp::udp::server::post_send(std::shared_ptr<sirius::library::net::session> session, std::vector<std::shared_ptr<sirius::library::net::session::send_buffer_t>> sendQ)
{
	DWORD nbytes = 0;
	DWORD flags = 0;
	int32_t length = 0;
	bool status = true;
	if ((session->fd() != INVALID_SOCKET))
	{
		session->push_back_send_buffer(sendQ);

		std::vector<std::shared_ptr<sirius::library::net::session::send_buffer_t>>::iterator iter;
		for (iter = sendQ.begin(); iter != sendQ.end(); iter++)
		{
			std::shared_ptr<sirius::library::net::session::send_buffer_t> send_buffer = *iter;
			if (send_buffer)
			{
				send_buffer->wsabuf.buf = send_buffer->packet;
				send_buffer->wsabuf.len = send_buffer->send_size;
				int32_t addr_size = sizeof(SOCKADDR_IN);
				if (_multicast)
					send_buffer->peer = _listen_socket_addr;
				else
					send_buffer->peer = session->recv_context()->peer;
					
				//log_debug("[Server] send packet size : %d", send_buffer->send_size );
				//log_debug("[Server] num of pes packets : %d", send_buffer->send_size /188);
				int value = WSASendTo(session->fd(), &(send_buffer->wsabuf), 1, &nbytes, flags, (SOCKADDR*)&send_buffer->peer, addr_size, &(send_buffer->overlapped), NULL);
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
			std::shared_ptr<sirius::library::net::session::send_buffer_t> send_error_buffer = *iter;
			if (send_error_buffer) {
				std::vector<std::shared_ptr<sirius::library::net::session::send_buffer_t>>::iterator iter_find = std::find(sendQ.begin(), sendQ.end(), send_error_buffer);
				if (iter_find != sendQ.end())
					sendQ.erase(iter_find);
			}
		}
		session->update_last_access_tm();
	}
	//log_debug("");
	return status;
}

void sirius::library::net::iocp::udp::server::execute(void)
{
	sirius::library::net::session * session = nullptr;
	sirius::library::net::iocp::io_context_t * io_context = nullptr;
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
		//log_debug("[Server] sent : %d err_code : %d ", nbytes, err_code);
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
				/*if (_udp_clnt_list.size() == 0)
				
				_udp_clnt_list.push_back(_udp_clnt_addr);*/
				//start_streaming();
				//close(session->shared_from_this());
				//break;
			}

			if (err_code)
			{
				switch (err_code)
				{
				case ERROR_PORT_UNREACHABLE:
					break;
				}
				log_debug("[GQCS Error] %d", err_code);
				session->close();
				break;
			}

			// IO 성격에 따라 그에 따른 처리
			if (io_context->operation == sirius::library::net::iocp::io_context_t::operation_t::recv)
			{
				sirius::library::net::session::recv_buffer_t * recv_buffer = static_cast<sirius::library::net::session::recv_buffer_t*>(io_context);
				if (!recv_completion_callback(session->shared_from_this(), recv_buffer->shared_from_this(), nbytes))
					session->close();
			}
			else if (io_context->operation == sirius::library::net::iocp::io_context_t::operation_t::send)
			{
				sirius::library::net::session::send_buffer_t * send_buffer = static_cast<sirius::library::net::session::send_buffer_t*>(io_context);
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
