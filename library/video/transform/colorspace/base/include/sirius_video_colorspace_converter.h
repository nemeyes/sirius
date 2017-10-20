#ifndef _SIRIUS_VIDEO_COLORSPACE_CONVERTER_H_
#define _SIRIUS_VIDEO_COLORSPACE_CONVERTER_H_

#include <sirius_video.h>

#if defined(EXPORT_VIDEO_COLORSPACE_CONVERTER_LIB)
#define EXP_VIDEO_COLORSPACE_CONVERTER_CLASS __declspec(dllexport)
#else
#define EXP_VIDEO_COLORSPACE_CONVERTER_CLASS __declspec(dllimport)
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
					class EXP_VIDEO_COLORSPACE_CONVERTER_CLASS converter
						: public sirius::library::video::base
					{
					public:
						converter(void);
						virtual ~converter(void);

						virtual int32_t initialize(void * context);
						virtual int32_t release(void);

						virtual int32_t convert(sirius::library::video::transform::colorspace::converter::entity_t * input, sirius::library::video::transform::colorspace::converter::entity_t * output);
					};
				};
			};
		};
	};
};


#endif
