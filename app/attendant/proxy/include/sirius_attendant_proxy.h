#ifndef _SIRIUS_ATTENDANT_PROXY_H_
#define _SIRIUS_ATTENDANT_PROXY_H_

#include <sirius.h>

#if defined(EXPORT_ATTENDANT_PROXY)
#define EXP_ATTENDANT_PROXY_CLASS __declspec(dllexport)
#else
#define EXP_ATTENDANT_PROXY_CLASS __declspec(dllimport)
#endif


typedef void(*FuncPtrCallback)(uint8_t *, size_t);

namespace sirius
{
	namespace app
	{
		namespace attendant
		{
			class EXP_ATTENDANT_PROXY_CLASS proxy
				: public sirius::base
			{
				class core;
			public:
				static const int32_t STREAMER_PORTNUMBER = 7000;
				typedef struct _netstate_t
				{
					static const int32_t connecting = 0;
					static const int32_t connected = 1;
					static const int32_t disconnecting = 2;
					static const int32_t disconnected = 3;
				} netstate_t;

				typedef struct _mediastate_t
				{
					static const int32_t stopped = 0;
					static const int32_t playing = 1;
					static const int32_t paused = 2;
				} mediastate_t;

				typedef struct EXP_ATTENDANT_PROXY_CLASS _context_t
				{
					int32_t type;
					int32_t reconnect;
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
					int32_t video_quantization_colors;
					int32_t gpuindex;
					int32_t present;
					HWND	hwnd;
					void *	user_data;
					int32_t id;
					wchar_t uuid[MAX_PATH];
					int32_t controller_portnumber;
					int32_t play_after_connect;

					_context_t(void)
						: type(sirius::app::attendant::proxy::attendant_type_t::web)
						, reconnect(true)
						, video_codec(sirius::app::attendant::proxy::video_submedia_type_t::unknown)
						, video_width(1280)
						, video_height(720)
						, video_fps(0)
						, video_nbuffer(2)
						, video_process_type(sirius::app::attendant::proxy::video_memory_type_t::host)
						, video_block_width(128)
						, video_block_height(72)
						, video_compression_level(-1)
						, video_quantization_colors(128)
						, gpuindex(0)
						, present(false)
						, hwnd(NULL)
						, user_data(NULL)
						, id(0)
						, controller_portnumber(5000)
						, play_after_connect(0)
					{
						memset(url, 0x00, sizeof(url));
						memset(uuid, 0x00, sizeof(uuid));
					}

					~_context_t(void)
					{

					}
				} context_t;


				static bool parse_argument(int32_t argc, wchar_t * argv[], sirius::app::attendant::proxy::context_t * context);

				proxy(void);
				virtual ~proxy(void);

				virtual sirius::app::attendant::proxy::context_t * context(void);

				virtual bool	is_initialized(void);

				virtual int32_t initialize(void);
				virtual int32_t release(void);

				virtual int32_t connect(void);
				virtual int32_t disconnect(void);

				virtual int32_t play(void);
				virtual int32_t stop(void);

				virtual void connect_attendant_callback(int32_t code);
				virtual void disconnect_attendant_callback(void);
				virtual void start_attendant_callback(const char * client_uuid, const char * client_id);
				virtual void stop_attendant_callback(const char * client_uuid);
				virtual void destroy_callback(void);

				virtual void key_up_callback(int8_t type, int32_t key);
				virtual void key_down_callback(int8_t type, int32_t key);

				virtual void mouse_move_callback(int32_t pos_x, int32_t pos_y);
				virtual void mouse_lbd_callback(int32_t pos_x, int32_t pos_y);
				virtual void mouse_lbu_callback(int32_t pos_x, int32_t pos_y);
				virtual void mouse_rbd_callback(int32_t pos_x, int32_t pos_y);
				virtual void mouse_rbu_callback(int32_t pos_x, int32_t pos_y);
				virtual void mouse_lb_dclick_callback(int32_t pos_x, int32_t pos_y);
				virtual void mouse_rb_dclick_callback(int32_t pos_x, int32_t pos_y);
				virtual void mouse_wheel_callback(int32_t pos_x, int32_t pos_y, int32_t wheel_z);

				void app_to_attendant(uint8_t * packet, int32_t len);
				void attendant_to_app_callback(uint8_t * packet, int32_t len);
				void set_attendant_cb(FuncPtrCallback fncallback);

			private:
				sirius::app::attendant::proxy::context_t _context;
				sirius::app::attendant::proxy::core * _core;
				bool _key_pressed;
				bool _initialized;
			};
		};
	};
};

#endif