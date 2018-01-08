#include "sirius_attendant_proxy.h"
#include <windows.h>
#include <process.h>

BOOL APIENTRY DllMain(HMODULE hmodule, DWORD  ul_reason_for_call, LPVOID reserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}