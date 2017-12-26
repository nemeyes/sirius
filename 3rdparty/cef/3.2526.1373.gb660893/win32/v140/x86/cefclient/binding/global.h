#ifndef GLOBAL_H_
#define GLOBAL_H_
#pragma once

#include "include/base/cef_macros.h"
#include "include/cef_command_line.h"

namespace client {
	namespace binding {

		class global {
		public:
			static global& getInstance();

		private:
			static global* sInstance;

			global();
			DISALLOW_COPY_AND_ASSIGN(global);

		public:
			void setWindowHandle(HWND hWnd) { hWnd_ = hWnd; }
			HWND getWindowHandle() const { return hWnd_; }

			// Javascript Inejction
			const CefString& getJavaScriptInjection() const { return javaScriptInjection_; }

		private:
			//windows handle
			HWND hWnd_;

			// Javascript Inejction
			CefString javaScriptInjection_;
		};

	}  // namespace csb
}  // namespace client

#endif  // GLOBAL_H_
