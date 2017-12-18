#ifndef _PEROFRMANCE_MONITOR_H_
#define _PERFORMANCE_MONITOR_H_

#include "sirius_performance_monitor.h"

#include <pdh.h>
#include <pdhmsg.h>
#include <vector>

#if defined(WIN32) || defined(WIN64)

namespace sirius
{
	namespace library
	{
		namespace misc
		{
			namespace performance
			{
				class monitor::core
				{
				public:
					typedef struct _info_t
					{
						int32_t			process_id;
						double			cpu_usage;
						ULARGE_INTEGER	last_cpu;
						ULARGE_INTEGER	last_sys_cpu;
						ULARGE_INTEGER	last_user_cpu;

					} info_t;

				public:
					core(sirius::library::misc::performance::monitor * front);
					virtual ~core(void);

					int32_t initialize(void);
					int32_t release(void);

					char * cpu_info(void);
					char * mem_info(void);
					double total_cpu_usage(void);
					double process_cpu_usage(int32_t container_number);
					double total_mem_usage(void);

				private:
					static unsigned __stdcall process_cb(void * param);
					void process(void);

				private:
					sirius::library::misc::performance::monitor * _front;
					HANDLE			_thread;
					bool			_run;
					char	_cpu_info[MAX_PATH];
					char	_mem_info[MAX_PATH];

					PDH_HQUERY      _cpu_query;
					PDH_HCOUNTER    _cpu_total;

					std::vector<sirius::library::misc::performance::monitor::core::info_t> _cpu_stats;
				};
			};
		};
	};
};

#endif

#endif