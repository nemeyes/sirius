#ifndef _SIRIUS_D3D11_VIDEO_CAPTURE_H_
#define _SIRIUS_D3D11_VIDEO_CAPTURE_H_

#if defined(EXPORT_D3D11_CAPTURER_LIB)
#define EXP_D3D11_CAPTURE_CLASS __declspec(dllexport)
#else
#define EXP_D3D11_CAPTURE_CLASS __declspec(dllimport)
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
					class EXP_D3D11_CAPTURE_CLASS capturer
						: public sirius::library::video::source::capturer
					{
					public:
						class core;
						class proxy;
					public:
						static sirius::library::video::source::d3d11::capturer & instance(void);

						typedef struct EXP_D3D11_CAPTURE_CLASS _context_t
							: public sirius::library::video::source::capturer::context_t
						{
							static _context_t & instance(void)
							{
								static _context_t _instance;
								return _instance;
							}
						} context_t;

						int32_t initialize(sirius::library::video::source::capturer::context_t * context);
						int32_t release(void);
						int32_t start(void);
						int32_t stop(void);
						int32_t pause(void);

					private:
						capturer(void);
						virtual ~capturer(void);

					private:
						sirius::library::video::source::d3d11::capturer::core * _core;
					};
				};
			};
		};
	};
};

#endif
