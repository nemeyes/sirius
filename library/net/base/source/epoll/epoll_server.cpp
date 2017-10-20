#include <Platform.h>
#pragma comment(lib,"ws2_32.lib")

#include <abstract_server.h>
#include <iocp_server.h>
#include <cap_log4cplus_logger.h>
#include <cap_auto_lock.h>

amadeus::library::net::iocp_server::iocp_server(amadeus::library::net::server * processor, int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size, int32_t recv_buffer_size, bool dynamic_alloc)
	: _processor(processor)
	, _mtu(mtu)
	, _so_rcvbuf_size(so_rcvbuf_size)
	, _so_sndbuf_size(so_sndbuf_size)
	, _recv_buffer_size(recv_buffer_size)
	, _listen_socket(INVALID_SOCKET)
	, _dynamic_alloc(dynamic_alloc)
{
	_iocp = new iocp_handler(this);
}

amadeus::library::net::iocp_server::~iocp_server(void)
{
	if (_iocp)
		delete _iocp;
	_iocp = nullptr;
}

bool amadeus::library::net::iocp_server::initialize(void)
{
	WSADATA wsd;
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
		return false;
	return true;
}

void amadeus::library::net::iocp_server::release(void)
{
	WSACleanup();
}


bool amadeus::library::net::iocp_server::stop(void)
{
	if (_processor)
		_processor->clear_conn_client();

	if (_listen_socket && _listen_socket != INVALID_SOCKET)
	{
		/*_baccept = false;
		if (::WaitForSingleObject(_thread, INFINITE) == WAIT_OBJECT_0)
			::CloseHandle(_thread);*/

		closesocket(_listen_socket);
		_listen_socket = INVALID_SOCKET;


		if (_iocp)
			_iocp->close_thread_pool();
	}
	return true;
}

void amadeus::library::net::iocp_server::close(std::shared_ptr<amadeus::library::net::session> session)
{
	if(session->fd() != INVALID_SOCKET)
		session->close();
	_processor->unregister_conn_client(session);
	_processor->destroy_session_callback(session);
	LOGGER::make_info_log(CAPS, "%s, %d, Streamer close", __FUNCTION__, __LINE__);
}

bool amadeus::library::net::iocp_server::recv_completion_callback(std::shared_ptr<amadeus::library::net::session> session, std::shared_ptr<amadeus::library::net::session::recv_buffer_t> context, int32_t nbytes)
{
	int32_t length = session->push_recv_packet(session->recv_context()->packet, nbytes);
	if (length < 0)
	{
		LOGGER::make_error_log(CAPS, "%s, %d : recv_completion_callback=%d , errono=%d", __FUNCTION__, __LINE__, session->fd(), length);
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

bool amadeus::library::net::iocp_server::send_completion_callback(std::shared_ptr<amadeus::library::net::session> session, std::shared_ptr<amadeus::library::net::session::send_buffer_t> context, int32_t nbytes)
{
	if (session->fd() == INVALID_SOCKET)
		return false;

	session->pop_send_buffer(context);
	return true;
}

bool amadeus::library::net::iocp_server::other_completion_callback(std::shared_ptr<amadeus::library::net::session> session, int32_t nbytes)
{
	// 현재 여기로 오면 Recv , Send 이외의 이상한 동작을 가리킴 소켓 끊어버리자.
	//close(session);
	return false;
}

void amadeus::library::net::iocp_server::start_streaming(void)
{
	if (_processor)
		_processor->start_streaming();
}
std::shared_ptr<amadeus::library::net::session> amadeus::library::net::iocp_server::allocate_session(SOCKET client_socket)
{
	std::shared_ptr<amadeus::library::net::session> session;
	if(_processor)
		session = _processor->create_session_callback(client_socket, _mtu, _recv_buffer_size, _dynamic_alloc);
	return session;
}
