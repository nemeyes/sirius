#ifndef _DINPU_RECEIVER_H_
#define _DINPU_RECEIVER_H_

#define DIRECTINPUT_VERSION 0x0800

#include "sirius_dinput_receiver.h"
#include <atlbase.h>
#include <dinput.h>

#include <time.h>
namespace sirius
{
	namespace library
	{
		namespace user
		{
			namespace event
			{
				namespace dinput
				{
					class receiver::core
					{
					public:
						core(sirius::library::user::event::dinput::receiver * front);
						virtual ~core(void);

						int32_t initialize(HINSTANCE inst, HWND hwnd);
						int32_t release(void);

						void	set_using_mouse(bool value) { _using_mouse = value; }
						void	set_keystroke(int32_t interval) { _keystroke_interval = interval; }

					private:
						static unsigned __stdcall process_callback(void * param);
						void process(void);

						int32_t read_keyboard(void);
						int32_t read_mouse(void);

						int32_t parse_keyinput(int32_t key_value, bool up);
						int32_t parse_mouseinput();

					private:
						sirius::library::user::event::dinput::receiver * _front;

						ATL::CComPtr<IDirectInput8> _dx_input;
						ATL::CComPtr<IDirectInputDevice8> _keyboard;
						ATL::CComPtr<IDirectInputDevice8> _mouse;

						bool	_using_mouse;
						int32_t _keystroke_interval;

						int32_t _screen_width;
						int32_t _screen_height;
						int32_t _mouse_x;
						int32_t _mouse_y;

						unsigned char	_keyboard_state[256];
						unsigned char	_prev_keyboard_state[256];
						clock_t			_prev_keyboard_clock[256];
						DIMOUSESTATE	_mouse_state;
						DIMOUSESTATE	_prev_mouse_state;

						DWORD	_left_button_down_tick;
						DWORD	_right_button_down_tick;

						bool	_run;
						HANDLE	_thread;
					};
				};
			};
		};
	};
};

#endif
