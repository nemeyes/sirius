#include "host_video_source.h"
#include "web_server_framework.h"

sirius::library::framework::server::web::host_video_source::host_video_source(sirius::library::framework::server::web::core * framework)
	: _framework(framework)
{
}

sirius::library::framework::server::web::host_video_source::~host_video_source(void)
{
}

int32_t sirius::library::framework::server::web::host_video_source::start(int32_t fps, int32_t player_type)
{
	int32_t code = sirius::library::video::source::cpu::capturer::err_code_t::success;


	sirius::library::video::source::cpu::capturer::context_t::instance().present = _framework->_context.present;
	sirius::library::video::source::cpu::capturer::context_t::instance().handler = this;
	sirius::library::video::source::cpu::capturer::context_t::instance().width = _framework->_context.video_width;
	sirius::library::video::source::cpu::capturer::context_t::instance().height = _framework->_context.video_height;
	sirius::library::video::source::cpu::capturer::context_t::instance().fps = fps;

	code = sirius::library::video::source::cpu::capturer::instance().initialize(&sirius::library::video::source::cpu::capturer::context_t::instance());
	if (code == sirius::library::video::source::cpu::capturer::err_code_t::success)
		code = sirius::library::video::source::cpu::capturer::instance().start();

	return code;
}

int32_t sirius::library::framework::server::web::host_video_source::stop(void)
{
	int32_t code = sirius::library::video::source::cpu::capturer::err_code_t::success;
	code = sirius::library::video::source::cpu::capturer::instance().stop();
	if (code == sirius::library::video::source::cpu::capturer::err_code_t::success)
		code = sirius::library::video::source::cpu::capturer::instance().release();
	return code;
}

void sirius::library::framework::server::web::host_video_source::on_initialize(void * device, int32_t hwnd, int32_t smt, int32_t width, int32_t height)
{
	_framework->on_video_initialize(device, hwnd, smt, width, height);
}

void sirius::library::framework::server::web::host_video_source::on_process(sirius::library::video::source::capturer::entity_t * input)
{
	_framework->on_video_receive(input);
}

void sirius::library::framework::server::web::host_video_source::on_release(void)
{
	_framework->on_video_release();
}