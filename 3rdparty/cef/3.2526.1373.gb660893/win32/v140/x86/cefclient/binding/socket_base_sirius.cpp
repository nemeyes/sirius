
#if defined(WITH_EXTERNAL_INTERFACE)
#include <process.h>
#include <assert.h>
#include "socket_win.h"
#include "socket_base_sirius.h"
#include "attendant_proxy_wrapper.h"
#pragma comment(lib, "Ws2_32.lib")

namespace client {
	namespace binding {

#ifdef _DEBUG
#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

		socketbase::socketbase() {
			_thread_while = TRUE;
			_socket = NULL;
			_event = NULL;
			client::binding::attendant_proxy_wrapper& apc = attendant_proxy_wrapper::getInstance();
			_attendant = apc._proxy;
			_attendant->set_attendant_cb(&calback_attendant_to_app);
		}

		socketbase::~socketbase() {

		}

		void socketbase::ProcessingThread()
		{

		}

		int socketbase::send_data(const void *lpBuf, int nBuflen, int nFlags)
		{
			_attendant->app_to_attendant((uint8_t *)lpBuf, nBuflen);
			return true;
		}

		bool socketbase::create(UINT nSocketPort, int nSocketType, LPCTSTR lpszSocketAddress)
		{
			return true;
		}

		void socketbase::calback_attendant_to_app(uint8_t* packet, size_t len)
		{
			socket_win::get_instance()->sirius_to_javascript((uint8_t*)packet, len);
		}

	}  // namespace binding
}  // namespace client
#endif