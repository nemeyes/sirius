#ifndef _EPOLL_HANDLER_H_
#define _EPOLL_HANDLER_H_

#include <platform.h>

namespace amadeus
{
	namespace library
	{
		namespace net
		{
            namespace epoll
            {
#if defined(WITH_WORKING_AS_SERVER)
#define         endpoint server
#else
#define         endpoint client
#endif
                class endpoint;
                class handler
                {
                public:
                    handler(amadeus::library::net::epoll::endpoint * endpoint);
                    virtual ~handler(void);

                    bool create(int32_t number_of_pooled_threads = 0, int32_t * error_code = NULL);
                    bool associate(SOCKET socket, ULONG_PTR key, int32_t * error_code = NULL);
                    bool associate(HANDLE handle, ULONG_PTR key, int32_t * error_code = NULL);
                    bool post_completion_status(ULONG_PTR key, DWORD bytes_of_transfered = 0, OVERLAPPED * overlapped = NULL, int32_t * error_code = NULL);
                    bool get_completion_status(ULONG_PTR * key, LPDWORD bytes_of_transfered, LPOVERLAPPED * overlapped, int32_t * error_code = NULL, DWORD waiting_time = INFINITE);
                    void create_thread_pool(void);
                    void close_thread_pool(void);

                private:
                    static unsigned __stdcall process(void * param);

                private:
                    CRITICAL_SECTION	_cs;
                    endpoint *          _endpoint;

                    int32_t				_number_of_threads;
                    std::vector<HANDLE>	_threads;
                    HANDLE				_iocp_handle;
                };
            };
		};
	};
};

#endif
