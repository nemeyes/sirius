#ifndef _SIRIUS_PARTIAL_WEBP_COMPRESSOR_H_
#define _SIRIUS_PARTIAL_WEBP_COMPRESSOR_H_

#include <sirius_video_compressor.h>

#if defined(EXPORT_PARTIAL_WEBP_COMPRESSOR_LIB)
#define EXP_PARTIAL_WEBP_COMPRESSOR_CLASS __declspec(dllexport)
#else
#define EXP_PARTIAL_WEBP_COMPRESSOR_CLASS __declspec(dllimport)
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
					namespace partial
					{
						namespace webp
						{
							class EXP_PARTIAL_WEBP_COMPRESSOR_CLASS compressor
								: public sirius::library::video::transform::codec::compressor
							{
							public:
								class core;
							public:
								typedef struct EXP_PARTIAL_WEBP_COMPRESSOR_CLASS _context_t
									: public sirius::library::video::transform::codec::compressor::context_t
								{
									int32_t block_width;
									int32_t block_height;
									int32_t mb_width;
									int32_t mb_height;
									float	quality;
									int32_t	method;
									bool	binvalidate;
									bool	indexed_video;
									int32_t nthread;
									_context_t(void);
									_context_t(const _context_t & clone);
									_context_t & operator=(const _context_t & clone);
								} context_t;

							public:
								compressor(void);
								virtual ~compressor(void);

								virtual int32_t initialize(sirius::library::video::transform::codec::partial::webp::compressor::context_t * context);
								virtual int32_t release(void);

								virtual int32_t play(void);
								virtual int32_t pause(void);
								virtual int32_t stop(void);
								virtual int32_t invalidate(void);

								virtual int32_t compress(sirius::library::video::transform::codec::partial::webp::compressor::entity_t * input, sirius::library::video::transform::codec::partial::webp::compressor::entity_t * bitstream);
								virtual int32_t compress(sirius::library::video::transform::codec::partial::webp::compressor::entity_t * input);
									
							private:
								sirius::library::video::transform::codec::partial::webp::compressor::context_t * _context;
								sirius::library::video::transform::codec::partial::webp::compressor::core * _core;
							};
						};
					};
				};
			};
		};
	};
};








#endif
