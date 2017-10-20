#ifndef _SIRIUS_SIMD_COLORSPACE_CONVERTER_H_
#define _SIRIUS_SIMD_COLORSPACE_CONVERTER_H_

#include <sirius_video_colorspace_converter.h>

#if defined(EXPORT_SIMD_COLORSPACE_CONVERTER_LIB)
#define EXP_SIMD_COLORSPACE_CONVERTER_CLASS __declspec(dllexport)
#else
#define EXP_SIMD_COLORSPACE_CONVERTER_CLASS __declspec(dllimport)
#endif

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
						class EXP_SIMD_COLORSPACE_CONVERTER_CLASS converter
							: public sirius::library::video::transform::colorspace::converter
						{
						public:
							class core;
						public:
							typedef struct EXP_SIMD_COLORSPACE_CONVERTER_CLASS _context_t
							{
								int32_t iformat; //intput colorspace
								int32_t oformat; //output colorspace
								_context_t(void);
								_context_t(const sirius::library::video::transform::colorspace::simd::converter::_context_t & clone);
								_context_t & operator=(const _context_t & clone);
							} context_t;

						public:
							converter(void);
							virtual ~converter(void);

							virtual int32_t initialize(void * context);
							virtual int32_t release(void);
							virtual int32_t convert(sirius::library::video::transform::colorspace::simd::converter::entity_t * input, sirius::library::video::transform::colorspace::simd::converter::entity_t * output);

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
							sirius::library::video::transform::colorspace::simd::converter::core * _core;
							/*
							typedef enum _color_space
							{
								color_space_yv12,
								color_space_nv12,
								color_space_rgb24,
								color_space_rgb32
							} color_space;

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
							converter(void);
							converter(const converter & clone);
							virtual ~converter(void);

						private:
							static uint8_t _buffer[8294400]; //1920*1080*4
							*/
						};
					};
				};
			};
		};
	};
};


#endif