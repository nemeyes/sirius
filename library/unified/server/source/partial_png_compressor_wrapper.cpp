#include "partial_png_compressor_wrapper.h"
#include "unified_compressor.h"

sirius::library::unified::partialpng::compressor::compressor(sirius::library::unified::compressor * compressor)
	: _compressor(compressor)
{}

sirius::library::unified::partialpng::compressor::~compressor(void)
{}

void sirius::library::unified::partialpng::compressor::after_process_callback(int32_t count, int32_t * index, uint8_t ** compressed, int32_t * size, long long before_compress_timestamp, long long after_compress_timestamp)
{
	if (_compressor)
		_compressor->after_video_compressing_callback(count, index, compressed, size, before_compress_timestamp, after_compress_timestamp);
}

void sirius::library::unified::partialpng::compressor::after_process_callback(int32_t index, uint8_t * compressed, int32_t size, long long before_compress_timestamp, long long after_compress_timestamp)
{
	if (_compressor)
		_compressor->after_video_compressing_callback(index, compressed, size, before_compress_timestamp, after_compress_timestamp);
}

void sirius::library::unified::partialpng::compressor::after_process_callback(int32_t count, int16_t * x, int16_t * y, int16_t * width, int16_t * height, uint8_t ** compressed, int32_t * size, long long before_compress_timestamp, long long after_compress_timestamp)
{
	if (_compressor)
		_compressor->after_video_compressing_callback(count, x, y, width, height, compressed, size, before_compress_timestamp, after_compress_timestamp);
}

void sirius::library::unified::partialpng::compressor::after_process_callback(int16_t x, int16_t y, int16_t width, int16_t height, uint8_t * compressed, int32_t size, long long before_compress_timestamp, long long after_compress_timestamp)
{
	if (_compressor)
		_compressor->after_video_compressing_callback(x, y, width, height, compressed, size, before_compress_timestamp, after_compress_timestamp);
}