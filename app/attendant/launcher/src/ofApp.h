#pragma once

#include <sirius.h>
#include <sdkddkver.h>
#include <process.h>
//#include <atlbase.h>
#include <dxgi1_3.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <rpcdce.h>
#include <algorithm>
#include <memory>
#include <string>
#include <map>

#include <sirius_stringhelper.h>

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxDatGui.h"

class ofApp : public ofBaseApp
{
public:
	void mousePressed(int x, int y, int button);
	void mouseDragged(int x, int y, int button);
	void onMouseDragged(ofMouseEventArgs & mouse);

	typedef struct _gpudesc_t
	{
		char		description[128];
		int32_t		adaptorIndex;
		uint32_t	vendorId;
		uint32_t	deviceId;
		uint32_t	subsysId;
		uint32_t	revision;
		int32_t		coordLeft;
		int32_t		coordTop;
		int32_t		coordRight;
		int32_t		coordBottom;
		char		luid[64];
		_gpudesc_t(void)
			: adaptorIndex(-1)
			, vendorId(0)
			, deviceId(0)
			, subsysId(0)
			, revision(0)
			, coordLeft(0)
			, coordTop(0)
			, coordRight(0)
			, coordBottom(0)
		{
			memset(description, 0x00, sizeof(description));
			memset(luid, 0x00, sizeof(luid));
		}

		_gpudesc_t(const _gpudesc_t & clone)
		{
			strncpy_s(description, clone.description, sizeof(description));
			adaptorIndex = clone.adaptorIndex;
			vendorId = clone.vendorId;
			deviceId = clone.deviceId;
			subsysId = clone.subsysId;
			revision = clone.revision;
			strncpy_s(luid, clone.luid, sizeof(luid));
			coordLeft = clone.coordLeft;
			coordTop = clone.coordTop;
			coordRight = clone.coordRight;
			coordBottom = clone.coordBottom;
		}

		_gpudesc_t operator=(const _gpudesc_t & clone)
		{
			strncpy_s(description, clone.description, sizeof(description));
			adaptorIndex = clone.adaptorIndex;
			vendorId = clone.vendorId;
			deviceId = clone.deviceId;
			subsysId = clone.subsysId;
			revision = clone.revision;
			strncpy_s(luid, clone.luid, sizeof(luid));
			coordLeft = clone.coordLeft;
			coordTop = clone.coordTop;
			coordRight = clone.coordRight;
			coordBottom = clone.coordBottom;
			return (*this);
		}
	} gpudesc_t;

	void on_close_event(ofxDatGuiButtonEvent e);

	void on_reconnect_change_event(ofxDatGuiToggleEvent e);
	void on_inject_hook_change_event(ofxDatGuiToggleEvent e);
	void on_play_after_connect_change_event(ofxDatGuiToggleEvent e);
	void on_url_change_event(ofxDatGuiTextInputEvent e);
	void on_uuid_change_event(ofxDatGuiTextInputEvent e);
	void on_client_uuid_change_event(ofxDatGuiTextInputEvent e);
	void on_ctrlsrv_addr_change_event(ofxDatGuiTextInputEvent e);
	void on_ctrlsrv_port_change_event(ofxDatGuiTextInputEvent e);
	void on_strmsrv_addr_change_event(ofxDatGuiTextInputEvent e);
	void on_strmsrv_port_change_event(ofxDatGuiTextInputEvent e);
	void on_video_width_change_event(ofxDatGuiTextInputEvent e);
	void on_video_height_change_event(ofxDatGuiTextInputEvent e);
	void on_video_fps_change_event(ofxDatGuiTextInputEvent e);
	void on_video_present_change_event(ofxDatGuiToggleEvent e);
	void on_video_repeat_change_event(ofxDatGuiToggleEvent e);

	void retreieve_gpu_info(gpudesc_t * adapters, size_t capacity, size_t & count);

	void on_gen_uuid_event(ofxDatGuiButtonEvent e);
	void on_gen_client_uuid_event(ofxDatGuiButtonEvent e);

	void on_create_process_suspend_event(ofxDatGuiButtonEvent e);
	void on_resume_event(ofxDatGuiButtonEvent e);
	void on_unload_hook_dll_event(ofxDatGuiButtonEvent e);

	void setup(void);
	void exit(void);
	void draw(void);
	void update(void);

private:
	void make_command_line(void);
	HMODULE inject(DWORD ProcessID, char * dllName);

private:
	static const int32_t window_width = 1280;
	static const int32_t window_height = 800;
	static const int32_t count_of_spectrum = 256;
	static const int32_t count_of_cloud_point = 500;
	static const int32_t radius_band_index_in_spectrum = 2;
	static const int32_t velocity_band_index_in_spectrum = 100;

	ofImage					_logo;
	ofImage					_key_visual;
	ofSoundPlayer			_sound;
	float					_spectrum[count_of_spectrum];
	float					_radius;
	float					_velocity;
	float					_tx[count_of_cloud_point];
	float					_ty[count_of_cloud_point];
	ofPoint					_cloud_point[count_of_cloud_point];
	float					_time0;
	float					_bg_transparent;


	int32_t					_mouse_pressed_pos_x;
	int32_t					_mouse_pressed_pos_y;

	ofTrueTypeFont			_font;
	ofxDatGuiTheme *		_theme;
	ofxDatGuiTheme *		_theme_close;
	ofxDatGuiButton *		_close;

	ofxDatGuiTextInput *	_app_path;
	ofxDatGuiBreak *		_break_app_path;
	ofxDatGuiTextInput *	_command_line;

	ofxDatGui *				_controller_streamer_option;
	ofxDatGui *				_video_gpu_option;

	ofxDatGuiToggle *		_reconnect;
	ofxDatGuiToggle *		_inject_hook;
	ofxDatGuiToggle *		_play_after_connect;
	ofxDatGuiTextInput *	_url;
	ofxDatGuiTextInput *	_uuid;
	ofxDatGuiTextInput *	_client_uuid;
	ofxDatGuiTextInput *	_ctrlsrv_addr;
	ofxDatGuiTextInput *	_ctrlsrv_port;
	ofxDatGuiTextInput *	_strmsrv_addr;
	ofxDatGuiTextInput *	_strmsrv_port;
	ofxDatGuiTextInput *	_video_width;
	ofxDatGuiTextInput *	_video_height;
	ofxDatGuiTextInput *	_video_fps;
	ofxDatGuiToggle *		_video_present;
	ofxDatGuiToggle *		_video_repeat;
	ofxDatGuiDropdown *		_gpus;

	ofxDatGuiButton *		_gen_uuid_btn;
	ofxDatGuiButton *		_gen_client_uuid_btn;
	ofxDatGuiButton *		_create_process_btn;
	ofxDatGuiButton *		_resume_btn;
	ofxDatGuiButton *		_unload_hook_dll_btn;

	PROCESS_INFORMATION		_process_info;
	HANDLE					_attendant_process_handle;
	DWORD					_attendant_process_id;

	std::string				_command_line_str;
};
