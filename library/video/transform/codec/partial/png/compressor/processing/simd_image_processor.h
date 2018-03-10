#ifndef _SIMD_IMAGE_PROCESSOR_H_
#define _SIMD_IMAGE_PROCESSOR_H_

#include <cstdint>
#include <malloc.h>
#include <immintrin.h>

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
					namespace partial
					{
						namespace simd
						{
							template <class T> __forceinline char to_char(T value, size_t index)
							{
								return ((char*)&value)[index];
							}
#define FORCE_TO_CHAR(a) char(a)
#define FORCE_TO_2CHARS(a) \
	sirius::library::video::transform::codec::partial::simd::to_char(int16_t(a), 0), \
	sirius::library::video::transform::codec::partial::simd::to_char(int16_t(a), 1)

#define FORCE_TO_4CHARS(a) \
	sirius::library::video::transform::codec::partial::simd::to_char(int32_t(a), 0), \
	sirius::library::video::transform::codec::partial::simd::to_char(int32_t(a), 1), \
	sirius::library::video::transform::codec::partial::simd::to_char(int32_t(a), 2), \
	sirius::library::video::transform::codec::partial::simd::to_char(int32_t(a), 3)

#define FORCE_TO_8CHARS(a) \
	sirius::library::video::transform::codec::partial::simd::to_char(int64_t(a), 0), \
	sirius::library::video::transform::codec::partial::simd::to_char(int64_t(a), 1), \
	sirius::library::video::transform::codec::partial::simd::to_char(int64_t(a), 2), \
	sirius::library::video::transform::codec::partial::simd::to_char(int64_t(a), 3), \
	sirius::library::video::transform::codec::partial::simd::to_char(int64_t(a), 4), \
	sirius::library::video::transform::codec::partial::simd::to_char(int64_t(a), 5), \
	sirius::library::video::transform::codec::partial::simd::to_char(int64_t(a), 6), \
	sirius::library::video::transform::codec::partial::simd::to_char(int64_t(a), 7)

#define ASIGN_MM256_SET1_EPI8(a) \
	{FORCE_TO_CHAR(a), FORCE_TO_CHAR(a), FORCE_TO_CHAR(a), FORCE_TO_CHAR(a), \
	FORCE_TO_CHAR(a), FORCE_TO_CHAR(a), FORCE_TO_CHAR(a), FORCE_TO_CHAR(a), \
	FORCE_TO_CHAR(a), FORCE_TO_CHAR(a), FORCE_TO_CHAR(a), FORCE_TO_CHAR(a), \
	FORCE_TO_CHAR(a), FORCE_TO_CHAR(a), FORCE_TO_CHAR(a), FORCE_TO_CHAR(a), \
	FORCE_TO_CHAR(a), FORCE_TO_CHAR(a), FORCE_TO_CHAR(a), FORCE_TO_CHAR(a), \
	FORCE_TO_CHAR(a), FORCE_TO_CHAR(a), FORCE_TO_CHAR(a), FORCE_TO_CHAR(a), \
	FORCE_TO_CHAR(a), FORCE_TO_CHAR(a), FORCE_TO_CHAR(a), FORCE_TO_CHAR(a), \
	FORCE_TO_CHAR(a), FORCE_TO_CHAR(a), FORCE_TO_CHAR(a), FORCE_TO_CHAR(a)}

#define ASIGN_MM256_SET2_EPI8(a0, a1) \
	{FORCE_TO_CHAR(a0), FORCE_TO_CHAR(a1), FORCE_TO_CHAR(a0), FORCE_TO_CHAR(a1), \
	FORCE_TO_CHAR(a0), FORCE_TO_CHAR(a1), FORCE_TO_CHAR(a0), FORCE_TO_CHAR(a1), \
	FORCE_TO_CHAR(a0), FORCE_TO_CHAR(a1), FORCE_TO_CHAR(a0), FORCE_TO_CHAR(a1), \
	FORCE_TO_CHAR(a0), FORCE_TO_CHAR(a1), FORCE_TO_CHAR(a0), FORCE_TO_CHAR(a1), \
	FORCE_TO_CHAR(a0), FORCE_TO_CHAR(a1), FORCE_TO_CHAR(a0), FORCE_TO_CHAR(a1), \
	FORCE_TO_CHAR(a0), FORCE_TO_CHAR(a1), FORCE_TO_CHAR(a0), FORCE_TO_CHAR(a1), \
	FORCE_TO_CHAR(a0), FORCE_TO_CHAR(a1), FORCE_TO_CHAR(a0), FORCE_TO_CHAR(a1), \
	FORCE_TO_CHAR(a0), FORCE_TO_CHAR(a1), FORCE_TO_CHAR(a0), FORCE_TO_CHAR(a1)}

#define ASIGN_MM256_SETR_EPI8(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, aa, ab, ac, ad, ae, af, b0, b1, b2, b3, b4, b5, b6, b7, b8, b9, ba, bb, bc, bd, be, bf) \
    {FORCE_TO_CHAR(a0), FORCE_TO_CHAR(a1), FORCE_TO_CHAR(a2), FORCE_TO_CHAR(a3), \
    FORCE_TO_CHAR(a4), FORCE_TO_CHAR(a5), FORCE_TO_CHAR(a6), FORCE_TO_CHAR(a7), \
    FORCE_TO_CHAR(a8), FORCE_TO_CHAR(a9), FORCE_TO_CHAR(aa), FORCE_TO_CHAR(ab), \
    FORCE_TO_CHAR(ac), FORCE_TO_CHAR(ad), FORCE_TO_CHAR(ae), FORCE_TO_CHAR(af), \
    FORCE_TO_CHAR(b0), FORCE_TO_CHAR(b1), FORCE_TO_CHAR(b2), FORCE_TO_CHAR(b3), \
    FORCE_TO_CHAR(b4), FORCE_TO_CHAR(b5), FORCE_TO_CHAR(b6), FORCE_TO_CHAR(b7), \
    FORCE_TO_CHAR(b8), FORCE_TO_CHAR(b9), FORCE_TO_CHAR(ba), FORCE_TO_CHAR(bb), \
    FORCE_TO_CHAR(bc), FORCE_TO_CHAR(bd), FORCE_TO_CHAR(be), FORCE_TO_CHAR(bf)}

#define ASIGN_MM256_SET1_EPI16(a) \
	{FORCE_TO_2CHARS(a), FORCE_TO_2CHARS(a), FORCE_TO_2CHARS(a), FORCE_TO_2CHARS(a), \
	FORCE_TO_2CHARS(a), FORCE_TO_2CHARS(a), FORCE_TO_2CHARS(a), FORCE_TO_2CHARS(a), \
	FORCE_TO_2CHARS(a), FORCE_TO_2CHARS(a), FORCE_TO_2CHARS(a), FORCE_TO_2CHARS(a), \
	FORCE_TO_2CHARS(a), FORCE_TO_2CHARS(a), FORCE_TO_2CHARS(a), FORCE_TO_2CHARS(a)}

#define ASIGN_MM256_SET2_EPI16(a0, a1) \
	{FORCE_TO_2CHARS(a0), FORCE_TO_2CHARS(a1), FORCE_TO_2CHARS(a0), FORCE_TO_2CHARS(a1), \
	FORCE_TO_2CHARS(a0), FORCE_TO_2CHARS(a1), FORCE_TO_2CHARS(a0), FORCE_TO_2CHARS(a1), \
	FORCE_TO_2CHARS(a0), FORCE_TO_2CHARS(a1), FORCE_TO_2CHARS(a0), FORCE_TO_2CHARS(a1), \
	FORCE_TO_2CHARS(a0), FORCE_TO_2CHARS(a1), FORCE_TO_2CHARS(a0), FORCE_TO_2CHARS(a1)}

#define ASIGN_MM256_SETR_EPI16(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, aa, ab, ac, ad, ae, af) \
    {FORCE_TO_2CHARS(a0), FORCE_TO_2CHARS(a1), FORCE_TO_2CHARS(a2), FORCE_TO_2CHARS(a3), \
    FORCE_TO_2CHARS(a4), FORCE_TO_2CHARS(a5), FORCE_TO_2CHARS(a6), FORCE_TO_2CHARS(a7), \
    FORCE_TO_2CHARS(a8), FORCE_TO_2CHARS(a9), FORCE_TO_2CHARS(aa), FORCE_TO_2CHARS(ab), \
    FORCE_TO_2CHARS(ac), FORCE_TO_2CHARS(ad), FORCE_TO_2CHARS(ae), FORCE_TO_2CHARS(af)}

#define ASIGN_MM256_SET1_EPI32(a) \
	{FORCE_TO_4CHARS(a), FORCE_TO_4CHARS(a), FORCE_TO_4CHARS(a), FORCE_TO_4CHARS(a), \
	FORCE_TO_4CHARS(a), FORCE_TO_4CHARS(a), FORCE_TO_4CHARS(a), FORCE_TO_4CHARS(a)}

#define ASIGN_MM256_SET2_EPI32(a0, a1) \
	{FORCE_TO_4CHARS(a0), FORCE_TO_4CHARS(a1), FORCE_TO_4CHARS(a0), FORCE_TO_4CHARS(a1), \
	FORCE_TO_4CHARS(a0), FORCE_TO_4CHARS(a1), FORCE_TO_4CHARS(a0), FORCE_TO_4CHARS(a1)}

#define ASIGN_MM256_SETR_EPI32(a0, a1, a2, a3, a4, a5, a6, a7) \
    {FORCE_TO_4CHARS(a0), FORCE_TO_4CHARS(a1), FORCE_TO_4CHARS(a2), FORCE_TO_4CHARS(a3), \
    FORCE_TO_4CHARS(a4), FORCE_TO_4CHARS(a5), FORCE_TO_4CHARS(a6), FORCE_TO_4CHARS(a7)}

#define ASIGN_MM256_SET1_EPI64(a) \
	{FORCE_TO_8CHARS(a), FORCE_TO_8CHARS(a), FORCE_TO_8CHARS(a), FORCE_TO_8CHARS(a)}

#define ASIGN_MM256_SET2_EPI64(a0, a1) \
	{FORCE_TO_8CHARS(a0), FORCE_TO_8CHARS(a1), FORCE_TO_8CHARS(a0), FORCE_TO_8CHARS(a1)}

#define ASIGN_MM256_SETR_EPI64(a0, a1, a2, a3) \
    {FORCE_TO_8CHARS(a0), FORCE_TO_8CHARS(a1), FORCE_TO_8CHARS(a2), FORCE_TO_8CHARS(a3)}

							const int32_t	SIMD_ALIGN	= 32;
							const size_t	A			= sizeof(__m256i);
							const size_t	DA			= 2 * A;
							const size_t	QA			= 4 * A;
							const size_t	OA			= 8 * A;
							const size_t	HA			= A / 2;

							const __m256i	K_ZERO						= ASIGN_MM256_SET1_EPI8(0);
							const __m256i	K16_00FF					= ASIGN_MM256_SET1_EPI16(0x00FF);
							const int32_t	BGR_TO_GRAY_AVERAGING_SHIFT = 14;
							const int32_t	BGR_TO_GRAY_ROUND_TERM		= 1 << (BGR_TO_GRAY_AVERAGING_SHIFT - 1);
							const int32_t	BLUE_TO_GRAY_WEIGHT			= int32_t(0.114*(1 << BGR_TO_GRAY_AVERAGING_SHIFT) + 0.5);
							const int32_t	GREEN_TO_GRAY_WEIGHT		= int32_t(0.587*(1 << BGR_TO_GRAY_AVERAGING_SHIFT) + 0.5);
							const int32_t	RED_TO_GRAY_WEIGHT			= int32_t(0.299*(1 << BGR_TO_GRAY_AVERAGING_SHIFT) + 0.5);

							const __m256i	K16_BLUE_RED				= ASIGN_MM256_SET2_EPI16(BLUE_TO_GRAY_WEIGHT, RED_TO_GRAY_WEIGHT);
							const __m256i	K16_GREEN_0000				= ASIGN_MM256_SET2_EPI16(GREEN_TO_GRAY_WEIGHT, 0x0000);
							const __m256i	K32_ROUND_TERM				= ASIGN_MM256_SET1_EPI32(BGR_TO_GRAY_ROUND_TERM);

							const int32_t	LINEAR_SHIFT				= 4;
							const int32_t	LINEAR_ROUND_TERM			= 1 << (LINEAR_SHIFT - 1);
							const int32_t	BILINEAR_SHIFT				= LINEAR_SHIFT * 2;
							const int32_t	BILINEAR_ROUND_TERM			= 1 << (BILINEAR_SHIFT - 1);
							const int32_t	FRACTION_RANGE				= 1 << LINEAR_SHIFT;
							const double	FRACTION_ROUND_TERM			= 0.5 / FRACTION_RANGE;

							const __m256i	K8_SHUFFLE_X4				= ASIGN_MM256_SETR_EPI8(0x0, 0x4, 0x1, 0x5, 0x2, 0x6, 0x3, 0x7, 0x8, 0xC, 0x9, 0xD, 0xA, 0xE, 0xB, 0xF, 0x0, 0x4, 0x1, 0x5, 0x2, 0x6, 0x3, 0x7, 0x8, 0xC, 0x9, 0xD, 0xA, 0xE, 0xB, 0xF);
							const __m256i	K8_01_FF					= ASIGN_MM256_SET2_EPI8(0x01, 0xFF);

							template <class T> __forceinline void swap(T & a, T & b);

							__forceinline size_t	align_h(size_t size, size_t align);
							__forceinline void *	align_h(const void * ptr, size_t align);
							__forceinline size_t	align_l(size_t size, size_t align);
							__forceinline void *	align_l(const void * ptr, size_t align);
							__forceinline bool		aligned(size_t size, size_t align);
							__forceinline bool		aligned(const void * ptr, size_t align);

							void estimate_alpha_index(size_t ssize, size_t dsize, int32_t * indexes, int32_t * alphas, size_t channels);

							namespace avx2
							{
								__forceinline __m256i	load(bool aligned, const __m256i * p);
								__forceinline void		store(bool aligned, __m256i * p, __m256i a);

								__forceinline __m256i	pack_u16_to_u8(__m256i lo, __m256i hi);
								__forceinline __m256i	pack_i32_to_i16(__m256i lo, __m256i hi);

								__forceinline __m256i	set_mask(bool aligned, uint8_t first, size_t position, uint8_t second);
								__forceinline uint64_t	extract(bool aligned, __m256i value);

								__forceinline __m256i	unpack_u8(int32_t part, __m256i a, __m256i b);
								__forceinline __m256i	sub_unpacked_u8(int32_t part, __m256i a, __m256i b);

								__forceinline __m256i	horizontal_sum32(__m256i a);

								namespace resizer
								{
									static const int32_t CHANNELS = 4;
									typedef struct _buffer_t
									{
										uint8_t * bx[2];
										uint8_t * ax;
										int * ix;
										int * ay;
										int * iy;
										_buffer_t(size_t size, size_t width, size_t height)
										{
											_p = _aligned_malloc(3 * size + sizeof(int)*(2 * height + width), SIMD_ALIGN);
											bx[0] = (uint8_t*)_p;
											bx[1] = bx[0] + size;
											ax = bx[1] + size;
											ix = (int*)(ax + size);
											iy = ix + width;
											ay = iy + height;
										}

										~_buffer_t()
										{
											_aligned_free(_p);
										}

									private:
										void *_p;
									} buffer_t;

									typedef struct _index_t
									{
										int32_t src;
										int32_t dst;
										uint8_t shuffle[A];
									} index_t;

									typedef struct _buffer_g_t
									{
										_buffer_g_t(size_t width, size_t blocks, size_t height)
										{
											_p = _aligned_malloc(3 * width + sizeof(int) * 2 * height + blocks * sizeof(index_t) + 2 * A, SIMD_ALIGN);
											bx[0] = (uint8_t*)_p;
											bx[1] = bx[0] + width + A;
											ax = bx[1] + width + A;
											ix = (index_t*)(ax + width);
											iy = (int*)(ix + blocks);
											ay = iy + height;
										}

										~_buffer_g_t(void)
										{
											_aligned_free(_p);
										}

										uint8_t * bx[2];
										uint8_t * ax;
										index_t * ix;
										int32_t * ay;
										int32_t * iy;
									private:
										void *_p;
									} buffer_g_t;

									void					resize_bilinear(const uint8_t * src, size_t swidth, size_t sheight, size_t sstride, uint8_t *dst, size_t dwidth, size_t dheight, size_t dstride);
									__forceinline void		estimate_alpha_index_x(size_t src_size, size_t dst_size, int32_t * indexes, uint8_t * alphas);
									__forceinline void		interpolate_x(const __m256i * alpha, __m256i * buffer);
									__forceinline void		interpolate_inner_x(const __m256i * alpha, __m256i * buffer);
									__forceinline void		interpolate_y(bool aligned, const uint8_t * bx0, const uint8_t * bx1, __m256i alpha[2], uint8_t * dst);
									__forceinline __m256i	interpolate_y(bool aligned, const __m256i * pbx0, const __m256i * pbx1, __m256i alpha[2]);
								};

								namespace converter
								{
									/*
									__forceinline __m256i	avx2_bgra2gray32(__m256i bgra);
									__forceinline __m256i	avx2_bgra2gray(__m256i bgra[4]);
									//__forceinline void		avx2_bgra_load(bool aligned, const uint8_t * p, __m256i a[4]);
									void					avx2_bgra2gray(bool align, const uint8_t * bgra, size_t width, size_t height, size_t bgra_stride, uint8_t * gray, size_t gray_stride);

									__forceinline __m256i	avx2_set_mask(bool aligned, uint8_t first, size_t position, uint8_t second);
									__forceinline uint64_t	avx2_extract(bool aligned, __m256i value);
									bool					avx2_is_different(bool aligned, const uint8_t * a, size_t a_stride, const uint8_t * b, size_t b_stride, size_t width, size_t height);
									*/
								};

								namespace evaluator
								{
									bool		abs_eval(bool aligned, const uint8_t * a, size_t a_stride, const uint8_t * b, size_t b_stride, size_t width, size_t height);
									bool		squared_eval(bool aligned, const uint8_t * a, size_t a_stride, const uint8_t * b, size_t b_stride, size_t width, size_t height);

									__forceinline __m256i squared_difference(__m256i a, __m256i b);
								};

								namespace connected_component
								{
									bool		find(const uint8_t * img, int16_t width, int16_t height);
								};
							};
						};
					};
				};
			};
		};
	};
};

#endif
