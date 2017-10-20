#ifndef _X264_ENCODER_WRAPPER_H_
#define _X264_ENCODER_WRAPPER_H_

#include <sirius_png_compressor.h>

namespace sirius
{
	namespace library
	{
		namespace unified
		{
			class compressor;
			namespace png
			{
				class compressor
					: public sirius::library::video::transform::codec::png::compressor
				{
				public:
					compressor(sirius::library::unified::compressor * compressor);
					virtual ~compressor(void);
					virtual void after_process_callback(uint8_t * compressed, int32_t size, long long before_encode_timestamp, long long after_encode_timestamp);
				private:
					sirius::library::unified::compressor * _compressor;
				};
			};
		};
	};
};
#endif