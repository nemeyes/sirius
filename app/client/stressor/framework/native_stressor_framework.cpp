#include "native_stressor_framework.h"
#include <sirius_stringhelper.h>
#include <sirius_locks.h>
#include <sirius_log4cplus_logger.h>

sirius::library::framework::stressor::native::core::core(sirius::library::framework::stressor::native * front) :
	_front(front)
{
	::InitializeCriticalSection(&_vcs);	
}

sirius::library::framework::stressor::native::core::~core(void)
{	
	on_end_video();
	::DeleteCriticalSection(&_vcs);
}

int32_t sirius::library::framework::stressor::native::core::play(HWND hwnd)
{
	_hwnd = hwnd;
	return sirius::library::unified::client::play();
}

int32_t sirius::library::framework::stressor::native::core::stop(void)
{
	return sirius::library::unified::client::stop();
}

void sirius::library::framework::stressor::native::core::on_begin_video(int32_t codec, int32_t width, int32_t height, int32_t block_width, int32_t block_height)
{
	sirius::autolock mutex(&_vcs);	
	if (_front)
		_front->stream_connect_callback();
}

void sirius::library::framework::stressor::native::core::on_recv_video(int32_t codec, const uint8_t * data, int32_t length, long long dts, long long cts)
{
	sirius::autolock mutex(&_vcs);	
}

void sirius::library::framework::stressor::native::core::on_recv_video(int32_t codec, int32_t count, int32_t * index, uint8_t ** data, int32_t * length, long long dts, long long cts)
{
	sirius::autolock mutex(&_vcs);
}

void sirius::library::framework::stressor::native::core::on_end_video(void)
{
	sirius::autolock mutex(&_vcs);	
	if (_front)
		_front->stream_disconnect_callback();
}