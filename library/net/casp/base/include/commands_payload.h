#ifndef _COMMANDS_PAYLOAD_H_
#define _COMMANDS_PAYLOAD_H_

#include <cstdint>
#include "commands_payload.h"

#if defined(WITH_BRANDNEW_IPC)
namespace amadeus
{
	namespace library
	{
		namespace net
		{
			namespace casp
			{
				typedef struct _cmd_audio_stream_noti_t
				{
					uint8_t extradata[50];
					int32_t extradata_size;
				} cmd_audio_stream_noti_t;

#pragma pack(1)
				typedef struct _cmd_frame_t
				{
					int32_t		index;
					long long	timestamp;
					int32_t		length;
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

#else
namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace casp
			{
#pragma pack(1)
				typedef struct _cmd_frame_t
				{
					int32_t		index;
					long long	timestamp;
					int32_t		length;
				} cmd_frame_t;

				typedef struct _cmd_stream_data_t
				{
					uint8_t		count;
					cmd_frame_t data;
				} cmd_stream_data_t;

#if defined(STREAMING_BLOCKING_MODE)
				typedef struct _casp_packet_header_t
				{
					uint16_t command;
					uint8_t  version;     // .
					uint8_t  type; // 0:binary, 1:json.
					uint32_t length;      // header 를 포함한 총 길이.
					char  src[16];
					char  dst[16];
				} casp_packet_header_t;
#endif

				typedef struct _stream_packet_t
				{
#if defined(STREAMING_BLOCKING_MODE)
					casp_packet_header_t	header;
#endif
					cmd_stream_data_t		stream_data;
				} stream_packet_t;

#pragma pack()
			};
		};
	};
};
#endif
#endif
