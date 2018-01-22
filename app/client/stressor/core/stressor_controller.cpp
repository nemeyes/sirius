#include "stdafx.h"
#include "SiriusStressor.h"
#include "SiriusStressorDlg.h"
#include "stressor_controller.h"
#include <json/json.h>
#include <sirius_xml_parser.h>
#include <sirius_stringhelper.h>

#include <memory>
#include <string>

typedef sirius::library::framework::client::base * (*fpn_create_client_framework)();
typedef void(*fpn_destory_client_framework)(sirius::library::framework::client::base ** client_framework);

stressor_controller::stressor_controller(CSiriusStressorDlg * front, int32_t index, bool keepalive, bool tls)
	: _front(front)
	, _index(index)
	, _hmodule(NULL)
	, _framework(NULL)
{
	HINSTANCE inst = AfxGetInstanceHandle();
	HWND hwnd = _front->GetSafeHwnd();
	_controller = new sirius::app::client::proxy(this, keepalive, tls, nullptr, nullptr);
}

stressor_controller::~stressor_controller(void)
{
	if (_controller)
	{
		delete _controller;
		_controller = nullptr;
	}
}

void stressor_controller::on_pre_connect(wchar_t * address, int32_t portNumber, bool reconnection)
{
	HWND hwnd = _front->GetSafeHwnd();
	::PostMessage(hwnd, WM_CLIENT_CONNECTING_MSG, _index, NULL);
	memset(_address, 0x00, sizeof(_address));
	wcsncpy_s(_address, address, sizeof(_address) / sizeof(wchar_t) - 1);
}

void stressor_controller::on_post_connect(wchar_t * address, int32_t portNumber, bool reconnection)
{
	HWND hwnd = _front->GetSafeHwnd();

	if (!_framework)
	{
		_framework = new sirius::library::framework::stressor::native(this);
	}
}

void stressor_controller::on_pre_disconnect(void)
{
	HWND hwnd = _front->GetSafeHwnd();
	::PostMessage(hwnd, WM_CLIENT_DISCONNECTING_MSG, _index, NULL);
}

void stressor_controller::on_post_disconnect(void)
{
	if (_framework)
	{
		_framework->stop();
		delete _framework;
		_framework = nullptr;
	}
}

void stressor_controller::on_pre_create_session(void)
{
	HWND hwnd = _front->GetSafeHwnd();
	::PostMessage(hwnd, WM_CLIENT_CONNECTED_MSG, _index, NULL);
}

void stressor_controller::on_create_session(void)
{
	HWND hwnd = _front->GetSafeHwnd();
	//::PostMessage(hwnd, WM_CREATING_ATTENDANT_BEGIN_MESSAGE, 0, 0);

	CString client_id = L"";
	_front->_client_id.GetWindowTextW(client_id);
	connect_client((LPWSTR)(LPCWSTR)client_id);
}

void stressor_controller::on_post_create_session(void)
{

}

void stressor_controller::on_pre_destroy_session(void)
{
	HWND hwnd = _front->GetSafeHwnd();
	::PostMessage(hwnd, WM_CLIENT_DISCONNECTED_MSG, _index, 0);
}

void stressor_controller::on_destroy_session(void)
{
	if (_framework)
		_framework->stop();
}

void stressor_controller::on_post_destroy_session(void)
{

}

void stressor_controller::on_pre_connect_client(int32_t code, wchar_t * msg)
{
	HWND hwnd = _front->GetSafeHwnd();
	if (code == 0)
	{
		//::PostMessage(hwnd, WM_CREATING_ATTENDANT_END_MESSAGE, 0, 0);
	}
	else
	{
		MessageBox(hwnd, msg, _T("ERROR"), MB_OK);
	}
}

void stressor_controller::on_connect_client(int32_t code, wchar_t * msg)
{

}

void stressor_controller::on_post_connect_client(int32_t code, wchar_t * msg)
{

}

void stressor_controller::on_pre_disconnect_client(int32_t code)
{

}

void stressor_controller::on_disconnect_client(int32_t code)
{

}

void stressor_controller::on_post_disconnect_client(int32_t code)
{

}

void stressor_controller::on_pre_attendant_info(int32_t code, wchar_t * attendant_uuid, int32_t streamer_portnumber, int32_t video_width, int32_t video_height)
{
	wchar_t title[500] = { 0 };
	_snwprintf_s(title, sizeof(title), L"sirius_client attendant_uuid=%s, port=%d", attendant_uuid, streamer_portnumber);
	_front->SetWindowTextW(title);

	//_front->_video_width = video_width;
	//_front->_video_height = video_height;
}

void stressor_controller::on_post_attendant_info(int32_t code, wchar_t * attendant_uuid, int32_t streamer_portnumber, int32_t video_width, int32_t video_height)
{

}

void stressor_controller::on_open_streaming(wchar_t * attendant_uuid, int32_t streamer_portnumber, bool reconnection)
{
	//HWND hwnd = ::GetDlgItem(_front->GetSafeHwnd(), IDC_STATIC_VIDEO_VIEW);
	if (_framework)
		_framework->open(_address, streamer_portnumber, sirius::base::media_type_t::video | sirius::base::media_type_t::audio, reconnection);
}

void stressor_controller::on_play_streaming(void)
{
	//HWND hwnd = ::GetDlgItem(_front->GetSafeHwnd(), IDC_STATIC_VIDEO_VIEW);
	if (_framework)
		_framework->play(nullptr);
}

void stressor_controller::on_stop_streaming(void)
{
	if (_framework)
		_framework->stop();
}

void stressor_controller::on_pre_xml(const char * msg, size_t length)
{

}

void stressor_controller::on_xml(const char * msg, size_t length)
{

}

void stressor_controller::on_post_xml(const char * msg, size_t length)
{

}

void stressor_controller::on_pre_error(int32_t error_code)
{

}

void stressor_controller::on_error(int32_t error_code)
{
	CString message;
	message.Format(L"error code : %d", error_code);
	::AfxMessageBox(message);
}

void stressor_controller::on_post_error(int32_t error_code)
{

}

void stressor_controller::stream_connect_callback()
{
	HWND hwnd = _front->GetSafeHwnd();
	::PostMessage(hwnd, WM_STREAM_CONNECTED_MSG, _index, NULL);
}

void stressor_controller::stream_disconnect_callback()
{
	HWND hwnd = _front->GetSafeHwnd();
	::PostMessage(hwnd, WM_STREAM_DISCONNECTED_MSG, _index, NULL);
}