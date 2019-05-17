#ifndef _LOCALCACHE_PACKET_H_
#define _LOCALCACHE_PACKET_H_

#include <platform.h>

#define FLAG_PKT_BEGIN          0x0001
#define FLAG_PKT_PLAY           0x0002
#define FLAG_PKT_END            0x0004
#define FLAG_PKT_SEND           0x0008

namespace sirius
{
	namespace library
	{
		namespace cache
		{
			namespace local
			{
				typedef struct _packet_header_t
				{
					int16_t		command;
					int32_t		length;
				} packet_header_t;
			};
		};
	};
};

#endif
