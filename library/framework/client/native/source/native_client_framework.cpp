#include "native_client_framework.h"
#include <sirius_stringhelper.h>
#include <sirius_locks.h>
#include <sirius_png_decompressor.h>
#include <sirius_ddraw_renderer.h>
#include <sirius_log4cplus_logger.h>

#include <Simd\SimdLib.h>

sirius::library::framework::client::native::core::core(void)
{
	::InitializeCriticalSection(&_vcs);
	_decoder_buffer = static_cast<uint8_t*>(malloc(VIDEO_BUFFER_SIZE));
	_render_buffer = static_cast<uint8_t*>(malloc(VIDEO_BUFFER_SIZE));
	_processing_buffer = static_cast<uint8_t*>(malloc(VIDEO_BUFFER_SIZE));
}

sirius::library::framework::client::native::core::~core(void)
{
	{
		sirius::autolock mutex(&_vcs);
		if (_processing_buffer)
		{
			free(_processing_buffer);
			_processing_buffer = nullptr;
		}

		if (_render_buffer)
		{
			free(_render_buffer);
			_render_buffer = nullptr;
		}

		if (_decoder_buffer)
		{
			free(_decoder_buffer);
			_decoder_buffer = nullptr;
		}
	}

	on_end_video();
	::DeleteCriticalSection(&_vcs);
}

int32_t sirius::library::framework::client::native::core::play(HWND hwnd)
{
	_hwnd = hwnd;
	return sirius::library::unified::client::play();
}

int32_t sirius::library::framework::client::native::core::stop(void)
{
	return sirius::library::unified::client::stop();
}

void sirius::library::framework::client::native::core::on_begin_video(int32_t codec, int32_t width, int32_t height, int32_t block_width, int32_t block_height)
{
	sirius::autolock mutex(&_vcs);

	if (!_render_buffer)
		return;

	if (!_decoder_buffer)
		return;

	if (_video_decompressor)
	{
		_video_decompressor->release();
		delete _video_decompressor;
		_video_decompressor = nullptr;
	}
	if (_video_decompressor_context)
	{
		delete _video_decompressor_context;
		_video_decompressor_context = nullptr;
	}

	if (_video_renderer)
	{
		_video_renderer->release();
		delete _video_renderer;
		_video_renderer = nullptr;
	}
	if (_video_renderer_context)
	{
		delete _video_renderer_context;
		_video_renderer_context = nullptr;
	}

	_video_decompressor = new sirius::library::video::transform::codec::png::decompressor();
	_video_decompressor_context = new sirius::library::video::transform::codec::png::decompressor::context_t();
	_video_renderer = new sirius::library::video::sink::ddraw::renderer();
	_video_renderer_context = new sirius::library::video::sink::ddraw::renderer::context_t();

	sirius::library::video::transform::codec::png::decompressor * decompressor = static_cast<sirius::library::video::transform::codec::png::decompressor*>(_video_decompressor);
	sirius::library::video::transform::codec::png::decompressor::context_t * dctx = static_cast<sirius::library::video::transform::codec::png::decompressor::context_t*>(_video_decompressor_context);

	sirius::library::video::sink::ddraw::renderer * renderer = static_cast<sirius::library::video::sink::ddraw::renderer*>(_video_renderer);
	sirius::library::video::sink::ddraw::renderer::context_t * rctx = static_cast<sirius::library::video::sink::ddraw::renderer::context_t*>(_video_renderer_context);


	do
	{
		if (block_width != -1 && block_height != -1)
		{
			dctx->width = block_width;
			dctx->height = block_height;
			rctx->hwnd = _hwnd;
			rctx->width = width;
			rctx->height = height;
		}
		else
		{
			dctx->width = width;
			dctx->height = height;
			rctx->hwnd = _hwnd;
			rctx->width = width;
			rctx->height = height;
		}

		int32_t decode_err = decompressor->initialize(dctx);
		int32_t render_err = renderer->initialize(rctx);

	} while (0);
}

void sirius::library::framework::client::native::core::on_recv_video(int32_t codec, const uint8_t * data, int32_t length, long long dts, long long cts)
{
	sirius::autolock mutex(&_vcs);

	if (!_render_buffer)
		return;

	if (!_decoder_buffer)
		return;

	sirius::library::video::transform::codec::png::decompressor * decompressor = static_cast<sirius::library::video::transform::codec::png::decompressor*>(_video_decompressor);
	sirius::library::video::transform::codec::png::decompressor::context_t * dctx = static_cast<sirius::library::video::transform::codec::png::decompressor::context_t*>(_video_decompressor_context);

	sirius::library::video::sink::ddraw::renderer * renderer = static_cast<sirius::library::video::sink::ddraw::renderer*>(_video_renderer);
	sirius::library::video::sink::ddraw::renderer::context_t * rctx = static_cast<sirius::library::video::sink::ddraw::renderer::context_t*>(_video_renderer_context);


	sirius::library::video::transform::codec::png::decompressor::entity_t encoded;
	sirius::library::video::transform::codec::png::decompressor::entity_t decoded;

	encoded.data = (uint8_t*)data;
	encoded.data_size = length;

	decoded.data = _decoder_buffer;
	decoded.data_capacity = VIDEO_BUFFER_SIZE;

	int32_t decode_err = decompressor->decompress(&encoded, &decoded);
	if ((decode_err == sirius::library::video::transform::codec::png::decompressor::err_code_t::success) && (decoded.data_size > 0))
	{
		sirius::library::video::sink::ddraw::renderer::entity_t render;
		render.data = decoded.data;
		render.data_size = decoded.data_size;
		renderer->render(&render);
	}
}

void sirius::library::framework::client::native::core::on_recv_video(int32_t codec, int32_t count, int32_t * index, uint8_t ** data, int32_t * length, long long dts, long long cts)
{
	sirius::autolock mutex(&_vcs);

	if (!_render_buffer)
		return;

	if (!_decoder_buffer)
		return;

	sirius::library::video::transform::codec::png::decompressor * decompressor = static_cast<sirius::library::video::transform::codec::png::decompressor*>(_video_decompressor);
	sirius::library::video::transform::codec::png::decompressor::context_t * dctx = static_cast<sirius::library::video::transform::codec::png::decompressor::context_t*>(_video_decompressor_context);

	sirius::library::video::sink::ddraw::renderer * renderer = static_cast<sirius::library::video::sink::ddraw::renderer*>(_video_renderer);
	sirius::library::video::sink::ddraw::renderer::context_t * rctx = static_cast<sirius::library::video::sink::ddraw::renderer::context_t*>(_video_renderer_context);


	sirius::library::video::transform::codec::png::decompressor::entity_t encoded;
	sirius::library::video::transform::codec::png::decompressor::entity_t decoded;

	//SimdBgraToGray(_render_buffer, rctx->width, rctx->height, rctx->width << 2, _processing_buffer, rctx->width);
	//SimdGrayToBgra(_processing_buffer, rctx->width, rctx->height, rctx->width, _render_buffer, rctx->width << 2, 0);
	//memset(_render_buffer, 0x00, rctx->width * rctx->height * 4);

	for (int32_t x = 0; x < count; x++)
	{
		encoded.data = data[x];
		encoded.data_size = length[x];

		decoded.data = _decoder_buffer;
		decoded.data_capacity = VIDEO_BUFFER_SIZE;

		int32_t decode_err = decompressor->decompress(&encoded, &decoded);
		if ((decode_err == sirius::library::video::transform::codec::png::decompressor::err_code_t::success) && (decoded.data_size > 0))
		{
			int32_t hblock_cnt = rctx->height / dctx->height;
			int32_t wblock_cnt = rctx->width / dctx->width;
			int32_t h = index[x] / wblock_cnt;
			int32_t w = index[x] % wblock_cnt;

			h = h * dctx->height;
			w = w * dctx->width;

			for (int32_t bh = 0; bh < dctx->height; bh++)
			{
				int32_t src_index = bh * (dctx->width << 2);
				int32_t dst_index = (h + bh) * (rctx->width << 2) + (w << 2);
				memmove(_render_buffer + dst_index, _decoder_buffer + src_index, (dctx->width << 2));
			}
		}

		if (x == (count - 1))
		{
			sirius::library::video::sink::ddraw::renderer::entity_t render;
			render.data = _render_buffer;
			render.data_size = rctx->height * (rctx->width << 2);
			renderer->render(&render);
		}
	}
}

void sirius::library::framework::client::native::core::on_recv_video(int32_t codec, int32_t count, int16_t * x, int16_t * y, int16_t * width, int16_t * height, uint8_t ** data, int32_t * length, long long dts, long long cts)
{
	sirius::autolock mutex(&_vcs);

	if (!_render_buffer)
		return;

	if (!_decoder_buffer)
		return;

	sirius::library::video::transform::codec::png::decompressor * decompressor = static_cast<sirius::library::video::transform::codec::png::decompressor*>(_video_decompressor);
	sirius::library::video::transform::codec::png::decompressor::context_t * dctx = static_cast<sirius::library::video::transform::codec::png::decompressor::context_t*>(_video_decompressor_context);

	sirius::library::video::sink::ddraw::renderer * renderer = static_cast<sirius::library::video::sink::ddraw::renderer*>(_video_renderer);
	sirius::library::video::sink::ddraw::renderer::context_t * rctx = static_cast<sirius::library::video::sink::ddraw::renderer::context_t*>(_video_renderer_context);


	sirius::library::video::transform::codec::png::decompressor::entity_t encoded;
	sirius::library::video::transform::codec::png::decompressor::entity_t decoded;

	//SimdBgraToGray(_render_buffer, rctx->width, rctx->height, rctx->width << 2, _processing_buffer, rctx->width);
	//SimdGrayToBgra(_processing_buffer, rctx->width, rctx->height, rctx->width, _render_buffer, rctx->width << 2, 0);

	for (int32_t i = 0; i < count; i++)
	{
		encoded.data = data[i];
		encoded.data_size = length[i];

		decoded.data = _decoder_buffer;
		decoded.data_capacity = VIDEO_BUFFER_SIZE;

		int32_t decode_err = decompressor->decompress(&encoded, &decoded);
		if ((decode_err == sirius::library::video::transform::codec::png::decompressor::err_code_t::success) && (decoded.data_size > 0))
		{
			int16_t video_x = x[i];
			int16_t video_y = y[i];
			int16_t video_width = width[i];
			int16_t video_height = height[i];

			for (int32_t bh = 0; bh < video_height; bh++)
			{
				int32_t src_index = bh * (video_width << 2);
				int32_t dst_index = (video_y + bh) * (rctx->width << 2) + (video_x << 2);
				memmove(_render_buffer + dst_index, _decoder_buffer + src_index, (video_width << 2));
			}
		}

		if (i == (count - 1))
		{
			sirius::library::video::sink::ddraw::renderer::entity_t render;
			render.data = _render_buffer;
			render.data_size = rctx->height * (rctx->width << 2);
			renderer->render(&render);
		}
	}
}

void sirius::library::framework::client::native::core::on_end_video(void)
{
	sirius::autolock mutex(&_vcs);

	if (_video_decompressor)
	{
		_video_decompressor->release();
		delete _video_decompressor;
		_video_decompressor = nullptr;
	}
	if (_video_decompressor_context)
	{
		delete _video_decompressor_context;
		_video_decompressor_context = nullptr;
	}

	if (_video_renderer)
	{
		_video_renderer->release();
		delete _video_renderer;
		_video_renderer = nullptr;
	}
	if (_video_renderer_context)
	{
		delete _video_renderer_context;
		_video_renderer_context = nullptr;
	}
}