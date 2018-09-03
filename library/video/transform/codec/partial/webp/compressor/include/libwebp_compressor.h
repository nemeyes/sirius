#ifndef _LIBWEBP_COMPRESSOR_H_
#define _LIBWEBP_COMPRESSOR_H_

#include "sirius_partial_webp_compressor.h"

#include <atlbase.h>
#include <d3d11.h>
#include <stdio.h>
#include <stdint.h>
#include <webp/encode.h>  

#include <sirius_template_queue.h>
#include <sirius_video_processor.h>

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
					namespace libwebp
					{
						class compressor
							: public sirius::library::video::transform::codec::processor
						{
						public:
							typedef struct _ibuffer_t
							{
								void *		data;
								int32_t		data_size;
								int32_t		data_capacity;
								long long	timestamp;
								_ibuffer_t(void)
									: data(nullptr)
									, timestamp(0)
								{}
							} ibuffer_t;

							typedef struct _obuffer_t
							{
								uint8_t *	data;
								int32_t		data_size;
								int32_t		data_capacity;
								long long	timestamp;
								_obuffer_t(void)
									: data(nullptr)
									, data_size(0)
									, data_capacity(0)
									, timestamp(0)
								{}
							} obuffer_t;

							typedef struct _buffer_t
							{
								sirius::library::video::transform::codec::libwebp::compressor::ibuffer_t	input;
								sirius::library::video::transform::codec::libwebp::compressor::obuffer_t	output;
							} buffer_t;

						public:
							compressor(sirius::library::video::transform::codec::partial::webp::compressor * front);
							~compressor(void);

							int32_t state(void);

							int32_t initialize(sirius::library::video::transform::codec::partial::webp::compressor::context_t * context);
							int32_t release(void);

							int32_t play(void);
							int32_t pause(void);
							int32_t stop(void);

							int32_t compress(sirius::library::video::transform::codec::partial::webp::compressor::entity_t * input, sirius::library::video::transform::codec::partial::webp::compressor::entity_t * bitstream);

						private:
							sirius::library::video::transform::codec::partial::webp::compressor *				_front;
							sirius::library::video::transform::codec::partial::webp::compressor::context_t *	_context;
							int32_t																				_state;
						};
					};
				};
			};
		};
	};
};

#endif
