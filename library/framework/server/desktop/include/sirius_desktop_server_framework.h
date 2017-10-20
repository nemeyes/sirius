#ifndef _SIRIUS_DESKTOP_SERVER_FRAMEWORK_H_
#define _SIRIUS_DESKTOP_SERVER_FRAMEWORK_H_

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <sirius_server_framework.h>

namespace sirius
{
	namespace library
	{
		namespace framework
		{
			namespace server
			{
				class __declspec(dllexport) desktop
					: public sirius::library::framework::server::base
				{
				public:
					class core;
					class video_source;
				public:
					desktop(void);
					virtual ~desktop(void);

					void	set_notification_callee(sirius::library::misc::notification::internal::notifier::callee* callee);

					int32_t open(sirius::library::framework::server::desktop::context_t * context);
					int32_t close(void);
					int32_t play(void);
					int32_t pause(void);
					int32_t stop(void);
					int32_t state(void) const;

					void	on_keyup(int32_t key);
					void	on_keydown(int32_t key);

					int32_t seek(int32_t diff);
					int32_t seek_to(int32_t second);
					int32_t seek_stop(void);

					int32_t forward(void);
					int32_t backward(void);
					int32_t reverse(void);
					int32_t play_toggle(void);

					void	on_L_mouse_down(int32_t pos_x, int32_t pos_y);
					void	on_L_mouse_up(int32_t pos_x, int32_t pos_y);
					void	on_R_mouse_down(int32_t pos_x, int32_t pos_y);
					void	on_R_mouse_up(int32_t pos_x, int32_t pos_y);
					void	on_L_mouse_dclick(int32_t pos_x, int32_t pos_y);
					void	on_R_mouse_dclick(int32_t pos_x, int32_t pos_y);
					void	on_mouse_move(int32_t pos_x, int32_t pos_y);
					void	on_mouse_wheel(int32_t pos_x, int32_t pos_y, int32_t wheel_delta);

					void	on_gyro(float x, float y, float z);
					void	on_pinch_zoom(float delta);

					void	on_gyro_attitude(float x, float y, float z, float w);
					void	on_gyro_gravity(float x, float y, float z);
					void	on_gyro_rotation_rate(float x, float y, float z);
					void	on_gyro_rotation_rate_unbiased(float x, float y, float z);
					void	on_gyro_user_acceleration(float x, float y, float z);

					void	on_gyro_enabled_attitude(bool state);
					void	on_gyro_enabled_gravity(bool state);
					void	on_gyro_enabled_rotation_rate(bool state);
					void	on_gyro_enabled_rotation_rate_unbiased(bool state);
					void	on_gyro_enabled_user_acceleration(bool state);
					void	on_gyro_updateinterval(float interval);

					void on_ar_view_mat(float m00, float m01, float m02, float m03,
						float m10, float m11, float m12, float m13,
						float m20, float m21, float m22, float m23,
						float m30, float m31, float m32, float m33) {};
					void on_ar_proj_mat(float m00, float m01, float m02, float m03,
						float m10, float m11, float m12, float m13,
						float m20, float m21, float m22, float m23,
						float m30, float m31, float m32, float m33) {};

					void on_infoxml(const char * msg, int32_t length);

				private:
					sirius::library::framework::server::desktop::core * _core;
				};
			};
		};
	};
};

extern "C"
{
	__declspec(dllexport) sirius::library::framework::server::base * create_server_framework(void);
	__declspec(dllexport) void destroy_server_framework(sirius::library::framework::server::base ** player_framework);
}

extern "C" {
	typedef struct _gyro_data
	{
		float x, y, z, w;
	}gyro_data;

	__declspec(dllexport) gyro_data	get_attitude();
	__declspec(dllexport) gyro_data	get_gravity();
	__declspec(dllexport) gyro_data	get_rotation_rate();
	__declspec(dllexport) gyro_data	get_rotation_rate_unbiased();
	__declspec(dllexport) gyro_data	get_user_acceleration();
	__declspec(dllexport) void		set_gyro_enabled(bool state);
	__declspec(dllexport) bool		get_gyro_enabled_attitude();
	__declspec(dllexport) void		set_gyro_enabled_attitude(bool state);
	__declspec(dllexport) bool		get_gyro_enabled_gravity();
	__declspec(dllexport) void		set_gyro_enabled_gravity(bool state);
	__declspec(dllexport) bool		get_gyro_enabled_rotation_rate();
	__declspec(dllexport) void		set_gyro_enabled_rotation_rate(bool state);
	__declspec(dllexport) bool		get_gyro_enabled_rotation_rate_unbaised();
	__declspec(dllexport) void		set_gyro_enabled_rotation_rate_unbaised(bool state);
	__declspec(dllexport) bool		get_gyro_enabled_user_acceleration();
	__declspec(dllexport) void		set_gyro_enabled_user_acceleration(bool state);
	__declspec(dllexport) float		get_gyro_updateinterval();
	__declspec(dllexport) void		set_gyro_updateinterval(float interval);

	__declspec(dllexport) int*		get_xml_msg();
	__declspec(dllexport) void		send_xml_msg(int* msg);
}

#ifdef __cplusplus
	struct gyro_data_ex : _gyro_data
	{
		gyro_data_ex() { x = 0.0f; y = 0.0f; z = 0.0f; w = 0.0f; }
		gyro_data_ex(float _x, float _y, float _z, float _w) { x = _x;  y = _y; z = _z; w = _w; }
	};
#endif

#endif
