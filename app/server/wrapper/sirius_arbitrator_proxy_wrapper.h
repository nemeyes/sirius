#pragma once

#include <stdint.h>
#include "sirius_arbitrator_proxy.h"

using namespace System;
using namespace System::Runtime::InteropServices;

typedef void(*initialize_callback)(const char *, const char *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, bool, bool, bool, bool, bool, bool, bool, char *, char *);
typedef void(*system_monitor_info_callback)(double, double);
typedef void(*attendant_create_callback)(double);
typedef void(*start_callback)(void);
typedef void(*stop_callback)(void);
typedef void(*release_callback)(void);

public delegate void delegate_initialize_callback(const char *, const char *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, bool, bool, bool, bool, bool, bool, bool, char *, char *);
public delegate void delegate_system_monitor_info_callback(double, double);
public delegate void delegate_attendant_create_callback(double);
public delegate void delegate_start_callback(void);
public delegate void delegate_stop_callback(void);
public delegate void delegate_release_callback(void);

namespace sirius 
{
	namespace app
	{
		namespace server
		{
			namespace arbitrator
			{
				namespace wrapper
				{
					public class core : sirius::app::server::arbitrator::proxy::handler
					{
					public:
						core();
						virtual ~core();

						void initailize();
						void release();
						int start();
						int stop();
						int update(const char * uuid, const char * url, int32_t max_attendant_instance, int32_t attendant_creation_delay, int32_t portnumber, int32_t video_codec, int32_t video_width, int32_t video_height, int32_t video_fps, int32_t video_block_width, int32_t video_block_height, int32_t video_compression_level, int32_t video_quantization_colors, bool enable_tls, bool enable_gpu, bool enable_present, bool enable_auto_start, bool enable_quantization, bool enable_caching, bool enable_crc);
						int get_available_attendant_count();

						void on_initialize(const char * uuid, const char * url, int32_t max_attendant_instance, int32_t attendant_creation_delay, int32_t portnumber, int32_t video_codec, int32_t video_width, int32_t video_height, int32_t video_fps, int32_t video_block_width, int32_t video_block_height, int32_t video_compression_level, int32_t video_quantization_colors, bool enable_tls, bool enable_gpu, bool enable_present, bool enable_auto_start, bool enable_quantization, bool enable_caching, bool enable_crc, char * cpu, char * memory);
						void on_system_monitor_info(double cpu_usage, double memory_usage);
						void on_attendant_create(double percent);
						void on_start(void);
						void on_stop(void);
						void on_release(void);

						void set_initialize_callback(initialize_callback cbf) { _front_on_initalize = cbf; };
						void set_system_monitor_info_callback(system_monitor_info_callback cbf) { _front_on_system_monitor_info = cbf; };
						void set_attendant_create_callback(attendant_create_callback cbf) { _front_on_attendant_create = cbf; };
						void set_start_callback(start_callback cbf) { _front_on_start = cbf; };
						void set_stop_callback(stop_callback cbf) { _front_on_stop = cbf; };
						void set_release_callback(release_callback cbf) { _front_on_release = cbf; };

					private:
						sirius::app::server::arbitrator::proxy::context_t * _proxy_ctx;
						sirius::app::server::arbitrator::proxy *_proxy;

						initialize_callback _front_on_initalize;
						system_monitor_info_callback _front_on_system_monitor_info;
						attendant_create_callback _front_on_attendant_create;
						start_callback _front_on_start;
						stop_callback _front_on_stop;
						release_callback _front_on_release;

					};

					public ref class handler
					{
					public:
						handler();
						virtual ~handler();

						void initailize();
						void release();
						int start();
						int stop();
						int update(const char * uuid, const char * url, int32_t max_attendant_instance, int32_t attendant_creation_delay, int32_t portnumber, int32_t video_codec, int32_t video_width, int32_t video_height, int32_t video_fps, int32_t video_block_width, int32_t video_block_height, int32_t video_compression_level, int32_t video_quantization_colors, bool enable_tls, bool enable_gpu, bool enable_present, bool enable_auto_start, bool enable_quantization, bool enable_caching, bool enable_crc);
						int get_available_attendant_count();

						void set_initialize_callback(delegate_initialize_callback^  cbf);
						void set_system_monitor_info_callback(delegate_system_monitor_info_callback^ cbf);
						void set_attendant_create_callback(delegate_attendant_create_callback^ cbf);
						void set_start_callback(delegate_start_callback^ cbf);
						void set_stop_callback(delegate_stop_callback^ cbf);
						void set_release_callback(delegate_release_callback^ cbf);

					private:
						sirius::app::server::arbitrator::wrapper::core * _core;
					};
				}
			}
		}
	}
}
	
	


