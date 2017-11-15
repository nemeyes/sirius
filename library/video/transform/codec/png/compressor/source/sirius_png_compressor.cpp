#include "sirius_png_compressor.h"
#include "png_compressor.h"

sirius::library::video::transform::codec::png::compressor::_context_t::_context_t(void)
	: gamma(1/2.2f)
	, floyd(0.5f)
	, speed(10)
	, max_colors(64)
	, min_quality(50)
	, max_quality(80)
	, fast_compression(true)
{

}

sirius::library::video::transform::codec::png::compressor::_context_t::_context_t(const sirius::library::video::transform::codec::png::compressor::_context_t & clone)
{
	gamma = clone.gamma;
	floyd = clone.floyd;
	speed = clone.speed;
	max_colors = clone.max_colors;
	min_quality = clone.min_quality;
	max_quality = clone.max_quality;
	fast_compression = clone.fast_compression;
}

sirius::library::video::transform::codec::png::compressor::_context_t & sirius::library::video::transform::codec::png::compressor::_context_t::operator=(const sirius::library::video::transform::codec::png::compressor::_context_t & clone)
{
	gamma = clone.gamma;
	floyd = clone.floyd;
	speed = clone.speed;
	max_colors = clone.max_colors;
	min_quality = clone.min_quality;
	max_quality = clone.max_quality;
	fast_compression = clone.fast_compression;
	return (*this);
} 

sirius::library::video::transform::codec::png::compressor::compressor(void)
{
	_core = new sirius::library::video::transform::codec::png::compressor::core(this);
}

sirius::library::video::transform::codec::png::compressor::~compressor(void)
{
	if (_core)
		delete _core;
	_core = nullptr;
}

int32_t sirius::library::video::transform::codec::png::compressor::initialize(sirius::library::video::transform::codec::png::compressor::context_t * context)
{
	return _core->initialize(context);
}

int32_t sirius::library::video::transform::codec::png::compressor::release(void)
{
	return _core->release();
}

int32_t sirius::library::video::transform::codec::png::compressor::play(void)
{
	return _core->play();
}

int32_t sirius::library::video::transform::codec::png::compressor::pause(void)
{
	return _core->pause();
}

int32_t sirius::library::video::transform::codec::png::compressor::stop(void)
{
	return _core->stop();
}

int32_t sirius::library::video::transform::codec::png::compressor::compress(sirius::library::video::transform::codec::png::compressor::entity_t * input, sirius::library::video::transform::codec::png::compressor::entity_t * bitstream)
{
	return _core->compress(input, bitstream);
}

int32_t sirius::library::video::transform::codec::png::compressor::compress(sirius::library::video::transform::codec::compressor::entity_t * input)
{
	return _core->compress(input);
}