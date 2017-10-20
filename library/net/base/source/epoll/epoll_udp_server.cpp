
#include <WS2tcpip.h>
#include "abstract_server.h"
#include "iocp_udp_server.h"
#include "cap_log4cplus_logger.h"


amadeus::library::net::iocp::udp::server::server(amadeus::library::net::server * processor, int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size, int32_t recv_buffer_size, int32_t transport_type, bool dynamic_alloc)
	:amadeus::library::net::iocp_server(processor, mtu, so_rcvbuf_size, so_sndbuf_size, recv_buffer_size, dynamic_alloc)
	, _transport_type(transport_type)
{
}

amadeus::library::net::iocp::udp::server::~server()
{

}

bool amadeus::library::net::iocp::udp::server::start(char * address, int32_t port_number, int32_t thread_pool_count)
{
	_listen_socket = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (_listen_socket == INVALID_SOCKET)
		return false;

	int resueaddr_enable = 1;
	if (setsockopt(_listen_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&resueaddr_enable, sizeof(resueaddr_enable)) < 0)
	{
		closesocket(_listen_socket);
		return false;
	}
	SOCKADDR_IN sock_addr;
	if (!address || strlen(address)<1 || _transport_type == cap_base::protocol_type_t::ts_over_multicast)
		sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	else
		sock_addr.sin_addr.s_addr = inet_addr(address);
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons((short)port_number);
	if (bind(_listen_socket, (LPSOCKADDR)&sock_addr, sizeof(sock_addr)))
	{
		closesocket(_listen_socket);
		return false;
	}

	_listen_socket_addr.sin_family = AF_INET;
	_listen_socket_addr.sin_addr.s_addr = inet_addr(address);
	_listen_socket_addr.sin_port = htons(port_number);

	int err_code;
	if (!_iocp->create(thread_pool_count, &err_code))
	{ 
		closesocket(_listen_socket);
		return false;
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
	if (_transport_type == cap_base::protocol_type_t::ts_over_multicast)
	{
		int timeLive = 64;
		setsockopt(_listen_socket, IPPROTO_IP, IP_MULTICAST_TTL, (const char*)&timeLive, sizeof(timeLive));
	}
	std::shared_ptr<amadeus::library::net::session> session = NULL;
	session = allocate_session(_listen_socket);
	if (!session)
		session->close();

	if (!_iocp->associate(_listen_socket, reinterpret_cast<ULONG_PTR>(session.get()), &err_code))
		session->close();

	post_recv(session, session->get_first_recv_packet_size());

	if (_processor)
		_processor->register_conn_client(session);
	
	if(_transport_type == cap_base::protocol_type_t::ts_over_multicast)
		start_streaming();

	return true;
}

void amadeus::library::net::iocp::udp::server::execute(void)
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
				start_streaming();
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
bool amadeus::library::net::iocp::udp::server::post_recv(std::shared_ptr<amadeus::library::net::session> session, int32_t size)
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

#ifdef WITH_UDP_SERVER
			int value = WSARecvFrom(session->fd(), &(session->recv_context()->wsabuf), 1, &nbytes, &flags, (SOCKADDR*)&session->recv_context()->src_addr, &addr_size, &(session->recv_context()->overlapped), NULL);
			if (SOCKET_ERROR == value)
			{
				int err_code = WSAGetLastError();
				if (err_code != WSA_IO_PENDING)
				{
					status = false;
				}
			}
#endif
		}

	}
	return status;
}
bool amadeus::library::net::iocp::udp::server::post_send(std::shared_ptr<amadeus::library::net::session> session, std::vector<std::shared_ptr<amadeus::library::net::session::send_buffer_t>> sendQ)
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
				int addr_size = sizeof(SOCKADDR_IN);
#ifdef WITH_UDP_SERVER
				if(_transport_type == cap_base::protocol_type_t::ts_over_udp)
					send_buffer->dst_addr = session->recv_context()->src_addr;
				else if (_transport_type == cap_base::protocol_type_t::ts_over_multicast)
					send_buffer->dst_addr = _listen_socket_addr;
				log_debug("[Server] send packet size : %d", send_buffer->send_size );
				//log_debug("[Server] num of pes packets : %d", send_buffer->send_size /188);
				int value = WSASendTo(session->fd(), &(send_buffer->wsabuf), 1, &nbytes, flags, (SOCKADDR*)&send_buffer->dst_addr, addr_size, &(send_buffer->overlapped), NULL);
				if (value == SOCKET_ERROR)
				{
					int32_t err_code = ::WSAGetLastError();
					if (err_code != WSA_IO_PENDING)
					{
						status = false;
						break;
					}
				}
#endif
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
	//log_debug("                                                     ");
	return status;
}