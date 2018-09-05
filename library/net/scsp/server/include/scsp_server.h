#ifndef _PCSP_SERVER_H_
#define _PCSP_SERVER_H_

#include <string>
#include <map>
#include <vector>
#include <time.h>
#include <memory>
#include <sirius_sicp_server.h>

#include "sirius_scsp_server.h"

namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace scsp
			{
				class server::core
					: public sirius::library::net::sicp::server
				{
					static const int32_t MAX_STREAM_QUEUE = 30;
					static const int32_t IO_THREAD_POOL_COUNT = 2;
					static const int32_t COMMAND_THREAD_POOL_COUNT = 2;
					static const int32_t RECV_BUF_SIZE = 1024 * 1024;
					static const int32_t SEND_BUF_SIZE = 1024 * 1024;

				public:
					typedef struct _stream_session_info_t
						: public std::enable_shared_from_this<sirius::library::net::scsp::server::core::_stream_session_info_t>
					{
						char		uuid[64];
						bool		extradata_posted;
						_stream_session_info_t(void)
							: extradata_posted(false)
						{}

						_stream_session_info_t(const _stream_session_info_t & clone)
						{
							strncpy_s(uuid, clone.uuid, sizeof(uuid) - 1);
							extradata_posted = clone.extradata_posted;
						}
						_stream_session_info_t & operator=(const _stream_session_info_t & clone)
						{
							strncpy_s(uuid, clone.uuid, sizeof(uuid) - 1);
							extradata_posted = clone.extradata_posted;
							return (*this);
						}

					} stream_session_info_t;

					typedef struct _stream_conf
					{
						size_t				stream_type;
						int32_t				state;
						CRITICAL_SECTION	cs;
						std::vector<std::shared_ptr<sirius::library::net::scsp::server::core::stream_session_info_t>>	peers;
						_stream_conf(void)
						{
							peers.clear();
							::InitializeCriticalSection(&cs);
						}

						virtual ~_stream_conf(void)
						{
							::DeleteCriticalSection(&cs);
							peers.clear();
						}
					} stream_conf_t;

					core(const char * uuid, sirius::library::net::scsp::server * front);
					virtual ~core(void);

					int32_t start(sirius::library::net::scsp::server::context_t * context);
					int32_t stop(void);

					int32_t post_indexed_video(uint8_t * bytes, size_t nbytes, long long timestamp);
					int32_t post_coordinates_video(uint8_t * bytes, size_t nbytes, long long timestamp);

					int32_t play(int32_t flags);
					int32_t pause(int32_t flags);
					int32_t stop(int32_t flags);
					int32_t invalidate(void);

					void	on_create_session(const char * uuid);
					void	on_destroy_session(const char * uuid);

					int32_t play_callback(const char * client_uuid, int32_t type, const char * attendant_uuid);
					int32_t state(int32_t type);
				private:
					sirius::library::net::scsp::server::context_t * _context;
					sirius::library::net::scsp::server * _front;
					sirius::library::net::scsp::server::core::stream_conf_t	_video_conf;
				};
			};
		};
	};
};



#endif
