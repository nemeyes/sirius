#include "d3d11_duplication_manager.h"

sirius::library::video::source::d3d11::desktop::capturer::core::duplication_manager::duplication_manager(void)
	: _desktop_duplication(nullptr)
	, _acquired_desktop_image(nullptr)
	, _meta_data_buffer(nullptr)
	, _meta_data_size(0)
	, _output_number(0)
	, _device(nullptr)
{
	memset(&_output_desc, 0x00, sizeof(_output_desc));
}

sirius::library::video::source::d3d11::desktop::capturer::core::duplication_manager::~duplication_manager(void)
{

}

int32_t sirius::library::video::source::d3d11::desktop::capturer::core::duplication_manager::initialize(ID3D11Device * device, int32_t output)
{
	_output_number = output;

	_device = device;
	_device->AddRef();

	IDXGIDevice * dxgi_device = nullptr;
	HRESULT hr = _device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgi_device));
	if (FAILED(hr))
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;

	IDXGIAdapter * dxgi_adapter = nullptr;
	hr = dxgi_device->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&dxgi_adapter));
	dxgi_device->Release();
	dxgi_device = nullptr;
	if (FAILED(hr))
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;

	IDXGIOutput * dxgi_output = nullptr;
	hr = dxgi_adapter->EnumOutputs(_output_number, &dxgi_output);
	DXGI_ADAPTER_DESC desc;
	dxgi_adapter->GetDesc(&desc);
	dxgi_adapter->Release();
	dxgi_adapter = nullptr;
	if (FAILED(hr))
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;

	dxgi_output->GetDesc(&_output_desc);

	IDXGIOutput1 * dxgi_output1 = nullptr;
	hr = dxgi_output->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void**>(&dxgi_output1));
	dxgi_output->Release();
	dxgi_output = nullptr;
	if (FAILED(hr))
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;

	hr = dxgi_output1->DuplicateOutput(_device, &_desktop_duplication);
	dxgi_output1->Release();
	dxgi_output1 = nullptr;
	if (FAILED(hr))
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;

	return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::core::duplication_manager::release(void)
{
	if (_desktop_duplication)
	{
		_desktop_duplication->Release();
		_desktop_duplication = nullptr;
	}

	if (_acquired_desktop_image)
	{
		_acquired_desktop_image->Release();
		_acquired_desktop_image = nullptr;
	}

	if (_meta_data_buffer)
	{
		delete[] _meta_data_buffer;
		_meta_data_buffer = nullptr;
	}

	if (_device)
	{
		_device->Release();
		_device = nullptr;
	}

	return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::core::duplication_manager::get_mouse(sirius::library::video::source::d3d11::desktop::capturer::core::capture_info_t * info, DXGI_OUTDUPL_FRAME_INFO * frame_info, int32_t offset_x, int32_t offset_y)
{
	if (frame_info->LastMouseUpdateTime.QuadPart == 0)
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;

	bool update_position = true;

	if (!frame_info->PointerPosition.Visible && (info->who_updated_position_last != _output_number))
		update_position = false;

	if (!frame_info->PointerPosition.Visible && info->visible &&
		(info->who_updated_position_last != _output_number) &&
		(info->last_timestamp.QuadPart > frame_info->LastMouseUpdateTime.QuadPart))
		update_position = false;

	if (update_position)
	{
		info->position.x = frame_info->PointerPosition.Position.x + _output_desc.DesktopCoordinates.left - offset_x;
		info->position.y = frame_info->PointerPosition.Position.y + _output_desc.DesktopCoordinates.top - offset_y;
		info->who_updated_position_last = _output_number;
		info->last_timestamp = frame_info->LastMouseUpdateTime;
		info->visible = frame_info->PointerPosition.Visible != 0;
	}

	if (frame_info->PointerShapeBufferSize == 0)
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;

	if (frame_info->PointerShapeBufferSize > info->buffer_size)
	{
		if (info->shape_buffer)
		{
			delete[] info->shape_buffer;
			info->shape_buffer = nullptr;
		}

		info->shape_buffer = new (std::nothrow) unsigned char[frame_info->PointerShapeBufferSize];
		if (!info->shape_buffer)
		{
			info->buffer_size = 0;
			return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
		}

		info->buffer_size = frame_info->PointerShapeBufferSize;
	}

	uint32_t required_buffer_size;
	HRESULT hr = _desktop_duplication->GetFramePointerShape(frame_info->PointerShapeBufferSize, reinterpret_cast<VOID*>(info->shape_buffer), &required_buffer_size, &(info->shape_info));
	if (FAILED(hr))
	{
		delete[] info->shape_buffer;
		info->shape_buffer = nullptr;
		info->buffer_size = 0;
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}

	return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::core::duplication_manager::acquire_frame(sirius::library::video::source::d3d11::desktop::capturer::core::frame_data_t * data, bool * timeout)
{
	IDXGIResource * desktop_resource = nullptr;
	DXGI_OUTDUPL_FRAME_INFO frame_info;

	//get new frame
	HRESULT hr = _desktop_duplication->AcquireNextFrame(500, &frame_info, &desktop_resource);
	if (hr == DXGI_ERROR_WAIT_TIMEOUT)
	{
		*timeout = true;
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;
	}

	*timeout = false;
	if (FAILED(hr))
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;

	if (_acquired_desktop_image)
	{
		_acquired_desktop_image->Release();
		_acquired_desktop_image = nullptr;
	}

	hr = desktop_resource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&_acquired_desktop_image));
	desktop_resource->Release();
	desktop_resource = nullptr;
	if (FAILED(hr))
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;

	if (frame_info.TotalMetadataBufferSize)
	{
		if (frame_info.TotalMetadataBufferSize > _meta_data_size)
		{
			if (_meta_data_buffer)
			{
				delete[] _meta_data_buffer;
				_meta_data_buffer = nullptr;
			}

			_meta_data_buffer = new (std::nothrow) unsigned char[frame_info.TotalMetadataBufferSize];
			if (!_meta_data_buffer)
			{
				_meta_data_size = 0;
				data->move_count = 0;
				data->dirty_count = 0;
				return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
			}
			_meta_data_size = frame_info.TotalMetadataBufferSize;
		}

		uint32_t buffer_size = frame_info.TotalMetadataBufferSize;
		hr = _desktop_duplication->GetFrameMoveRects(buffer_size, reinterpret_cast<DXGI_OUTDUPL_MOVE_RECT*>(_meta_data_buffer), &buffer_size);
		if (FAILED(hr))
		{
			data->move_count = 0;
			data->dirty_count = 0;
			return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
		}

		data->move_count = buffer_size / sizeof(DXGI_OUTDUPL_MOVE_RECT);
		unsigned char * dirty_rects = _meta_data_buffer + buffer_size;
		buffer_size = frame_info.TotalMetadataBufferSize - buffer_size;

		// get dirty rectangles
		hr = _desktop_duplication->GetFrameDirtyRects(buffer_size, reinterpret_cast<RECT*>(dirty_rects), &buffer_size);
		if (FAILED(hr))
		{
			data->move_count = 0;
			data->dirty_count = 0;
			return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
		}

		data->dirty_count = buffer_size / sizeof(RECT);
		data->meta_data = _meta_data_buffer;
	}

	data->frame = _acquired_desktop_image;
	data->frame_info = frame_info;

	return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::core::duplication_manager::release_frame(void)
{
	HRESULT hr = _desktop_duplication->ReleaseFrame();
	if (FAILED(hr))
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;

	if (_acquired_desktop_image)
	{
		_acquired_desktop_image->Release();
		_acquired_desktop_image = nullptr;
	}

	return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::core::duplication_manager::get_output_description(DXGI_OUTPUT_DESC * desc)
{
	*desc = _output_desc;

	return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;
}