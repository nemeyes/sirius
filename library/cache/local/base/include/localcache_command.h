#ifndef _LOCALCACHE_COMMAND_H_
#define _LOCALCACHE_COMMAND_H_

#include <cstdint>
#include <memory>
#include <deque>
#include <localcache_commands.h>
#include <localcache_session.h>

#if defined(WITH_WORKING_AS_SERVER)
# include <rpc.h>
# include "localcache_server.h"
#else
# include "localcache_client.h"
#endif

#define COMMAND_SIZE sizeof(int)

#define CMD_ERR_CODE_SUCCESS		0
#define CMD_ERR_CODE_FAIL			1

namespace sirius
{
	namespace library
	{
		namespace cache
		{
			namespace local
			{
				class processor;
				class abstract_command
				{
					friend class sirius::library::cache::local::base;
				public:
					class tp_worker
						: public std::enable_shared_from_this<sirius::library::cache::local::abstract_command::tp_worker>
					{
					public:
						tp_worker(void)
							: command(NULL)
							, length(0)
							, msg(NULL)
							, session(NULL)
						{}

						~tp_worker(void)
						{
							if (msg)
								free(msg);
							msg = nullptr;
							length = 0;
						}

					public:
						sirius::library::cache::local::abstract_command * command;
						int32_t length;
						char * msg;
						std::shared_ptr<sirius::library::cache::local::session> session;
					};

					class tp_worker_scopped_lock
					{
					public:
						tp_worker_scopped_lock(SRWLOCK * srwl)
							: _srwl(srwl)
						{
							::AcquireSRWLockExclusive(_srwl);
						}
						~tp_worker_scopped_lock(void)
						{
							::ReleaseSRWLockExclusive(_srwl);
						}
					private:
						SRWLOCK * _srwl;
					};
					abstract_command(sirius::library::cache::local::localcache_processor * prcsr, int32_t command_id);
					abstract_command(int32_t command_id);
					virtual ~abstract_command(void);


					void set_processor(sirius::library::cache::local::localcache_processor * prcsr);
					sirius::library::cache::local::localcache_processor * get_processor(void);

					int32_t			command_id(void);
					BOOL			is_running(void);

					void			_execute(int32_t command_id, const char * msg, int32_t length, std::shared_ptr<sirius::library::cache::local::session> session);
					static void __stdcall tp_work_callback(PTP_CALLBACK_INSTANCE instance, PVOID parameter, PTP_WORK work);

				protected:
					void __execute(void);
					virtual void execute(int32_t command_id, const char * msg, int32_t length, std::shared_ptr<sirius::library::cache::local::session> session) = 0;

					sirius::library::cache::local::localcache_processor * _processor;
					int32_t		_command_id;
					PTP_WORK	_work;
					SRWLOCK		_srwl;
					std::deque<std::shared_ptr<sirius::library::cache::local::abstract_command::tp_worker>> _tp_ready_workers;
				};
			};
		};
	};
};

#endif