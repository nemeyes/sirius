#ifndef __CLUSTER_ADAPTER_H_
#define __CLUSTER_ADAPTER_H_
#include "sirius_backend_cluster.h"
#include "sirius_curl_client.h"
#include <deque>
#include <mutex>

#define CONFIG_FILE "\\configuration\\backend_cluster.ini"
#define QUEUE_MAX_SIZE							500
#define MAX_BACKOFFICE_DATA_BUFFER			300
static void CALLBACK timer_keepalive(uint32_t ui_id, uint32_t ui_msg, DWORD_PTR dw_user, DWORD_PTR dw1, DWORD_PTR dw2);

using namespace std;
namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace backend
			{
				class cluster_adapter
				{
				public:
					typedef struct _cluster_adapter_configuration_t
					{
						std::string ssp_ip;
						std::string ssp_port;
						std::string ssm_ip;
						std::string ssm_port;
					} cluster_adapter_configuration_t;

				public:
					static sirius::library::net::backend::cluster_adapter & get_instance() {
						static sirius::library::net::backend::cluster_adapter _instance;
						return _instance;
					}
					static BOOL convert_wide2multibyte(WCHAR * source, char ** destination)
					{
						UINT32 len = WideCharToMultiByte(CP_ACP, 0, source, wcslen(source), NULL, NULL, NULL, NULL);
						(*destination) = new char[NULL, len + 1];
						::ZeroMemory((*destination), (len + 1) * sizeof(char));
						WideCharToMultiByte(CP_ACP, 0, source, -1, (*destination), len, NULL, NULL);
						return TRUE;
					}
					int32_t start(std::string version);
					void stop();
					void backend_deinit();
					int32_t client_connect(char* client_id, int32_t use_count, int32_t attendant_num);
					int32_t client_disconnect(char* client_id, int32_t use_count, int32_t attendant_num);
					bool configure_load();
					bool	set_external_ip(char* ip);
					bool is_cluster_use() { return _use_cluster; };
					static unsigned __stdcall ssp_queue_thread(void * param);
					sirius::library::net::backend::cluster * _client;
					char _localip[MAX_PATH];
					char * _sirius_version;
					char *_ssp_url_st[QUEUE_MAX_SIZE];
					bool _backend_stat;
					bool _stat_timer;
					HANDLE	_ssp_event_push;
					uint32_t _system_user;
					int64_t _client_count;
					uint32_t _ssp_thread_exit;
					uint32_t _ssp_bufpos;
					uint32_t _keepalive_delay_time;
					uint32_t _keepalive_timeset_event;
					std::deque<char*> _ssp_queue;
					std::string get_ssp_ip() { return _ssp_ip; };
					std::string get_ssp_port() { return _ssp_port; };
					std::string get_ssm_ip() { return _ssm_ip; };
					std::string get_ssm_port() { return _ssm_port; };
					std::string get_idc_code() { return _idc_code; };
					std::string get_alive_interval() { return _alive_interval; };
					int _max_attendant;
				private:
					cluster_adapter_configuration_t _config;
					sirius::library::net::backend::cluster_adapter() {};
					bool _use_cluster;
					std::string _ssp_ip;
					std::string _ssp_port;
					std::string _ssm_ip;
					std::string _ssm_port;
					std::string _idc_code;
					std::string  _alive_interval;
				};
			};
		};
	};
};
#define SSP_ADT sirius::library::net::backend::cluster_adapter::get_instance()
#endif


