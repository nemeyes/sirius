#include "video_source.h"
#include "desktop_server_framework.h"

sirius::library::framework::server::desktop::video_source::video_source(sirius::library::framework::server::desktop::core * framework)
	: _framework(framework)
{
}

sirius::library::framework::server::desktop::video_source::~video_source(void)
{
}

int32_t sirius::library::framework::server::desktop::video_source::start(int32_t fps, int32_t player_type = sirius::library::framework::server::desktop::attendant_type_t::desktop)
{
	int32_t status = sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;

	_context.gpuindex = _framework->_context.gpuindex;
	_context.present = _framework->_context.present;
	_context.handler = this;
	_context.fps = fps;
	
	LOGGER::make_trace_log(SLVSC, "%s()_%d gpu_index:%d, enablePresent:%d : ", __FUNCTION__, __LINE__, _framework->_context.gpuindex, _framework->_context.present);
	
	status = _capturer.initialize(&_context);
	if(status == sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success)
		status = _capturer.start();

	return status;
}

int32_t sirius::library::framework::server::desktop::video_source::stop(void)
{
	int32_t status = sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;
	status = _capturer.stop();
	status = _capturer.release();
	return status;
}

void sirius::library::framework::server::desktop::video_source::on_initialize(void * device, int32_t hwnd, int32_t smt, int32_t width, int32_t height)
{
	_framework->on_video_initialize(device, hwnd, smt, width, height);
}

void sirius::library::framework::server::desktop::video_source::on_process(sirius::library::video::source::capturer::entity_t * input)
{
	_framework->on_video_receive(input);
}

void sirius::library::framework::server::desktop::video_source::on_release(void)
{
	_framework->on_video_release();
}