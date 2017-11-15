#ifndef _SIRIUS_VIDEO_PROCESSOR_H_
#define _SIRIUS_VIDEO_PROCESSOR_H_

#include <sirius.h>

#include <cguid.h>
#include <atlbase.h>
#include <d3d11.h>

namespace sirius
{
	namespace library
	{
		namespace video
		{
			namespace transform
			{
				namespace codec
				{
					class processor
						: public sirius::base
					{
					public:
						processor(void);
						virtual ~processor(void);

					protected:
						void		begin_elapsed_time(void);
						long long	elapsed_microseconds(void);
						long long	elapsed_milliseconds(void);

						int32_t initialize_d3d11(ID3D11Device * d3d11_device, int32_t iwidth, int32_t iheight, int32_t ifps, int32_t owidth, int32_t oheight, int32_t ofps, DXGI_FORMAT oformat = DXGI_FORMAT_NV12);
						int32_t release_d3d11(void);
						int32_t convert_d3d11_texture2d_format(ID3D11Texture2D * input, ID3D11Resource * output, int32_t iwidth, int32_t iheight, int32_t owidth, int32_t oheight);

					protected:
						LARGE_INTEGER _frequency;
						LARGE_INTEGER _begin_elapsed_microseconds;
						unsigned long long _frame_count;

						ATL::CComPtr<ID3D11VideoDevice> _d3d11_video_device;
						ATL::CComPtr<ID3D11DeviceContext> _d3d11_device_context;
						ATL::CComPtr<ID3D11VideoProcessorEnumerator> _d3d11_video_processor_enum;
						ATL::CComPtr<ID3D11VideoProcessor> _d3d11_video_processor;
					};
				};
			};
		};
	};
};


#endif
