#include "stdafx.h"


#include <sstream>
#include <fstream>

#include <json/json.h>

#include "configure.h"

#define CONFIG_FILE "\\configuration\\sirius_client.ini"

configure::configure()
{
}

configure::~configure()
{
}

bool configure::load()
{			
	TCHAR szFileName[MAX_PATH];
	::GetModuleFileName(NULL, szFileName, MAX_PATH);
	::PathRemoveFileSpec(szFileName);
	::PathAppend(szFileName, _T(CONFIG_FILE));

	if (!::PathFileExists(szFileName))
		return false;

	TCHAR value[MAX_PATH] = { 0 };

	::GetPrivateProfileString(_T("SERVER"), _T("ADDRESS"), _T("127.0.0.1"), value, MAX_PATH, szFileName);
	_server_address = value;

	::GetPrivateProfileString(_T("SERVER"), _T("PORT"), _T("3390"), value, MAX_PATH, szFileName);
	_server_port = value;

	::GetPrivateProfileString(_T("ATTENDANT"), _T("DEVICE_ID"), _T("aa:bb:cc:dd:ff:ee"), value, MAX_PATH, szFileName);
	_attendant_device_id = value;

	::GetPrivateProfileString(_T("PLAY"), _T("URL"), _T("127.0.0.1"), value, MAX_PATH, szFileName);
	_url = value;

	::GetPrivateProfileString(_T("PLAY"), _T("PORT"), _T("3400"), value, MAX_PATH, szFileName);
	_port = value;

	::GetPrivateProfileString(_T("CSR"), _T("URL"), _T("http://10.202.142.68:8290/CSRS/IFCSR_ROUTE_INFO.action"), value, MAX_PATH, szFileName);
	_csr_url = value;

	::GetPrivateProfileString(_T("CSR"), _T("SO_CODE"), _T("45"), value, MAX_PATH, szFileName);
	_csr_so_code = value;

 	return true;
}

bool configure::save()
{
	TCHAR szFileName[MAX_PATH];
	::GetModuleFileName(NULL, szFileName, MAX_PATH);
	::PathRemoveFileSpec(szFileName);
	::PathAppend(szFileName, _T(CONFIG_FILE));

	::WritePrivateProfileString(_T("SERVER"), _T("ADDRESS"),_server_address.c_str(), szFileName);
	::WritePrivateProfileString(_T("SERVER"), _T("PORT"), _server_port.c_str(), szFileName);
	::WritePrivateProfileString(_T("ATTENDANT"), _T("DEVICE_ID"), _attendant_device_id.c_str(), szFileName);
	::WritePrivateProfileString(_T("PLAY"), _T("URL"), _url.c_str(), szFileName);
	::WritePrivateProfileString(_T("PLAY"), _T("PORT"), _port.c_str(), szFileName);
	::WritePrivateProfileString(_T("CSR"), _T("URL"), _csr_url.c_str(), szFileName);
	::WritePrivateProfileString(_T("CSR"), _T("SO_CODE"), _csr_so_code.c_str(), szFileName);

	return true;
}

sirius::string & configure::get_attendant_device_id()
{
	return _attendant_device_id;
};
