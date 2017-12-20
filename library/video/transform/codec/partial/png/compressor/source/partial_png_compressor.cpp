#include "sirius_partial_png_compressor.h"
#include "partial_png_compressor.h"

#include <sirius_image_creator.h>
#include <sirius_stringhelper.h>
#include <sirius_locks.h>

#include <Simd/SimdLib.h>
#include <vector>

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
	if ((_state != sirius::library::video::transform::codec::partial::png::compressor::state_t::initialized) &&
		(_state != sirius::library::video::transform::codec::partial::png::compressor::state_t::compressed))
		return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;

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

		iobuffer->input.timestamp = input->timestamp;
		iobuffer->input.data_size = input->data_size;
		memmove((uint8_t*)iobuffer->input.data, input->data, iobuffer->input.data_size);

		::SetEvent(_event);
	}

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
	int32_t status = sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
	sirius::library::video::transform::codec::partial::png::compressor::core::buffer_t * iobuffer = nullptr;

	long long before_encode_timestamp = 0;
	long long after_encode_timestamp = 0;

	uint8_t *	resized_buffer = nullptr;
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
		context_width = _context->width >> 1;
		context_height = _context->height >> 1;
		block_width = _context->block_width >> 1;
		block_height = _context->block_height >> 1;
		resized_buffer = static_cast<uint8_t*>(malloc((context_width * context_height) << 2));

		me_buffer_size = context_width * context_height;
		me_buffer = static_cast<uint8_t*>(malloc(me_buffer_size));
		memset(me_buffer, 0x00, me_buffer_size);
		prev_me_buffer = static_cast<uint8_t*>(malloc(me_buffer_size));
		memset(prev_me_buffer, 0x00, me_buffer_size);

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


	int32_t		process_data_capacity = (_context->width * _context->height) << 2;
	int32_t		process_data_size = 0;
	int32_t		last_process_data_size = 0;
	uint8_t *	process_data = (uint8_t*)malloc(process_data_capacity);
	long long	process_timestamp = 0;
	while (_run)
	{
		if (::WaitForSingleObject(_event, INFINITE) == WAIT_OBJECT_0)
		{
			_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressing;

			if (_invalidate)
			{
				if (prev_me_filled)
				{
					prev_me_filled = false;
					process_data_size = last_process_data_size;
				}
				else
				{
					sirius::autolock lock(&_cs);
					iobuffer = _iobuffer_queue.get_pending();
					if (!iobuffer)
					{
						_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressed;
						continue;
					}

					process_data_size = iobuffer->input.data_size;
					process_timestamp = iobuffer->input.timestamp;
					memmove(process_data, iobuffer->input.data, process_data_size);

					last_process_data_size = process_data_size;
				}
				_invalidate = false;
			}
			else
			{
				sirius::autolock lock(&_cs);
				iobuffer = _iobuffer_queue.get_pending();
				if (!iobuffer)
				{
					_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressed;
					continue;
				}

				process_data_size = iobuffer->input.data_size;
				process_timestamp = iobuffer->input.timestamp;
				memmove(process_data, iobuffer->input.data, process_data_size);

				last_process_data_size = process_data_size;
			}
		}

		while (process_data_size>0)
		{
			before_encode_timestamp = process_timestamp;

			SimdResizeBilinear(process_data, _context->width, _context->height, _context->width << 2, resized_buffer, context_width, context_height, context_width << 2, 4);
			SimdBgraToGray(resized_buffer, context_width, context_height, context_width << 2, me_buffer, context_width);
			if (prev_me_filled)
			{
				int32_t count = 0;
				int32_t index = 0;

				uint8_t * real_compressed_buffer = compressed_buffer;

				for (int32_t h = 0, h2 = 0; h < _context->height; h = h + _context->block_height, h2 = h2 + block_height)
				{
					for (int32_t w = 0, w2 = 0; w < _context->width; w = w + _context->block_width, w2 = w2 + block_width)
					{
						uint64_t sum = 0;
						SimdAbsDifferenceSum(me_buffer + (h2*context_width + w2), context_width, prev_me_buffer + (h2*context_width + w2), context_width, block_width, block_height, &sum);
						if (sum > 0)
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
									count++;
								}
							}
						}
						index++;
					}
				}
				if (count>0)
					_front->after_process_callback(count, pindex, pcompressed, plength, before_encode_timestamp, after_encode_timestamp);
			}
			else
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
								count++;
							}
						}
						index++;
					}
				}

				if (count>0)
					_front->after_process_callback(count, pindex, pcompressed, plength, before_encode_timestamp, after_encode_timestamp);
			}

			if (!prev_me_filled)
				prev_me_filled = true;
			memcpy(prev_me_buffer, me_buffer, me_buffer_size);

			{
				sirius::autolock lock(&_cs);
				iobuffer = _iobuffer_queue.get_pending();
				if (!iobuffer)
				{
					process_data_size = 0;
					continue;
				}

				process_data_size = iobuffer->input.data_size;
				process_timestamp = iobuffer->input.timestamp;
				memmove(process_data, iobuffer->input.data, process_data_size);
			}
			continue;
		}
		_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressed;
	}

	if (process_data)
		free(process_data);
	process_data = nullptr;

	if (pindex)
		delete pindex;
	if (plength)
		delete plength;
	if (pcompressed)
		delete pcompressed;

	{
		if (compressed_buffer)
			free(compressed_buffer);
		compressed_buffer = nullptr;
		compressed_buffer_size = 0;

		if (block_buffer)
			free(block_buffer);
		block_buffer = nullptr;
		block_buffer_size = 0;


		if (prev_me_buffer)
			free(prev_me_buffer);
		prev_me_buffer = nullptr;

		if (me_buffer)
			free(me_buffer);
		me_buffer = nullptr;
		me_buffer_size = 0;

		if (resized_buffer)
			free(resized_buffer);
		resized_buffer = nullptr;
	}
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