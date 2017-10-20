#ifndef _SIRIUS_DDRAW_RENDERER_H_
#define _SIRIUS_DDRAW_RENDERER_H_

#include <sirius_video_renderer.h>

#if defined(EXPORT_DDRAW_RENDERER_LIB)
#define EXP_DDRAW_RENDERER_CLASS __declspec(dllexport)
#else
#define EXP_DDRAW_RENDERER_CLASS __declspec(dllimport)
#endif

namespace sirius
{
	namespace library
	{
		namespace video
		{
			namespace sink
			{
				namespace ddraw
				{
					class EXP_DDRAW_RENDERER_CLASS renderer
						: public sirius::library::video::sink::renderer
					{
					public:
						class core;
					public:
						renderer(void);
						virtual ~renderer(void);

						int32_t initialize(void * context);
						int32_t release(void);
						int32_t render(sirius::library::video::sink::ddraw::renderer::entity_t * decoded);

					private:
						sirius::library::video::sink::ddraw::renderer::core * _core;
					};
				};
			};
		};
	};
};

#endif