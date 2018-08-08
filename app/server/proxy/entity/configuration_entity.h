#ifndef _CONFIGURATION_ENTITY_H_
#define _CONFIGURATION_ENTITY_H_

#include <sirius.h>

namespace sirius
{
	namespace app
	{
		namespace server
		{
			namespace arbitrator
			{
				namespace entity
				{
					typedef struct _configuration_t
					{
						char	uuid[MAX_PATH];
						char	url[MAX_PATH];
						char	app_session_app[MAX_PATH * 5];		
						int32_t	max_attendant_instance;
						int32_t attendant_creation_delay;
						int32_t controller_portnumber;
						int32_t streamer_portnumber;
						int32_t video_codec;
						int32_t video_width;
						int32_t video_height;
						int32_t video_fps;
						int32_t video_buffer_count;
						int32_t video_block_width;
						int32_t video_block_height;
						int32_t video_compression_level;
						int32_t video_quantization_colors;
						bool	invalidate4client;
						bool	indexed_mode;
						int32_t	nthread;
						bool	enable_tls;
						bool	enable_keepalive;
						int32_t keepalive_timeout;
						bool	enable_present;
						bool	enable_auto_start;
						bool	enable_caching;
						bool	clean_attendant;
						_configuration_t(void)
							: max_attendant_instance(100)
							, attendant_creation_delay(2000)
							, controller_portnumber(5000)
							, streamer_portnumber(7000)
							, video_codec(sirius::base::video_submedia_type_t::png)
							, video_width(1280)
							, video_height(720)
							, video_fps(30)
							, video_buffer_count(1)
							, video_block_width(128)
							, video_block_height(72)
							, video_compression_level(1)
							, video_quantization_colors(128)
							, invalidate4client(false)
							, indexed_mode(false)
							, nthread(20)
							, enable_tls(false)
							, enable_keepalive(false)
							, keepalive_timeout(5000)
							, enable_present(false)
							, enable_auto_start(false)
							, enable_caching(false)
							, clean_attendant(false)
						{
							memset(uuid, 0x00, sizeof(uuid));
							memset(url, 0x00, sizeof(url));
							memset(app_session_app, 0x00, sizeof(app_session_app));
						}
					} configuration_t;
				};
			};
		};
	};
};




#endif
