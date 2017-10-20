#include "d3d11_video_capturer.h"
#include <process.h>
#include <tlhelp32.h>
#include <sirius_log4cplus_logger.h>

#ifdef WITH_SAVE_BMP
#include <ScreenGrab.h>
unsigned long nFrame = 0;
ID3D11Device * g_device = NULL;
#endif

HINSTANCE	g_hinst_main	= NULL;
LPVOID		g_current_swap	= NULL;
DXGI_FORMAT g_dxgi_format	= DXGI_FORMAT_UNKNOWN;

std::shared_ptr<sirius::library::video::source::d3d11::capturer::proxy> g_video_capture_proxy(new sirius::library::video::source::d3d11::capturer::proxy());

typedef HRESULT(WINAPI *CREATEDXGIFACTORY1PROC)(REFIID riid, void **ppFactory);
typedef HRESULT(WINAPI*D3D11CREATEPROC)(IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT, D3D_FEATURE_LEVEL*, UINT, UINT, DXGI_SWAP_CHAIN_DESC*, IDXGISwapChain**, IUnknown**, D3D_FEATURE_LEVEL*, IUnknown**);
typedef HRESULT(WINAPI*D3D11CREATEDEVICEPROC)(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags,	CONST D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext);
D3D11CREATEDEVICEPROC org_create_device = NULL;
D3D11CREATEDEVICEPROC hook_create_device = NULL;

HRESULT STDMETHODCALLTYPE create_device_hook(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags,
	CONST D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion,
	ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext)
{
	HRESULT hres = NO_ERROR;

	if ((org_create_device == NULL) || (hook_create_device == NULL))
	{
//		LOGGER::make_trace_log(SLVSC, "%s_%d uuid[%s] : ", __FUNCTION__, __LINE__, cap_d3d11_video_capture::CONFIGURATION_T::INSTANCE().uuid);
	}
	else
	{
		UINT gpu_index=0;		
		gpu_index = sirius::library::video::source::d3d11::capturer::context_t::instance().gpuindex;

		HMODULE dxgi_dll = ::GetModuleHandle(L"dxgi.dll");
		CREATEDXGIFACTORY1PROC createDXGIFactory1 = (CREATEDXGIFACTORY1PROC)GetProcAddress(dxgi_dll, "CreateDXGIFactory1");
		if (!createDXGIFactory1)
		{
			hres = E_FAIL;
			LOGGER::make_error_log(SLVSC, "Could not retrieve CreateDXGIFactory address.");
		}

		IDXGIFactory1 *factory;
		hres = (*createDXGIFactory1)(__uuidof(IDXGIFactory1), (void**)&factory);

		IDXGIAdapter1 *adapter;
		hres = factory->EnumAdapters1(gpu_index, &adapter);
		if (factory)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			factory->Release();
			factory = NULL;
		}
//		OutputDebugStringA("create_device_hook.............");
		LOGGER::make_trace_log(SLVSC, "%s()_%d : ", __FUNCTION__, __LINE__);
		hres = hook_create_device(adapter, D3D_DRIVER_TYPE_UNKNOWN, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
		LOGGER::make_trace_log(SLVSC, "%s()_%d : ", __FUNCTION__, __LINE__);
	}
	return hres;
}


typedef HRESULT(STDMETHODCALLTYPE *DXGISwapPresent1HookPROC)(IDXGISwapChain1 *swap, UINT syncInterval, UINT flags, const DXGI_PRESENT_PARAMETERS *pPresentParameters);
DXGISwapPresent1HookPROC org_DXGI_swap_present1;

HRESULT STDMETHODCALLTYPE dxgi_swap_present1_hook(IDXGISwapChain1 * swap, UINT syncInterval, UINT flags, const DXGI_PRESENT_PARAMETERS *pPresentParameters)
{
	//LOGGER::make_trace_log(SLVSC, "%s()_%d : swap=%p, flags=0x%x", __FUNCTION__, __LINE__, swap, flags);
//	OutputDebugStringA("dxgi_swap_present1_hook.............");
	if (!g_current_swap || g_current_swap!=swap)
	{
		g_current_swap = static_cast<LPVOID>(swap);
		g_video_capture_proxy->on_video_capture_proxy_initialized(sirius::library::video::source::d3d11::capturer::core::dxgi_type_t::dxgi1_2, g_current_swap);
	}

	if (!(flags & DXGI_PRESENT_TEST) && g_current_swap == swap)
	{
		g_current_swap = static_cast<LPVOID>(swap);
		g_video_capture_proxy->on_video_capture_proxy_receive(sirius::library::video::source::d3d11::capturer::core::dxgi_type_t::dxgi1_2, g_current_swap);
	}

	if (!sirius::library::video::source::d3d11::capturer::context_t::instance().present)
		return S_OK;

	return org_DXGI_swap_present1(swap, syncInterval, flags, pPresentParameters);
}
 
typedef HRESULT(STDMETHODCALLTYPE *DXGISwapPresentHookPROC)(IDXGISwapChain *swap, UINT syncInterval, UINT flags);
DXGISwapPresentHookPROC org_DXGI_swap_present;

HRESULT STDMETHODCALLTYPE dxgi_swap_present_hook(IDXGISwapChain *swap, UINT syncInterval, UINT flags)
{
	HRESULT hr = S_OK;
	//LOGGER::make_trace_log(SLVSC, "%s()_%d : swap=%p, flags=0x%x", __FUNCTION__, __LINE__, swap, flags);
//	OutputDebugStringA("dxgi_swap_present_hook.............");
	if (!g_current_swap)
	{
		g_current_swap = static_cast<LPVOID>(swap);
		g_video_capture_proxy->on_video_capture_proxy_initialized(sirius::library::video::source::d3d11::capturer::core::dxgi_type_t::dxgi1_0, swap);
	}

	if (!(flags & DXGI_PRESENT_TEST) && g_current_swap == swap)
	{
		g_current_swap = static_cast<LPVOID>(swap);
		g_video_capture_proxy->on_video_capture_proxy_receive(sirius::library::video::source::d3d11::capturer::core::dxgi_type_t::dxgi1_0, swap);
	}

	if (!sirius::library::video::source::d3d11::capturer::context_t::instance().present)
		return S_OK;

	hr = org_DXGI_swap_present(swap, syncInterval, flags);
	if (FAILED(hr))
	{
		LOGGER::make_trace_log(SLVSC, "%s()_%d : swap=%p", __FUNCTION__, __LINE__, swap);
	}
	return hr;
}

//sirius::library::video::source::d3d11::capturer::context_t * sirius::library::video::source::d3d11::capturer::core::_context = nullptr;

sirius::library::video::source::d3d11::capturer::core::core(void)
	: _support_dxgi1_2(false)
	, _used_dxgi1_2(false)
	, _thread(INVALID_HANDLE_VALUE)
	, _hwnd(NULL)
{
	//LOGGER::make_trace_log(SLVSC, "%s()_%d : ", __FUNCTION__, __LINE__);
}

sirius::library::video::source::d3d11::capturer::core::~core(void)
{
	//LOGGER::make_trace_log(SLVSC, "%s()_%d : ", __FUNCTION__, __LINE__);
}

int32_t sirius::library::video::source::d3d11::capturer::core::initialize(sirius::library::video::source::d3d11::capturer::context_t * context)
{
	HANDLE parent_thread = OpenThread(THREAD_ALL_ACCESS, NULL, GetCurrentThreadId());

	WNDCLASS wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.style = CS_OWNDC;
	wc.hInstance = g_hinst_main;
	wc.lpfnWndProc = (WNDPROC)DefWindowProc;

	wc.lpszClassName = TEXT("Event_Receive_Window");
	if (RegisterClass(&wc))
	{
		_hwnd = CreateWindowExW(0, TEXT("Event_Receive_Window"), TEXT("1111"), WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, 0, 1, 1, NULL, NULL, g_hinst_main, NULL);
		if (!_hwnd)
		{
			if(parent_thread != nullptr)
				CloseHandle(parent_thread);
			return sirius::library::video::source::d3d11::capturer::err_code_t::fail;
		}
	}
	else
	{
		if (parent_thread != nullptr)
			CloseHandle(parent_thread);

		return sirius::library::video::source::d3d11::capturer::err_code_t::fail;
	}

	unsigned thrdaddr = 0;
	_thread = (HANDLE)::_beginthreadex(NULL, 0, sirius::library::video::source::d3d11::capturer::core::process_cb, this, 0, &thrdaddr);
	if (!_thread){
		if (parent_thread != nullptr)
			CloseHandle(parent_thread);

		if (_hwnd != nullptr)
			DestroyWindow(_hwnd);

		return sirius::library::video::source::d3d11::capturer::err_code_t::fail;
	}

	//LOGGER::make_trace_log(SLVSC, "%s()_%d : ", __FUNCTION__, __LINE__);
	return sirius::library::video::source::d3d11::capturer::err_code_t::success;
}

int32_t sirius::library::video::source::d3d11::capturer::core::release(void)
{
	int32_t code = sirius::library::video::source::d3d11::capturer::err_code_t::fail;
	code = stop();

	if (_thread && _thread != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(_thread);
		_thread = INVALID_HANDLE_VALUE;
	}
	return code;
}

int32_t sirius::library::video::source::d3d11::capturer::core::start(void)
{
	//LOGGER::make_trace_log(SLVSC, "%s()_%d : ", __FUNCTION__, __LINE__);
	if (!sirius::library::video::source::d3d11::capturer::context_t::instance().handler)
		return sirius::library::video::source::d3d11::capturer::err_code_t::fail;

	g_video_capture_proxy->set_video_capture_proxy_initialize_callback(sirius::library::video::source::d3d11::capturer::core::on_video_capture_initialize_callback);
	g_video_capture_proxy->set_video_capture_proxy_receive_callback(sirius::library::video::source::d3d11::capturer::core::on_video_capture_receive_callback);
	return sirius::library::video::source::d3d11::capturer::err_code_t::success;
}

int32_t sirius::library::video::source::d3d11::capturer::core::stop(void)
{
	//LOGGER::make_trace_log(SLVSC, "%s()_%d : ", __FUNCTION__, __LINE__);
	g_video_capture_proxy->set_video_capture_proxy_receive_callback(nullptr);
	g_video_capture_proxy->set_video_capture_proxy_initialize_callback(nullptr);
	return sirius::library::video::source::d3d11::capturer::err_code_t::success;
}

int32_t sirius::library::video::source::d3d11::capturer::core::pause(void)
{
	return sirius::library::video::source::d3d11::capturer::err_code_t::success;
}

void sirius::library::video::source::d3d11::capturer::core::on_video_capture_initialize_callback(ID3D11Device * device, int32_t hwnd, int32_t smt, int32_t width, int32_t height)
{
	//LOGGER::make_info_log(SLVSC, "%s()_%d uuid[%s] : ", __FUNCTION__, __LINE__);
#ifdef WITH_SAVE_BMP
	g_device = device;
#endif
	if (sirius::library::video::source::d3d11::capturer::context_t::instance().handler)
		sirius::library::video::source::d3d11::capturer::context_t::instance().handler->on_initialize(device, hwnd, smt, width, height);
}

void sirius::library::video::source::d3d11::capturer::core::on_video_capture_receive_callback(ID3D11Texture2D * texture)
{
	//LOGGER::make_trace_log(SLVSC, "%s()_%d : ", __FUNCTION__, __LINE__);

#ifdef WITH_SAVE_BMP
	ID3D11DeviceContext*	context = nullptr;
	g_device->GetImmediateContext(&context);

	nFrame++;
	wchar_t szFileName[200];
	swprintf(szFileName, sizeof(szFileName), L"SCREENSHOT%u.BMP", nFrame);

	DirectX::SaveWICTextureToFile(context, texture, GUID_ContainerFormatBmp, szFileName, &GUID_WICPixelFormat16bppBGR565);
#endif

	sirius::library::video::source::capturer::entity_t captured;
	captured.memtype = sirius::library::video::source::d3d11::capturer::video_memory_type_t::d3d11;
	captured.data = texture;
	captured.data_size = 0;
	captured.data_capacity = 0;
	if (sirius::library::video::source::d3d11::capturer::context_t::instance().handler)
		sirius::library::video::source::d3d11::capturer::context_t::instance().handler->on_process(&captured);
}

unsigned sirius::library::video::source::d3d11::capturer::core::process_cb(void * param)
{
	sirius::library::video::source::d3d11::capturer::core * self = static_cast<sirius::library::video::source::d3d11::capturer::core*>(param);
	self->process();
	return 0;
}

void sirius::library::video::source::d3d11::capturer::core::process(void)
{
	LOGGER::make_trace_log(SLVSC, "%s()_%d : _hwnd=%d", __FUNCTION__, __LINE__, _hwnd);
	LOGGER::make_trace_log(SLVSC, "%s()_%d : _bsupport_dxgi1_2=%d", __FUNCTION__, __LINE__, _support_dxgi1_2);
	TCHAR lpDllPath[MAX_PATH];
	HMODULE hDll;

	//::Sleep(500);
#if 0
	SHGetFolderPath(NULL, CSIDL_SYSTEM, NULL, SHGFP_TYPE_CURRENT, lpDllPath);
	wcscat_s(lpDllPath, MAX_PATH, TEXT("\\d3d11.dll"));
#else
#if defined(WIN32)
	_snwprintf_s(lpDllPath, sizeof(lpDllPath)/sizeof(TCHAR), L"C:\\Windows\\SysWOW64\\d3d11.dll");
#else
	_snwprintf_s(lpDllPath, sizeof(lpDllPath) / sizeof(TCHAR), L"C:\\Windows\\System32\\d3d11.dll");
#endif
#endif
	hDll = GetModuleHandle(lpDllPath);
	while (!hDll)
	{
		::Sleep(10);
		hDll = GetModuleHandle(lpDllPath);
	}

	if (hDll)
	{
		org_create_device = (D3D11CREATEDEVICEPROC)GetProcAddress(hDll, "D3D11CreateDevice");
		hook_create_device = (D3D11CREATEDEVICEPROC)sirius::library::video::source::d3d11::capturer::core::function_detour((uint8_t*)org_create_device, (uint8_t*)create_device_hook, 5);
	}
	else
		log_error("can't find d3d11.dll %d",GetLastError());


	IDXGISwapChain * swap = create_dummy_swap();
	IDXGISwapChain1 *swap1 = NULL;

	if (swap)
	{
		swap1 = dynamic_cast_com_object<IDXGISwapChain1>(swap);
		UPARAM * vtable1 = *(UPARAM**)swap1;
		sirius::library::video::source::d3d11::capturer::core::hook_function(vtable1, (void*)&dxgi_swap_present1_hook, (void*)&org_DXGI_swap_present1, 22);
		SIRIUS_SAFE_RELEASE(swap1);

		UPARAM * vtable = *(UPARAM**)swap;
		sirius::library::video::source::d3d11::capturer::core::hook_function(vtable, (void*)&dxgi_swap_present_hook, (void*)&org_DXGI_swap_present, 8);
		SIRIUS_SAFE_RELEASE(swap);
	}
	else
	{
		log_error("%s()_%d", __FUNCTION__, __LINE__);
	}

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		//LOGGER::make_trace_log(SLVSC, "%s()_%d : ", __FUNCTION__, __LINE__);
	}
}

IDXGISwapChain * sirius::library::video::source::d3d11::capturer::core::create_dummy_swap(void)
{
	DXGI_SWAP_CHAIN_DESC swapDesc;
	ZeroMemory(&swapDesc, sizeof(swapDesc));
	swapDesc.BufferCount = 2;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.Width = 2;
	swapDesc.BufferDesc.Height = 2;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = _hwnd;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.Windowed = TRUE;

	IDXGISwapChain *swap = NULL;
	IUnknown *device = NULL;
	D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_NULL;

	HRESULT hErr;

	TCHAR lpDllPath[MAX_PATH];
	HMODULE hDll;

	// d3d11
#if 0
	SHGetFolderPath(NULL, CSIDL_SYSTEM, NULL, SHGFP_TYPE_CURRENT, lpDllPath);
	wcscat_s(lpDllPath, MAX_PATH, TEXT("\\d3d11.dll"));
#else
#if defined(WIN32)
	_snwprintf_s(lpDllPath, sizeof(lpDllPath) / sizeof(TCHAR), L"C:\\Windows\\SysWOW64\\d3d11.dll");
#else
	_snwprintf_s(lpDllPath, sizeof(lpDllPath) / sizeof(TCHAR), L"C:\\Windows\\System32\\d3d11.dll");
#endif
#endif
	hDll = GetModuleHandle(lpDllPath);
	if (hDll)
	{
		D3D11CREATEPROC d3d11Create = (D3D11CREATEPROC)GetProcAddress(hDll, "D3D11CreateDeviceAndSwapChain");
		if (d3d11Create)
		{
			D3D_FEATURE_LEVEL desiredLevels[7] =
			{
				D3D_FEATURE_LEVEL_11_1,
				D3D_FEATURE_LEVEL_11_0,
				D3D_FEATURE_LEVEL_10_1,
				D3D_FEATURE_LEVEL_10_0,
				D3D_FEATURE_LEVEL_9_3,
				D3D_FEATURE_LEVEL_9_2,
				D3D_FEATURE_LEVEL_9_1,
			};
			D3D_FEATURE_LEVEL receivedLevel;

			IUnknown *context;
			hErr = (*d3d11Create)(NULL, driverType, NULL, 0, desiredLevels, _countof(desiredLevels), D3D11_SDK_VERSION, &swapDesc, &swap, &device, &receivedLevel, &context);
			if (hErr == E_INVALIDARG)
			{
				hErr = (*d3d11Create)(NULL, driverType, NULL, 0, &desiredLevels[1], _countof(desiredLevels) - 1, D3D11_SDK_VERSION, &swapDesc, &swap, &device, &receivedLevel, &context);
			}
			if (SUCCEEDED(hErr))
			{
				IDXGIFactory *dxgiFactory = get_dxgi_factory_from_device(reinterpret_cast<ID3D11Device *>(device));

				if (dxgiFactory)
				{
					_support_dxgi1_2 = true;

					IDXGIFactory2 *dxgiFactory2 = NULL;
					hErr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void **>(&dxgiFactory2));
					if (SUCCEEDED(hErr))
					{
						DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
						swapChainDesc.BufferCount = 2;
						swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
						swapChainDesc.Width = 2;
						swapChainDesc.Height = 2;
						swapChainDesc.Stereo = FALSE;
						swapChainDesc.SampleDesc.Count = 1;
						swapChainDesc.SampleDesc.Quality = 0;
						swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
						swapChainDesc.SampleDesc.Count = 1;
						//swapChainDesc.Windowed = TRUE;

						IDXGISwapChain1 *swapChain1 = nullptr;
						HRESULT result = dxgiFactory2->CreateSwapChainForHwnd(device, _hwnd, &swapChainDesc, nullptr, nullptr, &swapChain1);
						if (SUCCEEDED(result))
						{
							swap = static_cast<IDXGISwapChain*>(swapChain1);
						}
						SIRIUS_SAFE_RELEASE(dxgiFactory2);
					}
					SIRIUS_SAFE_RELEASE(dxgiFactory);

				}

				context->Release();
				device->Release();
				return swap;
			}
		}
		else
		{

		}
	}
	return NULL;
}

IDXGIFactory * sirius::library::video::source::d3d11::capturer::core::get_dxgi_factory_from_device(ID3D11Device * device)
{
	IDXGIDevice *dxgiDevice = NULL;
	HRESULT result =
		device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void **>(&dxgiDevice));
	if (FAILED(result))
	{
		return NULL;
	}

	IDXGIAdapter *dxgiAdapter = NULL;
	result = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void **>(&dxgiAdapter));
	SIRIUS_SAFE_RELEASE(dxgiDevice);
	if (FAILED(result))
	{
		return NULL;
	}

	DXGI_ADAPTER_DESC desc;
	dxgiAdapter->GetDesc(&desc);

	IDXGIFactory *dxgiFactory = NULL;
	result = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void **>(&dxgiFactory));
	SIRIUS_SAFE_RELEASE(dxgiAdapter);
	if (FAILED(result))
	{
		return NULL;
	}

	return dxgiFactory;
}



void sirius::library::video::source::d3d11::capturer::core::hook_function(DWORD * p_vtable, void * p_hook_proc, void * p_old_proc, int index)
{
	DWORD old_protect;
	VirtualProtect((void*)&p_vtable[index], sizeof(DWORD), PAGE_READWRITE, &old_protect);

	if (p_old_proc)
	{
		*(DWORD*)p_old_proc = p_vtable[index];
	}

	p_vtable[index] = (DWORD)p_hook_proc;

	VirtualProtect(p_vtable, sizeof(DWORD), old_protect, &old_protect);
}

void* sirius::library::video::source::d3d11::capturer::core::function_detour(uint8_t *src, const uint8_t *dst, const int len)
{
	BYTE *jmp = (BYTE*)malloc(len + 5);
	DWORD dwback = 0;
	VirtualProtect(jmp, len + 5, PAGE_EXECUTE_READWRITE, &dwback);
	VirtualProtect(src, len, PAGE_READWRITE, &dwback);
	memcpy(jmp, src, len);
	jmp += len;
	jmp[0] = 0xE9;
	*(DWORD*)(jmp + 1) = (DWORD)(src + len - jmp) - 5;
	memset(src, 0x90, len);
	src[0] = 0xE9;
	*(DWORD*)(src + 1) = (DWORD)(dst - src) - 5;
	VirtualProtect(src, len, dwback, &dwback);
	return (jmp - len);
}
