#ifndef _SIRIUS_UNIFIED_CLIENT_H_
#define _SIRIUS_UNIFIED_CLIENT_H_

#include <sirius.h>
#include <vector>
#include <string>

#if defined(EXPORT_UNIFIED_CLIENT_LIB)
#define EXP_UNIFIED_CLIENT_CLASS __declspec(dllexport)
#else
#define EXP_UNIFIED_CLIENT_CLASS __declspec(dllimport)
#endif

namespace sirius
{
	namespace library
	{
		namespace unified
		{
			class EXP_UNIFIED_CLIENT_CLASS client
				: public sirius::base
			{
			public:
				class core;
			public:
				typedef struct _state
				{
					static const int32_t none = 0;
					static const int32_t running = 1;
					static const int32_t paused = 2;
					static const int32_t stopped = 3;
				} state_t;

				client(void);
				virtual ~client(void);

				int32_t state(void);
				int32_t open(wchar_t * url, int32_t port, int32_t recv_option, bool repeat);
				int32_t play(void);
				int32_t stop(void);

				virtual void on_begin_video(int32_t smt, const uint8_t * data, size_t data_size, long long dts, long long cts) = 0;
				virtual void on_recv_video(int32_t smt, const uint8_t * data, size_t data_size, long long dts, long long cts) = 0;
				virtual void on_end_video(void) = 0;

			private:
				sirius::library::unified::client::core * _core;
			};
		};
	};
};



#endif
