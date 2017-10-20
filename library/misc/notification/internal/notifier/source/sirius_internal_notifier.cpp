#include "internal_notifier.h"
#include "sirius_internal_notifier.h"

sirius::library::misc::notification::internal::notifier::_notification_t::_notification_t(void)
	: type(sirius::library::misc::notification::internal::notifier::type_t::unknown)
	, msg(nullptr)
	, size(0)
{}

sirius::library::misc::notification::internal::notifier::_notification_t::_notification_t(const sirius::library::misc::notification::internal::notifier::_notification_t & clone)
{
	type = clone.type;
	msg = clone.msg;
	size = clone.size;
}

sirius::library::misc::notification::internal::notifier::_notification_t sirius::library::misc::notification::internal::notifier::_notification_t::operator = (const sirius::library::misc::notification::internal::notifier::_notification_t & clone)
{
	type = clone.type;
	msg = clone.msg;
	size = clone.size;
	return *(this);
}

sirius::library::misc::notification::internal::notifier::notifier()
	: _callee(nullptr)
	, _enable(FALSE)
{
	_core = new sirius::library::misc::notification::internal::notifier::core(this);
}

sirius::library::misc::notification::internal::notifier::~notifier()
{
	if (_core)
	{
		delete _core;
		_core = NULL;
	}
}

void sirius::library::misc::notification::internal::notifier::push(int32_t type, VARIANT var)
{
	if (_core && _enable)
		_core->push(type, var);
}

void sirius::library::misc::notification::internal::notifier::send(int32_t type, char * msg, int32_t size)
{
	if(_callee)
		_callee->on_recv_notification(type, msg, size);
}

void sirius::library::misc::notification::internal::notifier::set_callee(sirius::library::misc::notification::internal::notifier::callee * callee)
{
	_callee = callee;
}

void sirius::library::misc::notification::internal::notifier::enable_notification(BOOL enable)
{
	_enable = enable;
}

BOOL sirius::library::misc::notification::internal::notifier::enable_notification(void)
{
	return _enable;
}