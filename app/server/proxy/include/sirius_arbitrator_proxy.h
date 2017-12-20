#ifndef _SIRIUS_ARBITRATOR_PROXY_H_
#define _SIRIUS_ARBITRATOR_PROXY_H_

#include <sirius.h>

#if defined(EXPORT_ARBITRATOR_PROXY)
#define EXP_ARBITRATOR_PROXY_CLASS __declspec(dllexport)
#else
#define EXP_ARBITRATOR_PROXY_CLASS __declspec(dllimport)
#endif


namespace sirius
{
	namespace app
	{
		namespace server
		{
			namespace arbitrator
			{
				class attendant_cmd;
				class connect_attendant_req;
				class EXP_ARBITRATOR_PROXY_CLASS proxy
					: public sirius::base
				{
					friend class sirius::app::server::arbitrator::attendant_cmd;
					friend class sirius::app::server::arbitrator::connect_attendant_req;
				public:
					class core;
				public:
					class handler
					{
					public:
						handler(void)
							: _proxy(nullptr)
						{}

						void set_proxy(sirius::app::server::arbitrator::proxy * prxy)
						{
							_proxy = prxy;
						}

						void update(const char * uuid, const char * url, int32_t max_attendant_instance, int32_t attendant_creation_delay, int32_t portnumber, int32_t video_codec, int32_t video_width, int32_t video_height, int32_t video_fps, int32_t video_block_width, int32_t video_block_height, int32_t video_compression_level, int32_t video_quantization_colors, bool enable_tls, bool enable_gpu, bool enable_present, bool enable_auto_start, bool enable_quantization, bool enable_caching, bool enable_crc)
						{
							if(_proxy)
								_proxy->update(uuid, url, max_attendant_instance, attendant_creation_delay, portnumber, video_codec, video_width, video_height, video_fps, video_block_width, video_block_height, video_compression_level, video_quantization_colors, enable_tls, enable_gpu, enable_present, enable_auto_start, enable_quantization, enable_caching, enable_crc);
						}

						virtual void on_initialize(const char * uuid, const char * url, int32_t max_attendant_instance, int32_t attendant_creation_delay, int32_t portnumber, int32_t video_codec, int32_t video_width, int32_t video_height, int32_t video_fps, int32_t video_block_width, int32_t video_block_height, int32_t video_compression_level, int32_t video_quantization_colors, bool enable_tls, bool enable_gpu, bool enable_present, bool enable_auto_start, bool enable_quantization, bool enable_caching, bool enable_crc, char * cpu, char * memory) = 0;
						virtual void on_system_monitor_info(double cpu_usage, double memory_usage) = 0;
						virtual void on_attendant_create(double percent) = 0;
						virtual void on_start(void) = 0;
						virtual void on_stop(void) = 0;
						virtual void on_release(void) = 0;

					private:
						sirius::app::server::arbitrator::proxy * _proxy;
					};

					typedef struct _context_t
					{
						char db_path[MAX_PATH];
						sirius::app::server::arbitrator::proxy::handler * handler;
					} context_t;


					proxy(void);
					virtual ~proxy(void);

					int32_t initialize(sirius::app::server::arbitrator::proxy::context_t * context);
					int32_t release(void);

					int32_t start(void);
					int32_t stop(void);

					int32_t update(const char * uuid, const char * url, int32_t max_attendant_instance, int32_t attendant_creation_delay, int32_t portnumber, int32_t video_codec, int32_t video_width, int32_t video_height, int32_t video_fps, int32_t video_block_width, int32_t video_block_height, int32_t video_compression_level, int32_t video_quantization_colors, bool enable_tls, bool enable_gpu, bool enable_present, bool enable_auto_start, bool enable_quantization, bool enable_caching, bool enable_crc);

					int32_t	connect_client(const char * uuid, const char * id);
					int32_t disconnect_client(const char * uuid);


					int32_t	connect_attendant_callback(const char * uuid, const char * id);
					void	disconnect_attendant_callback(const char * uuid);

					void	start_attendant_callback(const char * uuid, const char * id, int32_t code);
					void	stop_attendant_callback(const char * uuid, int32_t code);

				private:
					sirius::app::server::arbitrator::proxy::core * _core;
				};
			};
		};
	};
};

#endif