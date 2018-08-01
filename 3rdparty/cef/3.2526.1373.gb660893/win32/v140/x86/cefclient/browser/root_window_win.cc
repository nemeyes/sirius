// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include <fstream>
#include "cefclient/browser/root_window_win.h"

#include "include/base/cef_bind.h"
#include "include/base/cef_build.h"
#include "include/cef_app.h"
#include "cefclient/browser/browser_window_osr_win.h"
#include "cefclient/browser/browser_window_std_win.h"
#include "cefclient/browser/geometry_util.h"
#include "cefclient/browser/main_context.h"
#include "cefclient/browser/main_message_loop.h"
#include "cefclient/browser/resource.h"
#include "cefclient/browser/temp_window.h"
#include "cefclient/browser/util_win.h"
#include "cefclient/common/client_switches.h"

#if defined(WITH_EXTERNAL_INTERFACE)
#include "cefclient/binding/global.h"
#include "cefclient/binding/browser.h"
#include "cefclient/binding/socket_win.h"
#endif
#include <shlwapi.h>
#include <tchar.h>
#include <process.h>
#include <mmSystem.h>
#include <wininet.h>
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Wininet.lib")

#define MAX_URL_LENGTH  255
#define BUTTON_WIDTH    72
#define URLBAR_HEIGHT   24
#define TARGET_RESOLUTION				1

#ifdef WITH_ATTENDANT_PROXY
#define JS_INJECTION_PATH "\\js_injection\\injection.js"

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#endif

#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))
#endif
#endif
bool client::RootWindowWin::load_finish;
namespace client {

namespace {

// Message handler for the About box.
INT_PTR CALLBACK AboutWndProc(HWND hDlg, UINT message,
                              WPARAM wParam, LPARAM lParam) {
  UNREFERENCED_PARAMETER(lParam);
  switch (message) {
    case WM_INITDIALOG:
      return TRUE;

    case WM_COMMAND:
      if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
        EndDialog(hDlg, LOWORD(wParam));
        return TRUE;
      }
      break;
  }
  return FALSE;
}

int GetButtonWidth() {
  static int button_width = BUTTON_WIDTH;
  static bool initialized = false;

  if (!initialized) {
    button_width = LogicalToDevice(BUTTON_WIDTH, GetDeviceScaleFactor());
    initialized = true;
  }

  return button_width;
}

int GetURLBarHeight() {
  static int urlbar_height = URLBAR_HEIGHT;
  static bool initialized = false;

  if (!initialized) {
    urlbar_height = LogicalToDevice(URLBAR_HEIGHT, GetDeviceScaleFactor());
    initialized = true;
  }

  return urlbar_height;
}

}  // namespace

RootWindowWin::RootWindowWin()
    : delegate_(NULL),
      with_controls_(false),
      with_osr_(false),
      is_popup_(false),
      start_rect_(),
      initialized_(false),
      hwnd_(NULL),
      draggable_region_(NULL),
      font_(NULL),
      back_hwnd_(NULL),
      forward_hwnd_(NULL),
      reload_hwnd_(NULL),
      stop_hwnd_(NULL),
      edit_hwnd_(NULL),
      edit_wndproc_old_(NULL),
      find_hwnd_(NULL),
      find_message_id_(0),
      find_wndproc_old_(NULL),
	  outter_hwnd_(NULL),
      find_state_(),
      find_next_(false),
      find_match_case_last_(false),
      window_destroyed_(false),
      browser_destroyed_(false),
	  stat_timer(false)
{
  find_buff_[0] = 0;
  load_finish = false;
  // Create a HRGN representing the draggable window area.
  draggable_region_ = ::CreateRectRgn(0, 0, 0, 0);
#ifdef WITH_ATTENDANT_PROXY
  device_scale_factor_ = client::GetDeviceScaleFactor();
#endif

}

RootWindowWin::~RootWindowWin() {
  REQUIRE_MAIN_THREAD();

  ::DeleteObject(draggable_region_);
  ::DeleteObject(font_);

  // The window and browser should already have been destroyed.
  DCHECK(window_destroyed_);
  DCHECK(browser_destroyed_);
}

void RootWindowWin::Init(RootWindow::Delegate* delegate,
                         bool with_controls,
                         bool with_osr,
						 bool present,
						 void * hwnd,
                         const CefRect& bounds,
                         const CefBrowserSettings& settings,
                         const std::string& url) {
  DCHECK(delegate);
  DCHECK(!initialized_);

  delegate_ = delegate;
  with_controls_ = with_controls;
  with_osr_ = with_osr;
  present_ = present;
  if (hwnd)
	  outter_hwnd_ = (HWND*)hwnd;
  else
	  outter_hwnd_ = nullptr;

  start_rect_.left = bounds.x;
  start_rect_.top = bounds.y;
  start_rect_.right = bounds.x + bounds.width;
  start_rect_.bottom = bounds.y + bounds.height;

  CreateBrowserWindow(url);
  start_url = url;
  initialized_ = true;

  // Create the native root window on the main thread.
  if (CURRENTLY_ON_MAIN_THREAD()) {
    CreateRootWindow(settings);

#if defined(WITH_EXTERNAL_INTERFACE)
	binding::global::get_instance().set_window_handle(GetWindowHandle());
	binding::browser::getInstance();
#endif
  } else {
    MAIN_POST_CLOSURE(
        base::Bind(&RootWindowWin::CreateRootWindow, this, settings));
  }
}

void RootWindowWin::InitAsPopup(RootWindow::Delegate* delegate,
                                bool with_controls,
                                bool with_osr,
                                const CefPopupFeatures& popupFeatures,
                                CefWindowInfo& windowInfo,
                                CefRefPtr<CefClient>& client,
                                CefBrowserSettings& settings) {
  DCHECK(delegate);
  DCHECK(!initialized_);

  delegate_ = delegate;
  with_controls_ = with_controls;
  with_osr_ = with_osr;
  is_popup_ = true;

  if (popupFeatures.xSet)
    start_rect_.left = popupFeatures.x;
  if (popupFeatures.ySet)
    start_rect_.top = popupFeatures.y;
  if (popupFeatures.widthSet)
    start_rect_.right = start_rect_.left + popupFeatures.width;
  if (popupFeatures.heightSet)
    start_rect_.bottom = start_rect_.top + popupFeatures.height;

  CreateBrowserWindow(std::string());

  initialized_ = true;

  // The new popup is initially parented to a temporary window. The native root
  // window will be created after the browser is created and the popup window
  // will be re-parented to it at that time.
  browser_window_->GetPopupConfig(TempWindow::GetWindowHandle(),
                                  windowInfo, client, settings);
}

void RootWindowWin::Show(ShowMode mode) {
  REQUIRE_MAIN_THREAD();

  if (!hwnd_)
    return;

  int nCmdShow = SW_SHOWNORMAL;
  switch (mode) {
    case ShowMinimized:
      nCmdShow = SW_SHOWMINIMIZED;
      break;
    case ShowMaximized:
      nCmdShow = SW_SHOWMAXIMIZED;
      break;
    default:
      break;
  }

  if (present_)
  {
	  ShowWindow(hwnd_, nCmdShow);
	  UpdateWindow(hwnd_);
  }
}

void RootWindowWin::Hide() {
  REQUIRE_MAIN_THREAD();

  if (hwnd_)
    ShowWindow(hwnd_, SW_HIDE);
}

void RootWindowWin::SetBounds(int x, int y, size_t width, size_t height) {
  REQUIRE_MAIN_THREAD();

  if (hwnd_) {
    SetWindowPos(hwnd_, NULL,
                 x, y, static_cast<int>(width), static_cast<int>(height),
                 SWP_NOZORDER);
  }
}

void RootWindowWin::Close(bool force) {
  REQUIRE_MAIN_THREAD();

  if (hwnd_) {
    if (force)
      DestroyWindow(hwnd_);
    else
      PostMessage(hwnd_, WM_CLOSE, 0, 0);
  }
}

void RootWindowWin::SetDeviceScaleFactor(float device_scale_factor) {
  REQUIRE_MAIN_THREAD();

  if (browser_window_)
    browser_window_->SetDeviceScaleFactor(device_scale_factor);
}

float RootWindowWin::GetDeviceScaleFactor() const {
  REQUIRE_MAIN_THREAD();

  if (browser_window_)
    return browser_window_->GetDeviceScaleFactor();
  return client::GetDeviceScaleFactor();
}

CefRefPtr<CefBrowser> RootWindowWin::GetBrowser() const {
  REQUIRE_MAIN_THREAD();

  if (browser_window_)
    return browser_window_->GetBrowser();
  return NULL;
}

ClientWindowHandle RootWindowWin::GetWindowHandle() const {
  REQUIRE_MAIN_THREAD();
  return hwnd_;
}

int32_t RootWindowWin::first_reload()
{
	if (stat_timer == NULL)
	{
		OutputDebugStringA("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!first_reload");
		TIMECAPS _timecaps;
		if (timeGetDevCaps(&_timecaps, sizeof(TIMECAPS)) != TIMERR_NOERROR)
			return -1;

		/*uint32_t _timer_res = min(max(_timecaps.wPeriodMin, TARGET_RESOLUTION), _timecaps.wPeriodMax);

		if (timeBeginPeriod(_timer_res) != TIMERR_NOERROR)
			return -1;*/

		timeset_event = timeSetEvent(1000, 0, &timer_reload, (DWORD_PTR)this, TIME_PERIODIC | TIME_KILL_SYNCHRONOUS);
		stat_timer = true;
	}
	return int32_t();
}

void RootWindowWin::CreateBrowserWindow(const std::string& startup_url) {
  if (with_osr_) {
    OsrRenderer::Settings settings;
    MainContext::Get()->PopulateOsrSettings(&settings);
    browser_window_.reset(new BrowserWindowOsrWin(this, startup_url, settings));
  } else {
    browser_window_.reset(new BrowserWindowStdWin(this, startup_url));
  }
}

void RootWindowWin::CreateRootWindow(const CefBrowserSettings& settings) {
  REQUIRE_MAIN_THREAD();
  DCHECK(!hwnd_);

  HINSTANCE hInstance = GetModuleHandle(NULL);

  // Load strings from the resource file.
  const std::wstring& window_title = GetResourceString(IDS_APP_TITLE);
  const std::wstring& window_class = GetResourceString(IDC_CEFCLIENT);

  const cef_color_t background_color = MainContext::Get()->GetBackgroundColor();
  const HBRUSH background_brush = CreateSolidBrush(
      RGB(CefColorGetR(background_color),
          CefColorGetG(background_color),
          CefColorGetB(background_color)));

  // Register the window class.
  RegisterRootClass(hInstance, window_class, background_brush);

  // Register the message used with the find dialog.
  find_message_id_ = RegisterWindowMessage(FINDMSGSTRING);
  CHECK(find_message_id_);

  const DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;

  int x, y, width, height;
  if (::IsRectEmpty(&start_rect_)) {
    // Use the default window position/size.
    x = y = width = height = CW_USEDEFAULT;
  } else {
    // Adjust the window size to account for window frame and controls.
    RECT window_rect = start_rect_;
    ::AdjustWindowRectEx(&window_rect, dwStyle, with_controls_, 0);
    if (with_controls_)
      window_rect.bottom += GetURLBarHeight();

    x = start_rect_.left;
    y = start_rect_.top;
    width = window_rect.right - window_rect.left;
    height = window_rect.bottom - window_rect.top;
  }

  // Create the main window initially hidden.
  hwnd_ = CreateWindow(window_class.c_str(), window_title.c_str(),
                       dwStyle,
                       x, y, width, height,
                       NULL, NULL, hInstance, NULL);
  CHECK(hwnd_);

  if (outter_hwnd_)
  {
	  (*outter_hwnd_) = hwnd_;
  }

  // Associate |this| with the main window.
  SetUserDataPtr(hwnd_, this);

  RECT rect;
  GetClientRect(hwnd_, &rect);

  if (with_controls_) {
    // Create the child controls.
    int x_offset = 0;

    const int button_width = GetButtonWidth();
    const int urlbar_height = GetURLBarHeight();
    const int font_height =
        LogicalToDevice(14, client::GetDeviceScaleFactor());

    // Create a scaled font.
    font_ = ::CreateFont(
        -font_height, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial");

    back_hwnd_ = CreateWindow(
        L"BUTTON", L"Back",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_DISABLED,
        x_offset, 0, button_width, urlbar_height,
        hwnd_, reinterpret_cast<HMENU>(IDC_NAV_BACK), hInstance, 0);
    CHECK(back_hwnd_);
    SendMessage(back_hwnd_, WM_SETFONT, reinterpret_cast<WPARAM>(font_), TRUE);
    x_offset += button_width;

    forward_hwnd_ = CreateWindow(
        L"BUTTON", L"Forward",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_DISABLED,
        x_offset, 0, button_width, urlbar_height,
        hwnd_, reinterpret_cast<HMENU>(IDC_NAV_FORWARD), hInstance, 0);
    CHECK(forward_hwnd_);
    SendMessage(forward_hwnd_, WM_SETFONT,
                reinterpret_cast<WPARAM>(font_), TRUE);
    x_offset += button_width;

    reload_hwnd_ = CreateWindow(
        L"BUTTON", L"Reload",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON| WS_DISABLED,
        x_offset, 0, button_width, urlbar_height,
        hwnd_, reinterpret_cast<HMENU>(IDC_NAV_RELOAD), hInstance, 0);
    CHECK(reload_hwnd_);
    SendMessage(reload_hwnd_, WM_SETFONT,
                reinterpret_cast<WPARAM>(font_), TRUE);
    x_offset += button_width;

    stop_hwnd_ = CreateWindow(
        L"BUTTON", L"Stop",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_DISABLED,
        x_offset, 0, button_width, urlbar_height,
        hwnd_, reinterpret_cast<HMENU>(IDC_NAV_STOP), hInstance, 0);
    CHECK(stop_hwnd_);
    SendMessage(stop_hwnd_, WM_SETFONT, reinterpret_cast<WPARAM>(font_), TRUE);
    x_offset += button_width;

    edit_hwnd_ = CreateWindow(
        L"EDIT", 0,
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOVSCROLL |
        ES_AUTOHSCROLL| WS_DISABLED,
        x_offset, 0, rect.right - button_width * 4, urlbar_height,
        hwnd_, 0, hInstance, 0);
    SendMessage(edit_hwnd_, WM_SETFONT, reinterpret_cast<WPARAM>(font_), TRUE);
    CHECK(edit_hwnd_);

    // Override the edit control's window procedure.
    edit_wndproc_old_ = SetWndProcPtr(edit_hwnd_, EditWndProc);

    // Associate |this| with the edit window.
    SetUserDataPtr(edit_hwnd_, this);

    rect.top += urlbar_height;

    if (!with_osr_) {
      // Remove the menu items that are only used with OSR.
      HMENU hMenu = ::GetMenu(hwnd_);
      if (hMenu) {
        HMENU hTestMenu = ::GetSubMenu(hMenu, 2);
        if (hTestMenu) {
          ::RemoveMenu(hTestMenu, ID_TESTS_OSR_FPS, MF_BYCOMMAND);
          ::RemoveMenu(hTestMenu, ID_TESTS_OSR_DSF, MF_BYCOMMAND);
        }
      }
    }
  } else {
    // No controls so also remove the default menu.
    ::SetMenu(hwnd_, NULL);
  }

  if (!is_popup_) {
    // Create the browser window.
    CefRect cef_rect(rect.left, rect.top,
                     rect.right - rect.left,
                     rect.bottom - rect.top);
    browser_window_->CreateBrowser(hwnd_, cef_rect, settings,
                                   delegate_->GetRequestContext(this));
  } else {
    // With popups we already have a browser window. Parent the browser window
    // to the root window and show it in the correct location.
    browser_window_->ShowPopup(hwnd_,
                               rect.left, rect.top,
                               rect.right - rect.left,
                               rect.bottom - rect.top);
  }

  // Show this window.
  Show(ShowNormal);
}

#ifdef WITH_ATTENDANT_PROXY

void RootWindowWin::read_injection_js()
{
	char path[MAX_PATH] = { 0 };
	if (GetModuleFileNameA(GetModuleHandleA(NULL), path, MAX_PATH) == 0) {
		OutputDebugStringA("error read_injection_js\n");
		return;
	}
	*(strrchr(path, '\\') + 1) = 0;

	CefString path_str = path;

	std::string injection_path = path_str.ToString() + JS_INJECTION_PATH;
	std::ifstream file(injection_path);
	std::stringstream buffer;
	buffer << file.rdbuf();
	javascript_injection_ = buffer.str();
	binding::global::get_instance().set_java_script_injection(javascript_injection_);
	OutputDebugStringA("read_injection_js OK\n");

}
#endif

// static
void RootWindowWin::RegisterRootClass(HINSTANCE hInstance,
                                      const std::wstring& window_class,
                                      HBRUSH background_brush) {
  // Only register the class one time.
  static bool class_registered = false;
  if (class_registered)
    return;
  class_registered = true;

  WNDCLASSEX wcex;

  wcex.cbSize = sizeof(WNDCLASSEX);

  wcex.style         = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc   = RootWndProc;
  wcex.cbClsExtra    = 0;
  wcex.cbWndExtra    = 0;
  wcex.hInstance     = hInstance;
  wcex.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CEFCLIENT));
  wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = background_brush;
  wcex.lpszMenuName  = MAKEINTRESOURCE(IDC_CEFCLIENT);
  wcex.lpszClassName = window_class.c_str();
  wcex.hIconSm       = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

  RegisterClassEx(&wcex);
}

// static
LRESULT CALLBACK RootWindowWin::EditWndProc(HWND hWnd, UINT message,
                                            WPARAM wParam, LPARAM lParam) {
  REQUIRE_MAIN_THREAD();

  RootWindowWin* self = GetUserDataPtr<RootWindowWin*>(hWnd);
  DCHECK(self);
  DCHECK(hWnd == self->edit_hwnd_);

  switch (message) {
    case WM_CHAR:
      if (wParam == VK_RETURN) {
        // When the user hits the enter key load the URL.
        CefRefPtr<CefBrowser> browser = self->GetBrowser();
        if (browser) {
          wchar_t strPtr[MAX_URL_LENGTH+1] = {0};
          *((LPWORD)strPtr) = MAX_URL_LENGTH;
          LRESULT strLen = SendMessage(hWnd, EM_GETLINE, 0, (LPARAM)strPtr);
          if (strLen > 0) {
            strPtr[strLen] = 0;
            browser->GetMainFrame()->LoadURL(strPtr);
          }
        }
        return 0;
      }
      break;
    case WM_NCDESTROY:
      // Clear the reference to |self|.
      SetUserDataPtr(hWnd, NULL);
      self->edit_hwnd_ = NULL;
      break;
  }

  return CallWindowProc(self->edit_wndproc_old_, hWnd, message, wParam, lParam);
}

// static
LRESULT CALLBACK RootWindowWin::FindWndProc(HWND hWnd, UINT message,
                                            WPARAM wParam, LPARAM lParam) {
  REQUIRE_MAIN_THREAD();

  RootWindowWin* self = GetUserDataPtr<RootWindowWin*>(hWnd);
  DCHECK(self);
  DCHECK(hWnd == self->find_hwnd_);

  switch (message) {
    case WM_ACTIVATE:
      // Set this dialog as current when activated.
      MainMessageLoop::Get()->SetCurrentModelessDialog(
          wParam == 0 ? NULL : hWnd);
      return FALSE;
    case WM_NCDESTROY:
      // Clear the reference to |self|.
      SetUserDataPtr(hWnd, NULL);
      self->find_hwnd_ = NULL;
      break;
  }

  return CallWindowProc(self->find_wndproc_old_, hWnd, message, wParam, lParam);
}

// static
LRESULT CALLBACK RootWindowWin::RootWndProc(HWND hWnd, UINT message,
                                            WPARAM wParam, LPARAM lParam) {
  REQUIRE_MAIN_THREAD();

  RootWindowWin* self = GetUserDataPtr<RootWindowWin*>(hWnd);
  if (!self)
    return DefWindowProc(hWnd, message, wParam, lParam);
  DCHECK(hWnd == self->hwnd_);

  if (message == self->find_message_id_) {
    // Message targeting the find dialog.
    LPFINDREPLACE lpfr = reinterpret_cast<LPFINDREPLACE>(lParam);
    CHECK(lpfr == &self->find_state_);
    self->OnFindEvent();
    return 0;
  }

  // Callback for the main window
  switch (message) {
    case WM_COMMAND:
      if (self->OnCommand(LOWORD(wParam)))
        return 0;
      break;

    case WM_PAINT:
      self->OnPaint();
      return 0;

    case WM_SETFOCUS:
      self->OnFocus();
      return 0;

    case WM_SIZE:
      self->OnSize(wParam == SIZE_MINIMIZED);
      break;

    case WM_MOVING:
    case WM_MOVE:
      self->OnMove();
      return 0;

    case WM_ERASEBKGND:
      if (self->OnEraseBkgnd())
        break;
      // Don't erase the background.
      return 0;

    case WM_ENTERMENULOOP:
      if (!wParam) {
        // Entering the menu loop for the application menu.
        CefSetOSModalLoop(true);
      }
      break;

    case WM_EXITMENULOOP:
      if (!wParam) {
        // Exiting the menu loop for the application menu.
        CefSetOSModalLoop(false);
      }
      break;

    case WM_CLOSE:
      if (self->OnClose())
        return 0;  // Cancel the close.
      break;

    case WM_NCHITTEST: {
      LRESULT hit = DefWindowProc(hWnd, message, wParam, lParam);
      if (hit == HTCLIENT) {
        POINTS points = MAKEPOINTS(lParam);
        POINT point = { points.x, points.y };
        ::ScreenToClient(hWnd, &point);
        if (::PtInRegion(self->draggable_region_, point.x, point.y)) {
          // If cursor is inside a draggable region return HTCAPTION to allow
          // dragging.
          return HTCAPTION;
        }
      }
      return hit;
    }

    case WM_NCDESTROY:
      // Clear the reference to |self|.
      SetUserDataPtr(hWnd, NULL);
      self->hwnd_ = NULL;
	  self->outter_hwnd_ = NULL;
      self->OnDestroyed();
      return 0;

#ifdef WITH_ATTENDANT_PROXY
	case WM_SYSCHAR:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_CHAR:
		self->OnKeyEvent(message, wParam, lParam);
		break;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_MOUSELEAVE:
	case WM_MOUSEWHEEL:
		self->OnMouseEvent(message, wParam, lParam);
		break;
#endif
  }

  return DefWindowProc(hWnd, message, wParam, lParam);
}

void RootWindowWin::OnPaint() {
  PAINTSTRUCT ps;
  BeginPaint(hwnd_, &ps);
  EndPaint(hwnd_, &ps);
}

void RootWindowWin::OnFocus() {
  if (browser_window_)
    browser_window_->SetFocus(true);
}

void RootWindowWin::OnSize(bool minimized) {
  if (minimized) {
    // Notify the browser window that it was hidden and do nothing further.
    if (browser_window_)
      browser_window_->Hide();
    return;
  }

  if (browser_window_)
    browser_window_->Show();

  RECT rect;
  GetClientRect(hwnd_, &rect);

  if (with_controls_) {
    static int button_width = GetButtonWidth();
    static int urlbar_height = GetURLBarHeight();

    // Resize the window and address bar to match the new frame size.
    rect.top += urlbar_height;

    int urloffset = rect.left + button_width * 4;

    // |browser_hwnd| may be NULL if the browser has not yet been created.
    HWND browser_hwnd = NULL;
    if (browser_window_)
      browser_hwnd = browser_window_->GetWindowHandle();

    if (browser_hwnd) {
      // Resize both the browser and the URL edit field.
      HDWP hdwp = BeginDeferWindowPos(2);
      hdwp = DeferWindowPos(hdwp, edit_hwnd_, NULL, urloffset,
          0, rect.right - urloffset, urlbar_height, SWP_NOZORDER);
      hdwp = DeferWindowPos(hdwp, browser_hwnd, NULL,
          rect.left, rect.top, rect.right - rect.left,
          rect.bottom - rect.top, SWP_NOZORDER);
      BOOL result = EndDeferWindowPos(hdwp);
      ALLOW_UNUSED_LOCAL(result);
      DCHECK(result);
    } else {
      // Resize just the URL edit field.
      SetWindowPos(edit_hwnd_, NULL, urloffset,
          0, rect.right - urloffset, urlbar_height, SWP_NOZORDER);
    }
  } else if (browser_window_) {
    // Size the browser window to the whole client area.
    browser_window_->SetBounds(0, 0, rect.right, rect.bottom);
  }
}

void RootWindowWin::OnMove() {
  // Notify the browser of move events so that popup windows are displayed
  // in the correct location and dismissed when the window moves.
  CefRefPtr<CefBrowser> browser = GetBrowser();
  if (browser)
    browser->GetHost()->NotifyMoveOrResizeStarted();
}

bool RootWindowWin::OnEraseBkgnd() {
  // Erase the background when the browser does not exist.
  return (GetBrowser() == NULL);
}

bool RootWindowWin::OnCommand(UINT id) {
  if (id >= ID_TESTS_FIRST && id <= ID_TESTS_LAST) {
    delegate_->OnTest(this, id);
    return true;
  }

  switch (id) {
    case IDM_ABOUT:
      OnAbout();
      return true;
    case IDM_EXIT:
      delegate_->OnExit(this);
      return true;
    case ID_FIND:
      OnFind();
      return true;
    case IDC_NAV_BACK:   // Back button
      if (CefRefPtr<CefBrowser> browser = GetBrowser())
        browser->GoBack();
      return true;
    case IDC_NAV_FORWARD:  // Forward button
      if (CefRefPtr<CefBrowser> browser = GetBrowser())
        browser->GoForward();
      return true;
    case IDC_NAV_RELOAD:  // Reload button
      if (CefRefPtr<CefBrowser> browser = GetBrowser())
        browser->Reload();
      return true;
    case IDC_NAV_STOP:  // Stop button
      if (CefRefPtr<CefBrowser> browser = GetBrowser())
        browser->StopLoad();
      return true;
  }

  return false;
}

void RootWindowWin::OnFind() {
  if (find_hwnd_) {
    // Give focus to the existing find dialog.
    ::SetFocus(find_hwnd_);
    return;
  }

  // Configure dialog state.
  ZeroMemory(&find_state_, sizeof(find_state_));
  find_state_.lStructSize = sizeof(find_state_);
  find_state_.hwndOwner = hwnd_;
  find_state_.lpstrFindWhat = find_buff_;
  find_state_.wFindWhatLen = sizeof(find_buff_);
  find_state_.Flags = FR_HIDEWHOLEWORD | FR_DOWN;

  // Create the dialog.
  find_hwnd_ = FindText(&find_state_);

  // Override the dialog's window procedure.
  find_wndproc_old_ = SetWndProcPtr(find_hwnd_, FindWndProc);

  // Associate |self| with the dialog.
  SetUserDataPtr(find_hwnd_, this);
}

void RootWindowWin::OnFindEvent() {
  CefRefPtr<CefBrowser> browser = GetBrowser();

  if (find_state_.Flags & FR_DIALOGTERM) {
    // The find dialog box has been dismissed so invalidate the handle and
    // reset the search results.
    if (browser) {
      browser->GetHost()->StopFinding(true);
      find_what_last_.clear();
      find_next_ = false;
    }
  } else if ((find_state_.Flags & FR_FINDNEXT) && browser)  {
    // Search for the requested string.
    bool match_case = ((find_state_.Flags & FR_MATCHCASE) ? true : false);
    const std::wstring& find_what = find_buff_;
    if (match_case != find_match_case_last_ || find_what != find_what_last_) {
      // The search string has changed, so reset the search results.
      if (!find_what.empty()) {
        browser->GetHost()->StopFinding(true);
        find_next_ = false;
      }
      find_match_case_last_ = match_case;
      find_what_last_ = find_buff_;
    }

    browser->GetHost()->Find(0, find_what,
                             (find_state_.Flags & FR_DOWN) ? true : false,
                             match_case, find_next_);
    if (!find_next_)
      find_next_ = true;
  }
}

void RootWindowWin::OnAbout() {
  // Show the about box.
  DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd_,
            AboutWndProc);
}

bool RootWindowWin::OnClose() {
  if (browser_window_ && !browser_window_->IsClosing()) {
    CefRefPtr<CefBrowser> browser = GetBrowser();
    if (browser) {
      // Notify the browser window that we would like to close it. This
      // will result in a call to ClientHandler::DoClose() if the
      // JavaScript 'onbeforeunload' event handler allows it.
      browser->GetHost()->CloseBrowser(false);

      // Cancel the close.
      return true;
    }
  }

  // Allow the close.
  return false;
}

void RootWindowWin::OnDestroyed() {
  window_destroyed_ = true;
  NotifyDestroyedIfDone();
}

#ifdef WITH_ATTENDANT_PROXY
void RootWindowWin::OnKeyEvent(UINT message, WPARAM wParam, LPARAM lParam)
{
	OutputDebugStringA("========================OnKeyEvent========================\n");

	CefKeyEvent event;
	event.windows_key_code = wParam;
	event.native_key_code = lParam;
	event.is_system_key = message == WM_SYSCHAR ||
		message == WM_SYSKEYDOWN ||
		message == WM_SYSKEYUP;

	if (message == WM_KEYDOWN || message == WM_SYSKEYDOWN)
		event.type = KEYEVENT_RAWKEYDOWN;
	else if (message == WM_KEYUP || message == WM_SYSKEYUP)
		event.type = KEYEVENT_KEYUP;
	else
		event.type = KEYEVENT_CHAR;
	event.modifiers = GetCefKeyboardModifiers(wParam, lParam);
	CefRefPtr<CefBrowser> browser_ = this->GetBrowser();
	if (browser_)
		browser_->GetHost()->SendKeyEvent(event);
}

void RootWindowWin::OnMouseEvent(UINT message, WPARAM wParam, LPARAM lParam)
{
	CefRefPtr<CefBrowserHost> browser_host;
	if (browser_window_ == NULL || browser_window_->IsClosing())
		return;

	if (browser_window_->GetBrowser() == NULL)
		return;
	browser_host = browser_window_->GetBrowser()->GetHost();

	char log[MAX_PATH] = { 0 };

	LONG currentTime = 0;
	bool cancelPreviousClick = false;
	int x = GET_X_LPARAM(lParam);
	int y = GET_Y_LPARAM(lParam);

	if (message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN ||
		message == WM_MBUTTONDOWN || message == WM_MOUSEMOVE ||
		message == WM_MOUSELEAVE) {
		currentTime = GetMessageTime();
		cancelPreviousClick =
			(abs(last_click_x_ - x) > (GetSystemMetrics(SM_CXDOUBLECLK) / 2))
			|| (abs(last_click_y_ - y) > (GetSystemMetrics(SM_CYDOUBLECLK) / 2))
			|| ((currentTime - last_click_time_) > GetDoubleClickTime());

		if (cancelPreviousClick &&
			(message == WM_MOUSEMOVE || message == WM_MOUSELEAVE)) {
			last_click_count_ = 0;
			last_click_x_ = 0;
			last_click_y_ = 0;
			last_click_time_ = 0;
		}
	}

	switch (message) {
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN: {
		//		::SetCapture(hwnd_);
		//		::SetFocus(hwnd_);
		if (wParam & MK_SHIFT) {
			// Start rotation effect.
			last_mouse_pos_.x = current_mouse_pos_.x = x;
			last_mouse_pos_.y = current_mouse_pos_.y = y;
			mouse_rotation_ = true;
		}
		else {
			CefBrowserHost::MouseButtonType btnType =
				(message == WM_LBUTTONDOWN ? MBT_LEFT : (
					message == WM_RBUTTONDOWN ? MBT_RIGHT : MBT_MIDDLE));
			if (!cancelPreviousClick && (btnType == last_click_button_)) {
				++last_click_count_;
			}
			else {
				last_click_count_ = 1;
				last_click_x_ = x;
				last_click_y_ = y;
			}
			last_click_time_ = currentTime;
			last_click_button_ = btnType;

			if (browser_host) {
				CefMouseEvent mouse_event;
				mouse_event.x = x;
				mouse_event.y = y;
				//last_mouse_down_on_view_ = !IsOverPopupWidget(x, y);
				//	ApplyPopupOffset(mouse_event.x, mouse_event.y);
				DeviceToLogical(mouse_event, device_scale_factor_);
				mouse_event.modifiers = GetCefMouseModifiers(wParam);
				browser_host->SendMouseClickEvent(mouse_event, btnType, false, last_click_count_);
			}

		}
	}
						 break;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		//	if (GetCapture() == hwnd_)
		//		ReleaseCapture();
		if (mouse_rotation_) {
			// End rotation effect.
			mouse_rotation_ = false;
			//renderer_.SetSpin(0, 0);
			//Invalidate();
		}
		else {
			CefBrowserHost::MouseButtonType btnType =
				(message == WM_LBUTTONUP ? MBT_LEFT : (
					message == WM_RBUTTONUP ? MBT_RIGHT : MBT_MIDDLE));
			if (browser_host) {
				CefMouseEvent mouse_event;
				mouse_event.x = x;
				mouse_event.y = y;
				//if (last_mouse_down_on_view_ &&
				//	IsOverPopupWidget(x, y) &&
				//	(GetPopupXOffset() || GetPopupYOffset())) {
				//	break;
				//}
				//ApplyPopupOffset(mouse_event.x, mouse_event.y);
				DeviceToLogical(mouse_event, device_scale_factor_);
				mouse_event.modifiers = GetCefMouseModifiers(wParam);
				browser_host->SendMouseClickEvent(mouse_event, btnType, true, last_click_count_);
			}
		}
		break;

	case WM_MOUSEMOVE: {
		if (mouse_rotation_) {
			//// Apply rotation effect.
			//current_mouse_pos_.x = x;
			//current_mouse_pos_.y = y;
			////renderer_.IncrementSpin(
			////	current_mouse_pos_.x - last_mouse_pos_.x,
			////	current_mouse_pos_.y - last_mouse_pos_.y);
			//last_mouse_pos_.x = current_mouse_pos_.x;
			//last_mouse_pos_.y = current_mouse_pos_.y;
			////Invalidate();
		}
		else {
			if (!mouse_tracking_) {
				// Start tracking mouse leave. Required for the WM_MOUSELEAVE event to
				// be generated.
				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(TRACKMOUSEEVENT);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = hwnd_;
				TrackMouseEvent(&tme);
				mouse_tracking_ = true;
			}

			if (browser_host) {
				CefMouseEvent mouse_event;

				mouse_event.x = x;
				mouse_event.y = y;
				//  ApplyPopupOffset(mouse_event.x, mouse_event.y);
				DeviceToLogical(mouse_event, device_scale_factor_);
				mouse_event.modifiers = GetCefMouseModifiers(wParam);
				browser_host->SendMouseMoveEvent(mouse_event, false);
			}
		}
	}
					   break;

	case WM_MOUSELEAVE: {
		if (mouse_tracking_) {
			// Stop tracking mouse leave.
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_LEAVE & TME_CANCEL;
			tme.hwndTrack = hwnd_;
			TrackMouseEvent(&tme);
			mouse_tracking_ = false;

		}

		if (browser_host) {
			//	// Determine the cursor position in screen coordinates.
			//	POINT p;
			//	::GetCursorPos(&p);
			//	::ScreenToClient(hwnd_, &p);

			//				CefMouseEvent mouse_event;
			//				mouse_event.x = p.x;
			//				mouse_event.y = p.y;
			//				DeviceToLogical(mouse_event, device_scale_factor_);
			//				mouse_event.modifiers = GetCefMouseModifiers(wParam);
			//				browser_host->SendMouseMoveEvent(mouse_event, true);
		}
	} break;
	}
}
static int32_t cnt = 0;
void RootWindowWin::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
	char debug[MAX_PATH] = { 0 };
	_snprintf(debug, MAX_PATH, "httpStatusCode:%d\n", httpStatusCode);
	OutputDebugStringA(debug);
	if (cnt == 0)
	{
		first_reload();
		cnt++;
	}
	else
	{
		load_finish = true;
		_snprintf(debug, MAX_PATH, "RootWindowWin::OnLoadEnd->load_finish:%d\n", load_finish);
		OutputDebugStringA(debug);
	}
}
#endif

void RootWindowWin::OnBrowserCreated(CefRefPtr<CefBrowser> browser) 
{
  REQUIRE_MAIN_THREAD();

  if (is_popup_) {
    // For popup browsers create the root window once the browser has been
    // created.
    CreateRootWindow(CefBrowserSettings());
  } else {
    // Make sure the browser is sized correctly.
    OnSize(false);
  }
#if defined(WITH_ATTENDANT_PROXY)
  read_injection_js();
  browser->GetMainFrame()->ExecuteJavaScript(javascript_injection_, "about:blank", 0);
#endif
}

void RootWindowWin::OnBrowserWindowDestroyed() {
  REQUIRE_MAIN_THREAD();

  browser_window_.reset();

  if (!window_destroyed_) {
    // The browser was destroyed first. This could be due to the use of
    // off-screen rendering or execution of JavaScript window.close().
    // Close the RootWindow.
    Close(true);
  }

  browser_destroyed_ = true;
  NotifyDestroyedIfDone();
}

void RootWindowWin::OnSetAddress(const std::string& url) {
  REQUIRE_MAIN_THREAD();

  if (edit_hwnd_)
    SetWindowText(edit_hwnd_, CefString(url).ToWString().c_str());
}

void RootWindowWin::OnSetTitle(const std::string& title) {
  REQUIRE_MAIN_THREAD();

  if (hwnd_)
    SetWindowText(hwnd_, CefString(title).ToWString().c_str());
}

void RootWindowWin::OnSetFullscreen(bool fullscreen) {
  REQUIRE_MAIN_THREAD();

  CefRefPtr<CefBrowser> browser = GetBrowser();
 /* if (browser) {
    if (fullscreen)
      window_test::Maximize(browser);
    else
      window_test::Restore(browser);
  }*/
}

void RootWindowWin::OnSetLoadingState(bool isLoading,
                                      bool canGoBack,
                                      bool canGoForward) {
  REQUIRE_MAIN_THREAD();

  //if (!javascript_injection_.empty() && isLoading)
  //{
	 // GetBrowser()->GetMainFrame()->ExecuteJavaScript(javascript_injection_, GetBrowser()->GetMainFrame()->GetURL(), 0);
	 // //::Sleep(50);
  //}

  if (with_controls_) {
    EnableWindow(back_hwnd_, canGoBack);
    EnableWindow(forward_hwnd_, canGoForward);
    EnableWindow(reload_hwnd_, !isLoading);
    EnableWindow(stop_hwnd_, isLoading);
    EnableWindow(edit_hwnd_, TRUE);
  }
}

namespace {

LPCWSTR kParentWndProc = L"CefParentWndProc";
LPCWSTR kDraggableRegion = L"CefDraggableRegion";

LRESULT CALLBACK SubclassedWindowProc(
    HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  WNDPROC hParentWndProc = reinterpret_cast<WNDPROC>(
      ::GetPropW(hWnd, kParentWndProc));
  HRGN hRegion = reinterpret_cast<HRGN>(
      ::GetPropW(hWnd, kDraggableRegion));

  if (message == WM_NCHITTEST) {
    LRESULT hit = CallWindowProc(
        hParentWndProc, hWnd, message, wParam, lParam);
    if (hit == HTCLIENT) {
      POINTS points = MAKEPOINTS(lParam);
      POINT point = { points.x, points.y };
      ::ScreenToClient(hWnd, &point);
      if (::PtInRegion(hRegion, point.x, point.y)) {
        // Let the parent window handle WM_NCHITTEST by returning HTTRANSPARENT
        // in child windows.
        return HTTRANSPARENT;
      }
    }
    return hit;
  }

  return CallWindowProc(hParentWndProc, hWnd, message, wParam, lParam);
}

void SubclassWindow(HWND hWnd, HRGN hRegion) {
  HANDLE hParentWndProc = ::GetPropW(hWnd, kParentWndProc);
  if (hParentWndProc) {
    return;
  }

  SetLastError(0);
  LONG_PTR hOldWndProc = SetWindowLongPtr(
      hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(SubclassedWindowProc));
  if (hOldWndProc == 0 && GetLastError() != ERROR_SUCCESS) {
    return;
  }

  ::SetPropW(hWnd, kParentWndProc, reinterpret_cast<HANDLE>(hOldWndProc));
  ::SetPropW(hWnd, kDraggableRegion, reinterpret_cast<HANDLE>(hRegion));
}

void UnSubclassWindow(HWND hWnd) {
  LONG_PTR hParentWndProc = reinterpret_cast<LONG_PTR>(
    ::GetPropW(hWnd, kParentWndProc));
  if (hParentWndProc) {
      LONG_PTR hPreviousWndProc =
          SetWindowLongPtr(hWnd, GWLP_WNDPROC, hParentWndProc);
      ALLOW_UNUSED_LOCAL(hPreviousWndProc);
      DCHECK_EQ(hPreviousWndProc,
                reinterpret_cast<LONG_PTR>(SubclassedWindowProc));
  }

  ::RemovePropW(hWnd, kParentWndProc);
  ::RemovePropW(hWnd, kDraggableRegion);
}

BOOL CALLBACK SubclassWindowsProc(HWND hwnd, LPARAM lParam) {
  SubclassWindow(hwnd, reinterpret_cast<HRGN>(lParam));
  return TRUE;
}

BOOL CALLBACK UnSubclassWindowsProc(HWND hwnd, LPARAM lParam) {
  UnSubclassWindow(hwnd);
  return TRUE;
}

}  // namespace

void RootWindowWin::OnSetDraggableRegions(
    const std::vector<CefDraggableRegion>& regions) {
  REQUIRE_MAIN_THREAD();

  // Reset draggable region.
  ::SetRectRgn(draggable_region_, 0, 0, 0, 0);

  // Determine new draggable region.
  std::vector<CefDraggableRegion>::const_iterator it = regions.begin();
  for (;it != regions.end(); ++it) {
    HRGN region = ::CreateRectRgn(
        it->bounds.x, it->bounds.y,
        it->bounds.x + it->bounds.width,
        it->bounds.y + it->bounds.height);
    ::CombineRgn(
        draggable_region_, draggable_region_, region,
        it->draggable ? RGN_OR : RGN_DIFF);
    ::DeleteObject(region);
  }

  // Subclass child window procedures in order to do hit-testing.
  // This will be a no-op, if it is already subclassed.
  if (hwnd_) {
    WNDENUMPROC proc = !regions.empty() ?
        SubclassWindowsProc : UnSubclassWindowsProc;
    ::EnumChildWindows(
        hwnd_, proc, reinterpret_cast<LPARAM>(draggable_region_));
  }
}

void RootWindowWin::NotifyDestroyedIfDone() {
  // Notify once both the window and the browser have been destroyed.
  if (window_destroyed_ && browser_destroyed_)
    delegate_->OnRootWindowDestroyed(this);
}

// static
scoped_refptr<RootWindow> RootWindow::Create() {
  return new RootWindowWin();
}

}  // namespace client

void CALLBACK timer_reload(uint32_t ui_id, uint32_t ui_msg, DWORD_PTR dw_user, DWORD_PTR dw1, DWORD_PTR dw2)
{
	OutputDebugStringA("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!timer_reload");
	client::binding::socket_win::get_instance()->sirius_to_javascript((uint8_t*)"reload", 5);
}
