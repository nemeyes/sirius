#ifndef _SIRIUS_VIDEO_COMPRESSOR_H_
#define _SIRIUS_VIDEO_COMPRESSOR_H_

#include <sirius_video.h>

#if defined(EXPORT_VIDEO_COMPRESSOR_LIB)
#define EXP_VIDEO_COMPRESSOR_CLASS __declspec(dllexport)
#else
#define EXP_VIDEO_COMPRESSOR_CLASS __declspec(dllimport)
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
					class EXP_VIDEO_COMPRESSOR_CLASS compressor
						: public sirius::library::video::base
					{
					public:
						typedef struct _state_t
						{
							static const int32_t none = 0;
							static const int32_t initializing = 1;
							static const int32_t initialized = 2;
							static const int32_t compressing = 3;
							static const int32_t compressed = 4;
							static const int32_t releasing = 5;
							static const int32_t released = 6;
						} state_t;

						typedef struct EXP_VIDEO_COMPRESSOR_CLASS _context_t
						{
							int32_t memtype;
							void *	device;
							int32_t width; //output width
							int32_t height; //output height
							int32_t nbuffer;
							_context_t(void);
							_context_t(const sirius::library::video::transform::codec::compressor::_context_t & clone);
							_context_t & operator=(const _context_t & clone);
						} context_t;

					public:
						compressor(void);
						virtual ~compressor(void);

						virtual int32_t initialize(sirius::library::video::transform::codec::compressor::context_t * context);
						virtual int32_t release(void);

						virtual int32_t play(void);
						virtual int32_t pause(void);
						virtual int32_t stop(void);
						virtual int32_t invalidate(void);

						virtual int32_t compress(sirius::library::video::transform::codec::compressor::entity_t * input, sirius::library::video::transform::codec::compressor::entity_t * bitstream);
						virtual int32_t compress(sirius::library::video::transform::codec::compressor::entity_t * input);
						//virtual void	after_process_callback(uint8_t * compressed, int32_t size, long long before_compress_timestamp, long long after_compress_timestamp);
						virtual void	after_process_callback(int32_t count, int32_t * index, uint8_t ** compressed, int32_t * size, long long before_compress_timestamp, long long after_compress_timestamp);
						virtual void	after_process_callback(int32_t index, uint8_t * compressed, int32_t size, long long before_compress_timestamp, long long after_compress_timestamp);
					};
				};
			};
		};
	};
};


#endif
