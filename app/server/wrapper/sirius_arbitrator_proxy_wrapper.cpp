// 기본 DLL 파일입니다.

#include "stdafx.h"

#include "sirius_arbitrator_proxy_wrapper.h"


sirius::app::server::arbitrator::wrapper::core::core()
{
	 _front_on_initalize = nullptr;
	 _front_on_system_monitor_info = nullptr;
	 _front_on_attendant_create = nullptr;
	 _front_on_start = nullptr;
	 _front_on_stop = nullptr;
	 _front_on_release = nullptr;
}

sirius::app::server::arbitrator::wrapper::core::~core()
{
	
}

void sirius::app::server::arbitrator::wrapper::core::initailize()
{	
	_proxy_ctx = new sirius::app::server::arbitrator::proxy::context_t();
	_proxy_ctx->handler = this;
	_proxy = new sirius::app::server::arbitrator::proxy();
	set_proxy(_proxy);
	_proxy->initialize(_proxy_ctx);
}

void sirius::app::server::arbitrator::wrapper::core::release()
{
	_proxy->release();
}

int sirius::app::server::arbitrator::wrapper::core::start()
{
	return _proxy->start();
}

int sirius::app::server::arbitrator::wrapper::core::stop()
{
	return _proxy->stop();
}

int sirius::app::server::arbitrator::wrapper::core::update(const char * uuid, const char * url, int32_t max_attendant_instance, int32_t attendant_creation_delay, int32_t portnumber, int32_t video_codec, int32_t video_width, int32_t video_height, int32_t video_fps, int32_t video_block_width, int32_t video_block_height, int32_t video_compression_level, int32_t video_quantization_colors, bool enable_tls, bool enable_gpu, bool enable_present, bool enable_auto_start, bool enable_quantization, bool enable_caching, bool enable_crc)
{
	return _proxy->update(uuid, url, max_attendant_instance, attendant_creation_delay, portnumber, video_codec, video_width, video_height, video_fps, video_block_width, video_block_height, video_compression_level, video_quantization_colors, enable_tls, enable_gpu, enable_present, enable_auto_start, enable_quantization, enable_caching, enable_crc);
}

void sirius::app::server::arbitrator::wrapper::core::on_initialize(const char * uuid, const char * url, int32_t max_attendant_instance, int32_t attendant_creation_delay, int32_t portnumber, int32_t video_codec, int32_t video_width, int32_t video_height, int32_t video_fps, int32_t video_block_width, int32_t video_block_height, int32_t video_compression_level, int32_t video_quantization_colors, bool enable_tls, bool enable_gpu, bool enable_present, bool enable_auto_start, bool enable_quantization, bool enable_caching, bool enable_crc, char * cpu, char * memory)
{
	_front_on_initalize(uuid, url, max_attendant_instance, attendant_creation_delay, portnumber, video_codec, video_width, video_height, video_fps, video_block_width, video_block_height, video_compression_level, video_quantization_colors, enable_tls, enable_gpu, enable_present, enable_auto_start, enable_quantization, enable_caching, enable_crc, cpu, memory);
}

void sirius::app::server::arbitrator::wrapper::core::on_system_monitor_info(double cpu_usage, double memory_usage)
{
	_front_on_system_monitor_info(cpu_usage, memory_usage);
}

void sirius::app::server::arbitrator::wrapper::core::on_attendant_create(double percent)
{
	_front_on_attendant_create(percent);
}

void sirius::app::server::arbitrator::wrapper::core::on_start()
{
	_front_on_start();
}

void sirius::app::server::arbitrator::wrapper::core::on_stop()
{
	_front_on_stop();
}

void sirius::app::server::arbitrator::wrapper::core::on_release()
{
	_front_on_release();
}

sirius::app::server::arbitrator::wrapper::handler::handler()
{
	_core = new sirius::app::server::arbitrator::wrapper::core;
}

sirius::app::server::arbitrator::wrapper::handler::~handler()
{
	if (_core)
	{
		delete _core;
		_core = nullptr;
	}
}

void sirius::app::server::arbitrator::wrapper::handler::initailize()
{
	_core->initailize();
}

void sirius::app::server::arbitrator::wrapper::handler::release()
{
	_core->release();
}

int sirius::app::server::arbitrator::wrapper::handler::start()
{
	return _core->start();
}

int sirius::app::server::arbitrator::wrapper::handler::stop()
{
	return _core->stop();
}

int sirius::app::server::arbitrator::wrapper::handler::update(const char * uuid, const char * url, int32_t max_attendant_instance, int32_t attendant_creation_delay, int32_t portnumber, int32_t video_codec, int32_t video_width, int32_t video_height, int32_t video_fps, int32_t video_block_width, int32_t video_block_height, int32_t video_compression_level, int32_t video_quantization_colors, bool enable_tls, bool enable_gpu, bool enable_present, bool enable_auto_start, bool enable_quantization, bool enable_caching, bool enable_crc)
{
	return _core->update(uuid, url, max_attendant_instance, attendant_creation_delay, portnumber, video_codec, video_width, video_height, video_fps, video_block_width, video_block_height, video_compression_level, video_quantization_colors, enable_tls, enable_gpu, enable_present, enable_auto_start, enable_quantization, enable_caching, enable_crc);
}

void sirius::app::server::arbitrator::wrapper::handler::set_initialize_callback(delegate_initialize_callback^ cbf)
{
	IntPtr ptr = Marshal::GetFunctionPointerForDelegate(cbf);
	initialize_callback fptr = static_cast<initialize_callback>(ptr.ToPointer());
	_core->set_initialize_callback(fptr);
}

void sirius::app::server::arbitrator::wrapper::handler::set_system_monitor_info_callback(delegate_system_monitor_info_callback^ cbf)
{
	IntPtr ptr = Marshal::GetFunctionPointerForDelegate(cbf);
	system_monitor_info_callback fptr = static_cast<system_monitor_info_callback>(ptr.ToPointer());
	_core->set_system_monitor_info_callback(fptr);
}

void sirius::app::server::arbitrator::wrapper::handler::set_attendant_create_callback(delegate_attendant_create_callback^ cbf)
{
	IntPtr ptr = Marshal::GetFunctionPointerForDelegate(cbf);
	attendant_create_callback fptr = static_cast<attendant_create_callback>(ptr.ToPointer());
	_core->set_attendant_create_callback(fptr);
}

void sirius::app::server::arbitrator::wrapper::handler::set_start_callback(delegate_start_callback^ cbf)
{
	IntPtr ptr = Marshal::GetFunctionPointerForDelegate(cbf);
	start_callback fptr = static_cast<start_callback>(ptr.ToPointer());
	_core->set_start_callback(fptr);
}

void sirius::app::server::arbitrator::wrapper::handler::set_stop_callback(delegate_stop_callback^ cbf)
{
	IntPtr ptr = Marshal::GetFunctionPointerForDelegate(cbf);
	stop_callback fptr = static_cast<stop_callback>(ptr.ToPointer());
	_core->set_stop_callback(fptr);
}

void sirius::app::server::arbitrator::wrapper::handler::set_release_callback(delegate_release_callback^ cbf)
{
	IntPtr ptr = Marshal::GetFunctionPointerForDelegate(cbf);
	release_callback fptr = static_cast<release_callback>(ptr.ToPointer());
	_core->set_release_callback(fptr);
}


