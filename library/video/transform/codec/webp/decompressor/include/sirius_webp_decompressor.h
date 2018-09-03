#ifndef _SIRIUS_LIBWEBP_DECOMPRESSOR_H_
#define _SIRIUS_LIBWEBP_DECOMPRESSOR_H_

#include <sirius_video_decompressor.h>

#if defined(EXPORT_LIBWEBP_DECOMPRESSOR_LIB)
#define EXP_LIBWEBP_DECOMPRESSOR_CLASS __declspec(dllexport)
#else
#define EXP_LIBWEBP_DECOMPRESSOR_CLASS __declspec(dllimport)
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
					namespace webp
					{
						class EXP_LIBWEBP_DECOMPRESSOR_CLASS decompressor
							: public sirius::library::video::transform::codec::decompressor
						{
						public:
							class core;
						public:
							typedef struct EXP_LIBWEBP_DECOMPRESSOR_CLASS _context_t
								: public sirius::library::video::transform::codec::decompressor::context_t
							{
								_context_t(void);
								_context_t(const _context_t & clone);
								_context_t & operator=(const _context_t & clone);
							} context_t;

						public:
							decompressor(void);
							virtual ~decompressor(void);

							virtual int32_t initialize(sirius::library::video::transform::codec::webp::decompressor::context_t * context);
							virtual int32_t release(void);

							virtual int32_t decompress(sirius::library::video::transform::codec::webp::decompressor::entity_t * input, sirius::library::video::transform::codec::webp::decompressor::entity_t * output);

						private:
							sirius::library::video::transform::codec::webp::decompressor::context_t * _context;
							sirius::library::video::transform::codec::webp::decompressor::core * _core;
						};
					};
				};
			};
		};
	};
};








#endif
