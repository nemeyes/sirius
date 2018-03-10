#ifndef _LIBPNG_COMPRESSOR_H_
#define _LIBPNG_COMPRESSOR_H_

#include "sirius_partial_png_compressor.h"

#include <libimagequant.h>

#include <atlbase.h>
#include <d3d11.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <setjmp.h>
#include <png.h>  /* if this include fails, you need to install libpng (e.g. libpng-devel package) and run ./configure */


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
					namespace libpng
					{
						class compressor
							: public sirius::library::video::transform::codec::processor
						{
						public:
							//compression level
							static const int32_t Z_BEST_COMPRESSION = 9;
							static const int32_t Z_BEST_SPEED = 1;
							static const int32_t Z_IPP_COMPRESSION = -2;

							//compression strategy
							static const int32_t Z_DEFAULT_STRATEGY = 0;
							static const int32_t Z_FILTERED = 1;
							static const int32_t Z_HUFFMAN_ONLY = 2;
							static const int32_t Z_RLE = 3;

							typedef enum
							{
								RWPNG_NONE,
								RWPNG_SRGB, // sRGB chunk
								RWPNG_ICCP, // used ICC profile
								RWPNG_ICCP_WARN_GRAY, // ignore and warn about GRAY ICC profile
								RWPNG_GAMA_CHRM, // used gAMA and cHRM
								RWPNG_GAMA_ONLY, // used gAMA only (i.e. not sRGB)
								RWPNG_COCOA, // Colors handled by Cocoa reader
							} png_color_transform;

							typedef struct _png_rgba_t
							{
								unsigned char r, g, b, a;
							} png_rgba_t;

							typedef struct _png_chunk_t
							{
								struct _png_chunk_t *	next;
								unsigned char *	data;
								size_t size;
								unsigned char name[5];
								unsigned char location;
							} png_chunk_t;

							typedef struct _png24_image_t
							{
								jmp_buf		jmpbuf;
								uint32_t	width;
								uint32_t	height;
								size_t		file_size;
								double		gamma;
								unsigned char **	row_pointers;
								unsigned char *		rgba_data;
								png_chunk_t *		chunks;
								png_color_transform	input_color;
								png_color_transform	output_color;
							} png24_image_t;

							typedef struct _png8_image_t
							{
								jmp_buf jmpbuf;
								uint32_t width;
								uint32_t height;
								size_t maximum_file_size;
								//size_t metadata_size;
								double gamma;
								unsigned char ** row_pointers;
								unsigned char * indexed_data;
								png_chunk_t *chunks;
								unsigned int num_palette;
								png_rgba_t palette[256];
								png_color_transform output_color;
								int32_t compression_level;
							} png8_image_t;

							typedef union _png_image_t
							{
								jmp_buf			jmpbuf;
								png24_image_t	png24;
								png8_image_t	png8;
							} png_image_t;

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
								sirius::library::video::transform::codec::libpng::compressor::ibuffer_t	input;
								sirius::library::video::transform::codec::libpng::compressor::obuffer_t	output;
							} buffer_t;


							typedef struct _png_write_state_t
							{
								sirius::library::video::transform::codec::partial::png::compressor::entity_t * compressed;
								png_size_t maximum_file_size;
								png_size_t bytes_written;
								int32_t retval;
							} png_write_state_t;

						public:
							compressor(sirius::library::video::transform::codec::partial::png::compressor * front);
							~compressor(void);

							int32_t state(void);

							int32_t initialize(sirius::library::video::transform::codec::partial::png::compressor::context_t * context);
							int32_t release(void);

							int32_t play(void);
							int32_t pause(void);
							int32_t stop(void);

							int32_t compress(sirius::library::video::transform::codec::partial::png::compressor::entity_t * input, sirius::library::video::transform::codec::partial::png::compressor::entity_t * bitstream);

						private:
							static void			png_error_handler(png_structp png_ptr, png_const_charp msg);
							static void			png_write_callback(png_structp  png_ptr, png_bytep data, png_size_t length);
							static void			png_flush_callback(png_structp png_ptr);
							static void			png_set_gamma(png_infop info_ptr, png_structp png_ptr, double gamma, png_color_transform color);
							static void			png_free_chunks(png_chunk_t * chunk);
							static int32_t		write_png_begin(png_image_t * img, png_structpp png_ptr_p, png_infopp info_ptr_p, int32_t compression_level);
							static void			write_png_end(png_infopp info_ptr_p, png_structpp png_ptr_p, png_bytepp row_pointers);
							static png_bytepp	png_create_row_pointers(png_infop info_ptr, png_structp png_ptr, unsigned char *base, unsigned int height, png_size_t rowbytes);

							int32_t				write_png_image8(png8_image_t * out, sirius::library::video::transform::codec::partial::png::compressor::entity_t * compressed);
							void				free_png_image8(png8_image_t * image);

						private:
							sirius::library::video::transform::codec::partial::png::compressor * _front;
							sirius::library::video::transform::codec::partial::png::compressor::context_t * _context;
							int32_t _state;
						};
					};
				};
			};
		};
	};
};

#endif
