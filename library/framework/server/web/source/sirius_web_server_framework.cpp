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

int32_t sirius::library::framework::server::web::open(sirius::library::framework::server::web::context_t * context)
{
	return _core->open(context);
}

int32_t sirius::library::framework::server::web::close(void)
{
	return _core->close();
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

int32_t sirius::library::framework::server::web::seek(int32_t diff)
{
	return _core->seek(diff);
}

int32_t sirius::library::framework::server::web::seek_to(int32_t second)
{
	return _core->seek_to(second);
}

int32_t sirius::library::framework::server::web::seek_stop()
{
	return _core->seek_stop();
}

int32_t sirius::library::framework::server::web::forward(void)
{
	return _core->forward();
}

int32_t sirius::library::framework::server::web::backward(void)
{
	return _core->backward();
}

int32_t sirius::library::framework::server::web::reverse(void)
{
	return _core->reverse();
}

int32_t sirius::library::framework::server::web::play_toggle(void)
{
	return _core->play_toggle();
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

void sirius::library::framework::server::web::on_gyro(float x, float y, float z)
{
	_core->on_gyro(x, y, z);
}

void sirius::library::framework::server::web::on_pinch_zoom(float delta)
{
	_core->on_pinch_zoom(delta);
}

void sirius::library::framework::server::web::on_gyro_attitude(float x, float y, float z, float w)
{
	_core->on_gyro_attitude(x, y, z, w);
}

void sirius::library::framework::server::web::on_gyro_gravity(float x, float y, float z)
{
	_core->on_gyro_gravity(x, y, z);
}

void sirius::library::framework::server::web::on_gyro_rotation_rate(float x, float y, float z)
{
	_core->on_gyro_rotation_rate(x, y, z);
}

void sirius::library::framework::server::web::on_gyro_rotation_rate_unbiased(float x, float y, float z)
{
	_core->on_gyro_rotation_rate_unbiased(x, y, z);
}

void sirius::library::framework::server::web::on_gyro_user_acceleration(float x, float y, float z)
{
	_core->on_gyro_user_acceleration(x, y, z);
}

void sirius::library::framework::server::web::on_gyro_enabled_attitude(bool state)
{
	_core->on_gyro_enabled_attitude(state);
}

void sirius::library::framework::server::web::on_gyro_enabled_gravity(bool state)
{
	_core->on_gyro_enabled_gravity(state);
}

void sirius::library::framework::server::web::on_gyro_enabled_rotation_rate(bool state)
{
	_core->on_gyro_enabled_rotation_rate(state);
}

void sirius::library::framework::server::web::on_gyro_enabled_rotation_rate_unbiased(bool state)
{
	_core->on_gyro_enabled_rotation_rate_unbiased(state);
}

void sirius::library::framework::server::web::on_gyro_enabled_user_acceleration(bool state)
{
	_core->on_gyro_enabled_user_acceleration(state);
}

void sirius::library::framework::server::web::on_gyro_updateinterval(float interval)
{
	_core->on_gyro_updateinterval(interval);
}

void sirius::library::framework::server::web::on_infoxml(const char * msg, int32_t length)
{
	_core->on_infoxml(msg, length);
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
