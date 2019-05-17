#ifndef _SIRIUS_LOCALCACHE_CLIENT_H_
#define _SIRIUS_LOCALCACHE_CLIENT_H_

#if defined(EXPORT_SIRIUS_LOCALCACHE_CLIENT_LIB)
#define EXP_SIRIUS_LOCALCACHE_CLIENT_CLASS __declspec(dllexport)
#else
#define EXP_SIRIUS_LOCALCACHE_CLIENT_CLASS __declspec(dllimport)
#endif

#include <winsock2.h>
#include <sirius.h>

namespace sirius
{
	namespace library
	{
		namespace cache
		{
			namespace local
			{
				class abstract_command;
				class EXP_SIRIUS_LOCALCACHE_CLIENT_CLASS client
					: public sirius::base
				{
				public:
					class core;
				public:
					client(void);
					virtual ~client(void);

					int32_t connect(const char * address, int32_t portnumber, BOOL reconnection = TRUE);
					int32_t disconnect(void);
					void	disconnect(BOOL enable);

					int32_t upload(const char * hash, const char * image, int32_t size, int32_t width, int32_t height);
					int32_t download(const char * hash, char * image, int32_t capacity, int32_t & size);
					int32_t ftell(const char * hash, int32_t fsize);

				private:
					sirius::library::cache::local::client::core * _client;
				};
			};
		};
	};
};




#endif
