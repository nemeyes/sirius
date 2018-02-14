#ifndef _ATTENDANT_PROXY_H_
#define _ATTENDANT_PROXY_H_

#include <sirius_sicp_client.h>
#include "sirius_attendant_proxy.h"
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
				core(sirius::app::attendant::proxy * front, const char * uuid, bool keepalive, bool tls);
				virtual ~core(void);

				int32_t initialize(sirius::app::attendant::proxy::context_t * contex);
				int32_t release(void);

				int32_t connect(void);
				int32_t disconnect(void);

				int32_t play(void);
				int32_t stop(void);

				void	connect_attendant_callback(int32_t code);
				void	disconnect_attendant_callback(void);
				void	start_attendant_callback(const char * client_uuid, const char * client_id);
				void	stop_attendant_callback(const char * client_uuid);
				void	destroy_callback(void);

				void	key_up_callback(int8_t type, int32_t key);
				void	key_down_callback(int8_t type, int32_t key);

				void	mouse_move_callback(int32_t pos_x, int32_t pos_y);
				void	mouse_lbd_callback(int32_t pos_x, int32_t pos_y);
				void	mouse_lbu_callback(int32_t pos_x, int32_t pos_y);
				void	mouse_rbd_callback(int32_t pos_x, int32_t pos_y);
				void	mouse_rbu_callback(int32_t pos_x, int32_t pos_y);
				void	mouse_lb_dclick_callback(int32_t pos_x, int32_t pos_y);
				void	mouse_rb_dclick_callback(int32_t pos_x, int32_t pos_y);
				void	mouse_wheel_callback(int32_t pos_x, int32_t pos_y, int32_t wheel_z);

				void	app_to_attendant(uint8_t * packet, int32_t len);
				void	attendant_to_app_callback(uint8_t * packet, int32_t len);
				void	set_attendant_cb(FuncPtrCallback fncallback) { _callback = fncallback; }

				FuncPtrCallback _callback;

			private:
				template <class T> inline T msecs2ticks(const T & t) { return t * 10000; }

				void on_create_session(void);
				void on_destroy_session(void);

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
				char _client_uuid[64];
				HANDLE		_wait_timer;
				HMODULE		_hmodule;
				//bool		_alloc;
			};
		};
	};
};



#endif