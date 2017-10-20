#ifndef _D3D11_OUTPUT_MANAGER_H_
#define _D3D11_OUTPUT_MANAGER_H_

#include "d3d11_desktop_capturer.h"
#include <atlbase.h>

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
						class capturer::core::output_manager
						{
						public:
							output_manager(void);
							~output_manager(void);

							int32_t initialize(sirius::library::video::source::d3d11::desktop::capturer::context_t * context, uint32_t * count_output, RECT * desktop_bounds);
							int32_t release(void);

							int32_t update_application_window(sirius::library::video::source::d3d11::desktop::capturer::core::capture_info_t * info, bool * occluded);
							HANDLE	shared_handle(void);
							void	resize_windows(void);

						private:
							int32_t	process_mono_mask(bool bmono, sirius::library::video::source::d3d11::desktop::capturer::core::capture_info_t * info, int32_t * width, int32_t * height, int32_t * left, int32_t * top, uint8_t ** buffer, D3D11_BOX * box);
							int32_t make_render_target_view(void);
							int32_t	set_view_port(uint32_t width, uint32_t height);
							int32_t initialize_shaders(void);
							int32_t create_shared_surface(int32_t single_output, uint32_t * count_output, RECT * desktop_bounds);
							int32_t draw_frame(void);
							int32_t draw_mouse(sirius::library::video::source::d3d11::desktop::capturer::core::capture_info_t * info);
							int32_t resize_swap_chain(void);

							int32_t convert(ID3D11Texture2D * input, ID3D11Resource * output);

						private:
							IDXGISwapChain1 *			_swap_chain;
							ID3D11Device *				_device;
							IDXGIFactory2 *				_factory;
							ID3D11DeviceContext *		_device_context;
							ID3D11RenderTargetView *	_render_target_view;
							ID3D11SamplerState *		_sampler_linear;
							ID3D11BlendState *			_blend_state;
							ID3D11VertexShader *		_vertex_shader;
							ID3D11PixelShader *			_pixel_shader;
							ID3D11InputLayout *			_input_layout;
							ID3D11Texture2D *			_shared_surface;
							IDXGIKeyedMutex *			_key_mutex;
							HWND						_hwnd;
							BOOL						_needs_resize;
							DWORD						_occlusion_cookie;
							BOOL						_present;
							int32_t						_video_submedia_type;
							int32_t						_video_memory_type;
							int32_t						_width;
							int32_t						_height;
							int32_t						_crop_x;
							int32_t						_crop_y;
							int32_t						_crop_width;
							int32_t						_crop_height;
							sirius::library::video::source::d3d11::desktop::capturer::handler * _handler;

							ATL::CComPtr<ID3D11Texture2D> _d3d11_2d_cpu_texture;
							uint8_t * _d3d11_2d_cpu_texture_buffer;

							ATL::CComPtr<ID3D11Texture2D> _d3d11_2d_texture_intermediate;
							ATL::CComPtr<ID3D11Texture2D> _d3d11_2d_texture;

							ATL::CComPtr<ID3D11VideoDevice>					_d3d11_video_device;
							ATL::CComPtr<ID3D11DeviceContext>				_d3d11_device_context;
							ATL::CComPtr<ID3D11VideoProcessorEnumerator>	_d3d11_video_processor_enum;
							ATL::CComPtr<ID3D11VideoProcessor>				_d3d11_video_processor;

						};
					};
				};
			};
		};
	};
};







#endif
