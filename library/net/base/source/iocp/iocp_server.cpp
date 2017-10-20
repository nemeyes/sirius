#include <Platform.h>
#pragma comment(lib,"ws2_32.lib")

#include <abstract_server.h>
#include <iocp_server.h>
#include <sirius_log4cplus_logger.h>
#include <sirius_locks.h>

sirius::library::net::iocp::server::server(sirius::library::net::server * processor, int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size, int32_t recv_buffer_size, bool dynamic_alloc)
	: _processor(processor)
	, _mtu(mtu)
	, _so_rcvbuf_size(so_rcvbuf_size)
	, _so_sndbuf_size(so_sndbuf_size)
	, _recv_buffer_size(recv_buffer_size)
	, _listen_socket(INVALID_SOCKET)
	, _dynamic_alloc(dynamic_alloc)
{
	_iocp = new handler(this);
}

sirius::library::net::iocp::server::~server(void)
{
	if (_iocp)
		delete _iocp;
	_iocp = nullptr;
}

int32_t sirius::library::net::iocp::server::initialize(void)
{
	WSADATA wsd;
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
		return sirius::library::net::iocp::server::err_code_t::fail;
	return sirius::library::net::iocp::server::err_code_t::success;
}

int32_t sirius::library::net::iocp::server::release(void)
{
	WSACleanup();
	return sirius::library::net::iocp::server::err_code_t::success;
}

/*
void sirius::library::net::iocp::server::start_streaming(void)
{
	if (_processor)
		_processor->start_streaming();
}
*/

std::shared_ptr<sirius::library::net::session> sirius::library::net::iocp::server::allocate_session(SOCKET client_socket)
{
	std::shared_ptr<sirius::library::net::session> session;
	if(_processor)
		session = _processor->create_session_callback(client_socket, _mtu, _recv_buffer_size, _dynamic_alloc);
	return session;
}
