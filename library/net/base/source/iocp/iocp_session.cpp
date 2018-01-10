#include "iocp_session.h"

#if defined(WITH_WORKING_AS_SERVER)
#include <iocp_server.h>
#else
#include <iocp_client.h>
#endif

#include <sirius_locks.h>

#define SEND_QUEUE_SIZE		100
#define SEND_BUFFER_SIZE	1024*1024*4
#define RECV_BUFFER_SIZE	1024*1024*4

sirius::library::net::iocp::session::session(sirius::library::net::iocp::processor * processor, int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, BOOL tls, SSL_CTX * ssl_ctx, BOOL reconnection)
	: _processor(processor)
	, _socket_listen(INVALID_SOCKET)
	, _socket(INVALID_SOCKET)
	, _so_recv_buffer_size(so_recv_buffer_size)
	, _so_send_buffer_size(so_send_buffer_size)
	, _recv_buffer_size(recv_buffer_size)
	, _send_buffer_size(send_buffer_size)
	, _pfn_connect(NULL)
	, _tls(tls)
	, _ssl_ctx(ssl_ctx)
	, _ssl(NULL)
	, _reconnection(reconnection)
	, _nsend_io_context(5)
{
	_cnct_io_context = std::shared_ptr<sirius::library::net::iocp::session::io_context_t>(new sirius::library::net::iocp::session::io_context_t(1024, _tls));
	_cnct_io_context->operation = sirius::library::net::iocp::io_context_t::operation_t::connect;
	_recv_io_context = std::shared_ptr<sirius::library::net::iocp::session::io_context_t>(new sirius::library::net::iocp::session::io_context_t(recv_buffer_size, _tls));
	_recv_io_context->operation = sirius::library::net::iocp::io_context_t::operation_t::recv;

	for (int32_t index = 0; index < _nsend_io_context; index++)
	{
		_send_io_context[index] = std::shared_ptr<sirius::library::net::iocp::session::io_context_t>(new sirius::library::net::iocp::session::io_context_t(send_buffer_size, _tls));
		_send_io_context[index]->operation = sirius::library::net::iocp::io_context_t::operation_t::send;
	}
	//::InitializeCriticalSection(&_lock);
	//::InitializeCriticalSection(&_connect_lock);
	if (_tls)
	{

		_recv_io_context->wsabuf.buf = _recv_io_context->ssl_packet;
		_recv_io_context->wsabuf.len = _recv_io_context->ssl_packet_capacity;
		for (int32_t index = 0; index < _nsend_io_context; index++)
		{
			_send_io_context[index]->wsabuf.buf = _send_io_context[index]->ssl_packet;
			_send_io_context[index]->wsabuf.len = 0;
		}

		_ssl = SSL_new(_ssl_ctx);
		_bio[sirius::library::net::iocp::session::io_context_t::operation_t::send] = BIO_new(BIO_s_mem());
		_bio[sirius::library::net::iocp::session::io_context_t::operation_t::recv] = BIO_new(BIO_s_mem());
		SSL_set_bio(_ssl, _bio[sirius::library::net::iocp::session::io_context_t::operation_t::recv], _bio[sirius::library::net::iocp::session::io_context_t::operation_t::send]);

		_status = sirius::library::net::iocp::session::status_t::handshaking;
	}
	else
	{
		_recv_io_context->wsabuf.buf = _recv_io_context->packet;
		_recv_io_context->wsabuf.len = _recv_io_context->packet_capacity;
		for (int32_t index = 0; index < _nsend_io_context; index++)
		{
			_send_io_context[index]->wsabuf.buf = _send_io_context[index]->packet;
			_send_io_context[index]->wsabuf.len = 0;
		}

		_status = sirius::library::net::iocp::session::status_t::none;
	}
}

sirius::library::net::iocp::session::~session(void)
{
	close();
	//::DeleteCriticalSection(&_connect_lock);
	//::DeleteCriticalSection(&_lock);
}

SOCKET sirius::library::net::iocp::session::listen_socket(void)
{
	return _socket_listen;
}

void sirius::library::net::iocp::session::listen_socket(SOCKET s)
{
	_socket_listen = s;
}

SOCKET sirius::library::net::iocp::session::socket(void)
{
	return _socket;
}

void sirius::library::net::iocp::session::socket(SOCKET s)
{
	_socket = s;
}

uint32_t sirius::library::net::iocp::session::status(void)
{
	return _status;
}

void sirius::library::net::iocp::session::status(uint32_t value)
{
	_status = value;
}

uint64_t sirius::library::net::iocp::session::timestamp(void)
{
	return _timestamp;
}

void sirius::library::net::iocp::session::update_timestamp(void)
{
	_timestamp = ::GetTickCount64();
}

BOOL sirius::library::net::iocp::session::pending(void)
{
	if (_recv_io_context->packet_size > 0)
		return TRUE;

	if ((_status & sirius::library::net::iocp::session::status_t::sending) == sirius::library::net::iocp::session::status_t::sending)
		return TRUE;

	if (_ssl)
	{
		if (BIO_pending(_bio[sirius::library::net::iocp::session::io_context_t::operation_t::send]) > 0 || BIO_pending(_bio[sirius::library::net::iocp::session::io_context_t::operation_t::recv]) > 0)
			return TRUE;
	}
	return FALSE;
}

void sirius::library::net::iocp::session::close(void)
{
	_status |= sirius::library::net::iocp::session::status_t::closing;
	if ((_socket != NULL && _socket != INVALID_SOCKET) && !pending())
	{
		LINGER linger;
		linger.l_onoff = 1;
		linger.l_linger = 0;
		setsockopt(_socket, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));
		if (closesocket(_socket) == 0)
		{
			_socket = INVALID_SOCKET;
		}
		else
		{
			int32_t code = ::WSAGetLastError();
			_socket = INVALID_SOCKET;
		}
	}


	if ((_socket == INVALID_SOCKET) && ((_status & sirius::library::net::iocp::session::status_t::operating) == 0))
	{
		if (_ssl)
		{
			SSL_shutdown(_ssl);
			SSL_free(_ssl);
			_ssl = NULL;
			_bio[sirius::library::net::iocp::session::io_context_t::operation_t::send] = NULL;
			_bio[sirius::library::net::iocp::session::io_context_t::operation_t::recv] = NULL;
		}
		_status = sirius::library::net::iocp::session::status_t::closed;
	}
}

void sirius::library::net::iocp::session::connect(const char * address, int32_t portnumber)
{
	if (address && (strlen(address) > 0) && (portnumber != -1))
	{
		strncpy_s(_address, address, MAX_PATH);
		_portnumber = portnumber;
	}

	sockaddr_storage addr = { 0 };
	sockaddr_in * addrin = (sockaddr_in*)&addr;
	addrinfo hints1 = { 0 };
	hints1.ai_family = AF_INET;
	hints1.ai_socktype = SOCK_STREAM;
	hints1.ai_protocol = IPPROTO_TCP;
	addrinfo * paddrinfo1 = 0;

	if (getaddrinfo(_address, "", &hints1, &paddrinfo1) == 0)
	{
		for (addrinfo * p = paddrinfo1; p != 0; p = p->ai_next)
		{
			memcpy_s(&addr, sizeof(addr), p->ai_addr, p->ai_addrlen);
			break;
		}
		freeaddrinfo(paddrinfo1);
		addrin->sin_port = htons(_portnumber);
	}

	memcpy_s(&_addresses[sirius::library::net::iocp::session::address_type_t::remote], sizeof(sockaddr_storage), &addr, sizeof(sockaddr_storage));

	int32_t err_code = 0;
	_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	setsockopt(_socket, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
	_processor->associate(_socket, (ULONG_PTR)this, &err_code);

	
	addrinfo hints = { 0 };
	hints.ai_family = addr.ss_family;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	addrinfo * paddrinfo = NULL;
	getaddrinfo(0, "", &hints, &paddrinfo);
	
	if (paddrinfo != NULL)
	{
		bind(_socket, paddrinfo->ai_addr, paddrinfo->ai_addrlen);
		freeaddrinfo(paddrinfo);
		paddrinfo = NULL;
	}

	{
		sirius::autolock lock(&_cnct_io_context->lock);
		if (!_pfn_connect)
		{
			DWORD	nbytes = 0;
			GUID	guid = WSAID_CONNECTEX;
			::WSAIoctl(_socket, SIO_GET_EXTENSION_FUNCTION_POINTER, (void*)&guid, sizeof(guid), (void*)&_pfn_connect, sizeof(_pfn_connect), &nbytes, NULL, NULL);
		}
	}

	_cnct_io_context->session	= this;
	_recv_io_context->session	= this;
	for (int32_t index = 0; index < _nsend_io_context; index++)
		_send_io_context[index]->session = this;

	_status |= sirius::library::net::iocp::session::status_t::connecting;
	(*_pfn_connect)(_socket, (sockaddr*)&addr, sizeof(sockaddr_storage), 0, 0, 0, &_cnct_io_context->overlapped);
	_cnct_io_context->result = ::WSAGetLastError();
	if ((_cnct_io_context->result != 0) && (_cnct_io_context->result != WSA_IO_PENDING))
	{
		_status &= ~sirius::library::net::iocp::session::status_t::connecting;
		close();
	}
}

void sirius::library::net::iocp::session::accept(void)
{
	int32_t err_code = 0;
	_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	_processor->associate(_socket, (ULONG_PTR)this, &err_code);

	_cnct_io_context->session	= this;
	_recv_io_context->session	= this;
	for (int32_t index = 0; index < _nsend_io_context; index++)
		_send_io_context[index]->session = this;

	_status |= sirius::library::net::iocp::session::status_t::accepting;
	BOOL accepted = ::AcceptEx(_socket_listen, _socket, _addresses, 0, sizeof(sockaddr_storage), sizeof(sockaddr_storage), 0, &(_cnct_io_context->overlapped));
	_cnct_io_context->result = ::WSAGetLastError();
	if (_cnct_io_context->result != WSA_IO_PENDING)
	{
		_status = sirius::library::net::iocp::session::status_t::none;
		close();
	}
}

void sirius::library::net::iocp::session::send(const char * packet, int32_t packet_size)
{
	DWORD nbytes = 0;

	if (_tls)
	{

	}
	else
	{
		for (int32_t index = 0; index < _nsend_io_context; index++)
		{
			sirius::autolock lock(&_send_io_context[index]->lock);
#if defined(WITH_WORKING_AS_SERVER)
			if ((_send_io_context[index]->wsabuf.len != 0) || (_socket_listen == INVALID_SOCKET)|| (_socket_listen == NULL))
#else
			if (_send_io_context[index]->wsabuf.len != 0)
#endif
				continue;

			if (_send_io_context[index]->packet_capacity < packet_size)
				_send_io_context[index]->resize(packet_size);

			memcpy_s(_send_io_context[index]->packet, packet_size, packet, packet_size);
			_send_io_context[index]->packet_size = packet_size;
			_send_io_context[index]->wsabuf.len = packet_size;
				
			_status |= sirius::library::net::iocp::session::status_t::sending;

			::WSASend(_socket, &_send_io_context[index]->wsabuf, 1, &nbytes, 0, &_send_io_context[index]->overlapped, 0);
			_send_io_context[index]->result = ::WSAGetLastError();

			if ((_send_io_context[index]->result != 0) && (_send_io_context[index]->result != WSA_IO_PENDING))
			{
				_status &= ~sirius::library::net::iocp::session::status_t::sending;
				close();
			}
			break;
		}
	}
	/*
			sirius::autolock lock(&_send_io_context[index]->lock);
			if (_send_io_context[index]->packet_size > 0)
			{
#if defined(WITH_WORKING_AS_SERVER)
				if (_send_io_context[index]->wsabuf.len == 0 && ((_socket_listen != INVALID_SOCKET) && (_socket_listen != 0)))
#else
				if (_send_io_context[index]->wsabuf.len == 0)
#endif
				{
					_send_io_context[index]->wsabuf.len = _send_io_context[index]->packet_size;
					_send_io_context[index]->packet_size = 0;
				}
			}	
	*/
	/*
	if (!send_io_context)
	{
		if (_nsend_io_context < 64)
		{
			_send_io_context[_nsend_io_context] = std::shared_ptr<sirius::library::net::iocp::session::io_context_t>(new sirius::library::net::iocp::session::io_context_t(_send_buffer_size, _tls));
			if (_tls)
				_send_io_context[_nsend_io_context]->wsabuf.buf = _send_io_context[_nsend_io_context]->ssl_packet;
			else
				_send_io_context[_nsend_io_context]->wsabuf.buf = _send_io_context[_nsend_io_context]->packet;

			_send_io_context[_nsend_io_context]->wsabuf.len = 0;
			_send_io_context[_nsend_io_context]->session = this;
			_nsend_io_context++;
		}
	}
	*/

	//process();
}

void sirius::library::net::iocp::session::recv(int32_t packet_size)
{
	if (((_status & sirius::library::net::iocp::session::status_t::closing) != sirius::library::net::iocp::session::status_t::closing) &&
		((_status & sirius::library::net::iocp::session::status_t::receiving) != sirius::library::net::iocp::session::status_t::receiving))
	{
		sirius::autolock lock(&_recv_io_context->lock);
		if (_recv_io_context->packet_size == 0)
		{
			_status |= sirius::library::net::iocp::session::status_t::receiving;
			DWORD size = 0;
			_recv_io_context->wsabuf.len = packet_size;
			::WSARecv(_socket, &_recv_io_context->wsabuf, 1, &size, &_wsa_flags[sirius::library::net::iocp::session::io_context_t::operation_t::recv], &_recv_io_context->overlapped, 0);
			_recv_io_context->result = ::WSAGetLastError();
			if ((_recv_io_context->result != 0) && (_recv_io_context->result != WSA_IO_PENDING))
			{
				_status &= ~sirius::library::net::iocp::session::status_t::receiving;
				close();
			}
		}
	}
}

void sirius::library::net::iocp::session::on_connect(std::shared_ptr<sirius::library::net::iocp::session::io_context_t> io_context)
{
	_status &= ~sirius::library::net::iocp::session::status_t::connecting;
	if (io_context->result == 0)
	{
		int32_t size = sizeof(_addresses[sirius::library::net::iocp::session::address_type_t::local]);
		getsockname(_socket, (sockaddr*)&_addresses[sirius::library::net::iocp::session::address_type_t::local], &size);

		int32_t zero = 1;
		setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&zero, sizeof(int32_t));
		zero = _so_recv_buffer_size;
		setsockopt(_socket, SOL_SOCKET, SO_RCVBUF, (const char*)&zero, sizeof(int32_t));
		zero = _so_send_buffer_size;
		setsockopt(_socket, SOL_SOCKET, SO_SNDBUF, (const char*)&zero, sizeof(int32_t));
		zero = 0;
		setsockopt(_socket, SOL_SOCKET, TCP_NODELAY, (char*)&zero, sizeof(int32_t));//disable nagle algorithm

		if (_tls)
		{
			_status |= sirius::library::net::iocp::session::status_t::handshaking;
			SSL_set_connect_state(_ssl);
		}
		else
		{
			_processor->on_session_connect(shared_from_this());
			_status |= sirius::library::net::iocp::session::status_t::connected;
		}
		//process();
	}
	/*
	else
	{
		_processor->on_session_connect(shared_from_this());
		close();

		if (_reconnection)
		{
			std::shared_ptr<sirius::library::net::iocp::session> new_session = _processor->create_session(_so_recv_buffer_size, _so_send_buffer_size, _recv_buffer_size, _send_buffer_size, _tls, _ssl_ctx);
			new_session->connect();
		}
	}
	*/
}

void sirius::library::net::iocp::session::on_accept(std::shared_ptr<sirius::library::net::iocp::session::io_context_t> io_context)
{
	if (io_context->result == 0)
	{
		std::shared_ptr<sirius::library::net::iocp::session> new_session = _processor->create_session(_so_recv_buffer_size, _so_send_buffer_size, _recv_buffer_size, _send_buffer_size, _tls, _ssl_ctx);
		new_session->_socket_listen = _socket_listen;
		new_session->accept();

		_status &= ~sirius::library::net::iocp::session::status_t::accepting;
		if (_tls)
			_status |= sirius::library::net::iocp::session::status_t::handshaking;
		else
			_status |= sirius::library::net::iocp::session::status_t::connected;

		int32_t		naddrslen[2]	= { 0, 0 };
		sockaddr *	paddrs[2]		= { 0, 0 };
		::GetAcceptExSockaddrs(_addresses, 0, sizeof(sockaddr_storage), sizeof(sockaddr_storage), &paddrs[sirius::library::net::iocp::session::address_type_t::local], &naddrslen[sirius::library::net::iocp::session::address_type_t::local], &paddrs[sirius::library::net::iocp::session::address_type_t::remote], &naddrslen[sirius::library::net::iocp::session::address_type_t::remote]);
		memcpy_s(&_addresses[sirius::library::net::iocp::session::address_type_t::local], sizeof(sockaddr_storage), paddrs[sirius::library::net::iocp::session::address_type_t::local], naddrslen[sirius::library::net::iocp::session::address_type_t::local]);
		memcpy_s(&_addresses[sirius::library::net::iocp::session::address_type_t::remote], sizeof(sockaddr_storage), paddrs[sirius::library::net::iocp::session::address_type_t::remote], naddrslen[sirius::library::net::iocp::session::address_type_t::remote]);
		setsockopt(_socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&_socket_listen, sizeof(SOCKET));

		int32_t resueaddr_enable = 1;
		if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&resueaddr_enable, sizeof(int32_t)) == SOCKET_ERROR)
			close();

		if (setsockopt(_socket, SOL_SOCKET, SO_RCVBUF, (const char*)&_so_recv_buffer_size, sizeof(int32_t)) == SOCKET_ERROR)
			close();

		if (setsockopt(_socket, SOL_SOCKET, SO_SNDBUF, (const char*)&_so_send_buffer_size, sizeof(int32_t)) == SOCKET_ERROR)
			close();

		bool tcp_nodelay_enable = false;
		if (setsockopt(_socket, SOL_SOCKET, TCP_NODELAY, (char *)&tcp_nodelay_enable, sizeof(int32_t)) == SOCKET_ERROR)
			close();

		if (_tls)
			SSL_set_accept_state(_ssl);
		else
			_processor->on_session_connect(shared_from_this());

		_processor->accept_session(new_session);
	}
}

void sirius::library::net::iocp::session::on_recv(std::shared_ptr<sirius::library::net::iocp::session::io_context_t> io_context)
{
	_status &= ~sirius::library::net::iocp::session::status_t::receiving;
	if (_tls)
	{

	}
	else
	{
		sirius::autolock lock(&io_context->lock);
		if (io_context->packet_size > 0)
		{
			int32_t packet_size = on_recv(io_context->packet, io_context->packet_size);
			io_context->packet_size = 0;
			recv(packet_size);
		}
		else if (io_context->result != 0)
		{
			close();
		}
	}
}

void sirius::library::net::iocp::session::on_send(std::shared_ptr<sirius::library::net::iocp::session::io_context_t> io_context)
{
	{
		sirius::autolock lock(&io_context->lock);
		io_context->wsabuf.len = 0;
	}

	if (io_context->result == 0)
	{
		BOOL sending = FALSE;
		for (int32_t index = 0; index < _nsend_io_context; index++)
		{
			if (_send_io_context[index]->wsabuf.len > 0)
			{
				sending = TRUE;
				break;
			}
		}

		if (!sending)
			_status &= ~sirius::library::net::iocp::session::status_t::sending;
	}
	else
	{
		_status &= ~sirius::library::net::iocp::session::status_t::sending;
		close();
	}
}

void sirius::library::net::iocp::session::on_completed(DWORD bytes_transfered, LPOVERLAPPED overlapped)
{
	sirius::library::net::iocp::session::io_context_t * p = (sirius::library::net::iocp::session::io_context_t*)overlapped;
	std::shared_ptr<sirius::library::net::iocp::session::io_context_t> io_context = p->shared_from_this();
	std::shared_ptr<sirius::library::net::iocp::session> self = shared_from_this();
	if (self)
		self->update_timestamp();

	DWORD dwOverlappeddNumberOfBytesTransferred = 0, dwOverlappedFlags = 0;
	BOOL succeeded = ::WSAGetOverlappedResult(_socket, overlapped, &dwOverlappeddNumberOfBytesTransferred, TRUE, &dwOverlappedFlags);
	io_context->result = ::WSAGetLastError();

	if (io_context->operation==sirius::library::net::iocp::io_context_t::operation_t::connect)
	{
		if ((_socket_listen == INVALID_SOCKET) || (_socket_listen == 0))
			on_connect(io_context);
		else
			on_accept(io_context);
	}
	else if (io_context->operation == sirius::library::net::iocp::io_context_t::operation_t::recv)
	{
		if(_tls)
			io_context->ssl_packet_size = bytes_transfered;
		else
			io_context->packet_size = bytes_transfered;
		on_recv(io_context);
	}
	else if (io_context->operation == sirius::library::net::iocp::io_context_t::operation_t::send)
	{
		if (_tls)
			io_context->ssl_packet_size = bytes_transfered;
		else
			io_context->packet_size = bytes_transfered;
		on_send(io_context);
	}

	if (_status == sirius::library::net::iocp::session::status_t::closed)
	{
		_processor->on_session_close(self);
	}
}

BOOL sirius::library::net::iocp::session::process(void)
{
	BOOL fatal_error_occurred = FALSE;
	if (_ssl)
	{
		/*
		if (_recv_io_context->bytes_transfered > 0)
		{
			int32_t bytes = BIO_write(_bio[sirius::library::net::iocp::session::io_context_t::operation_t::recv], _recv_io_context->ssl_packet, _recv_io_context->bytes_transfered);
			int32_t ssl_error = ssl_get_error(_ssl, bytes);
			if (bytes == _recv_io_context->bytes_transfered)
			{
				_recv_io_context->bytes_transfered = 0;
			}
			else if (!BIO_should_retry(_bio[sirius::library::net::iocp::session::io_context_t::operation_t::recv]))
			{
				fatal_error_occurred = TRUE;
			}
		}

		if (_recv_io_context->packet_size == 0)
		{
			int32_t bytes = 0;
			do
			{
				bytes = SSL_read(_ssl, _recv_io_context->packet, _recv_io_context->packet_capacity);
				int32_t ssl_error = ssl_get_error(_ssl, bytes);

				if (((_status & sirius::library::net::iocp::session::status_t::handshaking) == sirius::library::net::iocp::session::status_t::handshaking) && SSL_is_init_finished(_ssl))
				{
					_status &= ~sirius::library::net::iocp::session::status_t::handshaking;
					_status |= sirius::library::net::iocp::session::status_t::connected;

					_processor->on_session_connect(shared_from_this());
				}

				if (bytes > 0)
				{
					_recv_io_context->packet_size = bytes;
					_processor->on_session_recv(shared_from_this());
					_recv_io_context->packet_size = 0;
				}
				else if (ssl_is_fatal_error(ssl_error))
				{
					fatal_error_occurred = TRUE;
				}
			} while (bytes > 0);
		}

		if (_send_io_context[0]->packet_size > 0)
		{
			int32_t bytes = SSL_write(_ssl, _send_io_context[0]->packet, _send_io_context[0]->packet_size);
			int32_t ssl_error = ssl_get_error(_ssl, bytes);
			if (_send_io_context[0]->packet_size == bytes)
			{
				_send_io_context[0]->packet_size = 0;
			}
			else if (ssl_is_fatal_error(ssl_error))
			{
				fatal_error_occurred = TRUE;
			}
		}

#if defined(WITH_WORKING_AS_SERVER)
		if (_send_io_context[0]->wsabuf.len == 0 && (((_socket_listen != INVALID_SOCKET) && (_socket_listen != 0)) || BIO_pending(_bio[sirius::library::net::iocp::session::io_context_t::operation_t::send])))
#else
		if (_send_io_context[0]->wsabuf.len == 0 && BIO_pending(_bio[sirius::library::net::iocp::session::io_context_t::operation_t::send]))
#endif
		{
			int32_t bytes = BIO_read(_bio[sirius::library::net::iocp::session::io_context_t::operation_t::send], _send_io_context[0]->ssl_packet, _send_io_context[0]->ssl_packet_capacity);
			int32_t ssl_error = ssl_get_error(_ssl, bytes);
			if (bytes > 0)
			{
				_send_io_context[0]->wsabuf.len = bytes;
			}
			else if (ssl_is_fatal_error(ssl_error))
			{
				fatal_error_occurred = TRUE;
			}
		}

		if (fatal_error_occurred)
			close();
		*/
	}
	else
	{
		/*
		{
			sirius::autolock lock(&_recv_io_context->lock);
			if (_recv_io_context->bytes_transfered > 0)
			{
				_recv_io_context->packet_size = _recv_io_context->bytes_transfered;
				_processor->on_session_recv(shared_from_this());
				_recv_io_context->packet_size = 0;
				_recv_io_context->bytes_transfered = 0;
			}
		}

		for (int32_t index = 0; index < _nsend_io_context; index++)
		{
			sirius::autolock lock(&_send_io_context[index]->lock);
			if (_send_io_context[index]->packet_size > 0)
			{
#if defined(WITH_WORKING_AS_SERVER)
				if (_send_io_context[index]->wsabuf.len == 0 && ((_socket_listen != INVALID_SOCKET) && (_socket_listen != 0)))
#else
				if (_send_io_context[index]->wsabuf.len == 0)
#endif
				{
					_send_io_context[index]->wsabuf.len = _send_io_context[index]->packet_size;
					_send_io_context[index]->packet_size = 0;
				}
			}
		}
		*/
	}

	//send();
	//recv();

	return (!fatal_error_occurred);
}

BOOL sirius::library::net::iocp::session::ssl_is_fatal_error(int32_t error)
{
	switch (error)
	{
	case SSL_ERROR_NONE:
	case SSL_ERROR_WANT_READ:
	case SSL_ERROR_WANT_WRITE:
	case SSL_ERROR_WANT_CONNECT:
	case SSL_ERROR_WANT_ACCEPT:
		return FALSE;
	}
	return TRUE;
}

int32_t	sirius::library::net::iocp::session::ssl_get_error(SSL *ssl, int32_t result)
{
	int error = SSL_get_error(ssl, result);
	if (SSL_ERROR_NONE != error)
	{
		char message[512] = { 0 };
		int error_log = error;
		while (SSL_ERROR_NONE != error_log)
		{
			ERR_error_string_n(error_log, message, _countof(message));
			if (ssl_is_fatal_error(error_log))
			{
				// print error message to console or logs
			}
			error_log = ERR_get_error();
		}
	}
	return error;
}