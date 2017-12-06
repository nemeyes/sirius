#ifndef _PARTIAL_PNG_COMPRESSOR_H_
#define _PARTIAL_PNG_COMPRESSOR_H_

#include "sirius_partial_png_compressor.h"

#include <atlbase.h>
#include <d3d11.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <setjmp.h>

#include <sirius_template_queue.h>
#include <sirius_video_processor.h>

#include "libpng_compressor.h"

namespace sirius
{
	namespace library
	{
		namespace video
		{
			namespace transform
			{
				namespace codec
				{
					namespace partial
					{
						namespace png
						{
							class compressor::core
								: public sirius::library::video::transform::codec::processor
							{
							public:
								static const int32_t MAX_IO_BUFFERS = 30;
								static const int32_t MAX_PNG_SIZE = 1024 * 1024 * 4;

								typedef struct _ibuffer_t
								{
									void *		data;
									int32_t		data_size;
									int32_t		data_capacity;
									long long	timestamp;
									_ibuffer_t(void)
										: data(nullptr)
										, timestamp(0)
									{}
								} ibuffer_t;

								typedef struct _buffer_t
								{
									sirius::library::video::transform::codec::partial::png::compressor::core::ibuffer_t	input;
									//sirius::library::video::transform::codec::partial::png::compressor::core::obuffer_t	output;
								} buffer_t;

							public:
								core(sirius::library::video::transform::codec::partial::png::compressor * front);
								~core(void);

								int32_t state(void);

								int32_t initialize(sirius::library::video::transform::codec::partial::png::compressor::context_t * context);
								int32_t release(void);

								int32_t play(void);
								int32_t pause(void);
								int32_t stop(void);

								int32_t compress(sirius::library::video::transform::codec::partial::png::compressor::entity_t * input, sirius::library::video::transform::codec::partial::png::compressor::entity_t * bitstream);
								int32_t compress(sirius::library::video::transform::codec::partial::png::compressor::entity_t * input);

							private:
								static unsigned __stdcall process_callback(void * param);
								void	process(void);
								int32_t allocate_io_buffers(void);
								int32_t release_io_buffers(void);

							private:
								sirius::library::video::transform::codec::partial::png::compressor * _front;
								sirius::library::video::transform::codec::partial::png::compressor::context_t * _context;

								CRITICAL_SECTION	_cs;
								int32_t				_state;
								bool				_run;
								HANDLE				_thread;
								HANDLE				_event;

								sirius::library::video::transform::codec::partial::png::compressor::core::buffer_t					_iobuffer[MAX_IO_BUFFERS];
								sirius::queue<sirius::library::video::transform::codec::partial::png::compressor::core::buffer_t>	_iobuffer_queue;

								uint8_t *	_rgba_buffer;

								ATL::CComPtr<ID3D11Device>			_device;
								ATL::CComPtr<ID3D11DeviceContext>	_device_ctx;
								ATL::CComPtr<ID3D11Texture2D>		_intermediate_tex;

								CRITICAL_SECTION					_device_ctx_cs;

								sirius::library::video::transform::codec::libpng::compressor * _real_compressor;
							};
						};
					};
				};
			};
		};
	};
};


#endif
