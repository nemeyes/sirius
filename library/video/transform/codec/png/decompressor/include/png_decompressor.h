#ifndef _LIBPNG_DECOMPRESSOR_H_
#define _LIBPNG_DECOMPRESSOR_H_

#include "sirius_png_decompressor.h"
#include <png.h>
#include <sirius_circular_buffer.h>

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
						class decompressor::core
						{
						public:
							static const int32_t SIGNATURE_LENGTH = 8;
							static const int32_t MAX_VIDEO_SIZE = 1920 * 1080 * 4;

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
								size_t metadata_size;
								double gamma;
								unsigned char **row_pointers;
								unsigned char *indexed_data;
								png_chunk_t *chunks;
								unsigned int num_palette;
								png_rgba_t palette[256];
								png_color_transform output_color;
								char fast_compression;
							} png8_image_t;

							typedef union _png_image_t
							{
								jmp_buf			jmpbuf;
								png24_image_t	png24;
								png8_image_t	png8;
							} png_image_t;

							typedef struct _png_read_state_t
							{
								sirius::library::video::transform::codec::png::decompressor::core * self;
								png_size_t bytes_read;
								long long timestamp;
							} png_read_state_t;

							core(void);
							~core(void);

							int32_t initialize(sirius::library::video::transform::codec::png::decompressor::context_t * context);
							int32_t release(void);

							int32_t decompress(sirius::library::video::transform::codec::png::decompressor::entity_t * input, sirius::library::video::transform::codec::png::decompressor::entity_t * output);


						private:
							static int32_t	png_read_chunk_callback(png_structp png_ptr, png_unknown_chunkp inchunk);
							static void		png_read_callback(png_structp  png_ptr, png_bytep data, png_size_t length);
							static void		png_free_chunks(png_chunk_t * chunk);
							int32_t			read_png_image24(png24_image_t * in, long long timestamp);
							void			free_png_image24(png24_image_t * image);

						private:
							png_structp _prev_png_structp;

							uint8_t _recvd[MAX_VIDEO_SIZE];
							int32_t _recvd_size;
							int32_t _recvd_index;

							CRITICAL_SECTION _cs;
							bool	_decoding;
						};
					};
				};
			};
		};
	};
};


#endif
