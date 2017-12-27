#if defined(WITH_EXTERNAL_INTERFACE)
#include <sstream>
#include "cefclient/browser/root_window_win.h"
#include "cefclient/browser/util_win.h"
#include "attendant_switches.h"
#include "browser.h"
#include "global.h"
#include "js_binding.h"
#include "socket_win.h"
#include "msg_handler.h"

namespace client {
	namespace binding {

		CefRefPtr<browser> browser::pThis = NULL;

		browser::browser() {
		}

		browser::~browser() {
			pThis = nullptr;
		}

		CefRefPtr<browser> browser::getInstance() {
			if (!pThis) {
				pThis = new browser;
			}

			return pThis;
		}

		void browser::release() {
			message_handler::release();
			socket_win::release();

			if (pThis) {
				pThis->Release();
			}
		}
	}  // namespace csb
}  // namespace client
#endif
