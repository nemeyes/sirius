#ifndef _CLIENT_PROXY_H_
#define _CLIENT_PROXY_H_

#include <sirius_sicp_client.h>
#include <sirius_client_proxy.h>

namespace sirius
{
	namespace app
	{
		namespace client
		{
			class proxy::core
				: public sirius::library::net::sicp::client
			{
			public:
				//friend class sirius::app::client::client_cmd;
			public:
				core(sirius::app::client::proxy * front, HINSTANCE instance, HWND hwnd);
				virtual ~core(void);

				int32_t			state(void);
				bool			recv_attendant_info(void);
				const wchar_t * attendant_uuid(void);
				const wchar_t *	address(void);
				const int32_t	streamer_portnumber(void);

				int32_t			connect(wchar_t * address, int32_t portnumber, bool reconnection);
				int32_t			disconnect(void);

				int32_t			connect_client(wchar_t * id);
				int32_t			disconnect_client(void);

				int32_t			key_up(int32_t value);
				int32_t			key_down(int32_t value);

				int32_t			mouse_move(int32_t pos_x, int32_t pos_y);
				int32_t			mouse_wheel(int32_t pos_x, int32_t pos_y, int32_t wheel_delta);
				int32_t			mouse_lb_double(int32_t pos_x, int32_t pos_y);
				int32_t			mouse_lb_down(int32_t pos_x, int32_t pos_y);
				int32_t			mouse_lb_up(int32_t pos_x, int32_t pos_y);
				int32_t			mouse_rb_double(int32_t pos_x, int32_t pos_y);
				int32_t			mouse_rb_down(int32_t pos_x, int32_t pos_y);
				int32_t			mouse_rb_up(int32_t pos_x, int32_t pos_y);

			private:
				void			on_create_session(void);
				void			on_destroy_session(void);

			public:
				void			connect_client_callback(int32_t code, const char * msg);
				void			disconnect_client_callback(int32_t code);
				void			attendant_info_callback(int32_t code, const char * container_uuid, int32_t streamer_portnumber, int32_t video_width, int32_t video_height);

				void			xml_callback(const char * msg, size_t length);
				void			error_callback(int32_t code);

			private:
				//state
				int32_t		_state;

				//connection & disconnection
				char		_address[64];
				wchar_t		_waddress[64];
				bool		_reconnection;

				//connect attendant/disconnect attendant/attendant info noti
				bool		_recv_attendant_info;
				char		_szattendant_uuid[64];
				wchar_t		_szwattendant_uuid[64];
				int32_t		_streamer_portnumber;

			private:
				sirius::app::client::proxy * _front;
			};
		};
	};
};
#endif