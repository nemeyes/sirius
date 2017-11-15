#ifndef _DDRAW_RENDERER_H_
#define _DDRAW_RENDERER_H_

#include "sirius_ddraw_renderer.h"
#include <ddraw.h>

namespace sirius
{
	namespace library
	{
		namespace video
		{
			namespace sink
			{
				namespace ddraw
				{
					class renderer::core
					{
					public:
						core(void);
						~core(void);

						bool is_initialized(void);

						int32_t initialize(sirius::library::video::sink::ddraw::renderer::context_t * context);
						int32_t release(void);
						int32_t render(sirius::library::video::sink::ddraw::renderer::entity_t * decoded);

						int32_t open(void);
						int32_t close(void);

					private:
						int32_t create(HWND hwnd, DWORD width, DWORD height, DWORD pos_x, DWORD pos_y, DWORD *rgb_bitcount = 0);
						int32_t destroy(void);


						int32_t enable_time_text(bool enable = true);
						int32_t set_time_text_position(unsigned short x, unsigned short y);
						int32_t set_time_text_font_size(unsigned char size);
						int32_t set_time_text_color(unsigned char red, unsigned char green, unsigned char blue);

						int32_t enable_osd_text(bool enable = true);
						int32_t set_osd_text(wchar_t *osd);
						int32_t set_osd_text_position(unsigned short x, unsigned short y);
						int32_t set_osd_text_font_size(unsigned char size);
						int32_t set_osd_text_color(unsigned char red, unsigned char green, unsigned char blue);

						int32_t set_background_color(unsigned char red, unsigned char green, unsigned char blue);

						int32_t set_normal_screen_handle(HWND hwnd);
						int32_t set_full_screen_handle(HWND hwnd);

						int32_t enable_full_screen(bool enable);
						int32_t enable_stretch(bool enable);

						int32_t set_enable(bool enable);
						bool get_enable(void);

						int32_t make_normal_screen_display_size(int32_t & display_width, int32_t & display_height, int32_t & display_x, int32_t & display_y);
						int32_t make_full_screen_display_size(int32_t & display_width, int32_t & display_height, int32_t & display_x, int32_t & display_y);


					private:
						sirius::library::video::sink::ddraw::renderer::context_t * _context;

						bool		_enable;
						bool		_is_initialized;
						HINSTANCE	_library;
						HWND		_draw;

						LPDIRECTDRAW7			_pdd;
						LPDIRECTDRAWSURFACE7	_pdd_primary;
						LPDIRECTDRAWSURFACE7	_pdd_video;
						LPDIRECTDRAWSURFACE7	_pdd_rgb;
						LPDIRECTDRAWSURFACE7	_copy_video;
						LPDIRECTDRAWCLIPPER		_pdd_cliper;

						DWORD		_rgb_bitcount;
						DWORD		_out_rgb_bitcount;

						DWORD		_display_width;
						DWORD		_display_height;

						bool		_is_video_stretch;
						bool		_is_video_fullscreen;

						HWND		_noraml_screen_hwnd;
						HWND		_full_screen_hwnd;

						bool		_enable_time_text;
						wchar_t		_time[100];
						COLORREF	_time_text_shadow_color;
						uint16_t	_time_text_position_x;
						uint16_t	_time_text_position_y;
						uint8_t		_time_text_font_size;
						uint8_t		_time_text_color_red;
						uint8_t		_time_text_color_green;
						uint8_t		_time_text_color_blue;

						bool		_enable_osd_text;
						wchar_t		_osd[100];
						COLORREF	_osd_text_shadow_color;
						uint16_t	_osd_text_position_x;
						uint16_t	_osd_text_position_y;
						uint8_t		_osd_text_font_size;
						uint8_t		_osd_text_color_red;
						uint8_t		_osd_text_color_green;
						uint8_t		_osd_text_color_blue;
						uint8_t		_background_red;
						uint8_t		_background_green;
						uint8_t		_background_blue;
					};
				};
			};
		};
	};
};

#endif