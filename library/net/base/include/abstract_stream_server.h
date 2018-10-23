#ifndef _ABSTRACT_STREAM_SERVER_H_
#define _ABSTRACT_STREAM_SERVER_H_

#include <sirius.h>

#if defined(EXPORT_SERVER_BASE_LIB)
#define EXP_SERVER_BASE_CLASS __declspec(dllexport)
#else
#define EXP_SERVER_BASE_CLASS __declspec(dllimport)
#endif

namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace stream
			{
				class EXP_SERVER_BASE_CLASS server
					: public sirius::base
				{
				public:
					class proxy
					{
					public:
						virtual int32_t play(int32_t flags) = 0;
						virtual int32_t pause(int32_t flags) = 0;
						virtual int32_t stop(int32_t flags) = 0;
						virtual int32_t invalidate(void) = 0;
					};

					typedef struct EXP_SERVER_BASE_CLASS _context_t
					{
						char			uuid[64];
						int32_t			portnumber;
						bool			keepalive;
						int32_t			keepalive_timeout;
						int32_t			video_codec;
						int32_t			video_width;
						int32_t			video_height;
						int32_t			video_fps;
						int32_t			video_block_width;
						int32_t			video_block_height;
						proxy *			controller;
						_context_t(void)
							: keepalive(false)
							, keepalive_timeout(5000)
							, video_codec(sirius::library::net::stream::server::video_submedia_type_t::unknown)
							, video_width(1280)
							, video_height(720)
							, video_fps(30)
							, video_block_width(128)
							, video_block_height(72)
							, controller(nullptr)
						{
							memset(uuid, 0x00, sizeof(uuid));
						}
					} context_t;

					typedef struct EXP_SERVER_BASE_CLASS _network_usage_t
					{
						uint64_t video_transferred_bytes;
						_network_usage_t(void)
							: video_transferred_bytes(0)
						{}
					} network_usage_t;

				public:
					server(void);
					virtual ~server(void);

					virtual int32_t start(sirius::library::net::stream::server::context_t * context);
					virtual int32_t stop(void);

					virtual int32_t post_video(uint8_t * bytes, size_t nbytes, long long timestamp);
					virtual sirius::library::net::stream::server::network_usage_t & get_network_usage(void);


				protected:
#if defined(WITH_CASP)
					int32_t _vsmt;
					int32_t _asmt;
					char _address[MAX_PATH];
					int32_t _port_number;
					int32_t _slot_number;
					char _uuid[MAX_PATH];
					server::proxy * _controller;
#endif
					sirius::library::net::stream::server::network_usage_t		_network_usage;
				};
			};
		};
	};
};

#endif