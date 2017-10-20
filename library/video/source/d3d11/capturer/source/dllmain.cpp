#include <memory>
#include "sirius_d3d11_video_capturer.h"

//extern HANDLE gh_dll_main_thread;
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

//		sirius_d3d11_video_capture::CONFIGURATION_T::INSTANCE().gpuIndex = siriusPlayerController::CONFIGURATION_T::INSTANCE().gpuIndex;
//		sirius::library::video::source::d3d11::capturer::configuration_t::instance().gpuIndex = siriusPlayerController::Instance().GetConfiguration()->gpuIndex;
		sirius::library::video::source::d3d11::capturer::instance().initialize(&sirius::library::video::source::d3d11::capturer::context_t::instance()); //start hooking		
	}
	break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
	{
		sirius::library::video::source::d3d11::capturer::instance().stop();
		sirius::library::video::source::d3d11::capturer::instance().release(); //stop hooking
		//if (gh_dll_main_thread)
		//	CloseHandle(gh_dll_main_thread);
	}
	break;
	}
	return TRUE;
}