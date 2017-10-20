#ifndef _SIRIUS_DINPUT_RECEIVER_H_
#define _SIRIUS_DINPUT_RECEIVER_H_

#include <sirius.h>

#if defined(EXPORT_DINPUT_RECEIVER_LIB)
#define EXP_DINPUT_RECEIVER_CLASS __declspec(dllexport)
#else
#define EXP_DINPUT_RECEIVER_CLASS __declspec(dllimport)
#endif

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
					class EXP_DINPUT_RECEIVER_CLASS receiver
						: public sirius::base
					{
					public:
						class core;
					public:
						typedef struct _type_t
						{
							static const int8_t keyboard = 0;
						} type_t;

						typedef struct _value_t
						{
							static const int32_t scrlk = 145;
							static const int32_t enter = 13;
							static const int32_t esc = 27;
							static const int32_t space = 32;
							static const int32_t left = 37;
							static const int32_t up = 38;
							static const int32_t right = 39;
							static const int32_t down = 40;
							static const int32_t a = 65;
							static const int32_t z = 90;
							static const int32_t f2 = 113;
							static const int32_t f3 = 114;
							static const int32_t f4 = 115;
							static const int32_t f5 = 116;
							static const int32_t f6 = 117;
							static const int32_t f7 = 118;
							static const int32_t f8 = 119;
							static const int32_t f12 = 123;
						} value_t;

						receiver(void);
						virtual ~receiver(void);

						int32_t initialize(HINSTANCE inst, HWND hwnd);
						int32_t release(void);

						void	set_using_mouse(bool value);
						void	set_keystroke(int32_t interval);

						virtual void keyup_callback(int32_t value) {};
						virtual void keydown_callback(int32_t value) {};

						virtual void mouse_move_callback(int32_t pos_x, int32_t pos_y) {};
						virtual void mouse_left_button_down_callback(int32_t pos_x, int32_t pos_y) {};
						virtual void mouse_left_button_up_callback(int32_t pos_x, int32_t pos_y) {};
						virtual void mouse_right_button_down_callback(int32_t pos_x, int32_t pos_y) {};
						virtual void mouse_right_button_up_callback(int32_t pos_x, int32_t pos_y) {};
						//virtual void mouse_left_button_down_move_callback(int32_t pos_x, int32_t pos_y) {};
						//virtual void mouse_right_button_down_move_callback(int32_t pos_x, int32_t pos_y) {};
						virtual void mouse_left_button_double_callback(int32_t pos_x, int32_t pos_y) {};
						virtual void mosue_right_button_double_callback(int32_t pos_x, int32_t pos_y) {};
						virtual void mouse_wheel_callback(int32_t pos_x, int32_t pos_y, int32_t wheel_delta) {};


						virtual void keydown_seek_callback(int32_t diff) {};
						virtual void keyup_seek_callback() {};
						virtual void keyup_play_toggle() {};
						virtual void keyup_backward() {};
						virtual void keyup_forward() {};
						virtual void keyup_reverse() {};

						//virtual void seek(int32_t pos) {};
					private:
						receiver(const sirius::library::user::event::dinput::receiver & clone);

					private:
						sirius::library::user::event::dinput::receiver::core * _core;

					};
				};
			};
		};
	};
};

#endif
