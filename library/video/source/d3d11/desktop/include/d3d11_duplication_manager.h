#ifndef _D3D11_DUPLICATION_MANAGER_H_
#define _D3D11_DUPLICATION_MANAGER_H_

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
						class capturer::core::duplication_manager
						{
						public:
							duplication_manager(void);
							~duplication_manager(void);

							int32_t initialize(ID3D11Device * device, int32_t output);
							int32_t release(void);

							int32_t get_mouse(sirius::library::video::source::d3d11::desktop::capturer::core::capture_info_t * info, DXGI_OUTDUPL_FRAME_INFO * frame_info, int32_t offset_x, int32_t offset_y);
							int32_t acquire_frame(sirius::library::video::source::d3d11::desktop::capturer::core::frame_data_t * data, bool * timeout);
							int32_t release_frame(void);

							int32_t get_output_description(DXGI_OUTPUT_DESC * desc);

							

						private:
							IDXGIOutputDuplication *	_desktop_duplication;
							ID3D11Texture2D *			_acquired_desktop_image;
							unsigned char *				_meta_data_buffer;
							uint32_t					_meta_data_size;
							uint32_t					_output_number;
							DXGI_OUTPUT_DESC			_output_desc;
							ID3D11Device *				_device;
						};
					};
				};
			};
		};
	};
};

#endif
