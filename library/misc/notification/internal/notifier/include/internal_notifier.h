#ifndef _INTERNAL_NOTIFIER_H_
#define _INTERNAL_NOTIFIER_H_

#include "sirius_internal_notifier.h"

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
					class notifier::core
					{
					public:
						core(sirius::library::misc::notification::internal::notifier * front);
						~core();

						void push(int32_t type, VARIANT var);
						void send(sirius::library::misc::notification::internal::notifier::notification_t noti);

					private:
						int32_t parse(char** msg, VARIANT var);

						sirius::library::misc::notification::internal::notifier * _front;
						sirius::library::misc::notification::internal::notifier::queue_manager * _queue;
					};

				};
			};
		};
	};
};


#endif