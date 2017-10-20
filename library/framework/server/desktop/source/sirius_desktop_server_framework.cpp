#include "sirius_desktop_server_framework.h"
#include "desktop_server_framework.h"

sirius::library::framework::server::desktop::desktop(void)
{
	_core = new (std::nothrow) sirius::library::framework::server::desktop::core();
}

sirius::library::framework::server::desktop::~desktop(void)
{
	if (_core)
		delete _core;

	_core = nullptr;
}

void sirius::library::framework::server::desktop::set_notification_callee(sirius::library::misc::notification::internal::notifier::callee * callee)
{
	_core->set_notification_callee(callee);
}

int32_t sirius::library::framework::server::desktop::open(sirius::library::framework::server::desktop::context_t * context)
{
	return _core->open(context);
}

int32_t sirius::library::framework::server::desktop::close(void)
{
	return _core->close();
}

int32_t sirius::library::framework::server::desktop::play(void)
{
	return _core->play();
}

int32_t sirius::library::framework::server::desktop::pause(void)
{
	return _core->pause();
}

int32_t sirius::library::framework::server::desktop::stop(void)
{
	return _core->stop();
}

int32_t sirius::library::framework::server::desktop::state(void) const
{
	return _core->state();
}

int32_t sirius::library::framework::server::desktop::seek(int32_t diff)
{
	return _core->seek(diff);
}

int32_t sirius::library::framework::server::desktop::seek_to(int32_t second)
{
	return _core->seek_to(second);
}

int32_t sirius::library::framework::server::desktop::seek_stop()
{
	return _core->seek_stop();
}

int32_t sirius::library::framework::server::desktop::forward(void)
{
	return _core->forward();
}

int32_t sirius::library::framework::server::desktop::backward(void)
{
	return _core->backward();
}

int32_t sirius::library::framework::server::desktop::reverse(void)
{
	return _core->reverse();
}

int32_t sirius::library::framework::server::desktop::play_toggle(void)
{
	return _core->play_toggle();
}

void sirius::library::framework::server::desktop::on_keyup(int32_t key)
{
	_core->on_keyup(key);
}

void sirius::library::framework::server::desktop::on_keydown(int32_t key)
{
	_core->on_keydown(key);
}

void sirius::library::framework::server::desktop::on_L_mouse_down(int32_t pos_x, int32_t pos_y)
{
	_core->on_L_mouse_down(pos_x, pos_y);
}

void sirius::library::framework::server::desktop::on_L_mouse_up(int32_t pos_x, int32_t pos_y)
{
	_core->on_L_mouse_up(pos_x, pos_y);
}

void sirius::library::framework::server::desktop::on_R_mouse_down(int32_t pos_x, int32_t pos_y)
{
	_core->on_R_mouse_down(pos_x, pos_y);
}

void sirius::library::framework::server::desktop::on_R_mouse_up(int32_t pos_x, int32_t pos_y)
{
	_core->on_R_mouse_up(pos_x, pos_y);
}

void sirius::library::framework::server::desktop::on_L_mouse_dclick(int32_t pos_x, int32_t pos_y)
{
	_core->on_L_mouse_dclick(pos_x, pos_y);
}

void sirius::library::framework::server::desktop::on_R_mouse_dclick(int32_t pos_x, int32_t pos_y)
{
	_core->on_R_mouse_dclick(pos_x, pos_y);
}

void sirius::library::framework::server::desktop::on_mouse_move(int32_t pos_x, int32_t pos_y)
{
	_core->on_mouse_move(pos_x, pos_y);
}

void sirius::library::framework::server::desktop::on_mouse_wheel(int32_t pos_x, int32_t pos_y, int32_t wheel_delta)
{
	_core->on_mouse_wheel(pos_x, pos_y, wheel_delta);
}

void sirius::library::framework::server::desktop::on_gyro(float x, float y, float z)
{
	_core->on_gyro(x, y, z);
}

void sirius::library::framework::server::desktop::on_pinch_zoom(float delta)
{
	_core->on_pinch_zoom(delta);
}

void sirius::library::framework::server::desktop::on_gyro_attitude(float x, float y, float z, float w)
{
	_core->on_gyro_attitude(x, y, z, w);
}

void sirius::library::framework::server::desktop::on_gyro_gravity(float x, float y, float z)
{
	_core->on_gyro_gravity(x, y, z);
}

void sirius::library::framework::server::desktop::on_gyro_rotation_rate(float x, float y, float z)
{
	_core->on_gyro_rotation_rate(x, y, z);
}

void sirius::library::framework::server::desktop::on_gyro_rotation_rate_unbiased(float x, float y, float z)
{
	_core->on_gyro_rotation_rate_unbiased(x, y, z);
}

void sirius::library::framework::server::desktop::on_gyro_user_acceleration(float x, float y, float z)
{
	_core->on_gyro_user_acceleration(x, y, z);
}

void sirius::library::framework::server::desktop::on_gyro_enabled_attitude(bool state)
{
	_core->on_gyro_enabled_attitude(state);
}

void sirius::library::framework::server::desktop::on_gyro_enabled_gravity(bool state)
{
	_core->on_gyro_enabled_gravity(state);
}

void sirius::library::framework::server::desktop::on_gyro_enabled_rotation_rate(bool state)
{
	_core->on_gyro_enabled_rotation_rate(state);
}

void sirius::library::framework::server::desktop::on_gyro_enabled_rotation_rate_unbiased(bool state)
{
	_core->on_gyro_enabled_rotation_rate_unbiased(state);
}

void sirius::library::framework::server::desktop::on_gyro_enabled_user_acceleration(bool state)
{
	_core->on_gyro_enabled_user_acceleration(state);
}

void sirius::library::framework::server::desktop::on_gyro_updateinterval(float interval)
{
	_core->on_gyro_updateinterval(interval);
}

void sirius::library::framework::server::desktop::on_infoxml(const char * msg, int32_t length)
{
	_core->on_infoxml(msg, length);
}

sirius::library::framework::server::base * create_server_framework(void)
{
	return new sirius::library::framework::server::desktop();
}

void destroy_server_framework(sirius::library::framework::server::base ** server_framework)
{
	sirius::library::framework::server::desktop * self = dynamic_cast<sirius::library::framework::server::desktop*>((*server_framework));
	delete self;
	(*server_framework) = 0;
}
