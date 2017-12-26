// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include <windows.h>

#include "include/base/cef_scoped_ptr.h"
#include "include/cef_command_line.h"
#include "include/cef_sandbox_win.h"
#include "cefclient/browser/client_app_browser.h"
#include "cefclient/browser/main_context_impl.h"
#include "cefclient/browser/main_message_loop_multithreaded_win.h"
#include "cefclient/browser/main_message_loop_std.h"
#include "cefclient/browser/root_window_manager.h"
#include "cefclient/browser/test_runner.h"
#include "cefclient/common/client_app_other.h"
#include "cefclient/renderer/client_app_renderer.h"
#if defined(WITH_JAVASCRIPT)
#include "cefclient/binding/attendent_proxy_wrapper.h"
#endif
#if defined(WITH_ATTENDANT_PROXY)
#include <windows.h>
#include <shellapi.h>
#include <sirius_attendant_proxy.h>

#pragma comment(lib, "sirius_attendant_proxy.lib")
#endif
// When generating projects with CMake the CEF_USE_SANDBOX value will be defined
// automatically if using the required compiler version. Pass -DUSE_SANDBOX=OFF
// to the CMake command-line to disable use of the sandbox.
// Uncomment this line to manually enable sandbox support.
// #define CEF_USE_SANDBOX 1

#if defined(CEF_USE_SANDBOX)
// The cef_sandbox.lib static library is currently built with VS2013. It may not
// link successfully with other VS versions.
#pragma comment(lib, "cef_sandbox.lib")
#endif

namespace client {
namespace {

int RunMain(HINSTANCE hInstance, int nCmdShow) {
  // Enable High-DPI support on Windows 7 or newer.
  CefEnableHighDPISupport();
#if defined(WITH_JAVASCRIPT)
  HWND proxy_handle = NULL;
#endif
  CefMainArgs main_args(hInstance);

  void* sandbox_info = NULL;

#if defined(CEF_USE_SANDBOX)
  // Manage the life span of the sandbox information object. This is necessary
  // for sandbox support on Windows. See cef_sandbox_win.h for complete details.
  CefScopedSandboxInfo scoped_sandbox;
  sandbox_info = scoped_sandbox.sandbox_info();
#endif

  // Parse command-line arguments.
  CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
  command_line->InitFromString(::GetCommandLineW());

  // Create a ClientApp of the correct type.
  CefRefPtr<CefApp> app;
  ClientApp::ProcessType process_type = ClientApp::GetProcessType(command_line);
  if (process_type == ClientApp::BrowserProcess)
  {
    app = new ClientAppBrowser();
#if defined(WITH_JAVASCRIPT)
	  client::binding::attendent_proxy_wrapper& apc = client::binding::attendent_proxy_wrapper::getInstance();
	  apc.Initialize();
	  proxy_handle = apc._proxy_handle;
#endif
  }
  else if (process_type == ClientApp::RendererProcess)
    app = new ClientAppRenderer();
  else if (process_type == ClientApp::OtherProcess)
    app = new ClientAppOther();

#ifdef WITH_ATTENDANT_PROXY
#if defined(WITH_JAVASCRIPT)
#else
  sirius::app::attendant::proxy * proxy = nullptr;
#endif
  if (command_line->HasSwitch("single-process") || (process_type == ClientApp::BrowserProcess && command_line->HasSwitch("off-screen-rendering-enabled")) || (process_type == ClientApp::OtherProcess && !command_line->HasSwitch("off-screen-rendering-enabled")))
  {
#if defined(WITH_JAVASCRIPT)

#else
	  wchar_t * command = GetCommandLine();
	  int32_t argc = 0;

	  proxy = new sirius::app::attendant::proxy();
	  LPWSTR * argv = ::CommandLineToArgvW(command, &argc);
	  sirius::app::attendant::proxy::parse_argument(argc, argv, proxy->context());
#endif
  }
#endif

  // Execute the secondary process, if any.
  int exit_code = CefExecuteProcess(main_args, app, sandbox_info);
  if (exit_code >= 0)
    return exit_code;

  // Create the main context object.
  scoped_ptr<MainContextImpl> context(new MainContextImpl(command_line, true));

  CefSettings settings;

#if !defined(CEF_USE_SANDBOX)
  settings.no_sandbox = true;
#endif

  // Populate the settings based on command line arguments.
  context->PopulateSettings(&settings);

  // Create the main message loop object.
  scoped_ptr<MainMessageLoop> message_loop;
  if (settings.multi_threaded_message_loop)
    message_loop.reset(new MainMessageLoopMultithreadedWin);
  else
    message_loop.reset(new MainMessageLoopStd);

  // Initialize CEF.
  context->Initialize(main_args, settings, app, sandbox_info);

  // Register scheme handlers.
  test_runner::RegisterSchemeHandlers();

#ifdef WITH_ATTENDANT_PROXY
  bool present = true;
  if (command_line->HasSwitch("enable_present"))
  {
	  present = command_line->GetSwitchValue("enable_present").compare("true") == 0 ? true : false;
  }

#endif

#if defined(WITH_JAVASCRIPT)
  context->GetRootWindowManager()->CreateRootWindow(
	  false,  // Show controls.
	  settings.windowless_rendering_enabled ? true : false,
	  present,
	  proxy_handle,
	  CefRect(0, 0, 1282, 722),       // Use default system size.
	  std::string());   // Use default URL.
#else
  if (proxy)
  {
	  context->GetRootWindowManager()->CreateRootWindow(
		  false,  // Show controls.
		  settings.windowless_rendering_enabled ? true : false,
		  present,
		  &proxy->context()->hwnd,
		  CefRect(0, 0, 1282, 722),       // Use default system size.
		  std::string());   // Use default URL.

	  proxy->initialize();
	  if (proxy->is_initialized())
	  {
		  if (proxy->context()->play_after_connect)
		  {
			  proxy->connect();
		  }
		  else
		  {
			  proxy->play();
		  }
	  }
  }
  else
  {
	  // Create the first window.
	  context->GetRootWindowManager()->CreateRootWindow(
		  false,  // Show controls.
		  settings.windowless_rendering_enabled ? true : false,
		  present,
		  &proxy->context()->hwnd,
		  CefRect(0, 0, 1282, 722),       // Use default system size.
		  std::string());   // Use default URL.
  }
#endif



  // Run the message loop. This will block until Quit() is called by the
  // RootWindowManager after all windows have been destroyed.
  int result = message_loop->Run();
#if defined(WITH_JAVASCRIPT)
  client::binding::attendent_proxy_wrapper& apc = client::binding::attendent_proxy_wrapper::getInstance();
  apc.finalize();
#endif
#ifdef WITH_ATTENDANT_PROXY
#if defined(WITH_JAVASCRIPT)
#else
  if (proxy)
  {
	  if (proxy->is_initialized())
	  {
		  if (proxy->context()->play_after_connect)
		  {
			  proxy->stop();
			  proxy->disconnect();
		  }
		  else
		  {
			  proxy->stop();
		  }
		  proxy->release();
	  }
	  delete proxy;
	  proxy = nullptr;
  }
#endif
#endif
  // Shut down CEF.
  context->Shutdown();

  // Release objects in reverse order of creation.
  message_loop.reset();
  context.reset();

  return result;
}

}  // namespace
}  // namespace client

// Program entry point function.
int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPTSTR    lpCmdLine,
                      int       nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);
  return client::RunMain(hInstance, nCmdShow);
}
