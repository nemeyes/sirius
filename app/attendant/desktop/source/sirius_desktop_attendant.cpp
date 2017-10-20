#include <sirius_d3d11_desktop_capturer.h>
#include <limits.h>
#include <stdio.h>
#include <warning.h>
#include <d3d11.h>

#define OCCLUSION_STATUS_MSG WM_USER

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
bool ProcessCmdline(_Out_ INT* Output);
void ShowHelp();
void DisplayMsg(_In_ LPCWSTR Str, _In_ LPCWSTR Title, HRESULT hr);


class capture_handler
	: public sirius::library::video::source::d3d11::desktop::capturer::handler
{
public:
	capture_handler(void)
	{
	}

	~capture_handler(void)
	{
		
	}

	void on_initialize(void * device)
	{
		_device = (ID3D11Device*)device;

		WCHAR file_name[1000] = { 0 };
		_snwprintf_s(file_name, sizeof(file_name), L"saved.yuv");
		_file = ::CreateFile(file_name, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	void on_release(void)
	{
		::CloseHandle(_file);
	}

	void on_process(int32_t vst, int32_t vmt, void * video, uint32_t width, uint32_t height, uint32_t pitch)
	{
		if (vmt == sirius::library::video::source::d3d11::desktop::capturer::video_memory_type_t::host)
		{
			unsigned char * bytes = (unsigned char*)(video);
			if (vst == sirius::library::video::source::d3d11::desktop::capturer::video_submedia_type_t::rgb32)
			{
				if (_file != INVALID_HANDLE_VALUE)
				{
					uint32_t bytes2write = width*height * 4;
					uint32_t bytes_written = 0;
					do
					{
						uint32_t nb_write = 0;
						::WriteFile(_file, bytes, bytes2write, (LPDWORD)&nb_write, 0);
						bytes_written += nb_write;
						if (bytes2write == bytes_written)
							break;
					} while (1);
				}
			}
			else if (vst == sirius::library::video::source::d3d11::desktop::capturer::video_submedia_type_t::nv12)
			{
				if (_file != INVALID_HANDLE_VALUE)
				{
					uint32_t bytes2write = width*height*1.5;
					uint32_t bytes_written = 0;
					do
					{
						uint32_t nb_write = 0;
						::WriteFile(_file, bytes, bytes2write, (LPDWORD)&nb_write, 0);
						bytes_written += nb_write;
						if (bytes2write == bytes_written)
							break;
					} while (1);
				}
			}
		}
		else if (vmt == sirius::library::video::source::d3d11::desktop::capturer::video_memory_type_t::d3d11)
		{
			ID3D11Texture2D * surface = (ID3D11Texture2D*)(video);
			if (vst == sirius::library::video::source::d3d11::desktop::capturer::video_submedia_type_t::rgb32)
			{
				D3D11_TEXTURE2D_DESC desc;
				surface->GetDesc(&desc);
			}
			else if (vst == sirius::library::video::source::d3d11::desktop::capturer::video_submedia_type_t::nv12)
			{
				D3D11_TEXTURE2D_DESC desc;
				surface->GetDesc(&desc);
			}
		}
	}

private:
	ID3D11Device * _device;

	HANDLE _file;
};

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ INT nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	INT SingleOutput;

	// Synchronization
	HANDLE UnexpectedErrorEvent = nullptr;
	HANDLE ExpectedErrorEvent = nullptr;

	// Window
	HWND WindowHandle = nullptr;

	bool CmdResult = ProcessCmdline(&SingleOutput);
	if (!CmdResult)
	{
		ShowHelp();
		return 0;
	}

	// Event used by the threads to signal an unexpected error and we want to quit the app
	UnexpectedErrorEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	if (!UnexpectedErrorEvent)
	{
		//ProcessFailure(nullptr, L"UnexpectedErrorEvent creation failed", L"Error", E_UNEXPECTED);
		return 0;
	}

	// Event for when a thread encounters an expected error
	ExpectedErrorEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	if (!ExpectedErrorEvent)
	{
		//ProcessFailure(nullptr, L"ExpectedErrorEvent creation failed", L"Error", E_UNEXPECTED);
		return 0;
	}

	// Load simple cursor
	HCURSOR Cursor = nullptr;
	Cursor = LoadCursor(nullptr, IDC_ARROW);
	if (!Cursor)
	{
		//ProcessFailure(nullptr, L"Cursor load failed", L"Error", E_UNEXPECTED);
		return 0;
	}

	// Register class
	WNDCLASSEXW Wc;
	Wc.cbSize = sizeof(WNDCLASSEXW);
	Wc.style = CS_HREDRAW | CS_VREDRAW;
	Wc.lpfnWndProc = WndProc;
	Wc.cbClsExtra = 0;
	Wc.cbWndExtra = 0;
	Wc.hInstance = hInstance;
	Wc.hIcon = nullptr;
	Wc.hCursor = Cursor;
	Wc.hbrBackground = nullptr;
	Wc.lpszMenuName = nullptr;
	Wc.lpszClassName = L"desktop screen capture";
	Wc.hIconSm = nullptr;
	if (!RegisterClassExW(&Wc))
	{
		return 0;
	}

	// Create window
	RECT WindowRect = { 0, 0, 1280, 720 };
	AdjustWindowRect(&WindowRect, WS_OVERLAPPEDWINDOW, FALSE);
	WindowHandle = CreateWindowW(L"desktop screen capture", L"D3D11 desktop screen duplication", WS_OVERLAPPEDWINDOW, 0, 0,
		WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, nullptr, nullptr, hInstance, nullptr);

	if (!WindowHandle)
	{
		return 0;
	}

	DestroyCursor(Cursor);

	ShowWindow(WindowHandle, nCmdShow);
	UpdateWindow(WindowHandle);




	sirius::library::video::source::d3d11::desktop::capturer desktop_capturer;
	capture_handler desktop_capture_handler;
	RECT DeskBounds;
	UINT OutputCount;

	// Message loop (attempts to update screen when no other messages to process)
	MSG msg = { 0 };
	bool FirstTime = true;
	bool Occluded = true;

	while (WM_QUIT != msg.message)
	{
		int32_t status = sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == OCCLUSION_STATUS_MSG)
			{
				// Present may not be occluded now so try again
				Occluded = false;
			}
			else
			{
				// Process window messages
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else if (WaitForSingleObjectEx(UnexpectedErrorEvent, 0, FALSE) == WAIT_OBJECT_0)
		{
			// Unexpected error occurred so exit the application
			break;
		}
		else if (FirstTime || WaitForSingleObjectEx(ExpectedErrorEvent, 0, FALSE) == WAIT_OBJECT_0)
		{
			if (!FirstTime)
			{
				desktop_capturer.stop();
				ResetEvent(ExpectedErrorEvent);
				desktop_capturer.release();
			}
			else
			{
				// First time through the loop so nothing to clean up
				FirstTime = false;
			}


			sirius::library::video::source::d3d11::desktop::capturer::context_t ctx;
			ctx.hwnd = WindowHandle;
			ctx.occlusion_msg = OCCLUSION_STATUS_MSG;
			ctx.single_output = SingleOutput;
			ctx.unexpected_error_event = UnexpectedErrorEvent;
			ctx.expected_error_event = ExpectedErrorEvent;
			ctx.present = TRUE;
			ctx.video_memory_type = sirius::library::video::source::d3d11::desktop::capturer::video_memory_type_t::d3d11;
			ctx.video_submedia_type = sirius::library::video::source::d3d11::desktop::capturer::video_submedia_type_t::nv12;
			ctx.width = 1920;
			ctx.height = 1080;

			ctx.crop.left = 0;
			ctx.crop.top = 0;
			ctx.crop.right = 1920;
			ctx.crop.bottom = 1080;

			ctx.handler = &desktop_capture_handler;
			status = desktop_capturer.initialize(&ctx);
			if (status == sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success)
			{
				status = desktop_capturer.start();
			}
			Occluded = true;
		}
		else
		{
			if (!Occluded)
			{
				status = desktop_capturer.update_application_window(&Occluded);
			}
		}

		if (status != sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success)
		{
			if (status == sirius::library::video::source::d3d11::desktop::capturer::err_code_t::expected)
			{
				// Some type of system transition is occurring so retry
				SetEvent(ExpectedErrorEvent);
			}
			else
			{
				// Unexpected error so exit
				break;
			}
		}
	}

	// Make sure all other threads have exited
	desktop_capturer.stop();
	desktop_capturer.release();

	// Clean up
	CloseHandle(UnexpectedErrorEvent);
	CloseHandle(ExpectedErrorEvent);
	//CloseHandle(TerminateThreadsEvent);

	if (msg.message == WM_QUIT)
	{
		// For a WM_QUIT message we should return the wParam value
		return static_cast<INT>(msg.wParam);
	}

	return 0;
}

//
// Shows help
//
void ShowHelp()
{
	DisplayMsg(L"The following optional parameters can be used -\n  /output [all | n]\t\tto duplicate all outputs or the nth output\n  /?\t\t\tto display this help section",
		L"Proper usage", S_OK);
}

//
// Process command line parameters
//
bool ProcessCmdline(_Out_ INT* Output)
{
	*Output = -1;

	// __argv and __argc are global vars set by system
	for (UINT i = 1; i < static_cast<UINT>(__argc); ++i)
	{
		if ((strcmp(__argv[i], "-output") == 0) ||
			(strcmp(__argv[i], "/output") == 0))
		{
			if (++i >= static_cast<UINT>(__argc))
			{
				return false;
			}

			if (strcmp(__argv[i], "all") == 0)
			{
				*Output = -1;
			}
			else
			{
				*Output = atoi(__argv[i]);
			}
			continue;
		}
		else
		{
			return false;
		}
	}
	return true;
}

//
// Window message processor
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		break;
	}
	case WM_SIZE:
	{
		// Tell output manager that window size has changed
		//OutMgr.WindowResize();
		break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

void DisplayMsg(_In_ LPCWSTR Str, _In_ LPCWSTR Title, HRESULT hr)
{
	if (SUCCEEDED(hr))
	{
		MessageBoxW(nullptr, Str, Title, MB_OK);
		return;
	}

	const UINT StringLen = (UINT)(wcslen(Str) + sizeof(" with HRESULT 0x########."));
	wchar_t* OutStr = new wchar_t[StringLen];
	if (!OutStr)
	{
		return;
	}

	INT LenWritten = swprintf_s(OutStr, StringLen, L"%s with 0x%X.", Str, hr);
	if (LenWritten != -1)
	{
		MessageBoxW(nullptr, OutStr, Title, MB_OK);
	}

	delete[] OutStr;
}
