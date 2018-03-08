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
				typedef struct _index_header_t
				{
					int32_t		index;
					int32_t		length;
				} index_header_t;

				typedef struct _coordinates_header_t
				{
					int16_t		x;
					int16_t		y;
					int16_t		width;
					int16_t		height;
					int32_t		length;
				} coordinates_header_t;
			};
		};
	};
};

#endif
