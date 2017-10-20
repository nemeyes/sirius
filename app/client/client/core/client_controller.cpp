#include "stdafx.h"
#include "SiriusClient.h"
#include "SiriusClientDlg.h"
#include "client_controller.h"
#include <json/json.h>
#include <sirius_xml_parser.h>
#include <sirius_stringhelper.h>

#include <memory>
#include <string>

typedef sirius::library::framework::client::base * (*fpn_create_client_framework)();
typedef void(*fpn_destory_client_framework)(sirius::library::framework::client::base ** client_framework);

client_controller::client_controller(CSiriusClientDlg * front)
	: _front(front)
	, _hmodule(NULL)
	, _framework(NULL)
{
	HINSTANCE inst = AfxGetInstanceHandle();
	HWND hwnd = _front->GetSafeHwnd();
	_controller = new sirius::app::client::proxy(this, inst, hwnd);
	_controller->set_key_stroke(_front->_keystroke_interval);
}

client_controller::~client_controller(void)
{
	if (_controller)
	{
		delete _controller;
		_controller = nullptr;
	}
}

void client_controller::on_pre_connect(wchar_t * address, int32_t portNumber, BOOL reconnection)
{
	HWND hwnd = _front->GetSafeHwnd();
	::PostMessage(hwnd, WM_CONNECTION_BEGIN_MESSAGE, 0, 0);
}

void client_controller::on_post_connect(wchar_t * address, int32_t portNumber, BOOL reconnection)
{
	HWND hwnd = _front->GetSafeHwnd();

	if (!_framework)
	{
		HINSTANCE module_handle = ::GetModuleHandleA("sirius_client.exe");
		char module_path[MAX_PATH] = { 0 };
		char * module_name = module_path;
		module_name += GetModuleFileNameA(module_handle, module_name, (sizeof(module_path) / sizeof(*module_path)) - (module_name - module_path));
		if (module_name != module_path)
		{
			CHAR * slash = strrchr(module_path, '\\');
			if (slash != NULL)
			{
				module_name = slash + 1;
				_strset_s(module_name, strlen(module_name) + 1, 0);
			}
			else
			{
				_strset_s(module_path, strlen(module_path) + 1, 0);
			}
		}
		if (strlen(module_path)>0)
		{
			SetDllDirectoryA(module_path);
			_hmodule = ::LoadLibraryA("sirius_native_client_framework.dll");
			if (_hmodule)
			{
				fpn_create_client_framework pfn_create = (fpn_create_client_framework)::GetProcAddress(_hmodule, "create_client_framework");
				if (pfn_create)
				{
					_framework = (pfn_create)();
					if (!_framework)
					{
						FreeLibrary(_hmodule);
						_hmodule = NULL;
					}
				}
				else
				{
					FreeLibrary(_hmodule);
					_hmodule = NULL;
				}
			}
		}
	}
}

void client_controller::on_pre_disconnect(void)
{
	HWND hwnd = _front->GetSafeHwnd();
	::PostMessage(hwnd, WM_DISCONNECTION_BEGIN_MESSAGE, 0, 0);
}

void client_controller::on_post_disconnect(void)
{
	if (_hmodule)
	{
		fpn_destory_client_framework pfn_destroy = (fpn_destory_client_framework)::GetProcAddress(_hmodule, "destroy_client_framework");
		if (_framework)
		{
			_framework->stop();
			(pfn_destroy)(&_framework);
			_framework = nullptr;
		}
		FreeLibrary(_hmodule);
		_hmodule = NULL;
	}
}

void client_controller::on_pre_create_session(void)
{
	HWND hwnd = _front->GetSafeHwnd();
	::PostMessage(hwnd, WM_CONNECTION_END_MESSAGE, 0, 0);
}

void client_controller::on_create_session(void)
{
	HWND hwnd = _front->GetSafeHwnd();
	::PostMessage(hwnd, WM_CREATING_SLOT_BEGIN_MESSAGE, 0, 0);

	CString appId = L"";
	CString deviceId = L"";
	CString deviceType = L"";
	CString envirType = L"";
	CString modelType = L"";
	int32_t slot_resolution = 0;
	int32_t width = 0;
	int32_t height = 0;

	_front->_ctrl_client_app_id.GetWindowTextW(appId);
	_front->_ctrl_client_device_id.GetWindowTextW(deviceId);
	_front->_ctrl_client_device_type.GetWindowTextW(deviceType);
	_front->_ctrl_client_environment_type.GetWindowTextW(envirType);
	_front->_ctrl_client_model_name.GetWindowTextW(modelType);
	slot_resolution = _front->_ctrl_attendant_resolution.GetCurSel();


	if (slot_resolution == 0)	//720p
	{
		width = 1280;
		height = 720;
	}
	else if (slot_resolution == 1)	//1080p
	{
		width = 1920;
		height = 1080;
	}
	else if (slot_resolution == 2)	//4k
	{
		width = 3840;
		height = 2160;
	}
	else if (slot_resolution == 3)	//8k
	{
		width = 7680;
		height = 4320;
	}

	connect_attendant((LPWSTR)(LPCWSTR)appId, (LPWSTR)(LPCWSTR)deviceId, (LPWSTR)(LPCWSTR)deviceType, (LPWSTR)(LPCWSTR)envirType, (LPWSTR)(LPCWSTR)modelType, width, height, -1);
}

void client_controller::on_post_create_session(void)
{

}

void client_controller::on_pre_keepalive(void)
{

}

void client_controller::on_keepalive(void)
{

}

void client_controller::on_post_keepalive(void)
{

}

void client_controller::on_pre_destroy_session(void)
{
	HWND hwnd = _front->GetSafeHwnd();
	::PostMessage(hwnd, WM_DISCONNECTION_END_MESSAGE, 0, 0);
}

void client_controller::on_destroy_session(void)
{
	if(_framework)
		_framework->stop();
}

void client_controller::on_post_destroy_session(void)
{

}

void client_controller::on_pre_connect_attendant(int32_t code, wchar_t * msg)
{
	HWND hwnd = _front->GetSafeHwnd();
	if (code == 0)
	{
		::PostMessage(hwnd, WM_CREATING_SLOT_END_MESSAGE, 0, 0);
	}
	else
	{
		MessageBox(hwnd, msg, _T("ERROR"), MB_OK);
	}
}

void client_controller::on_connect_attendant(int32_t code, wchar_t * msg)
{

}

void client_controller::on_post_connect_attendant(int32_t code, wchar_t * msg)
{

}

void client_controller::on_pre_disconnect_attendant(void)
{

}

void client_controller::on_disconnect_attendant(void)
{

}

void client_controller::on_post_disconnect_attendant(void)
{

}

void client_controller::on_pre_attendant_info(int32_t code, wchar_t * attendant_uuid, wchar_t * streamer_address, int32_t streamer_portnumber)
{
	wchar_t title[500] = { 0 };
	_snwprintf_s(title, sizeof(title), L"sirius_client attendant_uuid=%s, port=%d", attendant_uuid, streamer_portnumber);
	_front->SetWindowTextW(title);
}

void client_controller::on_post_attendant_info(int32_t code, wchar_t * attendant_uuid, wchar_t * streamer_address, int32_t streamer_portnumber)
{

}

void client_controller::on_open_streaming(wchar_t * attendant_uuid, wchar_t * streamer_address, int32_t streamer_portnumber, BOOL reconnection)
{
	HWND hwnd = ::GetDlgItem(_front->GetSafeHwnd(), IDC_STATIC_VIDEO_VIEW);
	if (_framework)
		_framework->open(streamer_address, streamer_portnumber, sirius::base::media_type_t::video | sirius::base::media_type_t::audio, reconnection ? true : false);
}

void client_controller::on_play_streaming(void)
{
	HWND hwnd = ::GetDlgItem(_front->GetSafeHwnd(), IDC_STATIC_VIDEO_VIEW);
	if (_framework)
		_framework->play(hwnd);
}

void client_controller::on_stop_streaming(void)
{
	if (_framework)
		_framework->stop();
}

void client_controller::on_pre_playback_end(void)
{

}

void client_controller::on_playback_end(void)
{
	_front->_current_time = 0;
	_front->_current_rate = 1;
	_front->UpdateTimeInfo();
}

void client_controller::on_post_playback_end(void)
{

}

void client_controller::on_pre_playback_totaltime(int32_t tottime)
{

}

void client_controller::on_playback_totaltime(int32_t tottime)
{
	_front->_total_time = tottime;
	_front->UpdateTimeInfo();
}

void client_controller::on_post_playback_totaltime(int32_t tottime)
{

}

void client_controller::on_pre_playback_currenttime(int32_t curtime)
{

}

void client_controller::on_playback_currenttime(int32_t curtime)
{
	_front->_current_time = curtime;
	_front->UpdateTimeInfo();
}

void client_controller::on_post_playback_currenttime(int32_t curtime)
{

}

void client_controller::on_pre_playback_currentrate(float currate)
{

}

void client_controller::on_playback_currentrate(float currate)
{
	_front->_current_rate = currate;
	CWnd * wnd = (CWnd*)_front->GetDlgItem(IDC_STATIC_RATE);
	CString str;
	str.Format(L"%.1fX", _front->_current_rate);
	wnd->SetWindowText(str);
	wnd->Invalidate();
}

void client_controller::on_post_playback_currentrate(float currate)
{

}

void client_controller::on_pre_xml(const char * msg, size_t length)
{

}

void client_controller::on_xml(const char * msg, size_t length)
{

}

void client_controller::on_post_xml(const char * msg, size_t length)
{

}

void client_controller::on_pre_error(int32_t error_code)
{

}

void client_controller::on_error(int32_t error_code)
{
	CString message;
	message.Format(L"error code : %d", error_code);
	::AfxMessageBox(message);
}

void client_controller::on_post_error(int32_t error_code)
{

}