#ifndef _IOCP_HANDLER_H_
#define _IOCP_HANDLER_H_

#include <platform.h>

namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace iocp
			{
#if defined(WITH_WORKING_AS_SERVER)
#define			endpoint server
#else
#define			endpoint client
#endif
				class endpoint;
				class handler
				{
				public:
					handler(sirius::library::net::iocp::endpoint * endpoint);
					virtual ~handler(void);

					BOOL create(int32_t number_of_pooled_threads = 0, int32_t * error_code = NULL);
					BOOL destroy(void);
					BOOL associate(SOCKET socket, ULONG_PTR key, int32_t * error_code = NULL);
					BOOL associate(HANDLE handle, ULONG_PTR key, int32_t * error_code = NULL);
					BOOL post_completion_status(ULONG_PTR key, DWORD bytes_of_transfered = 0, OVERLAPPED * overlapped = NULL, int32_t * error_code = NULL);
					BOOL get_completion_status(ULONG_PTR * key, LPDWORD bytes_of_transfered, LPOVERLAPPED * overlapped, int32_t * error_code = NULL, DWORD waiting_time = INFINITE);
					void create_thread_pool(void);
					void close_thread_pool(void);

				private:
					static unsigned __stdcall process(void * param);

				private:
					endpoint *			_endpoint;

					int32_t				_nthreads;
					HANDLE	*			_threads;
					HANDLE				_iocp;
				};
			};
		};
	};
};

#endif