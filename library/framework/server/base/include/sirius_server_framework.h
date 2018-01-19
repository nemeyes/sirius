#ifndef _SIRIUS_SERVER_FRAMEWORK_H_
#define _SIRIUS_SERVER_FRAMEWORK_H_

#include <sirius.h>
#include <vector>
#include <string>

#include <sirius_internal_notifier.h>

namespace sirius
{
	namespace library
	{
		namespace framework
		{
			namespace server
			{
				class base
					: public sirius::base
				{
				public:
					typedef struct _state_t
					{
						static const int32_t unknown = 0;
						static const int32_t closed = 1;
						static const int32_t ready = 2;
						static const int32_t open_pending = 3;
						static const int32_t started = 4;
						static const int32_t paused = 5;
						static const int32_t stopped = 6;
						static const int32_t closing = 7;
					} state_t;

					typedef struct _context_t
					{
						wchar_t url[MAX_PATH];
						int32_t video_codec;
						int32_t video_width;
						int32_t video_height;
						int32_t video_fps;
						int32_t video_nbuffer;
						int32_t video_process_type;
						int32_t video_block_width;
						int32_t video_block_height;
						int32_t video_compression_level;
						int32_t video_qauntization_colors;
						bool	present;
						bool	keepalive;
						bool	tls;
						wchar_t uuid[MAX_PATH];
						int32_t portnumber;
						int32_t type;
						HWND	hwnd;
						void *	user_data;
						_context_t(void)
							: video_codec(sirius::library::framework::server::base::video_submedia_type_t::unknown)
							, video_width(0)
							, video_height(0)
							, video_fps(0)
							, video_nbuffer(2)
							, video_process_type(sirius::library::framework::server::base::video_memory_type_t::host)
							, video_block_width(0)
							, video_block_height(0)
							, video_compression_level(-1)
							, video_qauntization_colors(128)
							, present(false)
							, portnumber(7000)
							, hwnd(NULL)
							, type(sirius::library::framework::server::base::attendant_type_t::web) //web
							, user_data(NULL)
						{
							memset(url, 0x00, sizeof(url));
							memset(uuid, 0x00, sizeof(uuid));
						}

						~_context_t(void)
						{
						}
					} context_t;

					typedef struct _gpu_desc_t
					{
						char description[128];
						int32_t adaptor_index;
						uint32_t vendor_id;
						uint32_t device_id;
						uint32_t subsys_id;
						uint32_t revision;
						int32_t coord_left;
						int32_t coord_top;
						int32_t coord_right;
						int32_t coord_bottom;
						char luid[64];
						_gpu_desc_t(void)
							: adaptor_index(-1)
							, vendor_id(0)
							, device_id(0)
							, subsys_id(0)
							, revision(0)
							, coord_left(0)
							, coord_top(0)
							, coord_right(0)
							, coord_bottom(0)
						{
							memset(description, 0x00, sizeof(description));
							memset(luid, 0x00, sizeof(luid));
						}

						_gpu_desc_t(const _gpu_desc_t & clone)
						{
							strncpy_s(description, clone.description, sizeof(description));
							adaptor_index = clone.adaptor_index;
							vendor_id = clone.vendor_id;
							device_id = clone.device_id;
							subsys_id = clone.subsys_id;
							revision = clone.subsys_id;
							strncpy_s(luid, clone.luid, sizeof(luid));
							coord_left = clone.coord_left;
							coord_top = clone.coord_top;
							coord_right = clone.coord_right;
							coord_bottom = clone.coord_bottom;
						}

						_gpu_desc_t operator=(const _gpu_desc_t & clone)
						{
							strncpy_s(description, clone.description, sizeof(description));
							adaptor_index = clone.adaptor_index;
							vendor_id = clone.vendor_id;
							device_id = clone.device_id;
							subsys_id = clone.subsys_id;
							revision = clone.subsys_id;
							strncpy_s(luid, clone.luid, sizeof(luid));
							coord_left = clone.coord_left;
							coord_top = clone.coord_top;
							coord_right = clone.coord_right;
							coord_bottom = clone.coord_bottom;
							return (*this);
						}

					} gpu_desc_t;

					virtual void		set_notification_callee(sirius::library::misc::notification::internal::notifier::callee * callee) = 0;
					virtual int32_t		initialize(sirius::library::framework::server::base::context_t * context) = 0;
					virtual int32_t		release(void) = 0;
					virtual int32_t		play(void) = 0;
					virtual int32_t		pause(void) = 0;
					virtual int32_t		stop(void) = 0;
					virtual int32_t		state(void) const = 0;
					virtual void		on_keyup(int32_t key) = 0;
					virtual void		on_keydown(int32_t key) = 0;
					virtual void		on_L_mouse_down(int32_t pos_x, int32_t pos_y) = 0;
					virtual void		on_L_mouse_up(int32_t pos_x, int32_t pos_y) = 0;
					virtual void		on_R_mouse_down(int32_t pos_x, int32_t pos_y) = 0;
					virtual void		on_R_mouse_up(int32_t pos_x, int32_t pos_y) = 0;
					virtual void		on_L_mouse_dclick(int32_t pos_x, int32_t pos_y) = 0;
					virtual void		on_R_mouse_dclick(int32_t pos_x, int32_t pos_y) = 0;
					virtual void		on_mouse_move(int32_t pos_x, int32_t pos_y) = 0;
					virtual void		on_mouse_wheel(int32_t pos_x, int32_t pos_y, int32_t wheel_delta) = 0;
					virtual void		on_info_xml(const uint8_t * msg, int32_t length) = 0;
				};
			};
		};
	};
};

#endif
