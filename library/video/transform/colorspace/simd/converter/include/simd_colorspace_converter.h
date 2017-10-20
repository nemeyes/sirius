#ifndef _SIMD_COLORSPACE_CONVERTER_H_
#define _SIMD_COLORSPACE_CONVERTER_H_

#include "sirius_simd_colorspace_converter.h"

namespace sirius
{
	namespace library
	{
		namespace video
		{
			namespace transform
			{
				namespace colorspace
				{
					namespace simd
					{
						class converter::core
						{
						public:
							core(void);
							virtual ~core(void);

							int32_t initialize(sirius::library::video::transform::colorspace::simd::converter::context_t * context);
							int32_t release(void);
							int32_t convert(sirius::library::video::transform::colorspace::simd::converter::entity_t * input, sirius::library::video::transform::colorspace::simd::converter::entity_t * output);

							static void flip(int32_t width, int32_t height, int32_t stride, uint8_t * pixels);
							static void convert_rgba_to_rgba(int32_t width, int32_t height, uint8_t * src, int32_t src_stride, uint8_t * dst, int32_t dst_stride, bool flip = false);
							static void convert_rgba_to_yv12(int32_t width, int32_t height, uint8_t * src, int32_t src_stride, uint8_t * dst, int32_t dst_stride, bool flip = false);

							static void convert_i420_to_rgba(int32_t width, int32_t height, uint8_t * y, int32_t y_stride, uint8_t * u, int32_t u_stride, uint8_t * v, int32_t v_stride,
								uint8_t * dst, int32_t dst_stride, uint8_t alpha, bool flip = false);
							static void convert_i420_to_rgb(int32_t width, int32_t height, uint8_t * y, int32_t y_stride, uint8_t * u, int32_t u_stride, uint8_t * v, int32_t v_stride,
								uint8_t * dst, int32_t dst_stride, bool flip = false);
							static void convert_i420_to_rgba(int32_t width, int32_t height, uint8_t * src, int32_t src_stride, uint8_t * dst, int32_t dst_stride, uint8_t alpha, bool flip = false);
							static void convert_i420_to_rgb(int32_t width, int32_t height, uint8_t * src, int32_t src_stride, uint8_t * dst, int32_t dst_stride, bool flip = false);

							static void convert_yv12_to_rgba(int32_t width, int32_t height, uint8_t * src, int32_t src_stride, uint8_t * dst, int32_t dst_stride, uint8_t alpha, bool flip = false);
							static void convert_yv12_to_rgb(int32_t width, int32_t height, uint8_t * src, int32_t src_stride, uint8_t * dst, int32_t dst_stride, bool flip = false);

						private:
							sirius::library::video::transform::colorspace::simd::converter::context_t * _context;
							static uint8_t _buffer[8294400]; //1920*1080*4
						};
					};
				};
			};
		};
	};
};

#endif
