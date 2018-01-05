#ifndef _SIRIUS_BACKEND_CLUSTER_CLIENT_H_
#define _SIRIUS_BACKEND_CLUSTER_CLIENT_H_

#include "WinSock2.h"
#include <stdint.h>
#include <string>
#define SENDING_TIME 30
#if defined(EXPORTS_BACKEND_CLUSTER_LIB)
#define EXP_BACKEND_CLUSTER_CLASS __declspec(dllexport)
#else
#define EXP_BACKEND_CLUSTER_CLASS __declspec(dllimport)
#endif

namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace backend
			{
				class EXP_BACKEND_CLUSTER_CLASS cluster
				{
				public:
					class core;
				public:
					cluster();
					virtual ~cluster();
					void backend_init(std::string version);
					void backend_stop();
					void backend_client_connect(char* client_id, int32_t use_count, int32_t attendant_num);
					void backend_client_disconnect(char* client_id, int32_t use_count, int32_t attendant_num);
					void ssm_service_info(char* status, int32_t attendant_instance);
					void stop(void);
					void set_cluster_init(sirius::library::net::backend::cluster *client);
					void set_sending_timeout(uint32_t timeout);
					void ssp_status_info(char * ssp_data);
					void get_local_time(char * reg_time_date, char * reg_time_time);
				private:
					sirius::library::net::backend::cluster::core * _core;
				};
			};
		};
	};
};

#endif