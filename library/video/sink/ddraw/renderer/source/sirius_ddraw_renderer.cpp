#include <tchar.h>
#include "sirius_ddraw_renderer.h"
#include "ddraw_renderer.h"

sirius::library::video::sink::ddraw::renderer::renderer(void)
{
	_core = new sirius::library::video::sink::ddraw::renderer::core();
}

sirius::library::video::sink::ddraw::renderer::~renderer(void)
{
	if (_core)
	{
		delete _core;
		_core = nullptr;
	}
}

int32_t sirius::library::video::sink::ddraw::renderer::initialize(void * context)
{
	return _core->initialize(static_cast<sirius::library::video::sink::ddraw::renderer::context_t*>(context));
}

int32_t sirius::library::video::sink::ddraw::renderer::release(void)
{
	return _core->release();
}

int32_t sirius::library::video::sink::ddraw::renderer::render(sirius::library::video::sink::ddraw::renderer::entity_t * p)
{
	return _core->render(p);
}