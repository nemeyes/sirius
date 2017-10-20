#include "native_client_framework.h"
#include <sirius_stringhelper.h>
#include <sirius_locks.h>
#include <sirius_png_decompressor.h>
#include <sirius_ddraw_renderer.h>
#include <sirius_log4cplus_logger.h> 

sirius::library::framework::client::native::core::core(void)
{
	::InitializeCriticalSection(&_vcs);
	_video_buffer = static_cast<uint8_t*>(malloc(VIDEO_BUFFER_SIZE));
}

sirius::library::framework::client::native::core::~core(void)
{
	stop();
	{
		sirius::autolock mutex(&_vcs);
		if (_video_buffer)
		{
			free(_video_buffer);
			_video_buffer = nullptr;
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

void sirius::library::framework::client::native::core::on_begin_video(int32_t smt, const uint8_t * data, size_t data_size, long long dts, long long cts)
{
	sirius::autolock mutex(&_vcs);

	if (!_video_buffer)
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
		//RECT rect;
		//::GetWindowRect(_hwnd, &rect);
		//video_decoder_config->owidth = rect.right - rect.left;
		//video_decoder_config->oheight = rect.bottom - rect.top;
		//dctx->owidth = video_decoder_config->iwidth;
		//dctx->oheight = video_decoder_config->iheight;
		//dctx->codec = sirius::library::video::codec::ff::decoder::video_submedia_type_t::h264;
		//dctx->cs = sirius::library::video::codec::ff::decoder::video_submedia_type_t::rgb32;
		dctx->width = 1280;
		dctx->height = 720;

		rctx->hwnd = _hwnd;
		rctx->width = dctx->width;
		rctx->height = dctx->height;

		int32_t decode_err = decompressor->initialize(dctx);
		int32_t render_err = renderer->initialize(rctx);

		if (decode_err == sirius::library::video::transform::codec::png::decompressor::err_code_t::success)
		{
			sirius::library::video::transform::codec::png::decompressor::entity_t encoded;
			encoded.memtype = sirius::library::video::transform::codec::png::decompressor::video_memory_type_t::host;
			encoded.data = (void*)data;
			encoded.data_size = data_size;

			sirius::library::video::transform::codec::png::decompressor::entity_t decoded;
			decoded.memtype = sirius::library::video::transform::codec::png::decompressor::video_memory_type_t::host;
			decoded.data = _video_buffer;
			decoded.data_capacity = VIDEO_BUFFER_SIZE;

			decode_err = decompressor->decompress(&encoded, &decoded);
			if ((decode_err == sirius::library::video::transform::codec::png::decompressor::err_code_t::success) && (decoded.data_size > 0))
			{
				if (render_err == sirius::library::video::sink::ddraw::renderer::err_code_t::success)
				{
					sirius::library::video::sink::ddraw::renderer::entity_t render;
					render.memtype = sirius::library::video::sink::ddraw::renderer::video_memory_type_t::host;
					render.data = decoded.data;
					render.data_size = decoded.data_size;
					renderer->render(&render);
				}
			}
		}
	} while (0);
}

void sirius::library::framework::client::native::core::on_recv_video(int32_t smt, const uint8_t * data, size_t data_size, long long dts, long long cts)
{
	sirius::autolock mutex(&_vcs);

	if (!_video_buffer)
		return;

	sirius::library::video::transform::codec::png::decompressor * decompressor = static_cast<sirius::library::video::transform::codec::png::decompressor*>(_video_decompressor);
	sirius::library::video::transform::codec::png::decompressor::context_t * dctx = static_cast<sirius::library::video::transform::codec::png::decompressor::context_t*>(_video_decompressor_context);

	sirius::library::video::sink::ddraw::renderer * renderer = static_cast<sirius::library::video::sink::ddraw::renderer*>(_video_renderer);
	sirius::library::video::sink::ddraw::renderer::context_t * rctx = static_cast<sirius::library::video::sink::ddraw::renderer::context_t*>(_video_renderer_context);


	sirius::library::video::transform::codec::png::decompressor::entity_t encoded;
	encoded.memtype = sirius::library::video::transform::codec::png::decompressor::video_memory_type_t::host;
	sirius::library::video::transform::codec::png::decompressor::entity_t decoded;
	decoded.memtype = sirius::library::video::transform::codec::png::decompressor::video_memory_type_t::host;

	encoded.data = (uint8_t*)data;
	encoded.data_size = data_size;

	decoded.data = _video_buffer;
	decoded.data_capacity = VIDEO_BUFFER_SIZE;

	int32_t decode_err = decompressor->decompress(&encoded, &decoded);
	if ((decode_err == sirius::library::video::transform::codec::png::decompressor::err_code_t::success) && (decoded.data_size > 0))
	{
		sirius::library::video::sink::ddraw::renderer::entity_t render;
		render.memtype = sirius::library::video::sink::ddraw::renderer::video_memory_type_t::host;
		render.data = decoded.data;
		render.data_size = decoded.data_size;
		renderer->render(&render);
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