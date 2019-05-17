#ifndef _STORAGE_MANAGER_H_
#define _STORAGE_MANAGER_H_

#include "sirius_localcache_server.h"
#include <localcache_session.h>
#include <sirius.h>
#include <memory>
#include <map>
#include <unordered_map>
#include <functional>

#define MAXIMUM_IMAGE_SIZE 256 * 180 * 4
#define MTU_SIZE 35 * 1024

namespace sirius
{
	namespace library
	{
		namespace cache
		{
			namespace local
			{
				namespace storage
				{
					class manager
						: public sirius::base
					{
					public:
						typedef struct _uploading_state_t
						{
							char	hash[130];
							int32_t	size;
							int32_t width;
							int32_t height;
							int32_t nprocessed;
							int64_t	timestamp;
							SRWLOCK	lock;
							HANDLE	file;
							_uploading_state_t(void)
								: size(0)
								, width(0)
								, height(0)
								, nprocessed(0)
								, timestamp(0)
								, file(INVALID_HANDLE_VALUE)
							{
								memset(hash, 0x00, sizeof(hash));
								::InitializeSRWLock(&lock);
							}
							~_uploading_state_t(void)
							{}
						} uploading_state_t;

						typedef struct _cached_index_t
						{
							char	hash[130];		//130
#if defined(WIN64)
							int64_t	hitcount;		//8
#else
							long	hitcount;		//4
#endif
							int32_t	size;			//4
							int32_t width;			//4
							int32_t height;			//4
							BOOL	upload_needed;	//4
							BOOL	ftell_needed;	//4
							BOOL	serialized;		//4
							SRWLOCK	lock;
							_cached_index_t(void)
								: hitcount(0)
								, size(0)
								, upload_needed(FALSE)
								, ftell_needed(FALSE)
								, serialized(FALSE)
							{
								::InitializeSRWLock(&lock);
								memset(hash, 0x00, sizeof(hash));
							}

							~_cached_index_t(void)
							{}

							_cached_index_t(const sirius::library::cache::local::storage::manager::_cached_index_t & rhs)
							{
								memmove(hash, rhs.hash, sizeof(hash));
								hitcount = rhs.hitcount;
								size = rhs.size;
								width = rhs.width;
								height = rhs.height;
								upload_needed = rhs.upload_needed;
								ftell_needed = rhs.ftell_needed;
								serialized = rhs.serialized;
								::InitializeSRWLock(&lock);
							}

							_cached_index_t & operator=(const sirius::library::cache::local::storage::manager::_cached_index_t & rhs)
							{
								memmove(hash, rhs.hash, sizeof(hash));
								hitcount = rhs.hitcount;
								size = rhs.size;
								width = rhs.width;
								height = rhs.height;
								upload_needed = rhs.upload_needed;
								ftell_needed = rhs.ftell_needed;
								serialized = rhs.serialized;
								return (*this);
							}
						} cached_index_t;

						typedef struct _sorted_index_t
						{
							std::map<std::string, std::shared_ptr<sirius::library::cache::local::storage::manager::cached_index_t>> group;
							_sorted_index_t(void)
							{}

							~_sorted_index_t(void)
							{}
						} sorted_index_t;

						manager(void);
						~manager(void);

						int32_t initialize(sirius::library::cache::local::server::context_t * context);
						int32_t release(void);

						int32_t start(void);	
						int32_t stop(void);

						int32_t begin_uploading(const char * sessionid, const char * hash, int32_t fsize, int32_t width, int32_t height, const char * packet, int32_t size);
						int32_t processing_uploading(const char * sessionid, const char * hash, int32_t fsize, int32_t width, int32_t height, int32_t offset, const char * packet, int32_t size);
						int32_t end_uploading(const char * sessionid, const char * hash, int32_t fsize, int32_t width, int32_t height, int32_t offset, const char * packet, int32_t size);
						void	on_download(const char * sessionid, const char * hash, std::shared_ptr<sirius::library::cache::local::session> session);
						void	on_ftell(const char * hash, int32_t fsize);

					private:
						HANDLE	create_cache_file(int32_t width, int32_t height, const char * hash);
						BOOL	do_shared_writing(HANDLE file , int32_t offset, const char * packet, int32_t size);
						int32_t	create_serialized_cached_index(std::shared_ptr<sirius::library::cache::local::storage::manager::uploading_state_t> us);

						BOOL	read(HANDLE file, int64_t offset, char * packet, int32_t size);
						BOOL	read(HANDLE file, int64_t offset, int64_t & packet);
						BOOL	read(HANDLE file, int64_t offset, long & packet);
						BOOL	read(HANDLE file, int64_t offset, int32_t & packet);

						BOOL	write(HANDLE file, int64_t offset, char * packet, int32_t size);
						BOOL	write(HANDLE file, int64_t offset, int64_t packet);
						BOOL	write(HANDLE file, int64_t offset, long packet);
						BOOL	write(HANDLE file, int64_t offset, int32_t packet);


						static unsigned __stdcall process_cb(void * param);
						void	process(void);
								
					private:
						sirius::library::cache::local::server::context_t * _context;

						SRWLOCK _ulock;
						std::map<std::string, std::shared_ptr<sirius::library::cache::local::storage::manager::uploading_state_t>> _uploading;

						char _index_path[MAX_PATH];
						char _cache_path[MAX_PATH];
						int64_t _cache_limit;
						BOOL _cache_full;

						SRWLOCK _clock;
						std::map<std::string, std::shared_ptr<sirius::library::cache::local::storage::manager::cached_index_t>> _cache;

						HANDLE	_thread;
						BOOL	_run;
					};
				};
			};
		};
	};
};


#endif