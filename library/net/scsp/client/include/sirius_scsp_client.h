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

					virtual void on_begin_video(int32_t codec, int32_t width, int32_t height, int32_t block_width, int32_t block_height) = 0;
					virtual void on_recv_video(int32_t codec, const uint8_t * data, int32_t length, long long dts, long long cts) = 0;
					virtual void on_recv_video(int32_t codec, int32_t count, int32_t * index, uint8_t ** data, int32_t * length, long long dts, long long cts) = 0;
					virtual void on_end_video(void) = 0;

				private:
					sirius::library::net::scsp::client::core * _video_client;
				};
			};
		};
	};
};

#endif