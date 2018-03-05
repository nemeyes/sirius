#ifndef _CPU_CAPTURER_H_
#define _CPU_CAPTURER_H_

#include "sirius_cpu_video_capturer.h"


namespace sirius
{
	namespace library
	{
		namespace video
		{
			namespace source
			{
				namespace cpu
				{
					class capturer::core
					{
					public:
						core(void);
						virtual ~core(void);

						int32_t initialize(sirius::library::video::source::cpu::capturer::context_t * context);
						int32_t release(void);
						int32_t start(void);
						int32_t stop(void);
						int32_t pause(void);

						int32_t post(int32_t smt, int32_t video_width, int32_t video_height, uint8_t * video, int32_t x, int32_t y, int32_t width, int32_t height);

					private:
						bool		_brecv;
						uint8_t *	_buffer;
						int32_t		_buffer_size;

						sirius::library::video::source::cpu::capturer::context_t * _context;
					};
				};
			};
		};
	};
};

#endif