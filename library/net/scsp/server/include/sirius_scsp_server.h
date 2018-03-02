#ifndef _SIRIUS_PCSP_SERVER_H_
#define _SIRIUS_PCSP_SERVER_H_

#include <abstract_stream_server.h>

#define SERVER_UUID			"00000000-0000-0000-0000-000000000000"
#define MAX_VIDEO_ES_SIZE	1000000

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

					int32_t post_video_header(int32_t count, long long timestamp);
					int32_t post_video_payload(int32_t index, uint8_t * compressed, int32_t size);
					int32_t post_video(int32_t count, int32_t * index, uint8_t ** compressed, int32_t * size, long long timestamp);
					int32_t post_video(int32_t index, uint8_t * compressed, int32_t size, long long timestamp);


				private:
					sirius::library::net::scsp::server::context_t * _context;
					sirius::library::net::scsp::server::core *		_core;
					uint8_t *	_video_data;
					int32_t		_state;
				};
			};
		};
	};
};

#endif