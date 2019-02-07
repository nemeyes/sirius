#ifndef _SIRIUS_UNIFIED_SERVER_H_
#define _SIRIUS_UNIFIED_SERVER_H_

#include <sirius.h>
#include <sirius_video_compressor.h>

#if defined(EXPORT_UNIFIED_SERVER)
#define EXP_UNIFIED_SERVER_CLASS __declspec(dllexport)
#else
#define EXP_UNIFIED_SERVER_CLASS __declspec(dllimport)
#endif

namespace sirius
{
	namespace library
	{
		namespace unified
		{
			class EXP_UNIFIED_SERVER_CLASS server
				: public sirius::base
			{
			public:
				class core;
			public:
				typedef struct EXP_UNIFIED_SERVER_CLASS _context_t
				{
					wchar_t uuid[64];
					int32_t portnumber;
					bool	keepalive;
					int32_t	keepalive_timeout;
					int32_t video_codec;
					int32_t video_width;
					int32_t video_height;
					int32_t video_fps;
					int32_t video_block_width;
					int32_t video_block_height;
					_context_t(void)
						: portnumber(-1)
						, keepalive(false)
						, keepalive_timeout(5000)
						, video_codec(sirius::library::unified::server::video_submedia_type_t::unknown)
						, video_width(1280)
						, video_height(720)
						, video_fps(30)
						, video_block_width(128)
						, video_block_height(72)
					{
						memset(uuid, 0x00, sizeof(uuid));
					}
				} context_t;

				typedef struct EXP_UNIFIED_SERVER_CLASS _video_png_compressor_context_t
				{
					int32_t compression_level;
					bool	quantization_posterization;
					bool	quantization_dither_map;
					bool	quantization_contrast_maps;
					int32_t quantization_colors;
					_video_png_compressor_context_t()
						: compression_level(-1)
						, quantization_posterization(true)
						, quantization_dither_map(false)
						, quantization_contrast_maps(false)
						, quantization_colors(128)
					{}
				} video_png_compressor_context_t;

				typedef struct EXP_UNIFIED_SERVER_CLASS _video_webp_compressor_context_t
				{
					float	quality;
					int32_t method;
					_video_webp_compressor_context_t()
						: quality(100.f)
						, method(0)
					{}
				} video_webp_compressor_context_t;

				typedef struct EXP_UNIFIED_SERVER_CLASS _video_compressor_context_t
				{
					int32_t gpuindex;
					int32_t memtype;
					void *	device;
					int32_t codec;
					int32_t origin_width;
					int32_t origin_height;
					int32_t width;
					int32_t height;
					int32_t fps;
					int32_t nbuffer;
					int32_t block_width;
					int32_t block_height;
					video_png_compressor_context_t	png;
					video_webp_compressor_context_t webp;
					bool	invalidate4client;
					bool	indexed_mode;
					int32_t nthread;
					bool	play_after_init;
					bool	caching;
					_video_compressor_context_t(void)
						: gpuindex(0)
						, memtype(sirius::library::unified::server::video_memory_type_t::host)
						, device(nullptr)
						, codec(sirius::library::unified::server::video_submedia_type_t::png)
						, origin_width(0)
						, origin_height(0)
						, width(0)
						, height(0)
						, fps(0)
						, nbuffer(1)
						, block_width(0)
						, block_height(0)
						, play_after_init(false)
						, invalidate4client(false)
						, indexed_mode(false)
						, nthread(20)
						, caching(false)
					{
					
					}
				} video_compressor_context_t;

				typedef struct _network_usage_t
				{
					uint64_t video_transferred_bytes;
					_network_usage_t(void)
						: video_transferred_bytes(0)
					{
					}
				} network_usage_t;

				server(void);
				~server(void);

				int32_t initialize(sirius::library::unified::server::context_t * context);
				int32_t release(void);

				bool	is_video_compressor_initialized(void);
				int32_t initialize_video_compressor(sirius::library::unified::server::video_compressor_context_t * context);
				int32_t release_video_compressor(void);
				int32_t compress(sirius::library::video::transform::codec::compressor::entity_t * input);

				sirius::library::unified::server::network_usage_t & get_network_usage(void);

			private:
				server(sirius::library::unified::server & clone);

			private:
				sirius::library::unified::server::core * _core;
				void * _vcmprs_config;
			};
		};
	};
};
#endif