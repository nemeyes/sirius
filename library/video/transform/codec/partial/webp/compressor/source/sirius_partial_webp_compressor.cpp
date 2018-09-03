#include "sirius_partial_webp_compressor.h"
#include "partial_webp_compressor.h"

sirius::library::video::transform::codec::partial::webp::compressor::_context_t::_context_t(void)
	: block_width(128)
	, block_height(72)
	, mb_width(8)
	, mb_height(8)
	, quality(100)
	, method(0)
	, binvalidate(true)
	, indexed_video(true)
	, nthread(20)
{

}

sirius::library::video::transform::codec::partial::webp::compressor::_context_t::_context_t(const sirius::library::video::transform::codec::partial::webp::compressor::_context_t & clone)
{
	block_width = clone.block_width;
	block_height = clone.block_height;
	mb_width = clone.mb_width;
	mb_height = clone.mb_height;
	quality = clone.quality;
	method = clone.method;
	binvalidate = clone.binvalidate;
	indexed_video = clone.indexed_video;
	nthread = clone.nthread;
}

sirius::library::video::transform::codec::partial::webp::compressor::_context_t & sirius::library::video::transform::codec::partial::webp::compressor::_context_t::operator=(const sirius::library::video::transform::codec::partial::webp::compressor::_context_t & clone)
{
	block_width = clone.block_width;
	block_height = clone.block_height;
	mb_width = clone.mb_width;
	mb_height = clone.mb_height;
	quality = clone.quality;
	method = clone.method;
	binvalidate = clone.binvalidate;
	indexed_video = clone.indexed_video;
	nthread = clone.nthread;
	return (*this);
}

sirius::library::video::transform::codec::partial::webp::compressor::compressor(void)
{
	_core = new sirius::library::video::transform::codec::partial::webp::compressor::core(this);
}

sirius::library::video::transform::codec::partial::webp::compressor::~compressor(void)
{
	if (_core)
		delete _core;
	_core = nullptr;
}

int32_t sirius::library::video::transform::codec::partial::webp::compressor::initialize(sirius::library::video::transform::codec::partial::webp::compressor::context_t * context)
{
	return _core->initialize(context);
}

int32_t sirius::library::video::transform::codec::partial::webp::compressor::release(void)
{
	return _core->release();
}

int32_t sirius::library::video::transform::codec::partial::webp::compressor::play(void)
{
	return _core->play();
}

int32_t sirius::library::video::transform::codec::partial::webp::compressor::pause(void)
{
	return _core->pause();
}

int32_t sirius::library::video::transform::codec::partial::webp::compressor::stop(void)
{
	return _core->stop();
}

int32_t sirius::library::video::transform::codec::partial::webp::compressor::invalidate(void)
{
	return _core->invalidate();
}

int32_t sirius::library::video::transform::codec::partial::webp::compressor::compress(sirius::library::video::transform::codec::partial::webp::compressor::entity_t * input, sirius::library::video::transform::codec::partial::webp::compressor::entity_t * bitstream)
{
	return _core->compress(input, bitstream);
}

int32_t sirius::library::video::transform::codec::partial::webp::compressor::compress(sirius::library::video::transform::codec::partial::webp::compressor::entity_t * input)
{
	return _core->compress(input);
}