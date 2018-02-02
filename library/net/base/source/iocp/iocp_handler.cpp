#include <platform.h>
#include <iocp_io_context.h>
#include <iocp_handler.h>

#if defined(WITH_WORKING_AS_SERVER)
#include "iocp_server.h"
#else
#include "iocp_client.h"
#endif

#include <sirius_locks.h>

sirius::library::net::iocp::handler::handler(sirius::library::net::iocp::endpoint * endpoint)
	: _iocp(NULL)
	, _endpoint(endpoint)
	, _nthreads(1)
{
}

sirius::library::net::iocp::handler::~handler(void)
{
}

BOOL sirius::library::net::iocp::handler::create(int32_t number_of_pooled_threads, int32_t * error_code)
{
	assert(number_of_pooled_threads >= 0);
	if (number_of_pooled_threads == 0)
	{
		SYSTEM_INFO si;
		GetNativeSystemInfo(&si);
		// 디폴트 쓰레드 수로 
		// 2 * 프로세서수 + 2 의 공식을 따랐음
		_nthreads = (si.dwNumberOfProcessors * 2) + 2;
	}
	else
	{
		_nthreads = number_of_pooled_threads;
	}

	_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, _nthreads);
	if (((_iocp == NULL) || (_iocp == INVALID_HANDLE_VALUE)))
	{
		if (error_code != NULL)
			*error_code = GetLastError();
		return FALSE;
	}
	return TRUE;
}

BOOL sirius::library::net::iocp::handler::destroy(void)
{
	if (_iocp)
		CloseHandle(_iocp);

	_iocp = INVALID_HANDLE_VALUE;
	return TRUE;
}

BOOL sirius::library::net::iocp::handler::associate(SOCKET socket, ULONG_PTR key, int32_t * error_code)
{
	assert(socket != INVALID_SOCKET);
	return associate((HANDLE)socket, key, error_code);
}

BOOL sirius::library::net::iocp::handler::associate(HANDLE handle, ULONG_PTR key, int32_t * error_code)
{
	assert(handle != INVALID_HANDLE_VALUE);
	//assert(key != 0);

	HANDLE iocp_handle = CreateIoCompletionPort(handle, _iocp, key, 0);
	if ((iocp_handle != _iocp) && (error_code != NULL))
	{
		*error_code = GetLastError();
	}

	return (iocp_handle == _iocp);
}

BOOL sirius::library::net::iocp::handler::post_completion_status(ULONG_PTR key, DWORD bytes_of_transfered, OVERLAPPED * overlapped, int32_t * error_code)
{
	BOOL value = PostQueuedCompletionStatus(_iocp, bytes_of_transfered, key, overlapped);
	if (!value && (error_code != NULL))
	{
		*error_code = GetLastError();
	}
	return value ? true : false;
}

BOOL sirius::library::net::iocp::handler::get_completion_status(ULONG_PTR * key, LPDWORD bytes_of_transfered, LPOVERLAPPED * overlapped, int32_t * error_code, DWORD waiting_time)
{
	BOOL value = GetQueuedCompletionStatus(_iocp, bytes_of_transfered, key, overlapped, waiting_time);
	if (!value && (error_code != NULL))
	{
		*error_code = GetLastError();
	}
	return value ? true : false;
}

void sirius::library::net::iocp::handler::create_thread_pool(void)
{
	_threads = (HANDLE*)malloc(sizeof(HANDLE) * _nthreads);
	for (int32_t i = 0; i<_nthreads; i++)
		_threads[i] = (HANDLE)_beginthreadex(NULL, 0, sirius::library::net::iocp::handler::process, this, 0, 0);
}

void sirius::library::net::iocp::handler::close_thread_pool(void)
{
	post_completion_status(0, 0, 0);
	::WaitForMultipleObjects(_nthreads, _threads, TRUE, INFINITE);
	for (int32_t i = 0; i<_nthreads; i++)
		::CloseHandle(_threads[i]);
	free(_threads);
}

unsigned __stdcall sirius::library::net::iocp::handler::process(void * param)
{
	sirius::library::net::iocp::handler *self = static_cast<sirius::library::net::iocp::handler*>(param);

	if (self && self->_endpoint)
		self->_endpoint->execute();
	return 0;
}