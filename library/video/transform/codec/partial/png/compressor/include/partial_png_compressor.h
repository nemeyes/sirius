#ifndef _PARTIAL_PNG_COMPRESSOR_H_
#define _PARTIAL_PNG_COMPRESSOR_H_

#include "sirius_partial_png_compressor.h"

#include <atlbase.h>
#include <d3d11.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <setjmp.h>
#include <map>

#include <sirius_template_queue.h>
#include <sirius_video_processor.h>

#include "libpng_compressor.h"

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
						namespace png
						{
#if defined(WITH_FULLSCAN)
#if defined(WITH_AVX2_SIMD)
							template <class T> __forceinline char to_char(T value, size_t index)
							{
								return ((char*)&value)[index];
							}
#define FORCE_TO_CHAR(a) char(a)
#define FORCE_TO_2CHARS(a) \
	sirius::library::video::transform::codec::partial::png::to_char(int16_t(a), 0), \
	sirius::library::video::transform::codec::partial::png::to_char(int16_t(a), 1)

#define FORCE_TO_4CHARS(a) \
	sirius::library::video::transform::codec::partial::png::to_char(int32_t(a), 0), \
	sirius::library::video::transform::codec::partial::png::to_char(int32_t(a), 1), \
	sirius::library::video::transform::codec::partial::png::to_char(int32_t(a), 2), \
	sirius::library::video::transform::codec::partial::png::to_char(int32_t(a), 3)

#define FORCE_TO_8CHARS(a) \
	sirius::library::video::transform::codec::partial::png::to_char(int64_t(a), 0), \
	sirius::library::video::transform::codec::partial::png::to_char(int64_t(a), 1), \
	sirius::library::video::transform::codec::partial::png::to_char(int64_t(a), 2), \
	sirius::library::video::transform::codec::partial::png::to_char(int64_t(a), 3), \
	sirius::library::video::transform::codec::partial::png::to_char(int64_t(a), 4), \
	sirius::library::video::transform::codec::partial::png::to_char(int64_t(a), 5), \
	sirius::library::video::transform::codec::partial::png::to_char(int64_t(a), 6), \
	sirius::library::video::transform::codec::partial::png::to_char(int64_t(a), 7)

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
#endif
#endif

							class compressor::core
								: public sirius::library::video::transform::codec::processor
							{
							public:
								static const int32_t	MAX_IO_BUFFERS = 60;
								static const int32_t	MAX_PNG_SIZE = 1024 * 1024 * 1;
#if defined(WITH_FULLSCAN)
#if defined(WITH_AVX2_SIMD)
								const __m256i	K16_00FF = ASIGN_MM256_SET1_EPI16(0x00FF);
								const int32_t	BGR_TO_GRAY_AVERAGING_SHIFT = 14;
								const int32_t	BGR_TO_GRAY_ROUND_TERM = 1 << (BGR_TO_GRAY_AVERAGING_SHIFT - 1);
								const int32_t	BLUE_TO_GRAY_WEIGHT = int32_t(0.114*(1 << BGR_TO_GRAY_AVERAGING_SHIFT) + 0.5);
								const int32_t	GREEN_TO_GRAY_WEIGHT = int32_t(0.587*(1 << BGR_TO_GRAY_AVERAGING_SHIFT) + 0.5);
								const int32_t	RED_TO_GRAY_WEIGHT = int32_t(0.299*(1 << BGR_TO_GRAY_AVERAGING_SHIFT) + 0.5);

								const __m256i	K16_BLUE_RED = ASIGN_MM256_SET2_EPI16(BLUE_TO_GRAY_WEIGHT, RED_TO_GRAY_WEIGHT);
								const __m256i	K16_GREEN_0000 = ASIGN_MM256_SET2_EPI16(GREEN_TO_GRAY_WEIGHT, 0x0000);
								const __m256i	K32_ROUND_TERM = ASIGN_MM256_SET1_EPI32(BGR_TO_GRAY_ROUND_TERM);
								static const size_t	AVX2_ALIGN_SIZE = sizeof(__m256i);
#endif
#endif
								typedef struct _ibuffer_t
								{
									void *		data;
									int32_t		data_size;
									int32_t		data_capacity;
									int32_t		x;
									int32_t		y;
									int32_t		width;
									int32_t		height;
									long long	timestamp;
									_ibuffer_t(void)
										: data(nullptr)
										, timestamp(0)
									{}
								} ibuffer_t;

								typedef struct _compressed_cache_image_t
								{
									uint8_t	*	cache;
									int32_t		cache_size;
									int32_t		cache_capacity;
									_compressed_cache_image_t(void)
										: cache_capacity(1024 * 512)
									{
										cache = static_cast<uint8_t*>(malloc(cache_capacity));
									}

									~_compressed_cache_image_t(void)
									{
										if (cache)
											free(cache);
										cache = nullptr;
									}
								} compressed_cache_image_t;


								typedef struct _buffer_t
								{
									sirius::library::video::transform::codec::partial::png::compressor::core::ibuffer_t	input;
								} buffer_t;

							public:
								core(sirius::library::video::transform::codec::partial::png::compressor * front);
								~core(void);

								int32_t state(void);

								int32_t initialize(sirius::library::video::transform::codec::partial::png::compressor::context_t * context);
								int32_t release(void);

								int32_t play(void);
								int32_t pause(void);
								int32_t stop(void);
								int32_t invalidate(void);

								int32_t compress(sirius::library::video::transform::codec::partial::png::compressor::entity_t * input, sirius::library::video::transform::codec::partial::png::compressor::entity_t * bitstream);
								int32_t compress(sirius::library::video::transform::codec::partial::png::compressor::entity_t * input);

							private:
								static unsigned __stdcall process_callback(void * param);
								void	process(void);
								int32_t allocate_io_buffers(void);
								int32_t release_io_buffers(void);


							private:
#if defined(WITH_FULLSCAN)
#if defined(WITH_AVX2_SIMD)
								__forceinline size_t	avx2_aligned_high(size_t size, size_t align);
								__forceinline void *	avx2_align_high(const void * ptr, size_t align);
								__forceinline size_t	avx2_align_low(size_t size, size_t align);
								__forceinline void *	avx2_align_low(const void * ptr, size_t align);
								__forceinline bool		avx2_is_aligned(size_t size, size_t align);
								__forceinline bool		avx2_is_aligned(const void * ptr, size_t align);
								__forceinline __m256i	avx2_load(bool aligned, const __m256i * p);
								__forceinline void		avx2_store(bool aligned, __m256i * p, __m256i a);
								__forceinline __m256i	avx2_pack_u16tou8(__m256i lo, __m256i hi);
								__forceinline __m256i	avx2_packi32toi16(__m256i lo, __m256i hi);
								
								__forceinline __m256i	avx2_bgra2gray32(__m256i bgra);
								__forceinline __m256i	avx2_bgra2gray(__m256i bgra[4]);
								//__forceinline void		avx2_bgra_load(bool aligned, const uint8_t * p, __m256i a[4]);
								void					avx2_bgra2gray(bool align, const uint8_t * bgra, size_t width, size_t height, size_t bgra_stride, uint8_t * gray, size_t gray_stride);

								__forceinline __m256i	avx2_set_mask(bool aligned, uint8_t first, size_t position, uint8_t second);
								__forceinline uint64_t	avx2_extract(bool aligned, __m256i value);
								bool					avx2_is_different(bool aligned, const uint8_t * a, size_t a_stride, const uint8_t * b, size_t b_stride, size_t width, size_t height);
#endif
#endif
							private:
								sirius::library::video::transform::codec::partial::png::compressor *			_front;
								sirius::library::video::transform::codec::partial::png::compressor::context_t * _context;

								CRITICAL_SECTION	_cs;
								int32_t				_state;
								bool				_run;
								HANDLE				_thread;
								HANDLE				_event;

								sirius::library::video::transform::codec::partial::png::compressor::core::buffer_t										_iobuffer[MAX_IO_BUFFERS];
								sirius::queue<sirius::library::video::transform::codec::partial::png::compressor::core::buffer_t>						_iobuffer_queue;
								sirius::library::video::transform::codec::libpng::compressor *															_real_compressor;								
								std::map<int32_t, sirius::library::video::transform::codec::partial::png::compressor::core::compressed_cache_image_t*>	_indexed_cache_image;
								bool _invalidate;
							};
						};
					};
				};
			};
		};
	};
};


#endif
