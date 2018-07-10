#ifndef _SIRIUS_CLIENT_FRAMEWORK_H_
#define _SIRIUS_CLIENT_FRAMEWORK_H_

#include <sirius.h>

namespace sirius
{
	namespace library
	{
		namespace framework
		{
			namespace client
			{
				class base
					: public sirius::base
				{
				public:
					typedef struct _state
					{
						static const int32_t none = 0;
						static const int32_t running = 1;
						static const int32_t paused = 2;
						static const int32_t stopped = 3;
					} state_t;

					typedef struct _debug_level_t
					{
						static const int32_t none = 0;
						static const int32_t gray = 1;
						static const int32_t frame = 2;
						static const int32_t file = 3;
					} debug_level_t;

					virtual int32_t state(void) = 0;
					virtual int32_t open(wchar_t * url, int32_t port, int32_t recv_option, bool repeat) = 0;
					virtual int32_t play(HWND hwnd) = 0;
					virtual int32_t stop(void) = 0;
					virtual int32_t change_debug_level(int32_t level) = 0;
				};
			};
		};
	};
};

#endif
