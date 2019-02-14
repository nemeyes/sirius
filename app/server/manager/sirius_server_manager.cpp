#include "sirius_server_manager.h"
#include "resource.h"
#include "server_manager.h"
#include "sirius_watchdog.h"
#include "sirius_stringhelper.h"
#include "sirius_log4cplus_logger.h"

using namespace ATL;

#include <stdio.h>
#include <fstream>
#include <iostream>

#define KB 1024
#define MB KB*KB
#define GB MB*KB
#pragma comment(lib,"version.lib")
typedef BOOL(WINAPI *P_GDFSE)(LPCTSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER);

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

LPCTSTR find_tokens(LPCTSTR p1, LPCTSTR p2)
{
	while (p1 != NULL && *p1 != NULL)
	{
		LPCTSTR p = p2;
		while (p != NULL && *p != NULL)
		{
			if (*p1 == *p)
				return CharNext(p1);
			p = CharNext(p);
		}
		p1 = CharNext(p1);
	}
	return NULL;
}

inline HRESULT sirius::app::server::manager::manager::enrollment_server(BOOL reg_type_lib, BOOL service)
{
	HRESULT res = CoInitialize(NULL);
	if (FAILED(res))
		return res;

	un_installation();
	UpdateRegistryFromResource(IDR_SIRIUSSERVERMANAGER, TRUE);

	CRegKey reg_key;
	LONG result = reg_key.Open(HKEY_CLASSES_ROOT, _T("AppID"), KEY_WRITE);
	if (result != ERROR_SUCCESS)
		return result;

	CRegKey key;
	key.Create(reg_key, _T("{37408648-4F24-4F23-AF29-2F22F5016AA0}"));
	result = key.Open(reg_key, _T("{37408648-4F24-4F23-AF29-2F22F5016AA0}"), KEY_WRITE);
	if (result != ERROR_SUCCESS)
		return result;
	key.DeleteValue(_T("LocalService"));

	if (service)
	{
		key.SetValue(_T("sirius_server_manager"), _T("LocalService"));
		key.SetValue(_T("-Service"), _T("ServiceParameters"));
		installation();
	}

	res = CComModule::RegisterServer(reg_type_lib);

	CoUninitialize();
	return res;
}


inline HRESULT sirius::app::server::manager::manager::unenrollment_server()
{
	HRESULT res = CoInitialize(NULL);
	if (FAILED(res))
		return res;

	UpdateRegistryFromResource(IDR_SIRIUSSERVERMANAGER, FALSE);
	un_installation();

	CRegKey reg_key;
	LONG lRes = reg_key.Open(HKEY_CLASSES_ROOT, _T("AppID"), KEY_WRITE);
	reg_key.DeleteSubKey(_T("{37408648-4F24-4F23-AF29-2F22F5016AA0}"));
	CComModule::UnregisterServer(TRUE);
	CoUninitialize();

	return S_OK;
}

inline void sirius::app::server::manager::manager::init(_ATL_OBJMAP_ENTRY* p, HINSTANCE h, UINT service_name, const GUID* lib_id)
{
	CComModule::Init(p, h, lib_id);

	_is_service = TRUE;

	LoadString(h, service_name, _service_name, sizeof(_service_name) / sizeof(TCHAR));

	_service_status = NULL;
	_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	_status.dwCurrentState = SERVICE_STOPPED;
	_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	_status.dwWin32ExitCode = 0;
	_status.dwServiceSpecificExitCode = 0;
	_status.dwCheckPoint = 0;
	_status.dwWaitHint = 0;
}

LONG sirius::app::server::manager::manager::un_lock()
{
	LONG i = CComModule::Unlock();
	if (i == 0 && !_is_service)
		PostThreadMessage(_thread_id, WM_QUIT, 0, 0);
	return i;
}

BOOL sirius::app::server::manager::manager::is_installation()
{
	BOOL result = FALSE;

	SC_HANDLE sc_handle = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (sc_handle != NULL)
	{
		SC_HANDLE sc_service = ::OpenService(sc_handle, _service_name, SERVICE_QUERY_CONFIG);
		if (sc_service != NULL)
		{
			result = TRUE;
			::CloseServiceHandle(sc_service);
		}
		::CloseServiceHandle(sc_handle);
	}
	return result;
}

inline BOOL sirius::app::server::manager::manager::installation()
{
	if (is_installation())
		return TRUE;

	SC_HANDLE sc_handle = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (sc_handle == NULL)
	{		
		LOGGER::make_error_log(SAW, "%s(), %d, Could open Service Manager. error_code:%d", __FUNCTION__, __LINE__, GetLastError());
		return FALSE;
	}
	TCHAR file_path[_MAX_PATH];
	::GetModuleFileName(NULL, file_path, _MAX_PATH);

	SC_HANDLE sc_service = ::CreateService(
		sc_handle, _service_name, _service_name,
		SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
		SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE,
		file_path, NULL, NULL, _T("RPCSS\0"), NULL, NULL);

	if (sc_service == NULL)
	{
		::CloseServiceHandle(sc_handle);
		LOGGER::make_error_log(SAW, "%s(), %d, Could not create Service. error_code:%d", __FUNCTION__, __LINE__, GetLastError());
		return FALSE;
	}

	::CloseServiceHandle(sc_service);
	::CloseServiceHandle(sc_handle);
	return TRUE;
}

inline BOOL sirius::app::server::manager::manager::un_installation()
{
	if (!is_installation())
		return TRUE;

	SC_HANDLE sc_handle = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (sc_handle == NULL)
	{		
		LOGGER::make_error_log(SAW, "%s(), %d, Could not open Service Manager. error_code:%d", __FUNCTION__, __LINE__, GetLastError());
		return FALSE;
	}

	SC_HANDLE sc_service = ::OpenService(sc_handle, _service_name, SERVICE_STOP | DELETE);

	if (sc_service == NULL)
	{
		::CloseServiceHandle(sc_handle);
		LOGGER::make_error_log(SAW, "%s(), %d, Could not open Service. error_code: %d", __FUNCTION__, __LINE__, GetLastError());
		return FALSE;
	}
	SERVICE_STATUS status;
	::ControlService(sc_service, SERVICE_CONTROL_STOP, &status);

	BOOL bDelete = ::DeleteService(sc_service);
	::CloseServiceHandle(sc_service);
	::CloseServiceHandle(sc_handle);

	if (bDelete)
		return TRUE;
	
	LOGGER::make_error_log(SAW, "%s(), %d, Unable to delete Service. error_code:%d", __FUNCTION__, __LINE__, GetLastError());

	return FALSE;
}

inline void sirius::app::server::manager::manager::service_start()
{
	SERVICE_TABLE_ENTRY st[] =
	{
		{ _service_name, _service_main },
		{ NULL, NULL }
	};
	if (_is_service && !::StartServiceCtrlDispatcher(st))
	{
		_is_service = FALSE;
	}
	if (_is_service == FALSE)
		run();
}

inline void sirius::app::server::manager::manager::service_main(DWORD , LPTSTR*)
{
	_status.dwCurrentState = SERVICE_START_PENDING;
	_service_status = RegisterServiceCtrlHandler(_service_name, _control_handler);
	if (_service_status == NULL)
	{		
		LOGGER::make_error_log(SAW, "%s(), %d, The handler is not installed", __FUNCTION__, __LINE__);
		return;
	}
	set_service_condition(SERVICE_START_PENDING);

	_status.dwWin32ExitCode = S_OK;
	_status.dwCheckPoint = 0;
	_status.dwWaitHint = 0;
		
	LOGGER::make_info_log(SAW, "%s(), %d, Service Start", __FUNCTION__, __LINE__);
	run();

	set_service_condition(SERVICE_STOPPED);
	LOGGER::make_info_log(SAW, "%s(), %d, Service Stop", __FUNCTION__, __LINE__);

}

inline void sirius::app::server::manager::manager::control_handler(DWORD code)
{
	switch (code)
	{
	case SERVICE_CONTROL_STOP:
		set_service_condition(SERVICE_STOP_PENDING);
		PostThreadMessage(_thread_id, WM_QUIT, 0, 0);
		break;
	case SERVICE_CONTROL_PAUSE:
		break;
	case SERVICE_CONTROL_CONTINUE:
		break;
	case SERVICE_CONTROL_INTERROGATE:
		break;
	case SERVICE_CONTROL_SHUTDOWN:
		break;
	default:
		LOGGER::make_error_log(SAW, "%s(), %d, Inappropriate service request!!", __FUNCTION__, __LINE__);
		break;
	}
}

void WINAPI sirius::app::server::manager::manager::_service_main(DWORD dw_argc, LPTSTR* lpstr_argv)
{
	_proc.service_main(dw_argc, lpstr_argv);
}
void WINAPI sirius::app::server::manager::manager::_control_handler(DWORD code)
{
	_proc.control_handler(code);
}

void sirius::app::server::manager::manager::set_service_condition(DWORD state)
{
	_status.dwCurrentState = state;
	::SetServiceStatus(_service_status, &_status);
}


void sirius::app::server::manager::manager::run()
{
	_proc._thread_id = GetCurrentThreadId();
	HRESULT result = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	_ASSERTE(SUCCEEDED(result));
	CSecurityDescriptor cd;
	cd.InitializeFromThreadToken();
	result = CoInitializeSecurity(cd, -1, NULL, NULL,
		RPC_C_AUTHN_LEVEL_PKT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
	_ASSERTE(SUCCEEDED(result));

	result = _proc.RegisterClassObjects(CLSCTX_LOCAL_SERVER | CLSCTX_REMOTE_SERVER, REGCLS_MULTIPLEUSE);
	_ASSERTE(SUCCEEDED(result));
	
	if (_is_service)
		set_service_condition(SERVICE_RUNNING);
	
	sirius::app::server::manager::monitor *core = new sirius::app::server::manager::monitor();
	core->init();	
	MSG msg;
	while (GetMessage(&msg, 0, 0, 0))
		DispatchMessage(&msg);

	_proc.RevokeClassObjects();

	if(core)
		delete core;
	
	CoUninitialize();	
}

extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE,
	LPTSTR lpCmdLine, int nShowCmd)
{
	DEVMODE Mode;
	ZeroMemory(&Mode, sizeof(Mode));
	Mode.dmSize = sizeof(Mode);
	Mode.dmPelsWidth = 1920;
	Mode.dmPelsHeight = 1080;
	Mode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
	int res = ChangeDisplaySettings(&Mode, 0);
	Sleep(5000);

	sirius::library::log::log4cplus::logger::create("configuration\\sirius_log_configuration.ini", SAW, "");
	lpCmdLine = GetCommandLine();
	_proc.init(ObjectMap, hInstance, IDS_SERVICENAME, &LIBID_SERVERMANAGERLIB);
	_proc._is_service = TRUE;

	TCHAR tokens[] = _T("-/");
	HRESULT result;
	LPCTSTR lps_tokens = find_tokens(lpCmdLine, tokens);
	while (lps_tokens != NULL)
	{
		if (lstrcmpi(lps_tokens, _T("UnRegServer")) == 0)
		{
			result = _proc.unenrollment_server();
			if (SUCCEEDED(result))
			{
				LOGGER::make_info_log(SAW, "%s(), %d, UnregServer Successed!.", __FUNCTION__, __LINE__);
			}
			else
			{
				LOGGER::make_info_log(SAW, "%s(), %d, UnregServer Failed!.", __FUNCTION__, __LINE__);
			}
			return result;
		}

		else if (lstrcmpi(lps_tokens, _T("RegServer")) == 0)
		{
			result = _proc.enrollment_server(TRUE, TRUE);
			if (SUCCEEDED(result))
			{
				LOGGER::make_info_log(SAW, "%s(), %d, UnregServer Successed!.", __FUNCTION__, __LINE__);
			}
			else
			{
				LOGGER::make_info_log(SAW, "%s(), %d, UnregServer Failed!.", __FUNCTION__, __LINE__);
			}
			return result;
		}
		else
		{
			return S_FALSE;
		}
		lps_tokens = find_tokens(lps_tokens, tokens);
	}

	CRegKey reg_key;
	LONG lRes = reg_key.Open(HKEY_CLASSES_ROOT, _T("AppID"), KEY_READ);
	if (lRes != ERROR_SUCCESS)
		return lRes;

	CRegKey key;
	lRes = key.Open(reg_key, _T("{37408648-4F24-4F23-AF29-2F22F5016AA0}"), KEY_READ);
	if (lRes != ERROR_SUCCESS)
		return lRes;

	TCHAR szValue[_MAX_PATH];
	DWORD dwLen = _MAX_PATH;
	lRes = key.QueryValue(szValue, _T("LocalService"), &dwLen);

	_proc._is_service = FALSE;
	if (lRes == ERROR_SUCCESS)
		_proc._is_service = TRUE;

	_proc.service_start();

	return _proc._status.dwWin32ExitCode;

}

