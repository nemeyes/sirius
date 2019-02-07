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
				//static const int32_t STREAMER_PORTNUMBER = 7000;
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


				typedef struct EXP_ATTENDANT_PROXY_CLASS _png_compressor_context_t
				{
					int32_t video_compression_level;
					bool	video_quantization_posterization;
					bool	video_quantization_dither_map;
					bool	video_quantization_contrast_maps;
					int32_t video_quantization_colors;
					_png_compressor_context_t(void)
						: video_compression_level(-1)
						, video_quantization_posterization(true)
						, video_quantization_dither_map(false)
						, video_quantization_contrast_maps(false)
						, video_quantization_colors(128)
					{}
				} png_compressor_context_t;

				typedef struct EXP_ATTENDANT_PROXY_CLASS _webp_compressor_context_t
				{
					float	video_quality;
					int32_t	video_method;
					_webp_compressor_context_t(void)
						: video_quality(100.f)
						, video_method(1)
					{}
				} webp_compressor_context_t;

				typedef struct EXP_ATTENDANT_PROXY_CLASS _context_t
				{
					int32_t type;
					int32_t min_attendant_restart_threshold;
					int32_t max_attendant_restart_threshold;
					bool	reconnect;
					wchar_t url[MAX_PATH];
					int32_t video_codec;
					int32_t video_width;
					int32_t video_height;
					int32_t video_fps;
					int32_t video_nbuffer;
					int32_t video_process_type;
					int32_t video_block_width;
					int32_t video_block_height;
					png_compressor_context_t	png;
					webp_compressor_context_t	webp;
					bool	invalidate4client;
					bool	indexed_mode;
					int32_t	nthread;

					bool	double_reloading_on_creating;
					bool	reloading_on_disconnecting;

					bool	present;
					bool	caching;
					bool	keepalive;
					int32_t keepalive_timeout;
					bool	tls;
					HWND	hwnd;
					void *	user_data;
					int32_t id;
					wchar_t uuid[MAX_PATH];
					int32_t controller_portnumber;
					int32_t streamer_portnumber;
					int32_t play_after_connect;

					_context_t(void)
						: type(sirius::app::attendant::proxy::attendant_type_t::web)
						, min_attendant_restart_threshold(0)
						, max_attendant_restart_threshold(0)
						, reconnect(true)
						, video_codec(sirius::app::attendant::proxy::video_submedia_type_t::unknown)
						, video_width(1280)
						, video_height(720)
						, video_fps(0)
						, video_nbuffer(2)
						, video_process_type(sirius::app::attendant::proxy::video_memory_type_t::host)
						, video_block_width(128)
						, video_block_height(72)
						, invalidate4client(false)
						, indexed_mode(false)
						, nthread(20)
						, double_reloading_on_creating(false)
						, reloading_on_disconnecting(false)
						, present(false)
						, caching(false)
						, keepalive(false)
						, keepalive_timeout(5000)
						, tls(false)
						, hwnd(NULL)
						, user_data(NULL)
						, id(0)
						, controller_portnumber(5000)
						, streamer_portnumber(7000)
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
				virtual void stop_attendant_callback(const char * client_uuid, bool * reload);
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
				virtual void finish_reload(void);
				void app_to_attendant(uint8_t * packet, int32_t len, int32_t mode);
				void attendant_to_app_callback(uint8_t * packet, int32_t len);
				void sync_attendant_to_app_callback(uint8_t * packet, int32_t len);
				void set_attendant_cb(FuncPtrCallback fncallback);
				void set_sync_attendant_cb(FuncPtrCallback fncallback);

			private:
				sirius::app::attendant::proxy::context_t _context;
				sirius::app::attendant::proxy::core * _core;
				bool _key_pressed;
				bool _initialized;
				bool is_reload;
			};
		};
	};
};

#endif