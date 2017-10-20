#include "sirius_png_decompressor.h"
#include "png_decompressor.h"

sirius::library::video::transform::codec::png::decompressor::_context_t::_context_t(void)
{

}

sirius::library::video::transform::codec::png::decompressor::_context_t::_context_t(const sirius::library::video::transform::codec::png::decompressor::_context_t & clone)
{

}

sirius::library::video::transform::codec::png::decompressor::_context_t & sirius::library::video::transform::codec::png::decompressor::_context_t::operator=(const sirius::library::video::transform::codec::png::decompressor::_context_t & clone)
{
	return (*this);
}

sirius::library::video::transform::codec::png::decompressor::decompressor(void)
{
	_core = new sirius::library::video::transform::codec::png::decompressor::core();
}

sirius::library::video::transform::codec::png::decompressor::~decompressor(void)
{
	if (_core)
		delete _core;
	_core = nullptr;
}

int32_t sirius::library::video::transform::codec::png::decompressor::initialize(sirius::library::video::transform::codec::png::decompressor::context_t * context)
{
	return _core->initialize(context);
}

int32_t sirius::library::video::transform::codec::png::decompressor::release(void)
{
	return _core->release();
}

int32_t sirius::library::video::transform::codec::png::decompressor::decompress(sirius::library::video::transform::codec::png::decompressor::entity_t * input, sirius::library::video::transform::codec::png::decompressor::entity_t * output)
{
	return _core->decompress(input, output);
}