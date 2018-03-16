#ifndef _NATIVE_CLIENT_PLAYER_FRAMEWORK_H_
#define _NATIVE_CLIENT_PLAYER_FRAMEWORK_H_

#include <cstdint>
#include <sirius_unified_client.h>

#include <sirius_video_decompressor.h>
#include <sirius_video_renderer.h>

#include "sirius_native_client_framework.h"

namespace sirius
{
	namespace library
	{
		namespace framework
		{
			namespace client
			{
				class native::core
					: public sirius::library::unified::client
				{
				public:
					static const int32_t VIDEO_BUFFER_SIZE = 3840 * 2160 * 4;
				public:
					core(void);
					virtual ~core(void);

					int32_t play(HWND hwnd);
					int32_t stop(void);

					int32_t change_debug_level(int32_t level);

					virtual void on_begin_video(int32_t codec, int32_t width, int32_t height, int32_t block_width, int32_t block_height);
					virtual void on_recv_video(int32_t codec, const uint8_t * data, int32_t length, long long dts, long long cts);
					virtual void on_recv_video(int32_t codec, int32_t count, int32_t * index, uint8_t ** data, int32_t * length, long long dts, long long cts);
					virtual void on_recv_video(int32_t codec, int32_t count, int16_t * x, int16_t * y, int16_t * width, int16_t * height, uint8_t ** data, int32_t * length, long long dts, long long cts);
					virtual void on_end_video(void);

				private:
					HWND _hwnd;
					sirius::library::video::transform::codec::decompressor * _video_decompressor;
					void * _video_decompressor_context;

					sirius::library::video::sink::renderer * _video_renderer;
					void * _video_renderer_context;

					CRITICAL_SECTION			_vcs;
					uint8_t *					_decoder_buffer;
					uint8_t *					_render_buffer;
					uint8_t *					_processing_buffer;

					int32_t						_debug_level;
				};
			};
		};
	};
};


#endif