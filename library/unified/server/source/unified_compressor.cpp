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
			venc_ctx->compression_level = _external_venc_ctx->compression_level;
			venc_ctx->gamma = 1 / 2.2f;
			venc_ctx->floyd = 0.5f;
			venc_ctx->speed = 10;
			venc_ctx->max_colors = _external_venc_ctx->quantization_colors;
			venc_ctx->min_quality = 50;
			venc_ctx->max_quality = 80;
			//venc_ctx->fast_compression = true;
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

void sirius::library::unified::compressor::after_video_compressing_callback(uint8_t * data, size_t size, long long before_encode_timestamp, long long after_encode_timestamp)
{
	if (_front)
		_front->after_video_compressing_callback(data, size, before_encode_timestamp, after_encode_timestamp);
}

void sirius::library::unified::compressor::after_video_compressing_callback(int32_t count, int32_t * index, uint8_t ** compressed, int32_t * size, long long before_compress_timestamp, long long after_compress_timestamp)
{
	if (_front)
		_front->after_video_compressing_callback(count, index, compressed, size, before_compress_timestamp, after_compress_timestamp);
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