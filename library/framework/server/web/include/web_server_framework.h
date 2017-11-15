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
					int32_t open(sirius::library::framework::server::web::context_t * context);
					int32_t close(void);
					int32_t play(void);
					int32_t pause(void);
					int32_t stop(void);
					int32_t state(void) const;

					int32_t seek(int32_t diff);
					int32_t seek_to(int32_t second);
					int32_t seek_stop(void);

					int32_t forward(void);
					int32_t backward(void);
					int32_t reverse(void);
					int32_t play_toggle(void);

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

					void	on_gyro(float x, float y, float z);
					void	on_pinch_zoom(float delta);
					void	on_gyro_attitude(float x, float y, float z, float w);
					void	on_gyro_gravity(float x, float y, float z);
					void	on_gyro_rotation_rate(float x, float y, float z);
					void	on_gyro_rotation_rate_unbiased(float x, float y, float z);
					void	on_gyro_user_acceleration(float x, float y, float z);

					void	on_gyro_enabled_attitude(bool state);
					void	on_gyro_enabled_gravity(bool state);
					void	on_gyro_enabled_rotation_rate(bool state);
					void	on_gyro_enabled_rotation_rate_unbiased(bool state);
					void	on_gyro_enabled_user_acceleration(bool state);
					void	on_gyro_updateinterval(float interval);

					void	on_infoxml(const char * msg, int32_t length);
					int*	get_xml_msg();
					void	send_xml_msg(char* msg);


					void		set_gyro_enabled(bool state);
					bool		get_gyro_enabled_attitude();
					void		set_gyro_enabled_attitude(bool state);
					bool		get_gyro_enabled_gravity();
					void		set_gyro_enabled_gravity(bool state);
					bool		get_gyro_enabled_rotation_rate();
					void		set_gyro_enabled_rotation_rate(bool state);
					bool		get_gyro_enabled_rotation_rate_unbiased();
					void		set_gyro_enabled_rotation_rate_unbiased(bool state);
					bool		get_gyro_enabled_user_acceleration();
					void		set_gyro_enabled_user_acceleration(bool state);
					float		get_gyro_updateinterval();
					void		set_gyro_updateinterval(float interval);
					gyro_data	get_attitude();
					gyro_data	get_gravity();
					gyro_data	get_rotation_rate();
					gyro_data	get_rotation_rate_unbiased();
					gyro_data	get_user_acceleration();

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
					bool _gyro_enabled_attitude;
					bool _gyro_enabled_gravity;
					bool _gyro_enabled_rotation_rate;
					bool _gyro_enabled_rotation_rate_unbiased;
					bool _gyro_enabled_user_acceleration;
					float _gyro_updateinterval;

					gyro_data _gyro_attitude;
					gyro_data _gyro_gravity;
					gyro_data _gyro_rotation_rate;
					gyro_data _gyro_rotation_rate_unbiased;
					gyro_data _gyro_user_acceleration;

					struct st_xml_msg
					{
						int size;
						std::string msg;
					};
					std::queue<st_xml_msg> _xml_msg_que;

					sirius::library::misc::notification::internal::notifier * _notifier;

					uint8_t	_bfloat;
				};
			};
		};
	};
};

#endif
