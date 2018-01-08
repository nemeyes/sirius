#include "sirius_png_decompressor.h"
#include "png_decompressor.h"
#include <sirius_locks.h>

sirius::library::video::transform::codec::png::decompressor::core::core(void)
	: _recvd_size(0)
	, _recvd_index(0)
	, _prev_png_structp(nullptr)
	, _decoding(false)
{
	::InitializeCriticalSection(&_cs);
}

sirius::library::video::transform::codec::png::decompressor::core::~core(void)
{
	::DeleteCriticalSection(&_cs);
}

int32_t sirius::library::video::transform::codec::png::decompressor::core::initialize(sirius::library::video::transform::codec::png::decompressor::context_t * context)
{
	return sirius::library::video::transform::codec::png::decompressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::png::decompressor::core::release(void)
{
	return sirius::library::video::transform::codec::png::decompressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::png::decompressor::core::decompress(sirius::library::video::transform::codec::png::decompressor::entity_t * input, sirius::library::video::transform::codec::png::decompressor::entity_t * output)
{
	if (input->memtype == sirius::library::video::transform::codec::png::decompressor::video_memory_type_t::host)
	{
		sirius::autolock lock(&_cs);
		
		uint8_t * data = static_cast<uint8_t*>(input->data);
		_recvd_size = input->data_size;
		memcpy(_recvd, data, _recvd_size);

		_decoding = true;

		png24_image_t img = { 0 };
		read_png_image24(&img, input->timestamp);

		while (_decoding)
			::Sleep(10);

		if (output->memtype == sirius::library::video::transform::codec::png::decompressor::video_memory_type_t::host)
		{
			output->data_size = img.width * img.height * 4;
			memcpy(output->data, img.rgba_data, output->data_size);
		}

		free_png_image24(&img);
	}
	return sirius::library::video::transform::codec::png::decompressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::png::decompressor::core::png_read_chunk_callback(png_structp png_ptr, png_unknown_chunkp inchunk)
{
	if (0 == memcmp("iCCP", inchunk->name, 5) ||
		0 == memcmp("cHRM", inchunk->name, 5) ||
		0 == memcmp("gAMA", inchunk->name, 5)) 
	{
		return 0; // not handled
	}

	if (inchunk->location == 0) 
	{
		return 1; // ignore chunks with invalid location
	}

	png_chunk_t ** head = (png_chunk_t**)png_get_user_chunk_ptr(png_ptr);

	png_chunk_t * chunk = (png_chunk_t*)malloc(sizeof(png_chunk_t));
	memcpy(chunk->name, inchunk->name, 5);
	chunk->size = inchunk->size;
	chunk->location = inchunk->location;
	chunk->data = inchunk->size ? (uint8_t*)malloc(inchunk->size) : NULL;
	if (inchunk->size)
	{
		memcpy(chunk->data, inchunk->data, inchunk->size);
	}

	chunk->next = *head;
	*head = chunk;

	return 1; // marks as "handled", libpng won't store it
}

void sirius::library::video::transform::codec::png::decompressor::core::png_read_callback(png_structp png_ptr, png_bytep data, png_size_t length)
{
	png_read_state_t * read_state = (png_read_state_t*)png_get_io_ptr(png_ptr);
	sirius::library::video::transform::codec::png::decompressor::core * self = read_state->self;

	if (self)
	{
		memcpy(data, self->_recvd + self->_recvd_index, length);
		self->_recvd_index += length;

		//wchar_t debug[MAX_PATH] = { 0 };
		//_snwprintf_s(debug, sizeof(debug) - 1, L"_recvd_size=%d, recvd_index=%d \n", self->_recvd_size, self->_recvd_index);
		//::OutputDebugString(debug);
		if (self->_recvd_index >= self->_recvd_size)
		{
			self->_decoding = false;
			self->_recvd_index = 0;
		}
	}
}

void sirius::library::video::transform::codec::png::decompressor::core::png_free_chunks(png_chunk_t * chunk)
{
	if (!chunk)
		return;
	png_free_chunks(chunk->next);
	free(chunk->data);
	free(chunk);
}

int32_t sirius::library::video::transform::codec::png::decompressor::core::read_png_image24(png24_image_t * in, long long timestamp)
{
	png_structp	png_ptr = NULL;
	png_infop	info_ptr = NULL;
	png_size_t	rowbytes;
	int32_t		color_type, bit_depth;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, in, NULL, NULL);
	if (!png_ptr) 
		return sirius::library::video::transform::codec::png::decompressor::err_code_t::png_out_of_memory_error;   /* out of memory */

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) 
	{
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return sirius::library::video::transform::codec::png::decompressor::err_code_t::png_out_of_memory_error;   /* out of memory */
	}

	/* setjmp() must be called in every function that calls a non-trivial
	* libpng function */
	if (setjmp(in->jmpbuf))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return sirius::library::video::transform::codec::png::decompressor::err_code_t::libpng_fatal_error;   /* fatal libpng error (via longjmp()) */
	}
	png_set_option(png_ptr, PNG_SKIP_sRGB_CHECK_PROFILE, PNG_OPTION_ON);
	png_set_keep_unknown_chunks(png_ptr, PNG_HANDLE_CHUNK_IF_SAFE, (png_const_bytep)"pHYs\0iTXt\0tEXt\0zTXt", 4);
	png_set_read_user_chunk_fn(png_ptr, &in->chunks, png_read_chunk_callback);

	png_read_state_t read_state;
	read_state.self = this;
	read_state.timestamp = timestamp;
	png_set_read_fn(png_ptr, &read_state, png_read_callback);
	png_set_bgr(png_ptr);
	png_read_info(png_ptr, info_ptr);  /* read all PNG info up to image data */

									   /* alternatively, could make separate calls to png_get_image_width(),
									   * etc., but want bit_depth and color_type for later [don't care about
									   * compression_type and filter_type => NULLs] */

	png_get_IHDR(png_ptr, info_ptr, &in->width, &in->height, &bit_depth, &color_type, NULL, NULL, NULL);

	/* expand palette images to RGB, low-bit-depth grayscale images to 8 bits,
	* transparency chunks to full alpha channel; strip 16-bit-per-sample
	* images to 8 bits per sample; and convert grayscale to RGB[A] */

	/* GRR TO DO:  preserve all safe-to-copy ancillary PNG chunks */

	if (!(color_type & PNG_COLOR_MASK_ALPHA)) 
	{
#ifdef PNG_READ_FILLER_SUPPORTED
		png_set_expand(png_ptr);
		png_set_filler(png_ptr, 65535L, PNG_FILLER_AFTER);
#else
		fprintf(stderr, "pngquant readpng:  image is neither RGBA nor GA\n");
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		mainprog_ptr->retval = WRONG_INPUT_COLOR_TYPE;
		return mainprog_ptr->retval;
#endif
	}

	if (bit_depth == 16) 
		png_set_strip_16(png_ptr);

	if (!(color_type & PNG_COLOR_MASK_COLOR))
		png_set_gray_to_rgb(png_ptr);

	/* get source gamma for gamma correction, or use sRGB default */
	double gamma = 0.45455;
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_sRGB)) 
	{
		in->input_color = RWPNG_SRGB;
		in->output_color = RWPNG_SRGB;
	}
	else 
	{
		//png_get_gAMA(png_ptr, info_ptr, &gamma);
		if (gamma > 0 && gamma <= 1.0) 
		{
			in->input_color = RWPNG_GAMA_ONLY;
			in->output_color = RWPNG_GAMA_ONLY;
		}
		else 
		{
			//fprintf(stderr, "pngquant readpng:  ignored out-of-range gamma %f\n", gamma);
			in->input_color = RWPNG_NONE;
			in->output_color = RWPNG_NONE;
			gamma = 0.45455;
		}
	}
	in->gamma = gamma;
	png_set_interlace_handling(png_ptr);

	/* all transformations have been registered; now update info_ptr data,
	* get rowbytes and channels, and allocate image memory */
	png_read_update_info(png_ptr, info_ptr);
	rowbytes = png_get_rowbytes(png_ptr, info_ptr);

	// For overflow safety reject images that won't fit in 32-bit
	if (rowbytes > INT_MAX / in->height) 
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return sirius::library::video::transform::codec::png::decompressor::err_code_t::png_out_of_memory_error;
	}

	if ((in->rgba_data = static_cast<uint8_t*>(malloc(rowbytes * in->height))) == NULL) 
	{
		//fprintf(stderr, "pngquant readpng:  unable to allocate image data\n");
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return sirius::library::video::transform::codec::png::decompressor::err_code_t::png_out_of_memory_error;
	}

	rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	png_bytepp row_pointers = static_cast<png_bytepp>(malloc(in->height * sizeof(row_pointers[0])));
	if(!row_pointers)
		return sirius::library::video::transform::codec::png::decompressor::err_code_t::png_out_of_memory_error;

	for (size_t row = 0; row < in->height; row++)
		row_pointers[row] = in->rgba_data + row * rowbytes;

	/* now we can go ahead and just read the whole image */

	png_read_image(png_ptr, row_pointers);

	/* and we're done!  (png_read_end() can be omitted if no processing of
	* post-IDAT text/time/etc. is desired) */

	png_read_end(png_ptr, NULL);

#if USE_LCMS
#if PNG_LIBPNG_VER < 10500
	png_charp ProfileData;
#else
	png_bytep ProfileData;
#endif
	png_uint_32 ProfileLen;

	cmsHPROFILE hInProfile = NULL;

	/* color_type is read from the image before conversion to RGBA */
	int COLOR_PNG = color_type & PNG_COLOR_MASK_COLOR;

	/* embedded ICC profile */
	if (png_get_iCCP(png_ptr, info_ptr, &(png_charp){0}, &(int){0}, &ProfileData, &ProfileLen)) {

		hInProfile = cmsOpenProfileFromMem(ProfileData, ProfileLen);
		cmsColorSpaceSignature colorspace = cmsGetColorSpace(hInProfile);

		/* only RGB (and GRAY) valid for PNGs */
		if (colorspace == cmsSigRgbData && COLOR_PNG) {
			mainprog_ptr->input_color = RWPNG_ICCP;
			mainprog_ptr->output_color = RWPNG_SRGB;
		}
		else {
			if (colorspace == cmsSigGrayData && !COLOR_PNG) {
				mainprog_ptr->input_color = RWPNG_ICCP_WARN_GRAY;
				mainprog_ptr->output_color = RWPNG_SRGB;
			}
			cmsCloseProfile(hInProfile);
			hInProfile = NULL;
		}
	}

	/* build RGB profile from cHRM and gAMA */
	if (hInProfile == NULL && COLOR_PNG &&
		!png_get_valid(png_ptr, info_ptr, PNG_INFO_sRGB) &&
		png_get_valid(png_ptr, info_ptr, PNG_INFO_gAMA) &&
		png_get_valid(png_ptr, info_ptr, PNG_INFO_cHRM)) {

		cmsCIExyY WhitePoint;
		cmsCIExyYTRIPLE Primaries;

		png_get_cHRM(png_ptr, info_ptr, &WhitePoint.x, &WhitePoint.y,
			&Primaries.Red.x, &Primaries.Red.y,
			&Primaries.Green.x, &Primaries.Green.y,
			&Primaries.Blue.x, &Primaries.Blue.y);

		WhitePoint.Y = Primaries.Red.Y = Primaries.Green.Y = Primaries.Blue.Y = 1.0;

		cmsToneCurve *GammaTable[3];
		GammaTable[0] = GammaTable[1] = GammaTable[2] = cmsBuildGamma(NULL, 1 / gamma);

		hInProfile = cmsCreateRGBProfile(&WhitePoint, &Primaries, GammaTable);

		cmsFreeToneCurve(GammaTable[0]);

		mainprog_ptr->input_color = RWPNG_GAMA_CHRM;
		mainprog_ptr->output_color = RWPNG_SRGB;
	}

	/* transform image to sRGB colorspace */
	if (hInProfile != NULL) {

		cmsHPROFILE hOutProfile = cmsCreate_sRGBProfile();
		cmsHTRANSFORM hTransform = cmsCreateTransform(hInProfile, TYPE_RGBA_8,
			hOutProfile, TYPE_RGBA_8,
			INTENT_PERCEPTUAL,
			omp_get_max_threads() > 1 ? cmsFLAGS_NOCACHE : 0);

#pragma omp parallel for \
            if (mainprog_ptr->height*mainprog_ptr->width > 8000) \
            schedule(static)
		for (unsigned int i = 0; i < mainprog_ptr->height; i++) {
			/* It is safe to use the same block for input and output,
			when both are of the same TYPE. */
			cmsDoTransform(hTransform, row_pointers[i],
				row_pointers[i],
				mainprog_ptr->width);
		}

		cmsDeleteTransform(hTransform);
		cmsCloseProfile(hOutProfile);
		cmsCloseProfile(hInProfile);

		mainprog_ptr->gamma = 0.45455;
	}
#endif

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	in->file_size = read_state.bytes_read;
	in->row_pointers = (unsigned char **)row_pointers;

	return sirius::library::video::transform::codec::png::decompressor::err_code_t::success;
}

void sirius::library::video::transform::codec::png::decompressor::core::free_png_image24(png24_image_t * image)
{
	free(image->row_pointers);
	image->row_pointers = NULL;

	free(image->rgba_data);
	image->rgba_data = NULL;

	png_free_chunks(image->chunks);
	image->chunks = NULL;
}