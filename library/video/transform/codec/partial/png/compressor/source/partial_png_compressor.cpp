#include "sirius_partial_png_compressor.h"
#include "partial_png_compressor.h"

#include <sirius_image_creator.h>
#include <sirius_stringhelper.h>
#include <sirius_locks.h>

#include <simd_image_processor.h>

#include <vector>
#include <assert.h>


#if defined(WITH_DEBUG_PNG)
#include <sirius_io.h>
#endif

#define SIMD_RESIZER	sirius::library::video::transform::codec::partial::simd::avx2::resizer
#define SIMD_EVALUATOR	sirius::library::video::transform::codec::partial::simd::avx2::evaluator

sirius::library::video::transform::codec::partial::png::compressor::core::core(sirius::library::video::transform::codec::partial::png::compressor * front)
	: _front(front)
	, _context(nullptr)
	, _state(sirius::library::video::transform::codec::partial::png::compressor::state_t::none)
	, _event(INVALID_HANDLE_VALUE)
	, _thread(INVALID_HANDLE_VALUE)
	, _run(false)
	, _invalidate(false)
{
	::InitializeCriticalSection(&_cs);
	_real_compressor = new sirius::library::video::transform::codec::libpng::compressor(front);

	::QueryPerformanceFrequency(&_frequency);
}

sirius::library::video::transform::codec::partial::png::compressor::core::~core(void)
{
	_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::none;
	if (_real_compressor)
		delete _real_compressor;
	_real_compressor = nullptr;
	::DeleteCriticalSection(&_cs);
}

int32_t sirius::library::video::transform::codec::partial::png::compressor::core::state(void)
{
	return _state;
}

int32_t sirius::library::video::transform::codec::partial::png::compressor::core::initialize(sirius::library::video::transform::codec::partial::png::compressor::context_t * context)
{
	int32_t status = sirius::library::video::transform::codec::partial::png::compressor::err_code_t::fail;
	if (!context)
		return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::fail;

	_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::initializing;

	_context = context;
	status = _real_compressor->initialize(_context);
	if (status == sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success)
	{
		allocate_io_buffers();
		_event = CreateEvent(NULL, FALSE, FALSE, NULL);
		_run = true;
		uint32_t thrdaddr = 0;
		_thread = (HANDLE)::_beginthreadex(NULL, 0, sirius::library::video::transform::codec::partial::png::compressor::core::process_callback, this, 0, &thrdaddr);
		if (_thread == NULL || _thread == INVALID_HANDLE_VALUE)
			return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::fail;

		status = sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
	}

	_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::initialized;
	return status;
}

int32_t sirius::library::video::transform::codec::partial::png::compressor::core::release(void)
{
	if (!((_state == sirius::library::video::transform::codec::partial::png::compressor::state_t::initialized) ||
		(_state == sirius::library::video::transform::codec::partial::png::compressor::state_t::compressing) ||
		(_state == sirius::library::video::transform::codec::partial::png::compressor::state_t::compressed)))
		return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::fail;

	_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::releasing;
	if (_thread != NULL && _thread != INVALID_HANDLE_VALUE)
	{
		_run = false;
		::SetEvent(_event);
		if (::WaitForSingleObject(_thread, INFINITE) == WAIT_OBJECT_0)
		{
			::CloseHandle(_thread);
			_thread = NULL;
		}
	}
	release_io_buffers();

	_real_compressor->release();

	_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::released;
	return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::partial::png::compressor::core::play(void)
{
	_invalidate = true;
	if (_event != NULL && _event != INVALID_HANDLE_VALUE)
		::SetEvent(_event);
	return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::partial::png::compressor::core::pause(void)
{
	return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::partial::png::compressor::core::stop(void)
{
	return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::partial::png::compressor::core::invalidate(void)
{
	_invalidate = true;
	if(_event != NULL && _event != INVALID_HANDLE_VALUE)
		::SetEvent(_event);
	return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::partial::png::compressor::core::compress(sirius::library::video::transform::codec::partial::png::compressor::entity_t * input, sirius::library::video::transform::codec::partial::png::compressor::entity_t * bitstream)
{
	return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
}


static int32_t ib_index = 0;
static int32_t ob_index = 0;

int32_t sirius::library::video::transform::codec::partial::png::compressor::core::compress(sirius::library::video::transform::codec::partial::png::compressor::entity_t * input)
{
	sirius::library::video::transform::codec::partial::png::compressor::core::buffer_t * iobuffer = nullptr;
#if 1

	bool force_fullmode = false;
	{
		sirius::autolock lock(&_cs);

		iobuffer = _iobuffer_queue.get_available();
		while (!iobuffer)
		{
			while (true)
			{
				iobuffer = _iobuffer_queue.get_pending();
				if (!iobuffer)
					break;
			}
			
			iobuffer = _iobuffer_queue.get_available();
			if (iobuffer)
			{
				force_fullmode = true;
				break;
			}
		}

		copy(input, iobuffer);
		iobuffer->input.force_fullmode = force_fullmode;
	}

#else
	if ((input->width != _prev_width) || (input->x != _prev_x) || (input->height != _prev_height) || (input->y != _prev_y))
	{
		{
			sirius::autolock lock(&_cs);
			iobuffer = _iobuffer_queue.get_available();
			if (iobuffer)
			{
				copy(input, iobuffer);
			}
		}
		while (!iobuffer)
		{
			{
				sirius::autolock lock(&_cs);
				iobuffer = _iobuffer_queue.get_available();
				if (iobuffer)
				{
					copy(input, iobuffer);
				}
			}
			::Sleep(10);
		}
	}
	else
	{
		sirius::autolock lock(&_cs);

		iobuffer = _iobuffer_queue.get_available();
		while (!iobuffer)
		{
			iobuffer = _iobuffer_queue.get_pending();
			iobuffer = _iobuffer_queue.get_available();
			if (iobuffer)
				break;
			::Sleep(10);
		}

		copy(input, iobuffer);
	}
#endif
	::SetEvent(_event);
	return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
}

unsigned __stdcall sirius::library::video::transform::codec::partial::png::compressor::core::process_callback(void * param)
{
	sirius::library::video::transform::codec::partial::png::compressor::core * self = static_cast<sirius::library::video::transform::codec::partial::png::compressor::core*>(param);
	if (self->_context->indexed_video)
	{
		if (self->_context->nthread > 1)
			self->process_threaded_indexed();
		else
			self->process_indexed();
	}
	else
	{
		if (self->_context->nthread > 1)
			self->process_threaded_coordinated();
		else
			self->process_coordinated();
	}
	return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
}

unsigned __stdcall sirius::library::video::transform::codec::partial::png::compressor::core::process_coordinated_encoding_callback(void * param)
{
	sirius::library::video::transform::codec::partial::png::compressor::core::coordinated_thread_context_t * self = static_cast<sirius::library::video::transform::codec::partial::png::compressor::core::coordinated_thread_context_t*>(param);
	self->parent->process_threaded_coordinated_encoding(self);
	return 0;
}

void sirius::library::video::transform::codec::partial::png::compressor::core::process_threaded_coordinated_encoding(coordinated_thread_context_t * thread_ctx)
{
	while (thread_ctx->run)
	{
		if (WaitForSingleObject(thread_ctx->signal, INFINITE) == WAIT_OBJECT_0)
		{
			if (thread_ctx->run)
			{
				int32_t status = thread_ctx->real_compressor->compress(&thread_ctx->input, &thread_ctx->output);
				if (status == sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success)
				{
					if (thread_ctx->output.data_capacity >= thread_ctx->output.data_size)
					{
						(*thread_ctx->plength) = thread_ctx->output.data_size;
						memmove(thread_ctx->pcompressed, thread_ctx->output.data, (*thread_ctx->plength));
					}
				}

				if (thread_ctx->rows)
				{
					free(thread_ctx->rows);
					thread_ctx->rows = nullptr;
				}
			}
			::SetEvent(thread_ctx->available);
		}
	}
}
void sirius::library::video::transform::codec::partial::png::compressor::core::process_coordinated(void)
{
	int32_t status = sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
	sirius::library::video::transform::codec::partial::png::compressor::core::buffer_t * iobuffer = nullptr;

	long long before_encode_timestamp = 0;
	long long after_encode_timestamp = 0;

	int32_t	compressed_buffer_size = 1024 * 1024 * 2;
	uint8_t * compressed_buffer = static_cast<uint8_t*>(malloc(compressed_buffer_size));
	memset(compressed_buffer, 0x00, compressed_buffer_size);

	int32_t		align = 32;
	int32_t		reference_buffer_size = (_context->width * _context->height) << 2;
	uint8_t *	reference_buffer = static_cast<uint8_t*>(_aligned_malloc(reference_buffer_size, align));
	memset(reference_buffer, 0x00, reference_buffer_size);

	int32_t		pcount = _context->width * _context->height;
	int16_t *	px = new int16_t[pcount];
	int16_t *	py = new int16_t[pcount];
	int16_t *	pwidth = new int16_t[pcount];
	int16_t *	pheight = new int16_t[pcount];
	uint8_t **	pcompressed = new uint8_t*[pcount];
	int32_t *	plength = new int32_t[pcount];

	int32_t		process_data_capacity = (_context->width * _context->height) << 2;
	int32_t		process_data_size = 0;
	uint8_t *	process_data = static_cast<uint8_t*>(_aligned_malloc(process_data_capacity, align));
	int32_t		process_x = 0;
	int32_t		process_y = 0;
	int32_t		process_width = 0;
	int32_t		process_height = 0;
	long long	process_timestamp = 0;
	bool		process_force_fullmode = false;
	bool		process_compress_first_time = true;

	while (_run)
	{
		if (::WaitForSingleObject(_event, INFINITE) == WAIT_OBJECT_0)
		{
			_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressing;

			{
				sirius::autolock lock(&_cs);
				iobuffer = _iobuffer_queue.get_pending();
				if (!iobuffer)
				{
					if (!_invalidate)
					{
						_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressed;
						::Sleep(10);
						continue;
					}
					process_data_size = 0;
				}
				else
				{
					process_data_size = iobuffer->input.data_size;
					process_timestamp = iobuffer->input.timestamp;
					memmove(process_data, iobuffer->input.data, process_data_size);
					process_x = iobuffer->input.x;
					process_y = iobuffer->input.y;
					process_width = iobuffer->input.width;
					process_height = iobuffer->input.height;
					process_force_fullmode = iobuffer->input.force_fullmode;
				}
			}


			if (process_compress_first_time)
			{
				uint8_t ** rows = static_cast<uint8_t**>(malloc(_context->block_height * sizeof(uint8_t*)));

				before_encode_timestamp = process_timestamp;
				int32_t count = 0;
				uint8_t * real_compressed_buffer = compressed_buffer;

				_real_compressor->initialize(_context);
				for (int32_t h = 0; h < _context->height; h = h + _context->block_height)
				{
					for (int32_t w = 0; w < _context->width; w = w + _context->block_width)
					{
						for (int32_t eh = 0, row_index = 0; eh < _context->block_height; eh++, row_index++)
						{
							int32_t src_index = (h + eh) * (_context->width << 2) + (w << 2);
							rows[row_index] = process_data + src_index;
						}

						if (_front)
						{
							sirius::library::video::transform::codec::partial::png::compressor::entity_t input;
							input.data = rows;
							input.data_capacity = _context->block_height;
							input.data_size = _context->block_height;
							input.x = 0;
							input.y = 0;
							input.width = _context->block_width;
							input.height = _context->block_height;

							sirius::library::video::transform::codec::partial::png::compressor::entity_t bitstream;
							bitstream.data = real_compressed_buffer;
							bitstream.data_capacity = real_compressed_buffer - compressed_buffer;
							bitstream.data_capacity = compressed_buffer_size - bitstream.data_capacity;
							bitstream.data_size = 0;
							bitstream.x = input.x;
							bitstream.y = input.y;
							bitstream.width = input.width;
							bitstream.height = input.height;

							status = _real_compressor->compress(&input, &bitstream);
							if (status == sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success)
							{
								px[count] = w;
								py[count] = h;
								pwidth[count] = _context->block_width;
								pheight[count] = _context->block_height;
								pcompressed[count] = (uint8_t*)bitstream.data;
								plength[count] = bitstream.data_size;
								real_compressed_buffer += bitstream.data_size;
								count++;
							}
						}
					}
				}
				_real_compressor->release();
				_front->after_process_callback(count, px, py, pwidth, pheight, pcompressed, plength, before_encode_timestamp, after_encode_timestamp);
				memmove(reference_buffer, process_data, process_data_size);
				process_compress_first_time = false;
				if (rows)
				{
					free(rows);
					rows = nullptr;
				}
				continue;
			}

			if (_invalidate && (process_data_size == 0))
			{
				uint8_t ** rows = static_cast<uint8_t**>(malloc(_context->block_height * sizeof(uint8_t*)));

				int32_t count = 0;
				uint8_t * real_compressed_buffer = compressed_buffer;
				_real_compressor->initialize(_context);
				for (int32_t h = 0; h < _context->height; h = h + _context->block_height)
				{
					for (int32_t w = 0; w < _context->width; w = w + _context->block_width)
					{
						for (int32_t eh = 0, row_index = 0; eh < _context->block_height; eh++, row_index++)
						{
							int32_t src_index = (h + eh) * (_context->width << 2) + (w << 2);
							rows[row_index] = process_data + src_index;
						}

						if (_front)
						{
							sirius::library::video::transform::codec::partial::png::compressor::entity_t input;
							input.data = rows;
							input.data_capacity = _context->block_height;
							input.data_size = _context->block_height;
							input.x = 0;
							input.y = 0;
							input.width = _context->block_width;
							input.height = _context->block_height;

							sirius::library::video::transform::codec::partial::png::compressor::entity_t bitstream;
							bitstream.data = real_compressed_buffer;
							bitstream.data_capacity = real_compressed_buffer - compressed_buffer;
							bitstream.data_capacity = compressed_buffer_size - bitstream.data_capacity;
							bitstream.data_size = 0;
							bitstream.x = input.x;
							bitstream.y = input.y;
							bitstream.width = input.width;
							bitstream.height = input.height;

							status = _real_compressor->compress(&input, &bitstream);
							if (status == sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success)
							{
								px[count] = w;
								py[count] = h;
								pwidth[count] = _context->block_width;
								pheight[count] = _context->block_height;
								pcompressed[count] = (uint8_t*)bitstream.data;
								plength[count] = bitstream.data_size;
								real_compressed_buffer += bitstream.data_size;
								count++;
							}
						}
					}
				}
				_real_compressor->release();
				_front->after_process_callback(count, px, py, pwidth, pheight, pcompressed, plength, before_encode_timestamp, after_encode_timestamp);
				memmove(reference_buffer, process_data, process_data_size);

				if (rows)
				{
					free(rows);
					rows = nullptr;
				}

				_invalidate = false;
			}
			else
			{
				while (process_data_size>0)
				{
					before_encode_timestamp = process_timestamp;
					int32_t count = 0;
					uint8_t * real_compressed_buffer = compressed_buffer;

					int32_t begin_height = 0;
					int32_t end_height = 0;
					int32_t begin_width = 0;
					int32_t end_width = 0;
					int32_t resized_begin_height = 0;
					int32_t resized_end_height = 0;
					int32_t resized_begin_width = 0;
					int32_t resized_end_width = 0;
					if (process_force_fullmode)
					{
						begin_height = 0;
						end_height = _context->height;
						begin_width = 0;
						end_width = _context->width;
					}
					else
					{
						begin_height = (process_y / _context->mb_height) * _context->mb_height;
						end_height = ((process_y + process_height) / _context->mb_height) * _context->mb_height;

						if (((process_y + process_height) % _context->mb_height) > 0)
							end_height += _context->mb_height;

						begin_width = (process_x / _context->mb_width) * _context->mb_width;
						end_width = ((process_x + process_width) / _context->mb_width) * _context->mb_width;
						if (((process_x + process_width) % _context->mb_width) > 0)
							end_width += _context->mb_width;
					}

					// find different area between current and previous images
					std::map<uint64_t, uint32_t> bfgs;
					__declspec(align(32)) uint8_t result[32] = { 0 };
					//const __m256i xor_operand = _mm256_setr_epi8(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

					for (int32_t h = begin_height, y = (begin_height / _context->mb_height); h < end_height; h = h + _context->mb_height, y++)
					{
						for (int32_t w = begin_width, x = (begin_width / _context->mb_width); w < end_width; w = w + _context->mb_width, x++)
						{
							bool diff = false;
							for (int32_t mbh = 0; mbh < _context->mb_height; mbh++)
							{
								for (int32_t mbw = 0; mbw < _context->mb_width; mbw = mbw + (align >> 2))
								{
									int32_t ci = (h + mbh) * (_context->width << 2) + ((w + mbw) << 2);
									uint8_t * p = process_data + ci;
									uint8_t * r = reference_buffer + ci;
									const __m256i current = _mm256_load_si256((__m256i*)p);
									const __m256i reference = _mm256_load_si256((__m256i*)r);
									const __m256i cmpeq = _mm256_cmpeq_epi8(current, reference);
									//const __m256i xor		= _mm256_xor_si256(xor_operand, cmpeq);
									_mm256_store_si256((__m256i*)result, cmpeq);

									/*
									const __m256i current = _mm256_load_si256((__m256i*)p);
									const __m256i reference = _mm256_load_si256((__m256i*)r);
									const __m256i cmpeq = _mm256_cmpeq_epi8(current, reference);
									_mm256_store_si256((__m256i*)simd_result, cmpeq);
									for (int32_t index = 0; index < simd_align; index++)
									{
									if (!simd_result[index])
									{
									diff = true;
									break;
									}
									}
									*/
									for (int32_t i = 0; i < align; i++)
									{
										if (!result[i])
										{
											/*
											char debug[MAX_PATH] = { 0 };
											_snprintf_s(debug, MAX_PATH, "difference coord is (%d, %d)\n", ccl_w, ccl_h);
											OutputDebugStringA(debug);
											*/
											uint64_t key = (uint64_t(x) << 32) | (y);
											uint32_t val = 0;
											bfgs.insert(std::make_pair(key, val));
											diff = true;
											break;
										}
									}

									if (diff)
										break;
								}
								if (diff)
									break;
							}
						}
					}

					// make connected component
					unsigned short * pseudo_stack = (unsigned short*)malloc(3 * sizeof(unsigned short)*((_context->width / _context->mb_width) * (_context->height / _context->mb_height) + 1));
					int32_t lable_number = 0;
					std::map<uint64_t, uint32_t>::iterator		ccl_iter;
					std::vector<connected_component_t*>			ccl_component_vec;
					for (ccl_iter = bfgs.begin(); ccl_iter != bfgs.end(); ccl_iter++)
					{
						uint64_t key = ccl_iter->first;
						uint32_t val = ccl_iter->second;
						if (val == UNLABELED) //new component found
						{
							lable_number++;
							//ccl_iter->second = lable_number;
							connected_component_t * cc = new connected_component_t(lable_number);
							ccl_component_vec.push_back(cc);
							uint32_t x = uint32_t((key & 0xFFFFFFFF00000000LL) >> 32);
							uint32_t y = uint32_t(key & 0x00000000FFFFFFFFLL);
							connect_component(pseudo_stack, &bfgs, cc, (_context->width / _context->mb_width), (_context->height / _context->mb_height), x, y);
						}
					}
					free(pseudo_stack);
					pseudo_stack = nullptr;

					std::vector<connected_component_t*>::iterator ccc_iter;
					for (ccc_iter = ccl_component_vec.begin(); ccc_iter != ccl_component_vec.end(); ccc_iter++)
					{
						//(*ccc_iter)->
						int32_t cc_x = (*ccc_iter)->left * _context->mb_width;
						int32_t cc_y = (*ccc_iter)->top * _context->mb_height;
						int32_t cc_height = ((*ccc_iter)->bottom - (*ccc_iter)->top + 1) * _context->mb_height;
						int32_t cc_width = ((*ccc_iter)->right - (*ccc_iter)->left + 1) * _context->mb_width;

						uint8_t ** cc_rows = static_cast<uint8_t**>(malloc(cc_height * sizeof(uint8_t*)));
						for (int32_t h = 0; h < cc_height; h++)
						{
							int32_t src_index = (cc_y + h) * (_context->width << 2) + (cc_x << 2);
							cc_rows[h] = process_data + src_index;
							memmove(reference_buffer + src_index, process_data + src_index, cc_width << 2);
						}

						if (_front)
						{
							sirius::library::video::transform::codec::partial::png::compressor::context_t context;
							context.speed = _context->speed;
							context.max_colors = _context->max_colors;
							context.min_quality = _context->min_quality;
							context.max_quality = _context->max_quality;
							context.gamma = _context->gamma;
							context.floyd = _context->floyd;

							_real_compressor->initialize(&context);
							sirius::library::video::transform::codec::partial::png::compressor::entity_t input;
							input.data = cc_rows;
							input.data_capacity = cc_height;
							input.data_size = cc_height;
							input.x = 0;
							input.y = 0;
							input.width = cc_width;
							input.height = cc_height;

							sirius::library::video::transform::codec::partial::png::compressor::entity_t bitstream;
							bitstream.data = real_compressed_buffer;
							bitstream.data_capacity = real_compressed_buffer - compressed_buffer;
							bitstream.data_capacity = compressed_buffer_size - bitstream.data_capacity;
							bitstream.data_size = 0;
							bitstream.x = input.x;
							bitstream.y = input.y;
							bitstream.width = input.width;
							bitstream.height = input.height;

							status = _real_compressor->compress(&input, &bitstream);
							if (status == sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success)
							{
								px[count] = cc_x;
								py[count] = cc_y;
								pwidth[count] = cc_width;
								pheight[count] = cc_height;
								pcompressed[count] = (uint8_t*)bitstream.data;
								plength[count] = bitstream.data_size;
								real_compressed_buffer += bitstream.data_size;

								count++;
							}
							_real_compressor->release();
						}

						if (cc_rows)
						{
							free(cc_rows);
							cc_rows = nullptr;
						}
					}

					process_force_fullmode = false;
					if (_invalidate)
					{
						uint8_t ** rows = static_cast<uint8_t**>(malloc(_context->block_height * sizeof(uint8_t*)));

						count = 0;
						uint8_t * real_compressed_buffer = compressed_buffer;
						_real_compressor->initialize(_context);
						for (int32_t h = 0; h < _context->height; h = h + _context->block_height)
						{
							for (int32_t w = 0; w < _context->width; w = w + _context->block_width)
							{
								for (int32_t eh = 0, row_index = 0; eh < _context->block_height; eh++, row_index++)
								{
									int32_t src_index = (h + eh) * (_context->width << 2) + (w << 2);
									rows[row_index] = process_data + src_index;
								}

								if (_front)
								{
									sirius::library::video::transform::codec::partial::png::compressor::entity_t input;
									input.data = rows;
									input.data_capacity = _context->block_height;
									input.data_size = _context->block_height;
									input.x = 0;
									input.y = 0;
									input.width = _context->block_width;
									input.height = _context->block_height;

									sirius::library::video::transform::codec::partial::png::compressor::entity_t bitstream;
									bitstream.data = real_compressed_buffer;
									bitstream.data_capacity = real_compressed_buffer - compressed_buffer;
									bitstream.data_capacity = compressed_buffer_size - bitstream.data_capacity;
									bitstream.data_size = 0;
									bitstream.x = input.x;
									bitstream.y = input.y;
									bitstream.width = input.width;
									bitstream.height = input.height;

									status = _real_compressor->compress(&input, &bitstream);
									if (status == sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success)
									{
										px[count] = w;
										py[count] = h;
										pwidth[count] = _context->block_width;
										pheight[count] = _context->block_height;
										pcompressed[count] = (uint8_t*)bitstream.data;
										plength[count] = bitstream.data_size;
										real_compressed_buffer += bitstream.data_size;
										count++;
									}
								}
							}
						}
						_real_compressor->release();
						_front->after_process_callback(count, px, py, pwidth, pheight, pcompressed, plength, before_encode_timestamp, after_encode_timestamp);
						memmove(reference_buffer, process_data, process_data_size);

						if (rows)
						{
							free(rows);
							rows = nullptr;
						}
						_invalidate = false;
					}
					else
					{
						if (count > 0)
						{
							_front->after_process_callback(count, px, py, pwidth, pheight, pcompressed, plength, before_encode_timestamp, after_encode_timestamp);
						}
					}

					for (ccc_iter = ccl_component_vec.begin(); ccc_iter != ccl_component_vec.end(); ccc_iter++)
					{
						connected_component_t * cc = *ccc_iter;
						if (cc)
						{
							cc->elements.clear();
							delete cc;
							cc = nullptr;
						}
					}
					ccl_component_vec.clear();
					//memmove(reference_buffer, process_data, process_data_size);

					{
						sirius::autolock lock(&_cs);
						iobuffer = _iobuffer_queue.get_pending();
						if (iobuffer)
						{
							process_data_size = iobuffer->input.data_size;
							process_timestamp = iobuffer->input.timestamp;
							memmove(process_data, iobuffer->input.data, process_data_size);
							process_x = iobuffer->input.x;
							process_y = iobuffer->input.y;
							process_width = iobuffer->input.width;
							process_height = iobuffer->input.height;
							process_force_fullmode = iobuffer->input.force_fullmode;
							continue;
						}
						else
						{
							process_data_size = 0;
							break;
						}
					}
				}
			}
			_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressed;
		}
	}

	if (process_data)
	{
		_aligned_free(process_data);
		process_data = nullptr;
		process_data_size = 0;
	}

	if (px)
	{
		delete[] px;
		px = nullptr;
	}
	if (py)
	{
		delete[] py;
		py = nullptr;
	}
	if (pwidth)
	{
		delete[] pwidth;
		pwidth = nullptr;
	}
	if (pheight)
	{
		delete[] pheight;
		pheight = nullptr;
	}
	if (plength)
	{
		delete[] plength;
		plength = nullptr;
	}
	if (pcompressed)
	{
		delete[] pcompressed;
		pcompressed = nullptr;
	}

	if (reference_buffer)
	{
		_aligned_free(reference_buffer);
		reference_buffer = nullptr;
		reference_buffer_size = 0;
	}

	if (compressed_buffer)
	{
		free(compressed_buffer);
		compressed_buffer = nullptr;
		compressed_buffer_size = 0;
	}
}

void sirius::library::video::transform::codec::partial::png::compressor::core::process_threaded_coordinated(void)
{
	int32_t status = sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
	sirius::library::video::transform::codec::partial::png::compressor::core::buffer_t * iobuffer = nullptr;

	long long before_encode_timestamp = 0;
	long long after_encode_timestamp = 0;

	int32_t		thread_count = 20;

	int32_t	compressed_buffer_size = 1024 * 1024 * 2;
	uint8_t * compressed_buffer = static_cast<uint8_t*>(malloc(compressed_buffer_size));
	memset(compressed_buffer, 0x00, compressed_buffer_size);

	int32_t		align = 32;
	int32_t		reference_buffer_size = (_context->width * _context->height) << 2;
	uint8_t *	reference_buffer = static_cast<uint8_t*>(_aligned_malloc(reference_buffer_size, align));
	memset(reference_buffer, 0x00, reference_buffer_size);

	int32_t		nbytes_compressed = 1024 * 512;//(1MB) (_context->block_width*_context->block_height) << 2;
	int32_t		block_count = (_context->width / _context->block_width) * (_context->height / _context->block_height);

	int32_t		pcount = _context->width * _context->height;
	int16_t *	px = new int16_t[pcount];
	int16_t *	py = new int16_t[pcount];
	int16_t *	pwidth = new int16_t[pcount];
	int16_t *	pheight = new int16_t[pcount];
	uint8_t **	pcompressed = new uint8_t*[pcount];
	//int32_t *	pcapacity = new int32_t[pcount];
	int32_t *	plength = new int32_t[pcount];

	for (int32_t index = 0; index < pcount; index++)
	{
		px[index] = 0;
		py[index] = 0;
		pwidth[index] = 0;
		pheight[index] = 0;
		//pcapacity[index] = nbytes_compressed;
		//pcompressed[index] = new uint8_t[pcapacity[index]];
		pcompressed[index] = nullptr;
		plength[index] = 0;
	}

	int32_t		process_data_capacity = (_context->width * _context->height) << 2;
	int32_t		process_data_size = 0;
	uint8_t *	process_data = static_cast<uint8_t*>(_aligned_malloc(process_data_capacity, align));
	int32_t		process_x = 0;
	int32_t		process_y = 0;
	int32_t		process_width = 0;
	int32_t		process_height = 0;
	long long	process_timestamp = 0;
	bool		process_force_fullmode = false;
	bool		process_compress_first_time = true;

	coordinated_thread_context_t ** thread_ctx = static_cast<coordinated_thread_context_t**>(malloc(sizeof(coordinated_thread_context_t*)*thread_count));
	for (int32_t tindex = 0; tindex < thread_count; tindex++)
	{
		uint32_t thrdaddr = 0;
		thread_ctx[tindex] = static_cast<coordinated_thread_context_t*>(malloc(sizeof(coordinated_thread_context_t)));

		thread_ctx[tindex]->compressed_buffer_size = 1024 * 512;
		thread_ctx[tindex]->compressed_buffer = static_cast<char*>(malloc(thread_ctx[tindex]->compressed_buffer_size));

		thread_ctx[tindex]->output.data = thread_ctx[tindex]->compressed_buffer;
		thread_ctx[tindex]->output.data_capacity = thread_ctx[tindex]->compressed_buffer_size;
		thread_ctx[tindex]->rows = nullptr;
		thread_ctx[tindex]->pcompressed = nullptr;
		thread_ctx[tindex]->plength = nullptr;

		thread_ctx[tindex]->run = TRUE;
		thread_ctx[tindex]->thread = (HANDLE)::_beginthreadex(NULL, 0, sirius::library::video::transform::codec::partial::png::compressor::core::process_coordinated_encoding_callback, thread_ctx[tindex], 0, &thrdaddr);
		thread_ctx[tindex]->signal = ::CreateEventA(NULL, FALSE, FALSE, NULL);
		thread_ctx[tindex]->available = ::CreateEventA(NULL, FALSE, FALSE, NULL);
		::SetEvent(thread_ctx[tindex]->available);
		thread_ctx[tindex]->parent = this;

		thread_ctx[tindex]->real_compressor = new sirius::library::video::transform::codec::libpng::compressor(_front);
	}

	while (_run)
	{
		if (::WaitForSingleObject(_event, INFINITE) == WAIT_OBJECT_0)
		{
			_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressing;

			{
				sirius::autolock lock(&_cs);
				iobuffer = _iobuffer_queue.get_pending();
				if (!iobuffer)
				{
					if (!_invalidate)
					{
						_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressed;
						::Sleep(10);
						continue;
					}
					process_data_size = 0;
				}
				else
				{
					process_data_size = iobuffer->input.data_size;
					process_timestamp = iobuffer->input.timestamp;
					memmove(process_data, iobuffer->input.data, process_data_size);
					process_x = iobuffer->input.x;
					process_y = iobuffer->input.y;
					process_width = iobuffer->input.width;
					process_height = iobuffer->input.height;
					process_force_fullmode = iobuffer->input.force_fullmode;
				}
			}


			if (process_compress_first_time)
			{
				before_encode_timestamp = process_timestamp;
				int32_t count = 0;
				int32_t index = 0;
				uint8_t * real_compressed_buffer = compressed_buffer;

				std::vector<coordinated_thread_context_t*> thread_contexts;
				for (int32_t h = 0; h < _context->height; h = h + _context->block_height)
				{
					for (int32_t w = 0; w < _context->width; w = w + _context->block_width)
					{
						if (_front)
						{
							while (TRUE)
							{
								BOOL found_thread_ctx = FALSE;
								for (int32_t tindex = 0; tindex < thread_count; tindex++)
								{
									if (::WaitForSingleObject(thread_ctx[tindex]->available, 0) == WAIT_OBJECT_0)
									{
										thread_ctx[tindex]->rows = static_cast<uint8_t**>(malloc(_context->block_height * sizeof(uint8_t*)));
										for (int32_t h2 = 0; h2 < _context->block_height; h2++)
										{
											int32_t src_index = (h + h2) * (_context->width << 2) + (w << 2);
											thread_ctx[tindex]->rows[h2] = process_data + src_index;
										}

										thread_ctx[tindex]->input.data = thread_ctx[tindex]->rows;
										thread_ctx[tindex]->input.data_capacity = _context->block_height;
										thread_ctx[tindex]->input.data_size = _context->block_height;
										thread_ctx[tindex]->input.x = 0;
										thread_ctx[tindex]->input.y = 0;
										thread_ctx[tindex]->input.width = _context->block_width;
										thread_ctx[tindex]->input.height = _context->block_height;

										thread_ctx[tindex]->output.data_size = 0;
										thread_ctx[tindex]->output.x = thread_ctx[tindex]->input.x;
										thread_ctx[tindex]->output.y = thread_ctx[tindex]->input.y;
										thread_ctx[tindex]->output.width = thread_ctx[tindex]->input.width;
										thread_ctx[tindex]->output.height = thread_ctx[tindex]->input.height;


										px[count] = w;
										py[count] = h;
										pwidth[count] = _context->block_width;
										pheight[count] = _context->block_height;
										if (pcompressed[count])
										{
											delete[] pcompressed[count];
											pcompressed[count] = nullptr;
										}
										pcompressed[count] = new uint8_t[nbytes_compressed];
										thread_ctx[tindex]->pcompressed = pcompressed[count];
										thread_ctx[tindex]->plength = &plength[count];

										thread_ctx[tindex]->real_compressor->release();
										thread_ctx[tindex]->real_compressor->initialize(_context);

										std::vector<coordinated_thread_context_t*>::iterator iter = std::find(thread_contexts.begin(), thread_contexts.end(), thread_ctx[tindex]);
										if (iter == thread_contexts.end())
											thread_contexts.push_back(thread_ctx[tindex]);

										count++;
										::SetEvent(thread_ctx[tindex]->signal);
										found_thread_ctx = TRUE;
									}
									if (found_thread_ctx)
										break;
								}
								if (found_thread_ctx)
									break;
							}
						}
						index++;
					}
				}

				std::vector<coordinated_thread_context_t*>::iterator iter;
				for (iter = thread_contexts.begin(); iter != thread_contexts.end(); iter++)
					::WaitForSingleObject((*iter)->available, INFINITE);

				_front->after_process_callback(count, px, py, pwidth, pheight, pcompressed, plength, before_encode_timestamp, after_encode_timestamp);
				memmove(reference_buffer, process_data, process_data_size);

				for (int32_t index = 0; index < pcount; index++)
				{
					if (pcompressed[index])
					{
						delete[] pcompressed[index];
						pcompressed[index] = nullptr;
					}
				}

				for (iter = thread_contexts.begin(); iter != thread_contexts.end(); iter++)
					::SetEvent((*iter)->available);

				process_compress_first_time = false;
				continue;
			}

			if (_invalidate && (process_data_size == 0))
			{
				uint8_t *** rows = static_cast<uint8_t***>(malloc(block_count * sizeof(uint8_t**)));

				before_encode_timestamp = process_timestamp;
				int32_t count = 0;
				int32_t index = 0;
				uint8_t * real_compressed_buffer = compressed_buffer;

				std::vector<coordinated_thread_context_t*> thread_contexts;
				for (int32_t h = 0; h < _context->height; h = h + _context->block_height)
				{
					for (int32_t w = 0; w < _context->width; w = w + _context->block_width)
					{
						if (_front)
						{
							while (TRUE)
							{
								BOOL found_thread_ctx = FALSE;
								for (int32_t tindex = 0; tindex < thread_count; tindex++)
								{
									if (::WaitForSingleObject(thread_ctx[tindex]->available, 0) == WAIT_OBJECT_0)
									{
										thread_ctx[tindex]->rows = static_cast<uint8_t**>(malloc(_context->block_height * sizeof(uint8_t*)));
										for (int32_t h2 = 0; h2 < _context->block_height; h2++)
										{
											int32_t src_index = (h + h2) * (_context->width << 2) + (w << 2);
											thread_ctx[tindex]->rows[h2] = process_data + src_index;
										}

										thread_ctx[tindex]->input.data = thread_ctx[tindex]->rows;
										thread_ctx[tindex]->input.data_capacity = _context->block_height;
										thread_ctx[tindex]->input.data_size = _context->block_height;
										thread_ctx[tindex]->input.x = 0;
										thread_ctx[tindex]->input.y = 0;
										thread_ctx[tindex]->input.width = _context->block_width;
										thread_ctx[tindex]->input.height = _context->block_height;

										thread_ctx[tindex]->output.data_size = 0;
										thread_ctx[tindex]->output.x = thread_ctx[tindex]->input.x;
										thread_ctx[tindex]->output.y = thread_ctx[tindex]->input.y;
										thread_ctx[tindex]->output.width = thread_ctx[tindex]->input.width;
										thread_ctx[tindex]->output.height = thread_ctx[tindex]->input.height;


										px[count] = int16_t(w);
										py[count] = int16_t(h);
										pwidth[count] = int16_t(_context->block_width);
										pheight[count] = int16_t(_context->block_height);
										if (pcompressed[count])
										{
											delete[] pcompressed[count];
											pcompressed[count] = nullptr;
										}
										pcompressed[count] = new uint8_t[nbytes_compressed];
										thread_ctx[tindex]->pcompressed = pcompressed[count];
										thread_ctx[tindex]->plength = &plength[count];

										thread_ctx[tindex]->real_compressor->release();
										thread_ctx[tindex]->real_compressor->initialize(_context);

										std::vector<coordinated_thread_context_t*>::iterator iter = std::find(thread_contexts.begin(), thread_contexts.end(), thread_ctx[tindex]);
										if (iter == thread_contexts.end())
											thread_contexts.push_back(thread_ctx[tindex]);

										count++;
										::SetEvent(thread_ctx[tindex]->signal);
										found_thread_ctx = TRUE;
									}
									if (found_thread_ctx)
										break;
								}
								if (found_thread_ctx)
									break;
							}
						}
						index++;
					}
				}

				std::vector<coordinated_thread_context_t*>::iterator iter;
				for (iter = thread_contexts.begin(); iter != thread_contexts.end(); iter++)
					::WaitForSingleObject((*iter)->available, INFINITE);

				_front->after_process_callback(count, px, py, pwidth, pheight, pcompressed, plength, before_encode_timestamp, after_encode_timestamp);
				memmove(reference_buffer, process_data, process_data_size);

				for (int32_t index = 0; index < pcount; index++)
				{
					if (pcompressed[index])
					{
						delete[] pcompressed[index];
						pcompressed[index] = nullptr;
					}
				}

				for (iter = thread_contexts.begin(); iter != thread_contexts.end(); iter++)
					::SetEvent((*iter)->available);

				_invalidate = false;
			}
			else
			{
				while (process_data_size>0)
				{
					before_encode_timestamp = process_timestamp;
					int32_t count = 0;
					uint8_t * real_compressed_buffer = compressed_buffer;

					int32_t begin_height = 0;
					int32_t end_height = 0;
					int32_t begin_width = 0;
					int32_t end_width = 0;
					int32_t resized_begin_height = 0;
					int32_t resized_end_height = 0;
					int32_t resized_begin_width = 0;
					int32_t resized_end_width = 0;
					if (process_force_fullmode)
					{
						begin_height = 0;
						end_height = _context->height;
						begin_width = 0;
						end_width = _context->width;
					}
					else
					{
						begin_height = (process_y / _context->mb_height) * _context->mb_height;
						end_height = ((process_y + process_height) / _context->mb_height) * _context->mb_height;

						if (((process_y + process_height) % _context->mb_height) > 0)
							end_height += _context->mb_height;

						begin_width = (process_x / _context->mb_width) * _context->mb_width;
						end_width = ((process_x + process_width) / _context->mb_width) * _context->mb_width;
						if (((process_x + process_width) % _context->mb_width) > 0)
							end_width += _context->mb_width;
					}

					// find different area between current and previous images
					std::map<uint64_t, uint32_t> bfgs;
					__declspec(align(32)) uint8_t result[32] = { 0 };
					for (int32_t h = begin_height, y = (begin_height / _context->mb_height); h < end_height; h = h + _context->mb_height, y++)
					{
						for (int32_t w = begin_width, x = (begin_width / _context->mb_width); w < end_width; w = w + _context->mb_width, x++)
						{
							bool diff = false;
							for (int32_t mbh = 0; mbh < _context->mb_height; mbh++)
							{
								for (int32_t mbw = 0; mbw < _context->mb_width; mbw = mbw + (align >> 2))
								{
									int32_t ci = (h + mbh) * (_context->width << 2) + ((w + mbw) << 2);
									uint8_t * p = process_data + ci;
									uint8_t * r = reference_buffer + ci;
									const __m256i current = _mm256_load_si256((__m256i*)p);
									const __m256i reference = _mm256_load_si256((__m256i*)r);
									const __m256i cmpeq = _mm256_cmpeq_epi8(current, reference);
									_mm256_store_si256((__m256i*)result, cmpeq);
									for (int32_t i = 0; i < align; i++)
									{
										if (!result[i])
										{
											uint64_t key = (uint64_t(x) << 32) | (y);
											uint32_t val = 0;
											bfgs.insert(std::make_pair(key, val));
											diff = true;
											break;
										}
									}

									if (diff)
										break;
								}
								if (diff)
									break;
							}
						}
					}

					// make connected component
					unsigned short * pseudo_stack = (unsigned short*)malloc(3 * sizeof(unsigned short)*((_context->width / _context->mb_width) * (_context->height / _context->mb_height) + 1));
					int32_t lable_number = 0;
					std::map<uint64_t, uint32_t>::iterator		ccl_iter;
					std::vector<connected_component_t*>			ccl_component_vec;
					for (ccl_iter = bfgs.begin(); ccl_iter != bfgs.end(); ccl_iter++)
					{
						uint64_t key = ccl_iter->first;
						uint32_t val = ccl_iter->second;
						if (val == UNLABELED) //new component found
						{
							lable_number++;
							//ccl_iter->second = lable_number;
							connected_component_t * cc = new connected_component_t(lable_number);
							ccl_component_vec.push_back(cc);
							uint32_t x = uint32_t((key & 0xFFFFFFFF00000000LL) >> 32);
							uint32_t y = uint32_t(key & 0x00000000FFFFFFFFLL);
							connect_component(pseudo_stack, &bfgs, cc, (_context->width / _context->mb_width), (_context->height / _context->mb_height), x, y);
						}
					}
					free(pseudo_stack);
					pseudo_stack = nullptr;

#if defined(WITH_CALIBRATION)
					std::vector<coordinated_thread_context_t*> thread_contexts;
					std::vector<connected_component_t*>::iterator ccc_iter;
					for (ccc_iter = ccl_component_vec.begin(); ccc_iter != ccl_component_vec.end(); ccc_iter++)
					{
						connected_component_t * cc = (*ccc_iter);
						int32_t cc_x = cc->left * _context->mb_width;
						int32_t cc_y = cc->top * _context->mb_height;
						int32_t cc_height = (cc->bottom - cc->top + 1) * _context->mb_height;
						int32_t cc_width = (cc->right - cc->left + 1) * _context->mb_width;

						int32_t requested_img_size = _context->block_height*_context->block_width;
						std::vector<ccl_info_t> ccl_infos;
						if ((cc_height*cc_width) > requested_img_size)
						{
							int32_t quotient_y = cc_height / _context->block_height;
							int32_t remainder_y = cc_height % _context->block_height;

							int32_t quotient_x = cc_width / _context->block_width;
							int32_t remainder_x = cc_width % _context->block_width;

							if ((quotient_y > 0) && (quotient_x > 0))
							{
								for (int32_t y = 0; y < quotient_y; y++)
								{
									for (int32_t x = 0; x < quotient_x; x++)
									{
										ccl_info_t ccl_info;
										ccl_info.x = cc_x + (x * _context->block_width);
										ccl_info.y = cc_y + (y * _context->block_height);
										ccl_info.width = _context->block_width;
										ccl_info.height = _context->block_height;
										ccl_infos.push_back(ccl_info);
									}
								}

								if (remainder_y > 0)
								{
									int32_t target_img_size = remainder_y * quotient_x * _context->block_width;
									if (target_img_size > requested_img_size)
									{
										int32_t quotient = target_img_size / requested_img_size;
										int32_t cc_width2 = (quotient_x * _context->block_width) / quotient;

										for (int32_t x = 0; x < quotient; x++)
										{
											ccl_info_t ccl_info;
											ccl_info.x = cc_x + (x * cc_width2);
											ccl_info.y = cc_y + (quotient_y * _context->block_height);
											ccl_info.width = cc_width2;
											ccl_info.height = remainder_y;
											ccl_infos.push_back(ccl_info);
										}

										int32_t cc_width3 = cc_width - cc_width2 * quotient;
										if (cc_width3> 0)
										{
											ccl_info_t ccl_info;
											ccl_info.x = cc_x + (quotient * cc_width2);
											ccl_info.y = cc_y + (quotient_y * _context->block_height);
											ccl_info.width = cc_width3;
											ccl_info.height = remainder_y;
											ccl_infos.push_back(ccl_info);
										}
									}
									else
									{
										ccl_info_t ccl_info;
										ccl_info.x = cc_x;
										ccl_info.y = cc_y + (quotient_y * _context->block_height);
										ccl_info.width = quotient_x * _context->block_width;
										ccl_info.height = remainder_y;
										ccl_infos.push_back(ccl_info);
									}
								}

								if (remainder_x > 0)
								{
									int32_t target_img_size = remainder_x * ((quotient_y * _context->block_height) + remainder_y);
									if (target_img_size > requested_img_size)
									{
										int32_t quotient = target_img_size / requested_img_size;
										int32_t cc_height2 = ((quotient_y * _context->block_height) + remainder_y) / quotient;
										for (int32_t y = 0; y < quotient; y++)
										{
											ccl_info_t ccl_info;
											ccl_info.x = cc_x + (quotient_x * _context->block_width);
											ccl_info.y = cc_y + (y * cc_height2);
											ccl_info.width = remainder_x;
											ccl_info.height = cc_height2;
											ccl_infos.push_back(ccl_info);
										}

										int32_t cc_height3 = ((quotient_y * _context->block_height) + remainder_y) - cc_height2 * quotient;
										if (cc_height3 > 0)
										{
											ccl_info_t ccl_info;
											ccl_info.x = cc_x + (quotient_x * _context->block_width);
											ccl_info.y = cc_y + (quotient * cc_height2);
											ccl_info.width = remainder_x;
											ccl_info.height = cc_height3;
											ccl_infos.push_back(ccl_info);
										}
									}
									else
									{
										ccl_info_t ccl_info;
										ccl_info.x = cc_x + (quotient_x * _context->block_width);
										ccl_info.y = cc_y;
										ccl_info.width = remainder_x;
										ccl_info.height = (quotient_y * _context->block_height) + remainder_y;
										ccl_infos.push_back(ccl_info);
									}
								}
							}
							else if (quotient_y > 0)
							{
								int32_t quotient = (cc_height*cc_width) / requested_img_size;
								int32_t cc_height2 = cc_height / quotient;
								for (int32_t y = 0; y < quotient; y++)
								{
									ccl_info_t ccl_info;
									ccl_info.x = cc_x;
									ccl_info.y = cc_y + (y * cc_height2);
									ccl_info.width = cc_width;
									ccl_info.height = cc_height2;
									ccl_infos.push_back(ccl_info);
								}

								int32_t cc_height3 = cc_height - cc_height2 * quotient;
								if (cc_height3 > 0)
								{
									ccl_info_t ccl_info;
									ccl_info.x = cc_x;
									ccl_info.y = cc_y + (quotient * cc_height2);
									ccl_info.width = cc_width;
									ccl_info.height = cc_height3;
									ccl_infos.push_back(ccl_info);
								}
							}
							else if (quotient_x > 0)
							{
								int32_t quotient = (cc_height*cc_width) / requested_img_size;
								int32_t cc_width2 = cc_width / quotient;
								for (int32_t x = 0; x < quotient; x++)
								{
									ccl_info_t ccl_info;
									ccl_info.x = cc_x + (x * cc_width2);
									ccl_info.y = cc_y;
									ccl_info.width = cc_width2;
									ccl_info.height = cc_height;
									ccl_infos.push_back(ccl_info);
								}

								int32_t cc_width3 = cc_width - cc_width2 * quotient;
								if (cc_width3> 0)
								{
									ccl_info_t ccl_info;
									ccl_info.x = cc_x + (quotient * cc_width2);
									ccl_info.y = cc_y;
									ccl_info.width = cc_width3;
									ccl_info.height = cc_height;
									ccl_infos.push_back(ccl_info);
								}
							}
							else
							{
								ccl_info_t ccl_info;
								ccl_info.x = cc_x;
								ccl_info.y = cc_y;
								ccl_info.width = cc_width;
								ccl_info.height = cc_height;
								ccl_infos.push_back(ccl_info);
							}
						}
						else
						{
							ccl_info_t ccl_info;
							ccl_info.x = cc_x;
							ccl_info.y = cc_y;
							ccl_info.width = cc_width;
							ccl_info.height = cc_height;
							ccl_infos.push_back(ccl_info);
						}

						if (_front)
						{
							std::vector<ccl_info_t>::iterator ccl_info_iter;
							for (ccl_info_iter = ccl_infos.begin(); ccl_info_iter != ccl_infos.end(); ccl_info_iter++)
							{
								ccl_info_t ccl_info = *ccl_info_iter;
								while (TRUE)
								{
									/*
									if ((ccl_info.width*ccl_info.height) > (_context->block_width * _context->block_height))
									{
									char debug[MAX_PATH] = { 0 };
									_snprintf_s(debug, MAX_PATH, "ccl_info.width=%d, ccl_info.height=%d \n", ccl_info.width, ccl_info.height);
									::OutputDebugStringA(debug);
									}
									*/

									BOOL found_thread_ctx = FALSE;
									for (int32_t tindex = 0; tindex < thread_count; tindex++)
									{
										if (::WaitForSingleObject(thread_ctx[tindex]->available, 0) == WAIT_OBJECT_0)
										{
											thread_ctx[tindex]->rows = static_cast<uint8_t**>(malloc(ccl_info.height * sizeof(uint8_t*)));
											for (int32_t h = 0; h < ccl_info.height; h++)
											{
												int32_t src_index = (ccl_info.y + h) * (_context->width << 2) + (ccl_info.x << 2);
												thread_ctx[tindex]->rows[h] = process_data + src_index;
												memmove(reference_buffer + src_index, process_data + src_index, ccl_info.width << 2);
											}

											thread_ctx[tindex]->input.data = thread_ctx[tindex]->rows;
											thread_ctx[tindex]->input.data_capacity = ccl_info.height;
											thread_ctx[tindex]->input.data_size = ccl_info.height;
											thread_ctx[tindex]->input.x = 0;
											thread_ctx[tindex]->input.y = 0;
											thread_ctx[tindex]->input.width = ccl_info.width;
											thread_ctx[tindex]->input.height = ccl_info.height;

											thread_ctx[tindex]->output.data_size = 0;
											thread_ctx[tindex]->output.x = thread_ctx[tindex]->input.x;
											thread_ctx[tindex]->output.y = thread_ctx[tindex]->input.y;
											thread_ctx[tindex]->output.width = thread_ctx[tindex]->input.width;
											thread_ctx[tindex]->output.height = thread_ctx[tindex]->input.height;

											px[count] = int16_t(ccl_info.x);
											py[count] = int16_t(ccl_info.y);
											pwidth[count] = int16_t(ccl_info.width);
											pheight[count] = int16_t(ccl_info.height);
											if (pcompressed[count])
											{
												delete[] pcompressed[count];
												pcompressed[count] = nullptr;
											}
											pcompressed[count] = new uint8_t[nbytes_compressed];
											thread_ctx[tindex]->pcompressed = pcompressed[count];
											thread_ctx[tindex]->plength = &plength[count];

											thread_ctx[tindex]->real_compressor->release();
											thread_ctx[tindex]->real_compressor->initialize(_context);

											std::vector<coordinated_thread_context_t*>::iterator iter = std::find(thread_contexts.begin(), thread_contexts.end(), thread_ctx[tindex]);
											if (iter == thread_contexts.end())
											{
												thread_contexts.push_back(thread_ctx[tindex]);
											}

											count++;
											::SetEvent(thread_ctx[tindex]->signal);
											found_thread_ctx = TRUE;
										}
										if (found_thread_ctx)
											break;
									}
									if (found_thread_ctx)
										break;
								}
							}
						}
					}

#else
					std::vector<coordinated_thread_context_t*> thread_contexts;
					std::vector<connected_component_t*>::iterator ccc_iter;
					for (ccc_iter = ccl_component_vec.begin(); ccc_iter != ccl_component_vec.end(); ccc_iter++)
					{
						connected_component_t * cc = (*ccc_iter);
						int32_t cc_x = cc->left * _context->mb_width;
						int32_t cc_y = cc->top * _context->mb_height;
						int32_t cc_height = (cc->bottom - cc->top + 1) * _context->mb_height;
						int32_t cc_width = (cc->right - cc->left + 1) * _context->mb_width;

						if (_front)
						{
							while (TRUE)
							{
								/*
								if ((ccl_info.width*ccl_info.height) > (_context->block_width * _context->block_height))
								{
									char debug[MAX_PATH] = { 0 };
									_snprintf_s(debug, MAX_PATH, "ccl_info.width=%d, ccl_info.height=%d \n", ccl_info.width, ccl_info.height);
									::OutputDebugStringA(debug);
								}
								*/

								BOOL found_thread_ctx = FALSE;
								for (int32_t tindex = 0; tindex < thread_count; tindex++)
								{
									if (::WaitForSingleObject(thread_ctx[tindex]->available, 0) == WAIT_OBJECT_0)
									{
										thread_ctx[tindex]->rows = static_cast<uint8_t**>(malloc(cc_height * sizeof(uint8_t*)));
										for (int32_t h = 0; h < cc_height; h++)
										{
											int32_t src_index = (cc_y + h) * (_context->width << 2) + (cc_x << 2);
											thread_ctx[tindex]->rows[h] = process_data + src_index;
											memmove(reference_buffer + src_index, process_data + src_index, cc_width << 2);
										}

										thread_ctx[tindex]->input.data = thread_ctx[tindex]->rows;
										thread_ctx[tindex]->input.data_capacity = cc_height;
										thread_ctx[tindex]->input.data_size = cc_height;
										thread_ctx[tindex]->input.x = 0;
										thread_ctx[tindex]->input.y = 0;
										thread_ctx[tindex]->input.width = cc_width;
										thread_ctx[tindex]->input.height = cc_height;

										thread_ctx[tindex]->output.data_size = 0;
										thread_ctx[tindex]->output.x = thread_ctx[tindex]->input.x;
										thread_ctx[tindex]->output.y = thread_ctx[tindex]->input.y;
										thread_ctx[tindex]->output.width = thread_ctx[tindex]->input.width;
										thread_ctx[tindex]->output.height = thread_ctx[tindex]->input.height;

										px[count] = int16_t(cc_x);
										py[count] = int16_t(cc_y);
										pwidth[count] = int16_t(cc_width);
										pheight[count] = int16_t(cc_height);
										if (pcompressed[count])
										{
											delete[] pcompressed[count];
											pcompressed[count] = nullptr;
										}
										pcompressed[count] = new uint8_t[nbytes_compressed];
										thread_ctx[tindex]->pcompressed = pcompressed[count];
										thread_ctx[tindex]->plength = &plength[count];

										thread_ctx[tindex]->real_compressor->release();
										thread_ctx[tindex]->real_compressor->initialize(_context);

										std::vector<coordinated_thread_context_t*>::iterator iter = std::find(thread_contexts.begin(), thread_contexts.end(), thread_ctx[tindex]);
										if (iter == thread_contexts.end())
										{
											thread_contexts.push_back(thread_ctx[tindex]);
										}

										count++;
										::SetEvent(thread_ctx[tindex]->signal);
										found_thread_ctx = TRUE;
									}
									if (found_thread_ctx)
										break;
								}
								if (found_thread_ctx)
									break;
							}
						}
					}
#endif

					std::vector<coordinated_thread_context_t*>::iterator iter;
					for (iter = thread_contexts.begin(); iter != thread_contexts.end(); iter++)
						::WaitForSingleObject((*iter)->available, INFINITE);

					process_force_fullmode = false;
					if (_invalidate)
					{
						for (iter = thread_contexts.begin(); iter != thread_contexts.end(); iter++)
							::SetEvent((*iter)->available);
						thread_contexts.clear();

						before_encode_timestamp = process_timestamp;
						int32_t count = 0;
						int32_t index = 0;
						uint8_t * real_compressed_buffer = compressed_buffer;

						for (int32_t h = 0; h < _context->height; h = h + _context->block_height)
						{
							for (int32_t w = 0; w < _context->width; w = w + _context->block_width)
							{
								if (_front)
								{
									while (TRUE)
									{
										BOOL found_thread_ctx = FALSE;
										for (int32_t tindex = 0; tindex < thread_count; tindex++)
										{
											if (::WaitForSingleObject(thread_ctx[tindex]->available, 0) == WAIT_OBJECT_0)
											{
												thread_ctx[tindex]->rows = static_cast<uint8_t**>(malloc(_context->block_height * sizeof(uint8_t*)));
												for (int32_t h2 = 0; h2 < _context->block_height; h2++)
												{
													int32_t src_index = (h + h2) * (_context->width << 2) + (w << 2);
													thread_ctx[tindex]->rows[h2] = process_data + src_index;
												}

												thread_ctx[tindex]->input.data = thread_ctx[tindex]->rows;
												thread_ctx[tindex]->input.data_capacity = _context->block_height;
												thread_ctx[tindex]->input.data_size = _context->block_height;
												thread_ctx[tindex]->input.x = 0;
												thread_ctx[tindex]->input.y = 0;
												thread_ctx[tindex]->input.width = _context->block_width;
												thread_ctx[tindex]->input.height = _context->block_height;

												thread_ctx[tindex]->output.data_size = 0;
												thread_ctx[tindex]->output.x = thread_ctx[tindex]->input.x;
												thread_ctx[tindex]->output.y = thread_ctx[tindex]->input.y;
												thread_ctx[tindex]->output.width = thread_ctx[tindex]->input.width;
												thread_ctx[tindex]->output.height = thread_ctx[tindex]->input.height;


												px[count] = int16_t(w);
												py[count] = int16_t(h);
												pwidth[count] = int16_t(_context->block_width);
												pheight[count] = int16_t(_context->block_height);
												if (pcompressed[count])
												{
													delete[] pcompressed[count];
													pcompressed[count] = nullptr;
												}
												pcompressed[count] = new uint8_t[nbytes_compressed];

												thread_ctx[tindex]->pcompressed = pcompressed[count];
												thread_ctx[tindex]->plength = &plength[count];

												thread_ctx[tindex]->real_compressor->release();
												thread_ctx[tindex]->real_compressor->initialize(_context);

												std::vector<coordinated_thread_context_t*>::iterator iter = std::find(thread_contexts.begin(), thread_contexts.end(), thread_ctx[tindex]);
												if (iter == thread_contexts.end())
													thread_contexts.push_back(thread_ctx[tindex]);

												count++;
												::SetEvent(thread_ctx[tindex]->signal);
												found_thread_ctx = TRUE;
											}
											if (found_thread_ctx)
												break;
										}
										if (found_thread_ctx)
											break;
									}
								}
								index++;
							}
						}

						std::vector<coordinated_thread_context_t*>::iterator iter;
						for (iter = thread_contexts.begin(); iter != thread_contexts.end(); iter++)
							::WaitForSingleObject((*iter)->available, INFINITE);

						_front->after_process_callback(count, px, py, pwidth, pheight, pcompressed, plength, before_encode_timestamp, after_encode_timestamp);
						memmove(reference_buffer, process_data, process_data_size);


						_invalidate = false;
					}
					else
					{
						if (count > 0)
						{
							_front->after_process_callback(count, px, py, pwidth, pheight, pcompressed, plength, before_encode_timestamp, after_encode_timestamp);
						}
					}

					for (ccc_iter = ccl_component_vec.begin(); ccc_iter != ccl_component_vec.end(); ccc_iter++)
					{
						connected_component_t * cc = *ccc_iter;
						if (cc)
						{
							cc->elements.clear();
							delete cc;
							cc = nullptr;
						}
					}
					ccl_component_vec.clear();

					for (int32_t index = 0; index < pcount; index++)
					{
						if (pcompressed[index])
						{
							delete[] pcompressed[index];
							pcompressed[index] = nullptr;
						}
					}

					for (iter = thread_contexts.begin(); iter != thread_contexts.end(); iter++)
						::SetEvent((*iter)->available);
					thread_contexts.clear();

					{
						sirius::autolock lock(&_cs);
						iobuffer = _iobuffer_queue.get_pending();
						if (iobuffer)
						{
							process_data_size = iobuffer->input.data_size;
							process_timestamp = iobuffer->input.timestamp;
							memmove(process_data, iobuffer->input.data, process_data_size);
							process_x = iobuffer->input.x;
							process_y = iobuffer->input.y;
							process_width = iobuffer->input.width;
							process_height = iobuffer->input.height;
							process_force_fullmode = iobuffer->input.force_fullmode;
							continue;
						}
						else
						{
							process_data_size = 0;
							break;
						}
					}
				}
			}
			_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressed;
		}
	}

	for (int32_t tindex = 0; tindex < thread_count; tindex++)
	{
		thread_ctx[tindex]->run = FALSE;
		if (::WaitForSingleObject(thread_ctx[tindex]->available, INFINITE) == WAIT_OBJECT_0)
		{
			::SetEvent(thread_ctx[tindex]->signal);

			if (::WaitForSingleObject(thread_ctx[tindex]->thread, INFINITE) == WAIT_OBJECT_0)
			{
				::CloseHandle(thread_ctx[tindex]->thread);
				thread_ctx[tindex]->thread = INVALID_HANDLE_VALUE;

				::CloseHandle(thread_ctx[tindex]->available);
				::CloseHandle(thread_ctx[tindex]->signal);
				thread_ctx[tindex]->available = INVALID_HANDLE_VALUE;
				thread_ctx[tindex]->signal = INVALID_HANDLE_VALUE;
			}
		}

		if (thread_ctx[tindex]->rows)
		{
			free(thread_ctx[tindex]->rows);
			thread_ctx[tindex]->rows = nullptr;
		}
		thread_ctx[tindex]->real_compressor->release();
		delete thread_ctx[tindex]->real_compressor;
		thread_ctx[tindex]->real_compressor = nullptr;

		free(thread_ctx[tindex]->compressed_buffer);
		thread_ctx[tindex]->compressed_buffer = nullptr;

		thread_ctx[tindex]->pcompressed = nullptr;
		thread_ctx[tindex]->plength = nullptr;

		free(thread_ctx[tindex]);
		thread_ctx[tindex] = nullptr;
	}
	free(thread_ctx);
	thread_ctx = nullptr;

	if (process_data)
	{
		_aligned_free(process_data);
		process_data = nullptr;
		process_data_size = 0;
	}

	if (px)
	{
		delete[] px;
		px = nullptr;
	}
	if (py)
	{
		delete[] py;
		py = nullptr;
	}
	if (pwidth)
	{
		delete[] pwidth;
		pwidth = nullptr;
	}
	if (pheight)
	{
		delete[] pheight;
		pheight = nullptr;
	}
	if (plength)
	{
		delete[] plength;
		plength = nullptr;
	}
	if (pcompressed)
	{
		for (int32_t index = 0; index < pcount; index++)
		{
			if (pcompressed[index])
			{
				delete[] pcompressed[index];
				pcompressed[index] = nullptr;
			}
		}
		delete[] pcompressed;
		pcompressed = nullptr;
	}

	if (reference_buffer)
	{
		_aligned_free(reference_buffer);
		reference_buffer = nullptr;
		reference_buffer_size = 0;
	}

	if (compressed_buffer)
	{
		free(compressed_buffer);
		compressed_buffer = nullptr;
		compressed_buffer_size = 0;
	}
}

#define RETURN { SP -= 3;                \
                 switch (pseudo_stack[SP+2])    \
                 {                       \
                 case 1 : goto RETURN1;  \
                 case 2 : goto RETURN2;  \
                 case 3 : goto RETURN3;  \
                 case 4 : goto RETURN4;  \
                 default: return;        \
                 }                       \
               }
#define X (pseudo_stack[SP-3])
#define Y (pseudo_stack[SP-2])

void sirius::library::video::transform::codec::partial::png::compressor::core::connect_component(unsigned short * pseudo_stack,
																								 std::map<uint64_t, uint32_t> * bfgs, connected_component_t * cc,
																								 uint32_t width, uint32_t height, uint32_t x, uint32_t y)
{
	pseudo_stack[0] = x;
	pseudo_stack[1] = y;
	pseudo_stack[2] = UNLABELED;
	int32_t SP = 3;

START:

	uint64_t key = (uint64_t(X) << 32) | Y;
	std::map<uint64_t, uint32_t>::iterator ccl_iter = bfgs->find(key);
	if (ccl_iter == bfgs->end())
		RETURN;
	if (ccl_iter->second != UNLABELED)
		RETURN;

	ccl_iter->second = cc->number;
	if (X < cc->left)
		cc->left = X;
	if (Y < cc->top)
		cc->top = Y;
	if (X > cc->right)
		cc->right = X;
	if (Y > cc->bottom)
		cc->bottom = Y;
	cc->elements.push_back(key);

	if (X > 0)
	{
		pseudo_stack[SP] = X - 1;
		pseudo_stack[SP + 1] = Y;
		pseudo_stack[SP + 2] = 1;
		SP += 3;
		goto START;
	}

RETURN1:
	if (X < width - 1)
	{
		pseudo_stack[SP] = X + 1;
		pseudo_stack[SP + 1] = Y;
		pseudo_stack[SP + 2] = 2;
		SP += 3;
		goto START;
	}

RETURN2:
	if (Y > 0)
	{
		pseudo_stack[SP] = X;
		pseudo_stack[SP + 1] = Y - 1;
		pseudo_stack[SP + 2] = 3;
		SP += 3;
		goto START;
	}

RETURN3:
	if (Y < height - 1)
	{
		pseudo_stack[SP] = X;
		pseudo_stack[SP + 1] = Y + 1;
		pseudo_stack[SP + 2] = 4;
		SP += 3;
		goto START;
	}

RETURN4:

	RETURN;
}

unsigned __stdcall sirius::library::video::transform::codec::partial::png::compressor::core::process_indexed_encoding_callback(void * param)
{
	sirius::library::video::transform::codec::partial::png::compressor::core::indexed_thread_context_t * self = static_cast<sirius::library::video::transform::codec::partial::png::compressor::core::indexed_thread_context_t*>(param);
	self->parent->process_threaded_indexed_encoding(self);
	return 0;
}

void sirius::library::video::transform::codec::partial::png::compressor::core::process_threaded_indexed_encoding(indexed_thread_context_t * thread_ctx)
{

	while (thread_ctx->run)
	{
		if (WaitForSingleObject(thread_ctx->signal, INFINITE) == WAIT_OBJECT_0)
		{
			if (thread_ctx->run)
			{
				int32_t status = thread_ctx->real_compressor->compress(&thread_ctx->input, &thread_ctx->output);
				if (status == sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success)
				{
					(*thread_ctx->pindex) = thread_ctx->index;
					(*thread_ctx->plength) = thread_ctx->output.data_size;
					memmove(thread_ctx->pcompressed, thread_ctx->output.data, (*thread_ctx->plength));

					(*thread_ctx->cached_index) = thread_ctx->index;
					(*thread_ctx->cached_length) = thread_ctx->output.data_size;
					memmove(thread_ctx->cached_compressed, thread_ctx->output.data, (*thread_ctx->cached_length));
				}
			}
			::SetEvent(thread_ctx->available);
		}
	}
}

void sirius::library::video::transform::codec::partial::png::compressor::core::process_indexed(void)
{
	int32_t status = sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
	sirius::library::video::transform::codec::partial::png::compressor::core::buffer_t * iobuffer = nullptr;

	long long before_encode_timestamp = 0;
	long long after_encode_timestamp = 0;

	int32_t	compressed_buffer_size = 1024 * 1024 * 2;
	uint8_t * compressed_buffer = static_cast<uint8_t*>(malloc(compressed_buffer_size));
	memset(compressed_buffer, 0x00, compressed_buffer_size);

	int32_t		simd_align = 32;
	int32_t		reference_buffer_size = (_context->width * _context->height) << 2;
	uint8_t *	reference_buffer = static_cast<uint8_t*>(_aligned_malloc(reference_buffer_size, simd_align));
	memset(reference_buffer, 0x00, reference_buffer_size);
	uint8_t ** rows = static_cast<uint8_t**>(malloc(_context->block_height * sizeof(uint8_t*)));

	int32_t		block_count = (_context->width / _context->block_width) * (_context->height / _context->block_height);
	int32_t *	pindex = new int32_t[block_count];
	uint8_t **	pcompressed = new uint8_t*[block_count];
	int32_t *	plength = new int32_t[block_count];

	int32_t		nbytes_compressed = (_context->block_width*_context->block_height) << 2;
	int32_t *	cached_index = new int32_t[block_count];
	uint8_t **	cached_compressed = new uint8_t*[block_count];
	int32_t *	cached_capacity = new int32_t[block_count];
	int32_t *	cached_length = new int32_t[block_count];
	for (int32_t index = 0; index < block_count; index++)
	{
		cached_index[index] = index;
		cached_capacity[index] = nbytes_compressed;
		cached_compressed[index] = new uint8_t[cached_capacity[index]];
		cached_length[index] = 0;
	}

	int32_t		process_data_capacity = (_context->width * _context->height) << 2;
	int32_t		process_data_size = 0;
	uint8_t *	process_data = static_cast<uint8_t*>(_aligned_malloc(process_data_capacity, simd_align));
	int32_t		process_x = 0;
	int32_t		process_y = 0;
	int32_t		process_width = 0;
	int32_t		process_height = 0;
	long long	process_timestamp = 0;
	bool		process_force_fullmode = false;
	bool		process_compress_first_time = true;

	_real_compressor->initialize(_context);

	while (_run)
	{
		if (::WaitForSingleObject(_event, INFINITE) == WAIT_OBJECT_0)
		{
			_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressing;

			{
				sirius::autolock lock(&_cs);
				iobuffer = _iobuffer_queue.get_pending();
				if (!iobuffer)
				{
					if (!_invalidate)
					{
						_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressed;
						::Sleep(10);
						continue;
					}
					process_data_size = 0;
				}
				else
				{
					process_data_size = iobuffer->input.data_size;
					process_timestamp = iobuffer->input.timestamp;
					memmove(process_data, iobuffer->input.data, process_data_size);
					process_x = iobuffer->input.x;
					process_y = iobuffer->input.y;
					process_width = iobuffer->input.width;
					process_height = iobuffer->input.height;
					process_force_fullmode = iobuffer->input.force_fullmode;
				}
			}

			if (process_compress_first_time)
			{
				before_encode_timestamp = process_timestamp;
				int32_t count = 0;
				int32_t index = 0;
				uint8_t * real_compressed_buffer = compressed_buffer;

				for (int32_t h = 0; h < _context->height; h = h + _context->block_height)
				{
					for (int32_t w = 0; w < _context->width; w = w + _context->block_width)
					{
						for (int32_t eh = 0, row_index = 0; eh < _context->block_height; eh++, row_index++)
						{
							int32_t src_index = (h + eh) * (_context->width << 2) + (w << 2);
							rows[row_index] = process_data + src_index;
						}

						if (_front)
						{
							sirius::library::video::transform::codec::partial::png::compressor::entity_t input;
							input.data = rows;
							input.data_capacity = _context->block_height;
							input.data_size = _context->block_height;
							input.x = 0;
							input.y = 0;
							input.width = _context->block_width;
							input.height = _context->block_height;

							sirius::library::video::transform::codec::partial::png::compressor::entity_t bitstream;
							bitstream.data = real_compressed_buffer;
							bitstream.data_capacity = real_compressed_buffer - compressed_buffer;
							bitstream.data_capacity = compressed_buffer_size - bitstream.data_capacity;
							bitstream.data_size = 0;
							bitstream.x = input.x;
							bitstream.y = input.y;
							bitstream.width = input.width;
							bitstream.height = input.height;

							status = _real_compressor->compress(&input, &bitstream);
							if (status == sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success)
							{
								pindex[count] = index;
								pcompressed[count] = (uint8_t*)bitstream.data;
								plength[count] = bitstream.data_size;
								real_compressed_buffer += bitstream.data_size;

								cached_index[index] = index;
								cached_length[index] = bitstream.data_size;
								memmove(cached_compressed[index], pcompressed[count], cached_length[index]);

								count++;
							}
						}
						index++;
					}
				}
				_front->after_process_callback(count, pindex, pcompressed, plength, before_encode_timestamp, after_encode_timestamp);

				process_compress_first_time = false;
				continue;
			}

			if (_invalidate && (process_data_size == 0))
			{
				_front->after_process_callback(block_count, cached_index, cached_compressed, cached_length, before_encode_timestamp, after_encode_timestamp);
				_invalidate = false;
			}
			else
			{
				while (process_data_size>0)
				{
					before_encode_timestamp = process_timestamp;
					int32_t count = 0;
					int32_t index = 0;
					uint8_t * real_compressed_buffer = compressed_buffer;

					int32_t begin_height = 0;
					int32_t end_height = 0;
					int32_t begin_width = 0;
					int32_t end_width = 0;
					int32_t resized_begin_height = 0;
					int32_t resized_end_height = 0;
					int32_t resized_begin_width = 0;
					int32_t resized_end_width = 0;
					if (process_force_fullmode)
					{
						begin_height = 0;
						end_height = _context->height;
						begin_width = 0;
						end_width = _context->width;
					}
					else
					{
						begin_height = (process_y / _context->block_height) * _context->block_height;
						end_height = ((process_y + process_height) / _context->block_height) * _context->block_height;

						if (((process_y + process_height) % _context->block_height) > 0)
							end_height += _context->block_height;

						begin_width = (process_x / _context->block_width) * _context->block_width;
						end_width = ((process_x + process_width) / _context->block_width) * _context->block_width;
						if (((process_x + process_width) % _context->block_width) > 0)
							end_width += _context->block_width;
					}

					for (int32_t h = begin_height; h < end_height; h = h + _context->block_height)
					{
						for (int32_t w = begin_width; w < end_width; w = w + _context->block_width)
						{
							//if (h >= begin_height && h < end_height && w >= begin_width && w < end_width)
							{
								bool	diff = false;
								__declspec(align(32)) uint8_t simd_result[32] = { 0 };
								for (int32_t bh = 0; bh < _context->block_height; bh++)
								{
									for (int32_t bw = 0; bw < _context->block_width; bw = bw + (simd_align >> 2))
									{
										int32_t ci = (h + bh) * (_context->width << 2) + (w << 2) + (bw << 2);
										uint8_t * p = process_data + ci;
										uint8_t * r = reference_buffer + ci;
										const __m256i current = _mm256_load_si256((__m256i*)p);
										const __m256i reference = _mm256_load_si256((__m256i*)r);
										const __m256i cmpeq = _mm256_cmpeq_epi8(current, reference);
										_mm256_store_si256((__m256i*)simd_result, cmpeq);
										for (int32_t index = 0; index < simd_align; index++)
										{
											if (!simd_result[index])
											{
												diff = true;
												break;
											}
										}

										if (diff)
											break;
									}
									if (diff)
										break;
								}

								if (diff)
								{
									for (int32_t eh = 0, row_index = 0; eh < _context->block_height; eh++, row_index++)
									{
										int32_t src_index = (h + eh) * (_context->width << 2) + (w << 2);
										//*(process_data + src_index) = 0;
										//*(process_data + src_index + 1) = 0;
										//*(process_data + src_index + 2) = 0;
										//if (_context->block_height == (bh + 1))
										//	memset(process_data + src_index, 0x00, _context->block_width << 2);
										rows[row_index] = process_data + src_index;
										memmove(reference_buffer + +src_index, process_data + src_index, _context->block_width << 2);
									}

									if (_front)
									{
										sirius::library::video::transform::codec::partial::png::compressor::entity_t input;
										input.data = rows;
										input.data_capacity = _context->block_height;
										input.data_size = _context->block_height;
										input.x = 0;
										input.y = 0;
										input.width = _context->block_width;
										input.height = _context->block_height;

										sirius::library::video::transform::codec::partial::png::compressor::entity_t bitstream;
										bitstream.data = real_compressed_buffer;
										bitstream.data_capacity = real_compressed_buffer - compressed_buffer;
										bitstream.data_capacity = compressed_buffer_size - bitstream.data_capacity;
										bitstream.data_size = 0;
										bitstream.x = input.x;
										bitstream.y = input.y;
										bitstream.width = input.width;
										bitstream.height = input.height;

										status = _real_compressor->compress(&input, &bitstream);
										if (status == sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success)
										{
											index = (h / _context->block_height) * (_context->width / _context->block_width) + w / _context->block_width;
											pindex[count] = index;
											pcompressed[count] = (uint8_t*)bitstream.data;
											plength[count] = bitstream.data_size;
											real_compressed_buffer += bitstream.data_size;

											cached_index[index] = index;
											cached_length[index] = bitstream.data_size;
											memmove(cached_compressed[index], pcompressed[count], cached_length[index]);

											count++;
										}
									}
								}
							}
						}
					}

					process_force_fullmode = false;
					if (_invalidate)
					{
						_front->after_process_callback(block_count, cached_index, cached_compressed, cached_length, before_encode_timestamp, after_encode_timestamp);
						_invalidate = false;
					}
					else
					{
						if (count > 0)
						{
							_front->after_process_callback(count, pindex, pcompressed, plength, before_encode_timestamp, after_encode_timestamp);
						}
					}

					{
						sirius::autolock lock(&_cs);
						iobuffer = _iobuffer_queue.get_pending();
						if (iobuffer)
						{
							process_data_size = iobuffer->input.data_size;
							process_timestamp = iobuffer->input.timestamp;
							memmove(process_data, iobuffer->input.data, process_data_size);
							process_x = iobuffer->input.x;
							process_y = iobuffer->input.y;
							process_width = iobuffer->input.width;
							process_height = iobuffer->input.height;
							process_force_fullmode = iobuffer->input.force_fullmode;
							continue;
						}
						else
						{
							process_data_size = 0;
							break;
						}
					}
				}
			}
			_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressed;
		}
	}

	_real_compressor->release();

	if (process_data)
		_aligned_free(process_data);
	process_data = nullptr;

	if (pindex)
		delete[] pindex;
	if (plength)
		delete[] plength;
	if (pcompressed)
		delete[] pcompressed;

	if (cached_index)
		delete[] cached_index;
	if (cached_length)
		delete[] cached_length;
	if (cached_capacity)
		delete[] cached_capacity;
	if (cached_compressed)
	{
		for (int32_t index = 0; index < block_count; index++)
		{
			delete[] cached_compressed[index];
		}
		delete[] cached_compressed;
	}

	if (rows)
		free(rows);
	rows = nullptr;

#if defined(WITH_AVX2_SIMD)
	if (gray_buffer)
		free(gray_buffer);
	gray_buffer = nullptr;
	gray_buffer_size = 0;

	if (resized_buffer)
		free(resized_buffer);
	resized_buffer = nullptr;
	resized_buffer_size = 0;

	if (reference_buffer)
		free(reference_buffer);
	reference_buffer = nullptr;
#else
#if defined(WITH_AVX2_BILINEAR_RESIZE)
	if (resized_buffer)
		free(resized_buffer);
	resized_buffer = nullptr;
	resized_buffer_size = 0;

	if (reference_buffer)
		free(reference_buffer);
	reference_buffer = nullptr;
#else
	if (reference_buffer)
		_aligned_free(reference_buffer);
	reference_buffer = nullptr;
	reference_buffer_size = 0;
#endif
#endif

	if (compressed_buffer)
		free(compressed_buffer);
	compressed_buffer = nullptr;
	compressed_buffer_size = 0;
}

void sirius::library::video::transform::codec::partial::png::compressor::core::process_threaded_indexed(void)
{
	int32_t status = sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
	sirius::library::video::transform::codec::partial::png::compressor::core::buffer_t * iobuffer = nullptr;

	long long before_encode_timestamp = 0;
	long long after_encode_timestamp = 0;

	int32_t		simd_align = 32;
	int32_t		reference_buffer_size = (_context->width * _context->height) << 2;
	uint8_t *	reference_buffer = static_cast<uint8_t*>(_aligned_malloc(reference_buffer_size, simd_align));
	memset(reference_buffer, 0x00, reference_buffer_size);


	int32_t		nbytes_compressed = (_context->block_width*_context->block_height) << 2;
	int32_t		block_count = (_context->width / _context->block_width) * (_context->height / _context->block_height);


	uint8_t *** rows = static_cast<uint8_t***>(malloc(block_count * sizeof(uint8_t**)));

	int32_t *	pindex = new int32_t[block_count];
	uint8_t **	pcompressed = new uint8_t*[block_count];
	int32_t *	pcapacity = new int32_t[block_count];
	int32_t *	plength = new int32_t[block_count];
	int32_t *	cached_index = new int32_t[block_count];
	uint8_t **	cached_compressed = new uint8_t*[block_count];
	int32_t *	cached_capacity = new int32_t[block_count];
	int32_t *	cached_length = new int32_t[block_count];

	for (int32_t index = 0; index < block_count; index++)
	{
		pindex[index] = index;
		pcapacity[index] = nbytes_compressed;
		pcompressed[index] = new uint8_t[pcapacity[index]];
		plength[index] = 0;
		cached_index[index] = index;
		cached_capacity[index] = nbytes_compressed;
		cached_compressed[index] = new uint8_t[cached_capacity[index]];
		cached_length[index] = 0;

		rows[index] = static_cast<uint8_t**>(malloc(_context->block_height * sizeof(uint8_t*)));
	}

	int32_t		process_data_capacity = (_context->width * _context->height) << 2;
	int32_t		process_data_size = 0;
	uint8_t *	process_data = static_cast<uint8_t*>(_aligned_malloc(process_data_capacity, simd_align));
	int32_t		process_x = 0;
	int32_t		process_y = 0;
	int32_t		process_width = 0;
	int32_t		process_height = 0;
	long long	process_timestamp = 0;
	bool		process_force_fullmode = false;
	bool		process_compress_first_time = true;


	int32_t		thread_count = 20;
	indexed_thread_context_t ** thread_ctx = static_cast<indexed_thread_context_t**>(malloc(sizeof(indexed_thread_context_t*)*thread_count));
	for (int32_t tindex = 0; tindex < thread_count; tindex++)
	{
		uint32_t thrdaddr = 0;
		thread_ctx[tindex] = static_cast<indexed_thread_context_t*>(malloc(sizeof(indexed_thread_context_t)));
		thread_ctx[tindex]->index = 0;

		thread_ctx[tindex]->compressed_buffer_size = 1024 * 512;
		thread_ctx[tindex]->compressed_buffer = static_cast<char*>(malloc(thread_ctx[tindex]->compressed_buffer_size));

		thread_ctx[tindex]->output.data = thread_ctx[tindex]->compressed_buffer;
		thread_ctx[tindex]->output.data_capacity = thread_ctx[tindex]->compressed_buffer_size;

		thread_ctx[tindex]->pindex = nullptr;
		thread_ctx[tindex]->pcompressed = nullptr;
		thread_ctx[tindex]->pcapacity = nullptr;
		thread_ctx[tindex]->plength = nullptr;

		thread_ctx[tindex]->cached_index = nullptr;
		thread_ctx[tindex]->cached_compressed = nullptr;
		thread_ctx[tindex]->cached_capacity = nullptr;
		thread_ctx[tindex]->cached_length = nullptr;


		thread_ctx[tindex]->run = TRUE;
		thread_ctx[tindex]->thread = (HANDLE)::_beginthreadex(NULL, 0, sirius::library::video::transform::codec::partial::png::compressor::core::process_indexed_encoding_callback, thread_ctx[tindex], 0, &thrdaddr);
		thread_ctx[tindex]->signal = ::CreateEventA(NULL, FALSE, FALSE, NULL);
		thread_ctx[tindex]->available = ::CreateEventA(NULL, FALSE, FALSE, NULL);
		::SetEvent(thread_ctx[tindex]->available);
		thread_ctx[tindex]->parent = this;

		thread_ctx[tindex]->real_compressor = new sirius::library::video::transform::codec::libpng::compressor(_front);
		thread_ctx[tindex]->real_compressor->initialize(_context);
	}

	while (_run)
	{
		if (::WaitForSingleObject(_event, INFINITE) == WAIT_OBJECT_0)
		{
			_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressing;

			{
				sirius::autolock lock(&_cs);
				iobuffer = _iobuffer_queue.get_pending();
				if (!iobuffer)
				{
					if (!_invalidate)
					{
						_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressed;
						::Sleep(10);
						continue;
					}
					process_data_size = 0;
				}
				else
				{
					process_data_size = iobuffer->input.data_size;
					process_timestamp = iobuffer->input.timestamp;
					memmove(process_data, iobuffer->input.data, process_data_size);
					process_x = iobuffer->input.x;
					process_y = iobuffer->input.y;
					process_width = iobuffer->input.width;
					process_height = iobuffer->input.height;
					process_force_fullmode = iobuffer->input.force_fullmode;
				}
			}


			if (process_compress_first_time)
			{
				before_encode_timestamp = process_timestamp;
				int32_t count = 0;
				int32_t index = 0;
				std::vector<indexed_thread_context_t*> thread_contexts;
				for (int32_t h = 0; h < _context->height; h = h + _context->block_height)
				{
					for (int32_t w = 0; w < _context->width; w = w + _context->block_width)
					{
						for (int32_t eh = 0, row_index = 0; eh < _context->block_height; eh++, row_index++)
						{
							int32_t src_index = (h + eh) * (_context->width << 2) + (w << 2);
							rows[index][row_index] = process_data + src_index;
						}

						if (_front)
						{
							while (TRUE)
							{
								BOOL found_thread_ctx = FALSE;
								for (int32_t tindex = 0; tindex < thread_count; tindex++)
								{
									if (::WaitForSingleObject(thread_ctx[tindex]->available, 0) == WAIT_OBJECT_0)
									{
										thread_ctx[tindex]->input.data = rows[index];
										thread_ctx[tindex]->input.data_capacity = _context->block_height;
										thread_ctx[tindex]->input.data_size = _context->block_height;
										thread_ctx[tindex]->input.x = 0;
										thread_ctx[tindex]->input.y = 0;
										thread_ctx[tindex]->input.width = _context->block_width;
										thread_ctx[tindex]->input.height = _context->block_height;

										//thread_ctx[tindex]->output.data = thread_ctx[tindex]->compressed_buffer;
										//thread_ctx[tindex]->output.data_capacity = thread_ctx[tindex]->compressed_buffer_size;
										thread_ctx[tindex]->output.data_size = 0;
										thread_ctx[tindex]->output.x = thread_ctx[tindex]->input.x;
										thread_ctx[tindex]->output.y = thread_ctx[tindex]->input.y;
										thread_ctx[tindex]->output.width = thread_ctx[tindex]->input.width;
										thread_ctx[tindex]->output.height = thread_ctx[tindex]->input.height;


										thread_ctx[tindex]->index = index;

										thread_ctx[tindex]->pindex = &pindex[count];
										thread_ctx[tindex]->pcompressed = pcompressed[count];
										thread_ctx[tindex]->plength = &plength[count];

										thread_ctx[tindex]->cached_index = &cached_index[index];
										thread_ctx[tindex]->cached_length = &cached_length[index];
										thread_ctx[tindex]->cached_compressed = cached_compressed[index];

										std::vector<indexed_thread_context_t*>::iterator iter = std::find(thread_contexts.begin(), thread_contexts.end(), thread_ctx[tindex]);
										if (iter == thread_contexts.end())
											thread_contexts.push_back(thread_ctx[tindex]);
										
										count++;
										::SetEvent(thread_ctx[tindex]->signal);
										found_thread_ctx = TRUE;
									}
									if (found_thread_ctx)
										break;
								}
								if (found_thread_ctx)
									break;
							}
						}
						index++;
					}
				}

				std::vector<indexed_thread_context_t*>::iterator iter;
				for (iter = thread_contexts.begin(); iter != thread_contexts.end(); iter++)
					::WaitForSingleObject((*iter)->available, INFINITE);
				
				_front->after_process_callback(count, pindex, pcompressed, plength, before_encode_timestamp, after_encode_timestamp);

				for (iter = thread_contexts.begin(); iter != thread_contexts.end(); iter++)
					::SetEvent((*iter)->available);
				
				process_compress_first_time = false;

				continue;
			}

			if (_invalidate && _context->binvalidate && (process_data_size == 0))
			{
				_front->after_process_callback(block_count, cached_index, cached_compressed, cached_length, before_encode_timestamp, after_encode_timestamp);
				_invalidate = false;
			}
			else
			{
				while (process_data_size>0)
				{
					before_encode_timestamp = process_timestamp;
					int32_t count = 0;
					int32_t index = 0;
					int32_t begin_height = 0;
					int32_t end_height = 0;
					int32_t begin_width = 0;
					int32_t end_width = 0;
					int32_t resized_begin_height = 0;
					int32_t resized_end_height = 0;
					int32_t resized_begin_width = 0;
					int32_t resized_end_width = 0;
					if (process_force_fullmode)
					{
						begin_height = 0;
						end_height = _context->height;
						begin_width = 0;
						end_width = _context->width;
					}
					else
					{
						begin_height = (process_y / _context->block_height) * _context->block_height;
						end_height = ((process_y + process_height) / _context->block_height) * _context->block_height;

						if (((process_y + process_height) % _context->block_height) > 0)
							end_height += _context->block_height;

						begin_width = (process_x / _context->block_width) * _context->block_width;
						end_width = ((process_x + process_width) / _context->block_width) * _context->block_width;
						if (((process_x + process_width) % _context->block_width) > 0)
							end_width += _context->block_width;

					}

					std::vector<indexed_thread_context_t*> thread_contexts;
					for (int32_t h = begin_height; h < end_height; h = h + _context->block_height)
					{
						for (int32_t w = begin_width; w < end_width; w = w + _context->block_width)
						{
							bool diff = false;
							__declspec(align(32)) uint8_t simd_result[32] = { 0 };
							for (int32_t bh = 0; bh < _context->block_height; bh++)
							{
								for (int32_t bw = 0; bw < _context->block_width; bw = bw + (simd_align >> 2))
								{
									int32_t ci = (h + bh) * (_context->width << 2) + (w << 2) + (bw << 2);
									uint8_t * p = process_data + ci;
									uint8_t * r = reference_buffer + ci;
									const __m256i current = _mm256_load_si256((__m256i*)p);
									const __m256i reference = _mm256_load_si256((__m256i*)r);
									const __m256i cmpeq = _mm256_cmpeq_epi8(current, reference);
									_mm256_store_si256((__m256i*)simd_result, cmpeq);
									for (int32_t index = 0; index < simd_align; index++)
									{
										if (!simd_result[index])
										{
											diff = true;
											break;
										}
									}

									if (diff)
										break;
								}
								if (diff)
									break;
							}

							if (diff)
							{
								index = (h / _context->block_height) * (_context->width / _context->block_width) + w / _context->block_width;
								for (int32_t eh = 0, row_index = 0; eh < _context->block_height; eh++, row_index++)
								{
									int32_t src_index = (h + eh) * (_context->width << 2) + (w << 2);
									rows[index][row_index] = process_data + src_index;
									memmove(reference_buffer + +src_index, process_data + src_index, _context->block_width << 2);
								}

								if (_front)
								{
									while (TRUE)
									{
										BOOL found_thread_ctx = FALSE;
										for (int32_t tindex = 0; tindex < thread_count; tindex++)
										{
											if (::WaitForSingleObject(thread_ctx[tindex]->available, 0) == WAIT_OBJECT_0)
											{
												thread_ctx[tindex]->input.data = rows[index];
												thread_ctx[tindex]->input.data_capacity = _context->block_height;
												thread_ctx[tindex]->input.data_size = _context->block_height;
												thread_ctx[tindex]->input.x = 0;
												thread_ctx[tindex]->input.y = 0;
												thread_ctx[tindex]->input.width = _context->block_width;
												thread_ctx[tindex]->input.height = _context->block_height;

												//thread_ctx[tindex]->output.data = thread_ctx[tindex]->compressed_buffer;
												//thread_ctx[tindex]->output.data_capacity = thread_ctx[tindex]->compressed_buffer_size;

												thread_ctx[tindex]->output.data_size = 0;
												thread_ctx[tindex]->output.x = thread_ctx[tindex]->input.x;
												thread_ctx[tindex]->output.y = thread_ctx[tindex]->input.y;
												thread_ctx[tindex]->output.width = thread_ctx[tindex]->input.width;
												thread_ctx[tindex]->output.height = thread_ctx[tindex]->input.height;

												//index = (h / _context->block_height) * (_context->width / _context->block_width) + w / _context->block_width;

												thread_ctx[tindex]->index = index;

												thread_ctx[tindex]->pindex = &pindex[count];
												thread_ctx[tindex]->plength = &plength[count];
												thread_ctx[tindex]->pcompressed = pcompressed[count];

												thread_ctx[tindex]->cached_index = &cached_index[index];
												thread_ctx[tindex]->cached_length = &cached_length[index];
												thread_ctx[tindex]->cached_compressed = cached_compressed[index];

												std::vector<indexed_thread_context_t*>::iterator iter = std::find(thread_contexts.begin(), thread_contexts.end(), thread_ctx[tindex]);
												if (iter == thread_contexts.end())
												{
													thread_contexts.push_back(thread_ctx[tindex]);
												}

												count++;
												::SetEvent(thread_ctx[tindex]->signal);
												found_thread_ctx = TRUE;
											}
											if (found_thread_ctx)
												break;
										}
										if (found_thread_ctx)
											break;
									}
								}
							}
						}
					}
					std::vector<indexed_thread_context_t*>::iterator iter;
					for (iter = thread_contexts.begin(); iter != thread_contexts.end(); iter++)
						::WaitForSingleObject((*iter)->available, INFINITE);

					process_force_fullmode = false;
					if (_invalidate && _context->binvalidate)
					{
						_front->after_process_callback(block_count, cached_index, cached_compressed, cached_length, before_encode_timestamp, after_encode_timestamp);
						_invalidate = false;
					}
					else
					{
						if (count > 0)
						{
							_front->after_process_callback(count, pindex, pcompressed, plength, before_encode_timestamp, after_encode_timestamp);
						}
					}

					for (iter = thread_contexts.begin(); iter != thread_contexts.end(); iter++)
						::SetEvent((*iter)->available);
					
					thread_contexts.clear();
					{
						sirius::autolock lock(&_cs);
						iobuffer = _iobuffer_queue.get_pending();
						if (iobuffer)
						{
							process_data_size = iobuffer->input.data_size;
							process_timestamp = iobuffer->input.timestamp;
							memmove(process_data, iobuffer->input.data, process_data_size);
							process_x = iobuffer->input.x;
							process_y = iobuffer->input.y;
							process_width = iobuffer->input.width;
							process_height = iobuffer->input.height;
							process_force_fullmode = iobuffer->input.force_fullmode;
							continue;
						}
						else
						{
							process_data_size = 0;
							break;
						}
					}
				}
			}
			_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressed;
		}
	}


	for (int32_t tindex = 0; tindex < thread_count; tindex++)
	{
		thread_ctx[tindex]->run = FALSE;
		if (::WaitForSingleObject(thread_ctx[tindex]->available, INFINITE) == WAIT_OBJECT_0)
		{
			::SetEvent(thread_ctx[tindex]->signal);

			if (::WaitForSingleObject(thread_ctx[tindex]->thread, INFINITE) == WAIT_OBJECT_0)
			{
				::CloseHandle(thread_ctx[tindex]->thread);
				thread_ctx[tindex]->thread = INVALID_HANDLE_VALUE;

				::CloseHandle(thread_ctx[tindex]->available);
				::CloseHandle(thread_ctx[tindex]->signal);
				thread_ctx[tindex]->available = INVALID_HANDLE_VALUE;
				thread_ctx[tindex]->signal = INVALID_HANDLE_VALUE;
			}
		}
		thread_ctx[tindex]->real_compressor->release();
		delete thread_ctx[tindex]->real_compressor;
		thread_ctx[tindex]->real_compressor = nullptr;

		free(thread_ctx[tindex]->compressed_buffer);
		thread_ctx[tindex]->compressed_buffer = nullptr;

		thread_ctx[tindex]->pindex = nullptr;
		thread_ctx[tindex]->pcompressed = nullptr;
		thread_ctx[tindex]->pcapacity = nullptr;
		thread_ctx[tindex]->plength = nullptr;

		thread_ctx[tindex]->cached_index = nullptr;
		thread_ctx[tindex]->cached_compressed = nullptr;
		thread_ctx[tindex]->cached_capacity = nullptr;
		thread_ctx[tindex]->cached_length = nullptr;

		free(thread_ctx[tindex]);
		thread_ctx[tindex] = nullptr;
	}
	free(thread_ctx);
	thread_ctx = nullptr;

	if (process_data)
		_aligned_free(process_data);
	process_data = nullptr;

	if (pindex)
		delete[] pindex;
	if (plength)
		delete[] plength;
	if (pcapacity)
		delete[] pcapacity;
	if (pcompressed)
	{
		for (int32_t index = 0; index < block_count; index++)
		{
			delete[] pcompressed[index];
		}
		delete[] pcompressed;
	}
		

	if (cached_index)
		delete[] cached_index;
	if (cached_length)
		delete[] cached_length;
	if (cached_capacity)
		delete[] cached_capacity;
	if (cached_compressed)
	{
		for (int32_t index = 0; index < block_count; index++)
		{
			delete[] cached_compressed[index];
		}
		delete[] cached_compressed;
	}

	for (int32_t index = 0; index < block_count; index++)
	{
		if(rows[index])
			free(rows[index]);
		rows[index] = nullptr;
	}
	if (rows)
		free(rows);
	rows = nullptr;

	if (reference_buffer)
		_aligned_free(reference_buffer);
	reference_buffer = nullptr;
	reference_buffer_size = 0;
}

/*
void sirius::library::video::transform::codec::partial::png::compressor::core::process_coordinates(void)
{
	int32_t status = sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
	sirius::library::video::transform::codec::partial::png::compressor::core::buffer_t * iobuffer = nullptr;

	long long before_encode_timestamp = 0;
	long long after_encode_timestamp = 0;

	int32_t		block_count = (_context->width / _context->block_width) * (_context->height / _context->block_height);
	int32_t		compressed_buffer_size = 1024 * 1024 * 2;
	uint8_t *	compressed_buffer = static_cast<uint8_t*>(malloc(compressed_buffer_size));

	int32_t		reference_buffer_size = (_context->width * _context->height) << 2;
	uint8_t *	reference_buffer = static_cast<uint8_t*>(malloc(reference_buffer_size));
	memset(reference_buffer, 0x00, reference_buffer_size);

	int32_t		block_buffer_size = (_context->block_width * _context->block_height) << 2;
	uint8_t *	block_buffer = static_cast<uint8_t*>(malloc(block_buffer_size));


	int32_t		macro_block_count = 0;
	int16_t *	px = nullptr;
	int16_t *	py = nullptr;
	int16_t *	pwidth = nullptr;
	int16_t *	pheight = nullptr;
	uint8_t **	pcompressed = nullptr;
	int32_t *	plength = nullptr;
	if (_context->partial_post)
	{
		macro_block_count = (_context->block_width * _context->block_height) / (MACRO_BLOCK_WIDTH * MACRO_BLOCK_HEIGHT);
		px = new int16_t[macro_block_count];
		py = new int16_t[macro_block_count];
		pwidth = new int16_t[macro_block_count];
		pheight = new int16_t[macro_block_count];
		pcompressed = new uint8_t*[macro_block_count];
		plength = new int32_t[macro_block_count];
	}
	else
	{
		macro_block_count = (_context->width * _context->height) / (MACRO_BLOCK_WIDTH * MACRO_BLOCK_HEIGHT);
		px = new int16_t[macro_block_count];
		py = new int16_t[macro_block_count];
		pwidth = new int16_t[macro_block_count];
		pheight = new int16_t[macro_block_count];
		pcompressed = new uint8_t*[macro_block_count];
		plength = new int32_t[macro_block_count];
	}

	int32_t		process_data_capacity = (_context->width * _context->height) << 2;
	int32_t		process_data_size = 0;
	uint8_t *	process_data = (uint8_t*)malloc(process_data_capacity);
	int32_t		process_x = 0;
	int32_t		process_y = 0;
	int32_t		process_width = 0;
	int32_t		process_height = 0;
	long long	process_timestamp = 0;

	while (_run)
	{
		if (::WaitForSingleObject(_event, INFINITE) == WAIT_OBJECT_0)
		{
			_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressing;

			{
				sirius::autolock lock(&_cs);
				iobuffer = _iobuffer_queue.get_pending();
				if (!iobuffer)
				{
					if (!_invalidate)
					{
						_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressed;
						continue;
					}
					process_data_size = 0;
				}
				else
				{
					process_data_size = iobuffer->input.data_size;
					process_timestamp = iobuffer->input.timestamp;
					memmove(process_data, iobuffer->input.data, process_data_size);
					process_x = iobuffer->input.x;
					process_y = iobuffer->input.y;
					process_width = iobuffer->input.width;
					process_height = iobuffer->input.height;
				}
			}

			if (_invalidate)
			{
				//_front->after_process_callback(block_count, cached_index, cached_compressed, cached_length, before_encode_timestamp, after_encode_timestamp);
				_invalidate = false;
			}
			else
			{

#if defined(WITH_DEBUG_PNG)
				int32_t png_index = 0;
#endif
				while (process_data_size>0)
				{
					before_encode_timestamp = process_timestamp;
					int32_t		count = 0;
					uint8_t *	real_compressed_buffer = compressed_buffer;

					int32_t begin_height = (process_y / _context->block_height) * _context->block_height;
					int32_t end_height = ((process_y + process_height) / _context->block_height) * _context->block_height;
					if (((process_y + process_height) % _context->block_height) > 0)
						end_height += _context->block_height;

					int32_t begin_width = (process_x / _context->block_width) * _context->block_width;
					int32_t end_width = ((process_x + process_width) / _context->block_width) * _context->block_width;
					if (((process_x + process_width) % _context->block_width) > 0)
						end_width += _context->block_width;

					for (int32_t h = begin_height; h < end_height; h = h + _context->block_height)
					{
						for (int32_t w = begin_width; w < end_width; w = w + _context->block_width)
						{
							std::vector<sirius::library::video::transform::codec::partial::png::compressor::core::bounding_box_t*> bounding_boxes;
							int32_t macro_block_index = 0;
							for (int32_t bh = 0; bh < _context->block_height; bh = bh + MACRO_BLOCK_HEIGHT)
							{
								for (int32_t bw = 0; bw < _context->block_width; bw = bw + MACRO_BLOCK_WIDTH)
								{
									bool diff = false;
									for (int32_t mh = 0; mh < MACRO_BLOCK_HEIGHT; mh++)
									{
										int32_t index = (h + bh + mh) * (_context->width << 2) + (w << 2) + (bw << 2);
										if (!diff)
										{
											for (int32_t mw = 0; mw < MACRO_BLOCK_WIDTH; mw = mw + 4)
											{
												int32_t bi = index + mw + 0;
												int32_t gi = index + mw + 1;
												int32_t ri = index + mw + 2;
												int32_t ai = index + mw + 3;

												int32_t bdiff = labs(*(process_data + bi) - *(reference_buffer + bi));
												int32_t gdiff = labs(*(process_data + gi) - *(reference_buffer + gi));
												int32_t rdiff = labs(*(process_data + ri) - *(reference_buffer + ri));
												int32_t adiff = labs(*(process_data + ai) - *(reference_buffer + ai));

												if ((bdiff + gdiff + rdiff + adiff) > 0)
												{
													diff = true;
													break;
												}
											}
										}
										//copy to reference 
										memmove(reference_buffer + index, process_data + index, MACRO_BLOCK_WIDTH << 2);
									}

									//32 x 18
									if (diff) // 4 x 4 block is match block(difference)
									{
										if (bounding_boxes.size() == 0)
										{
											bounding_box_t * bb = create_bounding_box(macro_block_index);
											bounding_boxes.push_back(bb);
										}
										else
										{
											bool hmatched = false;
											bool vmatched = false;
											bounding_box_t * bb = nullptr;
											std::vector<bounding_box_t*>::iterator iter;
											for (iter = bounding_boxes.begin(); iter != bounding_boxes.end(); iter++)
											{
												bb = *iter;
												if (((bb->bindex / (_context->block_width / MACRO_BLOCK_WIDTH)) == (macro_block_index / (_context->block_width / MACRO_BLOCK_WIDTH))) &&
													(bb->mb[macro_block_index - 1])) //horizontal matching
												{
													hmatched = true;
													break;
												}

												//if (bb->mb[macro_block_index - MACRO_BLOCK_WIDTH])	//vertical matching
												//{
												//	vmatched = true;
												//	break;
												//}
											}

											if (hmatched && vmatched)
											{

											}

											else if (hmatched)
											{
												bb->mb[macro_block_index] = true;
												bb->eindex = macro_block_index;
												bb->hcnt++;
												bb->cnt++;
												hmatched = false;
											}
											else if (vmatched)
											{
												if ((bb->bindex % (_context->block_width / MACRO_BLOCK_WIDTH)) == (macro_block_index % (_context->block_width / MACRO_BLOCK_WIDTH))) //begin index and macro_block_index has same column index
												{
													bb->mb[macro_block_index] = true;
													bb->eindex = macro_block_index;
													bb->vcnt++;
													bb->cnt++;
												}
												else
												{
													bounding_box_t * bb = create_bounding_box(macro_block_index - (_context->block_width / MACRO_BLOCK_WIDTH)); //create with previous vertical macro block index
													bb->mb[macro_block_index] = true;
													bb->eindex = macro_block_index;
													bb->vcnt++;
													bb->cnt++;
													bounding_boxes.push_back(bb);
												}
											}
											else
											{
												bounding_box_t * bb = create_bounding_box(macro_block_index); //create with previous vertical macro block index
												bounding_boxes.push_back(bb);
											}
										}
									}
									macro_block_index++;
								}
							}

							std::vector<bounding_box_t*>::iterator iter;
							for (iter = bounding_boxes.begin(); iter != bounding_boxes.end(); iter++)
							{
								bounding_box_t * bb = (*iter);
								int16_t bx = bb->bindex % (_context->block_width / MACRO_BLOCK_WIDTH);
								int16_t by = bb->bindex / (_context->block_width / MACRO_BLOCK_WIDTH);
								int16_t ex = bb->eindex % (_context->block_width / MACRO_BLOCK_WIDTH);
								int16_t ey = bb->eindex / (_context->block_width / MACRO_BLOCK_WIDTH);

								by = by * MACRO_BLOCK_HEIGHT;
								ey = ey * MACRO_BLOCK_HEIGHT + MACRO_BLOCK_HEIGHT;

								bx = bx * MACRO_BLOCK_WIDTH;
								ex = ex * MACRO_BLOCK_WIDTH + MACRO_BLOCK_WIDTH;

								int32_t cstride = (ex - bx) << 2;
								uint8_t ** rows = static_cast<uint8_t**>(malloc((ey - by) * sizeof(uint8_t*)));
								for (int32_t bh = by, row_index = 0; bh < ey; bh++, row_index++)
								{
									int32_t sindex = (h + bh) * (_context->width << 2) + (w << 2) + (bx << 2);
									int32_t dindex = bh * (_context->block_width << 2) + (bx << 2);

									memmove(block_buffer + dindex, process_data + sindex, cstride);
									rows[row_index] = block_buffer + dindex;
								}

								sirius::library::video::transform::codec::partial::png::compressor::entity_t input;
								input.data = rows;
								input.data_capacity = ey - by;
								input.data_size = ey - by;
								input.x = w + bx;
								input.y = h + by;
								input.width = (w + ex) - input.x;
								input.height = (h + ey) - input.y;

								sirius::library::video::transform::codec::partial::png::compressor::entity_t bitstream;
								bitstream.data = real_compressed_buffer;
								bitstream.data_capacity = real_compressed_buffer - compressed_buffer;
								bitstream.data_capacity = compressed_buffer_size - bitstream.data_capacity;
								bitstream.data_size = 0;
								bitstream.x = input.x;
								bitstream.y = input.y;
								bitstream.width = input.width;
								bitstream.height = input.height;

								status = _real_compressor->compress(&input, &bitstream);
								if (status == sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success)
								{
#if defined(WITH_DEBUG_PNG)
									char filename[MAX_PATH] = { 0 };
									_snprintf_s(filename, MAX_PATH, "attnt_index_%d__x_%d__y_%d__width_%d__height_%d.png", png_index, bitstream.x, bitstream.y, bitstream.width, bitstream.height);
									HANDLE hfile = create_file(filename);
									uint32_t written = 0;
									write_file(hfile, bitstream.data, bitstream.data_size, &written, NULL);
									png_index++;
									close_file(hfile);
#endif
									px[count] = bitstream.x;
									py[count] = bitstream.y;
									pwidth[count] = bitstream.width;
									pheight[count] = bitstream.height;
									pcompressed[count] = (uint8_t*)bitstream.data;
									plength[count] = bitstream.data_size;

									real_compressed_buffer += bitstream.data_size;
									count++;
								}

								free(rows);
								rows = nullptr;

								destroy_bounding_box(bb);
							}
							bounding_boxes.clear();

							if (_context->partial_post && count>0)
							{
								_front->after_process_callback(count, px, py, pwidth, pheight, pcompressed, plength, before_encode_timestamp, after_encode_timestamp);
								count = 0;
							}
						}
					}

					if (!_context->partial_post && count > 0)
					{
						_front->after_process_callback(count, px, py, pwidth, pheight, pcompressed, plength, before_encode_timestamp, after_encode_timestamp);
					}

					{
						sirius::autolock lock(&_cs);
						iobuffer = _iobuffer_queue.get_pending();
						if (iobuffer)
						{
							process_data_size = iobuffer->input.data_size;
							process_timestamp = iobuffer->input.timestamp;
							memmove(process_data, iobuffer->input.data, process_data_size);
							process_x = iobuffer->input.x;
							process_y = iobuffer->input.y;
							process_width = iobuffer->input.width;
							process_height = iobuffer->input.height;
						}
						else
						{
							process_data_size = 0;
						}
					}
					continue;
				}
			}
			_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressed;
		}
	}

	if (process_data)
		free(process_data);
	process_data = nullptr;

	if (px)
		delete[] px;
	if (py)
		delete[] py;
	if (pwidth)
		delete[] pwidth;
	if (pheight)
		delete[] pheight;
	if (plength)
		delete[] plength;
	if (pcompressed)
		delete[] pcompressed;

	if (block_buffer)
		free(block_buffer);
	block_buffer = nullptr;
	block_buffer_size = 0;

	if (reference_buffer)
		free(reference_buffer);
	reference_buffer = nullptr;
	reference_buffer_size = 0;

	if (compressed_buffer)
		free(compressed_buffer);
	compressed_buffer = nullptr;
	compressed_buffer_size = 0;
}

void sirius::library::video::transform::codec::partial::png::compressor::core::process_bsend_indexed(void)
{
	int32_t status = sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
	sirius::library::video::transform::codec::partial::png::compressor::core::buffer_t * iobuffer = nullptr;


	long long before_encode_timestamp = 0;
	long long after_encode_timestamp = 0;

	int32_t	compressed_buffer_size	= 1024 * 1024 * 2;
	uint8_t * compressed_buffer		= static_cast<uint8_t*>(malloc(compressed_buffer_size));
	memset(compressed_buffer, 0x00, compressed_buffer_size);

#if defined(WITH_AVX2_SIMD)
	int32_t		gray_buffer_size = ((_context->width) * (_context->height));
	uint8_t *	gray_buffer = static_cast<uint8_t*>(malloc(gray_buffer_size));
	memset(gray_buffer, 0x00, gray_buffer_size);

	int32_t		resized_buffer_size = ((_context->width >> 2) * (_context->height >> 2));
	uint8_t *	resized_buffer = static_cast<uint8_t*>(malloc(resized_buffer_size));
	memset(resized_buffer, 0x00, resized_buffer_size);

	uint8_t *	reference_buffer = static_cast<uint8_t*>(malloc(resized_buffer_size));
	memset(reference_buffer, 0x00, resized_buffer_size);
#else
#if defined(WITH_AVX2_BILINEAR_RESIZE)
#if defined(WITH_DOUBLE_DOWN_SIZE)
	int32_t		resized_buffer_size = ((_context->width >> 2) * (_context->height >> 2)) << 2;
	uint8_t *	resized_buffer = static_cast<uint8_t*>(malloc(resized_buffer_size));
	memset(resized_buffer, 0x00, resized_buffer_size);

	uint8_t *	reference_buffer = static_cast<uint8_t*>(malloc(resized_buffer_size));
	memset(reference_buffer, 0x00, resized_buffer_size);
#else
	int32_t		resized_buffer_size = ((_context->width >> 3) * (_context->height >> 3)) << 2;
	uint8_t *	resized_buffer = static_cast<uint8_t*>(malloc(resized_buffer_size));
	memset(resized_buffer, 0x00, resized_buffer_size);

	uint8_t *	reference_buffer = static_cast<uint8_t*>(malloc(resized_buffer_size));
	memset(reference_buffer, 0x00, resized_buffer_size);
#endif
#else
#if defined(WITH_AVX2)
	int32_t		simd_align = 32;
#else
	int32_t		simd_align = 16;
#endif

	int32_t		reference_buffer_size = (_context->width * _context->height) << 2;
	uint8_t *	reference_buffer = static_cast<uint8_t*>(_aligned_malloc(reference_buffer_size, simd_align));
	memset(reference_buffer, 0x00, reference_buffer_size);
#endif
#endif
	uint8_t ** rows = static_cast<uint8_t**>(malloc(_context->block_height * sizeof(uint8_t*)));

	int32_t		block_count = (_context->width / _context->block_width) * (_context->height / _context->block_height);
	int32_t *	pindex = new int32_t[block_count];
	uint8_t **	pcompressed = new uint8_t*[block_count];
	int32_t *	plength = new int32_t[block_count];

	int32_t		nbytes_compressed = (_context->block_width*_context->block_height) << 2;
	int32_t *	cached_index = new int32_t[block_count];
	uint8_t **	cached_compressed = new uint8_t*[block_count];
	int32_t *	cached_capacity = new int32_t[block_count];
	int32_t *	cached_length = new int32_t[block_count];
	for (int32_t index = 0; index < block_count; index++)
	{
		cached_index[index] = index;
		cached_capacity[index] = nbytes_compressed;
		cached_compressed[index] = new uint8_t[cached_capacity[index]];
		cached_length[index] = 0;
	}

	int32_t		process_data_capacity = (_context->width * _context->height) << 2;
	int32_t		process_data_size = 0;
	uint8_t *	process_data = static_cast<uint8_t*>(_aligned_malloc(process_data_capacity, simd_align));
	int32_t		process_x = 0;
	int32_t		process_y = 0;
	int32_t		process_width = 0;
	int32_t		process_height = 0;
	long long	process_timestamp = 0;
	bool		process_force_fullmode = false;
	bool		process_compress_first_time = true;
	while (_run)
	{
		if (::WaitForSingleObject(_event, INFINITE) == WAIT_OBJECT_0)
		{
			_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressing;

			{
				sirius::autolock lock(&_cs);
				iobuffer = _iobuffer_queue.get_pending();
				if (!iobuffer)
				{
					if (!_invalidate)
					{
						_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressed;
						::Sleep(10);
						continue;
					}
					process_data_size = 0;
				}
				else
				{
					process_data_size = iobuffer->input.data_size;
					process_timestamp = iobuffer->input.timestamp;
					memmove(process_data, iobuffer->input.data, process_data_size);
					process_x = iobuffer->input.x;
					process_y = iobuffer->input.y;
					process_width = iobuffer->input.width;
					process_height = iobuffer->input.height;
					process_force_fullmode = iobuffer->input.force_fullmode;
				}
			}

			if (process_compress_first_time)
			{
				before_encode_timestamp = process_timestamp;
				int32_t count = 0;
				int32_t index = 0;
				uint8_t * real_compressed_buffer = compressed_buffer;

				for (int32_t h = 0; h < _context->height; h = h + _context->block_height)
				{
					for (int32_t w = 0; w < _context->width; w = w + _context->block_width)
					{
						for (int32_t eh = 0, row_index = 0; eh < _context->block_height; eh++, row_index++)
						{
							int32_t src_index = (h + eh) * (_context->width << 2) + (w << 2);
							rows[row_index] = process_data + src_index;
						}

						if (_front)
						{
							sirius::library::video::transform::codec::partial::png::compressor::entity_t input;
							input.data = rows;
							input.data_capacity = _context->block_height;
							input.data_size = _context->block_height;
							input.x = 0;
							input.y = 0;
							input.width = _context->block_width;
							input.height = _context->block_height;

							sirius::library::video::transform::codec::partial::png::compressor::entity_t bitstream;
							bitstream.data = real_compressed_buffer;
							bitstream.data_capacity = real_compressed_buffer - compressed_buffer;
							bitstream.data_capacity = compressed_buffer_size - bitstream.data_capacity;
							bitstream.data_size = 0;
							bitstream.x = input.x;
							bitstream.y = input.y;
							bitstream.width = input.width;
							bitstream.height = input.height;

							status = _real_compressor->compress(&input, &bitstream);
							if (status == sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success)
							{
								pindex[count] = index;
								pcompressed[count] = (uint8_t*)bitstream.data;
								plength[count] = bitstream.data_size;
								real_compressed_buffer += bitstream.data_size;

								cached_index[index] = index;
								cached_length[index] = bitstream.data_size;
								memmove(cached_compressed[index], pcompressed[count], cached_length[index]);

								count++;
							}
						}
						index++;
					}
				}
				_front->after_process_callback(count, pindex, pcompressed, plength, before_encode_timestamp, after_encode_timestamp);

				process_compress_first_time = false;
				continue;
			}

			if (_invalidate && _context->binvalidate && (process_data_size == 0))
			{
				_front->after_process_callback(block_count, cached_index, cached_compressed, cached_length, before_encode_timestamp, after_encode_timestamp);
				_invalidate = false;
			}
			else
			{
				while (process_data_size>0)
				{
					before_encode_timestamp = process_timestamp;
					int32_t count = 0;
					int32_t index = 0;
					uint8_t * real_compressed_buffer = compressed_buffer;

					int32_t begin_height		= 0;
					int32_t end_height			= 0;
					int32_t begin_width			= 0;
					int32_t end_width			= 0;
					int32_t resized_begin_height	= 0;
					int32_t resized_end_height	= 0;
					int32_t resized_begin_width	= 0;
					int32_t resized_end_width		= 0;
					if (process_force_fullmode)
					{
						begin_height = 0;
						end_height = _context->height;
						begin_width = 0;
						end_width = _context->width;

#if defined(WITH_AVX2_BILINEAR_RESIZE)
						resized_begin_height = begin_height >> 2;
						resized_end_height = end_height >> 2;
						resized_begin_width = begin_width >> 2;
						resized_end_width = end_width >> 2;
#endif
					}
					else
					{
						begin_height = (process_y / _context->block_height) * _context->block_height;
						end_height = ((process_y + process_height) / _context->block_height) * _context->block_height;

						if (((process_y + process_height) % _context->block_height) > 0)
							end_height += _context->block_height;

						begin_width = (process_x / _context->block_width) * _context->block_width;
						end_width = ((process_x + process_width) / _context->block_width) * _context->block_width;
						if (((process_x + process_width) % _context->block_width) > 0)
							end_width += _context->block_width;

#if defined(WITH_AVX2_BILINEAR_RESIZE)
#if defined(WITH_DOUBLE_DOWN_SIZE)
						resized_begin_height = begin_height >> 2;// ((process_y >> 2) / (_context->block_height >> 2)) * (_context->block_height >> 2);
						resized_end_height = end_height >> 2; //(((process_y + process_height) >> 2) / (_context->block_height >> 2)) * (_context->block_height >> 2);

						if ((((process_y + process_height) >> 2) % (_context->block_height >> 2)) > 0)
							resized_end_height += (_context->block_height >> 2);

						resized_begin_width = begin_width >> 2;// ((process_x >> 2) / (_context->block_width >> 2)) * (_context->block_width >> 2);
						resized_end_width = end_width >> 2;// (((process_x + process_width) >> 2) / (_context->block_width >> 2)) * (_context->block_width >> 2);

						if ((((process_x + process_width) >> 2) % (_context->block_width >> 2)) > 0)
							resized_end_width += (_context->block_width >> 2);
#else
						resized_begin_height = begin_height >> 3;// ((process_y >> 2) / (_context->block_height >> 2)) * (_context->block_height >> 2);
						resized_end_height = end_height >> 3; //(((process_y + process_height) >> 2) / (_context->block_height >> 2)) * (_context->block_height >> 2);

						if ((((process_y + process_height) >> 3) % (_context->block_height >> 3)) > 0)
							resized_end_height += (_context->block_height >> 3);

						resized_begin_width = begin_width >> 3;// ((process_x >> 2) / (_context->block_width >> 2)) * (_context->block_width >> 2);
						resized_end_width = end_width >> 3;// (((process_x + process_width) >> 2) / (_context->block_width >> 2)) * (_context->block_width >> 2);

						if ((((process_x + process_width) >> 3) % (_context->block_width >> 3)) > 0)
							resized_end_width += (_context->block_width >> 3);

#endif
#endif
					}
#if defined(WITH_AVX2_SIMD)
					int32_t process_index = (begin_height) * (_context->width << 2) + (begin_width << 2);
					int32_t gray_index = (begin_height) * (_context->width) + (begin_width);
					int32_t resize_index = (begin_height >> 2) * (_context->width >> 2) + (begin_width >> 2);
					SimdBgraToGray(process_data + process_index, end_width - begin_width, end_height - begin_height, _context->width << 2, gray_buffer + gray_index, end_width - begin_width);
					SimdResizeBilinear(gray_buffer + gray_index, end_width - begin_width, end_height - begin_height, _context->width, resized_buffer + resize_index, (end_width - begin_width) >> 2, (end_height - begin_height) >> 2, _context->width >> 2, 1);
					for (int32_t h = 0, h2 = 0; h < _context->height; h = h + _context->block_height, h2 = h2 + (_context->block_height >> 2))
					{
						for (int32_t w = 0, w2 = 0; w < _context->width; w = w + _context->block_width, w2 = w2 + (_context->block_width >> 2))
						{
#else
#if defined(WITH_AVX2_BILINEAR_RESIZE)
#if defined(WITH_DOUBLE_DOWN_SIZE)
					//int32_t process_index = 0;	//	(begin_height) * (_context->width << 2) + (begin_width << 2);
					//int32_t resize_index = 0;	// (begin_height >> 2)* (_context->width) + begin_width;
					//SIMD_RESIZER::resize_bilinear(process_data + process_index, end_width - begin_width, end_height - begin_height, _context->width << 2, resized_buffer, (end_width - begin_width) >> 2, (end_height - begin_height) >> 2, _context->width);
					SIMD_RESIZER::resize_bilinear(process_data, _context->width, _context->height, _context->width << 2, resized_buffer, (_context->width) >> 2, (_context->height) >> 2, _context->width);
					for (int32_t h = begin_height, h2 = resized_begin_height; h < end_height; h = h + _context->block_height, h2 = h2 + (_context->block_height >> 2))
					{
						for (int32_t w = begin_width, w2 = resized_begin_width; w < end_width; w = w + _context->block_width, w2 = w2 + (_context->block_width >> 2))
						{
#else
					//int32_t process_index = (begin_height) * (_context->width << 2) + (begin_width << 2);
					//int32_t resize_index = (begin_height >> 3) * ((_context->width >> 3) << 2) + ((begin_width >> 3) << 2);
					//SIMD_RESIZER::resize_bilinear(process_data + process_index, end_width - begin_width, end_height - begin_height, _context->width << 2, resized_buffer, (end_width - begin_width) >> 3, (end_height - begin_height) >> 3, (_context->width >> 3) << 2);
					SIMD_RESIZER::resize_bilinear(process_data, _context->width, _context->height, _context->width << 2, resized_buffer, _context->width >> 3, _context->height >> 3, (_context->width >> 3) << 2);
					for (int32_t h = begin_height, h2 = resized_begin_height; h < end_height; h = h + _context->block_height, h2 = h2 + (_context->block_height >> 3))
					{
						for (int32_t w = begin_width, w2 = resized_begin_width; w < end_width; w = w + _context->block_width, w2 = w2 + (_context->block_width >> 3))
						{
#endif
#else
					for (int32_t h = begin_height; h < end_height; h = h + _context->block_height)
					{
						for (int32_t w = begin_width; w < end_width; w = w + _context->block_width)
						{
#endif
#endif
							//if (h >= begin_height && h < end_height && w >= begin_width && w < end_width)
							{
#if defined(WITH_AVX2_SIMD)
								uint64_t sum = 0;
								SimdSquaredDifferenceSum(resized_buffer + (h2 * (_context->width >> 2) + w2), _context->width >> 2, reference_buffer + (h2 * (_context->width >> 2) + w2), _context->width >> 2, _context->block_width >> 2, _context->block_height >> 2, &sum);
								if (sum>0)
#else
#if defined(WITH_AVX2_BILINEAR_RESIZE)
								bool diff = false;
#if defined(WITH_DOUBLE_DOWN_SIZE)
								for (int32_t bh = 0; bh < (_context->block_height >> 2); bh++)
								{
									for (int32_t bw = 0; bw < (_context->block_width >> 2); bw++)
									{
										int32_t ci = (h2 + bh) * (_context->width) + (w2 << 2) + (bw << 2);
#else
								for (int32_t bh = 0; bh < (_context->block_height >> 3); bh++)
								{
									for (int32_t bw = 0; bw < (_context->block_width >> 3); bw++)
									{
										int32_t ci = (h2 + bh) * ((_context->width >> 3) << 2) + (w2 << 2) + (bw << 2);
#endif
										int32_t bi = ci + 0;
										int32_t gi = ci + 1;
										int32_t ri = ci + 2;
										int32_t ai = ci + 3;
										int32_t bdiff = labs(*(resized_buffer + bi) - *(reference_buffer + bi));
										int32_t gdiff = labs(*(resized_buffer + gi) - *(reference_buffer + gi));
										int32_t rdiff = labs(*(resized_buffer + ri) - *(reference_buffer + ri));
										int32_t adiff = labs(*(resized_buffer + ai) - *(reference_buffer + ai));
										if ((bdiff + gdiff + rdiff + adiff) > 0)
										{
											diff = true;
											break;
										}
									}
								}

								if (diff)
#else
								bool	diff = false;
#if defined(WITH_AVX2)
								__declspec(align(32)) uint8_t simd_result[32] = { 0 };
#else
								__declspec(align(16)) uint8_t simd_result[16] = { 0 };
#endif
								for (int32_t bh = 0; bh < _context->block_height; bh++)
								{
									for (int32_t bw = 0; bw < _context->block_width; bw = bw + (simd_align >> 2))
									{
										int32_t ci = (h + bh) * (_context->width << 2) + (w << 2) + (bw << 2);
										uint8_t * p = process_data + ci;
										uint8_t * r = reference_buffer + ci;
#if 0
										int32_t bi0 = ci + 0;
										int32_t gi0 = ci + 1;
										int32_t ri0 = ci + 2;
										int32_t ai0 = ci + 3;

										int32_t bi1 = ci + 4;
										int32_t gi1 = ci + 5;
										int32_t ri1 = ci + 6;
										int32_t ai1 = ci + 7;

										int32_t bi2 = ci + 8;
										int32_t gi2 = ci + 9;
										int32_t ri2 = ci + 10;
										int32_t ai2 = ci + 11;

										int32_t bi3 = ci + 12;
										int32_t gi3 = ci + 13;
										int32_t ri3 = ci + 14;
										int32_t ai3 = ci + 15;

										int32_t bdiff0 = labs(*(process_data + bi0) - *(reference_buffer + bi0));
										int32_t gdiff0 = labs(*(process_data + gi0) - *(reference_buffer + gi0));
										int32_t rdiff0 = labs(*(process_data + ri0) - *(reference_buffer + ri0));
										int32_t adiff0 = labs(*(process_data + ai0) - *(reference_buffer + ai0));

										int32_t bdiff1 = labs(*(process_data + bi1) - *(reference_buffer + bi1));
										int32_t gdiff1 = labs(*(process_data + gi1) - *(reference_buffer + gi1));
										int32_t rdiff1 = labs(*(process_data + ri1) - *(reference_buffer + ri1));
										int32_t adiff1 = labs(*(process_data + ai1) - *(reference_buffer + ai1));

										int32_t bdiff2 = labs(*(process_data + bi2) - *(reference_buffer + bi2));
										int32_t gdiff2 = labs(*(process_data + gi2) - *(reference_buffer + gi2));
										int32_t rdiff2 = labs(*(process_data + ri2) - *(reference_buffer + ri2));
										int32_t adiff2 = labs(*(process_data + ai2) - *(reference_buffer + ai2));

										int32_t bdiff3 = labs(*(process_data + bi3) - *(reference_buffer + bi3));
										int32_t gdiff3 = labs(*(process_data + gi3) - *(reference_buffer + gi3));
										int32_t rdiff3 = labs(*(process_data + ri3) - *(reference_buffer + ri3));
										int32_t adiff3 = labs(*(process_data + ai3) - *(reference_buffer + ai3));


										int32_t diff0 = bdiff0 + gdiff0 + rdiff0 + adiff0;
										int32_t diff1 = bdiff1 + gdiff1 + rdiff1 + adiff1;
										int32_t diff2 = bdiff2 + gdiff2 + rdiff2 + adiff2;
										int32_t diff3 = bdiff3 + gdiff3 + rdiff3 + adiff3;

										if ((diff0 + diff1 + diff2 + diff3) > 0)
										{
											diff = true;
											break;
										}
#else
#if defined(WITH_AVX2)
										const __m256i current	= _mm256_load_si256((__m256i*)p);
										const __m256i reference = _mm256_load_si256((__m256i*)r);
										const __m256i cmpeq		= _mm256_cmpeq_epi8(current, reference);
										_mm256_store_si256((__m256i*)simd_result, cmpeq);
#else
										__asm
										{
											pushad
											mov			eax, p
											mov			ebx, r
											mov			esi, 0
											movdqa		xmm0, [eax + esi]
											movdqa		xmm1, [ebx + esi]
											pcmpeqb		xmm0, xmm1
											movdqa		simd_result, xmm0
											popad
										}
#endif
										for (int32_t index = 0; index < simd_align; index++)
										{
											if (!simd_result[index])
											{
												diff = true;
												break;
											}
										}

										if (diff)
											break;
										
#endif
									}
									if (diff)
										break;
								}

								
								//16byte
								//int32_t nloop		= (((_context->block_height * _context->block_width) << 2) / simd_align) * simd_align;
								//int32_t remain		= ((_context->block_height * _context->block_width) << 2) - nloop;

								//__asm
								//{
								//	pushad

								//	mov			eax, process_data
								//	mov			ebx, reference_buffer
								//	mov			esi, 0
								//	//mov			ecx, align

								//MLOOP:
								//	movdqu		xmm0, [eax + esi]
								//	movdqu		xmm1, [ebx + esi]
								//	pcmpeqd		xmm0, xmm1
								//	pmovmskb	edx, xmm0
								//	
								//		
								//	cmp			edx, 0
								//	jne			END
								//	jmp			CONTINUE

								//END:
								//	mov			diff, 1

								//CONTINUE:	
								//	add			esi, simd_align
								//	cmp			esi, nloop
								//	jne			MLOOP

								//	popad
								//}

								//bool diff = false;
								//for (int32_t bh = 0; bh < _context->block_height; bh++)
								//{
								//	for (int32_t bw = 0; bw < _context->block_width; bw++)
								//	{
								//		int32_t ci = (h + bh) * (_context->width << 2) + (w << 2) + (bw << 2);
								//		int32_t bi = ci + 0;
								//		int32_t gi = ci + 1;
								//		int32_t ri = ci + 2;
								//		int32_t ai = ci + 3;

								//		int32_t bdiff = labs(*(process_data + bi) - *(reference_buffer + bi));
								//		int32_t gdiff = labs(*(process_data + gi) - *(reference_buffer + gi));
								//		int32_t rdiff = labs(*(process_data + ri) - *(reference_buffer + ri));
								//		int32_t adiff = labs(*(process_data + ai) - *(reference_buffer + ai));

								//		if ((bdiff + gdiff + rdiff + adiff) > 0)
								//		{
								//			diff = true;
								//			break;
								//		}
								//	}
								//}

								if(diff)
#endif
#endif
								{
									for (int32_t eh = 0, row_index = 0; eh < _context->block_height; eh++, row_index++)
									{
										int32_t src_index = (h + eh) * (_context->width << 2) + (w << 2);
										//*(process_data + src_index) = 0;
										//*(process_data + src_index + 1) = 0;
										//*(process_data + src_index + 2) = 0;
										//if (_context->block_height == (bh + 1))
										//	memset(process_data + src_index, 0x00, _context->block_width << 2);
										rows[row_index] = process_data + src_index;

#if !defined(WITH_AVX2_BILINEAR_RESIZE) && !defined(WITH_AVX2_SIMD)
										memmove(reference_buffer + +src_index, process_data + src_index, _context->block_width << 2);
#endif
									}

									if (_front)
									{ 
										sirius::library::video::transform::codec::partial::png::compressor::entity_t input;
										input.data = rows;
										input.data_capacity = _context->block_height;
										input.data_size = _context->block_height;
										input.x = 0;
										input.y = 0;
										input.width = _context->block_width;
										input.height = _context->block_height;

										sirius::library::video::transform::codec::partial::png::compressor::entity_t bitstream;
										bitstream.data = real_compressed_buffer;
										bitstream.data_capacity = real_compressed_buffer - compressed_buffer;
										bitstream.data_capacity = compressed_buffer_size - bitstream.data_capacity;
										bitstream.data_size = 0;
										bitstream.x = input.x;
										bitstream.y = input.y;
										bitstream.width = input.width;
										bitstream.height = input.height;

#if defined(DEBUG)

										LARGE_INTEGER cmprs_begin;
										LARGE_INTEGER cmprs_end;
										::QueryPerformanceCounter(&cmprs_begin);
#endif
										status = _real_compressor->compress(&input, &bitstream);
#if defined(DEBUG)
										::QueryPerformanceCounter(&cmprs_end);
										LONGLONG time_diff = cmprs_end.QuadPart - cmprs_begin.QuadPart;
										double cmprs_duration = (double)time_diff * 1000.0 / (double)_frequency.QuadPart;
										char debug[100] = { 0 };
										_snprintf_s(debug, sizeof(debug), "png compression duration is %f\n", cmprs_duration);
										::OutputDebugStringA(debug);
#endif
										if (status == sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success)
										{
											index = (h / _context->block_height) * (_context->width / _context->block_width) + w / _context->block_width;
											pindex[count] = index;
											pcompressed[count] = (uint8_t*)bitstream.data;
											plength[count] = bitstream.data_size;
											real_compressed_buffer += bitstream.data_size;

											cached_index[index] = index;
											cached_length[index] = bitstream.data_size;
											memmove(cached_compressed[index], pcompressed[count], cached_length[index]);

											count++;
										}
									}
								}
							}
						}
					}

					process_force_fullmode = false;
					if (_invalidate && _context->binvalidate)
					{
						_front->after_process_callback(block_count, cached_index, cached_compressed, cached_length, before_encode_timestamp, after_encode_timestamp);
						_invalidate = false;
					}
					else
					{
						if (count > 0)
						{
							_front->after_process_callback(count, pindex, pcompressed, plength, before_encode_timestamp, after_encode_timestamp);
						}
					}
#if defined(WITH_AVX2_SIMD)
					memmove(reference_buffer, resized_buffer, resized_buffer_size);
#else
#if defined(WITH_AVX2_BILINEAR_RESIZE)
					memmove(reference_buffer, resized_buffer, resized_buffer_size);
#else
					//memmove(reference_buffer, process_data, process_data_size);
#endif
#endif
					{
						sirius::autolock lock(&_cs);
						iobuffer = _iobuffer_queue.get_pending();
						if (iobuffer)
						{
							process_data_size = iobuffer->input.data_size;
							process_timestamp = iobuffer->input.timestamp;
							memmove(process_data, iobuffer->input.data, process_data_size);
							process_x = iobuffer->input.x;
							process_y = iobuffer->input.y;
							process_width = iobuffer->input.width;
							process_height = iobuffer->input.height;
							process_force_fullmode = iobuffer->input.force_fullmode;
							continue;
						}
						else
						{
							process_data_size = 0;
							break;
						}
					}
				}
			}
			_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressed;
		}
	}

	if (process_data)
		_aligned_free(process_data);
	process_data = nullptr;

	if (pindex)
		delete[] pindex;
	if (plength)
		delete[] plength;
	if (pcompressed)
		delete[] pcompressed;

	if (cached_index)
		delete[] cached_index;
	if (cached_length)
		delete[] cached_length;
	if (cached_capacity)
		delete[] cached_capacity;
	if (cached_compressed)
	{
		for (int32_t index = 0; index < block_count; index++)
		{
			delete[] cached_compressed[index];
		}
		delete[] cached_compressed;
	}

	if (rows)
		free(rows);
	rows = nullptr;

#if defined(WITH_AVX2_SIMD)
	if (gray_buffer)
		free(gray_buffer);
	gray_buffer = nullptr;
	gray_buffer_size = 0;

	if (resized_buffer)
		free(resized_buffer);
	resized_buffer = nullptr;
	resized_buffer_size = 0;

	if (reference_buffer)
		free(reference_buffer);
	reference_buffer = nullptr;
#else
#if defined(WITH_AVX2_BILINEAR_RESIZE)
	if (resized_buffer)
		free(resized_buffer);
	resized_buffer = nullptr;
	resized_buffer_size = 0;

	if (reference_buffer)
		free(reference_buffer);
	reference_buffer = nullptr;
#else
	if (reference_buffer)
		_aligned_free(reference_buffer);
	reference_buffer = nullptr;
	reference_buffer_size = 0;
#endif
#endif

	if (compressed_buffer)
		free(compressed_buffer);
	compressed_buffer = nullptr;
	compressed_buffer_size = 0;
}

void sirius::library::video::transform::codec::partial::png::compressor::core::process_psend_indexed(void)
{
	int32_t status = sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
	sirius::library::video::transform::codec::partial::png::compressor::core::buffer_t * iobuffer = nullptr;

	long long before_encode_timestamp = 0;
	long long after_encode_timestamp = 0;

	int32_t	compressed_buffer_size = 1024 * 1024 * 2;
	uint8_t * compressed_buffer = static_cast<uint8_t*>(malloc(compressed_buffer_size));
	memset(compressed_buffer, 0x00, compressed_buffer_size);

#if defined(WITH_AVX2_SIMD)
	int32_t		gray_buffer_size = ((_context->width) * (_context->height));
	uint8_t *	gray_buffer = static_cast<uint8_t*>(malloc(gray_buffer_size));
	memset(gray_buffer, 0x00, gray_buffer_size);

	int32_t		resized_buffer_size = ((_context->width >> 2) * (_context->height >> 2));
	uint8_t *	resized_buffer = static_cast<uint8_t*>(malloc(resized_buffer_size));
	memset(resized_buffer, 0x00, resized_buffer_size);

	uint8_t *	reference_buffer = static_cast<uint8_t*>(malloc(resized_buffer_size));
	memset(reference_buffer, 0x00, resized_buffer_size);
#else
#if defined(WITH_AVX2_BILINEAR_RESIZE)
#if defined(WITH_DOUBLE_DOWN_SIZE)
	int32_t		resized_buffer_size = ((_context->width >> 2) * (_context->height >> 2)) << 2;
	uint8_t *	resized_buffer = static_cast<uint8_t*>(malloc(resized_buffer_size));
	memset(resized_buffer, 0x00, resized_buffer_size);

	uint8_t *	reference_buffer = static_cast<uint8_t*>(malloc(resized_buffer_size));
	memset(reference_buffer, 0x00, resized_buffer_size);
#else
	int32_t		resized_buffer_size = ((_context->width >> 3) * (_context->height >> 3)) << 2;
	uint8_t *	resized_buffer = static_cast<uint8_t*>(malloc(resized_buffer_size));
	memset(resized_buffer, 0x00, resized_buffer_size);

	uint8_t *	reference_buffer = static_cast<uint8_t*>(malloc(resized_buffer_size));
	memset(reference_buffer, 0x00, resized_buffer_size);
#endif
#else
	int32_t		reference_buffer_size = (_context->width * _context->height) << 2;
	uint8_t *	reference_buffer = static_cast<uint8_t*>(malloc(reference_buffer_size));
	memset(reference_buffer, 0x00, reference_buffer_size);
#endif
#endif

	uint8_t **	rows = static_cast<uint8_t**>(malloc(_context->block_height * sizeof(uint8_t*)));

	int32_t		block_count = (_context->width / _context->block_width) * (_context->height / _context->block_height);
	int32_t *	pindex = new int32_t[block_count];
	uint8_t **	pcompressed = new uint8_t*[block_count];
	int32_t *	plength = new int32_t[block_count];

	int32_t		nbytes_compressed = (_context->block_width*_context->block_height) << 2;
	int32_t *	cached_index = new int32_t[block_count];
	uint8_t **	cached_compressed = new uint8_t*[block_count];
	int32_t *	cached_capacity = new int32_t[block_count];
	int32_t *	cached_length = new int32_t[block_count];
	for (int32_t index = 0; index < block_count; index++)
	{
		cached_index[index] = index;
		cached_capacity[index] = nbytes_compressed;
		cached_compressed[index] = new uint8_t[cached_capacity[index]];
		cached_length[index] = 0;
	}

	int32_t		process_data_capacity = (_context->width * _context->height) << 2;
	int32_t		process_data_size = 0;
	uint8_t *	process_data = static_cast<uint8_t*>(malloc(process_data_capacity));
	int32_t		process_x = 0;
	int32_t		process_y = 0;
	int32_t		process_width = 0;
	int32_t		process_height = 0;
	long long	process_timestamp = 0;
	bool		process_force_fullmode = false;
	bool		process_compress_first_time = true;
	while (_run)
	{
		if (::WaitForSingleObject(_event, INFINITE) == WAIT_OBJECT_0)
		{
			_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressing;

			{
				sirius::autolock lock(&_cs);
				iobuffer = _iobuffer_queue.get_pending();
				if (!iobuffer)
				{
					if (!_invalidate)
					{
						_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressed;
						::Sleep(10);
						continue;
					}
					process_data_size = 0;
				}
				else
				{
					process_data_size = iobuffer->input.data_size;
					process_timestamp = iobuffer->input.timestamp;
					memmove(process_data, iobuffer->input.data, process_data_size);
					process_x = iobuffer->input.x;
					process_y = iobuffer->input.y;
					process_width = iobuffer->input.width;
					process_height = iobuffer->input.height;
					process_force_fullmode = iobuffer->input.force_fullmode;
				}
			}

			if (process_compress_first_time)
			{
				before_encode_timestamp = process_timestamp;
				int32_t count = 0;
				int32_t index = 0;
				uint8_t * real_compressed_buffer = compressed_buffer;

				for (int32_t h = 0; h < _context->height; h = h + _context->block_height)
				{
					for (int32_t w = 0; w < _context->width; w = w + _context->block_width)
					{
						for (int32_t eh = 0, row_index = 0; eh < _context->block_height; eh++, row_index++)
						{
							int32_t src_index = (h + eh) * (_context->width << 2) + (w << 2);
							rows[row_index] = process_data + src_index;
						}

						if (_front)
						{
							sirius::library::video::transform::codec::partial::png::compressor::entity_t input;
							input.data = rows;
							input.data_capacity = _context->block_height;
							input.data_size = _context->block_height;
							input.x = 0;
							input.y = 0;
							input.width = _context->block_width;
							input.height = _context->block_height;

							sirius::library::video::transform::codec::partial::png::compressor::entity_t bitstream;
							bitstream.data = real_compressed_buffer;
							bitstream.data_capacity = real_compressed_buffer - compressed_buffer;
							bitstream.data_capacity = compressed_buffer_size - bitstream.data_capacity;
							bitstream.data_size = 0;
							bitstream.x = input.x;
							bitstream.y = input.y;
							bitstream.width = input.width;
							bitstream.height = input.height;

							status = _real_compressor->compress(&input, &bitstream);
							if (status == sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success)
							{
								pindex[count] = index;
								pcompressed[count] = (uint8_t*)bitstream.data;
								plength[count] = bitstream.data_size;
								real_compressed_buffer += bitstream.data_size;

								cached_index[index] = index;
								cached_length[index] = bitstream.data_size;
								memmove(cached_compressed[index], pcompressed[count], cached_length[index]);

								count++;
							}
						}
						index++;
					}
				}
				_front->after_process_callback(count, pindex, pcompressed, plength, before_encode_timestamp, after_encode_timestamp);

				process_compress_first_time = false;
				continue;
			}

			if (_invalidate && _context->binvalidate && (process_data_size == 0))
			{
				_front->after_process_callback(block_count, cached_index, cached_compressed, cached_length, before_encode_timestamp, after_encode_timestamp);
				_invalidate = false;
			}
			else
			{
				while (process_data_size>0)
				{
					before_encode_timestamp = process_timestamp;

					int32_t		count = 0;
					int32_t		index = 0;
					uint8_t *	real_compressed_buffer = compressed_buffer;
					int32_t		begin_height = 0;
					int32_t		end_height = 0;
					int32_t		begin_width = 0;
					int32_t		end_width = 0;
					if (process_force_fullmode)
					{
						begin_height = 0;
						end_height = _context->height;
						begin_width = 0;
						end_width = _context->width;
					}
					else
					{
						begin_height = (process_y / _context->block_height) * _context->block_height;
						end_height = ((process_y + process_height) / _context->block_height) * _context->block_height;

						if (((process_y + process_height) % _context->block_height) > 0)
							end_height += _context->block_height;

						begin_width = (process_x / _context->block_width) * _context->block_width;
						end_width = ((process_x + process_width) / _context->block_width) * _context->block_width;

						if (((process_x + process_width) % _context->block_width) > 0)
							end_width += _context->block_width;
					}

#if defined(WITH_AVX2_SIMD)
					int32_t process_index = (begin_height) * (_context->width << 2) + (begin_width << 2);
					int32_t gray_index = (begin_height) * (_context->width) + (begin_width);
					int32_t resize_index = (begin_height >> 2) * (_context->width >> 2) + (begin_width >> 2);
					SimdBgraToGray(process_data + process_index, end_width - begin_width, end_height - begin_height, _context->width << 2, gray_buffer + gray_index, end_width - begin_width);
					SimdResizeBilinear(gray_buffer + gray_index, end_width - begin_width, end_height - begin_height, _context->width, resized_buffer + resize_index, (end_width - begin_width) >> 2, (end_height - begin_height) >> 2, _context->width >> 2, 1);

					for (int32_t h = 0, h2 = 0; h < _context->height; h = h + _context->block_height, h2 = h2 + (_context->block_height >> 2))
					{
						for (int32_t w = 0, w2 = 0; w < _context->width; w = w + _context->block_width, w2 = w2 + (_context->block_width >> 2))
						{
#else
#if defined(WITH_AVX2_BILINEAR_RESIZE)
#if defined(WITH_DOUBLE_DOWN_SIZE)
					//SIMD_RESIZER::resize_bilinear(process_data, _context->width, _context->height, _context->width << 2, resized_buffer, _context->width >> 2, _context->height >> 2, _context->width);
					//for (int32_t h = 0, h2 = 0; h < _context->height; h = h + _context->block_height, h2 = h2 + (_context->block_height >> 2))
					//{
					//	for (int32_t w = 0, w2 = 0; w < _context->width; w = w + _context->block_width, w2 = w2 + (_context->block_width >> 2))
					//	{

					int32_t process_index = (begin_height) * (_context->width << 2) + (begin_width << 2);
					int32_t resize_index = (begin_height >> 2) * (_context->width) + begin_width;
					SIMD_RESIZER::resize_bilinear(process_data + process_index, end_width - begin_width, end_height - begin_height, _context->width << 2, resized_buffer, (end_width - begin_width) >> 2, (end_height - begin_height) >> 2, _context->width);
					for (int32_t h = 0, h2 = 0; h < end_height; h = h + _context->block_height, h2 = h2 + (_context->block_height >> 2))
					{
						for (int32_t w = 0, w2 = 0; w < end_width; w = w + _context->block_width, w2 = w2 + (_context->block_width >> 2))
						{
#else
					int32_t process_index = (begin_height) * (_context->width << 2) + (begin_width << 2);
					int32_t resize_index = (begin_height >> 3) * ((_context->width >> 3) << 2) + ((begin_width >> 3) << 2);
					SIMD_RESIZER::resize_bilinear(process_data + process_index, end_width - begin_width, end_height - begin_height, _context->width << 2, resized_buffer, (end_width - begin_width) >> 3, (end_height - begin_height) >> 3, (_context->width >> 3) << 2);
					for (int32_t h = 0, h2 = 0; h < end_height; h = h + _context->block_height, h2 = h2 + (_context->block_height >> 3))
					{
						for (int32_t w = 0, w2 = 0; w < end_width; w = w + _context->block_width, w2 = w2 + (_context->block_width >> 3))
						{
#endif
#else
					for (int32_t h = 0; h < _context->height; h = h + _context->block_height)
					{
						for (int32_t w = 0; w < _context->width; w = w + _context->block_width)
						{
#endif
#endif
							if (h >= begin_height && h < end_height && w >= begin_width && w < end_width)
							{
#if defined(WITH_AVX2_SIMD)
								uint64_t sum = 0;
								SimdSquaredDifferenceSum(resized_buffer + (h2 * (_context->width >> 2) + w2), _context->width >> 2, reference_buffer + (h2 * (_context->width >> 2) + w2), _context->width >> 2, _context->block_width >> 2, _context->block_height >> 2, &sum);
								if (sum > 0)
#else
#if defined(WITH_AVX2_BILINEAR_RESIZE)
								bool diff = false;
#if defined(WITH_DOUBLE_DOWN_SIZE)
								for (int32_t bh = 0; bh < (_context->block_height >> 2); bh++)
								{
									for (int32_t bw = 0; bw < (_context->block_width >> 2); bw++)
									{
										int32_t ci = (h2 + bh) * (_context->width) + (w2 << 2) + (bw << 2);
#else
								for (int32_t bh = 0; bh < (_context->block_height >> 3); bh++)
								{
									for (int32_t bw = 0; bw < (_context->block_width >> 3); bw++)
									{
										int32_t ci = (h2 + bh) * ((_context->width >> 3) << 2) + (w2 << 2) + (bw << 2);
#endif
										int32_t bi = ci + 0;
										int32_t gi = ci + 1;
										int32_t ri = ci + 2;
										int32_t ai = ci + 3;
										int32_t bdiff = labs(*(resized_buffer + bi) - *(reference_buffer + bi));
										int32_t gdiff = labs(*(resized_buffer + gi) - *(reference_buffer + gi));
										int32_t rdiff = labs(*(resized_buffer + ri) - *(reference_buffer + ri));
										int32_t adiff = labs(*(resized_buffer + ai) - *(reference_buffer + ai));
										if ((bdiff + gdiff + rdiff + adiff) > 0)
										{
											diff = true;
											break;
										}
									}
								}

								if (diff)
#else
								bool diff = false;
								for (int32_t bh = 0; bh < _context->block_height; bh++)
								{
									for (int32_t bw = 0; bw < _context->block_width; bw++)
									{
										int32_t ci = (h + bh) * (_context->width << 2) + (w << 2) + (bw << 2);
										int32_t bi = ci + 0;
										int32_t gi = ci + 1;
										int32_t ri = ci + 2;
										int32_t ai = ci + 3;

										int32_t bdiff = labs(*(process_data + bi) - *(reference_buffer + bi));
										int32_t gdiff = labs(*(process_data + gi) - *(reference_buffer + gi));
										int32_t rdiff = labs(*(process_data + ri) - *(reference_buffer + ri));
										int32_t adiff = labs(*(process_data + ai) - *(reference_buffer + ai));

										if ((bdiff + gdiff + rdiff + adiff) > 0)
										{
											diff = true;
											break;
										}
									}
								}

								if (diff)
#endif
#endif
								{
									for (int32_t bh = 0, row_index = 0; bh < _context->block_height; bh++, row_index++)
									{
										int32_t src_index = (h + bh) * (_context->width << 2) + (w << 2);
										//*(process_data + src_index) = 0;
										//*(process_data + src_index + 1) = 0;
										//*(process_data + src_index + 2) = 0;
										//if (_context->block_height == (bh + 1))
										//	memset(process_data + src_index, 0x00, _context->block_width << 2);
										rows[row_index] = process_data + src_index;
									}

									if (_front)
									{
										sirius::library::video::transform::codec::partial::png::compressor::entity_t input;
										input.data = rows;
										input.data_capacity = _context->block_height;
										input.data_size = _context->block_height;
										input.x = 0;
										input.y = 0;
										input.width = _context->block_width;
										input.height = _context->block_height;

										sirius::library::video::transform::codec::partial::png::compressor::entity_t bitstream;
										bitstream.data = real_compressed_buffer;
										bitstream.data_capacity = real_compressed_buffer - compressed_buffer;
										bitstream.data_capacity = compressed_buffer_size - bitstream.data_capacity;
										bitstream.data_size = 0;
										bitstream.x = input.x;
										bitstream.y = input.y;
										bitstream.width = input.width;
										bitstream.height = input.height;

										status = _real_compressor->compress(&input, &bitstream);
										if (status == sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success)
										{
											pindex[count] = index;
											pcompressed[count] = (uint8_t*)bitstream.data;
											plength[count] = bitstream.data_size;
											real_compressed_buffer += bitstream.data_size;

											_front->after_process_callback(pindex[count], pcompressed[count], plength[count], before_encode_timestamp, after_encode_timestamp);

											cached_index[index] = index;
											cached_length[index] = bitstream.data_size;
											memmove(cached_compressed[index], pcompressed[count], cached_length[index]);

											count++;
										}
									}
								}
							}
							else
							{
								if (_invalidate && _context->binvalidate)
									_front->after_process_callback(cached_index[index], cached_compressed[index], cached_length[index], before_encode_timestamp, after_encode_timestamp);
							}
							index++;
						}
					}

					if (_invalidate && _context->binvalidate)
					{
						_invalidate = false;
					}
#if defined(WITH_AVX2_SIMD)
					memmove(reference_buffer, resized_buffer, resized_buffer_size);
#else
#if defined(WITH_AVX2_BILINEAR_RESIZE)
					memmove(reference_buffer, resized_buffer, resized_buffer_size);
#else
					memmove(reference_buffer, process_data, process_data_size);
#endif
#endif
					{
						sirius::autolock lock(&_cs);
						iobuffer = _iobuffer_queue.get_pending();
						if (iobuffer)
						{
							process_data_size = iobuffer->input.data_size;
							process_timestamp = iobuffer->input.timestamp;
							memmove(process_data, iobuffer->input.data, process_data_size);
							process_x = iobuffer->input.x;
							process_y = iobuffer->input.y;
							process_width = iobuffer->input.width;
							process_height = iobuffer->input.height;

							char msg[MAX_PATH] = { 0 };
							_snprintf_s(msg, MAX_PATH, "ob_index=%d,  x=%d, y=%d, width=%d, height=%d \n", ob_index, process_x, process_y, process_width, process_height);
							::OutputDebugStringA(msg);
							ob_index++;

							continue;
						}
						else
						{
							process_data_size = 0;
							break;
						}
					}

				}
			}
			_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressed;
		}
	}

	if (process_data)
		free(process_data);
	process_data = nullptr;

	if (pindex)
		delete[] pindex;
	if (plength)
		delete[] plength;
	if (pcompressed)
		delete[] pcompressed;

	if (cached_index)
		delete[] cached_index;
	if (cached_length)
		delete[] cached_length;
	if (cached_capacity)
		delete[] cached_capacity;
	if (cached_compressed)
	{
		for (int32_t index = 0; index < block_count; index++)
		{
			delete[] cached_compressed[index];
		}
		delete[] cached_compressed;
	}

	if (rows)
		free(rows);
	rows = nullptr;

#if defined(WITH_AVX2_SIMD)
	if (gray_buffer)
		free(gray_buffer);
	gray_buffer = nullptr;
	gray_buffer_size = 0;

	if (resized_buffer)
		free(resized_buffer);
	resized_buffer = nullptr;
	resized_buffer_size = 0;

	if (reference_buffer)
		free(reference_buffer);
	reference_buffer = nullptr;
#else
#if defined(WITH_AVX2_BILINEAR_RESIZE)
	if (resized_buffer)
		free(resized_buffer);
	resized_buffer = nullptr;
	resized_buffer_size = 0;

	if (reference_buffer)
		free(reference_buffer);
	reference_buffer = nullptr;
#else
	if (reference_buffer)
		free(reference_buffer);
	reference_buffer = nullptr;
	reference_buffer_size = 0;
#endif
#endif

	if (compressed_buffer)
		free(compressed_buffer);
	compressed_buffer = nullptr;
	compressed_buffer_size = 0;
}

void sirius::library::video::transform::codec::partial::png::compressor::core::process(void)
{
	int32_t status = sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
	sirius::library::video::transform::codec::partial::png::compressor::core::buffer_t * iobuffer = nullptr;

	long long before_encode_timestamp = 0;
	long long after_encode_timestamp = 0;

	uint8_t *	converted_buffer = nullptr;
	//uint8_t *	resized_buffer = nullptr;
	int32_t		me_buffer_size = 0;
	uint8_t *	me_buffer = nullptr;
	uint8_t *	prev_me_buffer = nullptr;
	bool		prev_me_filled = false;
	int32_t		block_buffer_size = 0;
	uint8_t *	block_buffer = nullptr;
	int32_t		compressed_buffer_size = 0;
	uint8_t *	compressed_buffer = nullptr;


	int32_t		context_width = 0;
	int32_t		context_height = 0;
	int32_t		block_width = 0;
	int32_t		block_height = 0;
	{
		context_width = _context->width >> 2;
		context_height = _context->height >> 2;
		block_width = _context->block_width >> 2;
		block_height = _context->block_height >> 2;
		converted_buffer = static_cast<uint8_t*>(malloc(_context->width * _context->height));

#if defined(WITH_AVX2_SIMD)
		me_buffer_size = context_width * context_height;
		me_buffer = static_cast<uint8_t*>(_aligned_malloc(me_buffer_size, AVX2_ALIGN_SIZE));
		memset(me_buffer, 0x00, me_buffer_size);
		prev_me_buffer = static_cast<uint8_t*>(_aligned_malloc(me_buffer_size, AVX2_ALIGN_SIZE));
		memset(prev_me_buffer, 0x00, me_buffer_size);
#else
		me_buffer_size = context_width * context_height;
		me_buffer = static_cast<uint8_t*>(malloc(me_buffer_size));
		memset(me_buffer, 0x00, me_buffer_size);
		prev_me_buffer = static_cast<uint8_t*>(malloc(me_buffer_size));
		memset(prev_me_buffer, 0x00, me_buffer_size);
#endif

		block_buffer_size = (_context->block_width * _context->block_height) << 2;
		block_buffer = static_cast<uint8_t*>(malloc(block_buffer_size));
		memset(block_buffer, 0x00, block_buffer_size);

		compressed_buffer_size = 1024 * 1024 * 1;
		compressed_buffer = static_cast<uint8_t*>(malloc(compressed_buffer_size));
		memset(compressed_buffer, 0x00, compressed_buffer_size);
	}

	int32_t		block_count = (_context->width / _context->block_width) * (_context->height / _context->block_height);
	int32_t *	pindex = new int32_t[block_count];
	uint8_t **	pcompressed = new uint8_t*[block_count];
	int32_t *	plength = new int32_t[block_count];

	int32_t		nbytes_compressed = (_context->block_width*_context->block_height) << 2;
	int32_t *	cached_index = new int32_t[block_count];
	uint8_t **	cached_compressed = new uint8_t*[block_count];
	int32_t *	cached_capacity = new int32_t[block_count];
	int32_t *	cached_length = new int32_t[block_count];
	for (int32_t index = 0; index < block_count; index++)
	{
		cached_index[index] = index;
		cached_capacity[index] = nbytes_compressed;
		cached_compressed[index] = new uint8_t[cached_capacity[index]];
		cached_length[index] = 0;
	}

	int32_t		process_data_capacity = (_context->width * _context->height) << 2;
	int32_t		process_data_size = 0;
	uint8_t *	process_data = (uint8_t*)malloc(process_data_capacity);
	int32_t		process_x = 0;
	int32_t		process_y = 0;
	int32_t		process_width = 0;
	int32_t		process_height = 0;
	long long	process_timestamp = 0;
	while (_run)
	{
		if (::WaitForSingleObject(_event, INFINITE) == WAIT_OBJECT_0)
		{
			_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressing;

			{
				sirius::autolock lock(&_cs);
				iobuffer = _iobuffer_queue.get_pending();
				if (!iobuffer)
				{
					if (!_invalidate)
					{
						_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressed;
						continue;
					}
					process_data_size = 0;
				}
				else
				{
					process_data_size = iobuffer->input.data_size;
					process_timestamp = iobuffer->input.timestamp;
					memmove(process_data, iobuffer->input.data, process_data_size);
					process_x = iobuffer->input.x;
					process_y = iobuffer->input.y;
					process_width = iobuffer->input.width;
					process_height = iobuffer->input.height;
				}
			}

			if (_invalidate && (process_data_size == 0))
			{
				_front->after_process_callback(block_count, cached_index, cached_compressed, cached_length, before_encode_timestamp, after_encode_timestamp);
				_invalidate = false;
			}
			else
			{
				while (process_data_size>0)
				{
					before_encode_timestamp = process_timestamp;
					if (prev_me_filled)
					{
#if defined(WITH_AVX2_SIMD)
						avx2_bgra2gray(true, process_data, _context->width, _context->height, _context->width << 2, converted_buffer, _context->width);
						SimdResizeBilinear(converted_buffer, _context->width, _context->height, _context->width, me_buffer, context_width, context_height, context_width, 1);
#else
						SimdBgraToGray(process_data, _context->width, _context->height, _context->width << 2, converted_buffer, _context->width);
						SimdResizeBilinear(converted_buffer, _context->width, _context->height, _context->width, me_buffer, context_width, context_height, context_width, 1);
#endif
						int32_t count = 0;
						int32_t index = 0;
						uint8_t * real_compressed_buffer = compressed_buffer;
						for (int32_t h = 0, h2 = 0; h < _context->height; h = h + _context->block_height, h2 = h2 + block_height)
						{
							for (int32_t w = 0, w2 = 0; w < _context->width; w = w + _context->block_width, w2 = w2 + block_width)
							{
#if defined(WITH_AVX2_SIMD)
								bool bdiff = avx2_is_different(true, me_buffer + (h2*context_width + w2), context_width, prev_me_buffer + (h2*context_width + w2), context_width, block_width, block_height);
								if (bdiff)
#else
								uint64_t sum = 0;
								SimdAbsDifferenceSum(me_buffer + (h2*context_width + w2), context_width, prev_me_buffer + (h2*context_width + w2), context_width, block_width, block_height, &sum);
								if (sum>0)
#endif
								{
									for (int32_t bh = 0; bh < _context->block_height; bh++)
									{
										int32_t src_index = (h + bh) * (_context->width << 2) + (w << 2);
										int32_t dst_index = bh * (_context->block_width << 2);
										memmove(block_buffer + dst_index, process_data + src_index, (_context->block_width << 2));
									}

									if (_front)
									{
										sirius::library::video::transform::codec::partial::png::compressor::entity_t input;
										input.memtype = sirius::library::video::transform::codec::partial::png::compressor::video_memory_type_t::host;
										input.data = block_buffer;
										input.data_capacity = block_buffer_size;
										input.data_size = block_buffer_size;

										sirius::library::video::transform::codec::partial::png::compressor::entity_t bitstream;
										bitstream.memtype = sirius::library::video::transform::codec::partial::png::compressor::video_memory_type_t::host;
										bitstream.data = real_compressed_buffer;
										bitstream.data_capacity = real_compressed_buffer - compressed_buffer;
										bitstream.data_capacity = compressed_buffer_size - bitstream.data_capacity;
										bitstream.data_size = 0;

										status = _real_compressor->compress(&input, &bitstream);
										if (status == sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success)
										{
											pindex[count] = index;
											pcompressed[count] = (uint8_t*)bitstream.data;
											plength[count] = bitstream.data_size;
											real_compressed_buffer += bitstream.data_size;

#if defined(WITH_PARTIAL_SENDING_MODE)
											_front->after_process_callback(pindex[count], pcompressed[count], plength[count], before_encode_timestamp, after_encode_timestamp);
#endif

											cached_index[index] = index;
											cached_length[index] = bitstream.data_size;
											memmove(cached_compressed[index], pcompressed[count], cached_length[index]);

											count++;
										}
									}
								}
								else
								{
#if defined(WITH_PARTIAL_SENDING_MODE)
									if (_invalidate)
										_front->after_process_callback(cached_index[index], cached_compressed[index], cached_length[index], before_encode_timestamp, after_encode_timestamp);
#endif
								}
								index++;
							}
						}
						if (_invalidate)
						{
#if !defined(WITH_PARTIAL_SENDING_MODE)
							_front->after_process_callback(block_count, cached_index, cached_compressed, cached_length, before_encode_timestamp, after_encode_timestamp);
#endif
							_invalidate = false;
						}
#if !defined(WITH_PARTIAL_SENDING_MODE)
						else
						{
							if (count>0)
								_front->after_process_callback(count, pindex, pcompressed, plength, before_encode_timestamp, after_encode_timestamp);
						}
#endif
					}
					else
					{

#if defined(WITH_AVX2_SIMD)
						avx2_bgra2gray(true, process_data, _context->width, _context->height, _context->width << 2, converted_buffer, _context->width);
						SimdResizeBilinear(converted_buffer, _context->width, _context->height, _context->width, me_buffer, context_width, context_height, context_width, 1);
#else
						SimdBgraToGray(process_data, _context->width, _context->height, _context->width << 2, converted_buffer, _context->width);
						SimdResizeBilinear(converted_buffer, _context->width, _context->height, _context->width, me_buffer, context_width, context_height, context_width, 1);
#endif

						int32_t count = 0;
						int32_t index = 0;

						uint8_t * real_compressed_buffer = compressed_buffer;

						for (int32_t h = 0; h < _context->height; h = h + _context->block_height)
						{
							for (int32_t w = 0; w < _context->width; w = w + _context->block_width)
							{

								for (int32_t bh = 0; bh < _context->block_height; bh++)
								{
									int32_t src_index = (h + bh) * (_context->width << 2) + (w << 2);
									int32_t dst_index = bh * (_context->block_width << 2);
									memmove(block_buffer + dst_index, process_data + src_index, (_context->block_width << 2));
								}

								if (_front)
								{
									sirius::library::video::transform::codec::partial::png::compressor::entity_t input;
									input.memtype = sirius::library::video::transform::codec::partial::png::compressor::video_memory_type_t::host;
									input.data = block_buffer;
									input.data_capacity = block_buffer_size;
									input.data_size = block_buffer_size;

									sirius::library::video::transform::codec::partial::png::compressor::entity_t bitstream;
									bitstream.memtype = sirius::library::video::transform::codec::partial::png::compressor::video_memory_type_t::host;
									bitstream.data = real_compressed_buffer;
									bitstream.data_capacity = real_compressed_buffer - compressed_buffer;
									bitstream.data_capacity = compressed_buffer_size - bitstream.data_capacity;
									bitstream.data_size = 0;

									status = _real_compressor->compress(&input, &bitstream);
									if (status == sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success)
									{
										pindex[count] = index;
										pcompressed[count] = (uint8_t*)bitstream.data;
										plength[count] = bitstream.data_size;
										real_compressed_buffer += bitstream.data_size;

#if defined(WITH_PARTIAL_SENDING_MODE)
										_front->after_process_callback(pindex[count], pcompressed[count], plength[count], before_encode_timestamp, after_encode_timestamp);
#endif

										cached_index[index] = index;
										cached_length[index] = plength[count];
										memmove(cached_compressed[index], pcompressed[count], cached_length[index]);

										count++;
									}
								}
								index++;
							}
						}
#if !defined(WITH_PARTIAL_SENDING_MODE)
						_front->after_process_callback(count, pindex, pcompressed, plength, before_encode_timestamp, after_encode_timestamp);
#endif
					}

					if (!prev_me_filled)
						prev_me_filled = true;
					memcpy(prev_me_buffer, me_buffer, me_buffer_size);

					{
						sirius::autolock lock(&_cs);
						iobuffer = _iobuffer_queue.get_pending();
					}
					if (!iobuffer)
					{
						process_data_size = 0;
						continue;
					}

					process_data_size = iobuffer->input.data_size;
					process_timestamp = iobuffer->input.timestamp;
					memmove(process_data, iobuffer->input.data, process_data_size);

					continue;
				}
			}
			_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressed;
		}
	}

	if (process_data)
		free(process_data);
	process_data = nullptr;

	if (pindex)
		delete[] pindex;
	if (plength)
		delete[] plength;
	if (pcompressed)
		delete[] pcompressed;

	if (cached_index)
		delete[] cached_index;
	if (cached_length)
		delete[] cached_length;
	if (cached_capacity)
		delete[] cached_capacity;
	if (cached_compressed)
	{
		for (int32_t index = 0; index < block_count; index++)
		{
			delete[] cached_compressed[index];
		}
		delete[] cached_compressed;
	}

	if (compressed_buffer)
		free(compressed_buffer);
	compressed_buffer = nullptr;
	compressed_buffer_size = 0;

	if (block_buffer)
		free(block_buffer);
	block_buffer = nullptr;
	block_buffer_size = 0;

#if defined(WITH_AVX2_SIMD)
	if (prev_me_buffer)
		_aligned_free(prev_me_buffer);
	prev_me_buffer = nullptr;

	if (me_buffer)
		_aligned_free(me_buffer);
	me_buffer = nullptr;
#else
	if (prev_me_buffer)
		free(prev_me_buffer);
	prev_me_buffer = nullptr;

	if (me_buffer)
		free(me_buffer);
	me_buffer = nullptr;
#endif

	me_buffer_size = 0;

	if (converted_buffer)
		free(converted_buffer);
	converted_buffer = nullptr;
}


sirius::library::video::transform::codec::partial::png::compressor::core::bounding_box_t * sirius::library::video::transform::codec::partial::png::compressor::core::create_bounding_box(int16_t macro_block_index)
{
	bounding_box_t * bb = static_cast<bounding_box_t*>(malloc(sizeof(bounding_box_t)));
	memset(bb, 0x00, sizeof(bounding_box_t));

	bb->mb = static_cast<uint8_t*>(malloc((_context->block_width >> 2)*(_context->block_height >> 2)));
	bb->mb[macro_block_index] = true;
	bb->bindex = macro_block_index;
	bb->eindex = macro_block_index;
	bb->hcnt++;
	bb->vcnt++;
	bb->cnt++;

	return bb;
}

void sirius::library::video::transform::codec::partial::png::compressor::core::destroy_bounding_box(bounding_box_t * bb)
{
	if (bb)
	{
		if (bb->mb)
		{
			free(bb->mb);
			bb->mb = nullptr;
		}
		free(bb);
		bb = nullptr;
	}
}
*/

int32_t sirius::library::video::transform::codec::partial::png::compressor::core::allocate_io_buffers(void)
{
	int32_t status = sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
	_iobuffer_queue.initialize(_iobuffer, _context->nbuffer);

	for (int32_t i = 0; i < _context->nbuffer; i++)
	{
		_iobuffer[i].input.data_capacity = _context->width * _context->height * 4;
		_iobuffer[i].input.data_size = 0;
		_iobuffer[i].input.data = static_cast<uint8_t*>(malloc(_iobuffer[i].input.data_capacity));
	}
	return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::partial::png::compressor::core::release_io_buffers(void)
{
	for (int32_t i = 0; i < _context->nbuffer; i++)
	{
		if (_iobuffer[i].input.data)
			free(_iobuffer[i].input.data);
		_iobuffer[i].input.data = nullptr;
	}

	_iobuffer_queue.release();
	return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
}

void sirius::library::video::transform::codec::partial::png::compressor::core::copy(sirius::library::video::transform::codec::partial::png::compressor::entity_t * input, sirius::library::video::transform::codec::partial::png::compressor::core::buffer_t * iobuffer)
{
	
	iobuffer->input.timestamp = input->timestamp;
	iobuffer->input.data_size = input->data_size;
	memmove((uint8_t*)iobuffer->input.data, input->data, iobuffer->input.data_size);
	iobuffer->input.x = input->x;
	iobuffer->input.y = input->y;
	iobuffer->input.width = input->width;
	iobuffer->input.height = input->height;

	_prev_x = input->x;
	_prev_y = input->y;
	_prev_width = input->width;
	_prev_height = input->height;


}