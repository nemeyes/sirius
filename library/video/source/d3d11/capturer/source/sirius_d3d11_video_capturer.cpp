#include "sirius_d3d11_video_capturer.h"
#include "d3d11_video_capturer.h"
#include <map>
#include <sirius_log4cplus_logger.h>

sirius::library::video::source::d3d11::capturer & sirius::library::video::source::d3d11::capturer::instance(void)
{
	static sirius::library::video::source::d3d11::capturer _instance;
	return _instance;
}

sirius::library::video::source::d3d11::capturer::capturer(void)
{
	_core = new sirius::library::video::source::d3d11::capturer::core();
}

sirius::library::video::source::d3d11::capturer::~capturer(void)
{
//	LOGGER::make_info_log(SLVSC, "%s_%d uuid[%s]", __FUNCTION__, __LINE__, cap_d3d11_video_capture::CONFIGURATION_T::INSTANCE().uuid);
	//cap_log4cplus_logger::destroy();
	if (_core)
	{
		delete _core;
		_core = nullptr;
	}
}

int32_t sirius::library::video::source::d3d11::capturer::initialize(sirius::library::video::source::capturer::context_t * context)
{
//	LOGGER::make_info_log(SLVSC, "%s_%d uuid[%s]", __FUNCTION__, __LINE__, cap_d3d11_video_capture::CONFIGURATION_T::INSTANCE().uuid);
	return _core->initialize(static_cast<sirius::library::video::source::d3d11::capturer::context_t*>(context));
}

int32_t sirius::library::video::source::d3d11::capturer::release(void)
{
	//LOGGER::make_info_log(SLVSC, "%s_%d uuid[%s]", __FUNCTION__, __LINE__, sirius::library::video::source::d3d11::capturer::context_t::instance().uuid);
	return _core->release();
}

int32_t sirius::library::video::source::d3d11::capturer::start(void)
{
	//LOGGER::make_trace_log(SLVSC, "%s_%d uuid[%s] : in_fps=%d", __FUNCTION__, __LINE__, sirius::library::video::source::d3d11::capturer::context_t::instance().uuid, fps);
	return _core->start();
}

int32_t sirius::library::video::source::d3d11::capturer::stop(void)
{
	//LOGGER::make_info_log(SLVSC, "%s_%d uuid[%s]", __FUNCTION__, __LINE__, sirius::library::video::source::d3d11::capturer::context_t::instance().uuid);
	return _core->stop();
}

int32_t sirius::library::video::source::d3d11::capturer::pause(void)
{
	//LOGGER::make_info_log(SLVSC, "%s_%d uuid[%s]", __FUNCTION__, __LINE__, sirius::library::video::source::d3d11::capturer::context_t::instance().uuid);
	return _core->pause();
}
