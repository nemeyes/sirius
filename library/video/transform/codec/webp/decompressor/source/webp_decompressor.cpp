#include "sirius_webp_decompressor.h"
#include "webp_decompressor.h"
#include <sirius_locks.h>

sirius::library::video::transform::codec::webp::decompressor::core::core(void)
	: _recvd_size(0)
	, _recvd_index(0)
	, _decoding(false)
{
	::InitializeCriticalSection(&_cs);
}

sirius::library::video::transform::codec::webp::decompressor::core::~core(void)
{
	::DeleteCriticalSection(&_cs);
}

int32_t sirius::library::video::transform::codec::webp::decompressor::core::initialize(sirius::library::video::transform::codec::webp::decompressor::context_t * context)
{
	return sirius::library::video::transform::codec::webp::decompressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::webp::decompressor::core::release(void)
{
	return sirius::library::video::transform::codec::webp::decompressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::webp::decompressor::core::decompress(sirius::library::video::transform::codec::webp::decompressor::entity_t * input, sirius::library::video::transform::codec::webp::decompressor::entity_t * output)
{
	sirius::autolock lock(&_cs);
	int32_t width=0, height = 0;
	uint8_t * bgra = WebPDecodeBGRA((const uint8_t*)input->data, (size_t)input->data_size, &width, &height);
	if (bgra)
	{
		output->width = width;
		output->height = height;
		output->data_size = (output->width * output->height) << 2;
		memcpy(output->data, bgra, output->data_size);
	}

	return sirius::library::video::transform::codec::webp::decompressor::err_code_t::success;
}
