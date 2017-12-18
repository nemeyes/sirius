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
						int32_t	max_attendant_instance;
						int32_t attendant_creation_delay;
						int32_t portnumber;
						int32_t video_codec;
						int32_t video_width;
						int32_t video_height;
						int32_t video_fps;
						int32_t video_block_width;
						int32_t video_block_height;
						int32_t video_compression_level;
						int32_t video_quantization_colors;
						bool	enable_tls;
						bool	enable_gpu;
						bool	enable_present;
						bool	enable_auto_start;
						bool	enable_quantization;
						bool	enable_caching;
						bool	enable_crc;
						_configuration_t(void)
							: max_attendant_instance(100)
							, attendant_creation_delay(2000)
							, portnumber(5000)
							, video_codec(sirius::base::video_submedia_type_t::png)
							, video_width(1280)
							, video_height(720)
							, video_fps(30)
							, video_block_width(128)
							, video_block_height(72)
							, video_compression_level(1)
							, video_quantization_colors(128)
							, enable_tls(false)
							, enable_gpu(false)
							, enable_present(false)
							, enable_auto_start(false)
							, enable_quantization(false)
							, enable_caching(false)
							, enable_crc(false)
						{
							memset(uuid, 0x00, sizeof(uuid));
							memset(url, 0x00, MAX_PATH);
						}
					} configuration_t;
				};
			};
		};
	};
};




#endif
