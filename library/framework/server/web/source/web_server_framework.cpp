#include <Windows.h>
#include <process.h>
#include <TlHelp32.h>
#include <sstream>

#include <sirius_internal_notifier.h>
#include "web_server_framework.h"

sirius::library::framework::server::web::core::core(void)
	: _video_source(nullptr)
	, _notifier(nullptr)
	, _gyro_enabled_attitude(true)
	, _gyro_enabled_gravity(false)
	, _gyro_enabled_rotation_rate(false)
	, _gyro_enabled_rotation_rate_unbiased(false)
	, _gyro_enabled_user_acceleration(false)
	, _gyro_updateinterval(0.15f)
{
	_video_source = new sirius::library::framework::server::web::video_source(this);
	_notifier = new sirius::library::misc::notification::internal::notifier();

	while (_xml_msg_que.empty() == false)
		_xml_msg_que.pop();
}


sirius::library::framework::server::web::core::~core()
{
	sirius::library::unified::server::instance().release_video_compressor();
	SIRIUS_SAFE_DELETE(_video_source);

	if (_notifier)
	{
		delete _notifier;
		_notifier = nullptr;
	}

	while (_xml_msg_que.empty() == false)
		_xml_msg_que.pop();
}

void sirius::library::framework::server::web::core::set_notification_callee(sirius::library::misc::notification::internal::notifier::callee * callee)
{
	_notifier->set_callee(callee);
	_notifier->enable_notification(TRUE);
}

int32_t sirius::library::framework::server::web::core::open(sirius::library::framework::server::web::context_t * context)
{
	memcpy((void*)&_context, (void*)context, sizeof(sirius::library::framework::server::web::context_t));

	//char log[MAX_PATH] = { 0 };
	//sprintf_s(log, MAX_PATH, "%s()_%d : parent window handle=%p, player_type:%d,port_num:%d", __FUNCTION__, __LINE__, _config.hwnd,_config.player_type,config->port_number);
	//OutputDebugStringA(log);

	LOGGER::make_trace_log(SLVSC, "%s()_%d : parent window handle=%d", __FUNCTION__, __LINE__, _context.hwnd);

	_video_source->start(_context.video_fps, 0);
	LOGGER::make_trace_log(SLVSC, "%s()_%d : capture video=%d", __FUNCTION__, __LINE__, _context.video_fps);

	if (!sirius::library::unified::server::instance().is_video_compressor_initialized())
	{
		_venc_context.gpuindex		= context->gpuindex;
		_venc_context.memtype		= sirius::library::video::transform::codec::compressor::video_memory_type_t::host;
		_venc_context.codec			= sirius::library::video::transform::codec::compressor::video_submedia_type_t::png;
		_venc_context.width			= context->video_width;
		_venc_context.height		= context->video_height;
		_venc_context.fps			= context->video_fps;
		_venc_context.nbuffer		= context->video_nbuffer;
	}
	return sirius::library::framework::server::web::err_code_t::success;
}

int32_t sirius::library::framework::server::web::core::close(void)
{
	_video_source->stop();

	if (_context.hwnd)
		PostMessageA(_context.hwnd, WM_CLOSE, 0L, 0L);

	return sirius::library::framework::server::web::err_code_t::success;
}

int32_t sirius::library::framework::server::web::core::play(void)
{
	return sirius::library::framework::server::web::err_code_t::success;
}

int32_t sirius::library::framework::server::web::core::pause(void)
{
	return sirius::library::framework::server::web::err_code_t::success;
}

int32_t sirius::library::framework::server::web::core::stop(void)
{
	return sirius::library::framework::server::web::err_code_t::success;
}

int32_t sirius::library::framework::server::web::core::state(void) const
{
	return sirius::library::framework::server::web::state_t::unknown;
}

int32_t sirius::library::framework::server::web::core::seek(int32_t diff)
{
	return sirius::library::framework::server::web::err_code_t::success;
}

int32_t sirius::library::framework::server::web::core::seek_to(int32_t second)
{
	return sirius::library::framework::server::web::err_code_t::success;
}

int32_t sirius::library::framework::server::web::core::seek_stop(void)
{
	return sirius::library::framework::server::web::err_code_t::success;
}

int32_t sirius::library::framework::server::web::core::forward(void)
{
	return sirius::library::framework::server::web::err_code_t::success;
}

int32_t sirius::library::framework::server::web::core::backward(void)
{
	return sirius::library::framework::server::web::err_code_t::success;
}

int32_t sirius::library::framework::server::web::core::reverse(void)
{
	return sirius::library::framework::server::web::err_code_t::success;
}

int32_t sirius::library::framework::server::web::core::play_toggle(void)
{
	return sirius::library::framework::server::web::err_code_t::success;
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

void sirius::library::framework::server::web::core::on_gyro(float x, float y, float z)
{

}

void sirius::library::framework::server::web::core::on_pinch_zoom(float delta)
{

}

void sirius::library::framework::server::web::core::on_gyro_attitude(float x, float y, float z, float w)
{
	//_gyro_attitude.x = y;
	//_gyro_attitude.y = -x;
	//_gyro_attitude.z = z;
	//_gyro_attitude.w = w;
	_gyro_attitude.x = x;
	_gyro_attitude.y = y;
	_gyro_attitude.z = z;
	_gyro_attitude.w = w;
}

void sirius::library::framework::server::web::core::on_gyro_gravity(float x, float y, float z)
{
	_gyro_gravity.x = x;
	_gyro_gravity.y = y;
	_gyro_gravity.z = z;
}

void sirius::library::framework::server::web::core::on_gyro_rotation_rate(float x, float y, float z)
{
	_gyro_rotation_rate.x = x;
	_gyro_rotation_rate.y = y;
	_gyro_rotation_rate.z = z;
}

void sirius::library::framework::server::web::core::on_gyro_rotation_rate_unbiased(float x, float y, float z)
{
	_gyro_rotation_rate_unbiased.x = x;
	_gyro_rotation_rate_unbiased.y = y;
	_gyro_rotation_rate_unbiased.z = z;
}

void sirius::library::framework::server::web::core::on_gyro_user_acceleration(float x, float y, float z)
{
	_gyro_user_acceleration.x = x;
	_gyro_user_acceleration.y = y;
	_gyro_user_acceleration.z = z;
}

void sirius::library::framework::server::web::core::on_gyro_enabled_attitude(bool state)
{
	_gyro_enabled_attitude = state;

	if (!_gyro_enabled_attitude &&
		!_gyro_enabled_gravity &&
		!_gyro_enabled_rotation_rate &&
		!_gyro_enabled_rotation_rate_unbiased &&
		!_gyro_enabled_user_acceleration)
	{
		_gyro_updateinterval = -1.0f;
	}
}

void sirius::library::framework::server::web::core::on_gyro_enabled_gravity(bool state)
{
	_gyro_enabled_gravity = state;

	if (!_gyro_enabled_attitude &&
		!_gyro_enabled_gravity &&
		!_gyro_enabled_rotation_rate &&
		!_gyro_enabled_rotation_rate_unbiased &&
		!_gyro_enabled_user_acceleration)
	{
		_gyro_updateinterval = -1.0f;
	}
}

void sirius::library::framework::server::web::core::on_gyro_enabled_rotation_rate(bool state)
{
	_gyro_enabled_rotation_rate = state;

	if (!_gyro_enabled_attitude &&
		!_gyro_enabled_gravity &&
		!_gyro_enabled_rotation_rate &&
		!_gyro_enabled_rotation_rate_unbiased &&
		!_gyro_enabled_user_acceleration)
	{
		_gyro_updateinterval = -1.0f;
	}
}

void sirius::library::framework::server::web::core::on_gyro_enabled_rotation_rate_unbiased(bool state)
{
	_gyro_enabled_rotation_rate_unbiased = state;

	if (!_gyro_enabled_attitude &&
		!_gyro_enabled_gravity &&
		!_gyro_enabled_rotation_rate &&
		!_gyro_enabled_rotation_rate_unbiased &&
		!_gyro_enabled_user_acceleration)
	{
		_gyro_updateinterval = -1.0f;
	}
}

void sirius::library::framework::server::web::core::on_gyro_enabled_user_acceleration(bool state)
{
	_gyro_enabled_user_acceleration = state;

	if (!_gyro_enabled_attitude &&
		!_gyro_enabled_gravity &&
		!_gyro_enabled_rotation_rate &&
		!_gyro_enabled_rotation_rate_unbiased &&
		!_gyro_enabled_user_acceleration)
	{
		_gyro_updateinterval = -1.0f;
	}
}

void sirius::library::framework::server::web::core::on_gyro_updateinterval(float interval)
{
	_gyro_updateinterval = interval;
}

void sirius::library::framework::server::web::core::on_infoxml(const char * msg, int32_t length)
{
	st_xml_msg st_msg;
	st_msg.size = length;
	st_msg.msg = msg;
	_xml_msg_que.push(st_msg);
}

int* sirius::library::framework::server::web::core::get_xml_msg()
{
	if (_xml_msg_que.empty())
		return nullptr;
	
	static st_xml_msg front;
	front = _xml_msg_que.front();
	_xml_msg_que.pop();
	
	return (int*)front.msg.c_str();
}

void sirius::library::framework::server::web::core::send_xml_msg(char* msg)
{
	VARIANT var;
	var.vt = VT_LPSTR;
	var.pcVal = msg;
	_notifier->push(sirius::library::misc::notification::internal::notifier::type_t::info_xml, var);
}

void sirius::library::framework::server::web::core::set_gyro_enabled(bool state)
{
	set_gyro_enabled_attitude(state);
	set_gyro_enabled_gravity(state);
	set_gyro_enabled_rotation_rate(state);
	set_gyro_enabled_rotation_rate_unbiased(state);
	set_gyro_enabled_user_acceleration(state);
}

bool sirius::library::framework::server::web::core::get_gyro_enabled_attitude()
{
	return _gyro_enabled_attitude;
}

void sirius::library::framework::server::web::core::set_gyro_enabled_attitude(bool state)
{
	BOOL state_4byte;
	if (state) state_4byte = TRUE;
	else state_4byte = FALSE;

	VARIANT var;
	var.vt = VT_I4;
	var.intVal = state_4byte;
	_notifier->push(sirius::library::misc::notification::internal::notifier::type_t::gyro_enabled_attitude, var);
}

bool sirius::library::framework::server::web::core::get_gyro_enabled_gravity()
{
	return _gyro_enabled_gravity;
}

void sirius::library::framework::server::web::core::set_gyro_enabled_gravity(bool state)
{
	BOOL state_4byte;
	if (state) state_4byte = TRUE;
	else state_4byte = FALSE;

	VARIANT var;
	var.vt = VT_I4;
	var.intVal = state_4byte;
	_notifier->push(sirius::library::misc::notification::internal::notifier::type_t::gyro_enabled_gravity, var);
}

bool sirius::library::framework::server::web::core::get_gyro_enabled_rotation_rate()
{
	return _gyro_enabled_rotation_rate;
}

void sirius::library::framework::server::web::core::set_gyro_enabled_rotation_rate(bool state)
{
	BOOL state_4byte;
	if (state) state_4byte = TRUE;
	else state_4byte = FALSE;

	VARIANT var;
	var.vt = VT_I4;
	var.intVal = state_4byte;
	_notifier->push(sirius::library::misc::notification::internal::notifier::type_t::gyro_enabled_rotation_rate, var);
}

bool sirius::library::framework::server::web::core::get_gyro_enabled_rotation_rate_unbiased()
{
	return _gyro_enabled_rotation_rate_unbiased;
}

void sirius::library::framework::server::web::core::set_gyro_enabled_rotation_rate_unbiased(bool state)
{
	BOOL state_4byte;
	if (state) state_4byte = TRUE;
	else state_4byte = FALSE;

	VARIANT var;
	var.vt = VT_I4;
	var.intVal = state_4byte;
	_notifier->push(sirius::library::misc::notification::internal::notifier::type_t::gyro_enabled_rotation_rate_unbiased, var);
}

bool sirius::library::framework::server::web::core::get_gyro_enabled_user_acceleration()
{
	return _gyro_enabled_user_acceleration;
}

void sirius::library::framework::server::web::core::set_gyro_enabled_user_acceleration(bool state)
{
	BOOL state_4byte;
	if (state) state_4byte = TRUE;
	else state_4byte = FALSE;

	VARIANT var;
	var.vt = VT_I4;
	var.intVal = state_4byte;
	_notifier->push(sirius::library::misc::notification::internal::notifier::type_t::gyro_enabled_user_acceleration, var);
}

float sirius::library::framework::server::web::core::get_gyro_updateinterval()
{
	return _gyro_updateinterval;
}

void sirius::library::framework::server::web::core::set_gyro_updateinterval(float interval)
{
	VARIANT var;
	var.vt = VT_R4;
	var.fltVal = interval;
	_notifier->push(sirius::library::misc::notification::internal::notifier::type_t::gyro_interval, var);
}

gyro_data sirius::library::framework::server::web::core::get_attitude()
{
	return _gyro_attitude;
}

gyro_data sirius::library::framework::server::web::core::get_gravity()
{
	return _gyro_gravity;
}

gyro_data sirius::library::framework::server::web::core::get_rotation_rate()
{
	return _gyro_rotation_rate;
}

gyro_data sirius::library::framework::server::web::core::get_rotation_rate_unbiased()
{
	return _gyro_rotation_rate_unbiased;
}

gyro_data sirius::library::framework::server::web::core::get_user_acceleration()
{
	return _gyro_user_acceleration;
}

void sirius::library::framework::server::web::core::on_video_initialize(void * device, int32_t hwnd, int32_t smt, int32_t width, int32_t height)
{
	int32_t ret;

	//char log[MAX_PATH] = { 0 };
	//sprintf_s(log, MAX_PATH, "on_video_initialize hwnd:%p", hwnd);
	//OutputDebugStringA(log);

	if (sirius::library::unified::server::instance().is_video_compressor_initialized())
		on_video_release();

	//if (!sirius::library::unified::server::instance().is_video_compressor_initialized())
	{
		_venc_context.device = device;
		_venc_context.origin_width = width;
		_venc_context.origin_height = height;
		
		int pid = 0;
		if (_context.type == sirius::library::framework::server::web::attendant_type_t::web)
		{
			pid = get_parent_pid();
		}
		else
		{
			pid = GetCurrentProcessId();
		}
		
		_context.hwnd = (HWND)find_main_window(pid);
		ret = sirius::library::unified::server::instance().initialize_video_compressor(&_venc_context);
		LOGGER::make_trace_log(SLVSC, "%s()_%d : iwidth=%d, iheight=%d, ret=%d hwnd=%p ", __FUNCTION__, __LINE__, _venc_context.origin_width, _venc_context.origin_height, ret, hwnd);
	}
}

void sirius::library::framework::server::web::core::on_video_receive(sirius::library::video::source::capturer::entity_t * captured)
{
	if (sirius::library::unified::server::instance().is_video_compressor_initialized())
	{
		sirius::library::video::transform::codec::compressor::entity_t input;
		input.memtype = sirius::library::video::transform::codec::compressor::video_memory_type_t::d3d11;
		input.data = captured->data;
		input.data_size = 0;
		input.data_capacity = 0;
		sirius::library::unified::server::instance().compress(&input);
	}
}

void sirius::library::framework::server::web::core::on_video_release(void)
{
	if(sirius::library::unified::server::instance().is_video_compressor_initialized())
		sirius::library::unified::server::instance().release_video_compressor();
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