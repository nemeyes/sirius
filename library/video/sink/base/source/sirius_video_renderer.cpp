#include <sirius_video_renderer.h>


sirius::library::video::sink::renderer::_context_t::_context_t(void)
	: width(0)
	, height(0)
	, hwnd_full(NULL)
	, hwnd(NULL)
{}

sirius::library::video::sink::renderer::_context_t::_context_t(const sirius::library::video::sink::renderer::_context_t & clone)
{
	width = clone.width;
	height = clone.height;
	hwnd_full = clone.hwnd_full;
	hwnd = clone.hwnd;
}

sirius::library::video::sink::renderer::_context_t & sirius::library::video::sink::renderer::_context_t::operator = (const sirius::library::video::sink::renderer::_context_t & clone)
{
	width = clone.width;
	height = clone.height;
	hwnd_full = clone.hwnd_full;
	hwnd = clone.hwnd;
	return (*this);
}

sirius::library::video::sink::renderer::renderer(void)
{

}

sirius::library::video::sink::renderer::~renderer(void)
{

}

int32_t sirius::library::video::sink::renderer::initialize(void * context)
{
	return sirius::library::video::sink::renderer::err_code_t::not_implemented;
}

int32_t sirius::library::video::sink::renderer::release(void)
{
	return sirius::library::video::sink::renderer::err_code_t::not_implemented;
}

int32_t sirius::library::video::sink::renderer::render(sirius::library::video::sink::renderer::entity_t * decoded)
{
	return sirius::library::video::sink::renderer::err_code_t::not_implemented;
}
