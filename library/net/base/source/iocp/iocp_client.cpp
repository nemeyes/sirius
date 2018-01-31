#include <platform.h>
#include <iocp_client.h>
#include <sirius_log4cplus_logger.h>
#include <sirius_locks.h>

#if _MSC_VER >= 1900
FILE _iob[] = { *stdin, *stdout, *stderr };
extern "C" FILE * __cdecl __iob_func(void) { return _iob; }
#pragma comment (lib, "legacy_stdio_definitions.lib")
#endif

int32_t				sirius::library::net::iocp::client::_nssl_locks = 0;
CRITICAL_SECTION *	sirius::library::net::iocp::client::_ssl_locks = nullptr;
SSL_CTX *			sirius::library::net::iocp::client::_ssl_ctx = nullptr;

sirius::library::net::iocp::client::client(int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, BOOL tls)
	: _so_recv_buffer_size(so_recv_buffer_size)
	, _so_send_buffer_size(so_send_buffer_size)
	, _recv_buffer_size(recv_buffer_size)
	, _send_buffer_size(send_buffer_size)
	, _run(FALSE)
	, _thread(INVALID_HANDLE_VALUE)
	, _tls(tls)
	, _reconnection(FALSE)
	, _io_thread_pool_count(0)
	, _waiting(FALSE)
	, _on_handshaking(FALSE)
	, _on_connected(FALSE)
	, _on_disconnected(FALSE)
{
	_iocp = new sirius::library::net::iocp::handler(this);
}

sirius::library::net::iocp::client::~client(void)
{
	if (_iocp)
		delete _iocp;
	_iocp = nullptr;
}

int32_t sirius::library::net::iocp::client::initialize(void)
{
	WSADATA wsd;
	if (::WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
		return sirius::library::net::iocp::client::err_code_t::fail;
	return sirius::library::net::iocp::client::err_code_t::success;
}

int32_t sirius::library::net::iocp::client::release(void)
{
	::WSACleanup();
	return sirius::library::net::iocp::client::err_code_t::success;
}

int32_t	sirius::library::net::iocp::client::connect(const char * address, int32_t portnumber, int32_t io_thread_pool_count, BOOL reconnection)
{
	if (address && strlen(address) > 0)
		strncpy_s(_address, address, MAX_PATH);
	_portnumber = portnumber;
	_reconnection = reconnection;
	_io_thread_pool_count = io_thread_pool_count;

	uint32_t thread_id;
	_run = TRUE;
	_thread = (HANDLE)_beginthreadex(NULL, 0, process_cb, this, 0, &thread_id);
	if (_thread == NULL || _thread == INVALID_HANDLE_VALUE)
		return sirius::library::net::iocp::client::err_code_t::fail;

	return sirius::library::net::iocp::client::err_code_t::success;
}

int32_t	sirius::library::net::iocp::client::disconnect(void)
{
	if (_thread != NULL && _thread != INVALID_HANDLE_VALUE)
	{
		_reconnection = FALSE;
		_run = FALSE;
		if (::WaitForSingleObject(_thread, INFINITE) == WAIT_OBJECT_0)
			::CloseHandle(_thread);
	}

	return sirius::library::net::iocp::client::err_code_t::success;
}

void sirius::library::net::iocp::client::disconnect(BOOL enable)
{
	//_reconnection = !enable;
	_run = !enable;
}

void sirius::library::net::iocp::client::close_waiting_flag(BOOL enable)
{
	_waiting = enable;
}

BOOL sirius::library::net::iocp::client::active(void) const
{
	return _run;
}

BOOL sirius::library::net::iocp::client::associate(SOCKET socket, ULONG_PTR key, int32_t * err_code)
{
	return _iocp->associate(socket, key, err_code);
}

void sirius::library::net::iocp::client::data_request(std::shared_ptr<sirius::library::net::iocp::session> session, const char * packet, int32_t packet_size)
{
	session->send(packet, packet_size);
}

void sirius::library::net::iocp::client::on_session_handshaking(std::shared_ptr<sirius::library::net::iocp::session> session)
{
	_on_handshaking = TRUE;
	_on_connected = FALSE;
	_on_disconnected = FALSE;
	session->recv(session->recv_context()->packet_capacity);
	on_app_session_handshaking(session);
}

void sirius::library::net::iocp::client::on_session_connect(std::shared_ptr<sirius::library::net::iocp::session> session)
{
	_on_connected = TRUE;
	_on_handshaking = FALSE;
	_on_disconnected = FALSE;
	if(_tls)
		session->recv(session->recv_context()->packet_capacity);
	else
		session->recv(session->packet_header_size());
	on_app_session_connect(session);
}

void sirius::library::net::iocp::client::on_session_close(std::shared_ptr<sirius::library::net::iocp::session> session)
{
	on_app_session_close(session);
	destroy_session(session);
	_on_disconnected = TRUE;
	_on_connected = FALSE;
	_on_handshaking = FALSE;
}

void sirius::library::net::iocp::client::execute(void)
{
	while (TRUE)
	{
		ULONG_PTR		completion_key = 0;
		LPOVERLAPPED	overlapped = 0;
		DWORD			nbytes = 0;
		int32_t			err_code = 0;

		BOOL value = _iocp->get_completion_status(&completion_key, &nbytes, &overlapped, &err_code);
		if (completion_key == NULL && overlapped == NULL)
		{
			_iocp->post_completion_status(0, 0, 0, &err_code);
			break;
		}

		sirius::library::net::iocp::session::io_context_t * io_context = reinterpret_cast<sirius::library::net::iocp::session::io_context_t*>(overlapped);
		if(io_context && io_context->session)
			io_context->session->on_completed(nbytes, overlapped);
	}
}

void sirius::library::net::iocp::client::initialization_tls(void)
{
	_nssl_locks = CRYPTO_num_locks();
	if (_nssl_locks > 0)
	{
		_ssl_locks = (CRITICAL_SECTION*)malloc(_nssl_locks * sizeof(CRITICAL_SECTION));
		for (int32_t n = 0; n < _nssl_locks; ++n)
			InitializeCriticalSection(&_ssl_locks[n]);
	}

#ifdef _DEBUG
	CRYPTO_malloc_debug_init();
	CRYPTO_dbg_set_options(V_CRYPTO_MDEBUG_ALL);
	CRYPTO_mem_ctrl(CRYPTO_MEM_CHECK_ON);
#endif

	CRYPTO_set_locking_callback(&ssl_lock_callback);
	CRYPTO_set_dynlock_create_callback(&ssl_lock_dyn_create_callback);
	CRYPTO_set_dynlock_lock_callback(&ssl_lock_dyn_callback);
	CRYPTO_set_dynlock_destroy_callback(&ssl_lock_dyn_destroy_callback);

	SSL_load_error_strings();
	SSL_library_init();

	const SSL_METHOD * meth = TLSv1_2_method(); //SSLv23_method();

	_ssl_ctx = SSL_CTX_new(meth);
	SSL_CTX_set_verify(_ssl_ctx, SSL_VERIFY_NONE, nullptr);
}

void sirius::library::net::iocp::client::release_tls(void)
{
	SSL_CTX_free(_ssl_ctx);
	_ssl_ctx = nullptr;

	CRYPTO_set_locking_callback(NULL);
	CRYPTO_set_dynlock_create_callback(NULL);
	CRYPTO_set_dynlock_lock_callback(NULL);
	CRYPTO_set_dynlock_destroy_callback(NULL);

	EVP_cleanup();
	CRYPTO_cleanup_all_ex_data();
	ERR_remove_state(0);
	ERR_free_strings();

	if (nullptr != _ssl_locks)
	{
		for (int n = 0; n < _nssl_locks; ++n)
			DeleteCriticalSection(&_ssl_locks[n]);

		free(_ssl_locks);
		_ssl_locks = nullptr;
		_nssl_locks = 0;
	}
}

void sirius::library::net::iocp::client::ssl_lock_callback(int mode, int n, const char * file, int line)
{
	if (mode & CRYPTO_LOCK)
		::EnterCriticalSection(&_ssl_locks[n]);
	else
		::LeaveCriticalSection(&_ssl_locks[n]);
}

CRYPTO_dynlock_value * sirius::library::net::iocp::client::ssl_lock_dyn_create_callback(const char * file, int line)
{
	CRYPTO_dynlock_value * l = (CRYPTO_dynlock_value*)malloc(sizeof(CRYPTO_dynlock_value));
	::InitializeCriticalSection(&l->lock);
	return l;
}

void sirius::library::net::iocp::client::ssl_lock_dyn_callback(int mode, CRYPTO_dynlock_value * l, const char * file, int line)
{
	if (mode & CRYPTO_LOCK)
		::EnterCriticalSection(&l->lock);
	else
		::LeaveCriticalSection(&l->lock);
}

void sirius::library::net::iocp::client::ssl_lock_dyn_destroy_callback(CRYPTO_dynlock_value * l, const char * file, int line)
{
	::DeleteCriticalSection(&l->lock);
	free(l);
}

unsigned __stdcall sirius::library::net::iocp::client::process_cb(void * param)
{
	sirius::library::net::iocp::client * self = static_cast<sirius::library::net::iocp::client*>(param);
	self->process();
	return 0;
}

void sirius::library::net::iocp::client::process(void)
{
	if (_tls)
		initialization_tls();

	do
	{
		int32_t err_code;
		if (!_iocp->create(_io_thread_pool_count, &err_code))
			return;

		_iocp->create_thread_pool();

		std::shared_ptr<sirius::library::net::iocp::session> session = create_session(_so_recv_buffer_size, _so_send_buffer_size, _recv_buffer_size, _send_buffer_size, _tls, _ssl_ctx);
		session->connect(_address, _portnumber);

		for(int32_t t = 0; t < 20 && !_on_handshaking && !_on_connected; t++)
			::Sleep(50);

		if (_on_handshaking || _on_connected)
		{
			//_run = TRUE;
			on_start();
			on_running();
			on_stop();
		}
		else
		{
			continue;
		}

		if (_iocp)
			_iocp->close_thread_pool();
		_iocp->destroy();

		
	} while (_reconnection);

	if (_tls)
		release_tls();
}