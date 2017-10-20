#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sirius_simd_colorspace_converter.h"
#include "simd_colorspace_converter.h"

#include <Simd/SimdLib.h>

uint8_t sirius::library::video::transform::colorspace::simd::converter::core::_buffer[8294400]; //1920*1080*4
sirius::library::video::transform::colorspace::simd::converter::core::core(void)
	: _context(nullptr)
{

}

sirius::library::video::transform::colorspace::simd::converter::core::~core(void)
{

}

int32_t sirius::library::video::transform::colorspace::simd::converter::core::initialize(sirius::library::video::transform::colorspace::simd::converter::context_t * context)
{
	_context = context;
	return sirius::library::video::transform::colorspace::simd::converter::err_code_t::success;
}

int32_t sirius::library::video::transform::colorspace::simd::converter::core::release(void)
{
	_context = nullptr;
	return sirius::library::video::transform::colorspace::simd::converter::err_code_t::success;
}

int32_t sirius::library::video::transform::colorspace::simd::converter::core::convert(sirius::library::video::transform::colorspace::simd::converter::entity_t * input, sirius::library::video::transform::colorspace::simd::converter::entity_t * output)
{
	return sirius::library::video::transform::colorspace::simd::converter::err_code_t::success;
}

void sirius::library::video::transform::colorspace::simd::converter::core::flip(int32_t width, int32_t height, int32_t stride, uint8_t * pixels)
{
	uint8_t * row = _buffer;// (unsigned char *)malloc(stride);
	uint8_t * low = pixels;
	uint8_t * high = &pixels[(height - 1) * stride];

	for (; low < high; low += stride, high -= stride)
	{
		memcpy(row, low, stride);
		memcpy(low, high, stride);
		memcpy(high, row, stride);
	}
}

void sirius::library::video::transform::colorspace::simd::converter::core::convert_rgba_to_rgba(int32_t width, int32_t height, uint8_t * src, int32_t src_stride, uint8_t * dst, int32_t dst_stride, bool flip)
{
	if (flip)
	{
		uint8_t * source = src + (height - 1)*src_stride;
		uint8_t * destination = dst;
		int32_t bytes_row = width << 2;
		for (int h = 0; h < height; h++)
		{
			memcpy(destination, source, bytes_row);
			destination += dst_stride;
			source -= src_stride;
		}
	}
	else
	{
		uint8_t * source = src;
		uint8_t * destination = dst;
		int32_t bytes_row = width << 2;
		for (int h = 0; h < height; h++)
		{
			memcpy(destination, source, bytes_row);
			destination += dst_stride;
			source += src_stride;
		}
	}
}

void sirius::library::video::transform::colorspace::simd::converter::core::convert_rgba_to_yv12(int32_t width, int32_t height, uint8_t * src, int32_t src_stride, uint8_t * dst, int32_t dst_stride, bool flip)
{
	int32_t uv_stride = dst_stride >> 1;
	uint8_t * y = dst;
	uint8_t * v = y + (height)* dst_stride;
	uint8_t * u = v + (height >> 1) * uv_stride;

	if (flip)
		sirius::library::video::transform::colorspace::simd::converter::core::flip(width, height, src_stride, src);
	SimdBgraToYuv420p(src, width, height, src_stride, y, dst_stride, u, uv_stride, v, uv_stride);
}

void sirius::library::video::transform::colorspace::simd::converter::core::convert_i420_to_rgba(int32_t width, int32_t height, uint8_t * y, int32_t y_stride, uint8_t * u, int32_t u_stride, uint8_t * v, int32_t v_stride,
	uint8_t * dst, int32_t dst_stride, uint8_t alpha, bool flip)
{
	SimdYuv420pToBgra(y, y_stride, u, u_stride, v, v_stride, width, height, dst, dst_stride, alpha);
	if (flip)
		sirius::library::video::transform::colorspace::simd::converter::core::flip(width, height, dst_stride, dst);
}

void sirius::library::video::transform::colorspace::simd::converter::core::convert_i420_to_rgb(int32_t width, int32_t height, uint8_t * y, int32_t y_stride, uint8_t * u, int32_t u_stride, uint8_t * v, int32_t v_stride,
	uint8_t * dst, int32_t dst_stride, bool flip)
{
	SimdYuv420pToBgr(y, y_stride, u, u_stride, v, v_stride, width, height, dst, dst_stride);
	if (flip)
		sirius::library::video::transform::colorspace::simd::converter::core::flip(width, height, dst_stride, dst);
}

void sirius::library::video::transform::colorspace::simd::converter::core::convert_i420_to_rgba(int32_t width, int32_t height, uint8_t * src, int32_t src_stride, uint8_t * dst, int32_t dst_stride, uint8_t alpha, bool flip)
{
	int32_t uv_stride = src_stride >> 1;
	uint8_t * y = src;
	uint8_t * u = y + (height)* dst_stride;
	uint8_t * v = u + (height >> 1) * uv_stride;

	SimdYuv420pToBgra(y, src_stride, u, uv_stride, v, uv_stride, width, height, dst, dst_stride, alpha);
	if (flip)
		sirius::library::video::transform::colorspace::simd::converter::core::flip(width, height, dst_stride, dst);
}

void sirius::library::video::transform::colorspace::simd::converter::core::convert_i420_to_rgb(int32_t width, int32_t height, uint8_t * src, int32_t src_stride, uint8_t * dst, int32_t dst_stride, bool flip)
{
	int32_t uv_stride = src_stride >> 1;
	uint8_t * y = src;
	uint8_t * u = y + (height)* dst_stride;
	uint8_t * v = u + (height >> 1) * uv_stride;
	SimdYuv420pToBgr(y, src_stride, u, uv_stride, v, uv_stride, width, height, dst, dst_stride);
	if (flip)
		sirius::library::video::transform::colorspace::simd::converter::core::flip(width, height, dst_stride, dst);
}

void sirius::library::video::transform::colorspace::simd::converter::core::convert_yv12_to_rgba(int32_t width, int32_t height, uint8_t * src, int32_t src_stride, uint8_t * dst, int32_t dst_stride, uint8_t alpha, bool flip)
{
	int32_t uv_stride = src_stride >> 1;
	uint8_t * y = src;
	uint8_t * v = y + (height)* dst_stride;
	uint8_t * u = v + (height >> 1) * uv_stride;

	SimdYuv420pToBgra(y, src_stride, u, uv_stride, v, uv_stride, width, height, dst, dst_stride, alpha);
	if (flip)
		sirius::library::video::transform::colorspace::simd::converter::core::flip(width, height, dst_stride, dst);
}

void sirius::library::video::transform::colorspace::simd::converter::core::convert_yv12_to_rgb(int32_t width, int32_t height, uint8_t * src, int32_t src_stride, uint8_t * dst, int32_t dst_stride, bool flip)
{
	int32_t uv_stride = src_stride >> 1;
	uint8_t * y = src;
	uint8_t * v = y + (height)* dst_stride;
	uint8_t * u = v + (height >> 1) * uv_stride;
	SimdYuv420pToBgr(y, src_stride, u, uv_stride, v, uv_stride, width, height, dst, dst_stride);
	if (flip)
		sirius::library::video::transform::colorspace::simd::converter::core::flip(width, height, dst_stride, dst);
}
