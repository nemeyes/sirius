#ifndef _ATTENDANT_PROXY_H_
#define _ATTENDANT_PROXY_H_

#include "sirius_attendant_proxy.h"
#include <sirius_sicp_client.h>
#include <sirius_unified_server.h>
#include <sirius_internal_notifier.h>
#include <sirius_server_framework.h>

namespace sirius
{
	namespace app
	{
		namespace attendant
		{
			class proxy::core
				: public sirius::library::net::sicp::client
				, public sirius::library::misc::notification::internal::notifier::callee
			{
			public:
				core(sirius::app::attendant::proxy * front, const char * uuid, const char * client_uuid);
				virtual ~core(void);

				int32_t initialize(sirius::app::attendant::proxy::context_t * contex);
				int32_t release(void);

				int32_t connect(void);
				int32_t disconnect(void);

				int32_t play(void);
				int32_t stop(void);

				int32_t play_toggle(void);
				int32_t backward(void);
				int32_t forward(void);
				int32_t reverse(void);
				

				void on_destroy(void);

				void on_key_up(int8_t type, int32_t key);
				void on_key_down(int8_t type, int32_t key);

				void on_mouse_move(int32_t pos_x, int32_t pos_y);
				void on_mouse_lbd(int32_t pos_x, int32_t pos_y);
				void on_mouse_lbu(int32_t pos_x, int32_t pos_y);
				void on_mouse_rbd(int32_t pos_x, int32_t pos_y);
				void on_mouse_rbu(int32_t pos_x, int32_t pos_y);
				void on_mouse_lb_dclick(int32_t pos_x, int32_t pos_y);
				void on_mouse_rb_dclick(int32_t pos_x, int32_t pos_y);
				void on_mouse_wheel(int32_t pos_x, int32_t pos_y, int32_t wheel_z);

				void on_gyro(float x, float y, float z);
				void on_pinch_zoom(float delta);

				void on_gyro_attitude(float x, float y, float z, float w);
				void on_gyro_gravity(float x, float y, float z);
				void on_gyro_rotation_rate(float x, float y, float z);
				void on_gyro_rotation_rate_unbiased(float x, float y, float z);
				void on_gyro_user_acceleration(float x, float y, float z);

				void on_gyro_enabled_attitude(bool state);
				void on_gyro_enabled_gravity(bool state);
				void on_gyro_enabled_rotation_rate(bool state);
				void on_gyro_enabled_rotation_rate_unbiased(bool state);
				void on_gyro_enabled_user_acceleration(bool state);
				void on_gyro_update_interval(float interval);

				void on_ar_view_mat(float m00, float m01, float m02, float m03,
					float m10, float m11, float m12, float m13,
					float m20, float m21, float m22, float m23,
					float m30, float m31, float m32, float m33);
				void on_ar_proj_mat(float m00, float m01, float m02, float m03,
					float m10, float m11, float m12, float m13,
					float m20, float m21, float m22, float m23,
					float m30, float m31, float m32, float m33);

				void on_app_to_container_xml(char * packet, int len);
				void on_container_to_app(char * packet, int len);
				virtual void set_callback(FuncPtrCallback fncallback) { callback = fncallback; }

				FuncPtrCallback callback;
			private:
				template <class T> inline T msecs2ticks(const T & t) { return t * 10000; }

				void create_session_callback(void);
				void destroy_session_callback(void);

				static void __stdcall alive_timer_callback(LPVOID args, DWORD low, DWORD high);

				void on_recv_notification(int32_t type, char* msg, int32_t size);

				void on_key_board_up(int32_t key);
				void on_key_board_down(int32_t key);

			private:
				sirius::app::attendant::proxy * _front;

				sirius::app::attendant::proxy::context_t * _context;
				int32_t _netstate;
				int32_t _mediastate;
				int32_t _unifiedstate;
				int32_t _key_event_count;
				sirius::library::unified::server::context_t _unified_context;

				char _client_uuid[64];
				sirius::library::framework::server::base * _framework;
				sirius::library::framework::server::base::context_t * _framework_context;
				uint32_t	_pid;
				HANDLE		_wait_timer;
				HMODULE		_hmodule;
			};
		};
	};
};



#endif