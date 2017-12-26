#ifndef BROWSER_H_
#define BROWSER_H_
#pragma once

#include "include/base/cef_macros.h"
#include "include/base/cef_scoped_ptr.h"
#include "include/cef_command_line.h"
#include "cefclient/renderer/client_app_renderer.h"

namespace client {
	namespace binding {

		class browser : public virtual CefBase {
		public:
			static CefRefPtr<browser> getInstance();
			static void release();
			//bool createSocket();

		protected:
			browser();
			virtual ~browser();

			static CefRefPtr<browser> pThis;
			CefString confErrString_;
			int confErrNum_;

		private:
			IMPLEMENT_REFCOUNTING(browser);
			DISALLOW_COPY_AND_ASSIGN(browser);
		};

	}  // namespace binding
}  // namespace client

#endif  // browser_H_
