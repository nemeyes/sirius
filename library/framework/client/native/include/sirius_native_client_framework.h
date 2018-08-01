#ifndef _SIRIUS_NATIVE_CLIENT_FRAMEWORK_H_
#define _SIRIUS_NATIVE_CLIENT_FRAMEWORK_H_

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <atlbase.h>

#include <sirius_client_framework.h>

namespace sirius
{
	namespace library
	{
		namespace framework
		{
			namespace client
			{
				class __declspec(dllexport) native
					: public sirius::library::framework::client::base
				{
				public:
					class core;
				public:
					native(void);
					virtual ~native(void);

					int32_t state(void);
					int32_t open(wchar_t * url, int32_t port=0, int32_t recv_option= sirius::library::framework::client::native::media_type_t::video, bool repeat=false);
					int32_t play(HWND hwnd);
					int32_t stop(void);

				private:
					sirius::library::framework::client::native::core * _core;

				};
			};
		};
	};
};

extern "C" __declspec(dllexport) sirius::library::framework::client::base * create_client_framework(void);
extern "C" __declspec(dllexport) void destroy_client_framework(sirius::library::framework::client::base ** client_framework);






#endif