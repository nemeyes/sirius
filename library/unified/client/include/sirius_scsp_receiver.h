#ifndef _SIRIUS_SCSP_RECEIVER_H_
#define _SIRIUS_SCSP_RECEIVER_H_

#include <sirius_receiver.h>
#include <sirius_scsp_client.h>

namespace sirius
{
	namespace library
	{
		namespace unified
		{
			namespace scsp
			{
				class receiver
					: public sirius::library::unified::receiver
					, public sirius::library::net::scsp::client
				{
				public:
					receiver(sirius::library::unified::client * front);
					virtual ~receiver(void);

					void play(const char * url, int32_t port, int32_t recv_option, bool repeat);
					void stop(void);

					void on_begin_video(int32_t codec, int32_t width, int32_t height, int32_t block_width, int32_t block_height);
					void on_recv_video(int32_t codec, const uint8_t * data, int32_t length, long long dts, long long cts);
					void on_recv_video(int32_t codec, int32_t count, int32_t * index, uint8_t ** data, int32_t * length, long long dts, long long cts);
					void on_end_video(void);

				private:
					bool _recv_video;
				};
			};
		};
	};
};


#endif