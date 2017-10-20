#ifndef _SIRIUS_ATTENDANT_PROXY_H_
#define _SIRIUS_ATTENDANT_PROXY_H_

#include <sirius.h>

#if defined(EXPORT_ATTENDANT_PROXY)
#define EXP_ATTENDANT_PROXY_CLASS __declspec(dllexport)
#else
#define EXP_ATTENDANT_PROXY_CLASS __declspec(dllimport)
#endif


typedef void(*FuncPtrCallback)(const char *, int);

namespace sirius
{
	namespace app
	{
		namespace attendant
		{
			class EXP_ATTENDANT_PROXY_CLASS proxy
				: public sirius::base
			{
				class core;
			public:
				typedef struct _netstate_t
				{
					static const int32_t connecting = 0;
					static const int32_t connected = 1;
					static const int32_t disconnecting = 2;
					static const int32_t disconnected = 3;
				} netstate_t;

				typedef struct _mediastate_t
				{
					static const int32_t stopped = 0;
					static const int32_t playing = 1;
					static const int32_t paused = 2;
				} mediastate_t;

				typedef struct EXP_ATTENDANT_PROXY_CLASS _context_t
				{
					int32_t type;
					int32_t reconnect;
					wchar_t url[MAX_PATH];
					int32_t video_codec;
					int32_t video_width;
					int32_t video_height;
					int32_t video_fps;
					int32_t video_nbuffer;

					int32_t gpuindex;
					int32_t present;
					int32_t repeat;
					int32_t device_type;
					HWND	hwnd;
					void *	user_data;

					wchar_t uuid[MAX_PATH];
					wchar_t client_uuid[MAX_PATH];
					wchar_t device_id[MAX_PATH];
					wchar_t control_server_address[MAX_PATH];
					int32_t control_server_portnumber;
					wchar_t streamer_address[MAX_PATH];
					int32_t streamer_portnumber;
					int32_t play_after_connect;
					int32_t render_type;

					_context_t(void)
						: type(sirius::app::attendant::proxy::attendant_type_t::web)
						, reconnect(true)
						, video_codec(sirius::app::attendant::proxy::video_submedia_type_t::unknown)
						, video_width(1280)
						, video_height(720)
						, video_fps(0)
						, video_nbuffer(1)
						, gpuindex(0)
						, present(false)
						, repeat(true)
						, device_type(0)
						, hwnd(NULL)
						, user_data(NULL)
						, control_server_portnumber(15000)
						, streamer_portnumber(0)
						, play_after_connect(0)
						, render_type(0)
					{
						memset(url, 0x00, sizeof(url));
						memset(uuid, 0x00, sizeof(uuid));
						memset(device_id, 0x00, sizeof(device_id));
						memset(client_uuid, 0x00, sizeof(uuid));
						memset(control_server_address, 0x00, sizeof(control_server_address));
						memset(streamer_address, 0x00, sizeof(streamer_address));
					}

					~_context_t(void)
					{

					}
				} context_t;


				static bool parse_argument(int32_t argc, wchar_t * argv[]);
				static sirius::app::attendant::proxy & instance(void);

				virtual sirius::app::attendant::proxy::context_t * context(void);

				virtual bool	is_initialized(void);

				virtual int32_t initialize(void);
				virtual int32_t release(void);

				virtual int32_t connect(void);
				virtual int32_t disconnect(void);

				virtual int32_t play(void);
				virtual int32_t stop(void);

				int32_t play_toggle(void);
				int32_t backward(void);
				int32_t forward(void);
				int32_t reverse(void);

				virtual void on_destroy(void);

				virtual void on_key_up(int8_t type, int32_t key);
				virtual void on_key_down(int8_t type, int32_t key);

				virtual void on_mouse_move(int32_t pos_x, int32_t pos_y);
				virtual void on_mouse_lbd(int32_t pos_x, int32_t pos_y);
				virtual void on_mouse_lbu(int32_t pos_x, int32_t pos_y);
				virtual void on_mouse_rbd(int32_t pos_x, int32_t pos_y);
				virtual void on_mouse_rbu(int32_t pos_x, int32_t pos_y);
				virtual void on_mouse_lb_dclick(int32_t pos_x, int32_t pos_y);
				virtual void on_mouse_rb_dclick(int32_t pos_x, int32_t pos_y);
				virtual void on_mouse_wheel(int32_t pos_x, int32_t pos_y, int32_t wheel_z);

				virtual void on_gyro(float x, float y, float z);
				virtual void on_pinch_zoom(float delta);

				virtual void on_gyro_attitude(float x, float y, float z, float w);
				virtual void on_gyro_gravity(float x, float y, float z);
				virtual void on_gyro_rotation_rate(float x, float y, float z);
				virtual void on_gyro_rotation_rate_unbiased(float x, float y, float z);
				virtual void on_gyro_user_acceleration(float x, float y, float z);

				virtual void on_gyro_enabled_attitude(bool state);
				virtual void on_gyro_enabled_gravity(bool state);
				virtual void on_gyro_enabled_rotation_rate(bool state);
				virtual void on_gyro_enabled_rotation_rate_unbiased(bool state);
				virtual void on_gyro_enabled_user_acceleration(bool state);
				virtual void on_gyro_update_interval(float interval);

				virtual void on_ar_view_mat(float m00, float m01, float m02, float m03,
					float m10, float m11, float m12, float m13,
					float m20, float m21, float m22, float m23,
					float m30, float m31, float m32, float m33);
				virtual void on_ar_proj_mat(float m00, float m01, float m02, float m03,
					float m10, float m11, float m12, float m13,
					float m20, float m21, float m22, float m23,
					float m30, float m31, float m32, float m33);

				void send(char * packet, int len);
				void on_container_to_app(char * packet, int len);
				void set_webcontainer_callback(FuncPtrCallback fncallback);

				virtual void OnAssocCompletion(void) {}
				virtual void OnLeaveCompletion(void) {}

			private:
				proxy(void);
				virtual ~proxy(void);

			private:
				sirius::app::attendant::proxy::context_t _context;
				sirius::app::attendant::proxy::core * _core;
				bool _key_pressed;
				bool _initialized;
			};
		};
	};
};

#endif