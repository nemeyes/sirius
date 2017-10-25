#include "video_source.h"
#include "web_server_framework.h"

sirius::library::framework::server::web::video_source::video_source(sirius::library::framework::server::web::core * framework)
	: _framework(framework)
{
}

sirius::library::framework::server::web::video_source::~video_source(void)
{
}

int32_t sirius::library::framework::server::web::video_source::start(int32_t fps, int32_t player_type = sirius::library::framework::server::web::attendant_type_t::web)
{
	int32_t code = sirius::library::video::source::d3d11::capturer::err_code_t::success;

	//sirius_d3d11_video_capturer.dll 로딩시  gpu_index 와 present param 을 셋팅하도록 수정 했습니다. 
//	sirius::library::video::source::d3d11::capturer::context_t::instance().gpuindex = _framework->_context.gpuindex;
//	sirius::library::video::source::d3d11::capturer::context_t::instance().present = _framework->_context.present;
	sirius::library::video::source::d3d11::capturer::context_t::instance().handler = this;
	sirius::library::video::source::d3d11::capturer::context_t::instance().fps = fps;

	LOGGER::make_trace_log(SLVSC, "%s()_%d gpu_index:%d, enablePresent:%d : ", __FUNCTION__, __LINE__, _framework->_context.gpuindex, _framework->_context.present);

	code = sirius::library::video::source::d3d11::capturer::instance().start();
	return code;
}

int32_t sirius::library::framework::server::web::video_source::stop(void)
{
	int32_t code = sirius::library::video::source::d3d11::capturer::err_code_t::success;
	code = sirius::library::video::source::d3d11::capturer::instance().stop();
	return code;
}

void sirius::library::framework::server::web::video_source::on_initialize(void * device, int32_t hwnd, int32_t smt, int32_t width, int32_t height)
{
	_framework->on_video_initialize(device, hwnd, smt, width, height);
}

void sirius::library::framework::server::web::video_source::on_process(sirius::library::video::source::capturer::entity_t * input)
{
	_framework->on_video_receive(input);
}

void sirius::library::framework::server::web::video_source::on_release(void)
{
	_framework->on_video_release();
}