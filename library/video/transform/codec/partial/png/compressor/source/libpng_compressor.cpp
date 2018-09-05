#include "sirius_partial_png_compressor.h"
#include "libpng_compressor.h"
#include <sirius_image_creator.h>
#include <sirius_stringhelper.h>

#include <emmintrin.h>
#include <omp.h>

#ifdef WITH_SAVE_BMP
#include <screengrab.h>
#include <wincodec.h>
unsigned long nFrame = 0;
#endif


sirius::library::video::transform::codec::libpng::compressor::compressor(sirius::library::video::transform::codec::partial::png::compressor * front)
	: _front(front)
	, _context(nullptr)
	, _state(sirius::library::video::transform::codec::partial::png::compressor::state_t::none)
{

}

sirius::library::video::transform::codec::libpng::compressor::~compressor(void)
{
	_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::none;
}

int32_t sirius::library::video::transform::codec::libpng::compressor::state(void)
{
	return _state;
}

int32_t sirius::library::video::transform::codec::libpng::compressor::initialize(sirius::library::video::transform::codec::partial::png::compressor::context_t * context)
{
	if (!context)
		return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::fail;

	_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::initializing;

	_context = context;

	_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::initialized;
	return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::libpng::compressor::release(void)
{
	if (!((_state == sirius::library::video::transform::codec::partial::png::compressor::state_t::initialized) ||
		(_state == sirius::library::video::transform::codec::partial::png::compressor::state_t::compressing) ||
		(_state == sirius::library::video::transform::codec::partial::png::compressor::state_t::compressed)))
		return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::fail;

	_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::releasing;


	_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::released;
	return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::libpng::compressor::play(void)
{
	return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::libpng::compressor::pause(void)
{
	return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::libpng::compressor::stop(void)
{
	return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::libpng::compressor::compress(sirius::library::video::transform::codec::partial::png::compressor::entity_t * input, sirius::library::video::transform::codec::partial::png::compressor::entity_t * bitstream)
{
	int32_t status = sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;

	if (_state == sirius::library::video::transform::codec::partial::png::compressor::state_t::compressing)
		return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;

	_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressing;

	liq_attr * liq = liq_attr_create();
	liq_set_speed(liq, _context->speed, _context->posterization, _context->use_dither_map, _context->use_contrast_maps);
	if (_context->max_colors == 0)
		liq_set_quality(liq, _context->min_quality, _context->max_quality);
	else
		liq_set_max_colors(liq, _context->max_colors);
	if(_context->posterization)
		liq_set_min_posterization(liq, 2);
	else
		liq_set_min_posterization(liq, 0);

	int32_t quality_percent = 90; // quality on 0-100 scale, updated upon successful remap
	png8_image_t qntpng = { 0 };
	liq_image * rgba = liq_image_create_rgba_rows(liq, (void**)input->data, input->width, input->height, _context->gamma);
	//liq_image_set_memory_ownership(rgba, LIQ_OWN_ROWS | LIQ_OWN_PIXELS);

	liq_result * remap = nullptr;
	liq_error liqerr = LIQ_OK;
#if 0
	liqerr = liq_image_quantize(rgba, liq, &remap);
#else
	remap = liq_quantize_image(liq, rgba);
#endif
	if (remap)
	{
		liq_set_output_gamma(remap, _context->gamma);
		liq_set_dithering_level(remap, _context->floyd);

		qntpng.width = liq_image_get_width(rgba);
		qntpng.height = liq_image_get_height(rgba);
		qntpng.gamma = liq_get_output_gamma(remap);
		qntpng.output_color = RWPNG_NONE;
		qntpng.indexed_data = static_cast<uint8_t*>(malloc(qntpng.height * qntpng.width));
		qntpng.row_pointers = static_cast<uint8_t**>(malloc(qntpng.height * sizeof(qntpng.row_pointers[0])));

		if (!qntpng.indexed_data || !qntpng.row_pointers)
		{
			if (rgba)
				liq_image_destroy(rgba);

			free_png_image8(&qntpng);

			liq_attr_destroy(liq);

			return sirius::library::video::transform::codec::compressor::err_code_t::out_of_memory_error;
		}

		//#pragma omp parallel for
		for (size_t row = 0; row < qntpng.height; row++)
			qntpng.row_pointers[row] = qntpng.indexed_data + row * qntpng.width;

		const liq_palette *palette = liq_get_palette(remap);
		qntpng.num_palette = palette->count;

		liqerr = liq_write_remapped_image_rows(remap, rgba, qntpng.row_pointers);
		if (liqerr != LIQ_OK)
			status = sirius::library::video::transform::codec::compressor::err_code_t::out_of_memory_error;

		palette = liq_get_palette(remap);
		qntpng.num_palette = palette->count;
		for (unsigned int i = 0; i < palette->count; i++)
		{
			const liq_color px = palette->entries[i];
			qntpng.palette[i].r = px.b;
			qntpng.palette[i].g = px.g;
			qntpng.palette[i].b = px.r;
			qntpng.palette[i].a = px.a;
		}

		double palette_error = liq_get_quantization_error(remap);
		if (palette_error >= 0)
			quality_percent = liq_get_quantization_quality(remap);
		liq_result_destroy(remap);
	}
	else if (liqerr == LIQ_QUALITY_TOO_LOW)
	{
		status = sirius::library::video::transform::codec::compressor::err_code_t::too_low_quality;
	}
	else
	{
		status = sirius::library::video::transform::codec::compressor::err_code_t::invalid_argument;
	}

	if (status == sirius::library::video::transform::codec::compressor::err_code_t::success)
	{
		qntpng.compression_level = _context->compression_level;
		status = write_png_image8(&qntpng, bitstream);
	}

	if (rgba)
		liq_image_destroy(rgba);
	free_png_image8(&qntpng);
	liq_attr_destroy(liq);

	_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressed;
	return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
}

void sirius::library::video::transform::codec::libpng::compressor::png_error_handler(png_structp png_ptr, png_const_charp msg)
{

}

void sirius::library::video::transform::codec::libpng::compressor::png_write_callback(png_structp  png_ptr, png_bytep data, png_size_t length)
{
	png_write_state_t * write_state = (png_write_state_t*)png_get_io_ptr(png_ptr);
	if (write_state->retval != sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success)
		return;

	sirius::library::video::transform::codec::partial::png::compressor::entity_t * compressed = write_state->compressed;
	if (compressed) //sending png per image
	{
		memcpy((uint8_t*)compressed->data + compressed->data_size, data, length);
		compressed->data_size += length;
	}
}

void sirius::library::video::transform::codec::libpng::compressor::png_flush_callback(png_structp png_ptr)
{

}

void sirius::library::video::transform::codec::libpng::compressor::png_set_gamma(png_infop info_ptr, png_structp png_ptr, double gamma, png_color_transform color)
{
	if (color != RWPNG_GAMA_ONLY && color != RWPNG_NONE) 
		png_set_gAMA(png_ptr, info_ptr, gamma);

	if (color == RWPNG_SRGB)
		png_set_sRGB(png_ptr, info_ptr, 0); // 0 = Perceptual
}

void sirius::library::video::transform::codec::libpng::compressor::png_free_chunks(png_chunk_t * chunk)
{
	if (!chunk)
		return;
	png_free_chunks(chunk->next);
	free(chunk->data);
	free(chunk);
}

int32_t sirius::library::video::transform::codec::libpng::compressor::write_png_begin(png_image_t * img, png_structpp png_ptr_p, png_infopp info_ptr_p, int32_t compression_level)
{
	*png_ptr_p = png_create_write_struct(PNG_LIBPNG_VER_STRING, img, NULL, NULL);

	if (!(*png_ptr_p))
	{
		return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::libpng_init_error;
	}

	*info_ptr_p = png_create_info_struct(*png_ptr_p);
	if (!(*info_ptr_p))
	{
		png_destroy_write_struct(png_ptr_p, NULL);
		return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::libpng_init_error;
	}

	// setjmp() must be called in every function that calls a PNG-writing
	// libpng function, unless an alternate error handler was installed--
	// but compatible error handlers must either use longjmp() themselves
	// (as in this program) or exit immediately, so here we go:

	if (setjmp(img->jmpbuf))
	{
		png_destroy_write_struct(png_ptr_p, info_ptr_p);
		return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::libpng_init_error;
	}

	png_set_compression_level(*png_ptr_p, (compression_level <= 0) ? Z_BEST_SPEED : compression_level);
	png_set_compression_mem_level(*png_ptr_p, 9); // Smaller values use less memory but are slower, while higher values use more memory to gain compression speed.

	return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
}

void sirius::library::video::transform::codec::libpng::compressor::write_png_end(png_infopp info_ptr_p, png_structpp png_ptr_p, png_bytepp row_pointers)
{
	png_write_info(*png_ptr_p, *info_ptr_p);

	png_set_packing(*png_ptr_p);

	png_write_image(*png_ptr_p, row_pointers);

	png_write_end(*png_ptr_p, NULL);

	png_destroy_write_struct(png_ptr_p, info_ptr_p);
}

png_bytepp sirius::library::video::transform::codec::libpng::compressor::png_create_row_pointers(png_infop info_ptr, png_structp png_ptr, unsigned char * base, unsigned int height, png_size_t rowbytes)
{
	if (!rowbytes) 
	{
		rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	}

	png_bytepp row_pointers = static_cast<png_bytepp>(malloc(height * sizeof(row_pointers[0])));
	if (!row_pointers) 
		return NULL;
	
	for (size_t row = 0; row < height; row++) 
		row_pointers[row] = base + row * rowbytes;
	
	return row_pointers;
}

int32_t sirius::library::video::transform::codec::libpng::compressor::write_png_image8(png8_image_t * out, sirius::library::video::transform::codec::partial::png::compressor::entity_t * compressed)
{
	png_structp png_ptr;
	png_infop info_ptr;

	if (out->num_palette > 256)
		return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::invalid_argument;

	int32_t retval = write_png_begin((png_image_t*)out, &png_ptr, &info_ptr, out->compression_level);
	if (retval)
		return retval;

	png_write_state_t write_state;
	write_state.maximum_file_size = out->maximum_file_size;
	write_state.retval = sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
	write_state.compressed = compressed;
	png_set_write_fn(png_ptr, &write_state, png_write_callback, png_flush_callback);

	png_set_compression_strategy(png_ptr, Z_RLE);
	
	// Palette images generally don't gain anything from filtering
	png_set_filter(png_ptr, PNG_FILTER_TYPE_BASE, PNG_FILTER_VALUE_NONE);

	png_set_gamma(info_ptr, png_ptr, out->gamma, out->output_color);

	// set the image parameters appropriately
	int32_t sample_depth;
	if (out->num_palette <= 2)
		sample_depth = 1;
	else if (out->num_palette <= 4)
		sample_depth = 2;
	else if (out->num_palette <= 16)
		sample_depth = 4;
	else
		sample_depth = 8;

	png_chunk_t * chunk = out->chunks;
	int32_t chunk_num = 0;
	while (chunk)
	{
		png_unknown_chunk pngchunk;

		pngchunk.size = chunk->size;
		pngchunk.data = chunk->data;
		pngchunk.location = chunk->location;
		memcpy(pngchunk.name, chunk->name, 5);
		png_set_unknown_chunks(png_ptr, info_ptr, &pngchunk, 1);

#if defined(PNG_HAVE_IHDR) && PNG_LIBPNG_VER < 10600
		png_set_unknown_chunk_location(png_ptr, info_ptr, chunk_num, pngchunk.location ? pngchunk.location : PNG_HAVE_IHDR);
#endif
		chunk = chunk->next;
		chunk_num++;
	}

	png_set_IHDR(png_ptr, info_ptr, out->width, out->height, sample_depth, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	png_color palette[256];
	png_byte trans[256];
	unsigned int num_trans = 0;
	for (unsigned int i = 0; i < out->num_palette; i++)
	{
		palette[i].red = out->palette[i].r;
		palette[i].green = out->palette[i].g;
		palette[i].blue = out->palette[i].b;
		trans[i] = out->palette[i].a;
		if (out->palette[i].a < 255)
			num_trans = i + 1;
	}

	png_set_PLTE(png_ptr, info_ptr, palette, out->num_palette);

	if (num_trans > 0) 
	{
		png_set_tRNS(png_ptr, info_ptr, trans, num_trans, NULL);
	}

	write_png_end(&info_ptr, &png_ptr, out->row_pointers);

	if (sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success == write_state.retval &&
		write_state.maximum_file_size &&
		write_state.bytes_written > write_state.maximum_file_size)
	{
		return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::too_large_file;
	}

	return write_state.retval;
}

void sirius::library::video::transform::codec::libpng::compressor::free_png_image8(png8_image_t * image)
{
	free(image->indexed_data);
	image->indexed_data = NULL;

	free(image->row_pointers);
	image->row_pointers = NULL;

	png_free_chunks(image->chunks);
	image->chunks = NULL;
}
