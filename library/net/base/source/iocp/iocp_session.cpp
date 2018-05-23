#include "iocp_session.h"

#if defined(WITH_WORKING_AS_SERVER)
#include <iocp_server.h>
#else
#include <iocp_client.h>
#endif

#include <sirius_locks.h>
#include <sirius_log4cplus_logger.h>

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
	, _secure_send_thread(INVALID_HANDLE_VALUE)
	, _secure_send_run(FALSE)
	, _secure_recv_thread(INVALID_HANDLE_VALUE)
	, _secure_recv_run(FALSE)
	, _ndestroy_session(0)
{
	::InitializeCriticalSection(&_lock);

	_cnct_io_context = std::shared_ptr<sirius::library::net::iocp::session::io_context_t>(new sirius::library::net::iocp::session::io_context_t(1024, _tls));
	_cnct_io_context->operation = sirius::library::net::iocp::io_context_t::operation_t::connect;

	if (_tls)
	{
		_recv_io_context = std::shared_ptr<sirius::library::net::iocp::session::io_context_t>(new sirius::library::net::iocp::session::io_context_t(recv_buffer_size, _tls));
		_recv_io_context->operation = sirius::library::net::iocp::io_context_t::operation_t::recv;
		_recv_io_context->wsabuf.buf = _recv_io_context->ssl_packet;
		_recv_io_context->wsabuf.len = _recv_io_context->ssl_packet_capacity;
		for (int32_t index = 0; index < _nsend_io_context; index++)
		{
			_send_io_context[index] = std::shared_ptr<sirius::library::net::iocp::session::io_context_t>(new sirius::library::net::iocp::session::io_context_t(send_buffer_size, _tls));
			_send_io_context[index]->operation = sirius::library::net::iocp::io_context_t::operation_t::send;
			_send_io_context[index]->wsabuf.buf = _send_io_context[index]->ssl_packet;
			_send_io_context[index]->wsabuf.len = 0;
		}

		_ssl = SSL_new(_ssl_ctx);
		_bio[sirius::library::net::iocp::session::io_context_t::operation_t::send] = BIO_new(BIO_s_mem());
		_bio[sirius::library::net::iocp::session::io_context_t::operation_t::recv] = BIO_new(BIO_s_mem());
		SSL_set_bio(_ssl, _bio[sirius::library::net::iocp::session::io_context_t::operation_t::recv], _bio[sirius::library::net::iocp::session::io_context_t::operation_t::send]);

		_secure_send_run = TRUE;
		_secure_send_thread = (HANDLE)::_beginthreadex(NULL, 0, sirius::library::net::iocp::session::secure_send_process_cb, this, CREATE_SUSPENDED, NULL);
		
		_secure_recv_run = TRUE;
		_secure_recv_thread = (HANDLE)::_beginthreadex(NULL, 0, sirius::library::net::iocp::session::secure_recv_process_cb, this, CREATE_SUSPENDED, NULL);

		_status = sirius::library::net::iocp::session::status_t::handshaking;
	}
	else
	{
		_recv_io_context = std::shared_ptr<sirius::library::net::iocp::session::io_context_t>(new sirius::library::net::iocp::session::io_context_t(recv_buffer_size, _tls));
		_recv_io_context->operation = sirius::library::net::iocp::io_context_t::operation_t::recv;
		_recv_io_context->wsabuf.buf = _recv_io_context->packet;
		_recv_io_context->wsabuf.len = _recv_io_context->packet_capacity;
		for (int32_t index = 0; index < _nsend_io_context; index++)
		{
			_send_io_context[index] = std::shared_ptr<sirius::library::net::iocp::session::io_context_t>(new sirius::library::net::iocp::session::io_context_t(send_buffer_size, _tls));
			_send_io_context[index]->operation = sirius::library::net::iocp::io_context_t::operation_t::send;
			_send_io_context[index]->wsabuf.buf = _send_io_context[index]->packet;
			_send_io_context[index]->wsabuf.len = 0;
		}

		_status = sirius::library::net::iocp::session::status_t::none;
	}

	_timestamp = ::GetTickCount64();
}

sirius::library::net::iocp::session::~session(void)
{
	close();

	::DeleteCriticalSection(&_lock);

	::OutputDebugStringA("sirius::library::net::iocp::session::~session(void)\n");
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

std::shared_ptr<sirius::library::net::iocp::session::io_context_t> sirius::library::net::iocp::session::recv_context(void)
{
	return _recv_io_context;
}

BOOL sirius::library::net::iocp::session::pending(void)
{
	if (_recv_io_context->packet_size > 0)
		return TRUE;

	if ((_status & sirius::library::net::iocp::session::status_t::sending) == sirius::library::net::iocp::session::status_t::sending)
		return TRUE;

	if (_ssl)
	{
		BOOL is_send_bio_pending = FALSE;
		BOOL is_recv_bio_pending = FALSE;

		is_send_bio_pending = BIO_pending(_bio[sirius::library::net::iocp::session::io_context_t::operation_t::send]);
		is_recv_bio_pending = BIO_pending(_bio[sirius::library::net::iocp::session::io_context_t::operation_t::recv]);

		if (is_send_bio_pending > 0 || is_recv_bio_pending > 0)
			return TRUE;
	}
	return FALSE;
}

BOOL sirius::library::net::iocp::session::is_session_destroy(void)
{
	if (_ndestroy_session > 0)
		return TRUE;
	else
		return FALSE;
}

void sirius::library::net::iocp::session::increase_session_destroy_count(void)
{
	::InterlockedIncrement(&_ndestroy_session);

	sirius::library::log::log4cplus::logger::make_debug_log(SAA, "ndestroy_session_count=%d", _ndestroy_session);
}

void sirius::library::net::iocp::session::close(void)
{
	sirius::autolock lock(&_lock);

	_status |= sirius::library::net::iocp::session::status_t::closing;

#if defined(WITH_WORKING_AS_SERVER)
	if ((_secure_send_thread != NULL) && (_secure_send_thread != INVALID_HANDLE_VALUE) && _tls && 
		((_status & sirius::library::net::iocp::session::status_t::accepting) == sirius::library::net::iocp::session::status_t::accepting))
	{
		::ResumeThread(_secure_send_thread);
		::ResumeThread(_secure_recv_thread);
	}
#else
	if ((_secure_send_thread != NULL) && (_secure_send_thread != INVALID_HANDLE_VALUE) && _tls &&
		((_status & sirius::library::net::iocp::session::status_t::connecting) == sirius::library::net::iocp::session::status_t::connecting))
	{
		::ResumeThread(_secure_send_thread);
		::ResumeThread(_secure_recv_thread);
	}
#endif

	if ((_secure_send_thread != NULL) && (_secure_send_thread != INVALID_HANDLE_VALUE))
	{
		_secure_send_run = FALSE;
		if (::WaitForSingleObject(_secure_send_thread, INFINITE) == WAIT_OBJECT_0)
		{
			::CloseHandle(_secure_send_thread);
			_secure_send_thread = INVALID_HANDLE_VALUE;
		}
	}

	if ((_secure_recv_thread != NULL) && (_secure_recv_thread != INVALID_HANDLE_VALUE))
	{
		_secure_recv_run = FALSE;
		if (::WaitForSingleObject(_secure_recv_thread, INFINITE) == WAIT_OBJECT_0)
		{
			::CloseHandle(_secure_recv_thread);
			_secure_recv_thread = INVALID_HANDLE_VALUE;
		}
	}

	int32_t code = 0;
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
			code = ::WSAGetLastError();
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
	if (((_status & sirius::library::net::iocp::session::status_t::closing) != sirius::library::net::iocp::session::status_t::closing) &&
		((_status & sirius::library::net::iocp::session::status_t::closed) != sirius::library::net::iocp::session::status_t::closed))
	{
		if (_tls)
		{
			BOOL fatal_error_occurred = FALSE;
			for (int32_t index = 0; index < _nsend_io_context; index++)
			{
				sirius::autolock lock(&_send_io_context[index]->lock);

				if (_send_io_context[index]->packet_size < 1)
				{
					_send_io_context[index]->packet_size = packet_size;

					if (_send_io_context[index]->packet_capacity < packet_size)
						_send_io_context[index]->resize(packet_size);
					memmove(_send_io_context[index]->packet, packet, _send_io_context[index]->packet_size);

					int32_t bytes = 0;
					int32_t ssl_error = 0;

					bytes = SSL_write(_ssl, _send_io_context[index]->packet, _send_io_context[index]->packet_size);
					ssl_error = ssl_get_error(_ssl, bytes);
					if (bytes == _send_io_context[index]->packet_size)
					{
						//char message[MAX_PATH] = { 0 };
						//_snprintf_s(message, sizeof(message), "SSL_write Success : bytes[%d], index[%d]\n", bytes, index);
						//OutputDebugStringA(message);
						_send_io_context[index]->packet_size = 0;
					}
					else  if (ssl_is_fatal_error(ssl_error))
					{
						_send_io_context[index]->packet_size = 0;
						fatal_error_occurred = TRUE;
					}
					break;
				}
			}

			if (fatal_error_occurred)
				close();
		}
		else
		{
			DWORD nbytes = 0;
			for (int32_t index = 0; index < _nsend_io_context; index++)
			{
				sirius::autolock lock(&_send_io_context[index]->lock);
#if defined(WITH_WORKING_AS_SERVER)
				if ((_send_io_context[index]->wsabuf.len != 0) || (_socket_listen == INVALID_SOCKET) || (_socket_listen == NULL))
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
	}
}

void sirius::library::net::iocp::session::recv(int32_t packet_size)
{
	if (((_status & sirius::library::net::iocp::session::status_t::closing) != sirius::library::net::iocp::session::status_t::closing) &&
		((_status & sirius::library::net::iocp::session::status_t::closed) != sirius::library::net::iocp::session::status_t::closed) &&
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

			::ResumeThread(_secure_recv_thread);
			::ResumeThread(_secure_send_thread);

			_processor->on_session_handshaking(shared_from_this());
		}
		else
		{
			_status |= sirius::library::net::iocp::session::status_t::connected;
			_processor->on_session_connect(shared_from_this());
		}
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
		{
			SSL_set_accept_state(_ssl);

			::ResumeThread(_secure_recv_thread);
			::ResumeThread(_secure_send_thread);

			_processor->remove_accept_waiting_session(shared_from_this());
			_processor->on_session_handshaking(shared_from_this());
		}
		else
		{
			_processor->remove_accept_waiting_session(shared_from_this());
			_processor->on_session_connect(shared_from_this());
		}

		_processor->add_accept_waiting_session(new_session);
	}
}

void sirius::library::net::iocp::session::on_recv(std::shared_ptr<sirius::library::net::iocp::session::io_context_t> io_context)
{
	_status &= ~sirius::library::net::iocp::session::status_t::receiving;
	if (_tls)
	{
		BOOL fatal_error_occurred = FALSE;
		{
			sirius::autolock lock(&io_context->lock);
			if (io_context->ssl_packet_size > 0)
			{
				int32_t bytes		= 0;
				int32_t ssl_error	= 0;

				bytes = BIO_write(_bio[sirius::library::net::iocp::session::io_context_t::operation_t::recv], io_context->ssl_packet, io_context->ssl_packet_size);
				ssl_error = ssl_get_error(_ssl, bytes);

				if (bytes == io_context->ssl_packet_size)
				{
					//char message[MAX_PATH] = { 0 };
					//_snprintf_s(message, sizeof(message), "BIO_write Success : bytes[%d]\n", bytes);
					//OutputDebugStringA(message);
					io_context->ssl_packet_size = 0;
				}
				else
				{
					BOOL bio_should_retry = TRUE;
					
					bio_should_retry = BIO_should_retry(_bio[sirius::library::net::iocp::session::io_context_t::operation_t::recv]);

					if(!bio_should_retry)
						fatal_error_occurred = TRUE;
				}
			}
			else if (io_context->result != 0)
			{
				close();
			}
		}

		if (fatal_error_occurred)
			close();
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
	sirius::library::net::iocp::session::io_context_t * p = reinterpret_cast<sirius::library::net::iocp::session::io_context_t*>(overlapped);
	std::shared_ptr<sirius::library::net::iocp::session::io_context_t> io_context = p->shared_from_this();
	
	update_timestamp();

	DWORD dwOverlappeddNumberOfBytesTransferred = 0, dwOverlappedFlags = 0;
	BOOL succeeded = ::WSAGetOverlappedResult(_socket, overlapped, &dwOverlappeddNumberOfBytesTransferred, TRUE, &dwOverlappedFlags);
	io_context->result = ::WSAGetLastError();

	if (io_context->operation==sirius::library::net::iocp::io_context_t::operation_t::connect)
	{
		sirius::autolock slock(&io_context->lock);
		if ((_socket_listen == INVALID_SOCKET) || (_socket_listen == 0))
			on_connect(io_context);
		else
			on_accept(io_context);

		//sirius::library::log::log4cplus::logger::make_debug_log(SAA, "operation == on_accept");
	}
	else if (io_context->operation == sirius::library::net::iocp::io_context_t::operation_t::recv)
	{
		if (bytes_transfered > 0)
		{
			if (_tls)
			{
				sirius::autolock slock(&io_context->lock);
				io_context->ssl_packet_size = bytes_transfered;
			}
			else
			{
				sirius::autolock slock(&io_context->lock);
				io_context->packet_size = bytes_transfered;
			}
			on_recv(io_context);
		}
		else
		{
			_processor->on_session_close(shared_from_this());
		}
	}
	else if (io_context->operation == sirius::library::net::iocp::io_context_t::operation_t::send)
	{
		if (bytes_transfered > 0)
		{
			if (_tls)
			{
				sirius::autolock slock(&io_context->lock);
				io_context->ssl_packet_size = bytes_transfered;
			}
			else
			{
				sirius::autolock slock(&io_context->lock);
				io_context->packet_size = bytes_transfered;
			}
			on_send(io_context);
		}
		else
		{
			_processor->on_session_close(shared_from_this());
		}
	}
	else
	{
		sirius::library::log::log4cplus::logger::make_debug_log(SAA, "undefined operation in iocp completion routine");
	}

	if (_status == sirius::library::net::iocp::session::status_t::closed)
	{
		_processor->on_session_close(shared_from_this());
		//sirius::library::log::log4cplus::logger::make_debug_log(SAA, "operation == close");
	}
}

unsigned sirius::library::net::iocp::session::secure_recv_process_cb(void * param)
{
	sirius::library::net::iocp::session * self = static_cast<sirius::library::net::iocp::session*>(param);
	self->secure_recv_process();
	return 0;
}

void sirius::library::net::iocp::session::secure_recv_process(void)
{
	while (_secure_recv_run)
	{
		BOOL fatal_error_occurred = FALSE;
		{
			sirius::autolock lock(&_recv_io_context->lock);
			if (_recv_io_context->ssl_packet_size == 0 && _recv_io_context->packet_size == 0)
			{
				int32_t bytes = 0;
				do
				{
					int32_t ssl_error = 0;

					bytes = SSL_read(_ssl, _recv_io_context->packet, _recv_io_context->packet_capacity);
					ssl_error = ssl_get_error(_ssl, bytes);

					if (((_status & sirius::library::net::iocp::session::status_t::handshaking) == sirius::library::net::iocp::session::status_t::handshaking) && SSL_is_init_finished(_ssl))
					{					
						//OutputDebugStringA("SSL_read : Connected\n");
						_status &= ~sirius::library::net::iocp::session::status_t::handshaking;
						_processor->on_session_connect(shared_from_this());
						_status |= sirius::library::net::iocp::session::status_t::connected;
					}

					if (bytes > 0)
					{
						//char message[MAX_PATH] = { 0 };
						//_snprintf_s(message, sizeof(message), "SSL_read Success : bytes[%d]\n", bytes);
						//OutputDebugStringA(message);

						_recv_io_context->packet_size = bytes;
						int32_t packet_size = on_recv(_recv_io_context->packet, _recv_io_context->packet_size);
						_recv_io_context->packet_size = 0;
						recv(_recv_io_context->packet_capacity);
					}
					else if (ssl_is_fatal_error(ssl_error))
					{
						fatal_error_occurred = TRUE;
						break;
					}
					else if (!SSL_is_init_finished(_ssl))
					{
						recv(_recv_io_context->packet_capacity);
					}

				} while (bytes > 0);
			}
		}

		/*
		if (fatal_error_occurred)
		{
			close();
			break;
		}
		*/
		::Sleep(10);
	}
}

unsigned sirius::library::net::iocp::session::secure_send_process_cb(void * param)
{
	sirius::library::net::iocp::session * self = static_cast<sirius::library::net::iocp::session*>(param);
	self->secure_send_process();
	return 0;
}

void sirius::library::net::iocp::session::secure_send_process(void)
{
	DWORD nbytes = 0;
	while (_secure_send_run)
	{
		BOOL fatal_error_occurred = FALSE;
		for (int32_t index = 0; index < _nsend_io_context; index++)
		{
			sirius::autolock lock(&_send_io_context[index]->lock);

#if defined(WITH_WORKING_AS_SERVER)
			if ((_send_io_context[index]->wsabuf.len == 0) && ((_socket_listen != NULL && _socket_listen != INVALID_SOCKET) || BIO_pending(_bio[sirius::library::net::iocp::session::io_context_t::operation_t::send])))
#else
			if ((_send_io_context[index]->wsabuf.len == 0) && BIO_pending(_bio[sirius::library::net::iocp::session::io_context_t::operation_t::send]))
#endif
			{
				int32_t ssl_error	= 0;

				_send_io_context[index]->ssl_packet_size = BIO_read(_bio[sirius::library::net::iocp::session::io_context_t::operation_t::send], _send_io_context[index]->ssl_packet, _send_io_context[index]->ssl_packet_capacity);
				ssl_error = ssl_get_error(_ssl, _send_io_context[index]->ssl_packet_size);

				if (_send_io_context[index]->ssl_packet_size > 0)
				{
					//char message[MAX_PATH] = { 0 };
					//_snprintf_s(message, sizeof(message), "BIO_read Success : bytes[%d], index[%d]\n", _send_io_context[index]->ssl_packet_size, index);
					//OutputDebugStringA(message);

					_send_io_context[index]->wsabuf.len = _send_io_context[index]->ssl_packet_size;

					_status |= sirius::library::net::iocp::session::status_t::sending;

					::WSASend(_socket, &_send_io_context[index]->wsabuf, 1, &nbytes, 0, &_send_io_context[index]->overlapped, 0);
					_send_io_context[index]->result = ::WSAGetLastError();

					if ((_send_io_context[index]->result != 0) && (_send_io_context[index]->result != WSA_IO_PENDING))
					{
						_status &= ~sirius::library::net::iocp::session::status_t::sending;
						close();
					}

				}
				else if (ssl_is_fatal_error(ssl_error))
				{
					fatal_error_occurred = TRUE;
				}
			}
		}

		/*
		if (fatal_error_occurred)
		{
			close();
			break;
		}
		*/
		::Sleep(10);
	}
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
	case SSL_ERROR_SYSCALL:
		return FALSE;
	}
	return TRUE;
}

int32_t	sirius::library::net::iocp::session::ssl_get_error(SSL *ssl, int32_t result)
{
	int32_t error = SSL_ERROR_NONE;
	error = SSL_get_error(ssl, result);
	
	/*
	if (SSL_ERROR_NONE != error)
	{
		char message[1024] = { 0 };
		int error_log = error;
		while (SSL_ERROR_NONE != error_log)
		{
			ERR_error_string_n(error_log, message, _countof(message));
			if (ssl_is_fatal_error(error_log))
			{
				// print error message to console or logs
				::OutputDebugStringA(message);
				::OutputDebugStringA("\n");
			}
			error_log = ERR_get_error();
		}
	}
	*/
	return error;
}