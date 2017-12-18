#include "sirius_web_server_framework.h"
#include "web_server_framework.h"

sirius::library::framework::server::web::web(void)
{
	_core = new (std::nothrow) sirius::library::framework::server::web::core();
}

sirius::library::framework::server::web::~web(void)
{
	if (_core)
		delete _core;

	_core = nullptr;
}

void sirius::library::framework::server::web::set_notification_callee(sirius::library::misc::notification::internal::notifier::callee * callee)
{
	_core->set_notification_callee(callee);
}

int32_t sirius::library::framework::server::web::initialize(sirius::library::framework::server::web::context_t * context)
{
	return _core->initialize(context);
}

int32_t sirius::library::framework::server::web::release(void)
{
	return _core->release();
}

int32_t sirius::library::framework::server::web::play(void)
{
	return _core->play();
}

int32_t sirius::library::framework::server::web::pause(void)
{
	return _core->pause();
}

int32_t sirius::library::framework::server::web::stop(void)
{
	return _core->stop();
}

int32_t sirius::library::framework::server::web::state(void) const
{
	return _core->state();
}

void sirius::library::framework::server::web::on_keyup(int32_t key)
{
	_core->on_keyup(key);
}

void sirius::library::framework::server::web::on_keydown(int32_t key)
{
	_core->on_keydown(key);
}

void sirius::library::framework::server::web::on_L_mouse_down(int32_t pos_x, int32_t pos_y)
{
	_core->on_L_mouse_down(pos_x, pos_y);
}

void sirius::library::framework::server::web::on_L_mouse_up(int32_t pos_x, int32_t pos_y)
{
	_core->on_L_mouse_up(pos_x, pos_y);
}

void sirius::library::framework::server::web::on_R_mouse_down(int32_t pos_x, int32_t pos_y)
{
	_core->on_R_mouse_down(pos_x, pos_y);
}

void sirius::library::framework::server::web::on_R_mouse_up(int32_t pos_x, int32_t pos_y)
{
	_core->on_R_mouse_up(pos_x, pos_y);
}

void sirius::library::framework::server::web::on_L_mouse_dclick(int32_t pos_x, int32_t pos_y)
{
	_core->on_L_mouse_dclick(pos_x, pos_y);
}

void sirius::library::framework::server::web::on_R_mouse_dclick(int32_t pos_x, int32_t pos_y)
{
	_core->on_R_mouse_dclick(pos_x, pos_y);
}

void sirius::library::framework::server::web::on_mouse_move(int32_t pos_x, int32_t pos_y)
{
	_core->on_mouse_move(pos_x, pos_y);
}

void sirius::library::framework::server::web::on_mouse_wheel(int32_t pos_x, int32_t pos_y, int32_t wheel_delta)
{
	_core->on_mouse_wheel(pos_x, pos_y, wheel_delta);
}

void sirius::library::framework::server::web::on_info_xml(const uint8_t * msg, int32_t length)
{
	_core->on_info_xml(msg, length);
}

sirius::library::framework::server::base * create_server_framework(void)
{
	return new sirius::library::framework::server::web();
}

void destroy_server_framework(sirius::library::framework::server::base ** server_framework)
{
	sirius::library::framework::server::web * self = dynamic_cast<sirius::library::framework::server::web*>((*server_framework));
	delete self;
	(*server_framework) = 0;
}
