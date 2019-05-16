#include "unified_compressor.h"
#include <atlbase.h>
#include <dxgi1_3.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <rpcdce.h>
#include <typeinfo>
#include <sirius_locks.h>
#include <sirius_stringhelper.h>

#include "unified_server.h"
#include "partial_png_compressor_wrapper.h"
#include "partial_webp_compressor_wrapper.h"

sirius::library::unified::compressor::compressor(sirius::library::unified::server::core * front)
	: _front(front)
	, _external_venc_ctx(nullptr)
	, _venc_ctx(nullptr)
	, _venc(nullptr)
	, _play_flag(0)
{
	InitializeSRWLock(&_venc_lock);
}

sirius::library::unified::compressor::~compressor(void)
{

}

int32_t sirius::library::unified::compressor::initialize_video_compressor(sirius::library::unified::server::video_compressor_context_t * context)
{
	int32_t status = sirius::library::unified::server::err_code_t::success;
	if (_venc || !context)
		return sirius::library::unified::server::err_code_t::fail;

	sirius::exclusive_scopedlock mutex(&_venc_lock);

	_external_venc_ctx = context;

	if (_external_venc_ctx->gpuindex < 0)
		_external_venc_ctx->gpuindex = 0;

	switch (_external_venc_ctx->codec)
	{
		case sirius::library::unified::server::video_submedia_type_t::png :
		{
			_venc_ctx = new  sirius::library::unified::partialpng::compressor::context_t();
			_venc = new sirius::library::unified::partialpng::compressor(this);

			sirius::library::unified::partialpng::compressor::context_t * venc_ctx = static_cast<sirius::library::unified::partialpng::compressor::context_t*>(_venc_ctx);
			sirius::library::unified::partialpng::compressor * venc = static_cast<sirius::library::unified::partialpng::compressor*>(_venc);

			venc_ctx->memtype = _external_venc_ctx->memtype;
			venc_ctx->device = _external_venc_ctx->device;
			venc_ctx->width = _external_venc_ctx->width;
			venc_ctx->height = _external_venc_ctx->height;
			venc_ctx->nbuffer = _external_venc_ctx->nbuffer;
			venc_ctx->block_width = _external_venc_ctx->block_width;
			venc_ctx->block_height = _external_venc_ctx->block_height;
			venc_ctx->binvalidate = _external_venc_ctx->invalidate4client;
			venc_ctx->indexed_video = _external_venc_ctx->indexed_mode;
			venc_ctx->nthread = _external_venc_ctx->nthread;
			venc_ctx->compression_level = _external_venc_ctx->png.compression_level;
			venc_ctx->posterization = _external_venc_ctx->png.quantization_posterization;
			venc_ctx->use_dither_map = _external_venc_ctx->png.quantization_dither_map;
			venc_ctx->use_contrast_maps = _external_venc_ctx->png.quantization_contrast_maps;
			venc_ctx->gamma = 0;// 1 / 2.2f;
			venc_ctx->floyd = 0.f; //0.5f
			venc_ctx->speed = 10;
			venc_ctx->max_colors = _external_venc_ctx->png.quantization_colors;
			venc_ctx->min_quality = 0;
			venc_ctx->max_quality = 100;

			venc_ctx->localcache = _external_venc_ctx->localcache;
			venc_ctx->localcache_legacy = _external_venc_ctx->localcache_legacy;
			venc_ctx->localcache_legacy_expire_time = _external_venc_ctx->localcache_legacy_expire_time;
			venc_ctx->localcache_portnumber = _external_venc_ctx->localcache_portnumber;
			wcsncpy_s(venc_ctx->localcache_path, _external_venc_ctx->localcache_path, MAX_PATH);

			venc->initialize(venc_ctx);
			if (_external_venc_ctx->play_after_init)
			{
				venc->play();
			}
			else
			{
				if (_play_flag & sirius::library::unified::server::media_type_t::video)
					venc->play();
			}
			break;
		}
		case sirius::library::unified::server::video_submedia_type_t::webp :
		{
			_venc_ctx = new  sirius::library::unified::partialwebp::compressor::context_t();
			_venc = new sirius::library::unified::partialwebp::compressor(this);

			sirius::library::unified::partialwebp::compressor::context_t * venc_ctx = static_cast<sirius::library::unified::partialwebp::compressor::context_t*>(_venc_ctx);
			sirius::library::unified::partialwebp::compressor * venc = static_cast<sirius::library::unified::partialwebp::compressor*>(_venc);

			venc_ctx->memtype = _external_venc_ctx->memtype;
			venc_ctx->device = _external_venc_ctx->device;
			venc_ctx->width = _external_venc_ctx->width;
			venc_ctx->height = _external_venc_ctx->height;
			venc_ctx->nbuffer = _external_venc_ctx->nbuffer;
			venc_ctx->block_width = _external_venc_ctx->block_width;
			venc_ctx->block_height = _external_venc_ctx->block_height;
			venc_ctx->binvalidate = _external_venc_ctx->invalidate4client;
			venc_ctx->indexed_video = _external_venc_ctx->indexed_mode;
			venc_ctx->nthread = _external_venc_ctx->nthread;
			venc_ctx->quality = _external_venc_ctx->webp.quality;
			venc_ctx->method = _external_venc_ctx->webp.method;
			venc->initialize(venc_ctx);
			if (_external_venc_ctx->play_after_init)
			{
				venc->play();
			}
			else
			{
				if (_play_flag & sirius::library::unified::server::media_type_t::video)
					venc->play();
			}
			break;
		}
	}
	return status;
}

int32_t sirius::library::unified::compressor::release_video_compressor(void)
{
	int32_t status = sirius::library::unified::server::err_code_t::success;

	sirius::exclusive_scopedlock mutex(&_venc_lock);

	if (_venc)
	{
		status = _venc->release();
		delete _venc;
		_venc = nullptr;
	}
	if (_venc_ctx)
	{
		delete _venc_ctx;
		_venc_ctx = nullptr;
	}
	return status;
}

int32_t sirius::library::unified::compressor::compress(sirius::library::video::transform::codec::compressor::entity_t * input)
{
	int32_t status = sirius::library::unified::server::err_code_t::success;
	if (!_venc)
		return sirius::library::unified::server::err_code_t::fail;

	status = _venc->compress(input);
	return status;
}

void sirius::library::unified::compressor::after_video_compressing_callback(int32_t count, int32_t * index, uint8_t ** compressed, int32_t * size, long long before_compress_timestamp, long long after_compress_timestamp)
{
	if (_front)
		_front->after_video_compressing_callback(count, index, compressed, size, before_compress_timestamp, after_compress_timestamp);
}

void sirius::library::unified::compressor::after_video_compressing_callback(int32_t index, uint8_t * compressed, int32_t size, long long before_compress_timestamp, long long after_compress_timestamp)
{
	if (_front)
		_front->after_video_compressing_callback(index, compressed, size, before_compress_timestamp, after_compress_timestamp);
}

void sirius::library::unified::compressor::after_video_compressing_callback(int32_t count, int16_t * x, int16_t * y, int16_t * width, int16_t * height, uint8_t ** compressed, int32_t * size, long long before_compress_timestamp, long long after_compress_timestamp)
{
	if (_front)
		_front->after_video_compressing_callback(count, x, y, width, height, compressed, size, before_compress_timestamp, after_compress_timestamp);
}

void sirius::library::unified::compressor::after_video_compressing_callback(int16_t x, int16_t y, int16_t width, int16_t height, uint8_t * compressed, int32_t size, long long before_compress_timestamp, long long after_compress_timestamp)
{
	if (_front)
		_front->after_video_compressing_callback(x, y, width, height, compressed, size, before_compress_timestamp, after_compress_timestamp);
}

int32_t sirius::library::unified::compressor::play(int32_t flag)
{
	int32_t status = sirius::library::unified::server::err_code_t::success;
	//if (flag & sirius::library::unified::server::media_type_t::video)
	{
		sirius::exclusive_scopedlock mutex(&_venc_lock);

		if (_venc)
			status |= _venc->play();
		else
			_play_flag |= (flag & sirius::library::unified::server::media_type_t::video) ? sirius::library::unified::server::media_type_t::video : 0;
	}

	return status;
}

int32_t sirius::library::unified::compressor::pause(int32_t flag)
{
	int32_t status = sirius::library::unified::server::err_code_t::success;
	//if (flag & sirius::library::unified::server::media_type_t::video)
	{
		sirius::exclusive_scopedlock mutex(&_venc_lock);

		if (_venc)
			status |= _venc->pause();
	}

	return status;
}

int32_t sirius::library::unified::compressor::stop(int32_t flag)
{
	int32_t status = sirius::library::unified::server::err_code_t::success;
	//if (flag & sirius::library::unified::server::media_type_t::video)
	{
		sirius::exclusive_scopedlock mutex(&_venc_lock);

		if (_venc)
			status |= _venc->stop();
	}

	return status;
}

int32_t	sirius::library::unified::compressor::invalidate(void)
{
	int32_t status = sirius::library::unified::server::err_code_t::success;
	//if (flag & elastics::app::static_view::lib::unified::server::media_type_t::video)
	{
		sirius::exclusive_scopedlock mutex(&_venc_lock);

		if (_venc)
			status |= _venc->invalidate();
	}

	return status;
}