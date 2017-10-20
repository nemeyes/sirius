#include "abstract_session.h"
#include <sirius_locks.h>

#define SEND_QUEUE_SIZE		100
#define SEND_BUFFER_SIZE	1024*1024*4
#define RECV_BUFFER_SIZE	1024*1024*4

sirius::library::net::session::session(SOCKET fd, int32_t mtu, int32_t recv_buffer_size, bool dynamic_alloc)
	: _dynamic_alloc(dynamic_alloc)
	, _recv_buffer_index(0)
	, _mtu(mtu)
	, _fd(fd)
{
	_recv_context = std::shared_ptr<sirius::library::net::session::recv_buffer_t>(new sirius::library::net::session::recv_buffer_t(recv_buffer_size));
	if (!_dynamic_alloc)
	{
		if (_mtu == 0)
		{
			for (int32_t i = 0; i < SEND_QUEUE_SIZE; i++)
			{
				std::shared_ptr<sirius::library::net::session::send_buffer_t> send_buffer = std::shared_ptr<sirius::library::net::session::send_buffer_t>(new sirius::library::net::session::send_buffer_t(MTU));
				_send_ready_queue.push_back(send_buffer);
			}
		}
		else
		{
			int32_t packet_count = SEND_BUFFER_SIZE / _mtu;
			int32_t remained = SEND_BUFFER_SIZE % _mtu;
			if (remained > 0)
				packet_count++;

			for (int32_t i = 0; i < packet_count; i++)
			{
				std::shared_ptr<sirius::library::net::session::send_buffer_t> send_buffer = std::shared_ptr<sirius::library::net::session::send_buffer_t>(new sirius::library::net::session::send_buffer_t(_mtu));
				_send_ready_queue.push_back(send_buffer);
			}
		}
	}

	update_last_access_tm();

	::InitializeCriticalSection(&_cs);
	::InitializeCriticalSection(&_ready_send_cs_lock);
	::InitializeCriticalSection(&_send_cs_lock);
	::InitializeCriticalSection(&_recv_cs_lock);
	
	_recv_buffer_size = MTU;

	_precv_buffer = static_cast<char*>(malloc(_recv_buffer_size));
	if (_precv_buffer == nullptr)
		throw std::exception("can't alloc memory.");
	memset(_precv_buffer, 0x0, _recv_buffer_size);
}

sirius::library::net::session::~session(void)
{
	close();

	{
		sirius::autolock lock(&_recv_cs_lock);
		if (_precv_buffer)
		{
			free(_precv_buffer);
			_precv_buffer = nullptr;
		}
	}

	{
		sirius::autolock lock(&_send_cs_lock);
		_send_queue.clear();
	}

	{
		sirius::autolock lock(&_ready_send_cs_lock);
		_send_ready_queue.clear();
	}

	::DeleteCriticalSection(&_recv_cs_lock);
	::DeleteCriticalSection(&_send_cs_lock);
	::DeleteCriticalSection(&_ready_send_cs_lock);
	::DeleteCriticalSection(&_cs);
}

bool sirius::library::net::session::shutdown_fd(void)
{
	sirius::autolock mutex(&_cs);
#if defined(WIN32)
	if (_fd == INVALID_SOCKET) /* Already clsoed */
		return false;

	LINGER linger;
	linger.l_onoff = 1;
	linger.l_linger = 0;
	setsockopt(_fd, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));

	if (closesocket(_fd) == 0)
	{
		_fd = INVALID_SOCKET;
		return true;
	}
	else
	{
		int32_t code = WSAGetLastError();
		//LOGGER::make_trace_log(CAPS, "%s()_%d : GetLastError:%d ", __FUNCTION__, __LINE__, code);

		_fd = INVALID_SOCKET;
		return false;
	}
#else
	int fd = _fd;
	if (fd <= 0) /* Already clsoed */
		return 0;

	shutdown(_fd, SHUT_RDWR);
	if (close(_fd) != 0)
	{
		//printf( "socket close failed [%d].", fd );
		LOG4CPLUS_ERROR(log4cplus::Logger::getInstance("system"), "socket close failed [" << fd << "]");
		clear_send_queue();
		clear_recv_queue();
		_fd = -1;
		return -1;
	}
	else
	{
		//printf( "socket close success [%d].", fd );
		LOG4CPLUS_TRACE(log4cplus::Logger::getInstance("system"), "socket close success [" << fd << "]");
		clear_send_queue();
		clear_recv_queue();
		_fd = -1;
		return 0;
	}
#endif
}

void sirius::library::net::session::push_back_send_buffer(std::vector<std::shared_ptr<sirius::library::net::session::send_buffer_t>> sendQ)
{
	sirius::autolock mutex(&_send_cs_lock);
	std::vector<std::shared_ptr<sirius::library::net::session::send_buffer_t>>::iterator iter;
	for (iter = sendQ.begin(); iter != sendQ.end(); iter++)
	{
		_send_queue.push_back(*iter);
	}
}

void sirius::library::net::session::push_back_send_buffer(std::shared_ptr<sirius::library::net::session::send_buffer_t> send_buffer)
{
	sirius::autolock mutex(&_send_cs_lock);
	_send_queue.push_back(send_buffer);
}

void sirius::library::net::session::pop_send_buffer(std::shared_ptr<sirius::library::net::session::send_buffer_t> send_buffer)
{
	if (!send_buffer)
		return;

	{
		sirius::autolock mutex(&_send_cs_lock);
		std::vector<std::shared_ptr<sirius::library::net::session::send_buffer_t>>::iterator iter = std::find(_send_queue.begin(), _send_queue.end(), send_buffer);
		if (iter != _send_queue.end())
			_send_queue.erase(iter);
	}
	{
		sirius::autolock mutex(&_ready_send_cs_lock);
		if (!_dynamic_alloc && send_buffer!=nullptr)
			_send_ready_queue.push_back(send_buffer);
	}
}

SOCKET sirius::library::net::session::fd(void)
{
	sirius::autolock mutex(&_cs);
	return _fd;
}

void sirius::library::net::session::fd(SOCKET fd)
{
	sirius::autolock mutex(&_cs);
	_fd = fd;
}

std::shared_ptr<sirius::library::net::session::recv_buffer_t> sirius::library::net::session::recv_context(void)
{
	return _recv_context;
}

uint64_t sirius::library::net::session::get_last_access_tm(void)
{
	return _last_access_tm;
}

void sirius::library::net::session::update_last_access_tm(void)
{
	_last_access_tm = ::GetTickCount64();
}

bool sirius::library::net::session::close(void)
{
	update_last_access_tm();
	if (fd() == INVALID_SOCKET)
		return true;

	shutdown_fd();

	return true;
}