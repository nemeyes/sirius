#include "dinput_receiver.h"
#include <cmath>

sirius::library::user::event::dinput::receiver::core::core(sirius::library::user::event::dinput::receiver * front)
	: _front(front)
	, _dx_input(NULL)
	, _keyboard(NULL)
	, _mouse(NULL)
	, _thread(INVALID_HANDLE_VALUE)
	, _run(false)
	, _using_mouse(false)
	, _keystroke_interval(0)
	, _left_button_down_tick(0)
	, _right_button_down_tick(0)
{
}

sirius::library::user::event::dinput::receiver::core::~core(void)
{
}

int32_t sirius::library::user::event::dinput::receiver::core::initialize(HINSTANCE inst, HWND hwnd)
{
	HRESULT hr = E_FAIL;

	ATL::CComPtr<IDirectInput8> dx_input;
	ATL::CComPtr<IDirectInputDevice8> keyboard;
	ATL::CComPtr<IDirectInputDevice8> mouse;

	RECT wrect;
	::GetWindowRect(hwnd, &wrect);
	_screen_width = std::abs(wrect.right - wrect.left);
	_screen_height = std::abs(wrect.bottom - wrect.top);
	_mouse_x = _screen_width / 2;
	_mouse_y = _screen_height / 2;

	hr = DirectInput8Create(inst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dx_input, NULL);
	if (FAILED(hr))
		return sirius::library::user::event::dinput::receiver::err_code_t::fail;
	hr = dx_input->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	if (FAILED(hr))
		return sirius::library::user::event::dinput::receiver::err_code_t::fail;
	hr = keyboard->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(hr))
		return sirius::library::user::event::dinput::receiver::err_code_t::fail;
	hr = keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	if (FAILED(hr))
		return sirius::library::user::event::dinput::receiver::err_code_t::fail;
	hr = keyboard->Acquire();
	//if (FAILED(hr))
	//	return sirius::library::user::event::dinput::receiver::err_code_t::fail;


	hr = dx_input->CreateDevice(GUID_SysMouse, &mouse, NULL);
	if (FAILED(hr))
		return sirius::library::user::event::dinput::receiver::err_code_t::fail;
	hr = mouse->SetDataFormat(&c_dfDIMouse);
	if (FAILED(hr))
		return sirius::library::user::event::dinput::receiver::err_code_t::fail;
	hr = mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(hr))
		return sirius::library::user::event::dinput::receiver::err_code_t::fail;
	hr = mouse->Acquire();
	//if (FAILED(hr))
	//	return sirius::library::user::event::dinput::receiver::err_code_t::fail;

	_dx_input = dx_input;
	_keyboard = keyboard;
	_mouse = mouse;

	memset(_keyboard_state, 0, 256);
	memset(_prev_keyboard_state, 0, 256);
	memset(&_mouse_state, 0, sizeof(DIMOUSESTATE));
	memset(&_prev_mouse_state, 0, sizeof(DIMOUSESTATE));

	_run = true;
	uint32_t thrdaddr = 0;
	_thread = (HANDLE)::_beginthreadex(NULL, 0, sirius::library::user::event::dinput::receiver::core::process_callback, this, 0, &thrdaddr);
	if(_thread==NULL || _thread==INVALID_HANDLE_VALUE)
		return sirius::library::user::event::dinput::receiver::err_code_t::fail;

	//for (int32_t index = 0; index < 100 || !_run; index++)
	//	::Sleep(10);

	return sirius::library::user::event::dinput::receiver::err_code_t::success;
}

int32_t sirius::library::user::event::dinput::receiver::core::release(void)
{
	if (_thread != NULL && _thread != INVALID_HANDLE_VALUE)
	{
		_run = false;
		if (::WaitForSingleObject(_thread, INFINITE) == WAIT_OBJECT_0)
		{
			::CloseHandle(_thread);
			_thread = INVALID_HANDLE_VALUE;
		}
	}

	if (_mouse)
	{
		_mouse->Unacquire();
		_mouse = NULL;
	}

	// Release the keyboard.
	if (_keyboard)
	{
		_keyboard->Unacquire();
		_keyboard = NULL;
	}

	// Release the main interface to direct input.
	if (_dx_input)
	{
		_dx_input = NULL;
	}
	return sirius::library::user::event::dinput::receiver::err_code_t::success;
}

unsigned sirius::library::user::event::dinput::receiver::core::process_callback(void * param)
{
	sirius::library::user::event::dinput::receiver::core * self = static_cast<sirius::library::user::event::dinput::receiver::core*>(param);
	self->process();
	return 0;
}

void sirius::library::user::event::dinput::receiver::core::process(void)
{
	MSG msg;
	while (_run)
	{
		int	key_code = 0;

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
			break;

		int32_t status = read_keyboard();
		if (status != sirius::library::user::event::dinput::receiver::err_code_t::success)
			continue;

		if (_using_mouse)
		{
			status = read_mouse();
			if (status != sirius::library::user::event::dinput::receiver::err_code_t::success)
				continue;
		}

		Sleep(1);
	}
}

int32_t sirius::library::user::event::dinput::receiver::core::read_keyboard(void)
{
	if (!_keyboard)
		return sirius::library::user::event::dinput::receiver::err_code_t::fail;

	HRESULT hr;
	try
	{
		hr = _keyboard->GetDeviceState(sizeof(_keyboard_state), (LPVOID)&_keyboard_state);
	}
	catch (...)
	{
		return sirius::library::user::event::dinput::receiver::err_code_t::fail;
	}
	if (FAILED(hr))
	{
		if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))
			_keyboard->Acquire();
		else
			return sirius::library::user::event::dinput::receiver::err_code_t::fail;
	}
	for (int32_t index = 0; index < ARRAYSIZE(_keyboard_state); index++)
	{
		if (_keyboard_state[index] != _prev_keyboard_state[index])
		{
			if ((_keyboard_state[index] & 0x80) ? true : false)
			{
				_prev_keyboard_clock[index] = clock();
				parse_keyinput(index, false);
			}

			if ((_prev_keyboard_state[index] & 0x80) ? true : false)
			{
				parse_keyinput(index, true);
			}

			_prev_keyboard_state[index] = _keyboard_state[index];
			break;
		}
		else
		{
			// reapeat key
			if (_keystroke_interval == 0) continue;

			if ((_keyboard_state[index] & 0x80) ? true : false)
			{
				if (clock() - _prev_keyboard_clock[index] > _keystroke_interval /*ms*/)
				{
					_prev_keyboard_clock[index] = clock();
					parse_keyinput(index, false);
					break;
				}
			}
		}
	}

	return sirius::library::user::event::dinput::receiver::err_code_t::success;
}

int32_t sirius::library::user::event::dinput::receiver::core::read_mouse(void)
{
	if(!_mouse)
		return sirius::library::user::event::dinput::receiver::err_code_t::fail;

	HRESULT hr;
	try
	{
		hr = _mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&_mouse_state);
	}
	catch (...)
	{
		return sirius::library::user::event::dinput::receiver::err_code_t::fail;
	}

	if (FAILED(hr))
	{
		if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))
			_mouse->Acquire();
		else
			return sirius::library::user::event::dinput::receiver::err_code_t::fail;
	}

	parse_mouseinput();
	_prev_mouse_state = _mouse_state;

	return sirius::library::user::event::dinput::receiver::err_code_t::success;
}

int32_t sirius::library::user::event::dinput::receiver::core::parse_keyinput(int32_t key_value, bool up)
{
	if (up)
	{
		if (key_value == DIK_SCROLL)
		{
			_front->keyup_callback(sirius::library::user::event::dinput::receiver::value_t::scrlk);
		}
		else if (key_value == DIK_SPACE)
		{
			_front->keyup_callback(sirius::library::user::event::dinput::receiver::value_t::space);
		}
		else if (key_value == DIK_RETURN)
		{
			_front->keyup_callback(sirius::library::user::event::dinput::receiver::value_t::enter);
		}
		else if (key_value == DIK_UP)
		{
			_front->keyup_callback(sirius::library::user::event::dinput::receiver::value_t::up);
		}
		else if (key_value == DIK_DOWN)
		{
			_front->keyup_callback(sirius::library::user::event::dinput::receiver::value_t::down);
		}
		else if (key_value == DIK_LEFT)
		{
			_front->keyup_callback(sirius::library::user::event::dinput::receiver::value_t::left);
		}
		else if (key_value == DIK_RIGHT)
		{
			_front->keyup_callback(sirius::library::user::event::dinput::receiver::value_t::right);
		}
		else if (key_value == DIK_ESCAPE)
		{
			_front->keyup_callback(sirius::library::user::event::dinput::receiver::value_t::esc);
		}
		else if (key_value == DIK_A)
		{
			_front->keyup_callback(sirius::library::user::event::dinput::receiver::value_t::a);
		}
		else if (key_value == DIK_Z)
		{
			_front->keyup_callback(sirius::library::user::event::dinput::receiver::value_t::z);
		}
		else if (key_value == DIK_F2)
		{
			_front->keyup_callback(sirius::library::user::event::dinput::receiver::value_t::f2);
		}
		else if (key_value == DIK_F3)
		{
			_front->keyup_callback(sirius::library::user::event::dinput::receiver::value_t::f3);
		}
		else if (key_value == DIK_F4)
		{
			_front->keyup_callback(sirius::library::user::event::dinput::receiver::value_t::f4);
		}
		else if (key_value == DIK_F5)
		{
			_front->keyup_callback(sirius::library::user::event::dinput::receiver::value_t::f5);
		}
		else if (key_value == DIK_F6)
		{
			_front->keyup_callback(sirius::library::user::event::dinput::receiver::value_t::f6);
		}
		else if (key_value == DIK_F7)
		{
			_front->keyup_callback(sirius::library::user::event::dinput::receiver::value_t::f7);
		}
		else if (key_value == DIK_F8)
		{
			_front->keyup_callback(sirius::library::user::event::dinput::receiver::value_t::f8);
		}
		else if (key_value == DIK_NUMPAD4)
		{
			_front->keyup_seek_callback(); //-10s
		}
		else if (key_value == DIK_NUMPAD6)
		{
			_front->keyup_seek_callback(); //+10s
		}
		else if (key_value == DIK_NUMPAD0)
		{
			_front->keyup_play_toggle();
		}
		else if (key_value == DIK_NUMPAD1)
		{
			_front->keyup_backward();
		}
		else if (key_value == DIK_NUMPAD3)
		{
			_front->keyup_forward();
		}
		else if (key_value == DIK_NUMPAD2)
		{
			_front->keyup_reverse();
		}
	}
	else
	{
		if (key_value == DIK_SCROLL)
		{
			_front->keydown_callback(sirius::library::user::event::dinput::receiver::value_t::scrlk);
		}
		else if (key_value == DIK_SPACE)
		{
			_front->keydown_callback(sirius::library::user::event::dinput::receiver::value_t::space);
		}
		else if (key_value == DIK_RETURN)
		{
			_front->keydown_callback(sirius::library::user::event::dinput::receiver::value_t::enter);
		}
		else if (key_value == DIK_UP)
		{
			_front->keydown_callback(sirius::library::user::event::dinput::receiver::value_t::up);
		}
		else if (key_value == DIK_DOWN)
		{
			_front->keydown_callback(sirius::library::user::event::dinput::receiver::value_t::down);
		}
		else if (key_value == DIK_LEFT)
		{
			_front->keydown_callback(sirius::library::user::event::dinput::receiver::value_t::left);
		}
		else if (key_value == DIK_RIGHT)
		{
			_front->keydown_callback(sirius::library::user::event::dinput::receiver::value_t::right);
		}
		else if (key_value == DIK_ESCAPE)
		{
			_front->keydown_callback(sirius::library::user::event::dinput::receiver::value_t::esc);
		}
		else if (key_value == DIK_A)
		{
			_front->keydown_callback(sirius::library::user::event::dinput::receiver::value_t::a);
		}
		else if (key_value == DIK_Z)
		{
			_front->keydown_callback(sirius::library::user::event::dinput::receiver::value_t::z);
		}
		else if (key_value == DIK_F2)
		{
			_front->keydown_callback(sirius::library::user::event::dinput::receiver::value_t::f2);
		}
		else if (key_value == DIK_F3)
		{
			_front->keydown_callback(sirius::library::user::event::dinput::receiver::value_t::f3);
		}
		else if (key_value == DIK_F4)
		{
			_front->keydown_callback(sirius::library::user::event::dinput::receiver::value_t::f4);
		}
		else if (key_value == DIK_F5)
		{
			_front->keydown_callback(sirius::library::user::event::dinput::receiver::value_t::f5);
		}
		else if (key_value == DIK_F6)
		{
			_front->keydown_callback(sirius::library::user::event::dinput::receiver::value_t::f6);
		}
		else if (key_value == DIK_F7)
		{
			_front->keydown_callback(sirius::library::user::event::dinput::receiver::value_t::f7);
		}
		else if (key_value == DIK_F8)
		{
			_front->keydown_callback(sirius::library::user::event::dinput::receiver::value_t::f8);
		}
		else if (key_value == DIK_NUMPAD4)
		{
			_front->keydown_seek_callback(-10); //10s
		}
		else if (key_value == DIK_NUMPAD6)
		{
			_front->keydown_seek_callback(10); //10s
		}
		else if (key_value == DIK_NUMPAD5)
		{
			_front->keydown_callback(sirius::library::user::event::dinput::receiver::value_t::f12);
		}
	}
	return sirius::library::user::event::dinput::receiver::err_code_t::success;
}

int32_t sirius::library::user::event::dinput::receiver::core::parse_mouseinput()
{
	if (_mouse_state.lZ != 0)
	{
		_front->mouse_wheel_callback(_mouse_x, _mouse_y, _mouse_state.lZ);
	}

	//left button
	if (_mouse_state.rgbButtons[0] != _prev_mouse_state.rgbButtons[0])
	{
		if ((_mouse_state.rgbButtons[0] & 0x80) ? true : false) //L button down
		{
			_front->mouse_left_button_down_callback(_mouse_x, _mouse_y);
		}
		if ((_prev_mouse_state.rgbButtons[0] & 0x80) ? true : false) //L button up
		{
			DWORD cur_tick = ::GetTickCount();
			if (cur_tick - _left_button_down_tick < 500) //0.5sec
			{
				_front->mouse_left_button_double_callback(_mouse_x, _mouse_y);
			}
			else
			{
				_front->mouse_left_button_up_callback(_mouse_x, _mouse_y);
			}
			_left_button_down_tick = cur_tick;
		}
		
		
	}
	//right button
	if (_mouse_state.rgbButtons[1] != _prev_mouse_state.rgbButtons[1])
	{
		if ((_mouse_state.rgbButtons[1] & 0x80) ? true : false) //R button down
		{
			_front->mouse_right_button_down_callback(_mouse_x, _mouse_y);
		}
		if ((_prev_mouse_state.rgbButtons[1] & 0x80) ? true : false) //R button up
		{
			DWORD cur_tick = ::GetTickCount();
			if (cur_tick - _right_button_down_tick < 500) //0.5sec
			{
				_front->mosue_right_button_double_callback(_mouse_x, _mouse_y);
			}
			else
			{
				_front->mouse_right_button_up_callback(_mouse_x, _mouse_y);
			}
			_right_button_down_tick = cur_tick;
		}
	}

	//move mouse
	if (_mouse_state.lX != 0 || _mouse_state.lY != 0)
	{
		_mouse_x += _mouse_state.lX;
		_mouse_y += _mouse_state.lY;

		if (_mouse_x < 0)
			_mouse_x = 0;
		if (_mouse_y < 0)
			_mouse_y = 0;
		if (_mouse_x > _screen_width)
			_mouse_x = _screen_width;
		if (_mouse_y > _screen_height)
			_mouse_y = _screen_height;

		//if (_mouse_state.rgbButtons[0] & 0x80)
		//	_front->mouse_move_callback(_mouse_x, _mouse_y);
		_front->mouse_move_callback(_mouse_x, _mouse_y);

		//if (_mouse_state.rgbButtons[0] & 0x80)
		//	_front->mouse_left_button_down_move_callback(_mouse_x, _mouse_y);
		//if (_mouse_state.rgbButtons[1] & 0x80)
		//	_front->mouse_right_button_down_move_callback(_mouse_x, _mouse_y);
	}


	return sirius::library::user::event::dinput::receiver::err_code_t::success;
}


