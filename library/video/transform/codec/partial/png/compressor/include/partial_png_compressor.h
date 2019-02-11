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

								static const int32_t UNLABELED = 0;
								static const int32_t LABELED = 1;
								typedef struct _connected_component_t
								{
									uint32_t number;
									uint32_t left;
									uint32_t top;
									uint32_t right;
									uint32_t bottom;
									std::vector<uint64_t> elements;
									_connected_component_t(int32_t lable_number)
										: number(lable_number)
										, left(1280)
										, top(720)
										, right(0)
										, bottom(0)
									{}
								} connected_component_t;

								typedef struct _indexed_thread_context_t
								{
									int32_t			index;

									char *			compressed_buffer;
									int32_t			compressed_buffer_size;


									int32_t *		pindex;
									uint8_t *		pcompressed;
									int32_t *		pcapacity;
									int32_t *		plength;

									int32_t *		cached_index;
									uint8_t *		cached_compressed;
									int32_t *		cached_capacity;
									int32_t *		cached_length;

									BOOL			run;
									HANDLE			thread;
									HANDLE			signal;
									HANDLE			available;
									sirius::library::video::transform::codec::partial::png::compressor::entity_t	input;
									sirius::library::video::transform::codec::partial::png::compressor::entity_t	output;
									sirius::library::video::transform::codec::libpng::compressor *					real_compressor;
									sirius::library::video::transform::codec::partial::png::compressor::core *		parent;
								} indexed_thread_context_t;

								typedef struct _coordinated_thread_context_t
								{
									char *			compressed_buffer;
									int32_t			compressed_buffer_size;

									uint8_t **		rows;
									uint8_t *		pcompressed;
									int32_t *		plength;

									BOOL			run;
									HANDLE			thread;
									HANDLE			signal;
									HANDLE			available;
									sirius::library::video::transform::codec::partial::png::compressor::entity_t	input;
									sirius::library::video::transform::codec::partial::png::compressor::entity_t	output;
									sirius::library::video::transform::codec::libpng::compressor *					real_compressor;
									sirius::library::video::transform::codec::partial::png::compressor::core *		parent;
								} coordinated_thread_context_t;

								typedef struct _ccl_info_t
								{
									int32_t x;
									int32_t y;
									int32_t width;
									int32_t height;
									_ccl_info_t(void)
										: x(0)
										, y(0)
										, width(0)
										, height(0)
									{}
									
									_ccl_info_t(const _ccl_info_t & clone)
									{
										x = clone.x;
										y = clone.y;
										width = clone.width;
										height = clone.height;
									}

									_ccl_info_t & operator=(const _ccl_info_t & clone)
									{
										x = clone.x;
										y = clone.y;
										width = clone.width;
										height = clone.height;
										return *this;
									}
								} ccl_info_t;

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
								static unsigned __stdcall process_indexed_encoding_callback(void * param);
								static unsigned __stdcall process_coordinated_encoding_callback(void * param);

								void	process_indexed(void);
								void	process_threaded_indexed_encoding(indexed_thread_context_t * thread_ctx);
								void	process_threaded_indexed(void);

								void	process_coordinated(void);
								void	process_threaded_coordinated_encoding(coordinated_thread_context_t * thread_ctx);
								void	process_threaded_coordinated(void);

								void	connect_component(unsigned short * pseudo_stack, std::map<uint64_t, uint32_t> * bfgs, connected_component_t * cc, uint32_t width, uint32_t height, uint32_t x, uint32_t y);

								//void	process_psend_indexed(void);
								//void	process_bsend_indexed(void);
								//void	process_coordinates(void);
								int32_t allocate_io_buffers(void);
								int32_t release_io_buffers(void);

								void	copy(sirius::library::video::transform::codec::partial::png::compressor::entity_t * input, sirius::library::video::transform::codec::partial::png::compressor::core::buffer_t * iobuffer);
								void	md5_hash(uint8_t * data, int32_t data_size, char* hash);

								
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
								
								int32_t				_prev_x;
								int32_t				_prev_y;
								int32_t				_prev_width;
								int32_t				_prev_height;
								bool				_prev_force_fullmode;


								LARGE_INTEGER		_frequency;
							};
						};
					};
				};
			};
		};
	};
};


#endif
