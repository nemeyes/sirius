#ifndef _SIRIUS_INTERNAL_NOTIFIER_H_
#define _SIRIUS_INTERNAL_NOTIFIER_H_

#if defined(EXPORT_INTERNAL_NOTIFIER_LIB)
#define EXP_INTERNAL_NOTIFIER_CLASS __declspec(dllexport)
#else
#define EXP_INTERNAL_NOTIFIER_CLASS __declspec(dllimport)
#endif

#include <cstdint>
#include <oaidl.h>

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
					class EXP_INTERNAL_NOTIFIER_CLASS notifier
					{
					public:
						class core;
						class queue_manager;

					public:
						typedef struct EXP_INTERNAL_NOTIFIER_CLASS _type_t
						{
							static const int32_t unknown = -1;
							static const int32_t presentation_end = 0;
							static const int32_t total_time = 1;
							static const int32_t current_time = 2;
							static const int32_t current_rate = 3;

							static const int32_t gyro_enabled_attitude = 4;
							static const int32_t gyro_enabled_gravity = 5;
							static const int32_t gyro_enabled_rotation_rate = 6;
							static const int32_t gyro_enabled_rotation_rate_unbiased = 7;
							static const int32_t gyro_enabled_user_acceleration = 8;
							static const int32_t gyro_interval = 9;

							static const int32_t info_xml = 10;
							static const int32_t info_json = 11;

							static const int32_t error = 10000;
						} type_t;

						typedef struct EXP_INTERNAL_NOTIFIER_CLASS _notification_t
						{
							int32_t type;
							char *	msg;
							int32_t	size;
							_notification_t(void);
							_notification_t(const sirius::library::misc::notification::internal::notifier::_notification_t & clone);
							sirius::library::misc::notification::internal::notifier::_notification_t operator = (const sirius::library::misc::notification::internal::notifier::_notification_t & clone);
						} notification_t;

						class EXP_INTERNAL_NOTIFIER_CLASS callee
						{
						public:
							callee(void) {}
							virtual ~callee(void) {}
							virtual void on_recv_notification(int32_t type, char* msg, int32_t size) = 0;
						};

					public:
						notifier(void);
						~notifier(void);

						void push(int32_t type, VARIANT var);
						void send(int32_t type, char* msg, int32_t size);
						void set_callee(sirius::library::misc::notification::internal::notifier::callee * callee);
						void enable_notification(BOOL enable);
						BOOL enable_notification(void);
					private:
						sirius::library::misc::notification::internal::notifier::callee * _callee;
						sirius::library::misc::notification::internal::notifier::core * _core;
						BOOL _enable;

					};
				};
			};
		};
	};
};

#endif