#include "backend_cluster.h"
#include <process.h>
#include <atlstr.h>
#include<time.h>
#include "sirius_log4cplus_logger.h"
#include "device_manager.h"
#include "cluster_adapter.h"
#include "sirius_version.h"

sirius::library::net::backend::cluster::core::core()
	:_run(false),
	_cnt(1),
	_front(NULL)
{
	::InitializeCriticalSection(&_lock);
	_cpu = sirius::library::net::backend::device_manager::get_cpu_name();
	_os = sirius::library::net::backend::device_manager::get_operatingsystem_name();
	_memory = sirius::library::net::backend::device_manager::get_memory_info();
	_host_name = new char[MAX_PATH];
}

sirius::library::net::backend::cluster::core::~core()
{
	delete[] _os;
	delete[] _cpu;
	delete[] _host_name;

	if (_curl_ssp)
	{
		delete _curl_ssp;
		_curl_ssp = NULL;
	}
	if (_curl_ssm)
	{
		delete _curl_ssm;
		_curl_ssm = NULL;
	}
	::DeleteCriticalSection(&_lock);
}

void sirius::library::net::backend::cluster::core::backend_init(std::string version)
{
	bool ret = SSP_ADT.configure_load();
	if (ret == true)
	{
		set_hostname();
		CString gpu_name;
		DISPLAY_DEVICE display;
		memset(&display, 0, sizeof(DISPLAY_DEVICE));
		display.cb = sizeof(display);
		if (EnumDisplayDevices(NULL, 0, &display, 0))
			gpu_name = display.DeviceString;

		memset(_gpu_name, 0, MAX_PATH);
		int gpu_name_size = (int)wcslen(gpu_name);
		size_t converted_chars = 0;
		wcstombs_s(&converted_chars, _gpu_name, gpu_name_size + 1, gpu_name, _TRUNCATE);
		SSP_ADT.start(version);
	}
}

void sirius::library::net::backend::cluster::core::backend_stop()
{
	SSP_ADT.stop();
}

void sirius::library::net::backend::cluster::core::backend_client_connect(char* client_id, int32_t use_count, int32_t attendant_num)
{
	SSP_ADT.client_connect(client_id, use_count, attendant_num);

}

void sirius::library::net::backend::cluster::core::backend_client_disconnect(char* client_id, int32_t use_count, int32_t attendant_num)
{
	SSP_ADT.client_disconnect(client_id, use_count, attendant_num);
}

void sirius::library::net::backend::cluster::core::ssm_service_info(char* status, int32_t attendant_instance)
{
	if (SSP_ADT.is_cluster_use())
	{
		char ssm_data[BUF_SIZE] = { 0, };
		if (strcmp(status, "START") == 0)
		{
			SSP_ADT._max_attendant = attendant_instance;
			_snprintf(ssm_data, BUF_SIZE, "http://%s:%s/BC/SSMS/IFSSM_SERV_INFO.do?attendant_instance=%d&cpu_name=%s&gpu_name=%s&memory=%d&host_name=%s&os_version=%s&sirius_version=%s&sirius_ip=%s&sirius_status=%s",
				SSP_ADT.get_ssm_ip().c_str(), SSP_ADT.get_ssm_port().c_str(), attendant_instance, _cpu, _gpu_name, _memory, _host_name, _os, GEN_VER_VERSION_STRING, SSP_ADT._localip, status);
		}
		else
		{
			_snprintf(ssm_data, BUF_SIZE, "http://%s:%s/BC/SSMS/IFSSM_SERV_INFO.do?sirius_ip=%s&sirius_status=%s",
				SSP_ADT.get_ssm_ip().c_str(), SSP_ADT.get_ssm_port().c_str(), SSP_ADT._localip, status);
		}
		sirius::library::net::curl::client curl_ssm(SENDING_TIME);
		char url[100] = { 0, };
		curl_ssm.set_get_data(ssm_data, 0);
		bool res = curl_ssm.send();
		if (res == false)
		{
			int err = curl_ssm.get_send_err();
			if (err != CURLE_OPERATION_TIMEDOUT)
				LOGGER::make_info_log(SAA, "[[[backoffice data request]]] %s, %d, etc_error!!!!! ssm_serv_info (error_code:%d) url=%s", __FUNCTION__, __LINE__, err, ssm_data);
		}
		else
			LOGGER::make_info_log(SAA, "[[[backoffice data request]]] %s, %d, ssm_serv_info url=%s", __FUNCTION__, __LINE__, ssm_data);
	}
}

void sirius::library::net::backend::cluster::core::stop(void)
{
	_run = false;
	close_thread_pool();
	WSACleanup();
}

void sirius::library::net::backend::cluster::core::set_cluster_init(sirius::library::net::backend::cluster * client)
{
	_front = client;
	_snprintf_s(_ssp_url, 100, "http://%s:%s/BC/SSPS/IFSSP_STATUS_INFO.do", _ssp_ip.c_str(), _ssp_port.c_str());
	_curl_ssp = new sirius::library::net::curl::client(SENDING_TIME);
	_curl_ssp->set_url(_ssp_url, strlen(_ssp_url));

	_snprintf_s(_ssm_url, 100, "http://%s:%s/BC/SSPS/IFSSP_STATUS_INFO.do", _ssm_ip.c_str(), _ssm_port.c_str());
	_curl_ssm = new sirius::library::net::curl::client(SENDING_TIME);
	_curl_ssm->set_url(_ssm_url, strlen(_ssm_url));
}

void sirius::library::net::backend::cluster::core::set_sending_timeout(uint32_t timeout)
{
	_sending_timeout = timeout;
}

void sirius::library::net::backend::cluster::core::ssp_status_info(char * ssp_data)
{
	_curl_ssp->set_get_data(ssp_data, 0);
	DWORD dw_start_time = timeGetTime();
	bool res = _curl_ssp->send();
	DWORD dw_end_time = timeGetTime();
	if (res == false)
	{
		int err = _curl_ssp->get_send_err();
		if (err == CURLE_OPERATION_TIMEDOUT)
			LOGGER::make_info_log(SAA, "[[[backoffice data request]]] %s, %d, ssp_status_info sending_timeout:%d(ms), (error_code:%d) url=%s", __FUNCTION__, __LINE__, _curl_ssp->get_sending_timeout(), err, ssp_data);
		else
			LOGGER::make_info_log(SAA, "[[[backoffice data request]]] %s, %d, etc_error!!!!! ssp_status_info sending_timeout:%d(ms), (error_code:%d) url=%s", __FUNCTION__, __LINE__, _curl_ssp->get_sending_timeout(), err, ssp_data);
	}
	else
	{
		LOGGER::make_info_log(SAA, "[[[backoffice data request]]] %s, %d, sending_time:%d(ms), ssp_status_info=%s ", __FUNCTION__, __LINE__, dw_end_time - dw_start_time, ssp_data);
	}
}

DWORD sirius::library::net::backend::cluster::core::get_local_time(char * reg_time_date, char * reg_time_time)
{
	SYSTEMTIME _time;
	GetLocalTime(&_time);
	sprintf_s(reg_time_date, MAX_PATH, "%04d-%02d-%02d", _time.wYear, _time.wMonth, _time.wDay);
	sprintf_s(reg_time_time, MAX_PATH, "%02d:%02d:%02d", _time.wHour, _time.wMinute, _time.wSecond);
	return true;
}

bool sirius::library::net::backend::cluster::core::set_hostname()
{
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
		return false;

	char host_name[MAX_PATH] = { NULL, };
	char local_ip[MAX_PATH] = { NULL, };
	HOSTENT* phost_ent;

	if (gethostname(host_name, sizeof(host_name)) != NULL)
	{
		int err = WSAGetLastError();
		LOGGER::make_error_log(SAA, "%s, %d, err=%d", __FUNCTION__, __LINE__, err);
		return false;
	}
	if ((phost_ent = gethostbyname(host_name)) != NULL)
	{
		strcpy_s(local_ip, inet_ntoa(*(struct in_addr *)*phost_ent->h_addr_list));
	}
	_host_name_length = strlen(host_name);
	memcpy_s(_host_name, MAX_PATH, &host_name, MAX_PATH);
	return true;
}

void sirius::library::net::backend::cluster::core::close_thread_pool()
{
	std::vector<HANDLE>::iterator iter;
	for (iter = _threads.begin(); iter != _threads.end(); iter++)
	{
		HANDLE thread = (*iter);
		if (thread != INVALID_HANDLE_VALUE)
		{
			::WaitForSingleObjectEx(thread, 100, FALSE);
			::CloseHandle(thread);
		}
	}
	_threads.clear();
}