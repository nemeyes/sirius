#ifndef _SIRIUS_NATIVE_STRESSOR_FRAMEWORK_H_
#define _SIRIUS_NATIVE_STRESSOR_FRAMEWORK_H_

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <atlbase.h>

#include <sirius_client_framework.h>
#include <stressor_controller.h>

class stressor_controller;
namespace sirius
{
	namespace library
	{
		namespace framework
		{
			namespace stressor
			{
				class native
					: public sirius::library::framework::client::base
				{
				public:
					class core;
				public:
					native(stressor_controller * front);
					virtual ~native(void);

					int32_t state(void);
					int32_t open(wchar_t * url, int32_t port = 0, int32_t recv_option = sirius::library::framework::stressor::native::media_type_t::video, bool reconnect = false, bool keepalive = false, int32_t keepalive_timeout = 5000);
					int32_t play(HWND hwnd);
					int32_t stop(void);

					int32_t change_debug_level(int32_t level) { return err_code_t::success; }

					void on_connect_stream(void);
					void on_disconnect_stream(void);
					void on_recv_stream(void);

				private:
					sirius::library::framework::stressor::native::core * _core;
					stressor_controller * _front;
				};
			};
		};
	};
};







#endif