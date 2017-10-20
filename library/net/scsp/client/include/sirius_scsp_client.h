#ifndef _SIRIUS_SCSP_CLIENT_H_
#define _SIRIUS_SCSP_CLIENT_H_

#include <sirius.h>

#if defined(EXPORT_SCSP_CLIENT_LIB)
#define EXP_SCSP_CLIENT_CLASS __declspec(dllexport)
#else
#define EXP_SCSP_CLIENT_CLASS __declspec(dllimport)
#endif

namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace scsp
			{
				class EXP_SCSP_CLIENT_CLASS client
					: public sirius::base
				{
				public:
					class core;
				public:
					typedef struct _state_t
					{
						static const int32_t disconnecting = 0;
						static const int32_t disconnected = 1;
						static const int32_t connecting = 2;
						static const int32_t connected = 3;
						static const int32_t streaming = 4;
					} state_t;

					client(void);
					virtual ~client(void);

					void play(const char * url, int32_t port, int32_t recv_option, bool reconnection = true);
					void stop(void);

					void on_recv_video_info(int32_t video_codec, int32_t video_width, int32_t video_height, int32_t video_fps);

					virtual void on_begin_video(int32_t smt, const uint8_t * data, size_t data_size, long long dts, long long cts) = 0;
					virtual void on_recv_video(int32_t smt, const uint8_t * data, size_t data_size, long long dts, long long cts) = 0;
					virtual void on_end_video(void) = 0;

				private:
					sirius::library::net::scsp::client::core * _video_client;

					int32_t _video_codec;
					int32_t _video_width;
					int32_t _video_height;
					int32_t _video_fps;
				};
			};
		};
	};
};

#endif