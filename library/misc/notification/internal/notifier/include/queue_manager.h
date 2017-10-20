#ifndef _QUEUE_MANAGER_H_
#define _QUEUE_MANAGER_H_

#include "sirius_internal_notifier.h"
#include <queue>

namespace sirius
{
	namespace library
	{
		namespace misc
		{
			namespace notification
			{
				namespace internal
				{
					class notifier::queue_manager
					{
					public:
						queue_manager(sirius::library::misc::notification::internal::notifier::core * front);
						~queue_manager(void);

						void push(sirius::library::misc::notification::internal::notifier::notification_t noti);

					private:
						static unsigned __stdcall	process(void * param);
						void						process_queue(void);

					private:
						sirius::library::misc::notification::internal::notifier::core *						_front;
						std::queue<sirius::library::misc::notification::internal::notifier::notification_t>	_queue;
						HANDLE				_thread;
						bool				_run_thread;
						HANDLE				_queue_event;
						CRITICAL_SECTION	_cs;
					};
				};
			};
		};
	};
};

#endif