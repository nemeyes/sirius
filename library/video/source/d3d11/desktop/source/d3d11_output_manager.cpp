#include "d3d11_output_manager.h"


sirius::library::video::source::d3d11::desktop::capturer::core::output_manager::output_manager(void)
	: _swap_chain(nullptr)
	, _device(nullptr)
	, _factory(nullptr)
	, _device_context(nullptr)
	, _render_target_view(nullptr)
	, _sampler_linear(nullptr)
	, _blend_state(nullptr)
	, _vertex_shader(nullptr)
	, _pixel_shader(nullptr)
	, _input_layout(nullptr)
	, _shared_surface(nullptr)
	, _key_mutex(nullptr)
	, _hwnd(nullptr)
	, _needs_resize(false)
	, _occlusion_cookie(0)
	, _video_submedia_type(sirius::library::video::source::d3d11::desktop::capturer::video_submedia_type_t::rgb32)
	, _video_memory_type(sirius::library::video::source::d3d11::desktop::capturer::video_memory_type_t::host)
	, _width(0)
	, _height(0)
	, _handler(nullptr)
	, _d3d11_2d_cpu_texture_buffer(nullptr)
{

}

sirius::library::video::source::d3d11::desktop::capturer::core::output_manager::~output_manager(void)
{
	//release();
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::core::output_manager::initialize(sirius::library::video::source::d3d11::desktop::capturer::context_t * context, uint32_t * count_output, RECT * desktop_bounds)
{
	HRESULT hr = E_FAIL;
	int32_t status = sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;

	_hwnd = context->hwnd;
	_present = context->present;
	_video_submedia_type = context->video_submedia_type;
	_video_memory_type = context->video_memory_type;
	_width = context->width;
	_height = context->height;

	_crop_x = context->crop.left;
	_crop_y = context->crop.top;
	_crop_width = context->crop.right - context->crop.left;
	int32_t crop_width_remain = _crop_width % 16;
	_crop_width = _crop_width - crop_width_remain;

	_crop_height = context->crop.bottom - context->crop.top;
	int32_t crop_height_remain = _crop_height % 16;
	_crop_height = _crop_height - crop_height_remain;


	if (_crop_width == 0 || _crop_height == 0)
	{
		_crop_width = _width;
		_crop_height = _height;
	}


	_handler = context->handler;

	uint32_t cnt_driver_types = COUNT_OF_DRIVER_TYPES;
	uint32_t cnt_feature_levels = COUNT_OF_FEATURE_LEVELS;

	D3D_FEATURE_LEVEL feature_level;
	for (uint32_t i = 0; i < cnt_driver_types; i++)
	{
		hr = D3D11CreateDevice(nullptr, DRIVER_TYPES[i], nullptr, 0, FEATURE_LEVELS, cnt_feature_levels, D3D11_SDK_VERSION, &_device, &feature_level, &_device_context);
		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;

	if (_handler && _video_memory_type == sirius::library::video::source::d3d11::desktop::capturer::video_memory_type_t::host)
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.ArraySize = 1;
		if (_video_submedia_type == sirius::library::video::source::d3d11::desktop::capturer::video_submedia_type_t::rgb32)
		{
			desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			_d3d11_2d_cpu_texture_buffer = static_cast<uint8_t*>(malloc(_crop_width * _crop_height * 4));
		}
		else if (_video_submedia_type == sirius::library::video::source::d3d11::desktop::capturer::video_submedia_type_t::nv12)
		{
			desc.Format = DXGI_FORMAT_NV12;
			_d3d11_2d_cpu_texture_buffer = static_cast<uint8_t*>(malloc(_crop_width * _crop_height * 1.5));
		}
		desc.Width = _crop_width;
		desc.Height = _crop_height;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.MiscFlags = 0;
		desc.BindFlags = 0;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		CComPtr<ID3D11Texture2D> d3d11_2d_cpu_texture = NULL;
		hr = _device->CreateTexture2D(&desc, NULL, &d3d11_2d_cpu_texture);
		if (SUCCEEDED(hr))
			_d3d11_2d_cpu_texture = d3d11_2d_cpu_texture;
	}
	else if (_handler && _video_memory_type == sirius::library::video::source::d3d11::desktop::capturer::video_memory_type_t::d3d11)
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_NV12;
		desc.Width = _crop_width;
		desc.Height = _crop_height;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.MiscFlags = 0;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		CComPtr<ID3D11Texture2D> d3d11_2d_texture = NULL;
		HRESULT hr = _device->CreateTexture2D(&desc, NULL, &d3d11_2d_texture);
		if (SUCCEEDED(hr))
			_d3d11_2d_texture = d3d11_2d_texture;
	}

	if (_handler && _video_submedia_type != sirius::library::video::source::d3d11::desktop::capturer::video_submedia_type_t::rgb32)
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.Width = _crop_width;
		desc.Height = _crop_height;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.MiscFlags = 0;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		CComPtr<ID3D11Texture2D> d3d11_2d_texture_intermediate = NULL;
		hr = _device->CreateTexture2D(&desc, NULL, &d3d11_2d_texture_intermediate);
		if (SUCCEEDED(hr))
			_d3d11_2d_texture_intermediate = d3d11_2d_texture_intermediate;

		ATL::CComPtr<ID3D11VideoContext> d3d11_video_context = NULL;
		do
		{
			hr = _device->QueryInterface(__uuidof(ID3D11VideoDevice), (void**)&_d3d11_video_device);
			if (FAILED(hr))
				break;

			_device->GetImmediateContext(&_d3d11_device_context);
			hr = _d3d11_device_context->QueryInterface(__uuidof(ID3D11VideoContext), (void**)&d3d11_video_context);
			if (FAILED(hr))
				break;

			D3D11_VIDEO_PROCESSOR_CONTENT_DESC content_desc;
			content_desc.InputFrameFormat = D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE;

			content_desc.InputWidth = (DWORD)_crop_width;
			content_desc.InputHeight = (DWORD)_crop_height;
			content_desc.OutputWidth = (DWORD)_crop_width;
			content_desc.OutputHeight = (DWORD)_crop_height;

			content_desc.InputFrameRate.Numerator = 30;
			content_desc.InputFrameRate.Denominator = 1;
			content_desc.OutputFrameRate.Numerator = 30;
			content_desc.OutputFrameRate.Denominator = 1;
			content_desc.Usage = D3D11_VIDEO_USAGE_PLAYBACK_NORMAL;

			hr = _d3d11_video_device->CreateVideoProcessorEnumerator(&content_desc, &_d3d11_video_processor_enum);
			if (FAILED(hr))
				break;

			UINT flags;
			DXGI_FORMAT output_format = DXGI_FORMAT_NV12;
			hr = _d3d11_video_processor_enum->CheckVideoProcessorFormat(output_format, &flags);
			if (FAILED(hr) || (flags & D3D11_VIDEO_PROCESSOR_FORMAT_SUPPORT_OUTPUT) == 0)
				break;

			DWORD index = 0;
			D3D11_VIDEO_PROCESSOR_CAPS caps = {};
			D3D11_VIDEO_PROCESSOR_RATE_CONVERSION_CAPS conv_caps = {};

			hr = _d3d11_video_processor_enum->GetVideoProcessorCaps(&caps);
			if (FAILED(hr))
				break;

			for (DWORD i = 0; i < caps.RateConversionCapsCount; i++)
			{
				hr = _d3d11_video_processor_enum->GetVideoProcessorRateConversionCaps(i, &conv_caps);
				if (FAILED(hr))
					break;

				if ((conv_caps.ProcessorCaps & D3D11_VIDEO_PROCESSOR_PROCESSOR_CAPS_DEINTERLACE_BOB) != 0)
				{
					index = i;
					break;
				}
			}

			if (FAILED(hr))
				break;

			hr = _d3d11_video_device->CreateVideoProcessor(_d3d11_video_processor_enum, index, &_d3d11_video_processor);
			if (FAILED(hr))
				break;

			status = sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;
		} while (0);
	}



	IDXGIDevice * dxgi_device = nullptr;
	hr = _device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgi_device));
	if (FAILED(hr))
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;

	IDXGIAdapter * dxgi_adapter = nullptr;
	hr = dxgi_device->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&dxgi_adapter));
	dxgi_device->Release();
	dxgi_device = nullptr;
	if (FAILED(hr))
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;

	hr = dxgi_adapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&_factory));
	dxgi_adapter->Release();
	dxgi_adapter = nullptr;
	if (FAILED(hr))
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;

	hr = _factory->RegisterOcclusionStatusWindow(context->hwnd, context->occlusion_msg, &_occlusion_cookie);
	if (FAILED(hr))
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;

	RECT rect;
	//::GetClientRect(_hwnd, &rect);
	//uint32_t width = rect.right - rect.left;
	//uint32_t height = rect.bottom - rect.top;

	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc;
	memset(&swap_chain_desc, 0x00, sizeof(swap_chain_desc));

	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	swap_chain_desc.BufferCount = 2;
	swap_chain_desc.Width = _width;
	swap_chain_desc.Height = _height;
	swap_chain_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.SampleDesc.Count = 1;
	swap_chain_desc.SampleDesc.Quality = 0;
	hr = _factory->CreateSwapChainForHwnd(_device, _hwnd, &swap_chain_desc, nullptr, nullptr, &_swap_chain);
	if (FAILED(hr))
	{
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}

	// Disable the ALT-ENTER shortcut for entering full-screen mode
	hr = _factory->MakeWindowAssociation(_hwnd, DXGI_MWA_NO_ALT_ENTER);
	if (FAILED(hr))
	{
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}

	status = create_shared_surface(context->single_output, count_output, desktop_bounds);
	if (status != sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success)
	{
		return status;
	}

	status = make_render_target_view();
	if (status != sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success)
	{
		return status;
	}

	set_view_port(_width, _height);

	// Create the sample state
	D3D11_SAMPLER_DESC sampler_desc;
	RtlZeroMemory(&sampler_desc, sizeof(sampler_desc));
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampler_desc.MinLOD = 0;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = _device->CreateSamplerState(&sampler_desc, &_sampler_linear);
	if (FAILED(hr))
	{
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}

	// Create the blend state
	D3D11_BLEND_DESC blend_state_desc;
	blend_state_desc.AlphaToCoverageEnable = FALSE;
	blend_state_desc.IndependentBlendEnable = FALSE;
	blend_state_desc.RenderTarget[0].BlendEnable = TRUE;
	blend_state_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blend_state_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blend_state_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend_state_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend_state_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blend_state_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend_state_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = _device->CreateBlendState(&blend_state_desc, &_blend_state);
	if (FAILED(hr))
	{
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}

	// Initialize shaders
	status = initialize_shaders();
	if (status != sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success)
	{
		return status;
	}

	GetWindowRect(_hwnd, &rect);
	MoveWindow(_hwnd, rect.left, rect.top, (desktop_bounds->right - desktop_bounds->left) / 2, (desktop_bounds->bottom - desktop_bounds->top) / 2, TRUE);

	if(_handler)
		_handler->on_initialize(_device, (int32_t)_hwnd, _video_submedia_type, _crop_width, _crop_height);

	return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::core::output_manager::release(void)
{
	if (_handler)
		_handler->on_release();

	if (_video_submedia_type != sirius::library::video::source::d3d11::desktop::capturer::video_submedia_type_t::rgb32)
	{
		_d3d11_video_processor = NULL;
		_d3d11_video_processor_enum = NULL;
		_d3d11_device_context = NULL;
		_d3d11_video_device = NULL;

		ID3D11Texture2D * d3d11_2d = nullptr;
		d3d11_2d = _d3d11_2d_texture.Detach();
		if (d3d11_2d)
		{
			d3d11_2d->Release();
			d3d11_2d = nullptr;
		}

		d3d11_2d = _d3d11_2d_texture_intermediate.Detach();
		if (d3d11_2d)
		{
			d3d11_2d->Release();
			d3d11_2d = nullptr;
		}
	}

	if (_vertex_shader)
	{
		_vertex_shader->Release();
		_vertex_shader = nullptr;
	}

	if (_pixel_shader)
	{
		_pixel_shader->Release();
		_pixel_shader = nullptr;
	}

	if (_input_layout)
	{
		_input_layout->Release();
		_input_layout = nullptr;
	}

	if (_render_target_view)
	{
		_render_target_view->Release();
		_render_target_view = nullptr;
	}

	if (_sampler_linear)
	{
		_sampler_linear->Release();
		_sampler_linear = nullptr;
	}

	if (_blend_state)
	{
		_blend_state->Release();
		_blend_state = nullptr;
	}

	if (_device_context)
	{
		_device_context->Release();
		_device_context = nullptr;
	}

	if (_handler && _video_memory_type == sirius::library::video::source::d3d11::desktop::capturer::video_memory_type_t::host)
	{
		ID3D11Texture2D * d3d11_2d = nullptr;
		d3d11_2d = _d3d11_2d_cpu_texture.Detach();
		if (d3d11_2d)
		{
			d3d11_2d->Release();
			d3d11_2d = nullptr;
		}
		_handler = nullptr;

		free(_d3d11_2d_cpu_texture_buffer);
		_d3d11_2d_cpu_texture_buffer = nullptr;
	}

	if (_device)
	{
		_device->Release();
		_device = nullptr;
	}

	if (_swap_chain)
	{
		_swap_chain->Release();
		_swap_chain = nullptr;
	}

	if (_shared_surface)
	{
		_shared_surface->Release();
		_shared_surface = nullptr;
	}

	if (_key_mutex)
	{
		_key_mutex->Release();
		_key_mutex = nullptr;
	}

	if (_factory)
	{
		if (_occlusion_cookie)
		{
			_factory->UnregisterOcclusionStatus(_occlusion_cookie);
			_occlusion_cookie = 0;
		}
		_factory->Release();
		_factory = nullptr;
	}

	return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::core::output_manager::update_application_window(sirius::library::video::source::d3d11::desktop::capturer::core::capture_info_t * info, bool * occluded)
{
	// In a typical desktop duplication application there would be an application running on one system collecting the desktop images
	// and another application running on a different system that receives the desktop images via a network and display the image. This
	// sample contains both these aspects into a single application.
	// This routine is the part of the sample that displays the desktop image onto the display

	// Try and acquire sync on common display buffer
	HRESULT hr = _key_mutex->AcquireSync(1, 100);
	if (hr == static_cast<HRESULT>(WAIT_TIMEOUT))
	{
		// Another thread has the keyed mutex so try again later
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;
	}
	else if (FAILED(hr))
	{
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}

	// Got mutex, so draw
	int32_t status = draw_frame();
	if (status == sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success)
	{
		// We have keyed mutex so we can access the mouse info
		if (info->visible)
		{
			// Draw mouse into texture
			status = draw_mouse(info);
		}
	}

	// Release keyed mutex
	hr = _key_mutex->ReleaseSync(0);
	if (FAILED(hr))
	{
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}

	// Present to window if all worked
	if (status == sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success)
	{
		if (_handler)
		{
			ID3D11Texture2D * dxgi_backbuffer = nullptr;
			hr = _swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&dxgi_backbuffer);
			if (FAILED(hr))
			{
				return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
			}
			else if (hr == DXGI_STATUS_OCCLUDED)
			{
				*occluded = true;
			}

			if (_video_memory_type == sirius::library::video::source::d3d11::desktop::capturer::video_memory_type_t::host)
			{
				D3D11_TEXTURE2D_DESC desc;
				dxgi_backbuffer->GetDesc(&desc);
				if (_video_submedia_type == sirius::library::video::source::d3d11::desktop::capturer::video_submedia_type_t::rgb32)
				{
					D3D11_BOX region;
					region.left = _crop_x;
					region.top = _crop_y;
					region.right = _crop_x + _crop_width;
					region.bottom = _crop_y + _crop_height;
					region.front = 0;
					region.back = 1;
					_device_context->CopySubresourceRegion(_d3d11_2d_cpu_texture, 0, 0, 0, 0, dxgi_backbuffer, 0, &region);

					D3D11_MAPPED_SUBRESOURCE subres;
					hr = _device_context->Map(_d3d11_2d_cpu_texture, 0, D3D11_MAP_READ, 0, &subres);
					if (SUCCEEDED(hr))
					{
						D3D11_TEXTURE2D_DESC d3d11_2d_cpu_texture_desc;
						_d3d11_2d_cpu_texture->GetDesc(&d3d11_2d_cpu_texture_desc);
						uint8_t * bytes = (uint8_t*)subres.pData;
						uint32_t width = d3d11_2d_cpu_texture_desc.Width;
						uint32_t height = d3d11_2d_cpu_texture_desc.Height;
						uint32_t pitch = subres.RowPitch;

						memcpy(_d3d11_2d_cpu_texture_buffer, bytes, width * height * 4);
						_device_context->Unmap(_d3d11_2d_cpu_texture, 0);

						sirius::library::video::source::d3d11::desktop::capturer::entity_t input;
						input.memtype = sirius::library::video::source::d3d11::desktop::capturer::video_memory_type_t::d3d11;
						input.data = _d3d11_2d_cpu_texture_buffer;
						input.data_size = width * height * 4;
						input.data_capacity = input.data_size;
						_handler->on_process(&input);
					}
				}
				else
				{
					D3D11_BOX region;
					region.left = _crop_x;
					region.top = _crop_y;
					region.right = _crop_x + _crop_width;
					region.bottom = _crop_y + _crop_height;
					region.front = 0;
					region.back = 1;
					_device_context->CopySubresourceRegion(_d3d11_2d_texture_intermediate, 0, 0, 0, 0, dxgi_backbuffer, 0, &region);

					if (convert(_d3d11_2d_texture_intermediate, _d3d11_2d_texture) == sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success)
					{
						_device_context->CopyResource((ID3D11Resource*)_d3d11_2d_cpu_texture, (ID3D11Resource*)_d3d11_2d_texture);

						D3D11_MAPPED_SUBRESOURCE subres;
						hr = _device_context->Map(_d3d11_2d_cpu_texture, 0, D3D11_MAP_READ, 0, &subres);
						if (SUCCEEDED(hr))
						{
							D3D11_TEXTURE2D_DESC d3d11_2d_cpu_texture_desc;
							_d3d11_2d_cpu_texture->GetDesc(&d3d11_2d_cpu_texture_desc);
							uint8_t * bytes = (uint8_t*)subres.pData;
							uint32_t width = d3d11_2d_cpu_texture_desc.Width;
							uint32_t height = d3d11_2d_cpu_texture_desc.Height;
							uint32_t pitch = subres.RowPitch;

							memcpy(_d3d11_2d_cpu_texture_buffer, bytes, width * height * 1.5);
							_device_context->Unmap(_d3d11_2d_cpu_texture, 0);

							sirius::library::video::source::d3d11::desktop::capturer::entity_t input;
							input.memtype = sirius::library::video::source::d3d11::desktop::capturer::video_memory_type_t::d3d11;
							input.data = _d3d11_2d_cpu_texture_buffer;
							input.data_size = width * height * 1.5;
							input.data_capacity = input.data_size;
							_handler->on_process(&input);
						}
					}
				}
			}
			else if (_video_memory_type == sirius::library::video::source::d3d11::desktop::capturer::video_memory_type_t::d3d11)
			{
				D3D11_TEXTURE2D_DESC desc;
				dxgi_backbuffer->GetDesc(&desc);
				if (_video_submedia_type == sirius::library::video::source::d3d11::desktop::capturer::video_submedia_type_t::rgb32)
				{
					D3D11_BOX region;
					region.left = _crop_x;
					region.top = _crop_y;
					region.right = _crop_x + _crop_width;
					region.bottom = _crop_y + _crop_height;
					region.front = 0;
					region.back = 1;
					_device_context->CopySubresourceRegion(_d3d11_2d_texture, 0, 0, 0, 0, dxgi_backbuffer, 0, &region);

					/*D3D11_TEXTURE2D_DESC d3d11_texture_desc;
					_d3d11_2d_texture->GetDesc(&d3d11_texture_desc);
					uint32_t width = d3d11_texture_desc.Width;
					uint32_t height = d3d11_texture_desc.Height;*/

					sirius::library::video::source::d3d11::desktop::capturer::entity_t input;
					input.memtype = sirius::library::video::source::d3d11::desktop::capturer::video_memory_type_t::d3d11;
					input.data = _d3d11_2d_texture;
					input.data_size = 0;
					input.data_capacity = 0;
					_handler->on_process(&input);
				}
				else
				{
					D3D11_BOX region;
					region.left = _crop_x;
					region.top = _crop_y;
					region.right = _crop_x + _crop_width;
					region.bottom = _crop_y + _crop_height;
					region.front = 0;
					region.back = 1;
					_device_context->CopySubresourceRegion(_d3d11_2d_texture_intermediate, 0, 0, 0, 0, dxgi_backbuffer, 0, &region);

					if (convert(_d3d11_2d_texture_intermediate, _d3d11_2d_texture) == sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success)
					{
						/*D3D11_TEXTURE2D_DESC d3d11_texture_desc;
						_d3d11_2d_texture->GetDesc(&d3d11_texture_desc);
						uint32_t width = d3d11_texture_desc.Width;
						uint32_t height = d3d11_texture_desc.Height;*/

						sirius::library::video::source::d3d11::desktop::capturer::entity_t input;
						input.memtype = sirius::library::video::source::d3d11::desktop::capturer::video_memory_type_t::d3d11;
						input.data = _d3d11_2d_texture;
						input.data_size = 0;
						input.data_capacity = 0;
						_handler->on_process(&input);
					}
				}
			}

			dxgi_backbuffer->Release();
			dxgi_backbuffer = nullptr;
		}

		if (_present)
		{
			hr = _swap_chain->Present(1, 0);
			if (FAILED(hr))
			{
				return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
			}
			else if (hr == DXGI_STATUS_OCCLUDED)
			{
				*occluded = true;
			}
		}
	}

	return status;
}

HANDLE sirius::library::video::source::d3d11::desktop::capturer::core::output_manager::shared_handle(void)
{
	HANDLE handle = nullptr;

	IDXGIResource * dxgi_resource = nullptr;
	HRESULT hr = _shared_surface->QueryInterface(__uuidof(IDXGIResource), reinterpret_cast<void**>(&dxgi_resource));
	if (SUCCEEDED(hr))
	{
		dxgi_resource->GetSharedHandle(&handle);
		dxgi_resource->Release();
		dxgi_resource = nullptr;
	}
	return handle;
}

void sirius::library::video::source::d3d11::desktop::capturer::core::output_manager::resize_windows(void)
{
	_needs_resize = TRUE;
}

int32_t	sirius::library::video::source::d3d11::desktop::capturer::core::output_manager::process_mono_mask(bool bmono, sirius::library::video::source::d3d11::desktop::capturer::core::capture_info_t * info, int32_t * width, int32_t * height, int32_t * left, int32_t * top, uint8_t ** buffer, D3D11_BOX * box)
{
	// Desktop dimensions
	D3D11_TEXTURE2D_DESC full_desc;
	_shared_surface->GetDesc(&full_desc);
	int32_t desktop_width = full_desc.Width;
	int32_t desktop_height = full_desc.Height;

	// Pointer position
	int32_t given_left = info->position.x;
	int32_t given_top = info->position.y;

	// Figure out if any adjustment is needed for out of bound positions
	if (given_left < 0)
	{
		*width = given_left + static_cast<INT>(info->shape_info.Width);
	}
	else if ((given_left + static_cast<INT>(info->shape_info.Width)) > desktop_width)
	{
		*width = desktop_width - given_left;
	}
	else
	{
		*width = static_cast<INT>(info->shape_info.Width);
	}

	if (bmono)
	{
		info->shape_info.Height = info->shape_info.Height / 2;
	}

	if (given_top < 0)
	{
		*height = given_top + static_cast<INT>(info->shape_info.Height);
	}
	else if ((given_top + static_cast<INT>(info->shape_info.Height)) > desktop_height)
	{
		*height = desktop_height - given_top;
	}
	else
	{
		*height = static_cast<INT>(info->shape_info.Height);
	}

	if (bmono)
	{
		info->shape_info.Height = info->shape_info.Height * 2;
	}

	*left = (given_left < 0) ? 0 : given_left;
	*top = (given_top < 0) ? 0 : given_top;

	// Staging buffer/texture
	D3D11_TEXTURE2D_DESC copy_buffer_desc;
	copy_buffer_desc.Width = *width;
	copy_buffer_desc.Height = *height;
	copy_buffer_desc.MipLevels = 1;
	copy_buffer_desc.ArraySize = 1;
	copy_buffer_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	copy_buffer_desc.SampleDesc.Count = 1;
	copy_buffer_desc.SampleDesc.Quality = 0;
	copy_buffer_desc.Usage = D3D11_USAGE_STAGING;
	copy_buffer_desc.BindFlags = 0;
	copy_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	copy_buffer_desc.MiscFlags = 0;

	ID3D11Texture2D * copy_buffer = nullptr;
	HRESULT hr = _device->CreateTexture2D(&copy_buffer_desc, nullptr, &copy_buffer);
	if (FAILED(hr))
	{
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}

	// Copy needed part of desktop image
	box->left = *left;
	box->top = *top;
	box->right = *left + *width;
	box->bottom = *top + *height;
	_device_context->CopySubresourceRegion(copy_buffer, 0, 0, 0, 0, _shared_surface, 0, box);

	// QI for IDXGISurface
	IDXGISurface * copy_surface = nullptr;
	hr = copy_buffer->QueryInterface(__uuidof(IDXGISurface), (void **)&copy_surface);
	copy_buffer->Release();
	copy_buffer = nullptr;
	if (FAILED(hr))
	{
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}

	// Map pixels
	DXGI_MAPPED_RECT mapped_surface;
	hr = copy_surface->Map(&mapped_surface, DXGI_MAP_READ);
	if (FAILED(hr))
	{
		copy_surface->Release();
		copy_surface = nullptr;
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}

	// New mouseshape buffer
	*buffer = new (std::nothrow) BYTE[*width * *height * BPP];
	if (!(*buffer))
	{
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}

	uint32_t * buffer32 = reinterpret_cast<uint32_t*>(*buffer);
	uint32_t * desktop32 = reinterpret_cast<UINT*>(mapped_surface.pBits);
	uint32_t desktop_pitch_in_pixels = mapped_surface.Pitch / sizeof(UINT);

	// What to skip (pixel offset)
	uint32_t skip_x = (given_left < 0) ? (-1 * given_left) : (0);
	uint32_t skip_y = (given_top < 0) ? (-1 * given_top) : (0);

	if (bmono)
	{
		for (int32_t row = 0; row < *height; ++row)
		{
			// Set mask
			BYTE mask = 0x80;
			mask = mask >> (skip_x % 8);
			for (int32_t col = 0; col < *width; ++col)
			{
				// Get masks using appropriate offsets
				BYTE and_mask = info->shape_buffer[((col + skip_x) / 8) + ((row + skip_y) * (info->shape_info.Pitch))] & mask;
				BYTE xor_mask = info->shape_buffer[((col + skip_x) / 8) + ((row + skip_y + (info->shape_info.Height / 2)) * (info->shape_info.Pitch))] & mask;
				UINT and_mask32 = (and_mask) ? 0xFFFFFFFF : 0xFF000000;
				UINT xor_mask32 = (xor_mask) ? 0x00FFFFFF : 0x00000000;

				// Set new pixel
				buffer32[(row * *width) + col] = (desktop32[(row * desktop_pitch_in_pixels) + col] & and_mask32) ^ xor_mask32;

				// Adjust mask
				if (mask == 0x01)
				{
					mask = 0x80;
				}
				else
				{
					mask = mask >> 1;
				}
			}
		}
	}
	else
	{
		uint32_t * Buffer32 = reinterpret_cast<UINT*>(info->shape_buffer);

		// Iterate through pixels
		for (int32_t row = 0; row < *height; ++row)
		{
			for (int32_t col = 0; col < *width; ++col)
			{
				// Set up mask
				uint32_t mask_val = 0xFF000000 & Buffer32[(col + skip_x) + ((row + skip_y) * (info->shape_info.Pitch / sizeof(UINT)))];
				if (mask_val)
				{
					// Mask was 0xFF
					buffer32[(row * *width) + col] = (desktop32[(row * desktop_pitch_in_pixels) + col] ^ buffer32[(col + skip_x) + ((row + skip_y) * (info->shape_info.Pitch / sizeof(UINT)))]) | 0xFF000000;
				}
				else
				{
					// Mask was 0x00
					buffer32[(row * *width) + col] = buffer32[(col + skip_x) + ((row + skip_y) * (info->shape_info.Pitch / sizeof(UINT)))] | 0xFF000000;
				}
			}
		}
	}

	// Done with resource
	hr = copy_surface->Unmap();
	copy_surface->Release();
	copy_surface = nullptr;
	if (FAILED(hr))
	{
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}

	return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;;
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::core::output_manager::make_render_target_view(void)
{
	// Get backbuffer
	ID3D11Texture2D * back_buffer = nullptr;
	HRESULT hr = _swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&back_buffer));
	if (FAILED(hr))
	{
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}

	// Create a render target view
	hr = _device->CreateRenderTargetView(back_buffer, nullptr, &_render_target_view);
	back_buffer->Release();
	if (FAILED(hr))
	{
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}

	// Set new render target
	_device_context->OMSetRenderTargets(1, &_render_target_view, nullptr);

	return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;
}

int32_t	sirius::library::video::source::d3d11::desktop::capturer::core::output_manager::set_view_port(uint32_t width, uint32_t height)
{
	D3D11_VIEWPORT VP;
	VP.Width = static_cast<FLOAT>(width);
	VP.Height = static_cast<FLOAT>(height);
	VP.MinDepth = 0.0f;
	VP.MaxDepth = 1.0f;
	VP.TopLeftX = 0;
	VP.TopLeftY = 0;
	_device_context->RSSetViewports(1, &VP);

	return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::core::output_manager::initialize_shaders(void)
{
	HRESULT hr;

	uint32_t size = ARRAYSIZE(g_VS);
	hr = _device->CreateVertexShader(g_VS, size, nullptr, &_vertex_shader);
	if (FAILED(hr))
	{
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nelements = ARRAYSIZE(layout);
	hr = _device->CreateInputLayout(layout, nelements, g_VS, size, &_input_layout);
	if (FAILED(hr))
	{
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}
	_device_context->IASetInputLayout(_input_layout);

	size = ARRAYSIZE(g_PS);
	hr = _device->CreatePixelShader(g_PS, size, nullptr, &_pixel_shader);
	if (FAILED(hr))
	{
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}

	return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::core::output_manager::create_shared_surface(int32_t single_output, uint32_t * count_output, RECT * desktop_bounds)
{
	HRESULT hr;

	// Get DXGI resources
	IDXGIDevice * dxgi_device = nullptr;
	hr = _device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgi_device));
	if (FAILED(hr))
	{
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}

	IDXGIAdapter * dxgi_adapter = nullptr;
	hr = dxgi_device->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&dxgi_adapter));
	dxgi_device->Release();
	dxgi_device = nullptr;
	if (FAILED(hr))
	{
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}

	// Set initial values so that we always catch the right coordinates
	desktop_bounds->left = INT_MAX;
	desktop_bounds->right = INT_MIN;
	desktop_bounds->top = INT_MAX;
	desktop_bounds->bottom = INT_MIN;

	IDXGIOutput * dxgi_output = nullptr;

	// Figure out right dimensions for full size desktop texture and # of outputs to duplicate
	UINT cnt_output;
	if (single_output < 0)
	{
		hr = S_OK;
		for (cnt_output = 0; SUCCEEDED(hr); ++cnt_output)
		{
			if (dxgi_output)
			{
				dxgi_output->Release();
				dxgi_output = nullptr;
			}
			hr = dxgi_adapter->EnumOutputs(cnt_output, &dxgi_output);
			if (dxgi_output && (hr != DXGI_ERROR_NOT_FOUND))
			{
				DXGI_OUTPUT_DESC desktop_desc;
				dxgi_output->GetDesc(&desktop_desc);

				desktop_bounds->left = min(desktop_desc.DesktopCoordinates.left, desktop_bounds->left);
				desktop_bounds->top = min(desktop_desc.DesktopCoordinates.top, desktop_bounds->top);
				desktop_bounds->right = max(desktop_desc.DesktopCoordinates.right, desktop_bounds->right);
				desktop_bounds->bottom = max(desktop_desc.DesktopCoordinates.bottom, desktop_bounds->bottom);
			}
		}

		--cnt_output;
	}
	else
	{
		hr = dxgi_adapter->EnumOutputs(single_output, &dxgi_output);
		if (FAILED(hr))
		{
			dxgi_adapter->Release();
			dxgi_adapter = nullptr;
			return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
		}
		DXGI_OUTPUT_DESC desktop_desc;
		dxgi_output->GetDesc(&desktop_desc);
		*desktop_bounds = desktop_desc.DesktopCoordinates;

		dxgi_output->Release();
		dxgi_output = nullptr;

		cnt_output = 1;
	}

	dxgi_adapter->Release();
	dxgi_adapter = nullptr;

	// Set passed in output count variable
	*count_output = cnt_output;

	if (cnt_output == 0)
	{
		// We could not find any outputs, the system must be in a transition so return expected error
		// so we will attempt to recreate
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}

	// Create shared texture for all duplication threads to draw into
	D3D11_TEXTURE2D_DESC DeskTexD;
	RtlZeroMemory(&DeskTexD, sizeof(D3D11_TEXTURE2D_DESC));
	DeskTexD.Width = desktop_bounds->right - desktop_bounds->left;
	DeskTexD.Height = desktop_bounds->bottom - desktop_bounds->top;
	DeskTexD.MipLevels = 1;
	DeskTexD.ArraySize = 1;
	DeskTexD.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	DeskTexD.SampleDesc.Count = 1;
	DeskTexD.Usage = D3D11_USAGE_DEFAULT;
	DeskTexD.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	DeskTexD.CPUAccessFlags = 0;
	DeskTexD.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;

	hr = _device->CreateTexture2D(&DeskTexD, nullptr, &_shared_surface);
	if (FAILED(hr))
	{
		if (cnt_output != 1)
		{
			// If we are duplicating the complete desktop we try to create a single texture to hold the
			// complete desktop image and blit updates from the per output DDA interface.  The GPU can
			// always support a texture size of the maximum resolution of any single output but there is no
			// guarantee that it can support a texture size of the desktop.
			// The sample only use this large texture to display the desktop image in a single window using DX
			// we could revert back to using GDI to update the window in this failure case.
			return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
		}
		else
		{
			return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
		}
	}

	// Get keyed mutex
	hr = _shared_surface->QueryInterface(__uuidof(IDXGIKeyedMutex), reinterpret_cast<void**>(&_key_mutex));
	if (FAILED(hr))
	{
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}

	return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::core::output_manager::draw_frame(void)
{
	HRESULT hr;

	// If window was resized, resize swapchain
	if (_needs_resize)
	{
		int32_t status = resize_swap_chain();
		if (status != sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success)
		{
			return status;
		}
		_needs_resize = FALSE;
	}

	// Vertices for drawing whole texture
	sirius::library::video::source::d3d11::desktop::capturer::core::vertex_t vertices[NVERTICES] =
	{
		{ DirectX::XMFLOAT3(-1.0f, -1.0f, 0), DirectX::XMFLOAT2(0.0f, 1.0f) },
		{ DirectX::XMFLOAT3(-1.0f, 1.0f, 0), DirectX::XMFLOAT2(0.0f, 0.0f) },
		{ DirectX::XMFLOAT3(1.0f, -1.0f, 0), DirectX::XMFLOAT2(1.0f, 1.0f) },
		{ DirectX::XMFLOAT3(1.0f, -1.0f, 0), DirectX::XMFLOAT2(1.0f, 1.0f) },
		{ DirectX::XMFLOAT3(-1.0f, 1.0f, 0), DirectX::XMFLOAT2(0.0f, 0.0f) },
		{ DirectX::XMFLOAT3(1.0f, 1.0f, 0), DirectX::XMFLOAT2(1.0f, 0.0f) },
	};

	D3D11_TEXTURE2D_DESC frame_desc;
	_shared_surface->GetDesc(&frame_desc);

	D3D11_SHADER_RESOURCE_VIEW_DESC shader_desc;
	shader_desc.Format = frame_desc.Format;
	shader_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shader_desc.Texture2D.MostDetailedMip = frame_desc.MipLevels - 1;
	shader_desc.Texture2D.MipLevels = frame_desc.MipLevels;

	// Create new shader resource view
	ID3D11ShaderResourceView * shader_resource = nullptr;
	hr = _device->CreateShaderResourceView(_shared_surface, &shader_desc, &shader_resource);
	if (FAILED(hr))
	{
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}

	// Set resources
	UINT stride = sizeof(sirius::library::video::source::d3d11::desktop::capturer::core::vertex_t);
	UINT offset = 0;
	FLOAT blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
	_device_context->OMSetBlendState(nullptr, blend_factor, 0xffffffff);
	_device_context->OMSetRenderTargets(1, &_render_target_view, nullptr);
	_device_context->VSSetShader(_vertex_shader, nullptr, 0);
	_device_context->PSSetShader(_pixel_shader, nullptr, 0);
	_device_context->PSSetShaderResources(0, 1, &shader_resource);
	_device_context->PSSetSamplers(0, 1, &_sampler_linear);
	_device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3D11_BUFFER_DESC buffer_desc;
	memset(&buffer_desc, 0x00, sizeof(buffer_desc));
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.ByteWidth = sizeof(sirius::library::video::source::d3d11::desktop::capturer::core::vertex_t) * NVERTICES;
	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA init_data;
	memset(&init_data, 0x00, sizeof(init_data));
	init_data.pSysMem = vertices;

	ID3D11Buffer * vertex_buffer = nullptr;

	// Create vertex buffer
	hr = _device->CreateBuffer(&buffer_desc, &init_data, &vertex_buffer);
	if (FAILED(hr))
	{
		shader_resource->Release();
		shader_resource = nullptr;
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}
	_device_context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);

	// Draw textured quad onto render target
	_device_context->Draw(NVERTICES, 0);

	vertex_buffer->Release();
	vertex_buffer = nullptr;

	// Release shader resource
	shader_resource->Release();
	shader_resource = nullptr;

	return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::core::output_manager::draw_mouse(sirius::library::video::source::d3d11::desktop::capturer::core::capture_info_t * info)
{
	// Vars to be used
	ID3D11Texture2D * mouse_texture = nullptr;
	ID3D11ShaderResourceView * shader_resource = nullptr;
	ID3D11Buffer * vertex_buffer_mouse = nullptr;
	D3D11_SUBRESOURCE_DATA init_data;
	D3D11_TEXTURE2D_DESC desc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_desc;

	// Position will be changed based on mouse position
	sirius::library::video::source::d3d11::desktop::capturer::core::vertex_t vertices[NVERTICES] =
	{
		{ DirectX::XMFLOAT3(-1.0f, -1.0f, 0), DirectX::XMFLOAT2(0.0f, 1.0f) },
		{ DirectX::XMFLOAT3(-1.0f, 1.0f, 0), DirectX::XMFLOAT2(0.0f, 0.0f) },
		{ DirectX::XMFLOAT3(1.0f, -1.0f, 0), DirectX::XMFLOAT2(1.0f, 1.0f) },
		{ DirectX::XMFLOAT3(1.0f, -1.0f, 0), DirectX::XMFLOAT2(1.0f, 1.0f) },
		{ DirectX::XMFLOAT3(-1.0f, 1.0f, 0), DirectX::XMFLOAT2(0.0f, 0.0f) },
		{ DirectX::XMFLOAT3(1.0f, 1.0f, 0), DirectX::XMFLOAT2(1.0f, 0.0f) },
	};

	D3D11_TEXTURE2D_DESC full_desc;
	_shared_surface->GetDesc(&full_desc);
	INT desktop_width = full_desc.Width;
	INT desktop_height = full_desc.Height;

	// Center of desktop dimensions
	INT center_x = (desktop_width / 2);
	INT center_y = (desktop_height / 2);

	// Clipping adjusted coordinates / dimensions
	INT pwidth = 0;
	INT pheight = 0;
	INT pleft = 0;
	INT ptop = 0;

	// Buffer used if necessary (in case of monochrome or masked pointer)
	BYTE * init_buffer = nullptr;

	// Used for copying pixels
	D3D11_BOX box;
	box.front = 0;
	box.back = 1;

	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	// Set shader resource properties
	shader_resource_desc.Format = desc.Format;
	shader_resource_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shader_resource_desc.Texture2D.MostDetailedMip = desc.MipLevels - 1;
	shader_resource_desc.Texture2D.MipLevels = desc.MipLevels;

	switch (info->shape_info.Type)
	{
	case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_COLOR:
	{
		pleft = info->position.x;
		ptop = info->position.y;

		pwidth = static_cast<INT>(info->shape_info.Width);
		pheight = static_cast<INT>(info->shape_info.Height);

		break;
	}

	case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MONOCHROME:
	{
		process_mono_mask(true, info, &pwidth, &pheight, &pleft, &ptop, &init_buffer, &box);
		break;
	}

	case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MASKED_COLOR:
	{
		process_mono_mask(false, info, &pwidth, &pheight, &pleft, &ptop, &init_buffer, &box);
		break;
	}

	default:
		break;
	}

	// VERTEX creation
	vertices[0].position.x = (pleft - center_x) / (FLOAT)center_x;
	vertices[0].position.y = -1 * ((ptop + pheight) - center_y) / (FLOAT)center_y;
	vertices[1].position.x = (pleft - center_x) / (FLOAT)center_x;
	vertices[1].position.y = -1 * (ptop - center_y) / (FLOAT)center_y;
	vertices[2].position.x = ((pleft + pwidth) - center_x) / (FLOAT)center_x;
	vertices[2].position.y = -1 * ((ptop + pheight) - center_y) / (FLOAT)center_y;
	vertices[3].position.x = vertices[2].position.x;
	vertices[3].position.y = vertices[2].position.y;
	vertices[4].position.x = vertices[1].position.x;
	vertices[4].position.y = vertices[1].position.y;
	vertices[5].position.x = ((pleft + pwidth) - center_x) / (FLOAT)center_x;
	vertices[5].position.y = -1 * (ptop - center_y) / (FLOAT)center_y;

	// Set texture properties
	desc.Width = pwidth;
	desc.Height = pheight;

	// Set up init data
	init_data.pSysMem = (info->shape_info.Type == DXGI_OUTDUPL_POINTER_SHAPE_TYPE_COLOR) ? info->shape_buffer : init_buffer;
	init_data.SysMemPitch = (info->shape_info.Type == DXGI_OUTDUPL_POINTER_SHAPE_TYPE_COLOR) ? info->shape_info.Pitch : pwidth * BPP;
	init_data.SysMemSlicePitch = 0;

	// Create mouseshape as texture
	HRESULT hr = _device->CreateTexture2D(&desc, &init_data, &mouse_texture);
	if (FAILED(hr))
	{
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}

	// Create shader resource from texture
	hr = _device->CreateShaderResourceView(mouse_texture, &shader_resource_desc, &shader_resource);
	if (FAILED(hr))
	{
		mouse_texture->Release();
		mouse_texture = nullptr;
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}

	D3D11_BUFFER_DESC buffer_desc;
	memset(&buffer_desc, 0x00, sizeof(D3D11_BUFFER_DESC));
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.ByteWidth = sizeof(sirius::library::video::source::d3d11::desktop::capturer::core::vertex_t) * NVERTICES;
	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags = 0;

	memset(&init_data, 0x00, sizeof(D3D11_SUBRESOURCE_DATA));
	init_data.pSysMem = vertices;

	// Create vertex buffer
	hr = _device->CreateBuffer(&buffer_desc, &init_data, &vertex_buffer_mouse);
	if (FAILED(hr))
	{
		shader_resource->Release();
		shader_resource = nullptr;
		mouse_texture->Release();
		mouse_texture = nullptr;
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}

	// Set resources
	FLOAT blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
	UINT stride = sizeof(sirius::library::video::source::d3d11::desktop::capturer::core::vertex_t);
	UINT offset = 0;
	_device_context->IASetVertexBuffers(0, 1, &vertex_buffer_mouse, &stride, &offset);
	_device_context->OMSetBlendState(_blend_state, blend_factor, 0xFFFFFFFF);
	_device_context->OMSetRenderTargets(1, &_render_target_view, nullptr);
	_device_context->VSSetShader(_vertex_shader, nullptr, 0);
	_device_context->PSSetShader(_pixel_shader, nullptr, 0);
	_device_context->PSSetShaderResources(0, 1, &shader_resource);
	_device_context->PSSetSamplers(0, 1, &_sampler_linear);

	// Draw
	//_device_context->Draw(NVERTICES, 0);

	// Clean
	if (vertex_buffer_mouse)
	{
		vertex_buffer_mouse->Release();
		vertex_buffer_mouse = nullptr;
	}
	if (shader_resource)
	{
		shader_resource->Release();
		shader_resource = nullptr;
	}
	if (mouse_texture)
	{
		mouse_texture->Release();
		mouse_texture = nullptr;
	}
	if (init_buffer)
	{
		delete[] init_buffer;
		init_buffer = nullptr;
	}

	return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::core::output_manager::resize_swap_chain(void)
{
	if (_render_target_view)
	{
		_render_target_view->Release();
		_render_target_view = nullptr;
	}

	RECT rect;
	GetClientRect(_hwnd, &rect);
	uint32_t width = rect.right - rect.left;
	uint32_t height = rect.bottom - rect.top;

	// Resize swapchain
	DXGI_SWAP_CHAIN_DESC swap_chain_desc;
	_swap_chain->GetDesc(&swap_chain_desc);
	HRESULT hr = _swap_chain->ResizeBuffers(swap_chain_desc.BufferCount, width, height, swap_chain_desc.BufferDesc.Format, swap_chain_desc.Flags);
	if (FAILED(hr))
	{
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}

	// Make new render target view
	int32_t status = make_render_target_view();
	if (status != sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success)
	{
		return status;
	}

	// Set new viewport
	set_view_port(width, height);

	return status;
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::core::output_manager::convert(ID3D11Texture2D * input, ID3D11Resource * output)
{
	int32_t status = sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	HRESULT hr = E_FAIL;

	ATL::CComPtr<ID3D11VideoProcessorInputView> input_view = NULL;
	ATL::CComPtr<ID3D11VideoProcessorOutputView> output_view = NULL;
	ATL::CComPtr<ID3D11VideoContext> video_context = NULL;
	do
	{
		hr = _d3d11_device_context->QueryInterface(__uuidof(ID3D11VideoContext), (void**)&video_context);
		if (FAILED(hr))
			break;

		D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC output_view_desc;
		memset(&output_view_desc, 0x00, sizeof(D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC));
		output_view_desc.ViewDimension = D3D11_VPOV_DIMENSION_TEXTURE2D;
		output_view_desc.Texture2D.MipSlice = 0;
		output_view_desc.Texture2DArray.MipSlice = 0;
		output_view_desc.Texture2DArray.FirstArraySlice = 0;
		hr = _d3d11_video_device->CreateVideoProcessorOutputView(output, _d3d11_video_processor_enum, &output_view_desc, &output_view);
		if (FAILED(hr))
			break;

		D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC input_view_desc;
		memset(&input_view_desc, 0x00, sizeof(D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC));
		input_view_desc.FourCC = 0;
		input_view_desc.ViewDimension = D3D11_VPIV_DIMENSION_TEXTURE2D;
		input_view_desc.Texture2D.MipSlice = 0;
		input_view_desc.Texture2D.ArraySlice = 0;
		hr = _d3d11_video_device->CreateVideoProcessorInputView(input, _d3d11_video_processor_enum, &input_view_desc, &input_view);
		if (FAILED(hr))
			break;

		video_context->VideoProcessorSetStreamFrameFormat(_d3d11_video_processor, 0, D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE);
		video_context->VideoProcessorSetStreamOutputRate(_d3d11_video_processor, 0, D3D11_VIDEO_PROCESSOR_OUTPUT_RATE_NORMAL, TRUE, NULL); // Output rate (repeat frames)

		RECT SRect = { 0, 0, _crop_width, _crop_height };
		RECT DRect = { 0, 0, _crop_width, _crop_height };
		video_context->VideoProcessorSetStreamSourceRect(_d3d11_video_processor, 0, TRUE, &SRect); // Source rect
		video_context->VideoProcessorSetStreamDestRect(_d3d11_video_processor, 0, TRUE, &DRect); // Stream dest rect
		video_context->VideoProcessorSetOutputTargetRect(_d3d11_video_processor, TRUE, &DRect);

		D3D11_VIDEO_PROCESSOR_COLOR_SPACE cs = {};
		cs.YCbCr_xvYCC = 1;
		video_context->VideoProcessorSetStreamColorSpace(_d3d11_video_processor, 0, &cs);
		video_context->VideoProcessorSetOutputColorSpace(_d3d11_video_processor, &cs); // Output color space

		D3D11_VIDEO_COLOR bgcolor = {};
		bgcolor.RGBA.A = 1.0F;
		bgcolor.RGBA.R = 1.0F * static_cast<float>(GetRValue(0)) / 255.0F;
		bgcolor.RGBA.G = 1.0F * static_cast<float>(GetGValue(0)) / 255.0F;
		bgcolor.RGBA.B = 1.0F * static_cast<float>(GetBValue(0)) / 255.0F;
		video_context->VideoProcessorSetOutputBackgroundColor(_d3d11_video_processor, TRUE, &bgcolor);

		D3D11_VIDEO_PROCESSOR_STREAM d3d11_stream_data;
		ZeroMemory(&d3d11_stream_data, sizeof(D3D11_VIDEO_PROCESSOR_STREAM));
		d3d11_stream_data.Enable = TRUE;
		d3d11_stream_data.OutputIndex = 0;
		d3d11_stream_data.InputFrameOrField = 0;
		d3d11_stream_data.PastFrames = 0;
		d3d11_stream_data.FutureFrames = 0;
		d3d11_stream_data.ppPastSurfaces = NULL;
		d3d11_stream_data.ppFutureSurfaces = NULL;
		d3d11_stream_data.pInputSurface = input_view;
		d3d11_stream_data.ppPastSurfacesRight = NULL;
		d3d11_stream_data.ppFutureSurfacesRight = NULL;

		hr = video_context->VideoProcessorBlt(_d3d11_video_processor, output_view, 0, 1, &d3d11_stream_data);
		if (FAILED(hr))
		{
			DWORD err = ::GetLastError();
			break;
		}

		status = sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;

	} while (0);

	return status;
}
