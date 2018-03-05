#ifndef _SIRIUS_CPU_CAPTURER_H_
#define _SIRIUS_CPU_CAPTURER_H_

#if defined(EXPORT_CPU_CAPTURER_LIB)
#define EXP_CPU_CAPTURER_CLASS __declspec(dllexport)
#else
#define EXP_CPU_CAPTURER_CLASS __declspec(dllimport)
#endif

#include <sirius_video_source_capturer.h>

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
					class EXP_CPU_CAPTURER_CLASS capturer
						: public sirius::library::video::source::capturer
					{
					public:
						class core;
						class proxy;
					public:
						static sirius::library::video::source::cpu::capturer & instance(void);

						typedef struct EXP_CPU_CAPTURER_CLASS _context_t
							: public sirius::library::video::source::capturer::context_t
						{
							static _context_t & instance(void)
							{
								static _context_t _instance;
								return _instance;
							}
						} context_t;

						virtual int32_t initialize(sirius::library::video::source::capturer::context_t * context);
						virtual int32_t release(void);
						virtual int32_t start(void);
						virtual int32_t stop(void);
						virtual int32_t pause(void);

						int32_t post(int32_t smt, int32_t video_width, int32_t video_height, uint8_t * video, int32_t x, int32_t y, int32_t width, int32_t height);

					private:
						capturer(void);
						virtual ~capturer(void);

					private:
						sirius::library::video::source::cpu::capturer::core * _core;
					};
				};
			};
		};
	};
};

#endif
