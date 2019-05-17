#ifndef _LOCALCACHE_BASE_H_
#define _LOCALCACHE_BASE_H_

#include <localcache_commands.h>

namespace sirius
{
	namespace library
	{
		namespace cache
		{
			namespace local
			{
				class abstract_command;
				class base
				{
				public:
					base(int32_t number_of_threads);
					virtual ~base(void);

					void initialize(void);
					void release(void);

					void add_worker(sirius::library::cache::local::abstract_command * command);
					void run_worker(PTP_WORK work);

					//Forwarded messages only
					void add_forarded_command(int32_t command_id);
					BOOL check_valid_command_id(int32_t command_id);

				protected:
					std::vector<int32_t>	_valid_command_ids;

				private:
					int32_t					_number_of_threads;
					TP_CALLBACK_ENVIRON		_callback_env;
					PTP_POOL				_pool;
					PTP_CLEANUP_GROUP		_cleanup_group;

					CRITICAL_SECTION		_lwiftp_base_cs;
				};
			};
		};
	};
};

#endif
