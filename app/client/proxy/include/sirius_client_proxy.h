#ifndef _SIRIUS_CLIENT_PROXY_H_
#define _SIRIUS_CLIENT_PROXY_H_

#if defined(EXPORT_SIRIUS_CLIENT_PROXY_LIB)
#define EXP_SIRIUS_CLIENT_PROXY_CLASS __declspec(dllexport)
#else
#define EXP_SIRIUS_CLIENT_PROXY_CLASS __declspec(dllimport)
#endif

#include <sirius.h>

namespace sirius
{
	namespace app
	{
		namespace client
		{
			class client_cmd;
			class EXP_SIRIUS_CLIENT_PROXY_CLASS proxy
				: public sirius::base
			{
			public:
				class core;
				friend class sirius::app::client::client_cmd;
			public:

				typedef struct _state_t
				{
					static const int32_t connecting = 0;
					static const int32_t connected = 1;
					static const int32_t disconnecting = 2;
					static const int32_t disconnected = 3;
					static const int32_t creating_attendant = 4;
					static const int32_t created_attendant = 5;
					static const int32_t destroying_attendant = 6;
					static const int32_t destroyed_attendant = 7;
				} state_t;

				class EXP_SIRIUS_CLIENT_PROXY_CLASS handler
				{
				public:
					handler(void);
					virtual ~handler(void);

					void set_proxy(sirius::app::client::proxy * prxy);

					int32_t state(void);

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

					virtual void on_pre_connect(wchar_t * address, int32_t portnumber, bool reconnection) = 0;
					virtual void on_post_connect(wchar_t * address, int32_t portnumber, bool reconnection) = 0;
					virtual void on_pre_disconnect(void) = 0;
					virtual void on_post_disconnect(void) = 0;

					virtual void on_pre_create_session(void) = 0;
					virtual void on_create_session(void) = 0;
					virtual void on_post_create_session(void) = 0;

					virtual void on_pre_destroy_session(void) = 0;
					virtual void on_destroy_session(void) = 0;
					virtual void on_post_destroy_session(void) = 0;

					virtual void on_pre_connect_client(int32_t code, wchar_t * msg) = 0;
					virtual void on_connect_client(int32_t code, wchar_t * msg) = 0;
					virtual void on_post_connect_client(int32_t code, wchar_t * msg) = 0;

					virtual void on_pre_disconnect_client(int32_t code) = 0;
					virtual void on_disconnect_client(int32_t code) = 0;
					virtual void on_post_disconnect_client(int32_t code) = 0;

					virtual void on_pre_attendant_info(int32_t code, wchar_t * attendant_uuid, int32_t streamer_portnumber, int32_t video_width, int32_t video_height) = 0;
					virtual void on_post_attendant_info(int32_t code, wchar_t * attendant_uuid, int32_t streamer_portnumber, int32_t video_width, int32_t video_height) = 0;

					virtual void on_open_streaming(wchar_t * attendant_uuid, int32_t streamer_portnumber, bool reconnection) = 0;
					virtual void on_play_streaming(void) = 0;
					virtual void on_stop_streaming(void) = 0;

					virtual void on_pre_xml(const char * msg, size_t length) = 0;
					virtual void on_xml(const char * msg, size_t length) = 0;
					virtual void on_post_xml(const char * msg, size_t length) = 0;

					virtual void on_pre_error(int32_t error_code) = 0;
					virtual void on_error(int32_t error_code) = 0;
					virtual void on_post_error(int32_t error_code) = 0;

				private:
					sirius::app::client::proxy * _proxy;

				};

				proxy(sirius::app::client::proxy::handler * hndlr, HINSTANCE instance, HWND hwnd);
				virtual ~proxy(void);

				int32_t state(void);

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
				void on_pre_connect(wchar_t * address, int32_t portnumber, bool reconnection);
				void on_post_connect(wchar_t * address, int32_t portnumber, bool reconnection);
				void on_pre_disconnect(void);
				void on_post_disconnect(void);

				void on_pre_create_session(void);
				void on_create_session(void);
				void on_post_create_session(void);

				void on_pre_destroy_session(void);
				void on_destroy_session(void);
				void on_post_destroy_session(void);

				void on_pre_connect_client(int32_t code, wchar_t * msg);
				void on_connect_client(int32_t code, wchar_t * msg);
				void on_post_connect_client(int32_t code, wchar_t * msg);

				void on_pre_disconnect_client(int32_t code);
				void on_disconnect_client(int32_t code);
				void on_post_disconnect_client(int32_t code);


				void on_pre_attendant_info(int32_t code, wchar_t * attendant_uuid, int32_t streamer_portnumber, int32_t video_width, int32_t video_height);
				void on_post_attendant_info(int32_t code, wchar_t * attendant_uuid, int32_t streamer_portnumber, int32_t video_width, int32_t video_height);

				void on_open_streaming(wchar_t * attendant_uuid, int32_t streamer_portnumber, bool reconnection);
				void on_play_streaming(void);
				void on_stop_streaming(void);

				void on_pre_xml(const char * msg, size_t length);
				void on_xml(const char * msg, size_t length);
				void on_post_xml(const char * msg, size_t length);

				void on_pre_error(int32_t error_code);
				void on_error(int32_t error_code);
				void on_post_error(int32_t error_code);

			private:
				sirius::app::client::proxy::handler *	_handler;
				sirius::app::client::proxy::core *		_core;
			};

		};
	};
};

#endif