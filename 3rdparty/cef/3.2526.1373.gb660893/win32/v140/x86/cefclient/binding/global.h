#ifndef GLOBAL_H_
#define GLOBAL_H_
#pragma once

#include "include/base/cef_macros.h"
#include "include/cef_command_line.h"

namespace client {
	namespace binding {

		class global {
		public:
			static global& get_instance();

		private:
			static global* _instance;

			global();
			DISALLOW_COPY_AND_ASSIGN(global);

		public:
			void set_window_handle(HWND handle) { _handle = handle; }
			void set_java_script_injection(CefString java_script_injection) { _java_script_injection = java_script_injection; }
			HWND get_window_handle() const { return _handle; }
			const CefString& get_java_script_injection() const { return _java_script_injection; }

			typedef enum _JS_MESSAGE_MODE {
				APP_TO_ATTENDANT = 0,
				SYNC_APP_TO_ATTENDANT,
				ATTENDANT_TO_APP,
				SYNC_ATTENDANT_TO_APP,
			}JS_MESSAGE_MODE;

		private:
			HWND _handle;
			CefString _java_script_injection;
		};

	}  // namespace binding
}  // namespace client

#endif  // GLOBAL_H_
