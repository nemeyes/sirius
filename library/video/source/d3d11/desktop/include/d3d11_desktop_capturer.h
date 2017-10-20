#ifndef _D3D11_DESKTOP_CAPTURER_H_
#define _D3D11_DESKTOP_CAPTURER_H_

#include "sirius_d3d11_desktop_capturer.h"

#include <atlbase.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <rpcdce.h>
#include <new>
#include <process.h>
#include <DirectXMath.h>

#include "vertex_shader.h"
#include "pixel_shader.h"

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
						class capturer::core
						{
						public:
							class display_manager;
							class duplication_manager;
							class output_manager;
							
							static const int32_t NVERTICES = 6;
							static const int32_t BPP = 4;

							typedef struct _capture_info_t
							{
								unsigned char *					shape_buffer;
								DXGI_OUTDUPL_POINTER_SHAPE_INFO	shape_info;
								POINT							position;
								bool							visible;
								uint32_t						buffer_size;
								uint32_t						who_updated_position_last;
								LARGE_INTEGER					last_timestamp;
							} capture_info_t;

							typedef struct _d3d11_resources_t
							{
								ID3D11Device *			device;
								ID3D11DeviceContext *	context;
								ID3D11VertexShader *	vertex_shader;
								ID3D11PixelShader *		pixel_shader;
								ID3D11InputLayout *		input_layout;
								ID3D11SamplerState *	sampler_linear;
							} d3d11_resources_t;

							typedef struct _thread_context_t
							{
								HANDLE				unexpected_error_event;
								HANDLE				expected_error_event;
								HANDLE				texure_shared_handle;
								int32_t				output;
								int32_t				offset_x;
								int32_t				offset_y;
								sirius::library::video::source::d3d11::desktop::capturer::core::capture_info_t *	info;
								sirius::library::video::source::d3d11::desktop::capturer::core::d3d11_resources_t	resource;
								sirius::library::video::source::d3d11::desktop::capturer::core *					self;
							} thread_context_t;

							typedef struct _frame_data_t
							{
								ID3D11Texture2D *		frame;
								DXGI_OUTDUPL_FRAME_INFO frame_info;
								unsigned char	*		meta_data;
								int32_t					dirty_count;
								int32_t					move_count;
							} frame_data_t;

							typedef struct _vertex_t
							{
								DirectX::XMFLOAT3	position;
								DirectX::XMFLOAT2	texture_coord;
							} vertex_t;

							const static D3D_DRIVER_TYPE	DRIVER_TYPES[];
							const static D3D_FEATURE_LEVEL	FEATURE_LEVELS[];
							const static int32_t			COUNT_OF_DRIVER_TYPES;
							const static int32_t			COUNT_OF_FEATURE_LEVELS;

							core(void);
							~core(void);

							int32_t initialize(sirius::library::video::source::d3d11::desktop::capturer::context_t * context);
							int32_t release(void);

							int32_t start(void);
							int32_t stop(void);

							int32_t update_application_window(bool * occluded);

						private:
							int32_t initialize_d3d11(sirius::library::video::source::d3d11::desktop::capturer::core::d3d11_resources_t * resource);
							int32_t release_d3d11(sirius::library::video::source::d3d11::desktop::capturer::core::d3d11_resources_t * resource);
							 

						private:
							static unsigned __stdcall process_cb(void * p);

						private:
							sirius::library::video::source::d3d11::desktop::capturer::core::capture_info_t _info;
							sirius::library::video::source::d3d11::desktop::capturer::context_t * _context;

							//HANDLE		_terminate_threads_event;
							sirius::library::video::source::d3d11::desktop::capturer::core::thread_context_t * _threads_data;
							int32_t		_threads_count;
							HANDLE	*	_threads;
							BOOL		_run;
							sirius::library::video::source::d3d11::desktop::capturer::core::output_manager * _output_mgr;
							
						};
					};
				};
			};
		};
	};
};












#endif
