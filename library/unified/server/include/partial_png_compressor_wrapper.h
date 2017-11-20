#ifndef _SIRIUS_PARTIAL_PNG_COMPRESSOR_WRAPPER_H_
#define _SIRIUS_PARTIAL_PNG_COMPRESSOR_WRAPPER_H_

#include <sirius_partial_png_compressor.h>

namespace sirius
{
	namespace library
	{
		namespace unified
		{
			class compressor;
			namespace partialpng
			{
				class compressor
					: public sirius::library::video::transform::codec::partial::png::compressor
				{
				public:
					compressor(sirius::library::unified::compressor * compressor);
					virtual ~compressor(void);
					virtual void after_process_callback(uint8_t * compressed, int32_t size, long long before_encode_timestamp, long long after_encode_timestamp);
					virtual void after_process_callback(int32_t count, int32_t * index, uint8_t ** compressed, int32_t * size, long long before_compress_timestamp, long long after_compress_timestamp);

				private:
					sirius::library::unified::compressor * _compressor;
				};
			};
		};
	};
};
#endif