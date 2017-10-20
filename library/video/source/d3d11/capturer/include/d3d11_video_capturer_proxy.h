#ifndef _D3D11_VIDEO_PROXY_H_
#define _D3D11_VIDEO_PROXY_H_

#include <cstdint>
#include <d3d11.h>
#include <d3d11_1.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <d3dcompiler.h>

#include "d3d11_video_capturer.h"

typedef void	(__stdcall * fnVideoCaptureInitializeCallback)(ID3D11Device * device, int32_t hwnd, int32_t smt, int32_t width, int32_t height);
typedef void	(__stdcall * fnVideoCaptureReceiveCallback)(ID3D11Texture2D * texture);

extern LPVOID		g_current_swap;
extern DXGI_FORMAT	g_dxgi_format;

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
					class capturer::proxy
					{
					public:
						proxy(void);
						virtual ~proxy(void);

						void on_video_capture_proxy_initialized(uint32_t dxgi_ver, LPVOID swap);
						void on_video_capture_proxy_receive(uint32_t dxgi_ver, LPVOID swap);

						void set_video_capture_proxy_initialize_callback(fnVideoCaptureInitializeCallback func);
						void set_video_capture_proxy_receive_callback(fnVideoCaptureReceiveCallback func);

					private:
						DXGI_FORMAT fix_copy_texture_format(DXGI_FORMAT format);

						void fix_sample_count(ID3D11Texture2D * texture);

					private:
						fnVideoCaptureInitializeCallback	_initialize_func;
						//fnVideoCaptureReleaseCallback *	_release_func;
						fnVideoCaptureReceiveCallback		_receive_func;

						ID3D11Device **			_device;
						ID3D11Texture2D *		_copy_texture;
						DXGI_FORMAT				_format;
						uint32_t				_cx;
						uint32_t				_cy;
						DWORD					_hwnd;
						uint32_t				_sample_count;
					};
				};
			};
		};
	};
};

#endif
