#ifndef _SIRIUS_VIDEO_RENDERER_H_
#define _SIRIUS_VIDEO_RENDERER_H_

#include <sirius_video.h>

#if defined(EXPORT_VIDEO_RENDERER_LIB)
#define EXP_VIDEO_RENDERER_CLASS __declspec(dllexport)
#else
#define EXP_VIDEO_RENDERER_CLASS __declspec(dllimport)
#endif

namespace sirius
{
	namespace library
	{
		namespace video
		{
			namespace sink
			{
				class EXP_VIDEO_RENDERER_CLASS renderer
					: public sirius::library::video::base
				{
				public:
					typedef struct EXP_VIDEO_RENDERER_CLASS _context_t
					{
						int32_t width;
						int32_t height;
						HWND	hwnd_full;
						HWND	hwnd;
						_context_t(void);
						_context_t(const _context_t & clone);
						_context_t & operator=(const _context_t & clone);
					} context_t;

					renderer(void);
					virtual ~renderer(void);

					virtual int32_t initialize(void * context);
					virtual int32_t release(void);
					virtual int32_t render(sirius::library::video::sink::renderer::entity_t * decoded);
				};
			};
		};
	};
};


#endif