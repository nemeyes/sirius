#include <memory>
#include "sirius_d3d11_video_capturer.h"

#include <algorithm>
#include <map>

bool parse_argument(int argc, wchar_t * argv[])
{
	wchar_t * pargv;
	std::map<std::wstring, std::wstring> param;

	for (int32_t i = 1; i < argc; i++)
	{
		pargv = argv[i];
		if (wcsncmp(pargv, L"--", 2) == 0)
		{
			const wchar_t *p = wcschr(pargv + 2, L'=');
			if (p)
			{
				const wchar_t *f = pargv + 2;
				std::wstring name(f, p);
				std::wstring val(p + 1);
				param.insert(std::make_pair(name, val));
			}
			else
			{
				std::wstring name(pargv + 2);
				std::wstring val;
				val.clear();
				param.insert(std::make_pair(name, val));
			}
		}
		else
		{
			continue;
		}
	}

	std::map<std::wstring, std::wstring>::iterator iter;
	std::wstring value;
	if (param.end() != (iter = param.find(L"enable_present")))
	{
		value = iter->second;
		std::transform(value.begin(), value.end(), value.begin(), ::tolower);
		if (_wcsicmp(value.c_str(), L"true") == 0) {
			sirius::library::video::source::d3d11::capturer::context_t::instance().present = 1;
			OutputDebugStringA("find enable_present TRUE");
		}
		else {
			OutputDebugStringA("find enable_present FALSE");
			sirius::library::video::source::d3d11::capturer::context_t::instance().present = 0;
		}
	}

	if (param.end() != (iter = param.find(L"gpu_index")))
	{
		value = iter->second;
		sirius::library::video::source::d3d11::capturer::context_t::instance().gpuindex = _wtoi(value.c_str());
	}

	if (param.end() != (iter = param.find(L"player_type")))
	{
		value = iter->second;

		if (!_wcsicmp(value.c_str(), L"vr360"))
			sirius::library::video::source::d3d11::capturer::context_t::instance().player_type = sirius::library::video::source::d3d11::capturer::player_type_t::vr360;
		else if (!_wcsicmp(value.c_str(), L"web"))
			sirius::library::video::source::d3d11::capturer::context_t::instance().player_type = sirius::library::video::source::d3d11::capturer::player_type_t::web;
		else if (!_wcsicmp(value.c_str(), L"unity"))
			sirius::library::video::source::d3d11::capturer::context_t::instance().player_type = sirius::library::video::source::d3d11::capturer::player_type_t::unity;
		else if (!_wcsicmp(value.c_str(), L"multi"))
			sirius::library::video::source::d3d11::capturer::context_t::instance().player_type = sirius::library::video::source::d3d11::capturer::player_type_t::mutiview;
		else if (!_wcsicmp(value.c_str(), L"websp"))
			sirius::library::video::source::d3d11::capturer::context_t::instance().player_type = sirius::library::video::source::d3d11::capturer::player_type_t::web_single_process;
		else
		{
			sirius::library::video::source::d3d11::capturer::context_t::instance().player_type = sirius::library::video::source::d3d11::capturer::player_type_t::vr360;
		}
	}
	return true;
}

//extern HANDLE gh_dll_main_thread;
extern HINSTANCE g_hinst_main;
BOOL APIENTRY DllMain(HMODULE h_module, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		wchar_t name[4096];

		wchar_t * cmdline = GetCommandLine();

		int32_t argc = 0;
		LPWSTR * argv = CommandLineToArgvW(cmdline, &argc);
		parse_argument(argc, argv);

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