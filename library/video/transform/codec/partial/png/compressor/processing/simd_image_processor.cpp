#include "simd_image_processor.h"
#include <assert.h>
#include <math.h>

template <class T>
void sirius::library::video::transform::codec::partial::simd::swap(T & a, T & b)
{
	T t = a;
	a = b;
	b = t;
}

size_t sirius::library::video::transform::codec::partial::simd::align_h(size_t size, size_t align)
{
	return (size + align - 1) & ~(align - 1);
}

void * sirius::library::video::transform::codec::partial::simd::align_h(const void * ptr, size_t align)
{
	return (void *)((((size_t)ptr) + align - 1) & ~(align - 1));
}

size_t sirius::library::video::transform::codec::partial::simd::align_l(size_t size, size_t align)
{
	return size & ~(align - 1);
}

void * sirius::library::video::transform::codec::partial::simd::align_l(const void * ptr, size_t align)
{
	return (void *)(((size_t)ptr) & ~(align - 1));
}

bool sirius::library::video::transform::codec::partial::simd::aligned(size_t size, size_t align)
{
	return size == align_l(size, align);
}

bool sirius::library::video::transform::codec::partial::simd::aligned(const void * ptr, size_t align)
{
	return ptr == align_l(ptr, align);
}

__m256i	sirius::library::video::transform::codec::partial::simd::avx2::load(bool aligned, const __m256i * p)
{
	if (aligned)
		return _mm256_load_si256(p);
	else
		return _mm256_loadu_si256(p);
}

void sirius::library::video::transform::codec::partial::simd::avx2::store(bool aligned, __m256i * p, __m256i a)
{
	if (aligned)
		return _mm256_store_si256(p, a);
	else
		return _mm256_storeu_si256(p, a);
}

__m256i	sirius::library::video::transform::codec::partial::simd::avx2::pack_u16_to_u8(__m256i lo, __m256i hi)
{
	return _mm256_permute4x64_epi64(_mm256_packus_epi16(lo, hi), 0xD8);
}

__m256i	sirius::library::video::transform::codec::partial::simd::avx2::pack_i32_to_i16(__m256i lo, __m256i hi)
{
	return _mm256_permute4x64_epi64(_mm256_packs_epi32(lo, hi), 0xD8);
}

__m256i	sirius::library::video::transform::codec::partial::simd::avx2::set_mask(bool aligned, uint8_t first, size_t position, uint8_t second)
{
	const size_t size = SIMD_ALIGN / sizeof(uint8_t);
	assert(position <= size);
	uint8_t mask[size];
	for (size_t i = 0; i < position; ++i)
		mask[i] = first;
	for (size_t i = position; i < size; ++i)
		mask[i] = second;
	if (aligned)
		return _mm256_load_si256((__m256i*)mask);
	else
		return _mm256_loadu_si256((__m256i*)mask);
}

uint64_t sirius::library::video::transform::codec::partial::simd::avx2::extract(bool aligned, __m256i value)
{
	const size_t size = SIMD_ALIGN / sizeof(uint64_t);
	uint64_t buffer[size];

	if (aligned)
		_mm256_store_si256((__m256i*)buffer, value);
	else
		_mm256_storeu_si256((__m256i*)buffer, value);

	uint64_t sum = 0;
	for (size_t i = 0; i < size; ++i)
		sum += buffer[i];

	return sum;
}

__m256i sirius::library::video::transform::codec::partial::simd::avx2::unpack_u8(int32_t part, __m256i a, __m256i b)
{
	if(part==0)
		return _mm256_unpacklo_epi8(a, b);
	else
		return _mm256_unpackhi_epi8(a, b);
}

__m256i sirius::library::video::transform::codec::partial::simd::avx2::sub_unpacked_u8(int32_t part, __m256i a, __m256i b)
{
	return _mm256_maddubs_epi16(unpack_u8(part, a, b), K8_01_FF);
}

__m256i sirius::library::video::transform::codec::partial::simd::avx2::horizontal_sum32(__m256i a)
{
	return _mm256_add_epi64(_mm256_unpacklo_epi32(a, sirius::library::video::transform::codec::partial::simd::K_ZERO), _mm256_unpackhi_epi32(a, sirius::library::video::transform::codec::partial::simd::K_ZERO));
}

void sirius::library::video::transform::codec::partial::simd::estimate_alpha_index(size_t ssize, size_t dsize, int32_t * indexes, int32_t * alphas, size_t channels)
{
	float scale = (float)ssize / dsize;

	for (size_t i = 0; i < dsize; ++i)
	{
		float alpha = (float)((i + 0.5)*scale - 0.5);
		ptrdiff_t index = (ptrdiff_t)::floor(alpha);
		alpha -= index;

		if (index < 0)
		{
			index = 0;
			alpha = 0;
		}

		if (index >(ptrdiff_t)ssize - 2)
		{
			index = ssize - 2;
			alpha = 1;
		}

		for (size_t c = 0; c < channels; c++)
		{
			size_t offset = i*channels + c;
			indexes[offset] = (int)(channels*index + c);
			alphas[offset] = (int)(alpha * sirius::library::video::transform::codec::partial::simd::FRACTION_RANGE + 0.5);
		}
	}
}

void sirius::library::video::transform::codec::partial::simd::avx2::resizer::resize_bilinear(const uint8_t * src, size_t swidth, size_t sheight, size_t sstride, uint8_t *dst, size_t dwidth, size_t dheight, size_t dstride)
{
	assert(dwidth >= A);

	struct One { uint8_t channels[CHANNELS]; };
	struct Two { uint8_t channels[CHANNELS * 2]; };

	size_t size			= 2 * dwidth * CHANNELS;
	size_t buffer_size	= align_h(dwidth, A)* CHANNELS * 2;
	size_t aligned_size = align_h(size, DA) - DA;
	const size_t step	= A * CHANNELS;

	sirius::library::video::transform::codec::partial::simd::avx2::resizer::buffer_t buffer(buffer_size, dwidth, dheight);
	sirius::library::video::transform::codec::partial::simd::estimate_alpha_index(sheight, dheight, buffer.iy, buffer.ay, 1);
	sirius::library::video::transform::codec::partial::simd::avx2::resizer::estimate_alpha_index_x(swidth, dwidth, buffer.ix, buffer.ax);

	ptrdiff_t previous = -2;
	__m256i a[2];
	for (size_t yDst = 0; yDst < dheight; yDst++, dst += dstride)
	{
		a[0] = _mm256_set1_epi16(int16_t(sirius::library::video::transform::codec::partial::simd::FRACTION_RANGE - buffer.ay[yDst]));
		a[1] = _mm256_set1_epi16(int16_t(buffer.ay[yDst]));

		ptrdiff_t sy = buffer.iy[yDst];
		int k = 0;

		if (sy == previous)
		{
			k = 2;
		}
		else if (sy == previous + 1)
		{
			sirius::library::video::transform::codec::partial::simd::swap(buffer.bx[0], buffer.bx[1]);
			k = 1;
		}

		previous = sy;
		for (; k < 2; k++)
		{
			Two * pb = (Two *)buffer.bx[k];
			const One * psrc = (const One *)(src + (sy + k)*sstride);
			for (size_t x = 0; x < dwidth; x++)
			{
				pb[x] = *(Two *)(psrc + buffer.ix[x]);
			}

			uint8_t * pbx = buffer.bx[k];
			for (size_t i = 0; i < buffer_size; i += step)
			{
				interpolate_x((__m256i*)(buffer.ax + i), (__m256i*)(pbx + i));
			}
		}

		for (size_t ib = 0, id = 0; ib < aligned_size; ib += DA, id += A)
		{
			interpolate_y(true, buffer.bx[0] + ib, buffer.bx[1] + ib, a, dst + id);
		}
		size_t i = size - DA;
		interpolate_y(false, buffer.bx[0] + i, buffer.bx[1] + i, a, dst + i / 2);
	}
}

void sirius::library::video::transform::codec::partial::simd::avx2::resizer::estimate_alpha_index_x(size_t ssize, size_t dsize, int32_t * indexes, uint8_t * alphas)
{
	float scale = (float)ssize / dsize;

	for (size_t i = 0; i < dsize; ++i)
	{
		float alpha		 = (float)((i + 0.5)*scale - 0.5);
		ptrdiff_t index  = (ptrdiff_t)::floor(alpha);
		alpha			-= index;

		if (index < 0)
		{
			index = 0;
			alpha = 0;
		}

		if (index >(ptrdiff_t)ssize - 2)
		{
			index = ssize - 2;
			alpha = 1;
		}

		indexes[i]	= (int)index;
		alphas[1]	= (uint8_t)(alpha * sirius::library::video::transform::codec::partial::simd::FRACTION_RANGE + 0.5);
		alphas[0]	= (uint8_t)(sirius::library::video::transform::codec::partial::simd::FRACTION_RANGE - alphas[1]);

		for (size_t channel = 1; channel < CHANNELS; channel++)
			((uint16_t*)alphas)[channel] = *(uint16_t*)alphas;
		alphas += 2 * CHANNELS;
	}
}

void sirius::library::video::transform::codec::partial::simd::avx2::resizer::interpolate_x(const __m256i * alpha, __m256i * buffer)
{
	interpolate_inner_x(alpha + 0, buffer + 0);
	interpolate_inner_x(alpha + 1, buffer + 1);
	interpolate_inner_x(alpha + 2, buffer + 2);
	interpolate_inner_x(alpha + 3, buffer + 3);
}

void sirius::library::video::transform::codec::partial::simd::avx2::resizer::interpolate_y(bool aligned, const uint8_t * bx0, const uint8_t * bx1, __m256i alpha[2], uint8_t * dst)
{
	__m256i lo = interpolate_y(aligned, (__m256i*)bx0 + 0, (__m256i*)bx1 + 0, alpha);
	__m256i hi = interpolate_y(aligned, (__m256i*)bx0 + 1, (__m256i*)bx1 + 1, alpha);
	_mm256_storeu_si256((__m256i*)dst, pack_u16_to_u8(lo, hi));
}

void sirius::library::video::transform::codec::partial::simd::avx2::resizer::interpolate_inner_x(const __m256i * alpha, __m256i * buffer)
{
	__m256i src = _mm256_shuffle_epi8(_mm256_load_si256(buffer), sirius::library::video::transform::codec::partial::simd::K8_SHUFFLE_X4);
	_mm256_store_si256(buffer, _mm256_maddubs_epi16(src, _mm256_load_si256(alpha)));
}

const __m256i K16_FRACTION_ROUND_TERM = ASIGN_MM256_SET1_EPI16(sirius::library::video::transform::codec::partial::simd::BILINEAR_ROUND_TERM);
__m256i sirius::library::video::transform::codec::partial::simd::avx2::resizer::interpolate_y(bool aligned, const __m256i * pbx0, const __m256i * pbx1, __m256i alpha[2])
{
	if (aligned)
	{
		__m256i sum = _mm256_add_epi16(_mm256_mullo_epi16(_mm256_load_si256(pbx0), alpha[0]), _mm256_mullo_epi16(_mm256_load_si256(pbx1), alpha[1]));
		return _mm256_srli_epi16(_mm256_add_epi16(sum, K16_FRACTION_ROUND_TERM), sirius::library::video::transform::codec::partial::simd::BILINEAR_SHIFT);
	}
	else
	{
		__m256i sum = _mm256_add_epi16(_mm256_mullo_epi16(_mm256_loadu_si256(pbx0), alpha[0]), _mm256_mullo_epi16(_mm256_loadu_si256(pbx1), alpha[1]));
		return _mm256_srli_epi16(_mm256_add_epi16(sum, K16_FRACTION_ROUND_TERM), sirius::library::video::transform::codec::partial::simd::BILINEAR_SHIFT);
	}
}

bool sirius::library::video::transform::codec::partial::simd::avx2::evaluator::abs_eval(bool baligned, const uint8_t * a, size_t a_stride, const uint8_t * b, size_t b_stride, size_t width, size_t height)
{
	assert(width >= SIMD_ALIGN);
	if (baligned)
	{
		assert(aligned(a_stride, SIMD_ALIGN));
		assert(aligned(b_stride, SIMD_ALIGN));
		assert(aligned(b, SIMD_ALIGN));
		assert(aligned(a, SIMD_ALIGN));
	}

	size_t body_width = sirius::library::video::transform::codec::partial::simd::align_l(width, SIMD_ALIGN);
	__m256i tail_mask = sirius::library::video::transform::codec::partial::simd::avx2::set_mask(baligned, 0, SIMD_ALIGN - width + body_width, 0xFF);
	__m256i sum = _mm256_setzero_si256();
	__m256i compare = _mm256_setzero_si256();
	for (size_t row = 0; row < height; ++row)
	{
		for (size_t col = 0; col < body_width; col += SIMD_ALIGN)
		{
			const __m256i a_ = sirius::library::video::transform::codec::partial::simd::avx2::load(baligned, (__m256i*)(a + col));
			const __m256i b_ = sirius::library::video::transform::codec::partial::simd::avx2::load(baligned, (__m256i*)(b + col));
			sum = _mm256_add_epi64(_mm256_sad_epu8(a_, b_), sum);

			if (sirius::library::video::transform::codec::partial::simd::avx2::extract(true, sum) > 0)
				return true;
		}
		if (width - body_width)
		{
			const __m256i a_ = _mm256_and_si256(tail_mask, sirius::library::video::transform::codec::partial::simd::avx2::load(false, (__m256i*)(a + width - SIMD_ALIGN)));
			const __m256i b_ = _mm256_and_si256(tail_mask, sirius::library::video::transform::codec::partial::simd::avx2::load(false, (__m256i*)(b + width - SIMD_ALIGN)));
			sum = _mm256_add_epi64(_mm256_sad_epu8(a_, b_), sum);

			if (sirius::library::video::transform::codec::partial::simd::avx2::extract(false, sum) > 0)
				return true;
		}
		a += a_stride;
		b += b_stride;
	}

	if (sirius::library::video::transform::codec::partial::simd::avx2::extract(false, sum) > 0)
		return true;
	else
		return false;
}

bool sirius::library::video::transform::codec::partial::simd::avx2::evaluator::squared_eval(bool align, const uint8_t * a, size_t a_stride, const uint8_t * b, size_t b_stride, size_t width, size_t height)
{
	assert(width < 0x10000);
	if (align)
	{
		assert(aligned(a, SIMD_ALIGN) && aligned(a_stride, SIMD_ALIGN) && aligned(b, SIMD_ALIGN) && aligned(b_stride, SIMD_ALIGN));
	}

	size_t body_width = sirius::library::video::transform::codec::partial::simd::align_l(width, A);

	__m256i tail_mask = sirius::library::video::transform::codec::partial::simd::avx2::set_mask(false, 0, A - width + body_width, 0xFF);
	__m256i full_sum = _mm256_setzero_si256();
	for (size_t row = 0; row < height; ++row)
	{
		__m256i row_sum = _mm256_setzero_si256();
		for (size_t col = 0; col < body_width; col += A)
		{
			const __m256i a_ = sirius::library::video::transform::codec::partial::simd::avx2::load(align, (__m256i*)(a + col));
			const __m256i b_ = sirius::library::video::transform::codec::partial::simd::avx2::load(align, (__m256i*)(b + col));
			row_sum = _mm256_add_epi32(row_sum, sirius::library::video::transform::codec::partial::simd::avx2::evaluator::squared_difference(a_, b_));
			if (sirius::library::video::transform::codec::partial::simd::avx2::extract(false, row_sum) > 0)
				return true;
		}
		if (width - body_width)
		{
			const __m256i a_ = _mm256_and_si256(tail_mask, sirius::library::video::transform::codec::partial::simd::avx2::load(false, (__m256i*)(a + width - A)));
			const __m256i b_ = _mm256_and_si256(tail_mask, sirius::library::video::transform::codec::partial::simd::avx2::load(false, (__m256i*)(b + width - A)));
			row_sum = _mm256_add_epi32(row_sum, sirius::library::video::transform::codec::partial::simd::avx2::evaluator::squared_difference(a_, b_));
			if (sirius::library::video::transform::codec::partial::simd::avx2::extract(false, row_sum) > 0)
				return true;
		}
		full_sum = _mm256_add_epi64(full_sum, sirius::library::video::transform::codec::partial::simd::avx2::horizontal_sum32(row_sum));
		a += a_stride;
		b += b_stride;
	}

	if (sirius::library::video::transform::codec::partial::simd::avx2::extract(false, full_sum) > 0)
		return true;
	else
		return false;
}

__m256i sirius::library::video::transform::codec::partial::simd::avx2::evaluator::squared_difference(__m256i a, __m256i b)
{
	const __m256i lo = sirius::library::video::transform::codec::partial::simd::avx2::sub_unpacked_u8(0, a, b);
	const __m256i hi = sirius::library::video::transform::codec::partial::simd::avx2::sub_unpacked_u8(1, a, b);
	return _mm256_add_epi32(_mm256_madd_epi16(lo, lo), _mm256_madd_epi16(hi, hi));
}

bool sirius::library::video::transform::codec::partial::simd::avx2::connected_component::find(const uint8_t * img, int16_t width, int16_t height)
{
	const uint8_t * row			= img;
	const uint8_t * last_row	= 0;

	return false;
}

/*
__m256i	sirius::library::video::transform::codec::partial::png::compressor::core::avx2_bgra2gray32(__m256i bgra)
{
	const __m256i g0a0 = _mm256_and_si256(_mm256_srli_si256(bgra, 1), K16_00FF);
	const __m256i b0r0 = _mm256_and_si256(bgra, K16_00FF);
	const __m256i weightedSum = _mm256_add_epi32(_mm256_madd_epi16(g0a0, K16_GREEN_0000), _mm256_madd_epi16(b0r0, K16_BLUE_RED));
	return _mm256_srli_epi32(_mm256_add_epi32(weightedSum, K32_ROUND_TERM), BGR_TO_GRAY_AVERAGING_SHIFT);
}

__m256i	sirius::library::video::transform::codec::partial::png::compressor::core::avx2_bgra2gray(__m256i bgra[4])
{
	const __m256i lo = avx2_packi32toi16(avx2_bgra2gray32(bgra[0]), avx2_bgra2gray32(bgra[1]));
	const __m256i hi = avx2_packi32toi16(avx2_bgra2gray32(bgra[2]), avx2_bgra2gray32(bgra[3]));
	return avx2_pack_u16tou8(lo, hi);
}

void sirius::library::video::transform::codec::partial::png::compressor::core::avx2_bgra2gray(bool aligned, const uint8_t * bgra, size_t width, size_t height, size_t bgra_stride, uint8_t * gray, size_t gray_stride)
{
	assert(width >= AVX2_ALIGN_SIZE);
	if (aligned)
		assert(avx2_is_aligned(bgra, AVX2_ALIGN_SIZE) && avx2_is_aligned(bgra_stride, AVX2_ALIGN_SIZE) && avx2_is_aligned(gray, AVX2_ALIGN_SIZE) && avx2_is_aligned(gray_stride, AVX2_ALIGN_SIZE));

	size_t aligned_width = avx2_align_low(width, AVX2_ALIGN_SIZE);
	__m256i a[4];
	for (size_t row = 0; row < height; ++row)
	{
		for (size_t col = 0; col < aligned_width; col += AVX2_ALIGN_SIZE)
		{
			a[0] = avx2_load(aligned, (__m256i*)(bgra + 4 * col) + 0);
			a[1] = avx2_load(aligned, (__m256i*)(bgra + 4 * col) + 1);
			a[2] = avx2_load(aligned, (__m256i*)(bgra + 4 * col) + 2);
			a[3] = avx2_load(aligned, (__m256i*)(bgra + 4 * col) + 3);
			avx2_store(aligned, (__m256i*)(gray + col), avx2_bgra2gray(a));
		}
		if (aligned_width != width)
		{
			a[0] = avx2_load(aligned, (__m256i*)(bgra + 4 * (width - AVX2_ALIGN_SIZE)) + 0);
			a[1] = avx2_load(aligned, (__m256i*)(bgra + 4 * (width - AVX2_ALIGN_SIZE)) + 1);
			a[2] = avx2_load(aligned, (__m256i*)(bgra + 4 * (width - AVX2_ALIGN_SIZE)) + 2);
			a[3] = avx2_load(aligned, (__m256i*)(bgra + 4 * (width - AVX2_ALIGN_SIZE)) + 3);
			avx2_store(false, (__m256i*)(gray + width - AVX2_ALIGN_SIZE), avx2_bgra2gray(a));
		}
		bgra += bgra_stride;
		gray += gray_stride;
	}
}

__m256i	sirius::library::video::transform::codec::partial::png::compressor::core::avx2_set_mask(bool aligned, uint8_t first, size_t position, uint8_t second)
{
	const size_t size = AVX2_ALIGN_SIZE / sizeof(uint8_t);
	assert(position <= size);
	uint8_t mask[size];
	for (size_t i = 0; i < position; ++i)
		mask[i] = first;
	for (size_t i = position; i < size; ++i)
		mask[i] = second;
	if (aligned)
		return _mm256_load_si256((__m256i*)mask);
	else
		return _mm256_loadu_si256((__m256i*)mask);
}

uint64_t sirius::library::video::transform::codec::partial::png::compressor::core::avx2_extract(bool aligned, __m256i value)
{
	const size_t size = AVX2_ALIGN_SIZE / sizeof(uint64_t);
	uint64_t buffer[size];

	if (aligned)
		_mm256_store_si256((__m256i*)buffer, value);
	else
		_mm256_storeu_si256((__m256i*)buffer, value);

	uint64_t sum = 0;
	for (size_t i = 0; i < size; ++i)
		sum += buffer[i];

	return sum;
}

bool sirius::library::video::transform::codec::partial::png::compressor::core::avx2_is_different(bool aligned, const uint8_t * a, size_t a_stride, const uint8_t * b, size_t b_stride, size_t width, size_t height)
{

}
*/