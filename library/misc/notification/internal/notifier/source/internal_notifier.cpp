#include "internal_notifier.h"
#include "sirius_internal_notifier.h"
#include "queue_manager.h"
#include "json/json.h"

sirius::library::misc::notification::internal::notifier::core::core(sirius::library::misc::notification::internal::notifier * front)
	: _front(front)
{
	_queue = new sirius::library::misc::notification::internal::notifier::queue_manager(this);
}

sirius::library::misc::notification::internal::notifier::core::~core(void)
{
	if (_queue)
	{
		delete _queue;
		_queue = NULL;
	}	
}
void sirius::library::misc::notification::internal::notifier::core::push(int32_t type, VARIANT var)
{
	notification_t noti;
	noti.type = type;
	noti.msg = NULL;
	noti.size = parse(&noti.msg, var);
	_queue->push(noti);
}

void sirius::library::misc::notification::internal::notifier::core::send(notification_t noti)
{
	_front->send(noti.type, noti.msg, noti.size);
}

int32_t sirius::library::misc::notification::internal::notifier::core::parse(char ** msg, VARIANT var)
{
	Json::Value noti_packet;
	Json::StyledWriter writer;
	int32_t	size = 0;
	switch (var.vt)
	{
	case VT_I4:
		{
			noti_packet["value"] = var.intVal;
		}
		break;
	case VT_R4:
		{
			noti_packet["value"] = var.fltVal;
		}
		break;
	case VT_EMPTY:
		{
			//nothing to implement
		}
		break;
	case VT_BSTR:
		{
			noti_packet["value"] = var.bstrVal;
		}
		break;
	case VT_LPSTR:
		{
			//noti_packet["xml"] = var.pcVal;
			std::string noti_msg = var.pcVal;
			size = noti_msg.size() + 1;
			*msg = static_cast<char*>(malloc(size));
			memcpy((void*)(*msg), noti_msg.c_str(), size);
			return size;
		}
		break;
	default:
		{
			//not implemented
		}
		break;
	}
	
	if (noti_packet.isNull() == false)
	{
		std::string noti_msg = writer.write(noti_packet);
		size = noti_msg.size();
		*msg = static_cast<char*>(malloc(size + 1));
		memcpy((void*)(*msg), noti_msg.c_str(), size + 1);
	}

	return size;
}