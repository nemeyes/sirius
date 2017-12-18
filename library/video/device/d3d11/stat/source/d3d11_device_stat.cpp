#include "d3d11_device_stat.h"
#include <sirius_stringhelper.h>

void sirius::library::video::device::d3d11::stat::core::retreieve(sirius::library::video::device::d3d11::stat::desc_t * adapters, int32_t capacity, int32_t & count, int32_t option)
{
	IDXGIFactory3 * dxgi_factory = NULL;
	IDXGIAdapter1 * dxgi_adapter = NULL;
	HRESULT result = CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**)&dxgi_factory);
	if (FAILED(result))
		return;

	int32_t index = 0;
	for (int32_t i = 0; (dxgi_factory->EnumAdapters1(i, &dxgi_adapter) != DXGI_ERROR_NOT_FOUND) && index<capacity; i++)
	{
		DXGI_ADAPTER_DESC1 desc;
		dxgi_adapter->GetDesc1(&desc);

		if (option == sirius::library::video::device::d3d11::stat::option_t::hw)
		{
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				continue;
		}
		else if (option == sirius::library::video::device::d3d11::stat::option_t::sw)
		{
			if (!(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE))
				continue;
		}

		char * description = nullptr;
		sirius::stringhelper::convert_wide2multibyte(desc.Description, &description);
		if (description)
		{
			strncpy_s(adapters[index].description, description, sizeof(adapters[index].description));
			free(description);
			description = nullptr;
		}
		adapters[index].adaptorIndex = i;
		adapters[index].vendorId = desc.VendorId;
		adapters[index].subsysId = desc.SubSysId;
		adapters[index].deviceId = desc.DeviceId;
		adapters[index].revision = desc.Revision;

		IDXGIOutput * output = NULL;
		if (DXGI_ERROR_NOT_FOUND != dxgi_adapter->EnumOutputs(0, &output))
		{
			DXGI_OUTPUT_DESC output_desc;
			HRESULT hr = output->GetDesc(&output_desc);
			adapters[index].coordLeft = output_desc.DesktopCoordinates.left;
			adapters[index].coordTop = output_desc.DesktopCoordinates.top;
			adapters[index].coordRight = output_desc.DesktopCoordinates.right;
			adapters[index].coordBottom = output_desc.DesktopCoordinates.bottom;

			SIRIUS_SAFE_RELEASE(output);
		}
		index++;

		if (dxgi_adapter)
			dxgi_adapter->Release();
		dxgi_adapter = NULL;
	}
	count = index;

	if (dxgi_factory)
		dxgi_factory->Release();
	dxgi_factory = NULL;
}