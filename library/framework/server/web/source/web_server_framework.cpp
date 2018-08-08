#include <sirius_internal_notifier.h>
#include "web_server_framework.h"
#include <process.h>
#include <TlHelp32.h>
#include <sstream>

sirius::library::framework::server::web::core::core(void)
	: _d3d11_video_source(nullptr)
	, _host_video_source(nullptr)
	, _notifier(nullptr)
	, _unified_server_ctx(nullptr)
	, _unified_server(nullptr)
{
	_notifier = new sirius::library::misc::notification::internal::notifier();
	while (_data_msg_queue.empty() == false)
		_data_msg_queue.pop();

	_unified_server_ctx = new sirius::library::unified::server::context_t();
	_unified_server = new sirius::library::unified::server();
}

sirius::library::framework::server::web::core::~core()
{
	if (_unified_server && _unified_server_ctx)
	{
		if (_unified_server->is_video_compressor_initialized())
			_unified_server->release_video_compressor();
	}

	if (_unified_server_ctx)
	{
		delete _unified_server_ctx;
		_unified_server_ctx = nullptr;
	}
	if (_unified_server)
	{
		delete _unified_server;
		_unified_server = nullptr;
	}

	if (_notifier)
	{
		delete _notifier;
		_notifier = nullptr;
	}

	while (_data_msg_queue.empty() == false)
		_data_msg_queue.pop();
}

void sirius::library::framework::server::web::core::set_notification_callee(sirius::library::misc::notification::internal::notifier::callee * callee)
{
	_notifier->set_callee(callee);
	_notifier->enable_notification(TRUE);
}

int32_t sirius::library::framework::server::web::core::initialize(sirius::library::framework::server::web::context_t * context)
{
	int32_t status = sirius::library::framework::server::web::err_code_t::fail;

	memmove((void*)&_context, (void*)context, sizeof(sirius::library::framework::server::web::context_t));

	_unified_server_ctx->video_codec = context->video_codec;
	_unified_server_ctx->video_width = context->video_width;
	_unified_server_ctx->video_height = context->video_height;
	_unified_server_ctx->video_fps = context->video_fps;
	_unified_server_ctx->video_block_width = context->video_block_width;
	_unified_server_ctx->video_block_height = context->video_block_height;
	wcsncpy_s(_unified_server_ctx->uuid, context->uuid, sizeof(_unified_server_ctx->uuid));
	_unified_server_ctx->portnumber = context->portnumber;

	if (_unified_server_ctx->video_codec != sirius::library::unified::server::video_submedia_type_t::unknown)
	{
		status = _unified_server->initialize(_unified_server_ctx);
		if (status != sirius::library::framework::server::web::err_code_t::success)
		{
			_unified_server->release();
			return status;
		}
	}

	if (!_unified_server->is_video_compressor_initialized())
	{
		_venc_context.memtype		= context->video_process_type;
		_venc_context.codec			= context->video_codec;
		_venc_context.width			= context->video_width;
		_venc_context.height		= context->video_height;
		_venc_context.fps			= context->video_fps;
		_venc_context.nbuffer		= context->video_nbuffer;
		_venc_context.block_width	= context->video_block_width;
		_venc_context.block_height	= context->video_block_height;
		_venc_context.compression_level		= context->video_compression_level;
		_venc_context.quantization_colors	= context->video_qauntization_colors;
		_venc_context.invalidate4client = context->invalidate4client;
		_venc_context.indexed_mode = context->indexed_mode;
		_venc_context.nthread = context->nthread;
	}
	return sirius::library::framework::server::web::err_code_t::success;
}

int32_t sirius::library::framework::server::web::core::release(void)
{
	return _unified_server->release();
}

int32_t sirius::library::framework::server::web::core::play(void)
{
	_host_video_source = new sirius::library::framework::server::web::host_video_source(this);
	_host_video_source->start(_context.video_fps, 0);
	return sirius::library::framework::server::web::err_code_t::success;
}

int32_t sirius::library::framework::server::web::core::pause(void)
{
	return sirius::library::framework::server::web::err_code_t::success;
}

int32_t sirius::library::framework::server::web::core::stop(void)
{
	if (_host_video_source)
	{
		_host_video_source->stop();
		delete _host_video_source;
		_host_video_source = nullptr;
	}
	return sirius::library::framework::server::web::err_code_t::success;
}

int32_t sirius::library::framework::server::web::core::state(void) const
{
	return sirius::library::framework::server::web::state_t::unknown;
}

void sirius::library::framework::server::web::core::post_msg_key(HWND hwnd, int32_t msg, int32_t key_code, bool extended)
{
	int32_t scan_code = MapVirtualKey(key_code, 0);
	int32_t param;

	//KEY DOWN
	param = (0x00000001 | (scan_code << 16));
	if (msg == WM_KEYUP)
	{
		//KEY UP
		param |= 0xC0000000;  // set previous key and transition states (bits 30 and 31)
	}
	if (extended)
	{
		param |= 0x01000000;
	}
//	SendMessage(hwnd, WM_ACTIVATE, WA_ACTIVE, 0);
	PostMessage(hwnd, msg, key_code, param);
	//SendMessage(hwnd, msg, key_code, param);
}

void sirius::library::framework::server::web::core::post_msg_mouse(HWND hwnd, int32_t msg, int32_t pos_x, int32_t pos_y)
{
	int32_t lparam, wparam;

	wparam = pos_x;
	lparam = pos_y;
	//	SendMessage(hwnd, WM_SETCURSOR, WPARAM(hwnd), (LPARAM)MAKELONG(HTCLIENT, WM_MOUSEMOVE));
	PostMessage(hwnd, msg, 0, MAKELPARAM(wparam, lparam));

}

void sirius::library::framework::server::web::core::on_keyup(int32_t key)
{
	post_msg_key(_context.hwnd, WM_KEYUP, key, true);
}

void sirius::library::framework::server::web::core::on_keydown(int32_t key)
{
	post_msg_key(_context.hwnd, WM_KEYDOWN, key, true);
}

void sirius::library::framework::server::web::core::on_L_mouse_down(int32_t pos_x, int32_t pos_y)
{
	post_msg_mouse(_context.hwnd, WM_LBUTTONDOWN, pos_x, pos_y);
}

void sirius::library::framework::server::web::core::on_L_mouse_up(int32_t pos_x, int32_t pos_y)
{
	post_msg_mouse(_context.hwnd, WM_LBUTTONUP, pos_x, pos_y);
}

void sirius::library::framework::server::web::core::on_R_mouse_down(int32_t pos_x, int32_t pos_y)
{

}

void sirius::library::framework::server::web::core::on_R_mouse_up(int32_t pos_x, int32_t pos_y)
{

}

void sirius::library::framework::server::web::core::on_L_mouse_dclick(int32_t pos_x, int32_t pos_y)
{

}

void sirius::library::framework::server::web::core::on_R_mouse_dclick(int32_t pos_x, int32_t pos_y)
{

}

void sirius::library::framework::server::web::core::on_mouse_move(int32_t pos_x, int32_t pos_y)
{
	post_msg_mouse(_context.hwnd, WM_MOUSEMOVE, pos_x, pos_y);
}

void sirius::library::framework::server::web::core::on_mouse_wheel(int32_t pos_x, int32_t pos_y, int32_t wheel_delta)
{

}

void sirius::library::framework::server::web::core::on_end2end_data(const uint8_t * packet, int32_t packet_size)
{
	st_data_msg data;
	data.size = packet_size;
	data.msg =(const char*)packet;
	_data_msg_queue.push(data);
}

int* sirius::library::framework::server::web::core::get_end2end_data(void)
{
	if (_data_msg_queue.empty())
		return nullptr;
	
	static st_data_msg front;
	front = _data_msg_queue.front();
	_data_msg_queue.pop();
	
	return (int*)front.msg.c_str();
}

void sirius::library::framework::server::web::core::send_end2end_data(uint8_t * msg)
{
	VARIANT var;
	var.vt = VT_LPSTR;
	var.pcVal = (char*)msg;
	_notifier->push(sirius::library::misc::notification::internal::notifier::type_t::end2end_data, var);
}

void sirius::library::framework::server::web::core::on_video_initialize(void * device, int32_t hwnd, int32_t smt, int32_t width, int32_t height)
{
	int32_t status;
	if (_unified_server->is_video_compressor_initialized())
		on_video_release();

	_venc_context.device = device;
	_venc_context.origin_width = width;
	_venc_context.origin_height = height;
		
	status = _unified_server->initialize_video_compressor(&_venc_context);
}

void sirius::library::framework::server::web::core::on_video_receive(sirius::library::video::source::capturer::entity_t * captured)
{
	if (_unified_server->is_video_compressor_initialized())
	{
		sirius::library::video::transform::codec::compressor::entity_t input;
		input.data = captured->data;
		input.data_size = captured->data_size;
		input.data_capacity = captured->data_size;
		input.x = captured->x;
		input.y = captured->y;
		input.width = captured->width;
		input.height = captured->height;
		_unified_server->compress(&input);
	}
}

void sirius::library::framework::server::web::core::on_video_release(void)
{
	if(_unified_server->is_video_compressor_initialized())
		_unified_server->release_video_compressor();
}

int32_t sirius::library::framework::server::web::core::get_parent_pid(void)
{
	int pid = -1;
	HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 pe = { 0 };
	pe.dwSize = sizeof(PROCESSENTRY32);

	pid = GetCurrentProcessId();

	char buf[MAX_PATH] = { 0 };

	if (Process32First(h, &pe)) {
		do {
			if (pe.th32ProcessID == pid) {
				sprintf_s(buf, MAX_PATH, "PID: %i; PPID: %i\n", pid, pe.th32ParentProcessID);
				OutputDebugStringA(buf);
				break;
			}
		} while (Process32Next(h, &pe));
	}

	CloseHandle(h);
	return pe.th32ParentProcessID;
}

HWND sirius::library::framework::server::web::core::find_main_window(DWORD process_id)
{
	sirius::library::framework::server::web::core::handle_data_t data;
	data.process_id = process_id;
	data.best_handle = 0;
	EnumWindows(enum_windows_callback, (LPARAM)&data);
	return data.best_handle;
}

BOOL sirius::library::framework::server::web::core::is_main_window(HWND handle)
{
	return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
}

BOOL CALLBACK sirius::library::framework::server::web::core::enum_windows_callback(HWND handle, LPARAM lParam)
{
	sirius::library::framework::server::web::core::handle_data_t & data = *(sirius::library::framework::server::web::core::handle_data_t*)lParam;
	unsigned long process_id = 0;
	GetWindowThreadProcessId(handle, &process_id);
	if (data.process_id != process_id || !is_main_window(handle)) 
	{
		return TRUE;
	}
	data.best_handle = handle;
	return FALSE;
}