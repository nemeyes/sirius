#include <platform.h>
#include <iocp_io_context.h>
#include <iocp_handler.h>

#if defined(WITH_WORKING_AS_SERVER)
#include "iocp_server.h"
#else
#include "iocp_client.h"
#endif

#include <cap_auto_lock.h>

amadeus::library::net::iocp_handler::iocp_handler(amadeus::library::net::iocp_endpoint * iocp_endpoint)
	: _iocp_handle(NULL)
	, _iocp_endpoint(iocp_endpoint)
	, _number_of_threads(1)
{
	::InitializeCriticalSection(&_cs);
}

amadeus::library::net::iocp_handler::~iocp_handler(void)
{
	::DeleteCriticalSection(&_cs);

	if (_iocp_handle)
		CloseHandle(_iocp_handle);

	_iocp_handle = INVALID_HANDLE_VALUE;
}

bool amadeus::library::net::iocp_handler::create(int32_t number_of_pooled_threads, int32_t * error_code)
{
	assert(number_of_pooled_threads >= 0);
	if (number_of_pooled_threads == 0)
	{
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		// 디폴트 쓰레드 수로 
		// 2 * 프로세서수 + 2 의 공식을 따랐음
		_number_of_threads = (si.dwNumberOfProcessors * 2) + 2;
	}
	else
	{
		_number_of_threads = number_of_pooled_threads;
	}

	_iocp_handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, _number_of_threads);
	if (((_iocp_handle == NULL) || (_iocp_handle == INVALID_HANDLE_VALUE)))
	{
		if (error_code != NULL)
			*error_code = GetLastError();
		return false;
	}
	return true;
}

bool amadeus::library::net::iocp_handler::associate(SOCKET socket, ULONG_PTR key, int32_t * error_code)
{
	assert(socket != INVALID_SOCKET);
	return associate((HANDLE)socket, key, error_code);
}

bool amadeus::library::net::iocp_handler::associate(HANDLE handle, ULONG_PTR key, int32_t * error_code)
{
	assert(handle != INVALID_HANDLE_VALUE);
	assert(key != 0);

	HANDLE iocp_handle = CreateIoCompletionPort(handle, _iocp_handle, key, 0);
	if ((iocp_handle != _iocp_handle) && (error_code != NULL))
	{
		*error_code = GetLastError();
	}

	return (iocp_handle == _iocp_handle);
}

bool amadeus::library::net::iocp_handler::post_completion_status(ULONG_PTR key, DWORD bytes_of_transfered, OVERLAPPED * overlapped, int32_t * error_code)
{
	BOOL value = PostQueuedCompletionStatus(_iocp_handle, bytes_of_transfered, key, overlapped);
	if (!value && (error_code != NULL))
	{
		*error_code = GetLastError();
	}
	return value ? true : false;
}

bool amadeus::library::net::iocp_handler::get_completion_status(ULONG_PTR * key, LPDWORD bytes_of_transfered, LPOVERLAPPED * overlapped, int32_t * error_code, DWORD waiting_time)
{
	BOOL value = GetQueuedCompletionStatus(_iocp_handle, bytes_of_transfered, key, overlapped, waiting_time);
	if (!value && (error_code != NULL))
	{
		*error_code = GetLastError();
	}
	return value ? true : false;
}

void amadeus::library::net::iocp_handler::create_thread_pool(void)
{
	cap_auto_lock mutext(&_cs);
	//#pragma omp parallel
	for (int32_t i = 0; i<_number_of_threads; i++)
	{
		//#pragma omp single nowait
		unsigned int thread_id = 0;
		HANDLE thread_handle = (HANDLE)_beginthreadex(NULL, 0, iocp_handler::process, this, 0, &thread_id);
		_threads.push_back(thread_handle);
	}
}

void amadeus::library::net::iocp_handler::close_thread_pool(void)
{
	cap_auto_lock mutext(&_cs);

	//#pragma omp parallel
	{
		for (int32_t i = 0; i < _number_of_threads; i++)
		{
			//#pragma omp single nowait 
			if (!post_completion_status(KILL_THREAD))
				i--;
		}
	}

	//#pragma omp parallel
	std::vector<HANDLE>::iterator iter;
	for (iter = _threads.begin(); iter != _threads.end(); iter++)
	{
		//#pragma omp single nowait 
		HANDLE thread = (*iter);
		if (thread != INVALID_HANDLE_VALUE)
		{
			::WaitForSingleObjectEx(thread, INFINITE, FALSE);
			::CloseHandle(thread);
		}
	}
	_threads.clear();
}

unsigned __stdcall amadeus::library::net::iocp_handler::process(void * param)
{
	iocp_handler *self = static_cast<iocp_handler*>(param);

	if (self && self->_iocp_endpoint)
		self->_iocp_endpoint->execute();
	return 0;
}