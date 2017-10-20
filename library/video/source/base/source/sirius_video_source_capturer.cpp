#include "sirius_video_source_capturer.h"
#include <atlbase.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>

sirius::library::video::source::capturer::capturer(void)
{

}

sirius::library::video::source::capturer::~capturer(void)
{

}


int32_t sirius::library::video::source::capturer::retrieve_gpu_adapters(sirius::library::video::source::capturer::gpu_descriton_t * adapters, int32_t capacity, int32_t & count)
{
	IDXGIFactory3 * dxgi_factory = NULL;
	IDXGIAdapter1 * dxgi_adapter = NULL;
	HRESULT result = CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**)&dxgi_factory);
	if (FAILED(result))
		return sirius::library::video::source::capturer::err_code_t::fail;

	int32_t index = 0;
	for (int32_t i = 0; (dxgi_factory->EnumAdapters1(i, &dxgi_adapter) != DXGI_ERROR_NOT_FOUND) && index<capacity; i++)
	{
		DXGI_ADAPTER_DESC1 desc;
		dxgi_adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;

		wcsncpy_s(adapters[index].description, desc.Description, sizeof(adapters[index].description));
		adapters[index].adaptorIndex = i;
		adapters[index].vendorId = desc.VendorId;
		adapters[index].subsysId = desc.SubSysId;
		adapters[index].deviceId = desc.DeviceId;
		adapters[index].revision = desc.Revision;

		ATL::CComPtr<IDXGIOutput> output = NULL;
		if (DXGI_ERROR_NOT_FOUND != dxgi_adapter->EnumOutputs(0, &output))
		{
			DXGI_OUTPUT_DESC output_desc;
			HRESULT hr = output->GetDesc(&output_desc);
		}
		index++;

		if (dxgi_adapter)
			dxgi_adapter->Release();
		dxgi_adapter = NULL;
	}
	count = index + 1;

	if (dxgi_factory)
		dxgi_factory->Release();
	dxgi_factory = NULL;

	return sirius::library::video::source::capturer::err_code_t::success;
}

int32_t sirius::library::video::source::capturer::retrieve_gpu_outputs(sirius::library::video::source::capturer::gpu_descriton_t * adapters, int32_t & output)
{
	IDXGIFactory3 * dxgi_factory = NULL;
	IDXGIAdapter1 * dxgi_adapter = NULL;
	HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**)&dxgi_factory);
	if (FAILED(hr))
		return sirius::library::video::source::capturer::err_code_t::fail;

	output = 0;
	for (int32_t i = 0; (dxgi_factory->EnumAdapters1(i, &dxgi_adapter) != DXGI_ERROR_NOT_FOUND); i++)
	{
		DXGI_ADAPTER_DESC1 desc;
		dxgi_adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;

		if (adapters->adaptorIndex == i)
			break;

		if (dxgi_adapter)
			dxgi_adapter->Release();
		dxgi_adapter = NULL;
	}

	if (dxgi_adapter)
	{
		IDXGIOutput * dxgi_output = NULL;
		hr = S_OK;
		output = 0;
		for (int32_t i = 0; SUCCEEDED(hr); ++i)
		{
			if (dxgi_output)
			{
				dxgi_output->Release();
				dxgi_output = NULL;
			}
			hr = dxgi_adapter->EnumOutputs(i, &dxgi_output);
			output++;
		}

		dxgi_adapter->Release();
		dxgi_adapter = NULL;
	}

	if (dxgi_factory)
		dxgi_factory->Release();
	dxgi_factory = NULL;

	return sirius::library::video::source::capturer::err_code_t::success;
}

int32_t sirius::library::video::source::capturer::initialize(sirius::library::video::source::capturer::context_t * context)
{
	return sirius::library::video::source::capturer::err_code_t::not_implemented;
}

int32_t sirius::library::video::source::capturer::release(void)
{
	return sirius::library::video::source::capturer::err_code_t::not_implemented;
}

int32_t sirius::library::video::source::capturer::start(void)
{
	return sirius::library::video::source::capturer::err_code_t::not_implemented;
}

int32_t sirius::library::video::source::capturer::stop(void)
{
	return sirius::library::video::source::capturer::err_code_t::not_implemented;
}

int32_t sirius::library::video::source::capturer::pause(void)
{
	return sirius::library::video::source::capturer::err_code_t::not_implemented;
}