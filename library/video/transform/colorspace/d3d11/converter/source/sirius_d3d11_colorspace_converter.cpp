#include "sirius_d3d11_colorspace_converter.h"
#include "d3d11_colorspace_converter.h"

sirius::library::video::transform::colorspace::d3d11::converter::_context_t::_context_t(void)
	: device(nullptr)
	, device_context(nullptr)
{

}

sirius::library::video::transform::colorspace::d3d11::converter::_context_t::_context_t(const sirius::library::video::transform::colorspace::d3d11::converter::_context_t & clone)
{
	device = clone.device;
	device_context = clone.device_context;
}

sirius::library::video::transform::colorspace::d3d11::converter::_context_t & sirius::library::video::transform::colorspace::d3d11::converter::_context_t::operator=(const sirius::library::video::transform::colorspace::d3d11::converter::_context_t & clone)
{
	device = clone.device;
	device_context = clone.device_context;
	return (*this);
}

sirius::library::video::transform::colorspace::d3d11::converter::converter(void)
{
	_core = new sirius::library::video::transform::colorspace::d3d11::converter::core();
}

sirius::library::video::transform::colorspace::d3d11::converter::~converter(void)
{
	if (_core)
		delete _core;
	_core = nullptr;
}

int32_t sirius::library::video::transform::colorspace::d3d11::converter::initialize(void * context)
{
	return _core->initialize(static_cast<sirius::library::video::transform::colorspace::d3d11::converter::context_t*>(context));
}

int32_t sirius::library::video::transform::colorspace::d3d11::converter::release(void)
{
	return _core->release();
}

int32_t sirius::library::video::transform::colorspace::d3d11::converter::convert(sirius::library::video::transform::colorspace::d3d11::converter::entity_t * input, sirius::library::video::transform::colorspace::d3d11::converter::entity_t * output)
{
	return _core->convert(input, output);
}
