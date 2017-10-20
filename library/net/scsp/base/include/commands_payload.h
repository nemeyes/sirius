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
#pragma pack(1)
				typedef struct _cmd_frame_t
				{
					int32_t		index;
					int32_t		length;
					long long	timestamp;
				} cmd_frame_t;

				typedef struct _cmd_stream_data_t
				{
					uint8_t		count;
					cmd_frame_t data;
				} cmd_stream_data_t;

				typedef struct _stream_packet_t
				{
					cmd_stream_data_t		stream_data;
				} stream_packet_t;
#pragma pack()
			};
		};
	};
};

#endif
