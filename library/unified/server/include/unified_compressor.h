#ifndef _UNIFIED_ENCODER_H_
#define _UNIFIED_ENCODER_H_

#include "sirius_unified_server.h"

namespace sirius
{
	namespace library
	{
		namespace unified
		{
			namespace partialpng
			{
				class compressor;
			};

			class compressor
			{
			public:
				friend class sirius::library::unified::partialpng::compressor;

			public:
				compressor(sirius::library::unified::server::core * front);
				~compressor(void);

				int32_t		initialize_video_compressor(sirius::library::unified::server::video_compressor_context_t * context);
				int32_t		release_video_compressor(void);
				int32_t		compress(sirius::library::video::transform::codec::compressor::entity_t * input);

				int32_t		play(int32_t flag);
				int32_t		pause(int32_t flag);
				int32_t		stop(int32_t flag);
				int32_t		invalidate(void);

			private:
				void after_video_compressing_callback(uint8_t * data, size_t size, long long before_encode_timestamp, long long after_encode_timestamp);
				void after_video_compressing_callback(int32_t count, int32_t * index, uint8_t ** compressed, int32_t * size, long long before_compress_timestamp, long long after_compress_timestamp);

			private:
				sirius::library::unified::server::core * _front;

				sirius::library::unified::server::video_compressor_context_t * _external_venc_ctx;
				sirius::library::video::transform::codec::compressor * _venc;
				sirius::library::video::transform::codec::compressor::context_t * _venc_ctx;

				SRWLOCK		_venc_lock;
				int32_t		_play_flag;
			};
		};
	};
};

#endif