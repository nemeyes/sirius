#ifndef _SIRIUS_PCSP_SERVER_H_
#define _SIRIUS_PCSP_SERVER_H_

#include <abstract_stream_server.h>

#define SERVER_UUID			"00000000-0000-0000-0000-000000000000"
#define MAX_VIDEO_ES_SIZE	500000

#if defined(EXPORT_SCSP_SERVER_LIB)
#define EXP_SCSP_SERVER_CLASS __declspec(dllexport)
#else
#define EXP_SCSP_SERVER_CLASS __declspec(dllimport)
#endif

namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace scsp
			{
				class EXP_SCSP_SERVER_CLASS server
					: public sirius::library::net::stream::server
				{
				public:
					class core;
				public:
					static const int32_t port_number_base = 7000;
					typedef struct _state_t
					{
						static const int32_t stopped = 0;
						static const int32_t paused = 1;
						static const int32_t subscribing = 2;
						static const int32_t begin_publishing = 3;
						static const int32_t publishing = 4;
					} state_t;

					server(void);
					virtual ~server(void);

					int32_t start(sirius::library::net::scsp::server::context_t * context);
					int32_t stop(void);

					int32_t post_video(uint8_t * bytes, size_t nbytes, long long timestamp);

					int32_t set_es_stream(uint8_t * data, int32_t stream_type, int32_t data_pos, uint8_t * bitstream, size_t nb);
					int32_t set_casp_payload(uint8_t * data, uint8_t stream_type, int32_t & data_pos, uint8_t contents_count, size_t es_header_size, size_t nb, long long timestamp);

				private:
					sirius::library::net::scsp::server::context_t * _context;
					sirius::library::net::scsp::server::core * _core;
					int32_t _frame_index_video;
					uint8_t * _video_data;
					int32_t _state;
				};
			};
		};
	};
};

#endif