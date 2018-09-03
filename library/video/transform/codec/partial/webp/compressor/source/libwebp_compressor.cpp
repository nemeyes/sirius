#include "sirius_partial_webp_compressor.h"
#include "libwebp_compressor.h"
#include <sirius_image_creator.h>
#include <sirius_stringhelper.h>

#include <webp/encode.h>

sirius::library::video::transform::codec::libwebp::compressor::compressor(sirius::library::video::transform::codec::partial::webp::compressor * front)
	: _front(front)
	, _context(nullptr)
	, _state(sirius::library::video::transform::codec::partial::webp::compressor::state_t::none)
{

}

sirius::library::video::transform::codec::libwebp::compressor::~compressor(void)
{
	_state = sirius::library::video::transform::codec::partial::webp::compressor::state_t::none;
}

int32_t sirius::library::video::transform::codec::libwebp::compressor::state(void)
{
	return _state;
}

int32_t sirius::library::video::transform::codec::libwebp::compressor::initialize(sirius::library::video::transform::codec::partial::webp::compressor::context_t * context)
{
	if (!context)
		return sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::fail;

	_state = sirius::library::video::transform::codec::partial::webp::compressor::state_t::initializing;

	_context = context;

	_state = sirius::library::video::transform::codec::partial::webp::compressor::state_t::initialized;
	return sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::libwebp::compressor::release(void)
{
	if (!((_state == sirius::library::video::transform::codec::partial::webp::compressor::state_t::initialized) ||
		(_state == sirius::library::video::transform::codec::partial::webp::compressor::state_t::compressing) ||
		(_state == sirius::library::video::transform::codec::partial::webp::compressor::state_t::compressed)))
		return sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::fail;

	_state = sirius::library::video::transform::codec::partial::webp::compressor::state_t::releasing;


	_state = sirius::library::video::transform::codec::partial::webp::compressor::state_t::released;
	return sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::libwebp::compressor::play(void)
{
	return sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::libwebp::compressor::pause(void)
{
	return sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::libwebp::compressor::stop(void)
{
	return sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::libwebp::compressor::compress(sirius::library::video::transform::codec::partial::webp::compressor::entity_t * input, sirius::library::video::transform::codec::partial::webp::compressor::entity_t * bitstream)
{
	int32_t status = sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success;

	if (_state == sirius::library::video::transform::codec::partial::webp::compressor::state_t::compressing)
		return sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success;

	_state = sirius::library::video::transform::codec::partial::webp::compressor::state_t::compressing;

#if 0
	WebPConfig config;
	if (!WebPConfigPreset(&config, WEBP_PRESET_PHOTO, _context->quality))
		return sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::fail;   // version error

	config.lossless = 0;
	config.quality = _context->quality;
	config.method = 0;
	config.target_PSNR = 0;
	config.segments = 1;
	config.sns_strength = 0;
	config.filter_strength = 0;
	config.filter_sharpness = 0;
	config.filter_type = 0;
	config.autofilter = 0;
	config.alpha_compression = 0;
	config.alpha_filtering = 1;
	config.pass = 1;
	config.preprocessing = 0;
	config.partitions = 0;
	config.partition_limit = 0;
	config.use_sharp_yuv = 0;
	config.alpha_quality = _context->quality;
	WebPValidateConfig(&config);  // will verify parameter ranges (always a good habit)

	WebPPicture pic;
	if (!WebPPictureInit(&pic)) 
		sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::fail;
	pic.use_argb = 1;
	pic.argb = (uint32_t*)(input->data);
	pic.argb_stride = input->stride;
	pic.width = input->width;
	pic.height = input->height;
	//if (!WebPPictureAlloc(&pic)) return 0;   // memory error

#else
	bitstream->data_size = (int32_t)WebPEncodeBGRA((const uint8_t*)input->data, input->width, input->height, input->stride, _context->quality, (uint8_t**)(&bitstream->data));
#endif
	_state = sirius::library::video::transform::codec::partial::webp::compressor::state_t::compressed;
	return sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success;
}