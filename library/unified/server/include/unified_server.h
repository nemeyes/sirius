#ifndef _UNIFIED_SERVER_H_
#define _UNIFIED_SERVER_H_

#include "abstract_stream_server.h"
#include "sirius_unified_server.h"
#include "unified_compressor.h"

namespace sirius
{
	namespace library
	{
		namespace unified
		{
			class server::core
				: public sirius::library::net::stream::server::proxy
			{
				friend class sirius::library::unified::compressor;
			public:
				core(void);
				virtual ~core(void);

				int32_t initialize(sirius::library::unified::server::context_t * context);
				int32_t release(void);

				bool	is_video_compressor_initialized(void);
				int32_t initialize_video_compressor(sirius::library::unified::server::video_compressor_context_t * context);
				int32_t release_video_compressor(void);
				int32_t compress(sirius::library::video::transform::codec::compressor::entity_t * input);

				sirius::library::unified::server::network_usage_t & get_network_usage(void);

			private:
				int32_t play(int32_t flags);
				int32_t pause(int32_t flags);
				int32_t stop(int32_t flags);
				int32_t invalidate(void);

				int32_t publish_video(uint8_t * bytes, int32_t nbytes, long long before_encode_timestamp, long long after_encode_timestamp);
				int32_t publish_video(int32_t count, int32_t * index, uint8_t ** compressed, int32_t * size, long long before_compress_timestamp, long long after_compress_timestamp);

				void after_video_compressing_callback(uint8_t * data, size_t size, long long before_encode_timestamp, long long after_encode_timestamp);
				void after_video_compressing_callback(int32_t count, int32_t * index, uint8_t ** compressed, int32_t * size, long long before_compress_timestamp, long long after_compress_timestamp);

			private:
				sirius::library::unified::server::context_t * _context;
				void *	_streamer;
				void *	_streamer_context;
				sirius::library::unified::server::network_usage_t _network_usage;

				sirius::library::unified::compressor * _unified_compressor;
				bool _vcmprs_initialized;

				CRITICAL_SECTION	_vcs;
			};
		};
	};
};
#endif