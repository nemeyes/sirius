#include "stdafx.h"
#include "SiriusStressor.h"
#include "SiriusStressorDlg.h"
#include "stressor_controller.h"
#include <json/json.h>
#include <sirius_xml_parser.h>
#include <sirius_stringhelper.h>

#include <memory>
#include <string>
#include <fstream>

stressor_controller::stressor_controller(CSiriusStressorDlg * front, int32_t index, bool keepalive, bool tls)
	: _front(front)
	, _index(index)
	, _hmodule(NULL)
	, _framework(NULL)
	, _recv_stream_count(0)
{
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
	_latency = GetTickCount64();
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
	_key_event_run = false;
	close_key_event_thread_wait();
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
	check_stream_state();
}

void stressor_controller::on_post_destroy_session(void)
{

}

void stressor_controller::on_pre_connect_client(int32_t code, wchar_t * msg)
{

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
}

void stressor_controller::on_post_attendant_info(int32_t code, wchar_t * attendant_uuid, int32_t streamer_portnumber, int32_t video_width, int32_t video_height)
{

}

void stressor_controller::on_open_streaming(wchar_t * attendant_uuid, int32_t streamer_portnumber, bool reconnection)
{	
	if (_framework)
		_framework->open(_address, streamer_portnumber, sirius::base::media_type_t::video | sirius::base::media_type_t::audio, reconnection);
	check_stream_state();
}

void stressor_controller::on_play_streaming(void)
{	
	if (_framework)
		_framework->play(nullptr);
	check_stream_state();
}

void stressor_controller::on_stop_streaming(void)
{
	if (_framework)
		_framework->stop();
	check_stream_state();
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

}

void stressor_controller::on_post_error(int32_t error_code)
{

}

void stressor_controller::on_connect_stream(void)
{	
	HWND hwnd = _front->GetSafeHwnd();
	::PostMessage(hwnd, WM_STREAM_CONNECTED_MSG, _index, NULL);

	if (_front->IsDlgButtonChecked(IDC_CHECK_KEY_EVENT))
	{
		_key_event_run = true;
		unsigned int thread_id = 0;
		_key_event_thread = (HANDLE)_beginthreadex(NULL, 0, stressor_controller::key_event_process_cb, this, 0, &thread_id);
	}
}

void stressor_controller::on_disconnect_stream(void)
{
	_key_event_run = false;
	HWND hwnd = _front->GetSafeHwnd();
	::PostMessage(hwnd, WM_STREAM_DISCONNECTED_MSG, _index, NULL);
}

void stressor_controller::on_recv_stream(void)
{	
	DWORD recv_stream_time = GetTickCount();

	HWND hwnd = _front->GetSafeHwnd();
	if (_framework)
	{
		_recv_stream_count++;
		::PostMessage(hwnd, WM_STREAM_COUNT_MSG, _index, _recv_stream_count);

		if (_latency > 0 )
		{			
			::PostMessage(hwnd, WM_STREAM_LATENCY_MSG, _index, recv_stream_time - _latency);
			_latency = 0;
		}
	}
}


void stressor_controller::check_stream_state(void)
{
	HWND hwnd = _front->GetSafeHwnd();

	switch (_framework->state())
	{
	case sirius::library::framework::client::base::state_t::none:
		::PostMessage(hwnd, WM_STREAM_STATE_NONE_MSG, _index, NULL);
		break;
	case sirius::library::framework::client::base::state_t::running:
		::PostMessage(hwnd, WM_STREAM_STATE_RUNNING_MSG, _index, NULL);
		break;
	case sirius::library::framework::client::base::state_t::paused:
		::PostMessage(hwnd, WM_STREAM_STATE_PAUSED_MSG, _index, NULL);
		break;
	case sirius::library::framework::client::base::state_t::stopped:
		::PostMessage(hwnd, WM_STREAM_STATE_STOPPED_MSG, _index, NULL);
		break;
	default:
		break;
	}

}

unsigned stressor_controller::key_event_process_cb(void * param)
{
	stressor_controller * self = static_cast<stressor_controller*>(param);
	self->key_event_process();
	return 0;
}

void stressor_controller::key_event_process()
{
	::Sleep(_front->_key_interval * 1000);
	std::ifstream fin;
	fin.open(L"key_macro.json");
	const int bufferLength = 1024;
	char readBuffer[bufferLength] = { 0, };
	fin.read(readBuffer, bufferLength);
	fin.close();

	std::string config_doc = readBuffer;

	Json::Value root;
	Json::Reader reader;
	std::vector<int> keys;
	if (reader.parse(config_doc, root) == false)
	{
		for (int i = 0; i < 10; i++)
		{
			keys.push_back(-1);
		}
	}
	else
	{
		char key_name[20] = { 0 };
		for (int i = 0; i < 10; i++)
		{
			sprintf_s(key_name, "key_0%d", i);
			if (root[key_name].isInt())
			{
				int value = root[key_name].asInt();
				keys.push_back(value);
			}
		}
	}

	while (_key_event_run)
	{		
		if (_recv_stream_count > 0)
		{				
			for (int index = 0; index < keys.size(); index++)
			{
				if (keys[index] < 0) continue;

				_latency = GetTickCount();
				key_down(keys[index]);
				key_up(keys[index]);					
		
				int32_t sleep_second = 0;
				while(_front->_key_interval > sleep_second)
				{
					::Sleep(1000 * 1);
					sleep_second++;

					if (!_key_event_run) break;						
				}
				if (!_key_event_run) break;
			}
			if (!_front->_key_loop) 
				_key_event_run = false;
		}	
	}	
	keys.clear();
}

void stressor_controller::close_key_event_thread_wait()
{
	if (_key_event_thread != INVALID_HANDLE_VALUE)
	{
		if (::WaitForSingleObject(_key_event_thread, INFINITE) == WAIT_OBJECT_0)
		{
			::CloseHandle(_key_event_thread);
		}
		_key_event_thread = INVALID_HANDLE_VALUE;
	}
}
