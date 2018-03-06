#include "sirius_partial_png_compressor.h"
#include "partial_png_compressor.h"

#include <sirius_image_creator.h>
#include <sirius_stringhelper.h>
#include <sirius_locks.h>

#include <vector>
#include <assert.h>

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

int32_t sirius::library::video::transform::codec::partial::png::compressor::core::compress(sirius::library::video::transform::codec::partial::png::compressor::entity_t * input)
{
	sirius::library::video::transform::codec::partial::png::compressor::core::buffer_t * iobuffer = nullptr;
	
	{
		sirius::autolock lock(&_cs);
		iobuffer = _iobuffer_queue.get_available();
		if (!iobuffer)
		{
			while (true)
			{
				iobuffer = _iobuffer_queue.get_pending();
				if (!iobuffer)
				{
					iobuffer = _iobuffer_queue.get_available();
					break;
				}
			}
		}
	}

	iobuffer->input.timestamp = input->timestamp;
	iobuffer->input.data_size = input->data_size;
	memmove((uint8_t*)iobuffer->input.data, input->data, iobuffer->input.data_size);
	iobuffer->input.x = input->x;
	iobuffer->input.y = input->y;
	iobuffer->input.width = input->width;
	iobuffer->input.height = input->height;

	::SetEvent(_event);
	return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
}

unsigned __stdcall sirius::library::video::transform::codec::partial::png::compressor::core::process_callback(void * param)
{
	sirius::library::video::transform::codec::partial::png::compressor::core * self = static_cast<sirius::library::video::transform::codec::partial::png::compressor::core*>(param);
	self->process();
	return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
}

void sirius::library::video::transform::codec::partial::png::compressor::core::process(void)
{
#if !defined(WITH_FULLSCAN)
	int32_t status = sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
	sirius::library::video::transform::codec::partial::png::compressor::core::buffer_t * iobuffer = nullptr;

	long long before_encode_timestamp = 0;
	long long after_encode_timestamp = 0;
	int32_t		block_buffer_size = 0;
	uint8_t *	block_buffer = nullptr;
	int32_t		compressed_buffer_size = 0;
	uint8_t *	compressed_buffer = nullptr;

	block_buffer_size = (_context->block_width * _context->block_height) << 2;
	block_buffer = static_cast<uint8_t*>(malloc(block_buffer_size));
	memset(block_buffer, 0x00, block_buffer_size);

	compressed_buffer_size = 1024 * 1024 * 1;
	compressed_buffer = static_cast<uint8_t*>(malloc(compressed_buffer_size));
	memset(compressed_buffer, 0x00, compressed_buffer_size);

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

					if (((process_width - process_x) == _context->width) && ((process_height - process_y) == _context->height))
					{
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

										cached_index[index] = index;
										cached_length[index] = plength[count];
										memmove(cached_compressed[index], pcompressed[count], cached_length[index]);

										count++;
									}
								}
								index++;
							}
						}
						_front->after_process_callback(count, pindex, pcompressed, plength, before_encode_timestamp, after_encode_timestamp);
					}
					else
					{
						int32_t count = 0;
						uint8_t * real_compressed_buffer = compressed_buffer;

						int32_t begin_height	= (process_y / _context->block_height) * _context->block_height;
						int32_t end_height		= ((process_y + process_height) / _context->block_height) * _context->block_height;
						
						if (((process_y + process_height) % _context->block_height) > 0)
							end_height += _context->block_height;

						int32_t begin_width = (process_x / _context->block_width) * _context->block_width;
						int32_t end_width = ((process_x + process_width) / _context->block_width) * _context->block_width;
						
						if(((process_x + process_width) % _context->block_width) > 0)
							end_width += _context->block_width;

#if 1
						int32_t index = 0;
						for (int32_t h = 0; h < _context->height; h = h + _context->block_height)
						{
							for (int32_t w = 0; w < _context->width; w = w + _context->block_width)
							{
								if (h >= begin_height && h < end_height && w >= begin_width && w < end_width)
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
											//int32_t index = (h / _context->block_height) * (_context->width / _context->block_width) + w / _context->block_width;
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
#if defined(WITH_PARTIAL_SENDING_MODE)
								else
								{
									if (_invalidate)
										_front->after_process_callback(cached_index[index], cached_compressed[index], cached_length[index], before_encode_timestamp, after_encode_timestamp);
								}
#endif
								index++;
							}
						}
#else
						for (int32_t h = begin_height; h < end_height; h = h + _context->block_height)
						{
							for (int32_t w = begin_width; w < end_width; w = w + _context->block_width)
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
										int32_t index = (h / _context->block_height) * (_context->width/_context->block_width) + w / _context->block_width;
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
#endif

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

#else
	int32_t status = sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
	sirius::library::video::transform::codec::partial::png::compressor::core::buffer_t * iobuffer = nullptr;

	long long before_encode_timestamp = 0;
	long long after_encode_timestamp = 0;

	uint8_t *	converted_buffer = nullptr;
	//uint8_t *	resized_buffer = nullptr;
	int32_t		me_buffer_size = 0;
	uint8_t *	me_buffer = nullptr;
	uint8_t *	prev_me_buffer = nullptr;;
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
								if(sum>0)
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
#endif
}

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

#if defined(WITH_FULLSCAN)
#if defined(WITH_AVX2_SIMD)
size_t sirius::library::video::transform::codec::partial::png::compressor::core::avx2_aligned_high(size_t size, size_t align)
{
	return (size + align - 1) & ~(align - 1);
}

void * sirius::library::video::transform::codec::partial::png::compressor::core::avx2_align_high(const void * ptr, size_t align)
{
	return (void *)((((size_t)ptr) + align - 1) & ~(align - 1));
}

size_t sirius::library::video::transform::codec::partial::png::compressor::core::avx2_align_low(size_t size, size_t align)
{
	return size & ~(align - 1);
}

void * sirius::library::video::transform::codec::partial::png::compressor::core::avx2_align_low(const void * ptr, size_t align)
{
	return (void *)(((size_t)ptr) & ~(align - 1));
}

bool sirius::library::video::transform::codec::partial::png::compressor::core::avx2_is_aligned(size_t size, size_t align)
{
	return size == avx2_align_low(size, align);
}

bool sirius::library::video::transform::codec::partial::png::compressor::core::avx2_is_aligned(const void * ptr, size_t align)
{
	return ptr == avx2_align_low(ptr, align);
}

__m256i	sirius::library::video::transform::codec::partial::png::compressor::core::avx2_load(bool aligned, const __m256i * p)
{
	if (aligned)
		return _mm256_load_si256(p);
	else
		return _mm256_loadu_si256(p);
}

void sirius::library::video::transform::codec::partial::png::compressor::core::avx2_store(bool aligned, __m256i * p, __m256i a)
{
	if (aligned)
		return _mm256_store_si256(p, a);
	else
		return _mm256_storeu_si256(p, a);
}

__m256i sirius::library::video::transform::codec::partial::png::compressor::core::avx2_pack_u16tou8(__m256i lo, __m256i hi)
{
	return _mm256_permute4x64_epi64(_mm256_packus_epi16(lo, hi), 0xD8);
}

__m256i	sirius::library::video::transform::codec::partial::png::compressor::core::avx2_packi32toi16(__m256i lo, __m256i hi)
{
	return _mm256_permute4x64_epi64(_mm256_packs_epi32(lo, hi), 0xD8);
}

__m256i	sirius::library::video::transform::codec::partial::png::compressor::core::avx2_bgra2gray32(__m256i bgra)
{
	const __m256i g0a0 = _mm256_and_si256(_mm256_srli_si256(bgra, 1), K16_00FF);
	const __m256i b0r0 = _mm256_and_si256(bgra, K16_00FF);
	const __m256i weightedSum = _mm256_add_epi32(_mm256_madd_epi16(g0a0, K16_GREEN_0000), _mm256_madd_epi16(b0r0, K16_BLUE_RED));
	return _mm256_srli_epi32(_mm256_add_epi32(weightedSum, K32_ROUND_TERM), BGR_TO_GRAY_AVERAGING_SHIFT);
}

__m256i	sirius::library::video::transform::codec::partial::png::compressor::core::avx2_bgra2gray(__m256i bgra[4])
{
	const __m256i lo = avx2_packi32toi16(avx2_bgra2gray32(bgra[0]), avx2_bgra2gray32(bgra[1]));
	const __m256i hi = avx2_packi32toi16(avx2_bgra2gray32(bgra[2]), avx2_bgra2gray32(bgra[3]));
	return avx2_pack_u16tou8(lo, hi);
}

void sirius::library::video::transform::codec::partial::png::compressor::core::avx2_bgra2gray(bool aligned, const uint8_t * bgra, size_t width, size_t height, size_t bgra_stride, uint8_t * gray, size_t gray_stride)
{
	assert(width >= AVX2_ALIGN_SIZE);
	if (aligned)
		assert(avx2_is_aligned(bgra, AVX2_ALIGN_SIZE) && avx2_is_aligned(bgra_stride, AVX2_ALIGN_SIZE) && avx2_is_aligned(gray, AVX2_ALIGN_SIZE) && avx2_is_aligned(gray_stride, AVX2_ALIGN_SIZE));

	size_t aligned_width = avx2_align_low(width, AVX2_ALIGN_SIZE);
	__m256i a[4];
	for (size_t row = 0; row < height; ++row)
	{
		for (size_t col = 0; col < aligned_width; col += AVX2_ALIGN_SIZE)
		{
			a[0] = avx2_load(aligned, (__m256i*)(bgra + 4 * col) + 0);
			a[1] = avx2_load(aligned, (__m256i*)(bgra + 4 * col) + 1);
			a[2] = avx2_load(aligned, (__m256i*)(bgra + 4 * col) + 2);
			a[3] = avx2_load(aligned, (__m256i*)(bgra + 4 * col) + 3);
			avx2_store(aligned, (__m256i*)(gray + col), avx2_bgra2gray(a));
		}
		if (aligned_width != width)
		{
			a[0] = avx2_load(aligned, (__m256i*)(bgra + 4 * (width - AVX2_ALIGN_SIZE)) + 0);
			a[1] = avx2_load(aligned, (__m256i*)(bgra + 4 * (width - AVX2_ALIGN_SIZE)) + 1);
			a[2] = avx2_load(aligned, (__m256i*)(bgra + 4 * (width - AVX2_ALIGN_SIZE)) + 2);
			a[3] = avx2_load(aligned, (__m256i*)(bgra + 4 * (width - AVX2_ALIGN_SIZE)) + 3);
			avx2_store(false, (__m256i*)(gray + width - AVX2_ALIGN_SIZE), avx2_bgra2gray(a));
		}
		bgra += bgra_stride;
		gray += gray_stride;
	}
}

__m256i	sirius::library::video::transform::codec::partial::png::compressor::core::avx2_set_mask(bool aligned, uint8_t first, size_t position, uint8_t second)
{
	const size_t size = AVX2_ALIGN_SIZE / sizeof(uint8_t);
	assert(position <= size);
	uint8_t mask[size];
	for (size_t i = 0; i < position; ++i)
		mask[i] = first;
	for (size_t i = position; i < size; ++i)
		mask[i] = second;
	if(aligned)
		return _mm256_load_si256((__m256i*)mask);
	else
		return _mm256_loadu_si256((__m256i*)mask);
}

uint64_t sirius::library::video::transform::codec::partial::png::compressor::core::avx2_extract(bool aligned, __m256i value)
{
	const size_t size = AVX2_ALIGN_SIZE / sizeof(uint64_t);
	uint64_t buffer[size];

	if(aligned)
		_mm256_store_si256((__m256i*)buffer, value);
	else
		_mm256_storeu_si256((__m256i*)buffer, value);

	uint64_t sum = 0;
	for (size_t i = 0; i < size; ++i)
		sum += buffer[i];

	return sum;
}

bool sirius::library::video::transform::codec::partial::png::compressor::core::avx2_is_different(bool aligned, const uint8_t * a, size_t a_stride, const uint8_t * b, size_t b_stride, size_t width, size_t height)
{
	assert(width >= AVX2_ALIGN_SIZE);
	if (aligned)
	{
		assert(avx2_is_aligned(a_stride, AVX2_ALIGN_SIZE));
		assert(avx2_is_aligned(b_stride, AVX2_ALIGN_SIZE));
		assert(avx2_is_aligned(b, AVX2_ALIGN_SIZE));
		assert(avx2_is_aligned(a, AVX2_ALIGN_SIZE));
	}

	size_t body_width	= avx2_align_low(width, AVX2_ALIGN_SIZE);
	__m256i tail_mask	= avx2_set_mask(aligned, 0, AVX2_ALIGN_SIZE - width + body_width, 0xFF);
	__m256i sum			= _mm256_setzero_si256();
	__m256i compare		= _mm256_setzero_si256();
	for (size_t row = 0; row < height; ++row)
	{
		for (size_t col = 0; col < body_width; col += AVX2_ALIGN_SIZE)
		{
			const __m256i a_ = avx2_load(aligned, (__m256i*)(a + col));
			const __m256i b_ = avx2_load(aligned, (__m256i*)(b + col));
			sum = _mm256_add_epi64(_mm256_sad_epu8(a_, b_), sum);

			if (avx2_extract(false, sum) > 0)
				return true;
		}
		if (width - body_width)
		{
			const __m256i a_ = _mm256_and_si256(tail_mask, avx2_load(false, (__m256i*)(a + width - AVX2_ALIGN_SIZE)));
			const __m256i b_ = _mm256_and_si256(tail_mask, avx2_load(false, (__m256i*)(b + width - AVX2_ALIGN_SIZE)));
			sum = _mm256_add_epi64(_mm256_sad_epu8(a_, b_), sum);

			if (avx2_extract(false, sum) > 0)
				return true;
		}
		a += a_stride;
		b += b_stride;
	}

	if (avx2_extract(false, sum) > 0)
		return true;
	else
		return false;
}
#endif
#endif