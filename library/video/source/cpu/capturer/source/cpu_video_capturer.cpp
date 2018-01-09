#include "cpu_video_capturer.h"
#include <process.h>
#include <tlhelp32.h>

#include <sirius_log4cplus_logger.h>

sirius::library::video::source::cpu::capturer::core::core(void)
	: _brecv(false)
	, _buffer(nullptr)
	, _buffer_size(0)
	, _context(nullptr)
{
}

sirius::library::video::source::cpu::capturer::core::~core(void)
{

}

int32_t sirius::library::video::source::cpu::capturer::core::initialize(sirius::library::video::source::cpu::capturer::context_t * context)
{
	_context = context;
	_buffer_size = context->width * context->height * 4;
	_buffer = static_cast<uint8_t*>(malloc(_buffer_size));
	memset(_buffer, 0x00, _buffer_size);
	return sirius::library::video::source::cpu::capturer::err_code_t::success;
}

int32_t sirius::library::video::source::cpu::capturer::core::release(void)
{
	if (sirius::library::video::source::cpu::capturer::context_t::instance().handler)
		sirius::library::video::source::cpu::capturer::context_t::instance().handler->on_release();

	if (_buffer)
	{
		free(_buffer);
		_buffer = nullptr;
	}
	_buffer_size = 0;
	_context = nullptr;
	return sirius::library::video::source::cpu::capturer::err_code_t::success;
}

int32_t sirius::library::video::source::cpu::capturer::core::start(void)
{
	return sirius::library::video::source::cpu::capturer::err_code_t::success;
}

int32_t sirius::library::video::source::cpu::capturer::core::stop(void)
{
	return sirius::library::video::source::cpu::capturer::err_code_t::success;
}

int32_t sirius::library::video::source::cpu::capturer::core::pause(void)
{
	return sirius::library::video::source::cpu::capturer::err_code_t::success;
}

int32_t sirius::library::video::source::cpu::capturer::core::post(int32_t smt, uint8_t * video, int32_t width, int32_t height)
{
	if (sirius::library::video::source::cpu::capturer::context_t::instance().handler && _context && _context->width == width && _context->height == height)
	{
		if (!_brecv)
		{
			sirius::library::video::source::cpu::capturer::context_t::instance().handler->on_initialize(NULL, NULL, smt, width, height);
			_brecv = true;
		}
		sirius::library::video::source::cpu::capturer::entity_t input;
		input.data = _buffer;
		input.data_size = (width * height) << 2;
		input.data_capacity = _buffer_size;
		memmove(input.data, video, input.data_size);
		sirius::library::video::source::cpu::capturer::context_t::instance().handler->on_process(&input);

		::OutputDebugStringA("post\n");
	}

	return sirius::library::video::source::cpu::capturer::err_code_t::success;
}