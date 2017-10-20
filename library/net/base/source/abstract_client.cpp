
#include <abstract_client.h>
#include <sirius_locks.h>
#include <sirius_stringhelper.h>


sirius::library::net::client::client(int32_t mtu, int32_t so_rcvbuf_size, int32_t so_sndbuf_size,int32_t recv_buffer_size, bool dynamic_alloc, int32_t type, bool multicast)
	: _port_number(3390)
	, _reconnection(false)
	, _run(false)
	, _thread(INVALID_HANDLE_VALUE)
	, _bwaiting(false)
	, _io_thread_pool_count(0)
{
	::InitializeCriticalSection(&_cs);

	memset(_address, 0x00, sizeof(_address));

#if defined(WIN32)
	switch (type)
	{
	case sirius::library::net::client::ethernet_type_t::tcp :
		_client = new sirius::library::net::iocp::tcp::client(this, mtu, so_rcvbuf_size, so_sndbuf_size, recv_buffer_size, dynamic_alloc);
		break;
	case sirius::library::net::client::ethernet_type_t::udp :
		_client = new sirius::library::net::iocp::udp::client(this, mtu, so_rcvbuf_size, so_sndbuf_size, recv_buffer_size, dynamic_alloc, multicast);
		break;
	}
#elif defined(__linux__)
	_client = new sirius::library::net::epoll_client(this, mtu, so_rcvbuf_size, so_sndbuf_size, recv_buffer_size, dynamic_alloc);
#elif defined(__MACOS__) || defined(__FreeBSD__)
	_client = new sirius::library::net::kqueue_client(this, mtu, so_rcvbuf_size, so_sndbuf_size, recv_buffer_size, dynamic_alloc);
#endif
	_client->initialize();
}

sirius::library::net::client::~client(void)
{
	disconnect();
	if (_client)
	{
		_client->release();
		delete _client;
		_client = nullptr;
	}

	::DeleteCriticalSection(&_cs);
}

bool sirius::library::net::client::connect(const char * address, int32_t port_number, int32_t io_thread_pool_count, bool reconnection)
{
	sirius::autolock mutex(&_cs);

	if (!address || strlen(address) < 1)
		return false;
	strcpy_s(_address, address);
	_port_number = port_number;
	_io_thread_pool_count = io_thread_pool_count;
	_reconnection = reconnection;
	unsigned int thread_id = 0;
	_thread = (HANDLE)_beginthreadex(NULL, 0, sirius::library::net::client::process_cb, this, 0, &thread_id);
	if (_thread == NULL || _thread == INVALID_HANDLE_VALUE)
		return false;

	return true;
}

bool sirius::library::net::client::disconnect(void)
{
	sirius::autolock mutex(&_cs);

	_reconnection = false;
	_run = false;
	if (_thread != INVALID_HANDLE_VALUE)
	{
		if (::WaitForSingleObject(_thread, INFINITE) == WAIT_OBJECT_0)
		{
			if (_thread != INVALID_HANDLE_VALUE)
				::CloseHandle(_thread);
		}
		_thread = INVALID_HANDLE_VALUE;
	}
	return true;
}

void sirius::library::net::client::enable_disconnect_flag(bool enable)
{
	_run = !enable;
}

void sirius::library::net::client::enable_close_waiting_flag(bool enable)
{
	_bwaiting = enable;
}

void sirius::library::net::client::data_request(std::shared_ptr<sirius::library::net::session> session, std::vector<std::shared_ptr<sirius::library::net::session::send_buffer_t>> sendQ)
{
	_client->post_send(session, sendQ);
}

unsigned __stdcall sirius::library::net::client::process_cb(void * param)
{
	sirius::library::net::client * self = static_cast<sirius::library::net::client*>(param);
	self->process();
	return 0;
}