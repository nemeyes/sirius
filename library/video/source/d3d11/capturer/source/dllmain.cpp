#include <memory>
#include "sirius_d3d11_video_capturer.h"

#include <algorithm>
#include <map>

extern HINSTANCE g_hinst_main;
BOOL APIENTRY DllMain(HMODULE h_module, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		wchar_t name[4096];
		g_hinst_main = h_module;
		int32_t index = 0;
		GetModuleFileNameW(h_module, name, 4096);
		LoadLibrary(name);

		// Move mouse point for unity
		mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, 0, 0, 0, ::GetMessageExtraInfo());
		break;
	}
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}