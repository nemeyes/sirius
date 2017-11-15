#ifndef _SCSP_CLIENT_H_
#define _SCSP_CLIENT_H_

#include <memory>
#include <time.h>

#include <commands_payload.h>
#include "sirius_sicp_client.h"
#include "sirius_scsp_client.h"

namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace scsp
			{
				class client::core
					: public sirius::library::net::sicp::client
				{
					static const int32_t IO_THREAD_POOL_COUNT = 1;
					static const int32_t COMMAND_THREAD_POOL_COUNT = 1;

					static const int32_t RECV_BUF_SIZE = 1024 * 1024 * 2;
					static const int32_t SEND_BUF_SIZE = 1024 * 1024 * 2;

					static const int32_t MTU_SIZE = 0;

					static const int32_t VIDEO_DATA_SIZE = 1024 * 1024 * 2;
					//static const int32_t MTU_SIZE = 1500;
				public:
					core(sirius::library::net::scsp::client * front);
					virtual ~core(void);

					int32_t play(const char * url, int32_t port, int32_t recv_option, bool repeat);
					int32_t stop(void);

					void request_play(int32_t recv_option);
					void request_play_video(void);

					//callback
					void create_session_callback(void);
					void destroy_session_callback(void);

					void av_stream_callback(const char * msg, int32_t length);

					void push_video_packet(int32_t count, uint8_t * data, int32_t length);

				private:
					sirius::library::net::scsp::client * _front;
					int32_t _video_codec;
					int32_t _video_width;
					int32_t _video_height;
					int32_t _video_fps;
					int32_t _video_block_width;
					int32_t _video_block_height;
					int32_t _state;
					CRITICAL_SECTION _state_cs;
					int32_t _receive_option;
					bool _play;
					bool _rcv_first_video;

					clock_t _s_video_interval;
					clock_t _e_video_interval;

					uint8_t * _video_frame_data;// [VIDEO_DATA_SIZE];
					size_t _video_frame_size;
					int64_t _video_frame_timestamp;
				};
			};
		};
	};
};

#endif