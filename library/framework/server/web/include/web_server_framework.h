#ifndef _WEB_SERVER_FRAMEWORK_H_
#define _WEB_SERVER_FRAMEWORK_H_

#include "sirius_web_server_framework.h"
#include "d3d11_video_source.h"
#include "host_video_source.h"

#include <sirius_unified_server.h>
#include <sirius_log4cplus_logger.h>
#include <queue>

namespace sirius
{
	namespace library
	{
		namespace framework
		{
			namespace server
			{
				class web::core
				{
				public:
					friend class d3d11_video_source;
					friend class host_video_source;

					typedef struct _handle_data_t 
					{
						unsigned long process_id;
						HWND best_handle;
					} handle_data_t;

				public:
					core(void);
					virtual ~core(void);

					void	set_notification_callee(sirius::library::misc::notification::internal::notifier::callee * callee);
					int32_t initialize(sirius::library::framework::server::web::context_t * context);
					int32_t release(void);
					int32_t play(void);
					int32_t pause(void);
					int32_t stop(void);
					int32_t state(void) const;

					void	post_msg_key(HWND hwnd, int32_t msg, int32_t key_code, bool extended);
					void	post_msg_mouse(HWND hwnd, int32_t msg, int32_t pos_x, int32_t pos_y);

					void	on_keyup(int32_t key);
					void	on_keydown(int32_t key);
					void	on_L_mouse_down(int32_t pos_x, int32_t pos_y);
					void	on_L_mouse_up(int32_t pos_x, int32_t pos_y);
					void	on_R_mouse_down(int32_t pos_x, int32_t pos_y);
					void	on_R_mouse_up(int32_t pos_x, int32_t pos_y);
					void	on_L_mouse_dclick(int32_t pos_x, int32_t pos_y);
					void	on_R_mouse_dclick(int32_t pos_x, int32_t pos_y);
					void	on_mouse_move(int32_t pos_x, int32_t pos_y);
					void	on_mouse_wheel(int32_t pos_x, int32_t pos_y, int32_t wheel_delta);

					void		on_info_xml(const uint8_t * msg, int32_t length);
					int32_t*	get_xml_msg(void);
					void		send_xml_msg(uint8_t * msg);

				private:
					void on_video_initialize(void * device, int32_t hwnd, int32_t smt, int32_t width, int32_t height);
					void on_video_receive(sirius::library::video::source::capturer::entity_t * captured);
					void on_video_release(void);

					int32_t		get_parent_pid(void);
					HWND		find_main_window(DWORD process_id);
					static BOOL is_main_window(HWND handle);
					static BOOL __stdcall	enum_windows_callback(HWND handle, LPARAM lParam);

					

				private:
					d3d11_video_source * _d3d11_video_source;
					host_video_source * _host_video_source;
					HANDLE _video_file;

					sirius::library::framework::server::web::context_t _context;
					sirius::library::unified::server::video_compressor_context_t _venc_context;

				private:
					struct st_xml_msg
					{
						int size;
						std::string msg;
					};
					std::queue<st_xml_msg> _xml_msg_que;

					sirius::library::misc::notification::internal::notifier * _notifier;
					sirius::library::unified::server::context_t * _unified_server_ctx;
					sirius::library::unified::server * _unified_server;

					uint8_t	_bfloat;
				};
			};
		};
	};
};

#endif
