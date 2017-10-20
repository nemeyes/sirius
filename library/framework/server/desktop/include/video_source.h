#ifndef _VIDEO_SOURCE_H_
#define _VIDEO_SOURCE_H_

#include <sirius_d3d11_desktop_capturer.h>
#include <sirius_desktop_server_framework.h>

namespace sirius
{
	namespace library
	{
		namespace framework
		{
			namespace server
			{
				class desktop::video_source
					: public sirius::library::video::source::d3d11::desktop::capturer::handler
				{
				public:
					video_source(sirius::library::framework::server::desktop::core * framework);
					virtual ~video_source(void);

					int32_t start(int32_t fps, int32_t player_type);
					int32_t stop(void);

				private:
					virtual void on_initialize(void * device, int32_t hwnd, int32_t smt, int32_t width, int32_t height);
					virtual void on_process(sirius::library::video::source::capturer::entity_t * input);
					virtual void on_release(void);

				private:
					sirius::library::framework::server::desktop::core * _framework;

					sirius::library::video::source::d3d11::desktop::capturer _capturer;
					sirius::library::video::source::d3d11::desktop::capturer::context_t _context;
				};
			};
		};
	};
};

#endif
