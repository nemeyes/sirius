#ifndef _SIRIUS_PNGQUANT_COMPRESSOR_H_
#define _SIRIUS_PNGQUANT_COMPRESSOR_H_

#include <sirius_video_compressor.h>

#if defined(EXPORT_PNGQUANT_COMPRESSOR_LIB)
#define EXP_PNGQUANT_COMPRESSOR_CLASS __declspec(dllexport)
#else
#define EXP_PNGQUANT_COMPRESSOR_CLASS __declspec(dllimport)
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
					namespace png
					{
						class EXP_PNGQUANT_COMPRESSOR_CLASS compressor
							: public sirius::library::video::transform::codec::compressor
						{
						public:
							class core;
						public:
							typedef struct EXP_PNGQUANT_COMPRESSOR_CLASS _context_t
								: public sirius::library::video::transform::codec::compressor::context_t
							{
								double	gamma;
								float	floyd;
								int32_t speed;
								int32_t max_colors;
								int32_t min_quality;
								int32_t max_quality;
								bool	fast_compression;
								_context_t(void);
								_context_t(const _context_t & clone);
								_context_t & operator=(const _context_t & clone);
							} context_t;

						public:
							compressor(void);
							virtual ~compressor(void);

							virtual int32_t initialize(sirius::library::video::transform::codec::png::compressor::context_t * context);
							virtual int32_t release(void);

							virtual int32_t play(void);
							virtual int32_t pause(void);
							virtual int32_t stop(void);

							virtual int32_t compress(sirius::library::video::transform::codec::png::compressor::entity_t * input);
							//virtual void	after_process_callback(uint8_t * compressed, int32_t size, long long before_compress_timestamp, long long after_compress_timestamp);
						private:
							sirius::library::video::transform::codec::png::compressor::context_t * _context;
							sirius::library::video::transform::codec::png::compressor::core * _core;
						};
					};
				};
			};
		};
	};
};








#endif
