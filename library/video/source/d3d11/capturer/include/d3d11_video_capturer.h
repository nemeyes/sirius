#ifndef _D3D11_VIDEO_CAPTURER_H_
#define _D3D11_VIDEO_CAPTURER_H_

#include "sirius_d3d11_video_capturer.h"

#include <windows.h>
#include <shlobj.h>
//#define PSAPI_VERSION 1
#include <psapi.h>

#pragma intrinsic(memcpy, memset, memcmp)

#include <xmmintrin.h>
#include <emmintrin.h>
#include <objbase.h>
#include <string>
#include <sstream>
#include <fstream>
#include <d3d11.h>
#include <d3d11_1.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <d3dcompiler.h>
#include <initguid.h>
#include <wincodec.h>
#include <atlbase.h>
#include <shellapi.h>
#include <memory>

#include "d3d11_video_capturer_proxy.h"

typedef void(__stdcall * fnVideoCaptureInitializeCallback)(ID3D11Device * device, int32_t hwnd, int32_t smt, int32_t width, int32_t height);
typedef void(__stdcall * fnVideoCaptureReceiveCallback)(ID3D11Texture2D * texture);


namespace sirius
{
	namespace library
	{
		namespace video
		{
			namespace source
			{
				namespace d3d11
				{
					class capturer::core
					{
					public:
						typedef struct _dxgi_type_t
						{
							static const int32_t dxgi1_0 = 0;
							static const int32_t dxgi1_1 = 1;
							static const int32_t dxgi1_2 = 2;
						} dxgi_type_t;

						core(void);
						virtual ~core(void);

						int32_t initialize(sirius::library::video::source::d3d11::capturer::context_t * context);
						int32_t release(void);
						int32_t start(void);
						int32_t stop(void);
						int32_t pause(void);

						static void __stdcall on_video_capture_initialize_callback(ID3D11Device * device, int32_t hwnd, int32_t smt, int32_t width, int32_t height);
						static void __stdcall on_video_capture_receive_callback(ID3D11Texture2D * texture);

						static void		hook_function(DWORD* p_vtable, void* p_hook_proc, void* p_old_proc, int index);
						static void *	function_detour(uint8_t * src, const uint8_t * dst, const int len);

					private:
						static unsigned __stdcall process_cb(void * param);
						void process(void);

						IDXGISwapChain *	create_dummy_swap(void);
						IDXGIFactory *		get_dxgi_factory_from_device(ID3D11Device * device);

						template <typename otype>
						otype * dynamic_cast_com_object(IUnknown* object)
						{
							otype * oobj = NULL;
							HRESULT result = object->QueryInterface(__uuidof(otype), reinterpret_cast<void**>(&oobj));
							if (SUCCEEDED(result))
							{
								return oobj;
							}
							else
							{
								SIRIUS_SAFE_RELEASE(oobj);
								return NULL;
							}
						}

					private:
						//static sirius::library::video::source::d3d11::capturer::context_t * _context;
						bool	_support_dxgi1_2;	// system
						bool	_used_dxgi1_2;	//
						HANDLE	_thread;
						HWND	_hwnd;
					};
				};
			};
		};
	};
};

#endif