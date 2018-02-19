#include "arbitrator_proxy.h"
#include "sirius_arbitrator_proxy.h"
#include "configuration_dao.h"

sirius::app::server::arbitrator::proxy::proxy(void)
	: _core(nullptr)
{

}

sirius::app::server::arbitrator::proxy::~proxy(void)
{

}

int32_t sirius::app::server::arbitrator::proxy::initialize(sirius::app::server::arbitrator::proxy::context_t * context)
{
	int32_t status = sirius::app::server::arbitrator::proxy::err_code_t::fail;

	sirius::app::server::arbitrator::proxy::core::retrieve_db_path(context->db_path);
	sirius::app::server::arbitrator::db::configuration_dao dao(context->db_path);
	sirius::app::server::arbitrator::entity::configuration_t configuration;
	status = dao.retrieve(&configuration);

	if (status == sirius::app::server::arbitrator::proxy::err_code_t::success)
	{
		_core = new sirius::app::server::arbitrator::proxy::core(configuration.uuid, this, configuration.enable_keepalive, configuration.enable_tls);
		status = _core->initialize(context);
	}
	else
	{
		status = sirius::app::server::arbitrator::proxy::err_code_t::fail;
	}
	return status;
}

int32_t sirius::app::server::arbitrator::proxy::release(void)
{
	int32_t status = sirius::app::server::arbitrator::proxy::err_code_t::fail;
	
	if (_core)
	{
		status = _core->release();
		delete _core;
		_core = nullptr;
	}
	return status;
}

int32_t sirius::app::server::arbitrator::proxy::start(void)
{
	if (_core)
		return _core->start();
	else
		return sirius::app::server::arbitrator::proxy::err_code_t::fail;
}

int32_t sirius::app::server::arbitrator::proxy::stop(void)
{
	if (_core)
		return _core->stop();
	else
		return sirius::app::server::arbitrator::proxy::err_code_t::fail;
}

int32_t sirius::app::server::arbitrator::proxy::update(const char * uuid, const char * url, int32_t max_containter_instance, int32_t attendant_creation_delay, int32_t controller_portnumber, int32_t streamer_portnumber, int32_t video_codec, int32_t video_width, int32_t video_height, int32_t video_fps, int32_t video_block_width, int32_t video_block_height, int32_t video_compression_level, int32_t video_quantization_colors, bool enable_tls, bool enable_keepalive, bool enable_present, bool enable_auto_start, bool enable_caching)
{
	if (_core)
		return _core->update(uuid, url, max_containter_instance, attendant_creation_delay, controller_portnumber, streamer_portnumber, video_codec, video_width, video_height, video_fps, video_block_width, video_block_height, video_compression_level, video_quantization_colors, enable_tls, enable_keepalive, enable_present, enable_auto_start, enable_caching);
	else
		return sirius::app::server::arbitrator::proxy::err_code_t::fail;
}

int32_t	sirius::app::server::arbitrator::proxy::connect_client(const char * uuid, const char * id)
{
	if (_core)
		return _core->connect_client(uuid, id);
	else
		return sirius::app::server::arbitrator::proxy::err_code_t::fail;
}

int32_t sirius::app::server::arbitrator::proxy::disconnect_client(const char * uuid)
{
	if (_core)
		return _core->disconnect_client(uuid);
	else
		return sirius::app::server::arbitrator::proxy::err_code_t::fail;
}

int32_t sirius::app::server::arbitrator::proxy::get_available_attendant_count(void)
{
	if (_core)
		return _core->get_available_attendant_count();
	else
		return sirius::app::server::arbitrator::proxy::err_code_t::fail;
}

int32_t	sirius::app::server::arbitrator::proxy::connect_attendant_callback(const char * uuid, int32_t id, int32_t pid)
{
	if (_core)
		return _core->connect_attendant_callback(uuid, id, pid);
	else
		return sirius::app::server::arbitrator::proxy::err_code_t::fail;
}

void sirius::app::server::arbitrator::proxy::disconnect_attendant_callback(const char * uuid)
{
	if (_core)
		_core->disconnect_attendant_callback(uuid);
}

void sirius::app::server::arbitrator::proxy::start_attendant_callback(const char * uuid, int32_t id, const char * client_id, const char * client_uuid, int32_t code)
{
	if (_core)
		_core->start_attendant_callback(uuid, id, client_id, client_uuid, code);
}

void sirius::app::server::arbitrator::proxy::stop_attendant_callback(const char * uuid, int32_t code)
{
	if (_core)
		_core->stop_attendant_callback(uuid, code);
}
