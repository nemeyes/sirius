// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cefclient/browser/client_app_browser.h"

#include "include/base/cef_logging.h"
#include "include/cef_cookie.h"
#include "cefclient/common/client_switches.h"

namespace client {

ClientAppBrowser::ClientAppBrowser() {
  CreateDelegates(delegates_);
}

void ClientAppBrowser::OnBeforeCommandLineProcessing(
    const CefString& process_type,
    CefRefPtr<CefCommandLine> command_line) {
  // Pass additional command-line flags to the browser process.
  if (process_type.empty()) {
    // Pass additional command-line flags when off-screen rendering is enabled.
    if (command_line->HasSwitch(switches::kOffScreenRenderingEnabled)) {
      // If the PDF extension is enabled then cc Surfaces must be disabled for
      // PDFs to render correctly.
      // See https://bitbucket.org/chromiumembedded/cef/issues/1689 for details.
      if (!command_line->HasSwitch("disable-extensions") &&
          !command_line->HasSwitch("disable-pdf-extension")) {
        command_line->AppendSwitch("disable-surfaces");
      }

      // Use software rendering and compositing (disable GPU) for increased FPS
      // and decreased CPU usage. This will also disable WebGL so remove these
      // switches if you need that capability.
      // See https://bitbucket.org/chromiumembedded/cef/issues/1257 for details.
      if (!command_line->HasSwitch(switches::kEnableGPU)) {
        command_line->AppendSwitch("disable-gpu");
        command_line->AppendSwitch("disable-gpu-compositing");
      }

      // Synchronize the frame rate between all processes. This results in
      // decreased CPU usage by avoiding the generation of extra frames that
      // would otherwise be discarded. The frame rate can be set at browser
      // creation time via CefBrowserSettings.windowless_frame_rate or changed
      // dynamically using CefBrowserHost::SetWindowlessFrameRate. In cefclient
      // it can be set via the command-line using `--off-screen-frame-rate=XX`.
      // See https://bitbucket.org/chromiumembedded/cef/issues/1368 for details.
      command_line->AppendSwitch("enable-begin-frame-scheduling");
    }

    DelegateSet::iterator it = delegates_.begin();
    for (; it != delegates_.end(); ++it)
      (*it)->OnBeforeCommandLineProcessing(this, command_line);
  }
}

void ClientAppBrowser::OnContextInitialized() {
  // Register cookieable schemes with the global cookie manager.
  CefRefPtr<CefCookieManager> manager =
      CefCookieManager::GetGlobalManager(NULL);
  DCHECK(manager.get());
  manager->SetSupportedSchemes(cookieable_schemes_, NULL);

  print_handler_ = CreatePrintHandler();

  DelegateSet::iterator it = delegates_.begin();
  for (; it != delegates_.end(); ++it)
    (*it)->OnContextInitialized(this);
}

void ClientAppBrowser::OnBeforeChildProcessLaunch(
      CefRefPtr<CefCommandLine> command_line) {

//#ifdef WITH_ATTENDANT_PROXY
	CefString val = command_line->GetSwitchValue(CefString("type"));
	if (val.compare("gpu-process") == 0)
	{
		CefRefPtr<CefCommandLine> global_command_line =
			CefCommandLine::GetGlobalCommandLine();
		std::map<CefString, CefString> mswitches;
		global_command_line->GetSwitches(mswitches);
		for (std::map<CefString, CefString>::iterator it = mswitches.begin(); it != mswitches.end(); ++it)
		{
			if (!command_line->HasSwitch(it->first))
			{
				CefString sval = global_command_line->GetSwitchValue(it->first);
				if (!sval.empty())
					command_line->AppendSwitchWithValue(it->first, it->second);
				else
					command_line->AppendSwitch(it->first);
			}
		}
	}
//#endif

  DelegateSet::iterator it = delegates_.begin();
  for (; it != delegates_.end(); ++it)
    (*it)->OnBeforeChildProcessLaunch(this, command_line);
}

void ClientAppBrowser::OnRenderProcessThreadCreated(
    CefRefPtr<CefListValue> extra_info) {
  DelegateSet::iterator it = delegates_.begin();
  for (; it != delegates_.end(); ++it)
    (*it)->OnRenderProcessThreadCreated(this, extra_info);
}

}  // namespace client