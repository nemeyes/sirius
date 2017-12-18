#ifndef _SIRIUS_PERFORMANCE_MONITOR_H_
#define _SIRIUS_PERFORMANCE_MONITOR_H_

#include <sirius.h>

#if defined(EXPORT_PERFORMANCE_MONITOR)
#define EXP_PERFORMANCE_MONITOR_CLASS __declspec(dllexport)
#else
#define EXP_PERFORMANCE_MONITOR_CLASS __declspec(dllimport)
#endif


namespace sirius
{
	namespace library
	{
		namespace misc
		{
			namespace performance
			{
				class EXP_PERFORMANCE_MONITOR_CLASS monitor
					: public sirius::base
				{
				public:
					class core;

				public:
					monitor(void);
					virtual ~monitor(void);

					int32_t initialize(void);
					int32_t release(void);


					char * cpu_info(void);
					char * mem_info(void);
					double total_cpu_usage(void);
					double process_cpu_usage(int32_t container_number);
					double total_mem_usage(void);

				private:
					sirius::library::misc::performance::monitor::core * _core;
				};
			};
		};
	};
};


#endif