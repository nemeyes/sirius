#include "sirius_backend_cluster.h"
#include "backend_cluster.h"
sirius::library::net::backend::cluster::cluster()
{
	_core = new core();
}

sirius::library::net::backend::cluster::~cluster()
{

}

void sirius::library::net::backend::cluster::backend_init(std::string version)
{
	_core->backend_init(version);
}

void sirius::library::net::backend::cluster::backend_stop()
{
	_core->backend_stop();
}

void sirius::library::net::backend::cluster::backend_client_connect(char* client_id, int32_t use_count, int32_t attendant_num)
{
	_core->backend_client_connect(client_id, use_count, attendant_num);
}

void sirius::library::net::backend::cluster::backend_client_disconnect(char* client_id, int32_t use_count, int32_t attendant_num)
{
	_core->backend_client_disconnect(client_id, use_count, attendant_num);
}

void sirius::library::net::backend::cluster::ssm_service_info(char* status, int32_t attendant_instance)
{
	_core->ssm_service_info(status, attendant_instance);
}

void sirius::library::net::backend::cluster::stop(void)
{
	_core->stop();
}

void sirius::library::net::backend::cluster::set_cluster_init(sirius::library::net::backend::cluster * client)
{
	_core->set_cluster_init(client);
}


void sirius::library::net::backend::cluster::set_sending_timeout(uint32_t timeout)
{
	_core->set_sending_timeout(timeout);
}

void sirius::library::net::backend::cluster::ssp_status_info(char * ssp_data)
{
	_core->ssp_status_info(ssp_data);
}

void sirius::library::net::backend::cluster::get_local_time(char * reg_time_date, char * reg_time_time)
{
	_core->get_local_time(reg_time_date, reg_time_time);
}