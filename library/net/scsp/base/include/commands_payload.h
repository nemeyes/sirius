#ifndef _COMMANDS_PAYLOAD_H_
#define _COMMANDS_PAYLOAD_H_

#include <cstdint>
#include <sirius_commands.h>

namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace scsp
			{
				typedef struct _header_t
				{
					int32_t		index;
					int32_t		length;
				} header_t;
			};
		};
	};
};

#endif
