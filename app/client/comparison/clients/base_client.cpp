#include "base_client.h"

//#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
//#include <memory>
#include <string.h>

base_client::base_client(int recv_buffer_size)
	: _run(false)
	, _portnumber(-1)
	, _fd(INVALID_SOCKET)
	, _send_buffer_size(MAX_SEND_BUFFER_SIZE)
	, _send_buffer(nullptr)
	, _recv_buffer_size(recv_buffer_size)
	, _recv_buffer(nullptr)
	, _circular_buffer(nullptr)
{
	WSADATA wsa_data;
	WSAStartup(MAKEWORD(2, 2), &wsa_data);
	pthread_mutex_init(&_lock, 0);
	memset(_address, 0x00, sizeof(_address));

	pthread_mutex_init(&_send_buffer_lock, 0);
	_circular_buffer = sirius::circular::create(MAX_SEND_BUFFER_SIZE);
	_send_circular_buffer = static_cast<base_client::buffer_t*>(malloc(sizeof(base_client::buffer_t)));
	init(_send_circular_buffer);
	_send_buffer = static_cast<char*>(malloc(_send_buffer_size));
	_recv_buffer = static_cast<char*>(malloc(_recv_buffer_size));
}

base_client::~base_client(void)
{
	if (_recv_buffer)
	{
		free(_recv_buffer);
		_recv_buffer = nullptr;
	}

	if (_send_buffer)
	{
		free(_send_buffer);
		_send_buffer = nullptr;
	}

	while (_send_circular_buffer->next)
	{
		base_client::buffer_t * buffer = _send_circular_buffer->next;
		_send_circular_buffer->next = buffer->next;
		free(buffer);
	}

	if (_send_circular_buffer)
	{
		free(_send_circular_buffer);
		_send_circular_buffer = nullptr;
	}
	pthread_mutex_destroy(&_send_buffer_lock);

	sirius::circular::destroy(_circular_buffer);
	_circular_buffer = nullptr;

	pthread_mutex_destroy(&_lock);
	WSACleanup();
}

int base_client::start(const char * address, int portnumber)
{
	strncpy_s(_address, sizeof(_address)-1, address, strlen(address));
	_portnumber = portnumber;
	_run = true;
	pthread_create(&_thread, 0, &base_client::process_cb, this);

	return base_client::err_code_t::success;
}

int base_client::stop(void)
{
	_run = false;
	pthread_join(_thread, 0);

	return base_client::err_code_t::success;
}

int base_client::send(const char * packet, int packet_size)
{
	return push(packet, packet_size, 0);
}

int base_client::connect(const char * address, int portnumber)
{
	_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_fd == INVALID_SOCKET)
		return base_client::err_code_t::fail;

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(address);
	server_addr.sin_port = htons(portnumber);

	int val = ::connect(_fd, (SOCKADDR*)&server_addr, sizeof(server_addr));
	if (val == SOCKET_ERROR)
	{
		val = ::closesocket(_fd);
		if (val == SOCKET_ERROR)
			return base_client::err_code_t::closesocket_error;
		else
			return base_client::err_code_t::connection_error;
	}
	return base_client::err_code_t::success;
}

int base_client::disconnect(void)
{
	int val = ::closesocket(_fd);
	if (val == SOCKET_ERROR)
		return base_client::err_code_t::closesocket_error;

	return base_client::err_code_t::success;
}

void base_client::process(void * self)
{
	int val = connect(_address, _portnumber);
	if (val == base_client::err_code_t::success)
	{
		fd_set			orset;
		fd_set			owset;
		fd_set			rset;
		fd_set			wset;
		timeval			timeout;

		FD_ZERO(&orset);
		FD_ZERO(&owset);
		FD_SET(_fd, &orset);
		FD_SET(_fd, &owset);

		int				recv_size = 0;
		int				send_size = 0;
		on_connect_to_server();
		while (_run)
		{
			timeout.tv_sec = 0;
			timeout.tv_usec = 100000;
			rset = orset;
			wset = owset;

			int sel = select(_fd + 1, &rset, &wset, NULL, &timeout);
			if (sel == -1)
			{
				DWORD err = ::WSAGetLastError();
				OutputDebugStringA("error on select\n");
			}

			if (FD_ISSET(_fd, &rset))
			{
				recv_size = ::recv(_fd, _recv_buffer, _recv_buffer_size, 0);
				if (recv_size == -1)
				{
					OutputDebugStringA("recv error\n");
				}
				else
				{
					on_recv(_recv_buffer, recv_size);
				}
			}

			if (FD_ISSET(_fd, &wset))
			{
				long long timestamp = 0;
				pop(_send_buffer, send_size, timestamp);
				if (send_size > 0)
				{
					::send(_fd, _send_buffer, send_size, 0);
				}
			}
		}
		on_disconnect_from_server();
	}
	if (val == base_client::err_code_t::success)
		disconnect();
}

void * base_client::process_cb(void * param)
{
	base_client * self = static_cast<base_client*>(param);
	self->process(self);
	return 0;
}


int base_client::init(base_client::buffer_t * buffer)
{
	buffer->timestamp = 0;
	buffer->amount = 0;
	buffer->next = nullptr;
	buffer->prev = nullptr;
	return base_client::err_code_t::success;
}

int base_client::push(const char * data, int size, long long timestamp)
{
	int32_t status = base_client::err_code_t::success;
	if (data && size > 0)
	{
		scopedlock mutex(&_send_buffer_lock);
		base_client::buffer_t * buffer = _send_circular_buffer;
		buffer->amount = MAX_SEND_BUFFER_SIZE;

		//move to tail
		do
		{
			if (!buffer->next)
				break;
			buffer = buffer->next;
		} while (1);

		buffer->next = static_cast<buffer_t*>(malloc(sizeof(base_client::buffer_t)));
		init(buffer->next);
		buffer->next->prev = buffer;
		buffer = buffer->next;

		buffer->timestamp = timestamp;
		buffer->amount = size;
		int32_t result = sirius::circular::write(_circular_buffer, data, buffer->amount);
		if (result == -1)
			status = base_client::err_code_t::fail;
	}
	return status;
}

int base_client::pop(char * data, int & size, long long & timestamp)
{
	int32_t status = base_client::err_code_t::success;
	size = 0;
	scopedlock mutex(&_send_buffer_lock);
	base_client::buffer_t * buffer = _send_circular_buffer->next;
	if (buffer)
	{
		_send_circular_buffer->next = buffer->next;
		if (_send_circular_buffer->next)
			_send_circular_buffer->next->prev = _send_circular_buffer;

		int32_t result = sirius::circular::read(_circular_buffer, data, buffer->amount);
		if (result == -1)
			status = base_client::err_code_t::fail;
		//assert(status != base_client::err_code_t::fail);

		size = buffer->amount;
		timestamp = buffer->timestamp;
		free(buffer);
	}
	return status;
}

int base_client::flush(void)
{
	int32_t status = base_client::err_code_t::success;

	scopedlock mutex(&_send_buffer_lock);
	while (_send_circular_buffer->next)
	{
		base_client::buffer_t * buffer = _send_circular_buffer->next;
		_send_circular_buffer->next = buffer->next;
		sirius::circular::read(_circular_buffer, NULL, buffer->amount);
		free(buffer);
	}
	_send_circular_buffer->next = nullptr;
	return status;
}