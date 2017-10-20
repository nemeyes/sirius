#ifndef _SIRIUS_FILE_RECEIVER_H_
#define _SIRIUS_FILE_RECEIVER_H_

#include <sirius_receiver.h>

namespace sirius
{
	namespace library
	{
		namespace unified
		{
			namespace file
			{
				class receiver
					: public sirius::library::unified::receiver
				{
				public:
					receiver(sirius::library::unified::client * front);
					virtual ~receiver(void);

					void play(const char * url, int32_t port=0, int32_t recv_option= sirius::library::unified::client::media_type_t::video, bool repeat=false);
					void stop(void);

					void on_begin_video(int32_t smt, const uint8_t * data, size_t data_size, long long dts, long long cts);
					void on_recv_video(int32_t smt, const uint8_t * data, size_t data_size, long long dts, long long cts);
					void on_end_video(void);

				private:
					static unsigned __stdcall process_cb(void * param);
					void process(void);

					long long file_size(HANDLE f);

				private:
					bool _recv_video;
					bool _run;
					HANDLE _thread;

					char _url[MAX_PATH];
					FILE * _file;
				};
			};
		};
	};
};














#endif