#include <abstract_server.h>
#include <sirius_locks.h>
#include <sirius_stringhelper.h>

#if defined(WIN32)
# include <iocp_tcp_server.h>
# include <iocp_udp_server.h>
#elif defined(__linux__)
# include <epoll_tcp_server.h>
# include <epoll_udp_server.h>
#elif defined(__MACOS__) || defined(__FreeBSD__)
# include <kqueue_tcp_server.h>
# include <kqueue_udp_server.h>
#endif

sirius::library::net::server::server(int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size,int32_t recv_buffer_size, bool dynamic_alloc, int32_t type, bool multicast)
	: _port_number(3390)
	, _io_thread_pool_count(0)
	, _thread(INVALID_HANDLE_VALUE)
	, _run(false)
{
	memset(_address, 0x00, sizeof(_address));

#if defined(WIN32)
	switch (type)
	{
	case sirius::library::net::server::ethernet_type_t::tcp :
		_server = new sirius::library::net::iocp::tcp::server(this, mtu, so_rcvbuf_size, so_sndbuf_size, recv_buffer_size, dynamic_alloc);
		break;
	case sirius::library::net::server::ethernet_type_t::udp :
		if(multicast)
			_server = new sirius::library::net::iocp::udp::server(this, mtu, so_rcvbuf_size, so_sndbuf_size, recv_buffer_size, true, dynamic_alloc);
		else
			_server = new sirius::library::net::iocp::udp::server(this, mtu, so_rcvbuf_size, so_sndbuf_size, recv_buffer_size, false, dynamic_alloc);
		break;
	}
#elif defined(__linux__)
	_server = new sirius::library::net::epoll_server(this, mtu, so_rcvbuf_size, so_sndbuf_size,recv_buffer_size, dynamic_alloc);
#elif defined(__MACOS__) || defined(__FreeBSD__)
	_server = new sirius::library::net::kqueue_server(this, mtu, so_rcvbuf_size, so_sndbuf_size, recv_buffer_size,dynamic_alloc);
#endif
	_server->initialize();
	::InitializeCriticalSection(&_conn_sessions_cs);
}

sirius::library::net::server::~server(void)
{
	if (_server)
	{
		//_server->stop();
		_server->release();
		delete _server;
		_server = nullptr;
	}
	::DeleteCriticalSection(&_conn_sessions_cs);
}

bool sirius::library::net::server::start(char * address, int32_t port_number, int32_t thread_pool_count)
{
	if (address && strlen(address) > 0)
		strncpy_s(_address, address, sizeof(_address));
	_port_number = port_number;
	_io_thread_pool_count = thread_pool_count;
	
	unsigned int thread_id;
	_run = true;
	_thread = (HANDLE)_beginthreadex(NULL, 0, sirius::library::net::server::process_cb, this, 0, &thread_id);
	return true;
}

bool sirius::library::net::server::stop(void)
{
	_run = false;
	if (_thread != INVALID_HANDLE_VALUE)
	{
		::WaitForSingleObject(_thread, INFINITE);
		::CloseHandle(_thread);
		_thread = INVALID_HANDLE_VALUE;
	}
	return true;
}

bool sirius::library::net::server::is_run(void) const
{
	return _run;
}

void sirius::library::net::server::data_request(std::shared_ptr<sirius::library::net::session> session, std::vector<std::shared_ptr<sirius::library::net::session::send_buffer_t>> sendQ)
{
	_server->post_send(session, sendQ);
}

void sirius::library::net::server::register_conn_client(std::shared_ptr<sirius::library::net::session> session)
{
	sirius::autolock lock(&_conn_sessions_cs);
	_conn_sessions.push_back(session);
}

void sirius::library::net::server::unregister_conn_client(std::shared_ptr<sirius::library::net::session> session)
{
	sirius::autolock lock(&_conn_sessions_cs);
	std::vector<std::shared_ptr<sirius::library::net::session>>::iterator iter = std::find(_conn_sessions.begin(), _conn_sessions.end(), session);
	if (iter != _conn_sessions.end())
	{
		_conn_sessions.erase(iter);
	}
}

void sirius::library::net::server::clear_conn_client(void)
{
	sirius::autolock lock(&_conn_sessions_cs);
	std::vector<std::shared_ptr<sirius::library::net::session>>::iterator iter;
	for (iter = _conn_sessions.begin(); iter != _conn_sessions.end(); iter++)
	{
		std::shared_ptr<sirius::library::net::session> sess = (*iter);
		if (sess)
			sess->close();
	}
	_conn_sessions.clear();
}

unsigned __stdcall sirius::library::net::server::process_cb(void * param)
{
	sirius::library::net::server * self = static_cast<sirius::library::net::server*>(param);
	self->process();
	return 0;
}