#include <sirius_video_decompressor.h>


sirius::library::video::transform::codec::decompressor::_context_t::_context_t(void)
	: memtype(sirius::library::video::transform::codec::decompressor::video_memory_type_t::host)
	, device(nullptr)
	, width(0)
	, height(0)
	, nbuffer(0)
{

}

sirius::library::video::transform::codec::decompressor::_context_t::_context_t(const sirius::library::video::transform::codec::decompressor::_context_t & clone)
{
	memtype = clone.memtype;
	device = clone.device;
	width = clone.width;
	height = clone.height;
	nbuffer = clone.nbuffer;
}

sirius::library::video::transform::codec::decompressor::_context_t & sirius::library::video::transform::codec::decompressor::_context_t::operator=(const sirius::library::video::transform::codec::decompressor::_context_t & clone)
{
	memtype = clone.memtype;
	device = clone.device;
	width = clone.width;
	height = clone.height;
	nbuffer = clone.nbuffer;
	return (*this);
}

sirius::library::video::transform::codec::decompressor::decompressor(void)
{

}

sirius::library::video::transform::codec::decompressor::~decompressor(void)
{

}

int32_t sirius::library::video::transform::codec::decompressor::initialize(sirius::library::video::transform::codec::decompressor::context_t * context)
{
	return sirius::library::video::transform::codec::decompressor::err_code_t::not_implemented;
}

int32_t sirius::library::video::transform::codec::decompressor::release(void)
{
	return sirius::library::video::transform::codec::decompressor::err_code_t::not_implemented;
}

int32_t sirius::library::video::transform::codec::decompressor::decompress(sirius::library::video::transform::codec::decompressor::entity_t * input, sirius::library::video::transform::codec::decompressor::entity_t * output)
{
	return sirius::library::video::transform::codec::decompressor::err_code_t::not_implemented;
}