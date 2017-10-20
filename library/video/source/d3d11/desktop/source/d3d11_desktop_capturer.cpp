#include "d3d11_desktop_capturer.h"

#include "d3d11_duplication_manager.h"
#include "d3d11_display_manager.h"
#include "d3d11_output_manager.h"

const D3D_DRIVER_TYPE	sirius::library::video::source::d3d11::desktop::capturer::core::DRIVER_TYPES[] = { D3D_DRIVER_TYPE_HARDWARE };
const D3D_FEATURE_LEVEL	sirius::library::video::source::d3d11::desktop::capturer::core::FEATURE_LEVELS[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_9_1 };

const int32_t sirius::library::video::source::d3d11::desktop::capturer::core::COUNT_OF_DRIVER_TYPES = ARRAYSIZE(sirius::library::video::source::d3d11::desktop::capturer::core::DRIVER_TYPES);
const int32_t sirius::library::video::source::d3d11::desktop::capturer::core::COUNT_OF_FEATURE_LEVELS = ARRAYSIZE(sirius::library::video::source::d3d11::desktop::capturer::core::FEATURE_LEVELS);

sirius::library::video::source::d3d11::desktop::capturer::core::core(void)
	: _threads_count(0)
	, _threads(nullptr)
	, _output_mgr(nullptr)
	, _run(FALSE)
{
	::memset(&_info, 0x00, sizeof(_info));
	_output_mgr = new sirius::library::video::source::d3d11::desktop::capturer::core::output_manager();
}

sirius::library::video::source::d3d11::desktop::capturer::core::~core(void)
{
	if (_output_mgr)
	{
		delete _output_mgr;
	}
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::core::initialize(sirius::library::video::source::d3d11::desktop::capturer::context_t * context)
{
	int32_t status = sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;

	uint32_t count_output = 0;
	RECT desktop_bounds;
	status = _output_mgr->initialize(context, &count_output, &desktop_bounds);
	if (status != sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success)
	{
		return status;
	}

	HANDLE shared_hanle = _output_mgr->shared_handle();
	if (!shared_hanle)
	{
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}

	_threads_count = count_output;//context->threads_count;
	_threads = new (std::nothrow) HANDLE[_threads_count];
	_threads_data = new (std::nothrow) sirius::library::video::source::d3d11::desktop::capturer::core::thread_context_t;

	if (!_threads || !_threads_data)
		return status;

	for (int32_t i = 0; i < _threads_count; i++)
	{
		_threads_data[i].unexpected_error_event = context->unexpected_error_event;
		_threads_data[i].expected_error_event = context->expected_error_event;
		_threads_data[i].output = (context->single_output<0) ? i : context->single_output;
		_threads_data[i].texure_shared_handle = shared_hanle;
		_threads_data[i].offset_x = context->rect.left;
		_threads_data[i].offset_y = context->rect.top;
		_threads_data[i].info = &_info;
		_threads_data[i].self = this;

		memset(&_threads_data[i].resource, 0x00, sizeof(sirius::library::video::source::d3d11::desktop::capturer::core::d3d11_resources_t));
		status = initialize_d3d11(&_threads_data[i].resource);
		if (status != sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success)
			return status;
	}

	/*
	_terminate_threads_event = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	if (!_terminate_threads_event)
	{
		status = sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}
	*/
	return status;
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::core::release(void)
{
	if (_info.shape_buffer)
	{
		delete[] _info.shape_buffer;
		_info.shape_buffer = nullptr;
	}
	memset(&_info, 0x00, sizeof(_info));

	if (_threads)
	{
		for (int32_t i = 0; i < _threads_count; i++)
		{
			if (_threads[i])
				::CloseHandle(_threads[i]);
		}
		delete[] _threads;
		_threads = nullptr;
	}

	if (_threads_data)
	{
		for (int32_t i = 0; i < _threads_count; i++)
		{
			release_d3d11(&_threads_data[i].resource);
		}
		delete[] _threads_data;
		_threads_data = nullptr;
	}

	_threads_count = 0;

	_output_mgr->release();
	//::CloseHandle(_terminate_threads_event);
	return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::core::start(void)
{
	if(_threads_count > 0)
		_run = TRUE;

	for (int32_t i = 0; i < _threads_count; i++)
	{
		uint32_t threadid;
		_threads[i] = (HANDLE)::_beginthreadex(nullptr, 0, sirius::library::video::source::d3d11::desktop::capturer::core::process_cb, &_threads_data[i], 0, &threadid);
		if (_threads[i] == nullptr)
			return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}

	return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::core::stop(void)
{
	if (_threads_count > 0)
	{
		_run = FALSE;
		::WaitForMultipleObjectsEx(_threads_count, _threads, TRUE, INFINITE, FALSE);
	}

	return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::core::update_application_window(bool * occluded)
{
	return _output_mgr->update_application_window(&_info, occluded);
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::core::initialize_d3d11(sirius::library::video::source::d3d11::desktop::capturer::core::d3d11_resources_t * resource)
{
	uint32_t cnt_driver_types	= COUNT_OF_DRIVER_TYPES;
	uint32_t cnt_feature_levels	= COUNT_OF_FEATURE_LEVELS;

	HRESULT hr = E_FAIL;
	D3D_FEATURE_LEVEL feature_level;
	UINT create_device_flags = 0;
#if defined(_DEBUG)
	create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	for (uint32_t i = 0; i < cnt_driver_types; i++)
	{
		hr = D3D11CreateDevice(NULL, DRIVER_TYPES[i], NULL, create_device_flags, FEATURE_LEVELS, cnt_feature_levels, D3D11_SDK_VERSION, &resource->device, &feature_level, &resource->context);
		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;


	//vertex shader
	int32_t vs_size = ARRAYSIZE(g_VS);
	hr = resource->device->CreateVertexShader(g_VS, vs_size, nullptr, &resource->vertex_shader);
	if (FAILED(hr))
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;

	D3D11_INPUT_ELEMENT_DESC layout[] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	int32_t cnt_layout = ARRAYSIZE(layout);
	hr = resource->device->CreateInputLayout(layout, cnt_layout, g_VS, vs_size, &resource->input_layout);
	if (FAILED(hr))
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;

	resource->context->IASetInputLayout(resource->input_layout);

	//pixel shader
	int32_t ps_size = ARRAYSIZE(g_PS);
	hr = resource->device->CreatePixelShader(g_PS, ps_size, nullptr, &resource->pixel_shader);
	if (FAILED(hr))
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;

	D3D11_SAMPLER_DESC sampler_desc;
	memset(&sampler_desc, 0x00, sizeof(sampler_desc));
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampler_desc.MinLOD = 0;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = resource->device->CreateSamplerState(&sampler_desc, &resource->sampler_linear);
	if (FAILED(hr))
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;

	return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;

}

int32_t sirius::library::video::source::d3d11::desktop::capturer::core::release_d3d11(sirius::library::video::source::d3d11::desktop::capturer::core::d3d11_resources_t * resource)
{
	if (resource->device)
	{
		resource->device->Release();
		resource->device = nullptr;
	}

	if (resource->context)
	{
		resource->context->Release();
		resource->context = nullptr;
	}

	if (resource->vertex_shader)
	{
		resource->vertex_shader->Release();
		resource->vertex_shader = nullptr;
	}

	if (resource->pixel_shader)
	{
		resource->pixel_shader->Release();
		resource->pixel_shader = nullptr;
	}

	if (resource->input_layout)
	{
		resource->input_layout->Release();
		resource->input_layout = nullptr;
	}

	if (resource->sampler_linear)
	{
		resource->sampler_linear->Release();
		resource->sampler_linear = nullptr;
	}

	return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;
}

unsigned sirius::library::video::source::d3d11::desktop::capturer::core::process_cb(void * p)
{
	sirius::library::video::source::d3d11::desktop::capturer::core::display_manager disp_mgr;
	sirius::library::video::source::d3d11::desktop::capturer::core::duplication_manager dup_mgr;

	ID3D11Texture2D * shared_surface = nullptr;
	IDXGIKeyedMutex * key_mutex = nullptr;

	sirius::library::video::source::d3d11::desktop::capturer::core::thread_context_t * tctx = reinterpret_cast<sirius::library::video::source::d3d11::desktop::capturer::core::thread_context_t*>(p);

	int32_t status = sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;
	HDESK current_desktop = nullptr;
	current_desktop = OpenInputDesktop(0, FALSE, GENERIC_ALL);
	if (!current_desktop)
	{
		::SetEvent(tctx->expected_error_event);
		status = sirius::library::video::source::d3d11::desktop::capturer::err_code_t::expected;
		goto exit;
	}

	bool desktop_attached = ::SetThreadDesktop(current_desktop) != 0;
	::CloseDesktop(current_desktop);
	current_desktop = nullptr;
	if (!desktop_attached)
	{
		status = sirius::library::video::source::d3d11::desktop::capturer::err_code_t::expected;
		goto exit;
	}

	disp_mgr.initialize(&tctx->resource);

	HRESULT hr = tctx->resource.device->OpenSharedResource(tctx->texure_shared_handle, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&shared_surface));
	if (FAILED(hr))
	{
		status = sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
		goto exit;
	}

	hr = shared_surface->QueryInterface(__uuidof(IDXGIKeyedMutex), reinterpret_cast<void**>(&key_mutex));
	if (FAILED(hr))
	{
		status = sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
		goto exit;
	}

	status = dup_mgr.initialize(tctx->resource.device, tctx->output);
	if (status != sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success)
	{
		goto exit;
	}

	DXGI_OUTPUT_DESC desktop_desc;
	memset(&desktop_desc, 0x00, sizeof(desktop_desc));
	dup_mgr.get_output_description(&desktop_desc);

	bool wait_to_process_current_frame = false;
	sirius::library::video::source::d3d11::desktop::capturer::core::frame_data_t current_data;

	//while ((::WaitForSingleObjectEx(tctx->self->_terminate_threads_event, 0, FALSE) == WAIT_TIMEOUT))
	while(tctx->self->_run)
	{
		if (!wait_to_process_current_frame)
		{
			bool timeout;
			status = dup_mgr.acquire_frame(&current_data, &timeout);
			if (status != sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success)
			{
				break;
			}

			if (timeout)
			{
				continue;
			}
		}

		hr = key_mutex->AcquireSync(0, 1000);
		if (hr == static_cast<HRESULT>(WAIT_TIMEOUT))
		{
			wait_to_process_current_frame = true;
			continue;
		}
		else if (FAILED(hr))
		{
			status = sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
			dup_mgr.release_frame();
			break;
		}

		wait_to_process_current_frame = false;

		status = dup_mgr.get_mouse(tctx->info, &(current_data.frame_info), tctx->offset_x, tctx->offset_y);
		if (status != sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success)
		{
			dup_mgr.release_frame();
			key_mutex->ReleaseSync(1);
			break;
		}

		status = disp_mgr.process(&current_data, shared_surface, tctx->offset_x, tctx->offset_y, &desktop_desc);
		if (status != sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success)
		{
			dup_mgr.release_frame();
			key_mutex->ReleaseSync(1);
			break;
		}

		hr = key_mutex->ReleaseSync(1);
		if (FAILED(hr))
		{
			status = sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
			dup_mgr.release_frame();
			break;
		}

		status = dup_mgr.release_frame();
		if (status != sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success)
		{
			break;
		}
	}

exit:
	if (status != sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success)
	{
		if (status == sirius::library::video::source::d3d11::desktop::capturer::err_code_t::expected)
		{
			::SetEvent(tctx->expected_error_event);
		}
		else
		{
			::SetEvent(tctx->unexpected_error_event);
		}
	}

	if (shared_surface)
	{
		shared_surface->Release();
		shared_surface = nullptr;
	}

	if (key_mutex)
	{
		key_mutex->Release();
		key_mutex = nullptr;
	}
	return 0;
}