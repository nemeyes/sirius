#ifndef _SIRIUS_LOCALCACHE_SERVER_H_
#define _SIRIUS_LOCALCACHE_SERVER_H_

#if defined(EXPORT_SIRIUS_LOCALCACHE_SERVER_LIB)
#define EXP_SIRIUS_LOCALCACHE_SERVER_CLASS __declspec(dllexport)
#else
#define EXP_SIRIUS_LOCALCACHE_SERVER_CLASS __declspec(dllimport)
#endif

#include <winsock2.h>
#include <windows.h>
#include <cstdint>

namespace sirius
{
	namespace library
	{
		namespace cache
		{
			namespace local
			{
				class abstract_command;
				class EXP_SIRIUS_LOCALCACHE_SERVER_CLASS server
				{
				public:
					class core;

					typedef struct EXP_SIRIUS_LOCALCACHE_SERVER_CLASS _context_t
					{
						int32_t nsessions;
						int32_t cache_size_mib;
						int32_t nthread_pool;
						char 	path[MAX_PATH];
						int32_t portnumber;
						_context_t(void)
							: nsessions(1000)
							, cache_size_mib(0)
							, nthread_pool(0)
							, portnumber(5001)
						{
							::memset(path, 0x00, MAX_PATH);
						}
					} context_t;

					server();
					virtual ~server(void);

					int32_t initialize(sirius::library::cache::local::server::context_t * context);
					int32_t release(void);
					int32_t start(void);
					int32_t stop(void);

				private:
					sirius::library::cache::local::server::core * _server;
					sirius::library::cache::local::server::context_t * _context;
					BOOL _dynamic_alloc;
				};
			};
		};
	};
};

#endif