#include <sirius_video_compressor.h>


sirius::library::video::transform::codec::compressor::_context_t::_context_t(void)
	: memtype(sirius::library::video::transform::codec::compressor::video_memory_type_t::host)
	, device(nullptr)
	, width(0)
	, height(0)
	, nbuffer(0)
{

}

sirius::library::video::transform::codec::compressor::_context_t::_context_t(const sirius::library::video::transform::codec::compressor::_context_t & clone)
{
	memtype = clone.memtype;
	device = clone.device;
	width = clone.width;
	height = clone.height;
	nbuffer = clone.nbuffer;
}

sirius::library::video::transform::codec::compressor::_context_t & sirius::library::video::transform::codec::compressor::_context_t::operator=(const sirius::library::video::transform::codec::compressor::_context_t & clone)
{
	memtype = clone.memtype;
	device = clone.device;
	width = clone.width;
	height = clone.height;
	nbuffer = clone.nbuffer;
	return (*this);
}

sirius::library::video::transform::codec::compressor::compressor(void)
{

}

sirius::library::video::transform::codec::compressor::~compressor(void)
{

}

int32_t sirius::library::video::transform::codec::compressor::initialize(sirius::library::video::transform::codec::compressor::context_t * context)
{
	return sirius::library::video::transform::codec::compressor::err_code_t::not_implemented;
}

int32_t sirius::library::video::transform::codec::compressor::release(void)
{
	return sirius::library::video::transform::codec::compressor::err_code_t::not_implemented;
}

int32_t sirius::library::video::transform::codec::compressor::play(void)
{
	return sirius::library::video::transform::codec::compressor::err_code_t::not_implemented;
}

int32_t sirius::library::video::transform::codec::compressor::pause(void)
{
	return sirius::library::video::transform::codec::compressor::err_code_t::not_implemented;
}

int32_t sirius::library::video::transform::codec::compressor::stop(void)
{
	return sirius::library::video::transform::codec::compressor::err_code_t::not_implemented;
}

int32_t sirius::library::video::transform::codec::compressor::invalidate(void)
{
	return sirius::library::video::transform::codec::compressor::err_code_t::not_implemented;
}

int32_t sirius::library::video::transform::codec::compressor::compress(sirius::library::video::transform::codec::compressor::entity_t * input)
{
	return sirius::library::video::transform::codec::compressor::err_code_t::not_implemented;
}

int32_t sirius::library::video::transform::codec::compressor::compress(sirius::library::video::transform::codec::compressor::entity_t * input, sirius::library::video::transform::codec::compressor::entity_t * bitstream)
{
	return sirius::library::video::transform::codec::compressor::err_code_t::not_implemented;
}

void sirius::library::video::transform::codec::compressor::after_process_callback(uint8_t * compressed, int32_t size, long long before_compress_timestamp, long long after_compress_timestamp)
{

}

void sirius::library::video::transform::codec::compressor::after_process_callback(int32_t count, int32_t * index, uint8_t ** compressed, int32_t * size, long long before_compress_timestamp, long long after_compress_timestamp)
{

}