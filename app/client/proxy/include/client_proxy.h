#ifndef _CLIENT_PROXY_H_
#define _CLIENT_PROXY_H_

#include <sirius_client_proxy.h>
#include <sirius_dinput_receiver.h>
#include <sirius_sicp_client.h>

namespace sirius
{
	namespace app
	{
		namespace client
		{
			class proxy::core
				: public sirius::library::net::sicp::client
				, public sirius::library::user::event::dinput::receiver
			{
			public:
				//friend class sirius::app::client::client_cmd;
			public:
				core(sirius::app::client::proxy * front, HINSTANCE instance, HWND hwnd);
				virtual ~core(void);

				int32_t		state(void);
				bool		recv_attendant_info(void);
				const wchar_t * attendant_uuid(void);
				const wchar_t *	address(void);
				const int32_t	streamer_portnumber(void);

				int32_t		set_using_mouse(BOOL value);
				int32_t		set_key_stroke(int32_t interval);

				//IPC Client
				int32_t		connect(wchar_t * address, int32_t portnumber, BOOL reconnection);
				int32_t		disconnect(void);

				//CreateSlot
				int32_t		connect_attendant(wchar_t * appid, wchar_t * deviceid, wchar_t * devicetype, wchar_t * envtype, wchar_t * modeltype, int32_t width, int32_t height, int32_t gpuindex);
				int32_t		disconnect_attendant(void);

				//360 player seek
				int32_t		seek_to(int32_t second);
				int32_t		key_up_play_toggle(void);
				int32_t		key_up_backward(void);
				int32_t		key_up_forward(void);
				int32_t		key_up_reverse(void);

				int32_t		key_down_seek(int32_t diff);
				int32_t		key_up_seek(void);

				int32_t		key_up(int32_t value);
				int32_t		key_down(int32_t value);

				int32_t		mouse_move(int32_t pos_x, int32_t pos_y);
				int32_t		mouse_wheel(int32_t pos_x, int32_t pos_y, int32_t wheel_delta);
				int32_t		mouse_lb_double(int32_t pos_x, int32_t pos_y);
				int32_t		mouse_lb_down(int32_t pos_x, int32_t pos_y);
				int32_t		mouse_lb_up(int32_t pos_x, int32_t pos_y);
				int32_t		mouse_rb_double(int32_t pos_x, int32_t pos_y);
				int32_t		mouse_rb_down(int32_t pos_x, int32_t pos_y);
				int32_t		mouse_rb_up(int32_t pos_x, int32_t pos_y);

			private:
				void	create_session_callback(void);
				void	destroy_session_callback(void);

			public:
				void	connect_attendant_callback(const char * dst, const char * src, const char * msg, size_t length);
				void	disconnect_attendant_callback(const char * dst, const char * src, const char * msg, size_t length);
				void	attendant_info_callback(const char * dst, const char * src, const char * msg, size_t length);

				void	playback_end_callback(const char * dst, const char * src, const char * msg, size_t length);
				void	playback_totaltime_callback(const char * dst, const char * src, const char * msg, size_t length);
				void	playback_currenttime_callback(const char * dst, const char * src, const char * msg, size_t length);
				void	playback_currentrate_callback(const char * dst, const char * src, const char * msg, size_t length);
				void	xml_callback(const char * dst, const char * src, const char * msg, size_t length);
				void	error_callback(const char * dst, const char * src, const char * msg, size_t length);

			private:
				//state
				int32_t		_state;

				//connection & disconnection
				char		_address[64];
				wchar_t		_waddress[64];
				BOOL		_reconnection;

				//Key Callback
				HINSTANCE	_instance;
				HWND		_hwnd;
				BOOL		_dxkey_input_initialized;

				//connect attendant/disconnect attendant/attendant info noti
				BOOL		_recv_attendant_info;
				char		_szattendant_uuid[64];
				wchar_t		_szwattendant_uuid[64];
				int32_t		_streamer_portnumber;


				virtual void keydown_seek_callback(int32_t diff);
				virtual void keyup_seek_callback(void);

				//Keyboard callback
				virtual void keyup_callback(int32_t value);
				virtual void keydown_callback(int32_t value);

				//Mouse callback
				virtual void mouse_move_callback(int32_t pos_x, int32_t pos_y);
				virtual void mouse_wheel_callback(int32_t pos_x, int32_t pos_y, int32_t wheel_delta);
				virtual void mouse_left_button_double_callback(int32_t pos_x, int32_t pos_y);
				virtual void mouse_left_button_down_callback(int32_t pos_x, int32_t pos_y);
				virtual void mouse_left_button_up_callback(int32_t pos_x, int32_t pos_y);
				virtual void mosue_right_button_double_callback(int32_t pos_x, int32_t pos_y);
				virtual void mouse_right_button_down_callback(int32_t pos_x, int32_t pos_y);
				virtual void mouse_right_button_up_callback(int32_t pos_x, int32_t pos_y);

			private:
				sirius::app::client::proxy * _front;
			};
		};
	};
};
#endif