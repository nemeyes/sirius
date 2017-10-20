#ifndef _SIRIUS_VIDEO_DECOMPRESSOR_H_
#define _SIRIUS_VIDEO_DECOMPRESSOR_H_

#include <sirius_video.h>

#if defined(EXPORT_VIDEO_DECOMPRESSOR_LIB)
#define EXP_VIDEO_DECOMPRESSOR_CLASS __declspec(dllexport)
#else
#define EXP_VIDEO_DECOMPRESSOR_CLASS __declspec(dllimport)
#endif

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
					class EXP_VIDEO_DECOMPRESSOR_CLASS decompressor
						: public sirius::library::video::base
					{
					public:
						typedef struct EXP_VIDEO_DECOMPRESSOR_CLASS _context_t
						{
							int32_t memtype;
							void *	device;
							int32_t width; //output width
							int32_t height; //output height
							int32_t nbuffer;
							_context_t(void);
							_context_t(const sirius::library::video::transform::codec::decompressor::_context_t & clone);
							_context_t & operator=(const _context_t & clone);
						} context_t;

					public:
						decompressor(void);
						virtual ~decompressor(void);

						virtual int32_t initialize(sirius::library::video::transform::codec::decompressor::context_t * context);
						virtual int32_t release(void);

						virtual int32_t decompress(sirius::library::video::transform::codec::decompressor::entity_t * input, sirius::library::video::transform::codec::decompressor::entity_t * output);
					};
				};
			};
		};
	};
};


#endif
