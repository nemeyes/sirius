#include "sirius_partial_webp_compressor.h"
#include "partial_webp_compressor.h"

#include <sirius_image_creator.h>
#include <sirius_stringhelper.h>
#include <sirius_locks.h>
#include <vector>
#include <emmintrin.h>


sirius::library::video::transform::codec::partial::webp::compressor::core::core(sirius::library::video::transform::codec::partial::webp::compressor * front)
	: _front(front)
	, _context(nullptr)
	, _state(sirius::library::video::transform::codec::partial::webp::compressor::state_t::none)
	, _event(INVALID_HANDLE_VALUE)
	, _thread(INVALID_HANDLE_VALUE)
	, _run(FALSE)
	, _invalidate(FALSE)
{
	::InitializeCriticalSection(&_lock);
	_real_compressor = new sirius::library::video::transform::codec::libwebp::compressor(front);
}

sirius::library::video::transform::codec::partial::webp::compressor::core::~core(void)
{
	_state = sirius::library::video::transform::codec::partial::webp::compressor::state_t::none;
	if (_real_compressor)
		delete _real_compressor;
	_real_compressor = nullptr;
	::DeleteCriticalSection(&_lock);
}

int32_t sirius::library::video::transform::codec::partial::webp::compressor::core::state(void)
{
	return _state;
}

int32_t sirius::library::video::transform::codec::partial::webp::compressor::core::initialize(sirius::library::video::transform::codec::partial::webp::compressor::context_t * context)
{
	int32_t status = sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::fail;
	if (!context)
		return sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::fail;

	_state = sirius::library::video::transform::codec::partial::webp::compressor::state_t::initializing;
	_context = context;

	allocate_io_buffers();
	_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	_run = true;
	uint32_t thrdaddr = 0;
	_thread = (HANDLE)::_beginthreadex(NULL, 0, sirius::library::video::transform::codec::partial::webp::compressor::core::process_callback, this, 0, &thrdaddr);
	if (_thread == NULL || _thread == INVALID_HANDLE_VALUE)
		return sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::fail;

	status = sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success;

	_state = sirius::library::video::transform::codec::partial::webp::compressor::state_t::initialized;
	return status;
}

int32_t sirius::library::video::transform::codec::partial::webp::compressor::core::release(void)
{
	if (!((_state == sirius::library::video::transform::codec::partial::webp::compressor::state_t::initialized) ||
		(_state == sirius::library::video::transform::codec::partial::webp::compressor::state_t::compressing) ||
		(_state == sirius::library::video::transform::codec::partial::webp::compressor::state_t::compressed)))
		return sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::fail;

	_state = sirius::library::video::transform::codec::partial::webp::compressor::state_t::releasing;
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

	_state = sirius::library::video::transform::codec::partial::webp::compressor::state_t::released;
	return sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::partial::webp::compressor::core::play(void)
{
	_invalidate = true;
	if (_event != NULL && _event != INVALID_HANDLE_VALUE)
		::SetEvent(_event);
	return sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::partial::webp::compressor::core::pause(void)
{
	return sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::partial::webp::compressor::core::stop(void)
{
	return sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::partial::webp::compressor::core::invalidate(void)
{
	_invalidate = true;
	if (_event != NULL && _event != INVALID_HANDLE_VALUE)
		::SetEvent(_event);
	return sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::partial::webp::compressor::core::compress(sirius::library::video::transform::codec::partial::webp::compressor::entity_t * input, sirius::library::video::transform::codec::partial::webp::compressor::entity_t * bitstream)
{
	return sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::partial::webp::compressor::core::compress(sirius::library::video::transform::codec::partial::webp::compressor::entity_t * input)
{
	sirius::library::video::transform::codec::partial::webp::compressor::core::buffer_t * iobuffer = nullptr;

	bool force_fullmode = false;
	sirius::autolock lock(&_lock);
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
	::SetEvent(_event);

	return sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success;
}

unsigned __stdcall sirius::library::video::transform::codec::partial::webp::compressor::core::process_callback(void * param)
{
	sirius::library::video::transform::codec::partial::webp::compressor::core * self = static_cast<sirius::library::video::transform::codec::partial::webp::compressor::core*>(param);
	if (self->_context->indexed_video)
	{
		if (self->_context->nthread > 1)
			self->process_threaded_indexed();
		else
			self->process_indexed();
	}
	else
	{
		if(self->_context->nthread > 1)
			self->process_threaded_coordinated();
		else
			self->process_coordinated();
	}
	return 0;
}

void sirius::library::video::transform::codec::partial::webp::compressor::core::process_coordinated(void)
{
	int32_t status = sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success;
	sirius::library::video::transform::codec::partial::webp::compressor::core::buffer_t * iobuffer = nullptr;

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

	int32_t		process_data_size = reference_buffer_size;
	uint8_t *	process_data = static_cast<uint8_t*>(_aligned_malloc(process_data_size, align));
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
			_state = sirius::library::video::transform::codec::partial::webp::compressor::state_t::compressing;

			{
				sirius::autolock lock(&_lock);
				iobuffer = _iobuffer_queue.get_pending();
				if (!iobuffer)
				{
					if (!_invalidate)
					{
						_state = sirius::library::video::transform::codec::partial::webp::compressor::state_t::compressed;
						::Sleep(10);
						continue;
					}
					process_data_size = 0;
				}
				else
				{
					process_data_size = iobuffer->input.data_size;
					process_timestamp = iobuffer->input.timestamp;
					//process_data = static_cast<uint8_t*>(iobuffer->input.data);
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
				uint8_t * real_compressed_buffer = compressed_buffer;
				
				_real_compressor->initialize(_context);
				for (int32_t h = 0; h < _context->height; h = h + _context->block_height)
				{
					for (int32_t w = 0; w < _context->width; w = w + _context->block_width)
					{
						if (_front)
						{
							sirius::library::video::transform::codec::partial::webp::compressor::entity_t input;
							input.data = process_data + (h * (_context->width << 2) + (w << 2));
							input.data_capacity = _context->block_height;
							input.data_size = _context->block_height;
							input.x = 0;
							input.y = 0;
							input.width = _context->block_width;
							input.height = _context->block_height;
							input.stride = _context->width << 2;

							sirius::library::video::transform::codec::partial::webp::compressor::entity_t bitstream;
							bitstream.data = real_compressed_buffer;
							bitstream.data_capacity = real_compressed_buffer - compressed_buffer;
							bitstream.data_capacity = compressed_buffer_size - bitstream.data_capacity;
							bitstream.data_size = 0;
							bitstream.x = input.x;
							bitstream.y = input.y;
							bitstream.width = input.width;
							bitstream.height = input.height;

							status = _real_compressor->compress(&input, &bitstream);
							if (status == sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success)
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

				continue;
			}

			if (_invalidate && (process_data_size == 0))
			{
				int32_t count = 0;
				uint8_t * real_compressed_buffer = compressed_buffer;
				_real_compressor->initialize(_context);
				for (int32_t h = 0; h < _context->height; h = h + _context->block_height)
				{
					for (int32_t w = 0; w < _context->width; w = w + _context->block_width)
					{
						if (_front)
						{
							sirius::library::video::transform::codec::partial::webp::compressor::entity_t input;
							input.data = process_data + (h * (_context->width << 2) + (w << 2));
							input.data_capacity = _context->block_height;
							input.data_size = _context->block_height;
							input.x = 0;
							input.y = 0;
							input.width = _context->block_width;
							input.height = _context->block_height;
							input.stride = _context->width << 2;

							sirius::library::video::transform::codec::partial::webp::compressor::entity_t bitstream;
							bitstream.data = real_compressed_buffer;
							bitstream.data_capacity = real_compressed_buffer - compressed_buffer;
							bitstream.data_capacity = compressed_buffer_size - bitstream.data_capacity;
							bitstream.data_size = 0;
							bitstream.x = input.x;
							bitstream.y = input.y;
							bitstream.width = input.width;
							bitstream.height = input.height;

							status = _real_compressor->compress(&input, &bitstream);
							if (status == sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success)
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
				_front->after_process_callback (count, px, py, pwidth, pheight, pcompressed, plength, before_encode_timestamp, after_encode_timestamp);
				memmove(reference_buffer, process_data, process_data_size);

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
					__declspec(align(32)) uint64_t result[4] = { 0 };
					//const __m256i xor_operand = _mm256_setr_epi8(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
					for (int32_t h = begin_height, y = (begin_height/ _context->mb_height); h < end_height; h = h + _context->mb_height, y++)
					{
						for (int32_t w = begin_width, x = (begin_width/ _context->mb_width); w < end_width; w = w + _context->mb_width, x++)
						{
							bool diff = false;
							for (int32_t mbh = 0; mbh < _context->mb_height; mbh++)
							{
								for (int32_t mbw = 0; mbw < _context->mb_width; mbw = mbw + (align >> 2))
								{
									int32_t ci = (h + mbh) * (_context->width << 2) + ((w + mbw) << 2);
									uint8_t * p = process_data + ci;
									uint8_t * r = reference_buffer + ci;
									const __m256i current	= _mm256_load_si256((__m256i*)p);
									const __m256i reference = _mm256_load_si256((__m256i*)r);
									const __m256i cmpeq		= _mm256_cmpeq_epi64(current, reference);
									_mm256_store_si256((__m256i*)result, cmpeq);

									for (int32_t i = 0; i < 4; i++)
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

					std::vector<connected_component_t*>::iterator ccc_iter;
					for (ccc_iter = ccl_component_vec.begin(); ccc_iter != ccl_component_vec.end(); ccc_iter++)
					{
						//(*ccc_iter)->
						int32_t cc_x		= (*ccc_iter)->left * _context->mb_width;
						int32_t cc_y		= (*ccc_iter)->top * _context->mb_height;
						int32_t cc_height	= ((*ccc_iter)->bottom - (*ccc_iter)->top + 1) * _context->mb_height;
						int32_t cc_width	= ((*ccc_iter)->right - (*ccc_iter)->left + 1) * _context->mb_width;

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

								sirius::library::video::transform::codec::partial::webp::compressor::context_t context;
								/*
								context.speed = _context->speed;
								context.max_colors = _context->max_colors;
								context.min_quality = _context->min_quality;
								context.max_quality = _context->max_quality;
								context.gamma = _context->gamma;
								context.floyd = _context->floyd;
								*/

								_real_compressor->initialize(&context);
								sirius::library::video::transform::codec::partial::webp::compressor::entity_t input;
								input.data = process_data + (ccl_info.y * (_context->width << 2) + (ccl_info.x << 2));
								input.data_capacity = ccl_info.height;
								input.data_size = ccl_info.height;
								input.x = 0;
								input.y = 0;
								input.width = ccl_info.width;
								input.height = ccl_info.height;
								input.stride = _context->width << 2;

								sirius::library::video::transform::codec::partial::webp::compressor::entity_t bitstream;
								bitstream.data = real_compressed_buffer;
								bitstream.data_capacity = real_compressed_buffer - compressed_buffer;
								bitstream.data_capacity = compressed_buffer_size - bitstream.data_capacity;
								bitstream.data_size = 0;
								bitstream.x = input.x;
								bitstream.y = input.y;
								bitstream.width = input.width;
								bitstream.height = input.height;

								status = _real_compressor->compress(&input, &bitstream);
								if (status == sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success)
								{
									px[count] = int16_t(ccl_info.x);
									py[count] = int16_t(ccl_info.y);
									pwidth[count] = int16_t(ccl_info.width);
									pheight[count] = int16_t(ccl_info.height);
									pcompressed[count] = (uint8_t*)bitstream.data;
									plength[count] = bitstream.data_size;
									real_compressed_buffer += bitstream.data_size;

									count++;
								}
								_real_compressor->release();
							}
						}
					}

					process_force_fullmode = false;
					if (_invalidate)
					{
						count = 0;
						uint8_t * real_compressed_buffer = compressed_buffer;
						_real_compressor->initialize(_context);
						for (int32_t h = 0; h < _context->height; h = h + _context->block_height)
						{
							for (int32_t w = 0; w < _context->width; w = w + _context->block_width)
							{
								if (_front)
								{
									sirius::library::video::transform::codec::partial::webp::compressor::entity_t input;
									input.data = process_data + (h * (_context->width << 2) + (w << 2));
									input.data_capacity = _context->block_height;
									input.data_size = _context->block_height;
									input.x = 0;
									input.y = 0;
									input.width = _context->block_width;
									input.height = _context->block_height;

									sirius::library::video::transform::codec::partial::webp::compressor::entity_t bitstream;
									bitstream.data = real_compressed_buffer;
									bitstream.data_capacity = real_compressed_buffer - compressed_buffer;
									bitstream.data_capacity = compressed_buffer_size - bitstream.data_capacity;
									bitstream.data_size = 0;
									bitstream.x = input.x;
									bitstream.y = input.y;
									bitstream.width = input.width;
									bitstream.height = input.height;

									status = _real_compressor->compress(&input, &bitstream);
									if (status == sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success)
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
						_invalidate = false;
					}
					else
					{
						if (count > 0)
						{
							_front->after_process_callback(count, px, py, pwidth, pheight, pcompressed, plength, before_encode_timestamp, after_encode_timestamp);
						}
						memmove(reference_buffer, process_data, process_data_size);
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
						sirius::autolock lock(&_lock);
						iobuffer = _iobuffer_queue.get_pending();
						if (iobuffer)
						{
							process_data_size = iobuffer->input.data_size;
							process_timestamp = iobuffer->input.timestamp;
							//process_data = static_cast<uint8_t*>(iobuffer->input.data);
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
			_state = sirius::library::video::transform::codec::partial::webp::compressor::state_t::compressed;
		}
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

	if (process_data)
	{
		_aligned_free(process_data);
		process_data = nullptr;
		process_data_size = 0;
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


unsigned __stdcall sirius::library::video::transform::codec::partial::webp::compressor::core::process_threaded_coordinated_encoding_callback(void * param)
{
	sirius::library::video::transform::codec::partial::webp::compressor::core::coordinated_thread_context_t * self = static_cast<sirius::library::video::transform::codec::partial::webp::compressor::core::coordinated_thread_context_t*>(param);
	self->parent->process_threaded_coordinated_encoding(self);
	return 0;
}

void sirius::library::video::transform::codec::partial::webp::compressor::core::process_threaded_coordinated_encoding(coordinated_thread_context_t * thread_ctx)
{

	while (thread_ctx->run)
	{
		if (WaitForSingleObject(thread_ctx->signal, INFINITE) == WAIT_OBJECT_0)
		{
			if (thread_ctx->run)
			{
				int32_t status = thread_ctx->real_compressor->compress(&thread_ctx->input, &thread_ctx->output);
				if (status == sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success)
				{
					(*thread_ctx->plength) = thread_ctx->output.data_size;
					memmove(thread_ctx->pcompressed, thread_ctx->output.data, (*thread_ctx->plength));
				}
			}
			::SetEvent(thread_ctx->available);
		}
	}
}

void sirius::library::video::transform::codec::partial::webp::compressor::core::process_threaded_coordinated(void)
{
	int32_t status = sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success;
	sirius::library::video::transform::codec::partial::webp::compressor::core::buffer_t * iobuffer = nullptr;

	long long before_encode_timestamp = 0;
	long long after_encode_timestamp = 0;

	int32_t		thread_count = _context->nthread;

	int32_t		align = 32;
	int32_t		reference_buffer_size = (_context->width * _context->height) << 2;
	uint8_t *	reference_buffer = static_cast<uint8_t*>(_aligned_malloc(reference_buffer_size, align));
	memset(reference_buffer, 0x00, reference_buffer_size);

	int32_t		nbytes_compressed = (_context->block_width * _context->block_height) << 1; //1024 * 512;//(1MB) (_context->block_width*_context->block_height) << 2;
	int32_t		block_count = (_context->width / _context->block_width) * (_context->height / _context->block_height);

	int32_t		pcount = _context->width * _context->height;
	int16_t *	px = new int16_t[pcount];
	int16_t *	py = new int16_t[pcount];
	int16_t *	pwidth = new int16_t[pcount];
	int16_t *	pheight = new int16_t[pcount];
	uint8_t **	pcompressed = new uint8_t*[pcount];
	int32_t *	plength = new int32_t[pcount];

	for (int32_t index = 0; index < pcount; index++)
	{
		px[index] = 0;
		py[index] = 0;
		pwidth[index] = 0;
		pheight[index] = 0;
		pcompressed[index] = nullptr;
		plength[index] = 0;
	}

	int32_t		process_data_size = reference_buffer_size;
	uint8_t *	process_data = static_cast<uint8_t*>(_aligned_malloc(process_data_size, align));
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

		thread_ctx[tindex]->compressed_buffer_size = (_context->block_width * _context->block_height) << 1;
		thread_ctx[tindex]->compressed_buffer = static_cast<char*>(malloc(thread_ctx[tindex]->compressed_buffer_size));

		thread_ctx[tindex]->output.data = thread_ctx[tindex]->compressed_buffer;
		thread_ctx[tindex]->output.data_capacity = thread_ctx[tindex]->compressed_buffer_size;
		thread_ctx[tindex]->pcompressed = nullptr;
		thread_ctx[tindex]->plength = nullptr;

		thread_ctx[tindex]->run = TRUE;
		thread_ctx[tindex]->thread = (HANDLE)::_beginthreadex(NULL, 0, sirius::library::video::transform::codec::partial::webp::compressor::core::process_threaded_coordinated_encoding_callback, thread_ctx[tindex], 0, &thrdaddr);
		thread_ctx[tindex]->signal = ::CreateEventA(NULL, FALSE, FALSE, NULL);
		thread_ctx[tindex]->available = ::CreateEventA(NULL, FALSE, FALSE, NULL);
		::SetEvent(thread_ctx[tindex]->available);
		thread_ctx[tindex]->parent = this;

		thread_ctx[tindex]->real_compressor = new sirius::library::video::transform::codec::libwebp::compressor(_front);
	}

	while (_run)
	{
		if (::WaitForSingleObject(_event, INFINITE) == WAIT_OBJECT_0)
		{
			_state = sirius::library::video::transform::codec::partial::webp::compressor::state_t::compressing;

			{
				sirius::autolock lock(&_lock);
				iobuffer = _iobuffer_queue.get_pending();
				if (!iobuffer)
				{
					if (!_invalidate)
					{
						_state = sirius::library::video::transform::codec::partial::webp::compressor::state_t::compressed;
						::Sleep(10);
						continue;
					}
					process_data_size = 0;
				}
				else
				{
					process_data_size = iobuffer->input.data_size;
					process_timestamp = iobuffer->input.timestamp;
					//process_data = static_cast<uint8_t*>(iobuffer->input.data);
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
										thread_ctx[tindex]->input.data = process_data + (h * (_context->width << 2) + (w << 2));
										thread_ctx[tindex]->input.data_capacity = _context->block_height;
										thread_ctx[tindex]->input.data_size = _context->block_height;
										thread_ctx[tindex]->input.x = 0;
										thread_ctx[tindex]->input.y = 0;
										thread_ctx[tindex]->input.width = _context->block_width;
										thread_ctx[tindex]->input.height = _context->block_height;
										thread_ctx[tindex]->input.stride = _context->width << 2;

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
											delete [] pcompressed[count];
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
				before_encode_timestamp = process_timestamp;
				int32_t count = 0;
				int32_t index = 0;
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
										thread_ctx[tindex]->input.data = process_data + (h * (_context->width << 2) + (w << 2));
										thread_ctx[tindex]->input.data_capacity = _context->block_height;
										thread_ctx[tindex]->input.data_size = _context->block_height;
										thread_ctx[tindex]->input.x = 0;
										thread_ctx[tindex]->input.y = 0;
										thread_ctx[tindex]->input.width = _context->block_width;
										thread_ctx[tindex]->input.height = _context->block_height;
										thread_ctx[tindex]->input.stride = _context->width << 2;

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

				_invalidate = false;
			}
			else
			{
				while (process_data_size>0)
				{
					before_encode_timestamp = process_timestamp;
					int32_t count = 0;
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
					__declspec(align(32)) uint64_t result[4] = { 0 };
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
									const __m256i cmpeq = _mm256_cmpeq_epi64(current, reference);
									_mm256_store_si256((__m256i*)result, cmpeq);

									for (int32_t i = 0; i < 4; i++)
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

					std::vector<coordinated_thread_context_t*> thread_contexts;
					std::vector<connected_component_t*>::iterator ccc_iter;
					for (ccc_iter = ccl_component_vec.begin(); ccc_iter != ccl_component_vec.end(); ccc_iter++)
					{
						connected_component_t * cc = (*ccc_iter);
						int32_t cc_x		= cc->left * _context->mb_width;
						int32_t cc_y		= cc->top * _context->mb_height;
						int32_t cc_height	= (cc->bottom - cc->top + 1) * _context->mb_height;
						int32_t cc_width	= (cc->right - cc->left + 1) * _context->mb_width;

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
										ccl_info_t ccl_ctx;
										ccl_ctx.x = cc_x + (x * _context->block_width);
										ccl_ctx.y = cc_y + (y * _context->block_height);
										ccl_ctx.width = _context->block_width;
										ccl_ctx.height = _context->block_height;
										ccl_infos.push_back(ccl_ctx);
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
											ccl_info_t ccl_ctx;
											ccl_ctx.x = cc_x + (x * cc_width2);
											ccl_ctx.y = cc_y + (quotient_y * _context->block_height);
											ccl_ctx.width = cc_width2;
											ccl_ctx.height = remainder_y;
											ccl_infos.push_back(ccl_ctx);
										}

										int32_t cc_width3 = cc_width - cc_width2 * quotient;
										if (cc_width3> 0)
										{
											ccl_info_t ccl_ctx;
											ccl_ctx.x = cc_x + (quotient * cc_width2);
											ccl_ctx.y = cc_y + (quotient_y * _context->block_height);
											ccl_ctx.width = cc_width3;
											ccl_ctx.height = remainder_y;
											ccl_infos.push_back(ccl_ctx);
										}
									}
									else
									{
										ccl_info_t ccl_ctx;
										ccl_ctx.x = cc_x;
										ccl_ctx.y = cc_y + (quotient_y * _context->block_height);
										ccl_ctx.width = quotient_x * _context->block_width;
										ccl_ctx.height = remainder_y;
										ccl_infos.push_back(ccl_ctx);
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
											ccl_info_t ccl_ctx;
											ccl_ctx.x = cc_x + (quotient_x * _context->block_width);
											ccl_ctx.y = cc_y + (y * cc_height2);
											ccl_ctx.width = remainder_x;
											ccl_ctx.height = cc_height2;
											ccl_infos.push_back(ccl_ctx);
										}

										int32_t cc_height3 = ((quotient_y * _context->block_height) + remainder_y) - cc_height2 * quotient;
										if (cc_height3 > 0)
										{
											ccl_info_t ccl_ctx;
											ccl_ctx.x = cc_x + (quotient_x * _context->block_width);
											ccl_ctx.y = cc_y + (quotient * cc_height2);
											ccl_ctx.width = remainder_x;
											ccl_ctx.height = cc_height3;
											ccl_infos.push_back(ccl_ctx);
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
									BOOL found_thread_ctx = FALSE;
									for (int32_t tindex = 0; tindex < thread_count; tindex++)
									{
										if (::WaitForSingleObject(thread_ctx[tindex]->available, 0) == WAIT_OBJECT_0)
										{
											thread_ctx[tindex]->input.data = process_data + (ccl_info.y * (_context->width << 2) + (ccl_info.x << 2));
											thread_ctx[tindex]->input.data_capacity = ccl_info.height;
											thread_ctx[tindex]->input.data_size = ccl_info.height;
											thread_ctx[tindex]->input.x = 0;
											thread_ctx[tindex]->input.y = 0;
											thread_ctx[tindex]->input.width = ccl_info.width;
											thread_ctx[tindex]->input.height = ccl_info.height;
											thread_ctx[tindex]->input.stride = _context->width << 2;

											thread_ctx[tindex]->output.data_size = 0;
											thread_ctx[tindex]->output.x = thread_ctx[tindex]->input.x;
											thread_ctx[tindex]->output.y = thread_ctx[tindex]->input.y;
											thread_ctx[tindex]->output.width = thread_ctx[tindex]->input.width;
											thread_ctx[tindex]->output.height = thread_ctx[tindex]->input.height;

											px[count] = ccl_info.x;
											py[count] = ccl_info.y;
											pwidth[count] = ccl_info.width;
											pheight[count] = ccl_info.height;
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
												thread_ctx[tindex]->input.data = process_data + (h * (_context->width << 2) + (w << 2));
												thread_ctx[tindex]->input.data_capacity = _context->block_height;
												thread_ctx[tindex]->input.data_size = _context->block_height;
												thread_ctx[tindex]->input.x = 0;
												thread_ctx[tindex]->input.y = 0;
												thread_ctx[tindex]->input.width = _context->block_width;
												thread_ctx[tindex]->input.height = _context->block_height;
												thread_ctx[tindex]->input.stride = _context->width << 2;

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

						_invalidate = false;
					}
					else
					{
						if (count > 0)
						{
							_front->after_process_callback(count, px, py, pwidth, pheight, pcompressed, plength, before_encode_timestamp, after_encode_timestamp);
						}
						memmove(reference_buffer, process_data, process_data_size);
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
						sirius::autolock lock(&_lock);
						iobuffer = _iobuffer_queue.get_pending();
						if (iobuffer)
						{
							process_data_size = iobuffer->input.data_size;
							process_timestamp = iobuffer->input.timestamp;
							//process_data = static_cast<uint8_t*>(iobuffer->input.data);
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
			_state = sirius::library::video::transform::codec::partial::webp::compressor::state_t::compressed;
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
		thread_ctx[tindex]->pcompressed = nullptr;
		thread_ctx[tindex]->plength = nullptr;

		free(thread_ctx[tindex]);
		thread_ctx[tindex] = nullptr;
	}
	free(thread_ctx);
	thread_ctx = nullptr;

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
				delete [] pcompressed[index];
				pcompressed[index] = nullptr;
			}
		}
		delete[] pcompressed;
		pcompressed = nullptr;
	}

	if (process_data)
	{
		_aligned_free(process_data);
		process_data = nullptr;
		process_data_size = 0;
	}

	if (reference_buffer)
	{
		_aligned_free(reference_buffer);
		reference_buffer = nullptr;
		reference_buffer_size = 0;
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

void sirius::library::video::transform::codec::partial::webp::compressor::core::connect_component(unsigned short * pseudo_stack, 
																										 std::map<uint64_t, uint32_t> * bfgs, connected_component_t * cc,
																										 uint32_t width, uint32_t height, uint32_t x, uint32_t y)
{
	pseudo_stack[0] = x;
	pseudo_stack[1] = y;
	pseudo_stack[2] = UNLABELED;
	int32_t SP		= 3;

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
		//connect_component(pseudo_stack, bfgs, cc, width, height, x - 1, y); //left
	}

RETURN1:
	if (X < width - 1)
	{
		pseudo_stack[SP] = X + 1;
		pseudo_stack[SP + 1] = Y;
		pseudo_stack[SP + 2] = 2;
		SP += 3;
		goto START;
		//connect_component(pseudo_stack, bfgs, cc, width, height, x + 1, y); //right
	}

RETURN2:
	if (Y > 0)
	{
		pseudo_stack[SP] = X;
		pseudo_stack[SP + 1] = Y - 1;
		pseudo_stack[SP + 2] = 3;
		SP += 3;
		goto START;
		//connect_component(pseudo_stack, bfgs, cc, width, height, x, y - 1); //upper
	}

RETURN3:
	if (Y < height - 1)
	{
		pseudo_stack[SP] = X;
		pseudo_stack[SP + 1] = Y + 1;
		pseudo_stack[SP + 2] = 4;
		SP += 3;
		goto START;
		//connect_component(pseudo_stack, bfgs, cc, width, height, x, y + 1); //lower
	}

RETURN4:

	RETURN;
}

void sirius::library::video::transform::codec::partial::webp::compressor::core::process_indexed(void)
{
	int32_t status = sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success;
	sirius::library::video::transform::codec::partial::webp::compressor::core::buffer_t * iobuffer = nullptr;

	long long before_encode_timestamp = 0;
	long long after_encode_timestamp = 0;

	int32_t	compressed_buffer_size = 1024 * 1024 * 2;
	uint8_t * compressed_buffer = static_cast<uint8_t*>(malloc(compressed_buffer_size));
	memset(compressed_buffer, 0x00, compressed_buffer_size);

	int32_t		simd_align = 32;
	int32_t		reference_buffer_size = (_context->width * _context->height) << 2;
	uint8_t *	reference_buffer = static_cast<uint8_t*>(_aligned_malloc(reference_buffer_size, simd_align));
	memset(reference_buffer, 0x00, reference_buffer_size);

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

	int32_t		process_data_size = reference_buffer_size;
	uint8_t *	process_data = static_cast<uint8_t*>(_aligned_malloc(process_data_size, simd_align));
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
			_state = sirius::library::video::transform::codec::partial::webp::compressor::state_t::compressing;

			{
				sirius::autolock lock(&_lock);
				iobuffer = _iobuffer_queue.get_pending();
				if (!iobuffer)
				{
					if (!_invalidate)
					{
						_state = sirius::library::video::transform::codec::partial::webp::compressor::state_t::compressed;
						::Sleep(10);
						continue;
					}
					process_data_size = 0;
				}
				else
				{
					process_data_size = iobuffer->input.data_size;
					process_timestamp = iobuffer->input.timestamp;
					//process_data = static_cast<uint8_t*>(iobuffer->input.data);
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
						if (_front)
						{
							sirius::library::video::transform::codec::partial::webp::compressor::entity_t input;
							input.data = process_data + (h * (_context->width << 2) + (w << 2));
							input.data_size = nbytes_compressed;
							input.x = 0;
							input.y = 0;
							input.width = _context->block_width;
							input.height = _context->block_height;

							sirius::library::video::transform::codec::partial::webp::compressor::entity_t bitstream;
							bitstream.data = real_compressed_buffer;
							bitstream.data_capacity = real_compressed_buffer - compressed_buffer;
							bitstream.data_capacity = compressed_buffer_size - bitstream.data_capacity;
							bitstream.data_size = 0;
							bitstream.x = input.x;
							bitstream.y = input.y;
							bitstream.width = input.width;
							bitstream.height = input.height;

							status = _real_compressor->compress(&input, &bitstream);
							if (status == sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success)
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
							{
								bool	diff = false;
								__declspec(align(32)) uint64_t simd_result[4] = { 0 };
								for (int32_t bh = 0; bh < _context->block_height; bh++)
								{
									for (int32_t bw = 0; bw < _context->block_width; bw = bw + (simd_align >> 2))
									{
										int32_t ci = (h + bh) * (_context->width << 2) + (w << 2) + (bw << 2);
										uint8_t * p = process_data + ci;
										uint8_t * r = reference_buffer + ci;

										const __m256i current = _mm256_load_si256((__m256i*)p);
										const __m256i reference = _mm256_load_si256((__m256i*)r);
										const __m256i cmpeq = _mm256_cmpeq_epi64(current, reference);
										_mm256_store_si256((__m256i*)simd_result, cmpeq);
										for (int32_t index = 0; index < 4; index++)
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
									if (_front)
									{
										sirius::library::video::transform::codec::partial::webp::compressor::entity_t input;
										input.data = process_data + (h * (_context->width << 2) + (w << 2));
										input.data_capacity = _context->block_height;
										input.data_size = _context->block_height;
										input.x = 0;
										input.y = 0;
										input.width = _context->block_width;
										input.height = _context->block_height;

										sirius::library::video::transform::codec::partial::webp::compressor::entity_t bitstream;
										bitstream.data = real_compressed_buffer;
										bitstream.data_capacity = real_compressed_buffer - compressed_buffer;
										bitstream.data_capacity = compressed_buffer_size - bitstream.data_capacity;
										bitstream.data_size = 0;
										bitstream.x = input.x;
										bitstream.y = input.y;
										bitstream.width = input.width;
										bitstream.height = input.height;

										status = _real_compressor->compress(&input, &bitstream);
										if (status == sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success)
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
					memmove(reference_buffer, process_data, process_data_size);

					{
						sirius::autolock lock(&_lock);
						iobuffer = _iobuffer_queue.get_pending();
						if (iobuffer)
						{
							process_data_size = iobuffer->input.data_size;
							process_timestamp = iobuffer->input.timestamp;
							//process_data = static_cast<uint8_t*>(iobuffer->input.data);
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
			_state = sirius::library::video::transform::codec::partial::webp::compressor::state_t::compressed;
		}
	}

	_real_compressor->release();

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

	if (process_data)
	{
		_aligned_free(process_data);
		process_data = nullptr;
		process_data_size = 0;
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

unsigned __stdcall sirius::library::video::transform::codec::partial::webp::compressor::core::process_threaded_indexed_encoding_callback(void * param)
{
	sirius::library::video::transform::codec::partial::webp::compressor::core::indexed_thread_context_t * self = static_cast<sirius::library::video::transform::codec::partial::webp::compressor::core::indexed_thread_context_t*>(param);
	self->parent->process_threaded_indexed_encoding(self);
	return 0;
}

void sirius::library::video::transform::codec::partial::webp::compressor::core::process_threaded_indexed_encoding(indexed_thread_context_t * thread_ctx)
{

	while (thread_ctx->run)
	{
		if (WaitForSingleObject(thread_ctx->signal, INFINITE) == WAIT_OBJECT_0)
		{
			if (thread_ctx->run)
			{
				int32_t status = thread_ctx->real_compressor->compress(&thread_ctx->input, &thread_ctx->output);
				if (status == sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success)
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

void sirius::library::video::transform::codec::partial::webp::compressor::core::process_threaded_indexed(void)
{
	int32_t status = sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success;
	sirius::library::video::transform::codec::partial::webp::compressor::core::buffer_t * iobuffer = nullptr;

	long long before_encode_timestamp = 0;
	long long after_encode_timestamp = 0;

	int32_t		simd_align = 32;
	int32_t		reference_buffer_size = (_context->width * _context->height) << 2;
	uint8_t *	reference_buffer = static_cast<uint8_t*>(_aligned_malloc(reference_buffer_size, simd_align));
	memset(reference_buffer, 0x00, reference_buffer_size);

	int32_t		nbytes_compressed = (_context->block_width*_context->block_height) << 2;
	int32_t		block_count = (_context->width / _context->block_width) * (_context->height / _context->block_height);

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
	}

	int32_t		process_data_size = reference_buffer_size;
	uint8_t *	process_data = static_cast<uint8_t*>(_aligned_malloc(process_data_size, simd_align));
	int32_t		process_x = 0;
	int32_t		process_y = 0;
	int32_t		process_width = 0;
	int32_t		process_height = 0;
	long long	process_timestamp = 0;
	bool		process_force_fullmode = false;
	bool		process_compress_first_time = true;


	int32_t		thread_count = _context->nthread;
	indexed_thread_context_t ** thread_ctx = static_cast<indexed_thread_context_t**>(malloc(sizeof(indexed_thread_context_t*)*thread_count));
	for (int32_t tindex = 0; tindex < thread_count; tindex++)
	{
		uint32_t thrdaddr = 0;
		thread_ctx[tindex] = static_cast<indexed_thread_context_t*>(malloc(sizeof(indexed_thread_context_t)));
		thread_ctx[tindex]->index = 0;

		thread_ctx[tindex]->compressed_buffer_size = (_context->block_width * _context->block_height) << 1;
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
		thread_ctx[tindex]->thread = (HANDLE)::_beginthreadex(NULL, 0, sirius::library::video::transform::codec::partial::webp::compressor::core::process_threaded_indexed_encoding_callback, thread_ctx[tindex], 0, &thrdaddr);
		thread_ctx[tindex]->signal = ::CreateEventA(NULL, FALSE, FALSE, NULL);
		thread_ctx[tindex]->available = ::CreateEventA(NULL, FALSE, FALSE, NULL);
		::SetEvent(thread_ctx[tindex]->available);
		thread_ctx[tindex]->parent = this;

		thread_ctx[tindex]->real_compressor = new sirius::library::video::transform::codec::libwebp::compressor(_front);
		thread_ctx[tindex]->real_compressor->initialize(_context);
	}

	while (_run)
	{
		if (::WaitForSingleObject(_event, INFINITE) == WAIT_OBJECT_0)
		{
			_state = sirius::library::video::transform::codec::partial::webp::compressor::state_t::compressing;

			{
				sirius ::autolock lock(&_lock);
				iobuffer = _iobuffer_queue.get_pending();
				if (!iobuffer)
				{
					if (!_invalidate)
					{
						_state = sirius::library::video::transform::codec::partial::webp::compressor::state_t::compressed;
						::Sleep(10);
						continue;
					}
					process_data_size = 0;
				}
				else
				{
					process_data_size = iobuffer->input.data_size;
					process_timestamp = iobuffer->input.timestamp;
					//process_data = static_cast<uint8_t*>(iobuffer->input.data);
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
						if (_front)
						{
							while (TRUE)
							{
								BOOL found_thread_ctx = FALSE;
								for (int32_t tindex = 0; tindex < thread_count; tindex++)
								{
									if (::WaitForSingleObject(thread_ctx[tindex]->available, 0) == WAIT_OBJECT_0)
									{
										thread_ctx[tindex]->input.data = process_data + (h * (_context->width << 2) + (w << 2));
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
							__declspec(align(32)) uint64_t simd_result[4] = { 0 };
							for (int32_t bh = 0; bh < _context->block_height; bh++)
							{
								for (int32_t bw = 0; bw < _context->block_width; bw = bw + (simd_align >> 2))
								{
									int32_t ci = (h + bh) * (_context->width << 2) + (w << 2) + (bw << 2);
									uint8_t * p = process_data + ci;
									uint8_t * r = reference_buffer + ci;
									const __m256i current = _mm256_load_si256((__m256i*)p);
									const __m256i reference = _mm256_load_si256((__m256i*)r);
									const __m256i cmpeq = _mm256_cmpeq_epi64(current, reference);
									_mm256_store_si256((__m256i*)simd_result, cmpeq);
									for (int32_t index = 0; index < 4; index++)
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
								if (_front)
								{
									while (TRUE)
									{
										BOOL found_thread_ctx = FALSE;
										for (int32_t tindex = 0; tindex < thread_count; tindex++)
										{
											if (::WaitForSingleObject(thread_ctx[tindex]->available, 0) == WAIT_OBJECT_0)
											{
												thread_ctx[tindex]->input.data = process_data + (h * (_context->width << 2) + (w << 2));
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
					memmove(reference_buffer, process_data, process_data_size);

					for (iter = thread_contexts.begin(); iter != thread_contexts.end(); iter++)
						::SetEvent((*iter)->available);

					thread_contexts.clear();
					{
						sirius::autolock lock(&_lock);
						iobuffer = _iobuffer_queue.get_pending();
						if (iobuffer)
						{
							process_data_size = iobuffer->input.data_size;
							process_timestamp = iobuffer->input.timestamp;
							//process_data = static_cast<uint8_t*>(iobuffer->input.data);
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
			_state = sirius::library::video::transform::codec::partial::webp::compressor::state_t::compressed;
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

	if (process_data)
	{
		_aligned_free(process_data);
		process_data = nullptr;
		process_data_size = 0;
	}

	if (reference_buffer)
	{
		_aligned_free(reference_buffer);
		reference_buffer = nullptr;
		reference_buffer_size = 0;
	}
}

int32_t sirius::library::video::transform::codec::partial::webp::compressor::core::allocate_io_buffers(void)
{
	int32_t status = sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success;
	_iobuffer_queue.initialize(_iobuffer, _context->nbuffer);

	for (int32_t i = 0; i < _context->nbuffer; i++)
	{
		_iobuffer[i].input.data_capacity = (_context->width * _context->height) << 2;
		_iobuffer[i].input.data_size = 0;
		_iobuffer[i].input.data = static_cast<uint8_t*>(malloc(_iobuffer[i].input.data_capacity));
	}

	return sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::partial::webp::compressor::core::release_io_buffers(void)
{
	for (int32_t i = 0; i < _context->nbuffer; i++)
	{
		if (_iobuffer[i].input.data)
			free(_iobuffer[i].input.data);
		_iobuffer[i].input.data = nullptr;

		/*
		if (_iobuffer[i].output.data)
			free(_iobuffer[i].output.data);
		_iobuffer[i].output.data = nullptr;
		*/
	}

	_iobuffer_queue.release();
	return sirius::library::video::transform::codec::partial::webp::compressor::err_code_t::success;
}

void sirius::library::video::transform::codec::partial::webp::compressor::core::copy(sirius::library::video::transform::codec::partial::webp::compressor::entity_t * input, sirius::library::video::transform::codec::partial::webp::compressor::core::buffer_t * iobuffer)
{

	iobuffer->input.timestamp = input->timestamp;
	iobuffer->input.data_size = input->data_size;
	memmove((uint8_t*)iobuffer->input.data, input->data, iobuffer->input.data_size);
	iobuffer->input.x = input->x;
	iobuffer->input.y = input->y;
	iobuffer->input.width = input->width;
	iobuffer->input.height = input->height;

	/*
	_prev_x = input->x;
	_prev_y = input->y;
	_prev_width = input->width;
	_prev_height = input->height;
	*/

}