#ifndef _SIRIUS_D3D11_COLORSPACE_CONVERTER_H_
#define _SIRIUS_D3D11_COLORSPACE_CONVERTER_H_

#include <sirius_video_colorspace_converter.h>

#if defined(EXPORT_D3D11_COLORSPACE_CONVERTER_LIB)
#define EXP_D3D11_COLORSPACE_CONVERTER_CLASS __declspec(dllexport)
#else
#define EXP_D3D11_COLORSPACE_CONVERTER_CLASS __declspec(dllimport)
#endif

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
						class EXP_D3D11_COLORSPACE_CONVERTER_CLASS converter
							: public sirius::library::video::transform::colorspace::converter
						{
						public:
							static const int32_t THREAD_COUNT_X = 32;
							static const int32_t THREAD_COUNT_Y = 16;
							static const int32_t DXGI_FORMAT_I420 = 116;
						public:
							class core;
						public:
							typedef struct EXP_D3D11_COLORSPACE_CONVERTER_CLASS _context_t
							{
								void *	device;
								void *	device_context;
								_context_t(void);
								_context_t(const sirius::library::video::transform::colorspace::d3d11::converter::_context_t & clone);
								_context_t & operator=(const _context_t & clone);
							} context_t;

						public:
							converter(void);
							virtual ~converter(void);

							virtual int32_t initialize(void * context);
							virtual int32_t release(void);
							virtual int32_t convert(sirius::library::video::transform::colorspace::d3d11::converter::entity_t * input, sirius::library::video::transform::colorspace::d3d11::converter::entity_t * output);

						private:
							sirius::library::video::transform::colorspace::d3d11::converter::core * _core;
						};
					};
				};
			};
		};
	};
};

















#endif