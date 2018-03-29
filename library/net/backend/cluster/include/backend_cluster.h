#ifndef _BACKEND_CLUSTER_CLIENT_H_
#define _BACKEND_CLUSTER_CLIENT_H_

#define BUF_SIZE 1024
#define CLUSTER_BUF_SIZE 2000
#include "sirius_backend_cluster.h"
#include "sirius_curl_client.h"
#include "WinSock2.h"
#include <stdint.h>
#include <vector>

namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace backend
			{
				class cluster;
				class cluster::core
				{
				public:
					typedef enum _error_type_t {
						CURLE_FAILED_INIT = 2,
						CURLE_OPERATION_TIMEDOUT = 28
					}error_type_t;	
					core();
					~core();
					void backend_init(std::string version);
					void backend_stop();
					void backend_client_connect(char* client_id, int32_t use_count, int32_t attendant_num);
					void backend_client_disconnect(char* client_id, int32_t use_count, int32_t attendant_num);
					void ssm_service_info(char* status, int32_t attendant_instance);
					void stop(void);
					void	set_cluster_init(sirius::library::net::backend::cluster *client);
					void set_sending_timeout(uint32_t timeout);
					void ssp_status_info(char * ssp_data);
					void alive_start();
					char _gpu_name[MAX_PATH];
					char _cluster_buf[CLUSTER_BUF_SIZE];
					char _ssp_url[MAX_PATH];
					char _ssm_url[MAX_PATH];
					char* _sirius_ip;
					
				private:
					CRITICAL_SECTION _lock;
					bool			_run;
					sirius::library::net::curl::client * _curl_ssp;
					sirius::library::net::curl::client * _curl_ssm;
					sirius::library::net::backend::cluster * _front;
					std::string _ssp_ip;
					std::string _ssp_port;
					std::string _ssm_ip;
					std::string _ssm_port;

					void	close_thread_pool();
					std::vector<HANDLE>	_threads;
					bool			set_hostname();
					int					_thread_count;

				public:
					DWORD	get_local_time(char * reg_time_date, char * reg_time_time);
					int _cnt;
					char* _cluster_data;
				private:
					char*	_cpu;
					char*	_os;
					int		_memory;
					char*	_host_name;
					int		_host_name_length;
					int		_sending_timeout;
				};
			};
		};
	};
};
#endif
