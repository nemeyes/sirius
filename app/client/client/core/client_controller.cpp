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

client_controller::client_controller(CSiriusClientDlg * front, bool keepalive, bool tls)
	: _front(front)
	, _hmodule(NULL)
	, _framework(NULL)
{
	HINSTANCE inst = AfxGetInstanceHandle();
	HWND hwnd = _front->GetSafeHwnd();
	_controller = new sirius::app::client::proxy(this, keepalive, tls, inst, hwnd);
}

client_controller::~client_controller(void)
{
	if (_controller)
	{
		delete _controller;
		_controller = nullptr;
	}
}

void client_controller::on_pre_connect(wchar_t * address, int32_t portNumber, bool reconnection)
{
	HWND hwnd = _front->GetSafeHwnd();
	::PostMessage(hwnd, WM_CONNECTION_BEGIN_MESSAGE, 0, 0);
	memset(_address, 0x00, sizeof(_address));
	wcsncpy_s(_address, address, sizeof(_address) / sizeof(wchar_t) - 1);
}

void client_controller::on_post_connect(wchar_t * address, int32_t portNumber, bool reconnection)
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
	::PostMessage(hwnd, WM_CREATING_ATTENDANT_BEGIN_MESSAGE, 0, 0);

	CString device_id = L"";
	_front->_ctrl_device_id.GetWindowTextW(device_id);
	connect_client((LPWSTR)(LPCWSTR)device_id);
}

void client_controller::on_post_create_session(void)
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

void client_controller::on_pre_connect_client(int32_t code, wchar_t * msg)
{
	HWND hwnd = _front->GetSafeHwnd();
	if (code == 0)
	{
		::PostMessage(hwnd, WM_CREATING_ATTENDANT_END_MESSAGE, 0, 0);
	}
	else
	{
		MessageBox(hwnd, msg, _T("ERROR"), MB_OK);
	}
}

void client_controller::on_connect_client(int32_t code, wchar_t * msg)
{

}

void client_controller::on_post_connect_client(int32_t code, wchar_t * msg)
{

}

void client_controller::on_pre_disconnect_client(int32_t code)
{

}

void client_controller::on_disconnect_client(int32_t code)
{

}

void client_controller::on_post_disconnect_client(int32_t code)
{

}

void client_controller::on_pre_attendant_info(int32_t code, wchar_t * attendant_uuid, int32_t streamer_portnumber, int32_t video_width, int32_t video_height)
{
	wchar_t title[500] = { 0 };
	_snwprintf_s(title, sizeof(title), L"sirius_client attendant_uuid=%s, port=%d", attendant_uuid, streamer_portnumber);
	_front->SetWindowTextW(title);

	_front->_video_width = video_width;
	_front->_video_height = video_height;
}

void client_controller::on_post_attendant_info(int32_t code, wchar_t * attendant_uuid, int32_t streamer_portnumber, int32_t video_width, int32_t video_height)
{

}

void client_controller::on_open_streaming(wchar_t * attendant_uuid, int32_t streamer_portnumber, bool reconnection)
{
	HWND hwnd = ::GetDlgItem(_front->GetSafeHwnd(), IDC_STATIC_VIDEO_VIEW);
	if (_framework)
		_framework->open(_address, streamer_portnumber, sirius::base::media_type_t::video | sirius::base::media_type_t::audio, reconnection);
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

void client_controller::on_pre_end2end_data(const char * packet, int32_t packet_size)
{

}

void client_controller::on_end2end_data(const char * packet, int32_t packet_size)
{
	CString str(packet);
	_front->_ctrl_end2end_data.SetWindowText(str);
}

void client_controller::on_post_end2end_data(const char * packet, int32_t packet_size)
{
	_controller->post_end2end_data(packet, packet_size);
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