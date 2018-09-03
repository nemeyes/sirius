#ifndef _LIBWEBP_DECOMPRESSOR_H_
#define _LIBWEBP_DECOMPRESSOR_H_

#include "sirius_webp_decompressor.h"
#include <webp/decode.h>
#include <sirius_circular_buffer.h>

namespace sirius
{
	namespace library
	{
		namespace video
		{
			namespace transform
			{
				namespace codec
				{
					namespace webp
					{
						class decompressor::core
						{
						public:
							static const int32_t SIGNATURE_LENGTH = 8;
							static const int32_t MAX_VIDEO_SIZE = 1920 * 1080 * 4;

							core(void);
							~core(void);

							int32_t initialize(sirius::library::video::transform::codec::webp::decompressor::context_t * context);
							int32_t release(void);

							int32_t decompress(sirius::library::video::transform::codec::webp::decompressor::entity_t * input, sirius::library::video::transform::codec::webp::decompressor::entity_t * output);


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


#endif
