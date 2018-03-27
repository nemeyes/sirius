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

		CefRefPtr<browser> browser::_ptr_this = NULL;

		browser::browser() {
			socket_win::get_instance();
		}

		browser::~browser() {
			_ptr_this = nullptr;
		}

		CefRefPtr<browser> browser::getInstance() {
			if (!_ptr_this) {
				_ptr_this = new browser;
			}

			return _ptr_this;
		}

		void browser::release() {
			//message_handler::release();
			socket_win::release();

			if (_ptr_this) {
				_ptr_this->Release();
			}
		}
	}  // namespace binding
}  // namespace client

#endif
