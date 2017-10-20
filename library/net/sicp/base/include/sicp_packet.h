#ifndef _SIRIUS_SICP_PACKET_H_
#define _SIRIUS_SICP_PACKET_H_

#include <platform.h>

#define FLAG_PKT_BEGIN          0x0001
#define FLAG_PKT_PLAY           0x0002
#define FLAG_PKT_END            0x0004
#define FLAG_PKT_SEND           0x0008

namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace sicp
			{
#if defined(WITH_NEW_PROTOCOL)
				typedef struct _packet_header_t
				{
					char  pid;
					char  dst[16];
					char  src[16];
					uint8_t  version;
					uint16_t command;
					uint32_t length;
				} packet_header_t;
#else
				typedef struct _packet_header_t
				{
					uint16_t command;
					uint8_t  version;
					uint8_t  type;		// 0:binary, 1:json.
					uint32_t length;	// header 를 포함한 총 길이.
					char  src[16];
					char  dst[16];
				} packet_header_t;
#endif
			};
		};
	};
};

#endif
