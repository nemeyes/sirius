#ifndef _D3D11_COLORSPACE_CONVERTER_H_
#define _D3D11_COLORSPACE_CONVERTER_H_

#include "sirius_d3d11_colorspace_converter.h"
#include <atlbase.h>
#include <d3d11.h>
//#include <d3dx11.h>
#include <d3dcompiler.h>
//#include <xnamath.h>
#include <dxgiformat.h>

namespace sirius
{
	namespace library
	{
		namespace video
		{
			namespace transform
			{
				namespace colorspace
				{
					namespace d3d11
					{
						class converter::core
						{
						public:
							struct resizer_context_t
							{
								float wratio;
								float hratio;
							};

							core(void);
							~core(void);

							int32_t initialize(sirius::library::video::transform::colorspace::d3d11::converter::context_t * context);
							int32_t release(void);
							int32_t convert(sirius::library::video::transform::colorspace::d3d11::converter::entity_t * input, sirius::library::video::transform::colorspace::d3d11::converter::entity_t * output);

						private:
							HRESULT create_resize_input_shader_resource_view(int32_t width, int32_t height);
							HRESULT create_convert_input_shader_resource_view(int32_t swidth, int32_t sheight, int32_t dwidth, int32_t dheight);
							HRESULT create_output_unordered_access_view(int32_t width, int32_t height);
							HRESULT create_compute_shader(void);
							int32_t get_output_byte(DXGI_FORMAT cs, int32_t width, int32_t height);



						private:
							sirius::library::video::transform::colorspace::d3d11::converter::context_t * _context;
							ATL::CComPtr<ID3D11Device>			_device;
							ATL::CComPtr<ID3D11DeviceContext>	_device_context;
							ATL::CComPtr<ID3D11ComputeShader>	_cs_converter;
							ATL::CComPtr<ID3D11ComputeShader>	_cs_resizer;

							ID3D11Buffer *							_input_resize_buffer;
							ATL::CComPtr<ID3D11Texture2D>			_input_resize_texture;
							ID3D11ShaderResourceView *				_input_resize_srv;
							ATL::CComPtr<ID3D11Texture2D>			_input_convert_texture;
							ID3D11ShaderResourceView *				_input_convert_srv;
							ID3D11UnorderedAccessView *				_input_convert_uav;

							ID3D11Buffer *							_output_buffer;
							ID3D11UnorderedAccessView *				_output_uav;

							resizer_context_t _resize_ctx;
							int32_t _resize_thread_group_count_x;
							int32_t _resize_thread_group_count_y;
							int32_t _convert_thread_group_count_x;
							int32_t _convert_thread_group_count_y;

							DXGI_FORMAT _idxgi_format;
							DXGI_FORMAT _odxgi_format;
						};
					};
				};
			};
		};
	};
};



#endif
