#ifndef _SIRIUS_D3D11_DESKTOP_CAPTURE_H_
#define _SIRIUS_D3D11_DESKTOP_CAPTURE_H_

#if defined(EXPORT_D3D11_DESKTOP_CAPTURE_LIB)
#define EXP_D3D11_DESKTOP_CAPTURE_CLASS __declspec(dllexport)
#else
#define EXP_D3D11_DESKTOP_CAPTURE_CLASS __declspec(dllimport)
#endif

#include <sirius_video_source_capturer.h>

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
						class EXP_D3D11_DESKTOP_CAPTURE_CLASS capturer
							: public sirius::library::video::source::capturer
						{
						public:
							class core;

						public:
							typedef struct EXP_D3D11_DESKTOP_CAPTURE_CLASS _context_t
								: public sirius::library::video::source::capturer::context_t
							{
								int32_t occlusion_msg;
								int32_t single_output;
								HANDLE	unexpected_error_event;
								HANDLE	expected_error_event;
								RECT	rect;
								int32_t video_submedia_type;
								int32_t video_memory_type;
								RECT	crop;
								_context_t(void)
									: occlusion_msg(0)
									, single_output(-1)
									, unexpected_error_event(NULL)
									, expected_error_event(NULL)
									, video_submedia_type(sirius::library::video::source::d3d11::desktop::capturer::video_submedia_type_t::rgb32)
									, video_memory_type(sirius::library::video::source::d3d11::desktop::capturer::video_memory_type_t::host)
								{
									memset(&rect, 0x00, sizeof(rect));
									memset(&crop, 0x00, sizeof(crop));
								}

								~_context_t(void)
								{

								}
							} context_t;

							capturer(void);
							virtual ~capturer(void);

							int32_t initialize(sirius::library::video::source::d3d11::desktop::capturer::context_t * context);
							int32_t release(void);

							int32_t start(void);
							int32_t stop(void);

							int32_t update_application_window(bool * occluded);
						private:
							sirius::library::video::source::d3d11::desktop::capturer::core * _core;
						};
					};
				};
			};
		};
	};
};

#endif
