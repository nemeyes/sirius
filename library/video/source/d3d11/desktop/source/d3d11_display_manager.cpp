#include "d3d11_display_manager.h"

sirius::library::video::source::d3d11::desktop::capturer::core::display_manager::display_manager(void)
	: _device(nullptr)
	, _device_context(nullptr)
	, _move_surface(nullptr)
	, _vertex_shader(nullptr)
	, _pixel_shader(nullptr)
	, _input_layout(nullptr)
	, _render_target_view(nullptr)
	, _sampler_linear(nullptr)
	, _dirty_vertex_buffer_alloc(nullptr)
	, _dirty_vertex_buffer_alloc_size(0)
{

}

sirius::library::video::source::d3d11::desktop::capturer::core::display_manager::~display_manager(void)
{
	release();

	if (_dirty_vertex_buffer_alloc)
	{
		delete[] _dirty_vertex_buffer_alloc;
		_dirty_vertex_buffer_alloc = nullptr;
	}
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::core::display_manager::initialize(sirius::library::video::source::d3d11::desktop::capturer::core::d3d11_resources_t * resource)
{
	_device = resource->device;
	_device_context = resource->context;
	_vertex_shader = resource->vertex_shader;
	_pixel_shader = resource->pixel_shader;
	_input_layout = resource->input_layout;
	_sampler_linear = resource->sampler_linear;

	_device->AddRef();
	_device_context->AddRef();
	_vertex_shader->AddRef();
	_pixel_shader->AddRef();
	_input_layout->AddRef();
	_sampler_linear->AddRef();

	return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::core::display_manager::release(void)
{
	if (_device_context)
	{
		_device_context->Release();
		_device_context = nullptr;
	}

	if (_device)
	{
		_device->Release();
		_device = nullptr;
	}

	if (_move_surface)
	{
		_move_surface->Release();
		_move_surface = nullptr;
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

	if (_sampler_linear)
	{
		_sampler_linear->Release();
		_sampler_linear = nullptr;
	}

	if (_render_target_view)
	{
		_render_target_view->Release();
		_render_target_view = nullptr;
	}

	return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::core::display_manager::process(sirius::library::video::source::d3d11::desktop::capturer::core::frame_data_t * data, ID3D11Texture2D * shared_surface, int32_t offset_x, int32_t offset_y, DXGI_OUTPUT_DESC * desktop_desc)
{
	int32_t status = sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;

	if (data->frame_info.TotalMetadataBufferSize)
	{
		D3D11_TEXTURE2D_DESC desc;
		data->frame->GetDesc(&desc);
		if (data->move_count)
		{
			status = copy_move(shared_surface, reinterpret_cast<DXGI_OUTDUPL_MOVE_RECT*>(data->meta_data), data->move_count, offset_x, offset_y, desktop_desc, desc.Width, desc.Height);
			if (status != sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success)
				return status;
		}

		if (data->dirty_count)
		{
			status = copy_dirty(data->frame, shared_surface, reinterpret_cast<RECT*>(data->meta_data + (data->move_count * sizeof(DXGI_OUTDUPL_MOVE_RECT))), data->dirty_count, offset_x, offset_y, desktop_desc);
		}
	}

	return status;
}

ID3D11Device * sirius::library::video::source::d3d11::desktop::capturer::core::display_manager::device(void)
{
	return _device;
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::core::display_manager::set_move_rect(RECT * source_rect, RECT * dest_rect, DXGI_OUTPUT_DESC * desktop_desc, DXGI_OUTDUPL_MOVE_RECT * move_rect, int32_t texture_width, int32_t texture_height)
{
	switch (desktop_desc->Rotation)
	{
		case DXGI_MODE_ROTATION_UNSPECIFIED:
		case DXGI_MODE_ROTATION_IDENTITY:
		{
			source_rect->left = move_rect->SourcePoint.x;
			source_rect->top = move_rect->SourcePoint.y;
			source_rect->right = move_rect->SourcePoint.x + move_rect->DestinationRect.right - move_rect->DestinationRect.left;
			source_rect->bottom = move_rect->SourcePoint.y + move_rect->DestinationRect.bottom - move_rect->DestinationRect.top;
		
			*dest_rect = move_rect->DestinationRect;
			break;
		}
		case DXGI_MODE_ROTATION_ROTATE90:
		{
			source_rect->left = texture_height - (move_rect->SourcePoint.y + move_rect->DestinationRect.bottom - move_rect->DestinationRect.top);
			source_rect->top = move_rect->SourcePoint.x;
			source_rect->right = texture_height - move_rect->SourcePoint.y;
			source_rect->bottom = move_rect->SourcePoint.x + move_rect->DestinationRect.right - move_rect->DestinationRect.left;

			dest_rect->left = texture_height - move_rect->DestinationRect.bottom;
			dest_rect->top = move_rect->DestinationRect.left;
			dest_rect->right = texture_height - move_rect->DestinationRect.top;
			dest_rect->bottom = move_rect->DestinationRect.right;
			break;
		}
		case DXGI_MODE_ROTATION_ROTATE180:
		{
			source_rect->left = texture_width - (move_rect->SourcePoint.x + move_rect->DestinationRect.right - move_rect->DestinationRect.left);
			source_rect->top = texture_height - (move_rect->SourcePoint.y + move_rect->DestinationRect.bottom - move_rect->DestinationRect.top);
			source_rect->right = texture_width - move_rect->SourcePoint.x;
			source_rect->bottom = texture_height - move_rect->SourcePoint.y;

			dest_rect->left = texture_width - move_rect->DestinationRect.right;
			dest_rect->top = texture_height - move_rect->DestinationRect.bottom;
			dest_rect->right = texture_width - move_rect->DestinationRect.left;
			dest_rect->bottom = texture_height - move_rect->DestinationRect.top;
			break;
		}
		case DXGI_MODE_ROTATION_ROTATE270:
		{
			source_rect->left = move_rect->SourcePoint.x;
			source_rect->top = texture_width - (move_rect->SourcePoint.x + move_rect->DestinationRect.right - move_rect->DestinationRect.left);
			source_rect->right = move_rect->SourcePoint.y + move_rect->DestinationRect.bottom - move_rect->DestinationRect.top;
			source_rect->bottom = texture_width - move_rect->SourcePoint.x;

			dest_rect->left = move_rect->DestinationRect.top;
			dest_rect->top = texture_width - move_rect->DestinationRect.right;
			dest_rect->right = move_rect->DestinationRect.bottom;
			dest_rect->bottom = texture_width - move_rect->DestinationRect.left;
			break;
		}
		default:
		{
			memset(dest_rect, 0x00, sizeof(RECT));
			memset(source_rect, 0x00, sizeof(RECT));
			break;
		}
	}
	return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;
}

int32_t sirius::library::video::source::d3d11::desktop::capturer::capturer::core::display_manager::copy_move(ID3D11Texture2D * shared_surface, DXGI_OUTDUPL_MOVE_RECT * move_buffer, int32_t move_count, int32_t offset_x, int32_t offset_y, DXGI_OUTPUT_DESC * desktop_desc, int32_t texture_width, int32_t texture_height)
{
	HRESULT hr = E_FAIL;
	D3D11_TEXTURE2D_DESC full_desc;
	shared_surface->GetDesc(&full_desc);

	if (!_move_surface)
	{
		D3D11_TEXTURE2D_DESC move_desc;
		move_desc = full_desc;
		move_desc.Width = desktop_desc->DesktopCoordinates.right - desktop_desc->DesktopCoordinates.left;
		move_desc.Height = desktop_desc->DesktopCoordinates.bottom - desktop_desc->DesktopCoordinates.top;
		move_desc.BindFlags = D3D11_BIND_RENDER_TARGET;
		move_desc.MiscFlags = 0;

		hr = _device->CreateTexture2D(&move_desc, nullptr, &_move_surface);
		if (FAILED(hr))
			return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}

	for (int32_t i = 0; i < move_count; i++)
	{
		RECT source_rect;
		RECT dest_rect;
		set_move_rect(&source_rect, &dest_rect, desktop_desc, &(move_buffer[i]), texture_width, texture_height);

		//copy rect out of shared surface
		D3D11_BOX box;
		box.left = source_rect.left + desktop_desc->DesktopCoordinates.left - offset_x;
		box.top = source_rect.top + desktop_desc->DesktopCoordinates.top - offset_y;
		box.front = 0;
		box.right = source_rect.right + desktop_desc->DesktopCoordinates.left - offset_x;
		box.bottom = source_rect.bottom + desktop_desc->DesktopCoordinates.top - offset_y;
		box.back = 1;
		_device_context->CopySubresourceRegion(_move_surface, 0, source_rect.left, source_rect.top, 0, shared_surface, 0, &box);

		//copy back to shared surface
		box.left = source_rect.left;
		box.top = source_rect.top;
		box.front = 0;
		box.right = source_rect.right;
		box.bottom = source_rect.bottom;
		box.back = 1;
		_device_context->CopySubresourceRegion(shared_surface, 0, dest_rect.left + desktop_desc->DesktopCoordinates.left - offset_x, dest_rect.top + desktop_desc->DesktopCoordinates.top - offset_y, 0, _move_surface, 0, &box);
	}

	return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;
}


#pragma warning(push)
#pragma warning(disable:6001) // false positives in SetDirtyVert due to tool bug

int32_t sirius::library::video::source::d3d11::desktop::capturer::capturer::core::display_manager::set_dirty_vertex(
	sirius::library::video::source::d3d11::desktop::capturer::capturer::core::vertex_t * vertices, 
	RECT * dirty, int32_t offset_x, int32_t offset_y, DXGI_OUTPUT_DESC * desktop_desc, D3D11_TEXTURE2D_DESC * full_desc, D3D11_TEXTURE2D_DESC * desc)
{
	int32_t center_x = full_desc->Width / 2;
	int32_t center_y = full_desc->Height / 2;

	int32_t width = desktop_desc->DesktopCoordinates.right - desktop_desc->DesktopCoordinates.left;
	int32_t height = desktop_desc->DesktopCoordinates.bottom - desktop_desc->DesktopCoordinates.top;

	// Rotation compensated destination rect
	RECT dest_dirty = *dirty;

	// Set appropriate coordinates compensated for rotation
	switch (desktop_desc->Rotation)
	{
		case DXGI_MODE_ROTATION_ROTATE90:
		{
			dest_dirty.left = width - dirty->bottom;
			dest_dirty.top = dirty->left;
			dest_dirty.right = width - dirty->top;
			dest_dirty.bottom = dirty->right;

			vertices[0].texture_coord = DirectX::XMFLOAT2(dirty->right / static_cast<FLOAT>(desc->Width), dirty->bottom / static_cast<FLOAT>(desc->Height));
			vertices[1].texture_coord = DirectX::XMFLOAT2(dirty->left / static_cast<FLOAT>(desc->Width), dirty->bottom / static_cast<FLOAT>(desc->Height));
			vertices[2].texture_coord = DirectX::XMFLOAT2(dirty->right / static_cast<FLOAT>(desc->Width), dirty->top / static_cast<FLOAT>(desc->Height));
			vertices[5].texture_coord = DirectX::XMFLOAT2(dirty->left / static_cast<FLOAT>(desc->Width), dirty->top / static_cast<FLOAT>(desc->Height));
			break;
		}
		case DXGI_MODE_ROTATION_ROTATE180:
		{
			dest_dirty.left = width - dirty->right;
			dest_dirty.top = height - dirty->bottom;
			dest_dirty.right = width - dirty->left;
			dest_dirty.bottom = height - dirty->top;

			vertices[0].texture_coord = DirectX::XMFLOAT2(dirty->right / static_cast<FLOAT>(desc->Width), dirty->top / static_cast<FLOAT>(desc->Height));
			vertices[1].texture_coord = DirectX::XMFLOAT2(dirty->right / static_cast<FLOAT>(desc->Width), dirty->bottom / static_cast<FLOAT>(desc->Height));
			vertices[2].texture_coord = DirectX::XMFLOAT2(dirty->left / static_cast<FLOAT>(desc->Width), dirty->top / static_cast<FLOAT>(desc->Height));
			vertices[5].texture_coord = DirectX::XMFLOAT2(dirty->left / static_cast<FLOAT>(desc->Width), dirty->bottom / static_cast<FLOAT>(desc->Height));
			break;
		}
		case DXGI_MODE_ROTATION_ROTATE270:
		{
			dest_dirty.left = dirty->top;
			dest_dirty.top = height - dirty->right;
			dest_dirty.right = dirty->bottom;
			dest_dirty.bottom = height - dirty->left;

			vertices[0].texture_coord = DirectX::XMFLOAT2(dirty->left / static_cast<FLOAT>(desc->Width), dirty->top / static_cast<FLOAT>(desc->Height));
			vertices[1].texture_coord = DirectX::XMFLOAT2(dirty->right / static_cast<FLOAT>(desc->Width), dirty->top / static_cast<FLOAT>(desc->Height));
			vertices[2].texture_coord = DirectX::XMFLOAT2(dirty->left / static_cast<FLOAT>(desc->Width), dirty->bottom / static_cast<FLOAT>(desc->Height));
			vertices[5].texture_coord = DirectX::XMFLOAT2(dirty->right / static_cast<FLOAT>(desc->Width), dirty->bottom / static_cast<FLOAT>(desc->Height));
			break;
		}
		default:
			assert(false); // drop through
		case DXGI_MODE_ROTATION_UNSPECIFIED:
		case DXGI_MODE_ROTATION_IDENTITY:
		{
			vertices[0].texture_coord = DirectX::XMFLOAT2(dirty->left / static_cast<FLOAT>(desc->Width), dirty->bottom / static_cast<FLOAT>(desc->Height));
			vertices[1].texture_coord = DirectX::XMFLOAT2(dirty->left / static_cast<FLOAT>(desc->Width), dirty->top / static_cast<FLOAT>(desc->Height));
			vertices[2].texture_coord = DirectX::XMFLOAT2(dirty->right / static_cast<FLOAT>(desc->Width), dirty->bottom / static_cast<FLOAT>(desc->Height));
			vertices[5].texture_coord = DirectX::XMFLOAT2(dirty->right / static_cast<FLOAT>(desc->Width), dirty->top / static_cast<FLOAT>(desc->Height));
			break;
		}
	}

	// Set positions
	vertices[0].position = DirectX::XMFLOAT3((dest_dirty.left + desktop_desc->DesktopCoordinates.left - offset_x - center_x) / static_cast<FLOAT>(center_x),
		-1 * (dest_dirty.bottom + desktop_desc->DesktopCoordinates.top - offset_y - center_y) / static_cast<FLOAT>(center_y),
		0.0f);
	vertices[1].position = DirectX::XMFLOAT3((dest_dirty.left + desktop_desc->DesktopCoordinates.left - offset_x - center_x) / static_cast<FLOAT>(center_x),
		-1 * (dest_dirty.top + desktop_desc->DesktopCoordinates.top - offset_y - center_y) / static_cast<FLOAT>(center_y),
		0.0f);
	vertices[2].position = DirectX::XMFLOAT3((dest_dirty.right + desktop_desc->DesktopCoordinates.left - offset_x - center_x) / static_cast<FLOAT>(center_x),
		-1 * (dest_dirty.bottom + desktop_desc->DesktopCoordinates.top - offset_y - center_y) / static_cast<FLOAT>(center_y),
		0.0f);
	vertices[3].position = vertices[2].position;
	vertices[4].position = vertices[1].position;
	vertices[5].position = DirectX::XMFLOAT3((dest_dirty.right + desktop_desc->DesktopCoordinates.left - offset_x - center_x) / static_cast<FLOAT>(center_x),
		-1 * (dest_dirty.top + desktop_desc->DesktopCoordinates.top - offset_y - center_y) / static_cast<FLOAT>(center_y),
		0.0f);

	vertices[3].texture_coord = vertices[2].texture_coord;
	vertices[4].texture_coord = vertices[1].texture_coord;

	return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;
}

#pragma warning(pop)

int32_t sirius::library::video::source::d3d11::desktop::capturer::core::display_manager::copy_dirty(ID3D11Texture2D * source_surface, ID3D11Texture2D * shared_surface, RECT * dirty_buffer, uint32_t dirty_count, int32_t offset_x, int32_t offset_y, DXGI_OUTPUT_DESC * desktop_desc)
{
	HRESULT hr;

	D3D11_TEXTURE2D_DESC full_desc;
	shared_surface->GetDesc(&full_desc);

	D3D11_TEXTURE2D_DESC ThisDesc;
	source_surface->GetDesc(&ThisDesc);

	if (!_render_target_view)
	{
		hr = _device->CreateRenderTargetView(shared_surface, nullptr, &_render_target_view);
		if (FAILED(hr))
		{
			return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
		}
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC shader_desc;
	shader_desc.Format = ThisDesc.Format;
	shader_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shader_desc.Texture2D.MostDetailedMip = ThisDesc.MipLevels - 1;
	shader_desc.Texture2D.MipLevels = ThisDesc.MipLevels;

	// Create new shader resource view
	ID3D11ShaderResourceView * shader_resource = nullptr;
	hr = _device->CreateShaderResourceView(source_surface, &shader_desc, &shader_resource);
	if (FAILED(hr))
	{
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}

	FLOAT BlendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
	_device_context->OMSetBlendState(nullptr, BlendFactor, 0xFFFFFFFF);
	_device_context->OMSetRenderTargets(1, &_render_target_view, nullptr);
	_device_context->VSSetShader(_vertex_shader, nullptr, 0);
	_device_context->PSSetShader(_pixel_shader, nullptr, 0);
	_device_context->PSSetShaderResources(0, 1, &shader_resource);
	_device_context->PSSetSamplers(0, 1, &_sampler_linear);
	_device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create space for vertices for the dirty rects if the current space isn't large enough
	uint32_t bytes_needed = sizeof(sirius::library::video::source::d3d11::desktop::capturer::core::vertex_t) * NVERTICES * dirty_count;
	if (bytes_needed > _dirty_vertex_buffer_alloc_size)
	{
		if (_dirty_vertex_buffer_alloc)
		{
			delete[] _dirty_vertex_buffer_alloc;
		}

		_dirty_vertex_buffer_alloc = new (std::nothrow) BYTE[bytes_needed];
		if (!_dirty_vertex_buffer_alloc)
		{
			_dirty_vertex_buffer_alloc_size = 0;
			return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
		}

		_dirty_vertex_buffer_alloc_size = bytes_needed;
	}

	// Fill them in
	sirius::library::video::source::d3d11::desktop::capturer::core::vertex_t * dirty_vertex = 
		reinterpret_cast<sirius::library::video::source::d3d11::desktop::capturer::core::vertex_t*>(_dirty_vertex_buffer_alloc);
	for (uint32_t i = 0; i < dirty_count; ++i, dirty_vertex += NVERTICES)
	{
		set_dirty_vertex(dirty_vertex, &(dirty_buffer[i]), offset_x, offset_y, desktop_desc, &full_desc, &ThisDesc);
	}

	// Create vertex buffer
	D3D11_BUFFER_DESC buffer_desc;
	memset(&buffer_desc, 0x00, sizeof(buffer_desc));
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.ByteWidth = bytes_needed;
	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	RtlZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = _dirty_vertex_buffer_alloc;

	ID3D11Buffer * vertex_buffer = nullptr;
	hr = _device->CreateBuffer(&buffer_desc, &InitData, &vertex_buffer);
	if (FAILED(hr))
	{
		//return ProcessFailure(m_Device, L"Failed to create vertex buffer in dirty rect processing", L"Error", hr, SystemTransitionsExpectedErrors);
		return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::fail;
	}
	UINT stride = sizeof(sirius::library::video::source::d3d11::desktop::capturer::core::vertex_t);
	UINT offset = 0;
	_device_context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);

	D3D11_VIEWPORT VP;
	VP.Width = static_cast<FLOAT>(full_desc.Width);
	VP.Height = static_cast<FLOAT>(full_desc.Height);
	VP.MinDepth = 0.0f;
	VP.MaxDepth = 1.0f;
	VP.TopLeftX = 0.0f;
	VP.TopLeftY = 0.0f;
	_device_context->RSSetViewports(1, &VP);

	_device_context->Draw(NVERTICES * dirty_count, 0);

	vertex_buffer->Release();
	vertex_buffer = nullptr;

	shader_resource->Release();
	shader_resource = nullptr;

	return sirius::library::video::source::d3d11::desktop::capturer::err_code_t::success;
}