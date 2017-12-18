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
				core(sirius::app::attendant::proxy * front, const char * uuid);
				virtual ~core(void);

				int32_t initialize(sirius::app::attendant::proxy::context_t * contex);
				int32_t release(void);

				int32_t connect(void);
				int32_t disconnect(void);

				int32_t play(void);
				int32_t stop(void);

				void	on_connect_attendant(int32_t code);
				void	on_disconnect_attendant(void);
				void	on_start_attendant(const char * client_uuid, const char * client_id);
				void	on_stop_attendant(const char * client_uuid);
				void	on_destroy(void);

				void	on_key_up(int8_t type, int32_t key);
				void	on_key_down(int8_t type, int32_t key);

				void	on_mouse_move(int32_t pos_x, int32_t pos_y);
				void	on_mouse_lbd(int32_t pos_x, int32_t pos_y);
				void	on_mouse_lbu(int32_t pos_x, int32_t pos_y);
				void	on_mouse_rbd(int32_t pos_x, int32_t pos_y);
				void	on_mouse_rbu(int32_t pos_x, int32_t pos_y);
				void	on_mouse_lb_dclick(int32_t pos_x, int32_t pos_y);
				void	on_mouse_rb_dclick(int32_t pos_x, int32_t pos_y);
				void	on_mouse_wheel(int32_t pos_x, int32_t pos_y, int32_t wheel_z);

				void	app_to_attendant(uint8_t * packet, int32_t len);
				void	on_attendant_to_app(uint8_t * packet, int32_t len);
				void	set_attendant_cb(FuncPtrCallback fncallback) { _callback = fncallback; }

				FuncPtrCallback _callback;

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
				/*
				int32_t _netstate;
				int32_t _mediastate;
				int32_t _unifiedstate;
				*/

				int32_t _key_event_count;
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