#pragma once

#include "ofMain.h"
#include "ofxDatGui.h"

class ofApp : public ofBaseApp
{
public:
	void mousePressed(int x, int y, int button);
	void mouseDragged(int x, int y, int button);
	void setup(void);
	void exit(void);
	void draw(void);
	void update(void);


	void onMouseDragged(ofMouseEventArgs & mouse);
	void onButtonCloseEvent(ofxDatGuiButtonEvent e);
	void onScrollViewSessionEvent(ofxDatGuiScrollViewEvent e);

	bool mFullscreen;
	void refreshWindow();
	void toggleFullscreen();

	void keyPressed(int key);
	void onButtonEvent(ofxDatGuiButtonEvent e);
	void onToggleEvent(ofxDatGuiToggleEvent e);
	void onSliderEvent(ofxDatGuiSliderEvent e);
	void onTextInputEvent(ofxDatGuiTextInputEvent e);
	void on2dPadEvent(ofxDatGui2dPadEvent e);
	void onDropdownEvent(ofxDatGuiDropdownEvent e);
	void onColorPickerEvent(ofxDatGuiColorPickerEvent e);
	void onMatrixEvent(ofxDatGuiMatrixEvent e);


private:
	static const int32_t window_width = 1280;
	static const int32_t window_height = 800;
	static const int32_t count_of_spectrum = 256;
	static const int32_t count_of_cloud_point = 500;
	static const int32_t radius_band_index_in_spectrum = 2;
	static const int32_t velocity_band_index_in_spectrum = 100;

	int32_t					_mouse_pressed_pos_x;
	int32_t					_mouse_pressed_pos_y;

	ofTrueTypeFont			_font;
	ofxDatGuiTheme *		_theme;
	ofxDatGuiTheme *		_theme_close;
	ofxDatGuiButton *		_close;

	ofxDatGuiTextInput *	_uuid;
	ofxDatGuiBreak *		_break_uuid;
	ofxDatGuiTextInput *	_port;
	ofxDatGuiBreak *		_break_port;
	ofxDatGuiToggle	*		_display;
	ofxDatGuiBreak *		_break_display;
	ofxDatGuiButton *		_config;
	ofxDatGuiBreak *		_break_config;
	ofxDatGuiButton *		_start;
	ofxDatGuiBreak *		_break_start;
	ofxDatGuiButton *		_stop;

	ofxDatGuiTextInput *	_cpu;
	ofxDatGuiBreak *		_break_cpu;
	ofxDatGuiTextInput *	_memory;
	ofxDatGuiBreak *		_break_memory;
	ofxDatGuiDropdown *		_gpus;

	ofxDatGuiSlider *		_cpu_usage;
	ofxDatGuiBreak *		_break_cpu_usage;
	ofxDatGuiSlider *		_memory_usage;

	ofxDatGuiLabel *		_session_no_label;
	ofxDatGuiLabel *		_sssion_client_label;
	ofxDatGuiLabel *		_sssion_attendant_label;
	ofxDatGuiLabel *		_sssion_state_label;
	ofxDatGuiLabel *		_sssion_time_label;
	ofxDatGuiLabel *		_sssion_attendant_uuid_label;
	ofxDatGuiLabel *		_sssion_device_id_label;
	ofxDatGuiLabel *		_sssion_cpu_usage_label;

	ofxDatGuiScrollView *	_sssion_no;
	ofxDatGuiScrollView *	_sssion_client;
	ofxDatGuiScrollView *	_sssion_slot;
	ofxDatGuiScrollView *	_sssion_state;
	ofxDatGuiScrollView *	_sssion_time;
	ofxDatGuiScrollView *	_sssion_attendant_uuid;
	ofxDatGuiScrollView *	_sssion_device_id;
	ofxDatGuiScrollView *	_sssion_cpu_usage;

	ofImage					_logo;
	ofImage					_key_visual;

	ofSoundPlayer			_sound;	//Sound sample
	float					_spectrum[count_of_spectrum];
	float					_radius; //500.f;
	float					_velocity; //0.1f

	float					_tx[count_of_cloud_point];
	float					_ty[count_of_cloud_point];
	ofPoint					_cloud_point[count_of_cloud_point];
	float					_time0;
	float					_bg_transparent;
};
