#include "sirius_d3d11_desktop_capturer.h"
#include "d3d11_desktop_capturer.h"

sirius::library::video::source::d3d11::desktop::capturer::capturer(void)
{
	_core = new sirius::library::video::source::d3d11::desktop::capturer::core();
}

sirius::library::video::source::d3d11::desktop::capturer::~capturer(void)
{
	if (_core)
	{
		delete _core;
	}
	_core = nullptr;
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::initialize(sirius::library::video::source::d3d11::desktop::capturer::context_t * context)
{
	return _core->initialize(context);
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::release(void)
{
	return _core->release();
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::start(void)
{
	return _core->start();
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::stop(void)
{
	return _core->stop();
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::update_application_window(bool * occluded)
{
	return _core->update_application_window(occluded);
}