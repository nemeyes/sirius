#ifndef _D3D11_DISPLAY_MANAGER_H_
#define _D3D11_DISPLAY_MANAGER_H_

#include "d3d11_desktop_capturer.h"

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
					namespace desktop
					{
						class capturer::core::display_manager
						{
						public:
							display_manager(void);
							~display_manager(void);

							int32_t initialize(sirius::library::video::source::d3d11::desktop::capturer::core::d3d11_resources_t * resource);
							int32_t release(void);
							int32_t process(sirius::library::video::source::d3d11::desktop::capturer::core::frame_data_t * data, ID3D11Texture2D * shared_surface, int32_t offset_x, int32_t offset_y, DXGI_OUTPUT_DESC * desktop_desc);

							ID3D11Device * device(void);
						private:
							int32_t copy_dirty(ID3D11Texture2D * source_surface, ID3D11Texture2D * shared_surface, RECT * dirty_buffer, uint32_t  dirty_count, int32_t offset_x, int32_t offset_y, DXGI_OUTPUT_DESC * desktop_desc);
							int32_t copy_move(ID3D11Texture2D * shared_surface, DXGI_OUTDUPL_MOVE_RECT * move_buffer, int32_t move_count, int32_t offset_x, int32_t offset_y, DXGI_OUTPUT_DESC * desktop_desc, int32_t texture_width, int32_t texture_height);
							int32_t set_dirty_vertex(sirius::library::video::source::d3d11::desktop::capturer::core::vertex_t * vertices, RECT * dirty, int32_t offset_x, int32_t offset_y, DXGI_OUTPUT_DESC * desktop_desc, D3D11_TEXTURE2D_DESC * full_desc, D3D11_TEXTURE2D_DESC * desc);
							int32_t set_move_rect(RECT * source_rect, RECT * dest_rect, DXGI_OUTPUT_DESC * desktop_desc, DXGI_OUTDUPL_MOVE_RECT * move_rect, int32_t texture_width, int32_t texture_height);

						private:
							ID3D11Device *				_device;
							ID3D11DeviceContext *		_device_context;
							ID3D11Texture2D *			_move_surface;
							ID3D11VertexShader *		_vertex_shader;
							ID3D11PixelShader *			_pixel_shader;
							ID3D11InputLayout *			_input_layout;
							ID3D11RenderTargetView *	_render_target_view;
							ID3D11SamplerState *		_sampler_linear;
							unsigned char *				_dirty_vertex_buffer_alloc;
							uint32_t					_dirty_vertex_buffer_alloc_size;
						};
					};
				};
			};
		};
	};
};











#endif