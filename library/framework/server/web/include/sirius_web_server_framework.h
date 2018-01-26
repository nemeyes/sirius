#ifndef _SIRIUS_WEB_SERVER_FRAMEWORK_H_
#define _SIRIUS_WEB_SERVER_FRAMEWORK_H_

#include <sirius_server_framework.h>
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace sirius
{
	namespace library
	{
		namespace framework
		{
			namespace server
			{
				class __declspec(dllexport) web
					: public sirius::library::framework::server::base
				{
				public:
					class core;
					class d3d11_video_source;
					class host_video_source;
				public:
					web(void);
					virtual ~web(void);

					void	set_notification_callee(sirius::library::misc::notification::internal::notifier::callee* callee);

					int32_t initialize(sirius::library::framework::server::web::context_t * context);
					int32_t release(void);
					int32_t play(void);
					int32_t pause(void);
					int32_t stop(void);
					int32_t state(void) const;

					void	on_keyup(int32_t key);
					void	on_keydown(int32_t key);

					void	on_L_mouse_down(int32_t pos_x, int32_t pos_y);
					void	on_L_mouse_up(int32_t pos_x, int32_t pos_y);
					void	on_R_mouse_down(int32_t pos_x, int32_t pos_y);
					void	on_R_mouse_up(int32_t pos_x, int32_t pos_y);
					void	on_L_mouse_dclick(int32_t pos_x, int32_t pos_y);
					void	on_R_mouse_dclick(int32_t pos_x, int32_t pos_y);
					void	on_mouse_move(int32_t pos_x, int32_t pos_y);
					void	on_mouse_wheel(int32_t pos_x, int32_t pos_y, int32_t wheel_delta);
					void	on_end2end_data(const uint8_t * packet, int32_t packet_size);

				private:
					sirius::library::framework::server::web::core * _core;
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

#endif
