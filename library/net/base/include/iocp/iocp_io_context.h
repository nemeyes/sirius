#ifndef _IOCP_IO_CONTEXT_H_
#define _IOCP_IO_CONTEXT_H_

#include <platform.h>

#define MTU						1460
#define MAX_RECV_BUFFER_SIZE	MTU	//1024*512
#define SEND_RETRY_COUNT		1000
#define SEND_SLEEP_TIME			1

namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace iocp
			{
				typedef struct _io_context_t
				{
					typedef struct _operation_t
					{
						static const int32_t recv = 0;
						static const int32_t send = 1;
					} operation_t;

					WSAOVERLAPPED	overlapped;
					WSABUF			wsabuf;
					int32_t			operation;
					_io_context_t(int32_t op)
						: operation(op)
					{
						memset(&overlapped, 0x00, sizeof(WSAOVERLAPPED));
						memset(&wsabuf, 0x00, sizeof(WSABUF));
					}
				} io_context_t;
			};
		};
	};
};

#endif