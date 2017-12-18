#ifndef _ARBITRATOR_PROXY_H_
#define _ARBITRATOR_PROXY_H_

#include <sirius_uuid.h>
#include "sirius_arbitrator_proxy.h"
#include <sirius_sicp_server.h>
#include "process_controller.h"
#include <sirius_performance_monitor.h>

namespace sirius
{
	namespace app
	{
		namespace server
		{
			namespace arbitrator
			{
				class proxy::core
					: public sirius::library::net::sicp::server
				{
					static const int32_t BASE_PORTNUMBER = 7000;
					static const int32_t IO_THREAD_POOL_COUNT = 0;
					static const int32_t COMMAND_THREAD_POOL_COUNT = 0;
					static const int32_t MTU_SIZE = 1500;
					static const int32_t ARGUMENT_SIZE = 1024;
					static const int32_t MAX_GPU_COUNT = 20;
				public:
					typedef struct _attendant_state_t 
					{
						static const int32_t idle = 0;
						static const int32_t available = 1;
						static const int32_t starting = 2;
						static const int32_t running = 3;
						static const int32_t stopping = 4;
					} attendant_state_t;

					core(const char * uuid, sirius::app::server::arbitrator::proxy * front);
					virtual ~core(void);

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

					static void	retrieve_db_path(char * path);

				private:
					void	create_session_callback(const char * uuid);
					void	destroy_session_callback(const char * uuid);

					int32_t get_attendant_count(void);
					int32_t get_launcher_count(void);

					static unsigned __stdcall process_cb(void * param);
					void process(void);

					static unsigned __stdcall system_monitor_process_cb(void * param);
					void system_monitor_process(void);

				private:
					sirius::app::server::arbitrator::proxy * _front;
					sirius::app::server::arbitrator::proxy::context_t * _context;

					HANDLE _thread;
					bool _run;

					sirius::library::misc::performance::monitor * _monitor;
					HANDLE _system_monitor_thread;
					bool _system_monitor_run;

					CRITICAL_SECTION _attendant_cs;
				};
			};
		};
	};
};

#endif