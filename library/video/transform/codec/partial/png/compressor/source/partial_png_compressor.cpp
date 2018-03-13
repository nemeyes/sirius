#include "sirius_partial_png_compressor.h"
#include "partial_png_compressor.h"

#include <sirius_image_creator.h>
#include <sirius_stringhelper.h>
#include <sirius_locks.h>

#include <simd_image_processor.h>

#include <vector>
#include <assert.h>

#include <Simd\SimdLib.h>


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
	
	if(self->_context->indexed_video && self->_context->partial_post)
		self->process_psend_indexed();
	else if(self->_context->indexed_video && !self->_context->partial_post)
		self->process_bsend_indexed();
	else
		self->process_coordinates();
	return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
}

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

												/*
												if (bb->mb[macro_block_index - MACRO_BLOCK_WIDTH])	//vertical matching
												{
												vmatched = true;
												break;
												}
												*/
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
					/*
					if (_invalidate)
					{
					_front->after_process_callback(block_count, cached_index, cached_compressed, cached_length, before_encode_timestamp, after_encode_timestamp);
					_invalidate = false;
					}
					else
					{
					if (count>0)
					_front->after_process_callback(count, pindex, pcompressed, plength, before_encode_timestamp, after_encode_timestamp);
					}
					*/

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

	int32_t		gray_buffer_size = ((_context->width) * (_context->height));
	uint8_t *	gray_buffer = static_cast<uint8_t*>(malloc(gray_buffer_size));
	memset(gray_buffer, 0x00, gray_buffer_size);

	int32_t		resized_buffer_size = ((_context->width >> 2) * (_context->height >> 2));
	uint8_t *	resized_buffer = static_cast<uint8_t*>(malloc(resized_buffer_size));
	memset(resized_buffer, 0x00, resized_buffer_size);

	uint8_t *	reference_buffer = static_cast<uint8_t*>(malloc(resized_buffer_size));
	memset(reference_buffer, 0x00, resized_buffer_size);

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
	uint8_t *	process_data = static_cast<uint8_t*>(malloc(process_data_capacity));
	int32_t		process_x = 0;
	int32_t		process_y = 0;
	int32_t		process_width = 0;
	int32_t		process_height = 0;
	long long	process_timestamp = 0;
	bool		process_force_fullmode = false;
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
					//char msg[MAX_PATH] = { 0 };
					//_snprintf_s(msg, MAX_PATH, "ob_index=%d,  x=%d, y=%d, width=%d, height=%d \n", ob_index, process_x, process_y, process_width, process_height);
					//::OutputDebugStringA(msg);
					//ob_index++;

				}
			}

			//if (_invalidate && _context->binvalidate && (process_data_size == 0))
			//{
			//	_front->after_process_callback(block_count, cached_index, cached_compressed, cached_length, before_encode_timestamp, after_encode_timestamp);
			//	_invalidate = false;
			//}
			//else
			{
				while (process_data_size>0)
				{
					before_encode_timestamp = process_timestamp;

					SimdBgraToGray(process_data, _context->width, _context->height, _context->width << 2, gray_buffer, _context->width);
					SimdResizeBilinear(gray_buffer, _context->width, _context->height, _context->width, resized_buffer, _context->width >> 2, _context->height >> 2, _context->width >> 2, 1);
					if (process_force_fullmode || (((process_width - process_x) == _context->width) && ((process_height - process_y) == _context->height)))
					{
						int32_t count = 0;
						int32_t index = 0;
						uint8_t * real_compressed_buffer = compressed_buffer;

						for (int32_t h = 0, h2 = 0; h < _context->height; h = h + _context->block_height, h2 = h2 + (_context->block_height >> 2))
						{
							for (int32_t w = 0, w2 = 0; w < _context->width; w = w + _context->block_width, w2 = w2 + (_context->block_width >> 2))
							{
								//bool bdiff = SIMD_EVALUATOR::squared_eval(true, resized_buffer + (h2 * (_context->width >> 2) + w2), _context->width >> 2, reference_buffer + (h2 * (_context->width >> 2) + w2), _context->width >> 2, _context->block_width >> 2, _context->block_height >> 2);
								uint64_t sum = 0;
								SimdSquaredDifferenceSum(resized_buffer + (h2 * (_context->width >> 2) + w2), _context->width >> 2, reference_buffer + (h2 * (_context->width >> 2) + w2), _context->width >> 2, _context->block_width >> 2, _context->block_height >> 2, &sum);
								if (sum>0)
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

											cached_index[index] = index;
											cached_length[index] = plength[count];
											memmove(cached_compressed[index], pcompressed[count], cached_length[index]);

											count++;
										}
									}
								}
								index++;
							}
						}
						_front->after_process_callback(count, pindex, pcompressed, plength, before_encode_timestamp, after_encode_timestamp);
						process_force_fullmode = false;
					}
					else
					{
						int32_t count = 0;
						int32_t index = 0;
						uint8_t * real_compressed_buffer = compressed_buffer;

						int32_t begin_height	= (process_y / _context->block_height) * _context->block_height;
						int32_t end_height		= ((process_y + process_height) / _context->block_height) * _context->block_height;
						
						if (((process_y + process_height) % _context->block_height) > 0)
							end_height += _context->block_height;

						int32_t begin_width = (process_x / _context->block_width) * _context->block_width;
						int32_t end_width = ((process_x + process_width) / _context->block_width) * _context->block_width;
						
						if(((process_x + process_width) % _context->block_width) > 0)
							end_width += _context->block_width;

						for (int32_t h = 0, h2 = 0; h < _context->height; h = h + _context->block_height, h2 = h2 + (_context->block_height >> 2))
						{
							for (int32_t w = 0, w2 = 0; w < _context->width; w = w + _context->block_width, w2 = w2 + (_context->block_width >> 2))
							{
								if (h >= begin_height && h < end_height && w >= begin_width && w < end_width)
								{

									//bool bdiff = SIMD_EVALUATOR::squared_eval(true, resized_buffer + (h2 * (_context->width >> 2) + w2), _context->width >> 2, reference_buffer + (h2 * (_context->width >> 2) + w2), _context->width >> 2, _context->block_width >> 2, _context->block_height >> 2);
									uint64_t sum = 0;
									SimdSquaredDifferenceSum(resized_buffer + (h2 * (_context->width >> 2) + w2), _context->width >> 2, reference_buffer + (h2 * (_context->width >> 2) + w2), _context->width >> 2, _context->block_width >> 2, _context->block_height >> 2, &sum);
									if (sum>0)
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

												cached_index[index] = index;
												cached_length[index] = bitstream.data_size;
												memmove(cached_compressed[index], pcompressed[count], cached_length[index]);

												count++;
											}
										}
									}
								}
								index++;
							}
						}

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
					}


					memmove(reference_buffer, resized_buffer, resized_buffer_size);
					
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
							//char msg[MAX_PATH] = { 0 };
							//_snprintf_s(msg, MAX_PATH, "ob_index=%d,  x=%d, y=%d, width=%d, height=%d \n", ob_index, process_x, process_y, process_width, process_height);
							//::OutputDebugStringA(msg);
							//ob_index++;

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

	int32_t		gray_buffer_size = ((_context->width) * (_context->height));
	uint8_t *	gray_buffer = static_cast<uint8_t*>(malloc(gray_buffer_size));
	memset(gray_buffer, 0x00, gray_buffer_size);

	int32_t		resized_buffer_size = ((_context->width >> 2) * (_context->height >> 2));
	uint8_t *	resized_buffer = static_cast<uint8_t*>(malloc(resized_buffer_size));
	memset(resized_buffer, 0x00, resized_buffer_size);

	uint8_t *	reference_buffer = static_cast<uint8_t*>(malloc(resized_buffer_size));
	memset(reference_buffer, 0x00, resized_buffer_size);

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

					char msg[MAX_PATH] = { 0 };
					_snprintf_s(msg, MAX_PATH, "ob_index=%d,  x=%d, y=%d, width=%d, height=%d \n", ob_index, process_x, process_y, process_width, process_height);
					::OutputDebugStringA(msg);
					ob_index++;

				}
			}

			//if (_invalidate && _context->binvalidate && (process_data_size == 0))
			//{
			//	_front->after_process_callback(block_count, cached_index, cached_compressed, cached_length, before_encode_timestamp, after_encode_timestamp);
			//	_invalidate = false;
			//}
			//else
			{
				while (process_data_size>0)
				{
					before_encode_timestamp = process_timestamp;

					SimdBgraToGray(process_data, _context->width, _context->height, _context->width << 2, gray_buffer, _context->width);
					SimdResizeBilinear(gray_buffer, _context->width, _context->height, _context->width, resized_buffer, _context->width >> 2, _context->height >> 2, _context->width >> 2, 1);
					if (((process_width - process_x) == _context->width) && ((process_height - process_y) == _context->height))
					{
						int32_t count = 0;
						int32_t index = 0;
						uint8_t * real_compressed_buffer = compressed_buffer;

						for (int32_t h = 0, h2 = 0; h < _context->height; h = h + _context->block_height, h2 = h2 + (_context->block_height >> 2))
						{
							for (int32_t w = 0, w2 = 0; w < _context->width; w = w + _context->block_width, w2 = w2 + (_context->block_width >> 2))
							{
								//bool bdiff = SIMD_EVALUATOR::squared_eval(true, resized_buffer + (h2 * (_context->width >> 2) + w2), _context->width >> 2, reference_buffer + (h2 * (_context->width >> 2) + w2), _context->width >> 2, _context->block_width >> 2, _context->block_height >> 2);
								uint64_t sum = 0;
								SimdSquaredDifferenceSum(resized_buffer + (h2 * (_context->width >> 2) + w2), _context->width >> 2, reference_buffer + (h2 * (_context->width >> 2) + w2), _context->width >> 2, _context->block_width >> 2, _context->block_height >> 2, &sum);
								if (sum>0)
								{
									uint8_t ** rows = static_cast<uint8_t**>(malloc(_context->block_height * sizeof(uint8_t*)));
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
											cached_length[index] = plength[count];
											memmove(cached_compressed[index], pcompressed[count], cached_length[index]);

											count++;
										}
									}
									free(rows);
									rows = nullptr;
								}
								index++;
							}
						}
					}
					else
					{
						int32_t count = 0;
						int32_t index = 0;
						uint8_t * real_compressed_buffer = compressed_buffer;

						int32_t begin_height = (process_y / _context->block_height) * _context->block_height;
						int32_t end_height = ((process_y + process_height) / _context->block_height) * _context->block_height;

						if (((process_y + process_height) % _context->block_height) > 0)
							end_height += _context->block_height;

						int32_t begin_width = (process_x / _context->block_width) * _context->block_width;
						int32_t end_width = ((process_x + process_width) / _context->block_width) * _context->block_width;

						if (((process_x + process_width) % _context->block_width) > 0)
							end_width += _context->block_width;

						for (int32_t h = 0; h < _context->height; h = h + _context->block_height)
						{
							for (int32_t w = 0; w < _context->width; w = w + _context->block_width)
							{
								if (h >= begin_height && h < end_height && w >= begin_width && w < end_width)
								{
									uint8_t ** rows = static_cast<uint8_t**>(malloc(_context->block_height * sizeof(uint8_t*)));
									for (int32_t bh = 0, row_index = 0; bh < _context->block_height; bh++, row_index++)
									{
										int32_t src_index = (h + bh) * (_context->width << 2) + (w << 2);
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
									free(rows);
									rows = nullptr;
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
					}


					memmove(reference_buffer, resized_buffer, resized_buffer_size);

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

	if (compressed_buffer)
		free(compressed_buffer);
	compressed_buffer = nullptr;
	compressed_buffer_size = 0;
}

/*
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

void sirius::library::video::transform::codec::partial::png::compressor::core::copy(sirius::library::video::transform::codec::partial::png::compressor::entity_t * input, sirius::library::video::transform::codec::partial::png::compressor::core::buffer_t * iobuffer)
{
	
	iobuffer->input.timestamp = input->timestamp;
	iobuffer->input.data_size = input->data_size;
	memmove((uint8_t*)iobuffer->input.data, input->data, iobuffer->input.data_size);
	iobuffer->input.x = input->x;
	iobuffer->input.y = input->y;
	iobuffer->input.width = input->width;
	iobuffer->input.height = input->height;

	if ((input->width != _prev_width) || (input->x != _prev_x) || (input->height != _prev_height) || (input->y != _prev_y))
		iobuffer->input.force_fullmode = true;

	_prev_x = input->x;
	_prev_y = input->y;
	_prev_width = input->width;
	_prev_height = input->height;


}