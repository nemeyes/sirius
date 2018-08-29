#include "sirius_partial_png_compressor.h"
#include "partial_png_compressor.h"

sirius::library::video::transform::codec::partial::png::compressor::_context_t::_context_t(void)
	: block_width(128)
	, block_height(72)
	, mb_width(8)
	, mb_height(8)
	, compression_level(-1)
	, gamma(1 / 2.2f)
	, floyd(0)
	, speed(10)
	, posterization(true)
	, use_dither_map(false)
	, use_contrast_maps(false)
	, max_colors(256)
	, min_quality(50)
	, max_quality(100)
	, binvalidate(true)
	, indexed_video(true)
	, nthread(20)
{

}

sirius::library::video::transform::codec::partial::png::compressor::_context_t::_context_t(const sirius::library::video::transform::codec::partial::png::compressor::_context_t & clone)
{
	block_width = clone.block_width;
	block_height = clone.block_height;
	mb_width = clone.mb_width;
	mb_height = clone.mb_height;
	compression_level = clone.compression_level;
	gamma = clone.gamma;
	floyd = clone.floyd;
	speed = clone.speed;
	posterization = clone.posterization;
	use_dither_map = clone.use_dither_map;
	use_contrast_maps = clone.use_contrast_maps;
	max_colors = clone.max_colors;
	min_quality = clone.min_quality;
	max_quality = clone.max_quality;
	binvalidate = clone.binvalidate;
	indexed_video = clone.indexed_video;
	nthread = clone.nthread;
}

sirius::library::video::transform::codec::partial::png::compressor::_context_t & sirius::library::video::transform::codec::partial::png::compressor::_context_t::operator=(const sirius::library::video::transform::codec::partial::png::compressor::_context_t & clone)
{
	block_width = clone.block_width;
	block_height = clone.block_height;
	mb_width = clone.mb_width;
	mb_height = clone.mb_height;
	compression_level = clone.compression_level;
	gamma = clone.gamma;
	floyd = clone.floyd;
	speed = clone.speed;
	posterization = clone.posterization;
	use_dither_map = clone.use_dither_map;
	use_contrast_maps = clone.use_contrast_maps;
	max_colors = clone.max_colors;
	min_quality = clone.min_quality;
	max_quality = clone.max_quality;
	binvalidate = clone.binvalidate;
	indexed_video = clone.indexed_video;
	nthread = clone.nthread;
	return (*this);
}

sirius::library::video::transform::codec::partial::png::compressor::compressor(void)
{
	_core = new sirius::library::video::transform::codec::partial::png::compressor::core(this);
}

sirius::library::video::transform::codec::partial::png::compressor::~compressor(void)
{
	if (_core)
		delete _core;
	_core = nullptr;
}

int32_t sirius::library::video::transform::codec::partial::png::compressor::initialize(sirius::library::video::transform::codec::partial::png::compressor::context_t * context)
{
	return _core->initialize(context);
}

int32_t sirius::library::video::transform::codec::partial::png::compressor::release(void)
{
	return _core->release();
}

int32_t sirius::library::video::transform::codec::partial::png::compressor::play(void)
{
	return _core->play();
}

int32_t sirius::library::video::transform::codec::partial::png::compressor::pause(void)
{
	return _core->pause();
}

int32_t sirius::library::video::transform::codec::partial::png::compressor::stop(void)
{
	return _core->stop();
}

int32_t sirius::library::video::transform::codec::partial::png::compressor::invalidate(void)
{
	return _core->invalidate();
}

int32_t sirius::library::video::transform::codec::partial::png::compressor::compress(sirius::library::video::transform::codec::partial::png::compressor::entity_t * input, sirius::library::video::transform::codec::partial::png::compressor::entity_t * bitstream)
{
	return _core->compress(input, bitstream);
}

int32_t sirius::library::video::transform::codec::partial::png::compressor::compress(sirius::library::video::transform::codec::partial::png::compressor::entity_t * input)
{
	return _core->compress(input);
}