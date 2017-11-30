#include "d3d11_video_capturer_proxy.h"
#include <sirius_log4cplus_logger.h>

#ifdef DEBUG_BMP_CAPUTRE
#include <ScreenGrab.h>
unsigned long nFrame = 0;
#endif

sirius::library::video::source::d3d11::capturer::proxy::proxy(void)
	: _initialize_func(NULL)
	, _receive_func(NULL)
	, _format(DXGI_FORMAT_UNKNOWN)
	, _cx(0)
	, _cy(0)
	, _hwnd(0)
	, _device(NULL)
	, _sample_count(0)
	, _copy_texture(NULL)
{
}

sirius::library::video::source::d3d11::capturer::proxy::~proxy(void)
{
}

void sirius::library::video::source::d3d11::capturer::proxy::on_video_capture_proxy_initialized(uint32_t dxgi_ver, LPVOID swap)
{
	IUnknown *  deviceunk;
	DXGI_SWAP_CHAIN_DESC scd;
	HRESULT	hr=S_OK;
	//	LOGGER::make_trace_log(SLVSC, "%s %s()_%d : dxgi_ver=%d", __FILE__, __FUNCTION__, __LINE__, dxgi_ver);

	if (dxgi_ver == sirius::library::video::source::d3d11::capturer::core::dxgi_type_t::dxgi1_2)
	{
		IDXGISwapChain1 *swapchain = NULL;
		swapchain = static_cast<IDXGISwapChain1*>(swap);
		if (SUCCEEDED(swapchain->GetDevice(__uuidof(IUnknown), (void**)&deviceunk)))
		{
			if (SUCCEEDED(deviceunk->QueryInterface(__uuidof(ID3D11Device), (void**)&_device)))
			{
				hr = swapchain->GetDesc(&scd);
			}
		}
	}
	else if (dxgi_ver == sirius::library::video::source::d3d11::capturer::core::dxgi_type_t::dxgi1_0)
	{
		IDXGISwapChain *swapchain = NULL;
		swapchain = static_cast<IDXGISwapChain*>(swap);
		if (SUCCEEDED(swapchain->GetDevice(__uuidof(IUnknown), (void**)&deviceunk)))
		{
			if (SUCCEEDED(deviceunk->QueryInterface(__uuidof(ID3D11Device), (void**)&_device)))
			{
				hr = swapchain->GetDesc(&scd);
			}
		}
	}

	if (SUCCEEDED(hr))
	{
		//LOGGER::make_trace_log(SLVSC, "%s()_%d : ", __FUNCTION__, __LINE__);
		_format = scd.BufferDesc.Format;
		if (_format != DXGI_FORMAT_UNKNOWN)
		{
			if (_format != scd.BufferDesc.Format || _cx != scd.BufferDesc.Width || _cy != scd.BufferDesc.Height || _hwnd != (DWORD)scd.OutputWindow)
			{
				g_dxgi_format = fix_copy_texture_format(scd.BufferDesc.Format);
				_cx = scd.BufferDesc.Width;
				_cy = scd.BufferDesc.Height;

				_hwnd = (DWORD)scd.OutputWindow;
				_sample_count = scd.SampleDesc.Count > 1;
			}
			if (_initialize_func)
				(*_initialize_func)((ID3D11Device*)_device, _hwnd, sirius::base::video_submedia_type_t::rgb32, _cx, _cy);
		}

#ifdef DEBUG_BMP_CAPUTRE
		HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		if (FAILED(hr))
		{
			OutputDebugStringA("video_capture_proxy::on_video_capture_proxy_initialized Failed.");
			return;
		}
#endif
	}
}

void sirius::library::video::source::d3d11::capturer::proxy::on_video_capture_proxy_receive(uint32_t dxgi_ver, LPVOID swap)
{
	HRESULT hr = S_OK;
	ID3D11Texture2D * dxgi_buffer = NULL;
	ID3D11Texture2D * out_texture = NULL;
	ID3D11Device * device = NULL;
	DXGI_SWAP_CHAIN_DESC scd;

	//LOGGER::make_trace_log(SLVSC, "%s()_%d : dxgi_ver=%d", __FUNCTION__, __LINE__, dxgi_ver);
	if (dxgi_ver == sirius::library::video::source::d3d11::capturer::core::dxgi_type_t::dxgi1_2)
	{
		IDXGISwapChain1 *swapchain = NULL;
		swapchain = static_cast<IDXGISwapChain1*>(swap);
		if (swapchain)
		{
			hr = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&dxgi_buffer));
			hr = swapchain->GetDesc(&scd);
#ifdef DEBUG_BMP_CAPUTRE
			if (SUCCEEDED(hr))
			{

				ID3D11DeviceContext*	context = nullptr;
				((ID3D11Device*)_device)->GetImmediateContext(&context);

				nFrame++;
				wchar_t szFileName[200];
				swprintf(szFileName, sizeof(szFileName), L"SCREENSHOT%u.BMP", nFrame);

				DirectX::SaveWICTextureToFile(context, dxgi_buffer, GUID_ContainerFormatBmp, szFileName, &GUID_WICPixelFormat16bppBGR565);
			}
#endif
		}
	}
	else if (dxgi_ver == sirius::library::video::source::d3d11::capturer::core::dxgi_type_t::dxgi1_0)
	{
		IDXGISwapChain *swapchain = NULL;
		swapchain = static_cast<IDXGISwapChain*>(swap);
		if (swapchain)
		{
			hr = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&dxgi_buffer));
			hr = swapchain->GetDesc(&scd);

#ifdef DEBUG_BMP_CAPUTRE
			if (SUCCEEDED(hr))
			{

				ID3D11DeviceContext*	context = nullptr;
				((ID3D11Device*)_device)->GetImmediateContext(&context);

				nFrame++;
				wchar_t szFileName[200];
				swprintf(szFileName, sizeof(szFileName), L"SCREENSHOT%u.BMP", nFrame);

				DirectX::SaveWICTextureToFile(context, dxgi_buffer,
					GUID_ContainerFormatBmp, szFileName,
					&GUID_WICPixelFormat16bppBGR565);
			}
#endif
		}
	}

	if (SUCCEEDED(hr))
	{
		fix_sample_count(dxgi_buffer);

//		if ((scd.BufferDesc.Width != 1280) || (scd.BufferDesc.Height != 720))
//			LOGGER::make_trace_log(SLVSC, "%s()_%d : dxgi_buffer=%p, w=%d, h=%d", __FUNCTION__, __LINE__, _copy_texture, scd.BufferDesc.Width, scd.BufferDesc.Height);

		if (_receive_func)
			(*_receive_func)(_copy_texture);

		if (_copy_texture != NULL)
			SIRIUS_SAFE_RELEASE(_copy_texture);

		if (dxgi_buffer != NULL)
			SIRIUS_SAFE_RELEASE(dxgi_buffer);
	}

}

void sirius::library::video::source::d3d11::capturer::proxy::set_video_capture_proxy_initialize_callback(fnVideoCaptureInitializeCallback func)
{
	_initialize_func = func;
}

void sirius::library::video::source::d3d11::capturer::proxy::set_video_capture_proxy_receive_callback(fnVideoCaptureReceiveCallback func)
{
	_receive_func = func;
}

DXGI_FORMAT sirius::library::video::source::d3d11::capturer::proxy::fix_copy_texture_format(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		return DXGI_FORMAT_B8G8R8A8_UNORM;
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	}

	return format;
}

void sirius::library::video::source::d3d11::capturer::proxy::fix_sample_count(ID3D11Texture2D * texture)
{
	bool sample_count = false;
	HRESULT hr = S_OK;

	if ((texture) && (_device != NULL))
	{
		ID3D11Resource  * d3d11_resource = NULL;
		ID3D11DeviceContext *device_context = NULL;
		D3D11_TEXTURE2D_DESC description;
		texture->GetDesc(&description);

		sample_count = description.SampleDesc.Count > 1;

		if (sample_count == 0)
		{
			_copy_texture = texture;
			return;
		}
		((ID3D11Device*)_device)->GetImmediateContext(&device_context);

//		LOGGER::make_trace_log(SLVSC, "%s()_%d, Texture2d desc: arraysize(%d), bindflags(%d), cpuaccess(%d), format(%d), miplevels(%d), miscflag(%d), usage(%d), SampleDesc.(C:%d,Q:%d), Width(%d), Height(%d)", __FUNCTION__, __LINE__,
//		description.ArraySize, description.BindFlags, description.CPUAccessFlags, description.Format, description.MipLevels, description.MiscFlags, description.Usage, description.SampleDesc.Count, description.SampleDesc.Quality, description.Width, description.Height);

		description.ArraySize = 1;
		description.MipLevels = 1;
		description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		description.Usage = D3D11_USAGE_DEFAULT;
		description.MiscFlags = 0;
		description.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		description.CPUAccessFlags = 0;
		description.SampleDesc.Count = 1;
		description.SampleDesc.Quality = 0;

		if (_copy_texture == NULL)
		{
			if (FAILED(hr = ((ID3D11Device*)_device)->CreateTexture2D(&description, NULL, &_copy_texture)))
			{
				LOGGER::make_error_log(SLNS, "%s(), %d, HRESULT(0x%x", __FUNCTION__, __LINE__, hr);
				if (hr == DXGI_ERROR_DEVICE_REMOVED)
				{
					hr = ((ID3D11Device*)_device)->GetDeviceRemovedReason();
					LOGGER::make_error_log(SLNS, "%s(), %d, HRESULT(0x%x", __FUNCTION__, __LINE__, hr);
				}
			}
		}

		if (sample_count)
		{
			if (SUCCEEDED(hr = _copy_texture->QueryInterface(__uuidof(ID3D11Resource), (void**)&d3d11_resource)))
			{
				device_context->ResolveSubresource(d3d11_resource, 0, texture, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
				//LOGGER::make_trace_log(SLVSC, "%s()_%d", __FUNCTION__, __LINE__);
				device_context->CopyResource(_copy_texture, texture);
				SIRIUS_SAFE_RELEASE(d3d11_resource);
			}
			else
			{
				LOGGER::make_error_log(SLNS, "%s(), %d, HRESULT(0x%x", __FUNCTION__, __LINE__, hr);
			}
		}
		else
		{
			if (_copy_texture != NULL)
					device_context->CopyResource(_copy_texture, texture);
			else
				LOGGER::make_error_log(SLVSC, "%s()_%d : _copy_texture != nullprt", __FUNCTION__, __LINE__);
		}
	}
	else
	{
		LOGGER::make_error_log(SLNS, "%s(), %d, sirius_CRITICAL createTexture Failed", __FUNCTION__, __LINE__);
	}

}