#include "sirius_partial_png_compressor.h"
#include "libpng_compressor.h"
#include <sirius_image_creator.h>
#include <sirius_stringhelper.h>

#include <Simd/SimdLib.h>

#include <emmintrin.h>

#ifdef WITH_SAVE_BMP
#include <screengrab.h>
#include <wincodec.h>
unsigned long nFrame = 0;
#endif


sirius::library::video::transform::codec::libpng::compressor::compressor(sirius::library::video::transform::codec::partial::png::compressor * front)
	: _front(front)
	, _context(nullptr)
	, _state(sirius::library::video::transform::codec::partial::png::compressor::state_t::none)
	, _rgba_buffer(nullptr)
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

	_rgba_buffer = static_cast<uint8_t*>(malloc(_context->block_width * (_context->block_height << 2)));
	memset(_rgba_buffer, 0x00, _context->block_width * (_context->block_height << 2));

	allocate_io_buffers();
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

	release_io_buffers();

	if (_rgba_buffer)
		free(_rgba_buffer);
	_rgba_buffer = nullptr;

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

	sirius::library::video::transform::codec::libpng::compressor::buffer_t * iobuffer = nullptr;
	if (_state == sirius::library::video::transform::codec::partial::png::compressor::state_t::compressing)
		return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;

	_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressing;

	{
		iobuffer = _iobuffer_queue.get_available();
		if (!iobuffer)
			return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::fail;

		iobuffer->input.timestamp = input->timestamp;
		iobuffer->input.data_size = input->data_size;
		memmove((uint8_t*)iobuffer->input.data, input->data, iobuffer->input.data_size);
	}

	{
		iobuffer = _iobuffer_queue.get_pending();

		uint8_t * pixels = (uint8_t*)iobuffer->input.data;
		memcpy(_rgba_buffer, pixels, _context->block_width * _context->block_height * 4);

		liq_attr * liq = liq_attr_create();
		liq_set_speed(liq, _context->speed);
		liq_set_max_colors(liq, _context->max_colors);

		int32_t quality_percent = 90; // quality on 0-100 scale, updated upon successful remap
		png8_image_t qntpng = { 0 };
		liq_image * rgba = liq_image_create_rgba(liq, _rgba_buffer, _context->block_width, _context->block_height, _context->gamma);
		liq_image_set_memory_ownership(rgba, LIQ_OWN_ROWS | LIQ_OWN_PIXELS);

		liq_result * remap = nullptr;
		liq_error liqerr = liq_image_quantize(rgba, liq, &remap);
		if (liqerr == LIQ_OK)
		{
			liq_set_output_gamma(remap, 0.45455);
			liq_set_dithering_level(remap, _context->floyd);

			qntpng.width = liq_image_get_width(rgba);
			qntpng.height = liq_image_get_height(rgba);
			qntpng.gamma = liq_get_output_gamma(remap);
			qntpng.output_color = RWPNG_SRGB;
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
#if 0
				qntpng.palette[i].r = px.r;
				qntpng.palette[i].g = px.g;
				qntpng.palette[i].b = px.b;
				qntpng.palette[i].a = px.a;
#else
				qntpng.palette[i].r = px.b;
				qntpng.palette[i].g = px.g;
				qntpng.palette[i].b = px.r;
				qntpng.palette[i].a = px.a;
#endif
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
			qntpng.fast_compression = _context->fast_compression;
			status = write_png_image8(&qntpng, &iobuffer->output);
		}

		if (rgba)
			liq_image_destroy(rgba);
		free_png_image8(&qntpng);
		liq_attr_destroy(liq);

		bitstream->memtype = sirius::library::video::transform::codec::partial::png::compressor::video_memory_type_t::host;
		if (iobuffer->output.data_size > bitstream->data_capacity)
		{
			bitstream->data_size = bitstream->data_capacity;
			memmove(bitstream->data, iobuffer->output.data, bitstream->data_size);
		}
		else
		{
			bitstream->data_size = iobuffer->output.data_size;
			memmove(bitstream->data, iobuffer->output.data, bitstream->data_size);
		}

		iobuffer->output.data_size = 0;
	}
	_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressed;
	return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::libpng::compressor::allocate_io_buffers(void)
{
	int32_t status = sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
	_iobuffer_queue.initialize(_iobuffer, _context->nbuffer);

	for (int32_t i = 0; i < _context->nbuffer; i++)
	{
		_iobuffer[i].input.data_capacity = _context->block_width * _context->block_height * 4;
		_iobuffer[i].input.data_size = 0;
		_iobuffer[i].input.data = static_cast<uint8_t*>(malloc(_iobuffer[i].input.data_capacity));

		_iobuffer[i].output.data_capacity = sirius::library::video::transform::codec::libpng::compressor::MAX_PNG_SIZE;
		_iobuffer[i].output.data_size = 0;
		_iobuffer[i].output.data = static_cast<uint8_t*>(malloc(_iobuffer[i].output.data_capacity));
	}

	return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::libpng::compressor::release_io_buffers(void)
{
	for (int32_t i = 0; i < _context->nbuffer; i++)
	{
		if (_iobuffer[i].input.data)
			free(_iobuffer[i].input.data);
		_iobuffer[i].input.data = nullptr;

		if (_iobuffer[i].output.data)
			free(_iobuffer[i].output.data);
		_iobuffer[i].output.data = nullptr;
	}

	_iobuffer_queue.release();
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

	sirius::library::video::transform::codec::libpng::compressor::obuffer_t * compressed = write_state->compressed;
	if (compressed) //sending png per image
	{
		memcpy(compressed->data + compressed->data_size, data, length);
		compressed->data_size += length;
	}
}

void sirius::library::video::transform::codec::libpng::compressor::png_flush_callback(png_structp png_ptr)
{

}

void sirius::library::video::transform::codec::libpng::compressor::png_set_gamma(png_infop info_ptr, png_structp png_ptr, double gamma, png_color_transform color)
{
	//if (color != RWPNG_GAMA_ONLY && color != RWPNG_NONE) 
	//	png_set_gAMA(png_ptr, info_ptr, gamma);

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

int32_t sirius::library::video::transform::codec::libpng::compressor::write_png_begin(png_image_t * img, png_structpp png_ptr_p, png_infopp info_ptr_p, int32_t fast_compression)
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

#if defined(WITH_IPP)
	png_set_compression_level(*png_ptr_p, Z_IPP_COMPRESSION);
#else
	png_set_compression_level(*png_ptr_p, fast_compression ? Z_BEST_SPEED : Z_BEST_COMPRESSION);
#endif
	png_set_compression_mem_level(*png_ptr_p, fast_compression ? 9 : 5); // judging by optipng results, smaller mem makes libpng compress slightly better

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

int32_t sirius::library::video::transform::codec::libpng::compressor::write_png_image8(png8_image_t * out, sirius::library::video::transform::codec::libpng::compressor::obuffer_t * compressed)
{
	png_structp png_ptr;
	png_infop info_ptr;

	if (out->num_palette > 256)
		return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::invalid_argument;

	int32_t retval = write_png_begin((png_image_t*)out, &png_ptr, &info_ptr, out->fast_compression);
	if (retval)
		return retval;

	png_write_state_t write_state;
	write_state.maximum_file_size = out->maximum_file_size;
	write_state.retval = sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
	write_state.compressed = compressed;
	png_set_write_fn(png_ptr, &write_state, png_write_callback, png_flush_callback);

	//if(_context->memtype==sirius::library::video::transform::codec::partial::png::compressor::video_memory_type_t::host)
	//	png_set_bgr(png_ptr);

#if defined(WITH_IPP)
	png_set_compression_strategy(png_ptr, Z_DEFAULT_STRATEGY);
#else
	png_set_compression_strategy(png_ptr, Z_RLE);
#endif
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
	out->metadata_size = 0;
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

		out->metadata_size += chunk->size + 12;
		chunk = chunk->next;
		chunk_num++;
	}

	png_set_IHDR(png_ptr, info_ptr, out->width, out->height, sample_depth, PNG_COLOR_TYPE_PALETTE, 0, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_BASE);

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

	if (num_trans > 0) {
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
