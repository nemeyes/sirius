#include "sirius_dinput_receiver.h"
#include "dinput_receiver.h"

sirius::library::user::event::dinput::receiver::receiver(void)
{
	_core = new sirius::library::user::event::dinput::receiver::core(this);
}

sirius::library::user::event::dinput::receiver::~receiver(void)
{
	if (_core)
	{
		delete _core;
		_core = nullptr;
	}
}

int32_t sirius::library::user::event::dinput::receiver::initialize(HINSTANCE inst, HWND hwnd)
{
	if(_core)
		return _core->initialize(inst, hwnd);
	return sirius::library::user::event::dinput::receiver::err_code_t::fail;
}

int32_t sirius::library::user::event::dinput::receiver::release(void)
{
	if(_core)
		return _core->release();
	return sirius::library::user::event::dinput::receiver::err_code_t::fail;
}

void sirius::library::user::event::dinput::receiver::set_using_mouse(bool value)
{
	if (_core)
		_core->set_using_mouse(value);
}

void sirius::library::user::event::dinput::receiver::set_keystroke(int32_t interval)
{
	if (_core)
		_core->set_keystroke(interval);
}



