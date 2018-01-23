#ifndef _NATIVE_STRESSOR_PLAYER_FRAMEWORK_H_
#define _NATIVE_STRESSOR_PLAYER_FRAMEWORK_H_

#include <cstdint>
#include <sirius_unified_client.h>

#include "sirius_native_stressor_framework.h"

namespace sirius
{
	namespace library
	{
		namespace framework
		{
			namespace stressor
			{
				class native::core
					: public sirius::library::unified::client
				{
				public:
					core(sirius::library::framework::stressor::native * front);
					virtual ~core(void);

					int32_t play(HWND hwnd);
					int32_t stop(void);

					virtual void on_begin_video(int32_t codec, int32_t width, int32_t height, int32_t block_width, int32_t block_height);
					virtual void on_recv_video(int32_t codec, const uint8_t * data, int32_t length, long long dts, long long cts);
					virtual void on_recv_video(int32_t codec, int32_t count, int32_t * index, uint8_t ** data, int32_t * length, long long dts, long long cts);
					virtual void on_end_video(void);
				
				private:			
					HWND				_hwnd;			
					CRITICAL_SECTION	_vcs;
					sirius::library::framework::stressor::native * _front;
					int32_t				_stream_count;
				};
			};
		};
	};
};


#endif