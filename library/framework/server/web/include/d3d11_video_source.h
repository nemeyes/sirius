#ifndef _D3D11_VIDEO_SOURCE_H_
#define _D3D11_VIDEO_SOURCE_H_

#include <sirius_d3d11_video_capturer.h>
#include <sirius_web_server_framework.h>

namespace sirius
{
	namespace library
	{
		namespace framework
		{
			namespace server
			{
				class web::d3d11_video_source
					: public sirius::library::video::source::d3d11::capturer::handler
				{
				public:
					d3d11_video_source(sirius::library::framework::server::web::core * framework);
					virtual ~d3d11_video_source(void);

					int32_t start(int32_t fps, int32_t player_type = sirius::library::framework::server::web::attendant_type_t::web);
					int32_t stop(void);

				private:
					virtual void on_initialize(void * device, int32_t hwnd, int32_t smt, int32_t width, int32_t height);
					virtual void on_process(sirius::library::video::source::capturer::entity_t * input);
					virtual void on_release(void);

				private:
					sirius::library::framework::server::web::core * _framework;

				};
			};
		};
	};
};

#endif
