#include "sirius_png_compressor.h"
#include "png_compressor.h"
#include <sirius_image_creator.h>
#include <sirius_stringhelper.h>

#include <Simd/SimdLib.h>

#include <emmintrin.h>

#ifdef WITH_SAVE_BMP
#include <screengrab.h>
#include <wincodec.h>
unsigned long nFrame = 0;
#endif


sirius::library::video::transform::codec::png::compressor::core::core(sirius::library::video::transform::codec::png::compressor * front)
	: _front(front)
	, _context(nullptr)
	, _state(sirius::library::video::transform::codec::png::compressor::state_t::none)
	, _event(INVALID_HANDLE_VALUE)
	, _thread(INVALID_HANDLE_VALUE)
	, _run(false)
	, _rgba_buffer(nullptr)
{

}

sirius::library::video::transform::codec::png::compressor::core::~core(void)
{
	_state = sirius::library::video::transform::codec::png::compressor::state_t::none;
}

int32_t sirius::library::video::transform::codec::png::compressor::core::state(void)
{
	return _state;
}

int32_t sirius::library::video::transform::codec::png::compressor::core::initialize(sirius::library::video::transform::codec::png::compressor::context_t * context)
{
	if (!context)
		return sirius::library::video::transform::codec::png::compressor::err_code_t::fail;

	_state = sirius::library::video::transform::codec::png::compressor::state_t::initializing;

	_context = context;
	if (!_context->device)
		return sirius::library::video::transform::codec::png::compressor::err_code_t::fail;

	_rgba_buffer = static_cast<uint8_t*>(malloc(_context->width*_context->height*4));
	memset(_rgba_buffer, 0x00, _context->width*_context->height * 4);

	_device.Attach((ID3D11Device*)_context->device);
	_device->GetImmediateContext(&_device_ctx);
	if (!_device_ctx)
		return sirius::library::video::transform::codec::png::compressor::err_code_t::fail;

	allocate_io_buffers();
	_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	_run = true;
	uint32_t thrdaddr = 0;
	_thread = (HANDLE)::_beginthreadex(NULL, 0, sirius::library::video::transform::codec::png::compressor::core::process_callback, this, 0, &thrdaddr);
	if (_thread == NULL || _thread == INVALID_HANDLE_VALUE)
		return sirius::library::video::transform::codec::png::compressor::err_code_t::fail;

	_state = sirius::library::video::transform::codec::png::compressor::state_t::initialized;

	return sirius::library::video::transform::codec::png::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::png::compressor::core::release(void)
{
	if (!((_state == sirius::library::video::transform::codec::png::compressor::state_t::initialized) ||
		(_state == sirius::library::video::transform::codec::png::compressor::state_t::compressing) ||
		(_state == sirius::library::video::transform::codec::png::compressor::state_t::compressed)))
		return sirius::library::video::transform::codec::png::compressor::err_code_t::fail;

	_state = sirius::library::video::transform::codec::png::compressor::state_t::releasing;
	if (_thread != NULL && _thread != INVALID_HANDLE_VALUE)
	{
		_run = false;
		if (::WaitForSingleObject(_thread, INFINITE) == WAIT_OBJECT_0)
		{
			::CloseHandle(_thread);
			_thread = NULL;
		}
	}
	release_io_buffers();

	if (_rgba_buffer)
		free(_rgba_buffer);
	_rgba_buffer = nullptr;

	_state = sirius::library::video::transform::codec::png::compressor::state_t::released;
	return sirius::library::video::transform::codec::png::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::png::compressor::core::play(void)
{
	return sirius::library::video::transform::codec::png::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::png::compressor::core::pause(void)
{
	return sirius::library::video::transform::codec::png::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::png::compressor::core::stop(void)
{
	return sirius::library::video::transform::codec::png::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::png::compressor::core::compress(sirius::library::video::transform::codec::png::compressor::entity_t * input)
{
	if (!_device_ctx)
		return sirius::library::video::transform::codec::png::compressor::err_code_t::fail;

	if ((_state != sirius::library::video::transform::codec::png::compressor::state_t::initialized) && 
		(_state != sirius::library::video::transform::codec::png::compressor::state_t::compressed))
		return sirius::library::video::transform::codec::png::compressor::err_code_t::success;

	sirius::library::video::transform::codec::png::compressor::core::buffer_t * iobuffer = nullptr;
	if (input->memtype == sirius::library::video::transform::codec::png::compressor::video_memory_type_t::d3d11)
	{
		//if (_state == sirius::library::video::transform::codec::png::compressor::state_t::compressing)
		//	return sirius::library::video::transform::codec::png::compressor::err_code_t::success;

		iobuffer = _iobuffer_queue.get_available();
		if (!iobuffer)
			return sirius::library::video::transform::codec::png::compressor::err_code_t::fail;

		iobuffer->input.timestamp = input->timestamp;
		ID3D11Texture2D * gtex = (ID3D11Texture2D*)input->data;

		
		D3D11_TEXTURE2D_DESC gdesc;
		D3D11_TEXTURE2D_DESC hdesc;
		gtex->GetDesc(&gdesc);
		iobuffer->input.data->GetDesc(&hdesc);

		_device_ctx->CopyResource(iobuffer->input.data, gtex);

		D3D11_MAPPED_SUBRESOURCE sub_resource;
		HRESULT hr = _device_ctx->Map((ID3D11Resource*)iobuffer->input.data, 0, D3D11_MAP_READ, 0, &sub_resource);
		_device_ctx->Unmap((ID3D11Resource*)iobuffer->input.data, 0);
#ifdef WITH_SAVE_BMP
		//if (SUCCEEDED(hr))
		{
			nFrame++;
			wchar_t szFileName[200];
			swprintf(szFileName, sizeof(szFileName), L"SCREENSHOT%u.BMP", nFrame);

			DirectX::SaveWICTextureToFile(_device_ctx, iobuffer->input.data, GUID_ContainerFormatBmp, szFileName, &GUID_WICPixelFormat32bppBGRA);
		}
#endif
		::SetEvent(_event);
	}

	return sirius::library::video::transform::codec::png::compressor::err_code_t::success;
}

unsigned __stdcall sirius::library::video::transform::codec::png::compressor::core::process_callback(void * param)
{
	sirius::library::video::transform::codec::png::compressor::core * self = static_cast<sirius::library::video::transform::codec::png::compressor::core*>(param);
	self->process();
	return sirius::library::video::transform::codec::png::compressor::err_code_t::success;
}

void sirius::library::video::transform::codec::png::compressor::core::process(void)
{
	int32_t status = sirius::library::video::transform::codec::png::compressor::err_code_t::success;
	sirius::library::video::transform::codec::png::compressor::core::buffer_t * iobuffer = nullptr;

	long long before_encode_timestamp = 0;
	long long after_encode_timestamp = 0;
	while (_run)
	{
		if (::WaitForSingleObject(_event, INFINITE) == WAIT_OBJECT_0)
		{
			_state = sirius::library::video::transform::codec::png::compressor::state_t::compressing;
			iobuffer = _iobuffer_queue.get_pending();
			while (iobuffer)
			{
				before_encode_timestamp = iobuffer->input.timestamp;

				HRESULT hr = E_FAIL;
				D3D11_TEXTURE2D_DESC desc;
				iobuffer->input.data->GetDesc(&desc);

				D3D11_MAPPED_SUBRESOURCE sub_resource;
				hr = _device_ctx->Map((ID3D11Resource*)iobuffer->input.data, 0, D3D11_MAP_READ, 0, &sub_resource);
				if (SUCCEEDED(hr))
				{
					uint8_t * pixels = (uint8_t*)sub_resource.pData;
					//convert((int32_t*)pixels, (int32_t*)_rgba_buffer, _context->width * _context->height * 4);
					memcpy(_rgba_buffer, pixels, _context->width * _context->height * 4);
					_device_ctx->Unmap((ID3D11Resource*)iobuffer->input.data, 0);
#if defined(WITH_SAVE_BMP)
					HINSTANCE module_handle = ::GetModuleHandleA("sirius_png_compressor.dll");
					char module_path[MAX_PATH] = { 0 };
					char * module_name = module_path;
					module_name += GetModuleFileNameA(module_handle, module_name, (sizeof(module_path) / sizeof(*module_path)) - (module_name - module_path));
					if (module_name != module_path)
					{
						CHAR * slash = strrchr(module_path, '\\');
						if (slash != NULL)
						{
							module_name = slash + 1;
							_strset_s(module_name, strlen(module_name) + 1, 0);
						}
						else
						{
							_strset_s(module_path, strlen(module_path) + 1, 0);
						}
					}

					char tempname[MAX_PATH] = { 0 };
					GetTempFileNameA(module_path, NULL, 0, tempname);
					_snprintf_s(tempname, sizeof(tempname) - 1, "%s.bmp", tempname);

					wchar_t * wtempname = nullptr;
					sirius::stringhelper::convert_multibyte2wide(tempname, &wtempname);

					sirius::image::creator imgcreator(_context->width, _context->height);
					memcpy(imgcreator.pixel_buffer, pixels, _context->width * _context->height * 4);
					imgcreator.save(wtempname);

					if (wtempname)
						::SysFreeString(wtempname);
#endif

					liq_attr * liq = liq_attr_create();
					liq_set_speed(liq, _context->speed);
					liq_set_max_colors(liq, _context->max_colors);
					
					int32_t quality_percent = 90; // quality on 0-100 scale, updated upon successful remap
					png8_image_t qntpng = { 0 };
					liq_image * rgba = liq_image_create_rgba(liq, _rgba_buffer, _context->width, _context->height, _context->gamma);
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
							//iobuffer = _iobuffer_queue.get_pending();
							//continue;
							status = sirius::library::video::transform::codec::compressor::err_code_t::out_of_memory_error;
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
							qntpng.palette[i].r = px.r;
							qntpng.palette[i].g = px.g;
							qntpng.palette[i].b = px.b;
							qntpng.palette[i].a = px.a;
						}

						double palette_error = liq_get_quantization_error(remap);
						if (palette_error >= 0)
						{
							quality_percent = liq_get_quantization_quality(remap);
							//verbose_printf(options, "  mapped image to new colors...MSE=%.3f (Q=%d)", palette_error, quality_percent);
						}
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
						//qntpng.chunks = input_image_rwpng.chunks; input_image_rwpng.chunks = NULL;
						status = write_png_image8(&qntpng, &iobuffer->output);
					}
					if (rgba)
						liq_image_destroy(rgba);
					free_png_image8(&qntpng);
					liq_attr_destroy(liq);

#if defined(WITH_SAVE_PNG)
					HINSTANCE module_handle = ::GetModuleHandleA("sirius_png_compressor.dll");
					char module_path[MAX_PATH] = { 0 };
					char * module_name = module_path;
					module_name += GetModuleFileNameA(module_handle, module_name, (sizeof(module_path) / sizeof(*module_path)) - (module_name - module_path));
					if (module_name != module_path)
					{
						CHAR * slash = strrchr(module_path, '\\');
						if (slash != NULL)
						{
							module_name = slash + 1;
							_strset_s(module_name, strlen(module_name) + 1, 0);
						}
						else
						{
							_strset_s(module_path, strlen(module_path) + 1, 0);
						}
					}

					char tempname[MAX_PATH] = { 0 };
					GetTempFileNameA(module_path, NULL, 0, tempname);
					_snprintf_s(tempname, sizeof(tempname) - 1, "%s.png", tempname);

					DWORD nwritten = 0;
					HANDLE f = ::CreateFileA(tempname, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
					::WriteFile(f, iobuffer->output.data, iobuffer->output.data_size, &nwritten, NULL);
					::CloseHandle(f);
#endif

					if(_front)
						_front->after_process_callback(iobuffer->output.data, iobuffer->output.data_size, before_encode_timestamp, after_encode_timestamp);
					iobuffer->output.data_size = 0;
				}

				iobuffer = _iobuffer_queue.get_pending();
				continue;
			}
			_state = sirius::library::video::transform::codec::png::compressor::state_t::compressed;
		}
	}
}

int32_t sirius::library::video::transform::codec::png::compressor::core::allocate_io_buffers(void)
{
	int32_t status = sirius::library::video::transform::codec::png::compressor::err_code_t::success;
	_iobuffer_queue.initialize(_iobuffer, _context->nbuffer);

	if (_context->memtype == sirius::library::video::transform::codec::png::compressor::video_memory_type_t::host)
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.Width = _context->width;
		desc.Height = _context->height;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.MiscFlags = 0;
		desc.BindFlags = 0;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		ID3D11Texture2D * tex = NULL;
		for (uint32_t i = 0; i < _context->nbuffer; i++)
		{
			HRESULT hr = _device->CreateTexture2D(&desc, NULL, &tex);
			if (SUCCEEDED(hr))
			{
				_iobuffer[i].input.data = tex;
				_iobuffer[i].input.data->AddRef();
				tex->Release();
			}

			_iobuffer[i].output.data_capacity = sirius::library::video::transform::codec::png::compressor::core::MAX_PNG_SIZE;
			_iobuffer[i].output.data_size = 0;
			_iobuffer[i].output.data = static_cast<uint8_t*>(malloc(_iobuffer[i].output.data_capacity));
		}
	}
	else if (_context->memtype == sirius::library::video::transform::codec::png::compressor::video_memory_type_t::d3d11)
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.Width = _context->width;
		desc.Height = _context->height;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.MiscFlags = 0;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		ID3D11Texture2D * tex = NULL;
		for (uint32_t i = 0; i < _context->nbuffer; i++)
		{
			HRESULT hr = _device->CreateTexture2D(&desc, NULL, &tex);
			if (SUCCEEDED(hr))
			{
				_iobuffer[i].input.data = tex;
				_iobuffer[i].input.data->AddRef();
				tex->Release();
			}
			_iobuffer[i].output.data_capacity = sirius::library::video::transform::codec::png::compressor::core::MAX_PNG_SIZE;
			_iobuffer[i].output.data_size = 0;
			_iobuffer[i].output.data = static_cast<uint8_t*>(malloc(_iobuffer[i].output.data_capacity));
		}
	}
	
	return sirius::library::video::transform::codec::png::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::png::compressor::core::release_io_buffers(void)
{
	if ((_context->memtype == sirius::library::video::transform::codec::png::compressor::video_memory_type_t::host) ||
		(_context->memtype == sirius::library::video::transform::codec::png::compressor::video_memory_type_t::d3d11))
	{
		for (uint32_t i = 0; i < _context->nbuffer; i++)
		{
			_iobuffer[i].input.data->Release();
			_iobuffer[i].input.data = NULL;
		}
	}

	_iobuffer_queue.release();
	return sirius::library::video::transform::codec::png::compressor::err_code_t::success;
}

void sirius::library::video::transform::codec::png::compressor::core::png_error_handler(png_structp png_ptr, png_const_charp msg)
{

}

void sirius::library::video::transform::codec::png::compressor::core::png_write_callback(png_structp  png_ptr, png_bytep data, png_size_t length)
{
	png_write_state_t * write_state = (png_write_state_t*)png_get_io_ptr(png_ptr);
	if (write_state->retval!= sirius::library::video::transform::codec::png::compressor::err_code_t::success)
		return;

	sirius::library::video::transform::codec::png::compressor::core::obuffer_t * compressed = write_state->compressed;
	if (compressed) //sending png per image
	{
		memcpy(compressed->data + compressed->data_size, data, length);
		compressed->data_size += length;
	}
}

void sirius::library::video::transform::codec::png::compressor::core::png_flush_callback(png_structp png_ptr)
{
	int a = 2;
}

void sirius::library::video::transform::codec::png::compressor::core::png_set_gamma(png_infop info_ptr, png_structp png_ptr, double gamma, png_color_transform color)
{
	//if (color != RWPNG_GAMA_ONLY && color != RWPNG_NONE) 
	//	png_set_gAMA(png_ptr, info_ptr, gamma);

	if (color == RWPNG_SRGB) 
		png_set_sRGB(png_ptr, info_ptr, 0); // 0 = Perceptual
}

void sirius::library::video::transform::codec::png::compressor::core::png_free_chunks(png_chunk_t * chunk)
{
	if (!chunk) 
		return;
	png_free_chunks(chunk->next);
	free(chunk->data);
	free(chunk);
}

int32_t sirius::library::video::transform::codec::png::compressor::core::write_png_begin(png_image_t * img, png_structpp png_ptr_p, png_infopp info_ptr_p, int32_t fast_compression)
{
	*png_ptr_p = png_create_write_struct(PNG_LIBPNG_VER_STRING, img, NULL, NULL);

	if (!(*png_ptr_p)) 
	{
		return sirius::library::video::transform::codec::png::compressor::err_code_t::libpng_init_error;   /* out of memory */
	}

	*info_ptr_p = png_create_info_struct(*png_ptr_p);
	if (!(*info_ptr_p)) 
	{
		png_destroy_write_struct(png_ptr_p, NULL);
		return sirius::library::video::transform::codec::png::compressor::err_code_t::libpng_init_error;   /* out of memory */
	}

	/* setjmp() must be called in every function that calls a PNG-writing
	* libpng function, unless an alternate error handler was installed--
	* but compatible error handlers must either use longjmp() themselves
	* (as in this program) or exit immediately, so here we go: */

	if (setjmp(img->jmpbuf)) 
	{
		png_destroy_write_struct(png_ptr_p, info_ptr_p);
		return sirius::library::video::transform::codec::png::compressor::err_code_t::libpng_init_error;   /* libpng error (via longjmp()) */
	}

	png_set_compression_level(*png_ptr_p, fast_compression ? Z_BEST_SPEED : Z_BEST_COMPRESSION);
	png_set_compression_mem_level(*png_ptr_p, fast_compression ? 9 : 5); // judging by optipng results, smaller mem makes libpng compress slightly better

	return sirius::library::video::transform::codec::png::compressor::err_code_t::success;
}

void sirius::library::video::transform::codec::png::compressor::core::write_png_end(png_infopp info_ptr_p, png_structpp png_ptr_p, png_bytepp row_pointers)
{
	png_write_info(*png_ptr_p, *info_ptr_p);

	png_set_packing(*png_ptr_p);

	png_write_image(*png_ptr_p, row_pointers);

	png_write_end(*png_ptr_p, NULL);

	png_destroy_write_struct(png_ptr_p, info_ptr_p);
}

int32_t sirius::library::video::transform::codec::png::compressor::core::write_png_image8(png8_image_t * out, sirius::library::video::transform::codec::png::compressor::core::obuffer_t * compressed)
{
	png_structp png_ptr;
	png_infop info_ptr;

	if (out->num_palette > 256) 
		return sirius::library::video::transform::codec::png::compressor::err_code_t::invalid_argument;

	int32_t retval = write_png_begin((png_image_t*)out, &png_ptr, &info_ptr, out->fast_compression);
	if (retval)
		return retval;

	png_write_state_t write_state;
	write_state.maximum_file_size = out->maximum_file_size;
	write_state.retval = sirius::library::video::transform::codec::png::compressor::err_code_t::success;
	write_state.compressed = compressed;
	png_set_write_fn(png_ptr, &write_state, png_write_callback, png_flush_callback);

	// Palette images generally don't gain anything from filtering
	png_set_filter(png_ptr, PNG_FILTER_TYPE_BASE, PNG_FILTER_VALUE_NONE);

	png_set_gamma(info_ptr, png_ptr, out->gamma, out->output_color);

	/* set the image parameters appropriately */
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

	if (sirius::library::video::transform::codec::png::compressor::err_code_t::success == write_state.retval && 
		write_state.maximum_file_size && 
		write_state.bytes_written > write_state.maximum_file_size) 
	{
		return sirius::library::video::transform::codec::png::compressor::err_code_t::too_large_file;
	}

	return write_state.retval;
}

void sirius::library::video::transform::codec::png::compressor::core::free_png_image8(png8_image_t * image)
{
	free(image->indexed_data);
	image->indexed_data = NULL;

	free(image->row_pointers);
	image->row_pointers = NULL;

	png_free_chunks(image->chunks);
	image->chunks = NULL;
}

void sirius::library::video::transform::codec::png::compressor::core::convert(int32_t * RGBA, int32_t * BGRA, int32_t size)
{
	// assumes RGBA and BGRA are 16 byte aligned
	// assumes size % 4 = 0

	for (std::size_t i = 0; i < size; i += 4)
	{
		__m128i rgba = _mm_load_si128(reinterpret_cast<__m128i*>(RGBA + i));
		__m128i mask = _mm_set1_epi32(0xff000000);
		__m128i b = _mm_and_si128(_mm_slli_si128(rgba, 2), mask);
		__m128i r = _mm_srli_si128(_mm_and_si128(rgba, mask), 2);
		mask = _mm_set1_epi32(0x00ff00ff);
		__m128i bgra = _mm_or_si128(r, _mm_or_si128(b, _mm_and_si128(rgba, mask)));
		_mm_store_si128(reinterpret_cast<__m128i*>(BGRA + i), bgra);
	}
}