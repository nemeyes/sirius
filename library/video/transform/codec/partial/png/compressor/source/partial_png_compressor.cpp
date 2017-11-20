#include "sirius_partial_png_compressor.h"
#include "partial_png_compressor.h"

#include <sirius_image_creator.h>
#include <sirius_stringhelper.h>

#include <Simd/SimdLib.h>
#include <vector>

sirius::library::video::transform::codec::partial::png::compressor::core::core(sirius::library::video::transform::codec::partial::png::compressor * front)
	: _front(front)
	, _context(nullptr)
	, _state(sirius::library::video::transform::codec::partial::png::compressor::state_t::none)
	, _event(INVALID_HANDLE_VALUE)
	, _thread(INVALID_HANDLE_VALUE)
	, _run(false)
	, _rgba_buffer(nullptr)
{
	_real_compressor = new sirius::library::video::transform::codec::libpng::compressor(front);
}

sirius::library::video::transform::codec::partial::png::compressor::core::~core(void)
{
	_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::none;
	if (_real_compressor)
		delete _real_compressor;
	_real_compressor = nullptr;
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
	if (_context->memtype == sirius::library::video::transform::codec::partial::png::compressor::video_memory_type_t::d3d11)
	{
		if (!_context->device)
			return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::fail;

		_device.Attach((ID3D11Device*)_context->device);
		_device->GetImmediateContext(&_device_ctx);
		if (!_device_ctx)
			return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::fail;

		sirius::library::video::transform::codec::processor::initialize_d3d11((ID3D11Device*)_context->device, _context->width, _context->height, 30, _context->width, _context->height, 30, DXGI_FORMAT_R8G8B8A8_UNORM);
	}
	else if (_context->memtype == sirius::library::video::transform::codec::partial::png::compressor::video_memory_type_t::host)
	{

	}

	int32_t nbuffer = _context->width*_context->height * 4;
	_rgba_buffer = static_cast<uint8_t*>(malloc(nbuffer));
	memset(_rgba_buffer, 0x00, nbuffer);

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

	if (_context->memtype == sirius::library::video::transform::codec::partial::png::compressor::video_memory_type_t::d3d11)
	{
		sirius::library::video::transform::codec::processor::release_d3d11();
	}
	else if (_context->memtype == sirius::library::video::transform::codec::partial::png::compressor::video_memory_type_t::host)
	{

	}

	if (_rgba_buffer)
		free(_rgba_buffer);
	_rgba_buffer = nullptr;

	_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::released;
	return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::partial::png::compressor::core::play(void)
{
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
	if (input->memtype == sirius::library::video::transform::codec::partial::png::compressor::video_memory_type_t::d3d11)
	{
		if (!_device_ctx)
			return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::fail;

		iobuffer = _iobuffer_queue.get_available();
		if (!iobuffer)
			return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::fail;

		iobuffer->input.timestamp = input->timestamp;
		ID3D11Texture2D * gtex = (ID3D11Texture2D*)input->data;


		D3D11_TEXTURE2D_DESC gdesc;
		D3D11_TEXTURE2D_DESC hdesc;
		gtex->GetDesc(&gdesc);
		ID3D11Texture2D * input = (ID3D11Texture2D*)iobuffer->input.data;
		input->GetDesc(&hdesc);

		_device_ctx->CopyResource(input, gtex);

		D3D11_MAPPED_SUBRESOURCE sub_resource;
		HRESULT hr = _device_ctx->Map((ID3D11Resource*)iobuffer->input.data, 0, D3D11_MAP_READ, 0, &sub_resource);
		_device_ctx->Unmap((ID3D11Resource*)iobuffer->input.data, 0);

		::SetEvent(_event);
	}
	else if (input->memtype == sirius::library::video::transform::codec::partial::png::compressor::video_memory_type_t::host)
	{
		iobuffer = _iobuffer_queue.get_available();
		if (!iobuffer)
			return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::fail;

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

	int32_t me_buffer_size = 0;
	uint8_t * me_buffer = nullptr;
	uint8_t * prev_me_buffer = nullptr;;
	bool prev_me_filled = false;
	int32_t block_buffer_size = 0;
	uint8_t * block_buffer = nullptr;
	int32_t compressed_buffer_size = 0;
	uint8_t * compressed_buffer = nullptr;
	//if (_context->memtype == sirius::library::video::transform::codec::partial::png::compressor::video_memory_type_t::host)
	{
		me_buffer_size = _context->width * _context->height;
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

	int32_t block_count = (_context->width / _context->block_width) * (_context->height / _context->block_height);
	int32_t *	pindex = new int32_t[block_count];
	uint8_t **	pcompressed = new uint8_t*[block_count];
	int32_t *	plength = new int32_t[block_count];

	while (_run)
	{
		if (::WaitForSingleObject(_event, INFINITE) == WAIT_OBJECT_0)
		{
			_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressing;
			iobuffer = _iobuffer_queue.get_pending();
			while (iobuffer)
			{
				before_encode_timestamp = iobuffer->input.timestamp;

				if (_context->memtype == sirius::library::video::transform::codec::partial::png::compressor::video_memory_type_t::d3d11)
				{
					HRESULT hr = E_FAIL;
					D3D11_TEXTURE2D_DESC desc;
					ID3D11Texture2D * input = (ID3D11Texture2D*)iobuffer->input.data;
					input->GetDesc(&desc);

					D3D11_MAPPED_SUBRESOURCE sub_resource;
					hr = _device_ctx->Map((ID3D11Resource*)iobuffer->input.data, 0, D3D11_MAP_READ, 0, &sub_resource);
					if (SUCCEEDED(hr))
					{
						uint8_t * pixels = (uint8_t*)sub_resource.pData;
						memcpy(_rgba_buffer, pixels, _context->width * _context->height * 4);
						_device_ctx->Unmap((ID3D11Resource*)iobuffer->input.data, 0);
						SimdBgraToGray(_rgba_buffer, _context->width, _context->height, _context->width << 2, me_buffer, _context->width);

						if (prev_me_filled)
						{
							int32_t count = 0;
							int32_t index = 0;

							uint8_t * real_compressed_buffer = compressed_buffer;

							for (int32_t h = 0; h < _context->height; h = h + _context->block_height)
							{
								for (int32_t w = 0; w < _context->width; w = w + _context->block_width)
								{
									uint64_t sum = 0;
									SimdAbsDifferenceSum(me_buffer + (h*_context->width + w), _context->width, prev_me_buffer + (h*_context->width + w), _context->width, _context->block_width, _context->block_height, &sum);
									if (sum > 0)
									{
										for (int32_t bh = 0; bh < _context->block_height; bh++)
										{
											int32_t src_index = (h + bh) * (_context->width << 2) + (w << 2);
											int32_t dst_index = bh * (_context->block_width << 2);
											memmove(block_buffer + dst_index, _rgba_buffer + src_index, (_context->block_width << 2));
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

							if(count>0)
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
										memmove(block_buffer + dst_index, _rgba_buffer + src_index, (_context->block_width << 2));
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
					}


					iobuffer = _iobuffer_queue.get_pending();
					continue;
				}
				else if (_context->memtype == sirius::library::video::transform::codec::partial::png::compressor::video_memory_type_t::host)
				{
					uint8_t * pixels = (uint8_t*)iobuffer->input.data;
					int32_t		size = iobuffer->input.data_size;
					memmove(_rgba_buffer, pixels, size);
					SimdBgraToGray(_rgba_buffer, _context->width, _context->height, _context->width << 2, me_buffer, _context->width);

					if (prev_me_filled)
					{
						int32_t count = 0;
						int32_t index = 0;

						uint8_t * real_compressed_buffer = compressed_buffer;

						for (int32_t h = 0; h < _context->height; h = h + _context->block_height)
						{
							for (int32_t w = 0; w < _context->width; w = w + _context->block_width)
							{
								uint64_t sum = 0;
								SimdAbsDifferenceSum(me_buffer + (h*_context->width + w), _context->width, prev_me_buffer + (h*_context->width + w), _context->width, _context->block_width, _context->block_height, &sum);
								if (sum > 0)
								{
									for (int32_t bh = 0; bh < _context->block_height; bh++)
									{
										int32_t src_index = (h + bh) * (_context->width << 2) + (w << 2);
										int32_t dst_index = bh * (_context->block_width << 2);
										memmove(block_buffer + dst_index, _rgba_buffer + src_index, (_context->block_width << 2));
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
									int32_t src_index = (h+bh) * (_context->width <<2) + (w<<2);
									int32_t dst_index = bh * (_context->block_width << 2);
									memmove(block_buffer + dst_index, _rgba_buffer + src_index, (_context->block_width << 2));
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

						_front->after_process_callback(count, pindex, pcompressed, plength, before_encode_timestamp, after_encode_timestamp);

					}

					if (!prev_me_filled)
						prev_me_filled = true;
					memcpy(prev_me_buffer, me_buffer, me_buffer_size);

					iobuffer = _iobuffer_queue.get_pending();
					continue;
				}

			}
			_state = sirius::library::video::transform::codec::partial::png::compressor::state_t::compressed;
		}
	}

	if (pindex)
		delete pindex;
	if (plength)
		delete plength;
	if (pcompressed)
		delete pcompressed;

	//if (_context->memtype == sirius::library::video::transform::codec::partial::png::compressor::video_memory_type_t::host)
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
	}
}

int32_t sirius::library::video::transform::codec::partial::png::compressor::core::allocate_io_buffers(void)
{
	int32_t status = sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
	_iobuffer_queue.initialize(_iobuffer, _context->nbuffer);

	if (_context->memtype == sirius::library::video::transform::codec::partial::png::compressor::video_memory_type_t::host)
	{
		for (int32_t i = 0; i < _context->nbuffer; i++)
		{
			_iobuffer[i].input.data_capacity = _context->width * _context->height * 4;
			_iobuffer[i].input.data_size = 0;
			_iobuffer[i].input.data = static_cast<uint8_t*>(malloc(_iobuffer[i].input.data_capacity));

			/*
			_iobuffer[i].output.data_capacity = sirius::library::video::transform::codec::partial::png::compressor::core::MAX_PNG_SIZE;
			_iobuffer[i].output.data_size = 0;
			_iobuffer[i].output.data = static_cast<uint8_t*>(malloc(_iobuffer[i].output.data_capacity));
			*/
		}
	}
	else if (_context->memtype == sirius::library::video::transform::codec::partial::png::compressor::video_memory_type_t::d3d11)
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
		for (int32_t i = 0; i < _context->nbuffer; i++)
		{
			HRESULT hr = _device->CreateTexture2D(&desc, NULL, &tex);
			if (SUCCEEDED(hr))
			{
				_iobuffer[i].input.data = tex;
			}

			/*
			_iobuffer[i].output.data_capacity = sirius::library::video::transform::codec::partial::png::compressor::core::MAX_PNG_SIZE;
			_iobuffer[i].output.data_size = 0;
			_iobuffer[i].output.data = static_cast<uint8_t*>(malloc(_iobuffer[i].output.data_capacity));
			*/
		}

		/*
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.Width = _context->width;
		desc.Height = _context->height;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.MiscFlags = 0;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		_device->CreateTexture2D(&desc, NULL, &_intermediate_tex);
		*/
	}

	return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
}

int32_t sirius::library::video::transform::codec::partial::png::compressor::core::release_io_buffers(void)
{
	if (_context->memtype == sirius::library::video::transform::codec::partial::png::compressor::video_memory_type_t::d3d11)
	{
		for (int32_t i = 0; i < _context->nbuffer; i++)
		{
			ID3D11Texture2D * input = (ID3D11Texture2D*)_iobuffer[i].input.data;
			input->Release();
			_iobuffer[i].input.data = NULL;

			/*
			if (_iobuffer[i].output.data)
				free(_iobuffer[i].output.data);
			_iobuffer[i].output.data = nullptr;
			*/
		}
	}
	else if (_context->memtype == sirius::library::video::transform::codec::partial::png::compressor::video_memory_type_t::host)
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
	}

	_iobuffer_queue.release();
	return sirius::library::video::transform::codec::partial::png::compressor::err_code_t::success;
}