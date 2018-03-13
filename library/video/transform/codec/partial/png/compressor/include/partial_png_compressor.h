#ifndef _PARTIAL_PNG_COMPRESSOR_H_
#define _PARTIAL_PNG_COMPRESSOR_H_

#include "sirius_partial_png_compressor.h"

#include <atlbase.h>
#include <d3d11.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <setjmp.h>
#include <map>
#include <vector>

#include <sirius_template_queue.h>
#include <sirius_video_processor.h>

#include "libpng_compressor.h"

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
						namespace png
						{
							class compressor::core
								: public sirius::library::video::transform::codec::processor
							{
							public:
								static const int32_t	MAX_IO_BUFFERS		= 60;
								static const int32_t	MAX_PNG_SIZE		= 1024 * 1024 * 1;
								static const int32_t	MACRO_BLOCK_WIDTH	= 4;
								static const int32_t	MACRO_BLOCK_HEIGHT	= 4;
								typedef struct _ibuffer_t
								{
									void *		data;
									int32_t		data_size;
									int32_t		data_capacity;
									int32_t		x;
									int32_t		y;
									int32_t		width;
									int32_t		height;
									long long	timestamp;
									bool		force_fullmode;
									_ibuffer_t(void)
										: data(nullptr)
										, data_size(0)
										, data_capacity(0)
										, x(0)
										, y(0)
										, width(0)
										, height(0)
										, timestamp(0)
										, force_fullmode(false)
									{}
								} ibuffer_t;

								typedef struct _compressed_cache_image_t
								{
									uint8_t	*	cache;
									int32_t		cache_size;
									int32_t		cache_capacity;
									_compressed_cache_image_t(void)
										: cache_capacity(1024 * 512)
									{
										cache = static_cast<uint8_t*>(malloc(cache_capacity));
									}

									~_compressed_cache_image_t(void)
									{
										if (cache)
											free(cache);
										cache = nullptr;
									}
								} compressed_cache_image_t;

								typedef struct _macro_block_t
								{
									int16_t				index;
									_macro_block_t *	next;
								} macro_block_t;

								typedef struct _bounding_box_t
								{
									unsigned char *		mb;
									int16_t				bindex;
									int16_t				eindex;
									int16_t				hcnt;
									int16_t				vcnt;
									int16_t				cnt;
								} bounding_box_t;




								typedef struct _buffer_t
								{
									sirius::library::video::transform::codec::partial::png::compressor::core::ibuffer_t	input;
								} buffer_t;

							public:
								core(sirius::library::video::transform::codec::partial::png::compressor * front);
								~core(void);

								int32_t state(void);

								int32_t initialize(sirius::library::video::transform::codec::partial::png::compressor::context_t * context);
								int32_t release(void);

								int32_t play(void);
								int32_t pause(void);
								int32_t stop(void);
								int32_t invalidate(void);

								int32_t compress(sirius::library::video::transform::codec::partial::png::compressor::entity_t * input, sirius::library::video::transform::codec::partial::png::compressor::entity_t * bitstream);
								int32_t compress(sirius::library::video::transform::codec::partial::png::compressor::entity_t * input);

							private:
								static unsigned __stdcall process_callback(void * param);
								void	process_psend_indexed(void);
								void	process_bsend_indexed(void);
								void	process_coordinates(void);
								int32_t allocate_io_buffers(void);
								int32_t release_io_buffers(void);

								bounding_box_t * create_bounding_box(int16_t macro_block_index);
								void			 destroy_bounding_box(bounding_box_t * bb);

								void			copy(sirius::library::video::transform::codec::partial::png::compressor::entity_t * input, sirius::library::video::transform::codec::partial::png::compressor::core::buffer_t * iobuffer);

							private:
								sirius::library::video::transform::codec::partial::png::compressor *			_front;
								sirius::library::video::transform::codec::partial::png::compressor::context_t * _context;

								CRITICAL_SECTION	_cs;
								int32_t				_state;
								bool				_run;
								HANDLE				_thread;
								HANDLE				_event;
								bool				_invalidate;

								sirius::library::video::transform::codec::partial::png::compressor::core::buffer_t										_iobuffer[MAX_IO_BUFFERS];
								sirius::queue<sirius::library::video::transform::codec::partial::png::compressor::core::buffer_t>						_iobuffer_queue;
								sirius::library::video::transform::codec::libpng::compressor *															_real_compressor;								
								std::map<int32_t, sirius::library::video::transform::codec::partial::png::compressor::core::compressed_cache_image_t*>	_indexed_cache_image;
								
								//std::vector<sirius::library::video::transform::codec::partial::png::compressor::core::bounding_box_t*>					_bounding_boxes;
								
								int32_t				_prev_x;
								int32_t				_prev_y;
								int32_t				_prev_width;
								int32_t				_prev_height;
							};
						};
					};
				};
			};
		};
	};
};


#endif
