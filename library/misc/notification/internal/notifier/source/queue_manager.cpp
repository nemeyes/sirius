#include <windows.h>
#include <process.h>
#include "queue_manager.h"
#include "internal_notifier.h"
#include <sirius_locks.h>

sirius::library::misc::notification::internal::notifier::queue_manager::queue_manager(sirius::library::misc::notification::internal::notifier::core * front)
	: _thread(INVALID_HANDLE_VALUE)
	, _run_thread(false)
	, _front(front)
{

	::InitializeCriticalSection(&_cs);
	_queue_event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	//::SetEvent(_queue_event);


	_run_thread = true;
	unsigned int thrdaddr = 0;
	_thread = (HANDLE)::_beginthreadex(NULL, 0, process, this, 0, &thrdaddr);
	if (_thread == NULL || _thread == INVALID_HANDLE_VALUE)
		return;
}

sirius::library::misc::notification::internal::notifier::queue_manager::~queue_manager(void)
{
	if (_thread != NULL && _thread != INVALID_HANDLE_VALUE)
	{
		_run_thread = false;
		::SetEvent(_queue_event);
		if (::WaitForSingleObject(_thread, INFINITE) == WAIT_OBJECT_0)
		{
			::CloseHandle(_thread);
			_thread = INVALID_HANDLE_VALUE;
		}
	}

	while (_queue.empty() == false)
	{
		notification_t noti = _queue.front();
		if (noti.msg)
		{
			free(noti.msg);
			noti.msg = NULL;
		}
		_queue.pop();
	}

	CloseHandle(_queue_event);
	_queue_event = INVALID_HANDLE_VALUE;
	::DeleteCriticalSection(&_cs);
}

void sirius::library::misc::notification::internal::notifier::queue_manager::push(sirius::library::misc::notification::internal::notifier::notification_t noti)
{
	sirius::autolock lock(&_cs);
	_queue.push(noti);
	::SetEvent(_queue_event);
}

unsigned __stdcall sirius::library::misc::notification::internal::notifier::queue_manager::process(void * param)
{
	queue_manager * self = static_cast<queue_manager*>(param);
	self->process_queue();
	return 0;
}

void sirius::library::misc::notification::internal::notifier::queue_manager::process_queue(void)
{
	while (_run_thread)
	{
		if(::WaitForSingleObject(_queue_event, INFINITE)==WAIT_OBJECT_0)
		{
			sirius::autolock lock(&_cs);
			while (_queue.empty() == false)
			{
				notification_t noti = _queue.front();
				if (noti.msg)
				{
					if(_front)
						_front->send(noti);
					free(noti.msg);
					noti.msg = NULL;
				}
				_queue.pop();
			}
		}
	}
}