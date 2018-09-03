#ifndef _LIBWEBP_DECOMPRESSOR_V1_0_0_H_
#define _LIBWEBP_DECOMPRESSOR_V1_0_0_H_

#include "elastic_libwebp_decompressor_v1.0.0.h"
#include <decode.h>
#include <elastics_circular_buffer.h>

namespace elastics
{
	namespace lib
	{
		namespace video
		{
			namespace transform
			{
				namespace codec
				{
					namespace libwebp
					{
						namespace v1_0_0
						{
							class decompressor::core
							{
							public:
								static const int32_t SIGNATURE_LENGTH = 8;
								static const int32_t MAX_VIDEO_SIZE = 1920 * 1080 * 4;

								core(void);
								~core(void);

								int32_t initialize(elastics::lib::video::transform::codec::libwebp::v1_0_0::decompressor::context_t * context);
								int32_t release(void);

								int32_t decompress(elastics::lib::video::transform::codec::libwebp::v1_0_0::decompressor::entity_t * input, elastics::lib::video::transform::codec::libwebp::v1_0_0::decompressor::entity_t * output);


							private:
								uint8_t _recvd[MAX_VIDEO_SIZE];
								int32_t _recvd_size;
								int32_t _recvd_index;

								CRITICAL_SECTION _cs;
								bool	_decoding;
							};
						};
					};
				};
			};
		};
	};
};


#endif
