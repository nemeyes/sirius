#include "png_compressor_wrapper.h"
#include "unified_compressor.h"

sirius::library::unified::png::compressor::compressor(sirius::library::unified::compressor * compressor)
	: _compressor(compressor)
{}

sirius::library::unified::png::compressor::~compressor(void)
{}

void sirius::library::unified::png::compressor::after_process_callback(uint8_t * compressed, int32_t size, long long before_encode_timestamp, long long after_encode_timestamp)
{
	if (_compressor)
		_compressor->after_video_compressing_callback(compressed, size, before_encode_timestamp, after_encode_timestamp);
}