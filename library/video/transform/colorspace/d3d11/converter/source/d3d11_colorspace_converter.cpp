#include "d3d11_colorspace_converter.h"

sirius::library::video::transform::colorspace::d3d11::converter::core::core(void)
	: _device(NULL)
	, _device_context(NULL)
	, _cs_converter(NULL)
	, _cs_resizer(NULL)
{

}

sirius::library::video::transform::colorspace::d3d11::converter::core::~core(void)
{

}

int32_t sirius::library::video::transform::colorspace::d3d11::converter::core::initialize(sirius::library::video::transform::colorspace::d3d11::converter::context_t * context)
{
	_context = context;
	_device = (ID3D11Device*)_context->device;
	_device_context = (ID3D11DeviceContext*)_context->device_context;

	return sirius::library::video::transform::colorspace::d3d11::converter::err_code_t::success;
}

int32_t sirius::library::video::transform::colorspace::d3d11::converter::core::release(void)
{
	SIRIUS_SAFE_RELEASE(_output_uav);
	SIRIUS_SAFE_RELEASE(_output_buffer);
	SIRIUS_SAFE_RELEASE(_input_convert_uav);
	SIRIUS_SAFE_RELEASE(_input_convert_srv);
	_input_convert_texture = NULL;
	SIRIUS_SAFE_RELEASE(_input_resize_srv);
	_input_resize_texture = NULL;
	SIRIUS_SAFE_RELEASE(_input_resize_buffer);
	_cs_converter = NULL;
	_cs_resizer = NULL;
	_device_context = NULL;
	_device = NULL;
	return sirius::library::video::transform::colorspace::d3d11::converter::err_code_t::success;
}

int32_t sirius::library::video::transform::colorspace::d3d11::converter::core::convert(sirius::library::video::transform::colorspace::d3d11::converter::entity_t * input, sirius::library::video::transform::colorspace::d3d11::converter::entity_t * output)
{
	ATL::CComPtr<ID3D11Texture2D> stexture = (ID3D11Texture2D*)input->data;
	ATL::CComPtr<ID3D11Texture2D> dtexture = (ID3D11Texture2D*)output->data;

	if (!_cs_resizer || !_cs_converter)
	{
		D3D11_TEXTURE2D_DESC stexture_desc;
		D3D11_TEXTURE2D_DESC dtexture_desc;
		stexture->GetDesc(&stexture_desc);
		dtexture->GetDesc(&dtexture_desc);

		_idxgi_format = stexture_desc.Format;
		_odxgi_format = dtexture_desc.Format;

		HRESULT hr = S_OK;
		hr = create_resize_input_shader_resource_view(stexture_desc.Width, stexture_desc.Height);
		if (FAILED(hr))
			return sirius::library::video::transform::colorspace::d3d11::converter::err_code_t::fail;

		hr = create_convert_input_shader_resource_view(stexture_desc.Width, stexture_desc.Height, dtexture_desc.Width, dtexture_desc.Height);
		if (FAILED(hr))
			return sirius::library::video::transform::colorspace::d3d11::converter::err_code_t::fail;

		hr = create_output_unordered_access_view(dtexture_desc.Width, dtexture_desc.Height);
		if (FAILED(hr))
			return sirius::library::video::transform::colorspace::d3d11::converter::err_code_t::fail;

		hr = create_compute_shader();
		if (FAILED(hr))
			return sirius::library::video::transform::colorspace::d3d11::converter::err_code_t::fail;
	}

	{//Resize
		_device_context->CopyResource(_input_resize_texture, stexture);

		_device_context->CSSetShader(_cs_resizer, NULL, 0);
		_device_context->CSSetShaderResources(0, 1, &_input_resize_srv);
		_device_context->CSSetUnorderedAccessViews(0, 1, &_input_convert_uav, NULL);
		_device_context->Dispatch(_resize_thread_group_count_x, _resize_thread_group_count_y, 1);
	}
	{//Convert
		ID3D11ShaderResourceView* nullSRV[1] = { 0 };
		ID3D11UnorderedAccessView* nullUAV[1] = { 0 };

		_device_context->CSSetShader(_cs_converter, NULL, 0);

		_device_context->CSSetShaderResources(0, 1, nullSRV);
		_device_context->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);
		
		_device_context->CSSetShaderResources(0, 1, &_input_convert_srv);
		_device_context->CSSetUnorderedAccessViews(0, 1, &_output_uav, NULL);
		_device_context->Dispatch(_convert_thread_group_count_x, _convert_thread_group_count_y, 1);
	}

	_device_context->CopyResource(dtexture, _output_buffer);
	return sirius::library::video::transform::colorspace::d3d11::converter::err_code_t::success;
}

//private
HRESULT sirius::library::video::transform::colorspace::d3d11::converter::core::create_resize_input_shader_resource_view(int32_t width, int32_t height)
{
	_resize_thread_group_count_x = width / THREAD_COUNT_X;
	_resize_thread_group_count_y = height / THREAD_COUNT_Y;

	HRESULT hr = S_OK;
	D3D11_TEXTURE2D_DESC iresize_texture_desc;
	::ZeroMemory(&iresize_texture_desc, sizeof(iresize_texture_desc));
	iresize_texture_desc.Width = width;
	iresize_texture_desc.Height = height;
	iresize_texture_desc.MipLevels = 1;
	iresize_texture_desc.ArraySize = 1;
	iresize_texture_desc.Format = _idxgi_format;
	iresize_texture_desc.SampleDesc.Count = 1;
	iresize_texture_desc.SampleDesc.Quality = 0;
	iresize_texture_desc.Usage = D3D11_USAGE_DEFAULT;
	iresize_texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	hr = _device->CreateTexture2D(&iresize_texture_desc, NULL, &_input_resize_texture);
	if (FAILED(hr))
		return hr;

	D3D11_SHADER_RESOURCE_VIEW_DESC iresize_srv_desc;
	iresize_srv_desc.Format = iresize_texture_desc.Format;
	iresize_srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	iresize_srv_desc.Texture2D.MostDetailedMip = 0;
	iresize_srv_desc.Texture2D.MipLevels = 1;
	hr = _device->CreateShaderResourceView(_input_resize_texture, &iresize_srv_desc, &_input_resize_srv);
	if (FAILED(hr))
	{
		_input_resize_texture = NULL;
		return hr;
	}

	return hr;
}

HRESULT sirius::library::video::transform::colorspace::d3d11::converter::core::create_convert_input_shader_resource_view(int32_t swidth, int32_t sheight, int32_t dwidth, int32_t dheight)
{
	HRESULT hr = S_OK;

	_resize_ctx.wratio = (float)dwidth / swidth;
	_resize_ctx.hratio = (float)dheight / swidth;

	D3D11_BUFFER_DESC resize_buffer_desc;
	::ZeroMemory(&resize_buffer_desc, sizeof(D3D11_BUFFER_DESC));
	resize_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	resize_buffer_desc.ByteWidth = 16;// sizeof(_resize_ctx);
	resize_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	resize_buffer_desc.CPUAccessFlags = 0;
	resize_buffer_desc.MiscFlags = 0;

	hr = _device->CreateBuffer(&resize_buffer_desc, NULL, &_input_resize_buffer);
	if (FAILED(hr))
		return hr;

	_device_context->UpdateSubresource(_input_resize_buffer, 0, NULL, &_resize_ctx, 0, 0);
	_device_context->CSSetConstantBuffers(0, 1, &_input_resize_buffer);

	D3D11_TEXTURE2D_DESC iconvert_texture_desc;
	::ZeroMemory(&iconvert_texture_desc, sizeof(iconvert_texture_desc));
	iconvert_texture_desc.Width = dwidth;
	iconvert_texture_desc.Height = dheight;
	iconvert_texture_desc.MipLevels = 1;
	iconvert_texture_desc.ArraySize = 1;
	iconvert_texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UINT;
	iconvert_texture_desc.SampleDesc.Count = 1;
	iconvert_texture_desc.SampleDesc.Quality = 0;
	iconvert_texture_desc.Usage = D3D11_USAGE_DEFAULT;
	iconvert_texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	iconvert_texture_desc.MiscFlags = 0;
	hr = _device->CreateTexture2D(&iconvert_texture_desc, NULL, &_input_convert_texture);
	if (FAILED(hr))
	{
		SIRIUS_SAFE_RELEASE(_input_resize_srv);
		_input_resize_texture = NULL;
		_input_resize_buffer = NULL;
		return hr;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC iconvert_srv_desc;
	iconvert_srv_desc.Format = iconvert_texture_desc.Format;
	iconvert_srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	iconvert_srv_desc.Texture2D.MostDetailedMip = 0;
	iconvert_srv_desc.Texture2D.MipLevels = 1;
	hr = _device->CreateShaderResourceView(_input_convert_texture, &iconvert_srv_desc, &_input_convert_srv);
	if (FAILED(hr))
	{
		_input_convert_texture = NULL;
		SIRIUS_SAFE_RELEASE(_input_resize_srv);
		_input_resize_texture = NULL;
		_input_resize_buffer = NULL;
		return hr;
	}

	D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
	uav_desc.Format = iconvert_texture_desc.Format;
	uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uav_desc.Texture2D.MipSlice = 0;
	hr = _device->CreateUnorderedAccessView(_input_convert_texture, &uav_desc, &_input_convert_uav);
	if (FAILED(hr))
	{
		SIRIUS_SAFE_RELEASE(_input_convert_srv);
		_input_convert_texture = NULL;
		SIRIUS_SAFE_RELEASE(_input_resize_srv);
		_input_resize_texture = NULL;
		_input_resize_buffer = NULL;
		return hr;
	}

	return hr;
}

HRESULT sirius::library::video::transform::colorspace::d3d11::converter::core::create_output_unordered_access_view(int32_t width, int32_t height)
{
	HRESULT hr = S_OK;

	_convert_thread_group_count_x = width / THREAD_COUNT_X;
	_convert_thread_group_count_y = height / THREAD_COUNT_Y;

	D3D11_BUFFER_DESC output_desc;
	ZeroMemory(&output_desc, sizeof(output_desc));
	output_desc.Usage = D3D11_USAGE_DEFAULT;
	output_desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	output_desc.ByteWidth = get_output_byte(_odxgi_format, width, height);
	output_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	output_desc.CPUAccessFlags = 0;
	output_desc.StructureByteStride = 4;
	hr = _device->CreateBuffer(&output_desc, NULL, &_output_buffer);
	if (FAILED(hr))
	{
		SIRIUS_SAFE_RELEASE(_input_convert_uav);
		SIRIUS_SAFE_RELEASE(_input_convert_srv);
		_input_convert_texture = NULL;
		SIRIUS_SAFE_RELEASE(_input_resize_srv);
		_input_resize_texture = NULL;
		_input_resize_buffer = NULL;
		return hr;
	}

	D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
	uav_desc.Format = DXGI_FORMAT_UNKNOWN;	//It should be 'DXGI_FORMAT_UNKNOWN' for structured buffer.
	uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uav_desc.Buffer.FirstElement = 0;
	uav_desc.Buffer.Flags = 0;
	uav_desc.Buffer.NumElements = output_desc.ByteWidth / output_desc.StructureByteStride;
	hr = _device->CreateUnorderedAccessView(_output_buffer, &uav_desc, &_output_uav);
	if (FAILED(hr))
	{
		SIRIUS_SAFE_RELEASE(_output_buffer);
		SIRIUS_SAFE_RELEASE(_input_convert_uav);
		SIRIUS_SAFE_RELEASE(_input_convert_srv);
		_input_convert_texture = NULL;
		SIRIUS_SAFE_RELEASE(_input_resize_srv);
		_input_resize_texture = NULL;
		_input_resize_buffer = NULL;
		return hr;
	}

	return hr;
}

HRESULT sirius::library::video::transform::colorspace::d3d11::converter::core::create_compute_shader(void)
{
	LPCSTR profile = (_device->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "cs_5_0" : "cs_4_0";
	LPCWSTR shader_filepath = NULL;
	ID3DBlob * err_blob = NULL;
	ID3DBlob * blob = NULL;
	HRESULT hr = D3DCompileFromFile(L"shader\\resize.hlsl", NULL, NULL, "CS", profile, NULL, NULL, &blob, &err_blob);
	if (FAILED(hr))
	{
		if (err_blob)
			OutputDebugStringA((char*)err_blob->GetBufferPointer());
		if (err_blob)
			err_blob->Release();
		if (blob)
			blob->Release();

		return sirius::library::video::transform::colorspace::d3d11::converter::err_code_t::fail;
	}

	hr = _device->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &_cs_resizer);
	if (FAILED(hr))
	{
		if (err_blob)
			err_blob->Release();
		if (blob)
			blob->Release();

		return sirius::library::video::transform::colorspace::d3d11::converter::err_code_t::fail;
	}

	if (err_blob)
		err_blob->Release();
	if (blob)
		blob->Release();

	hr = D3DCompileFromFile(L"shader\\r8g8b8_to_nv12.hlsl", NULL, NULL, "CS", profile, NULL, NULL, &blob, &err_blob);
	if (FAILED(hr))
	{
		_cs_resizer = NULL;

		if (err_blob)
			err_blob->Release();
		if (blob)
			blob->Release();

		return sirius::library::video::transform::colorspace::d3d11::converter::err_code_t::fail;
	}

	hr = _device->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &_cs_converter);
	if (FAILED(hr))
	{
		_cs_resizer = NULL;

		if (err_blob)
			err_blob->Release();
		if (blob)
			blob->Release();

		return sirius::library::video::transform::colorspace::d3d11::converter::err_code_t::fail;
	}

	if (err_blob)
		err_blob->Release();
	if (blob)
		blob->Release();

	return hr;
}

int32_t sirius::library::video::transform::colorspace::d3d11::converter::core::get_output_byte(DXGI_FORMAT cs, int32_t width, int32_t height)
{
	int32_t nbytes = 0;
	switch (cs)
	{
	case DXGI_FORMAT_R8G8B8A8_UINT :
		nbytes = width * height * 4;
		break;
	case DXGI_FORMAT_NV12 :
		nbytes = width * height + width * height / 2;
		break;
	default:
		nbytes = 0;
		break;
	}

	return nbytes;
}