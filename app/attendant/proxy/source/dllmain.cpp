#include <windows.h>
#include <process.h>
#include "sirius_attendant_proxy.h"

HANDLE g_pthread = INVALID_HANDLE_VALUE;
HANDLE g_cthread = INVALID_HANDLE_VALUE;
BOOL g_run = FALSE;
unsigned int WINAPI main_process(HANDLE handle)
{
	sirius::app::attendant::proxy::instance().initialize(/*&AmadeusPlayerController::CONFIGURATION_T::INSTANCE()*/);
	if (sirius::app::attendant::proxy::instance().is_initialized())
	{
		if (sirius::app::attendant::proxy::instance().context()->play_after_connect)
		{
			sirius::app::attendant::proxy::instance().connect();
		}
		else
		{
			sirius::app::attendant::proxy::instance().play();
		}
	}
	
	g_run = TRUE;
	while (g_run)
	{
		::Sleep(30);
	}

	if (sirius::app::attendant::proxy::instance().is_initialized())
	{
		if (sirius::app::attendant::proxy::instance().context()->type == sirius::app::attendant::proxy::attendant_type_t::web)
		{
			sirius::app::attendant::proxy::instance().stop();
			if (sirius::app::attendant::proxy::instance().context()->play_after_connect)
				sirius::app::attendant::proxy::instance().disconnect();
		}
	}
	sirius::app::attendant::proxy::instance().release();
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hmodule, DWORD  ul_reason_for_call, LPVOID reserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			wchar_t * command = GetCommandLine();			
                        int32_t argc = 0;
			LPWSTR * argv = CommandLineToArgvW(command, &argc);
			if (!sirius::app::attendant::proxy::parse_argument(argc, argv))
				return FALSE;

			OutputDebugStringA("DllMain....    1     success ");
			if (sirius::app::attendant::proxy::instance().context()->type == sirius::app::attendant::proxy::attendant_type_t::web)
			{
				g_pthread = ::OpenThread(THREAD_ALL_ACCESS, NULL, GetCurrentThreadId());
				unsigned int thrdaddr = 0;
				if (!(g_cthread = (HANDLE)_beginthreadex(NULL, 0, main_process, (LPVOID)g_pthread, 0, &thrdaddr)))
				{
					::CloseHandle(g_pthread);
					g_pthread = INVALID_HANDLE_VALUE;
					g_cthread = INVALID_HANDLE_VALUE;
					return FALSE;
				}
				::CloseHandle(g_pthread);
				g_pthread = INVALID_HANDLE_VALUE;
			}
			break;
		}
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
		{
			OutputDebugString(L"ENTER DLL_PROCESS_DETACH");
			if (sirius::app::attendant::proxy::instance().context()->type == sirius::app::attendant::proxy::attendant_type_t::web)
			{
				if (g_run && g_cthread && g_cthread != INVALID_HANDLE_VALUE)
				{
					g_run = FALSE;
					::WaitForSingleObject(g_cthread, INFINITE);
					::CloseHandle(g_cthread);
					g_cthread = INVALID_HANDLE_VALUE;
				}

			}
			OutputDebugString(L"LEAVE DLL_PROCESS_DETACH");
			break;
		}
	}
	return TRUE;
}