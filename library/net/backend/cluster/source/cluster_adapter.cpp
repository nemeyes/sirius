#include "cluster_adapter.h"
#include "sirius_log4cplus_logger.h"
#include "sirius.h"
#include <shlwapi.h>
#include <tchar.h>
#include <process.h>
#include <mmSystem.h>
#include <wininet.h>
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Wininet.lib")

#define TARGET_RESOLUTION				1
//#define EXTERNALIP_FINDER_URL "http://checkip.dyndns.org/"
#define TITLE_PARSER _T("<body>Current IP Address: ")

int32_t sirius::library::net::backend::cluster_adapter::start(std::string version)
{
	int video_width = 1280;
	int video_height = 720;
	int bitrate = 3000000;
	int framerate = 3000;
	int sending_timeout = 0;
	
	_client = nullptr;
	_backend_stat = false;
	_ssp_event_push = nullptr;
	_system_user = FALSE;
	_client_count = NULL;
	_ssp_thread_exit = FALSE;
	_ssp_url_st[0] = NULL;
	_stat_timer = false;
	memset(_localip, 0, MAX_PATH);
	//configure_load();
	if (is_cluster_use())
	{
		const char *file_name = "webapp";
		sirius::library::net::backend::cluster * self = new sirius::library::net::backend::cluster();
		sending_timeout = 20;
		_client = (sirius::library::net::backend::cluster *)(self);
		_client->set_cluster_init(_client);
		_client->set_sending_timeout(sending_timeout);
		
		sirius::library::net::backend::cluster_adapter::cluster_adapter_configuration_t backend_config;
		backend_config.ssp_ip = get_ssp_ip();
		backend_config.ssp_port = get_ssp_port();
		backend_config.ssm_ip = get_ssm_ip();
		backend_config.ssm_port = get_ssm_port();
		_idc_code = get_idc_code();

		unsigned int thread_id = 0;
		_ssp_event_push = CreateEvent(NULL, FALSE, FALSE, NULL);
		HANDLE ssp_thread_handle = (HANDLE)_beginthreadex(NULL, 0, &sirius::library::net::backend::cluster_adapter::ssp_queue_thread, this, 0, &thread_id);
		for (int i = 0; i < QUEUE_MAX_SIZE; i++)
		{
			_ssp_url_st[i] = new char[MAX_BACKOFFICE_DATA_BUFFER];
			memset(_ssp_url_st[i], 0, sizeof(char) * MAX_BACKOFFICE_DATA_BUFFER);
		}
		_ssp_bufpos = 0;
		set_external_ip(_localip);
		Sleep(100);
		/*if (strlen(_localip) != 0)
		{
			_snprintf(_ssp_url_st[_ssp_bufpos], MAX_BACKOFFICE_DATA_BUFFER, "http://%s:%s/SSPS/IFSSP_STATUS_INFO.do?ip=%s&count=%I64d&id=SIRIUS&status=OPEN&client_id=NULL",
				get_ssp_ip().c_str(), get_ssp_port().c_str(), _localip, _client_count);
			_ssp_queue.push_back(_ssp_url_st[_ssp_bufpos]);
			if (_ssp_bufpos++ == QUEUE_MAX_SIZE - 1)
				_ssp_bufpos = 0;
		}*/
		if (_stat_timer == NULL)
		{
			TIMECAPS _timecaps;
			if (timeGetDevCaps(&_timecaps, sizeof(TIMECAPS)) != TIMERR_NOERROR)
				return -1;

			uint32_t _timer_res = min(max(_timecaps.wPeriodMin, TARGET_RESOLUTION), _timecaps.wPeriodMax);

			if (timeBeginPeriod(_timer_res) != TIMERR_NOERROR)
				return -1;

			std::string::size_type sz;
			_keepalive_delay_time = std::stoi(get_alive_interval(), &sz) * 1000;
			_keepalive_timeset_event = timeSetEvent(_keepalive_delay_time, _timer_res, &timer_keepalive, (DWORD_PTR)this, TIME_PERIODIC | TIME_KILL_SYNCHRONOUS);
			_stat_timer = true;
		}
	}
	return 0;
}

void sirius::library::net::backend::cluster_adapter::stop()
{
	if (is_cluster_use())
	{
		_client_count = NULL;
		_ssp_queue.clear();
		/*if (strlen(_localip) > 0)
		{
			char ssp_data[MAX_BACKOFFICE_DATA_BUFFER] = { 0, };
			_snprintf_s(ssp_data, sizeof(ssp_data), "http://%s:%s/SSPS/IFSSP_STATUS_INFO.do?ip=%s&count=%I64d&id=SIRIUS&status=CLOSE&client_id=NULL",
				get_ssp_ip().c_str(), get_ssp_port().c_str(), _localip, _client_count);
			sirius::library::net::curl::client curl_ssp(30);
			char url[100] = { 0, };
			curl_ssp.set_get_data(ssp_data, 0);
			DWORD dwStartTime = timeGetTime();
			bool res = curl_ssp.send();
			DWORD dwEndTime = timeGetTime();
			if (res == false)
				LOGGER::make_info_log(SAA, "[[[backoffice data request]]] %s(), %d ssp sending_timeout:%d(ms), ssp_status_info=%s", __FUNCTION__, __LINE__, dwEndTime - dwStartTime, ssp_data);
			else
			{
				LOGGER::make_info_log(SAA, "[[[backoffice data request]]] %s, %d, sending_time:%d(ms), ssp_status_info=%s ", __FUNCTION__, __LINE__, dwEndTime - dwStartTime, ssp_data);
			}
		}*/
		_stat_timer = false;
		backend_deinit();
	}
}

void sirius::library::net::backend::cluster_adapter::backend_deinit()
{
	if (_keepalive_timeset_event) {
		timeKillEvent(_keepalive_timeset_event);
		_keepalive_timeset_event = 0;
	}
	if (_ssp_event_push != nullptr)
	{
		_ssp_queue.clear();
		LOGGER::make_info_log(SAA, "%s, %d, ssp_queue.size:%d", __FUNCTION__, __LINE__, _ssp_queue.size());
		::CloseHandle(_ssp_event_push);
		_ssp_event_push = nullptr;
	}
	_ssp_thread_exit = TRUE;

	if (_ssp_url_st[0] != nullptr)
	{
		for (int i = 0; i < QUEUE_MAX_SIZE; i++)
		{
			delete[] _ssp_url_st[i];
			_ssp_url_st[i] = nullptr;
		}
	}
	if (_client != NULL)
	{
		_client->stop();
		delete _client;
		_client = nullptr;
	}
}

int32_t sirius::library::net::backend::cluster_adapter::client_connect(char* client_id, int32_t use_count, int32_t attendant_num)
{
		if (is_cluster_use())
		{
			char connect_date[MAX_PATH];
			char connect_time[MAX_PATH];
			_client->get_local_time(connect_date, connect_time);
			if (_client_count < _max_attendant)
				_client_count++;
			if (_client_count < 0)
				_client_count = 0;
			if (_ssp_queue.size() > QUEUE_MAX_SIZE)
			{
				LOGGER::make_error_log(SAA, "%s, %d, ssp_queue buffer full ssp_queue:%d", __FUNCTION__, __LINE__, _ssp_queue.size());
				_ssp_queue.clear();
				_ssp_bufpos = 0;
			}
			_snprintf(_ssp_url_st[_ssp_bufpos], MAX_BACKOFFICE_DATA_BUFFER, "http://%s:%s/SSPS/IFSSP_STATUS_INFO.do?ip=%s&count=%d&id=SIRIUS&status=OPEN&client_id=%s",
				get_ssp_ip().c_str(), get_ssp_port().c_str(), _localip, use_count, client_id);
			_ssp_queue.push_back(_ssp_url_st[_ssp_bufpos]);
			if (_ssp_bufpos++ == QUEUE_MAX_SIZE - 1)
				_ssp_bufpos = 0;

			if (_client_count >= _max_attendant * (1.0 - 5.0 / 100.0))
				LOGGER::make_error_log(SAA, "%s(),%d [CRITICAL] attendant count danger(error_code:%d) max_attendant_count %.0f%% max=%d, attendant_count=%I64d", __FUNCTION__, __LINE__, sirius::base::err_code_t::attendant_count_danger_ct, ((double)_client_count / (double)_max_attendant * 100), _max_attendant, _client_count);
			else if (_client_count >= _max_attendant * (1.0 - 10.0 / 100.0))
				LOGGER::make_error_log(SAA, "%s(),%d [MAJOR] attendant count danger(error_code:%d) max_attendant_count %.0f%% max=%d, attendant_count=%I64d", __FUNCTION__, __LINE__, sirius::base::err_code_t::attendant_count_danger_mg, ((double)_client_count / (double)_max_attendant * 100), _max_attendant, _client_count);
			else if (_client_count >= _max_attendant * (1.0 - 20.0 / 100.0))
				LOGGER::make_error_log(SAA, "%s(),%d [MINOR] attendant count danger(error_code:%d) max_attendant_count %.0f%% max=%d, attendant_count=%I64d", __FUNCTION__, __LINE__, sirius::base::err_code_t::attendant_count_danger_mn, ((double)_client_count / (double)_max_attendant * 100), _max_attendant, _client_count);
		}

	return true;
}

int32_t sirius::library::net::backend::cluster_adapter::client_disconnect(char* client_id, int32_t use_count, int32_t attendant_num)
{
	if (is_cluster_use())
	{
		char connect_date[MAX_PATH];
		char connect_time[MAX_PATH];
		_client->get_local_time(connect_date, connect_time);
		_client_count--;
		if (_client_count < 0)
			_client_count = 0;

		if (_ssp_queue.size() > QUEUE_MAX_SIZE)
		{
			LOGGER::make_error_log(SAA, "%s, %d, ssp_queue buffer full ssp_queue:%d", __FUNCTION__, __LINE__, _ssp_queue.size());
			_ssp_queue.clear();
			_ssp_bufpos = 0;
		}
		_snprintf(_ssp_url_st[_ssp_bufpos], MAX_BACKOFFICE_DATA_BUFFER, "http://%s:%s/SSPS/IFSSP_STATUS_INFO.do?ip=%s&count=%d&id=SIRIUS&status=CLOSE&client_id=%s&idc=%s",
			get_ssp_ip().c_str(), get_ssp_port().c_str(), _localip, use_count, client_id, get_idc_code().c_str());
		_ssp_queue.push_back(_ssp_url_st[_ssp_bufpos]);
		if (_ssp_bufpos++ == QUEUE_MAX_SIZE - 1)
			_ssp_bufpos = 0;
	}

	return true;
}

bool sirius::library::net::backend::cluster_adapter::configure_load()
{
	TCHAR szFileName[MAX_PATH];
	::GetModuleFileName(NULL, szFileName, MAX_PATH);
	::PathRemoveFileSpec(szFileName);
	::PathAppend(szFileName, _T(CONFIG_FILE));

	if (!::PathFileExists(szFileName))
		return false;

	char *ssp_ip, *ssp_port, *ssm_ip, *ssm_port, *idc_code, *use_cluster, *alive_interval;
	TCHAR value[MAX_PATH] = { 0 };

	::GetPrivateProfileString(_T("CLUSTER"), _T("USE_CLUSTER"), _T("ON"), value, MAX_PATH, szFileName);
	convert_wide2multibyte(value, &use_cluster);
	_use_cluster = strcmp(use_cluster, "ON") == 0 ? true : false;
	delete use_cluster;

	::GetPrivateProfileString(_T("CLUSTER"), _T("SSP_IP"), _T("0.0.0.0"), value, MAX_PATH, szFileName);
	convert_wide2multibyte(value, &ssp_ip);
	_ssp_ip = ssp_ip;
	delete ssp_ip;

	::GetPrivateProfileString(_T("CLUSTER"), _T("SSP_PORT"), _T("0"), value, MAX_PATH, szFileName);
	convert_wide2multibyte(value, &ssp_port);
	_ssp_port = ssp_port;
	delete ssp_port;

	::GetPrivateProfileString(_T("CLUSTER"), _T("SSM_IP"), _T("0.0.0.0"), value, MAX_PATH, szFileName);
	convert_wide2multibyte(value, &ssm_ip);
	_ssm_ip = ssm_ip;
	delete ssm_ip;

	::GetPrivateProfileString(_T("CLUSTER"), _T("SSM_PORT"), _T("0"), value, MAX_PATH, szFileName);
	convert_wide2multibyte(value, &ssm_port);
	_ssm_port = ssm_port;
	delete ssm_port;

	::GetPrivateProfileString(_T("CLUSTER"), _T("IDC_CODE"), _T("0"), value, MAX_PATH, szFileName);
	convert_wide2multibyte(value, &idc_code);
	_idc_code = idc_code;
	delete idc_code;

	::GetPrivateProfileString(_T("CLUSTER"), _T("ALIVE_INTERVAL"), _T("60"), value, MAX_PATH, szFileName);
	convert_wide2multibyte(value, &alive_interval);
	_alive_interval = alive_interval;
	delete alive_interval;


	return true;
}

bool sirius::library::net::backend::cluster_adapter::set_external_ip(char * ip)
{
	HINTERNET internet, file;
	DWORD size;
	char buffer[MAX_PATH] = { 0 };
	char title_parser[MAX_PATH] = { 0 };
	LPCWSTR external_ip_finder_url = L"http://checkip.dyndns.org/";
	sprintf_s(title_parser, MAX_PATH, "%s", "<body>Current IP Address: ");
	internet = InternetOpen(NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (NULL == internet)
		return 0;

	file = InternetOpenUrl(internet, external_ip_finder_url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
	if (file)
	{
		InternetReadFile(file, &buffer, sizeof(buffer), &size);
		buffer[size] = '\0';
		int shift = strlen(title_parser);
		std::string str_html = buffer;
		std::string::size_type nIdx = str_html.find(title_parser);
		str_html.erase(str_html.begin(), str_html.begin() + nIdx + shift);
		nIdx = str_html.find("</body>");
		str_html.erase(str_html.begin() + nIdx, str_html.end());

		wchar_t _wtext[MAX_PATH];
		std::wstring widestr = std::wstring(str_html.begin(), str_html.end());
		const wchar_t* widecstr = widestr.c_str();

		_tcscpy(_wtext, widecstr);
		WideCharToMultiByte(CP_ACP, 0, _wtext, -1, ip, wcslen(_wtext), 0, 0);

		InternetCloseHandle(file);
		InternetCloseHandle(internet);
	}

	return 0;
}

unsigned sirius::library::net::backend::cluster_adapter::ssp_queue_thread(void * param)
{
	cluster_adapter * self = static_cast<cluster_adapter*>(param);
	int ssp_event_check = 0;
	char ssp_data[1024] = { 0, };
	while (self->_ssp_thread_exit == FALSE)
	{
		ssp_event_check = WaitForSingleObject(self->_ssp_event_push, 1);

		if (ssp_event_check == WAIT_FAILED)
		{
			LOGGER::make_error_log(SAA, "%s, %d, ssp_queue_thread event_check WAIT_FAILED!!!!", __FUNCTION__, __LINE__);
			break;
		}
		else if (ssp_event_check == WAIT_TIMEOUT) {
			if (self->_ssp_queue.size() > QUEUE_MAX_SIZE)
			{
				LOGGER::make_error_log(SAA, "%s, %d, ssp_queue buffer full ssp_queue:%d", __FUNCTION__, __LINE__, self->_ssp_queue.size());
				self->_ssp_queue.clear();
			}
			if (self->_ssp_queue.size() > 0)
			{
				LOGGER::make_info_log(SAA, "%s, %d, _ssp_queue size : %d", __FUNCTION__, __LINE__, self->_ssp_queue.size());
				_snprintf_s(ssp_data, sizeof(ssp_data), "%s", self->_ssp_queue.front());
				self->_client->ssp_status_info(ssp_data);
				self->_ssp_queue.pop_front();
			}
			continue;
		}
	}
	self->_ssp_thread_exit = FALSE;
	LOGGER::make_info_log(SAA, "ssp_queue_thread  exit!!!!");

	return 0;
}

void CALLBACK timer_keepalive(uint32_t ui_id, uint32_t ui_msg, DWORD_PTR dw_user, DWORD_PTR dw1, DWORD_PTR dw2)
{
	char ssm_data[MAX_PATH] = { 0, };
	_snprintf(ssm_data, MAX_PATH, "http://%s:%s/SSMS/IFSSM_SERV_INFO.do?sirius_ip=%s&sirius_status=ALIVE",
		SSP_ADT.get_ssm_ip().c_str(), SSP_ADT.get_ssm_port().c_str(), SSP_ADT._localip);
	sirius::library::net::curl::client curl_ssm(SENDING_TIME);
	char url[100] = { 0, };
	curl_ssm.set_get_data(ssm_data, 0);
	bool res = curl_ssm.send();
	LOGGER::make_info_log(SAA, "[[[backoffice data request]]] %s, %d, ssm_serv_info url=%s", __FUNCTION__, __LINE__, ssm_data);
}
