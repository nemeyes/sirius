#include "ofApp.h"


void ofApp::onMouseDragged(ofMouseEventArgs & mouse)
{
	//mouse.


	//if ((_count % 2) == 0)
	if (mouse.type == ofMouseEventArgs::Dragged)
	{
		ofVec2f pos;
#if 1
		//ofSetWindowPosition(pos.x, pos.y);
		char debug[500];
		snprintf(debug, sizeof(debug), "mouse.x[%.4f], mouse.y[%.4f]\n", pos.x, pos.y);
		::OutputDebugStringA(debug);
#else
		ofSetWindowPosition(mouse.x, mouse.y);
		char debug[500];
		snprintf(debug, sizeof(debug), "mouse.x[%.4f], mouse.y[%.4f]\n", mouse.x, mouse.y);
		::OutputDebugStringA(debug);
#endif
	}
}

void ofApp::onButtonCloseEvent(ofxDatGuiButtonEvent e)
{
	HWND hwnd = ofGetWin32Window();
	::SendMessage(hwnd, WM_KEYDOWN, VK_ESCAPE, NULL);
	::SendMessage(hwnd, WM_KEYUP, VK_ESCAPE, NULL);
}

void ofApp::onScrollViewSessionEvent(ofxDatGuiScrollViewEvent e)
{

}

void ofApp::mousePressed(int x, int y, int button)
{
	if (button == 0) // 0 means mouse left button
	{
		_mouse_pressed_pos_x = x;
		_mouse_pressed_pos_y = y;
	}
}

void ofApp::mouseDragged(int x, int y, int button)
{
	if (button == 0) // 0 means mouse left button
	{
		int32_t pos_x = ofGetWindowPositionX() + x - _mouse_pressed_pos_x;
		int32_t pos_y = ofGetWindowPositionY() + y - _mouse_pressed_pos_y;
		ofSetWindowPosition(pos_x, pos_y);
	}
}

void ofApp::setup(void)
{
	_radius = 500.f;
	_velocity = 0.1f;
	_time0 = 0;
	_bg_transparent = 255;

	_logo.load("img/company.png");
	//ofSetBackgroundAuto(false);
	ofSetFrameRate(24);
	ofEnableAlphaBlending();
	ofSetWindowPosition((ofGetScreenWidth() / 2) - (ofApp::window_width / 2), (ofGetScreenHeight() / 2) - (ofApp::window_height / 2));
	//ofAddListener(ofEvents().mouseDragged, this, &ofApp::onMouseDragged);
	//ofAddListener(ofEvents().mouse, this, &ofApp::onMouseDragged);

	//Set up sound sample
	_sound.loadSound("zvezda.mp3", false);
	_sound.setLoop(true);
	_sound.play();
	_sound.setVolume(0);

	//Set spectrum values to 0
	for (int i = 0; i<count_of_spectrum; i++)
	{
		_spectrum[i] = 0.0f;
	}

	//Initialize points offsets by random numbers
	for (int j = 0; j<count_of_cloud_point; j++)
	{
		_tx[j] = ofRandom(0, 1000);
		_ty[j] = ofRandom(0, 1000);
	}

	int32_t pos_x = 25;
	int32_t pos_y = _logo.getHeight();

	_theme = new ofxDatGuiTheme(true);// ofxDatGuiThemeEntrix();
	_theme_close = new ofxDatGuiThemeEntrix();

	_font.load("ofxbraitsch/fonts/NotoSans-Bold.ttf", 15);

	_close = new ofxDatGuiButton("x");
	_close->setTheme(_theme_close);
	_close->setWidth(_close->getHeight());
	_close->onButtonEvent(this, &ofApp::onButtonCloseEvent);
	_close->setLabelUpperCase(false);
	_close->setPosition(ofGetWidth() - _close->getWidth(), 0);

	_uuid = new ofxDatGuiTextInput("uuid", "9701AE27-7655-41CA-ADEA-F8AEA3BD645C");
	_uuid->setTheme(_theme);
	_uuid->setWidth(350, 70);
	_uuid->setOpacity(.5f);
	_uuid->setPosition(pos_x, pos_y);
	_uuid->setLabelUpperCase(false);
	pos_y += _uuid->getHeight();

	_break_uuid = new ofxDatGuiBreak();
	_break_uuid->setTheme(_theme);
	_break_uuid->setWidth(350, 70);
	_break_uuid->setHeight(1);
	_break_uuid->setOpacity(.5f);
	_break_uuid->setPosition(pos_x, pos_y);
	_break_uuid->setBackgroundColor({ 0x55, 0x55, 0x55 });
	pos_y += _break_uuid->getHeight();

	_port = new ofxDatGuiTextInput("Port", "5000");
	_port->setTheme(_theme);
	_port->setWidth(350, 70);
	_port->setOpacity(.5f);
	_port->setPosition(pos_x, pos_y);
	_port->setLabelUpperCase(false);
	pos_y += _port->getHeight();

	_break_port = new ofxDatGuiBreak();
	_break_port->setTheme(_theme);
	_break_port->setWidth(350, 70);
	_break_port->setHeight(1);
	_break_port->setOpacity(.5f);
	_break_port->setPosition(pos_x, pos_y);
	_break_port->setBackgroundColor({ 0x55, 0x55, 0x55 });
	pos_y += _break_port->getHeight();

	_display = new ofxDatGuiToggle("Display", false);
	_display->setTheme(_theme);
	_display->setWidth(350);
	_display->setOpacity(.5f);
	_display->setPosition(pos_x, pos_y);
	_display->setLabelUpperCase(false);
	pos_y += _display->getHeight();

	_break_display = new ofxDatGuiBreak();
	_break_display->setTheme(_theme);
	_break_display->setWidth(350, 70);
	_break_display->setHeight(1);
	_break_display->setOpacity(.5f);
	_break_display->setPosition(pos_x, pos_y);
	_break_display->setBackgroundColor({ 0x55, 0x55, 0x55 });
	pos_y += _break_display->getHeight();

	pos_x = 25;
	_config = new ofxDatGuiButton("Configuration");
	_config->setTheme(_theme);
	_config->setWidth(116);
	_config->setOpacity(.5f);
	_config->setPosition(pos_x, pos_y);
	_config->setLabelUpperCase(false);
	_config->setLabelAlignment(ofxDatGuiAlignment::CENTER);
	pos_x += _config->getWidth();

	_break_config = new ofxDatGuiBreak();
	_break_config->setTheme(_theme);
	_break_config->setWidth(1, 1);
	_break_config->setHeight(_config->getHeight());
	_break_config->setOpacity(.5f);
	_break_config->setPosition(pos_x, pos_y);
	_break_config->setBackgroundColor({ 0x55, 0x55, 0x55 });
	pos_x += _break_config->getWidth();

	_start = new ofxDatGuiButton("Start");
	_start->setTheme(_theme);
	_start->setWidth(116);
	_start->setOpacity(.5f);
	_start->setPosition(pos_x, pos_y);
	_start->setLabelUpperCase(false);
	_start->setLabelAlignment(ofxDatGuiAlignment::CENTER);
	pos_x += _start->getWidth();

	_break_start = new ofxDatGuiBreak();
	_break_start->setTheme(_theme);
	_break_start->setWidth(1, 1);
	_break_start->setHeight(_config->getHeight());
	_break_start->setOpacity(.5f);
	_break_start->setPosition(pos_x, pos_y);
	_break_start->setBackgroundColor({ 0x55, 0x55, 0x55 });
	pos_x += _break_start->getWidth();

	_stop = new ofxDatGuiButton("Stop");
	_stop->setTheme(_theme);
	_stop->setWidth(116);
	_stop->setOpacity(.5f);
	_stop->setPosition(pos_x, pos_y);
	_stop->setLabelUpperCase(false);
	_stop->setLabelAlignment(ofxDatGuiAlignment::CENTER);

	//pos_x = (ofApp::window_width / 2) - (_break_start->getWidth() / 2);
	pos_y = _logo.getHeight();

	_cpu = new ofxDatGuiTextInput("CPU", "Intel(R) Xeon(R) CPU E5 - 2630 v4 @ 2.20GHz");
	_cpu->setTheme(_theme);
	_cpu->setWidth(350, 100);
	_cpu->setOpacity(.5f);
	_cpu->setPosition((ofApp::window_width / 2) - (_cpu->getWidth() / 2), pos_y);
	_cpu->setLabelUpperCase(false);
	pos_y += _cpu->getHeight();
	pos_x = _cpu->getX();

	_break_cpu = new ofxDatGuiBreak();
	_break_cpu->setTheme(_theme);
	_break_cpu->setWidth(350, 70);
	_break_cpu->setHeight(1);
	_break_cpu->setOpacity(.5f);
	_break_cpu->setPosition(pos_x, pos_y);
	_break_cpu->setBackgroundColor({ 0x55, 0x55, 0x55 });
	pos_y += _break_cpu->getHeight();

	_memory = new ofxDatGuiTextInput("Memory", "67108 MB");
	_memory->setTheme(_theme);
	_memory->setWidth(350, 100);
	_memory->setOpacity(.5f);
	_memory->setPosition(pos_x, pos_y);
	_memory->setLabelUpperCase(false);
	pos_y += _memory->getHeight();

	_break_memory = new ofxDatGuiBreak();
	_break_memory->setTheme(_theme);
	_break_memory->setWidth(350, 70);
	_break_memory->setHeight(1);
	_break_memory->setOpacity(.5f);
	_break_memory->setPosition(pos_x, pos_y);
	_break_memory->setBackgroundColor({ 0x55, 0x55, 0x55 });
	pos_y += _break_memory->getHeight();

	std::vector<std::string> gpus_opts = { "0. NVIDIA Quadro M2000(10DE 1430 - 10DE 1190)",
		"1. NVIDIA Quadro M2000(10DE 1430 - 10DE 1190)",
		"2. NVIDIA Quadro M2000(10DE 1430 - 10DE 1190)",
		"3. NVIDIA Quadro M2000(10DE 1430 - 10DE 1190)",
		"4. NVIDIA Quadro M2000(10DE 1430 - 10DE 1190)",
		"5. NVIDIA Quadro M2000(10DE 1430 - 10DE 1190)",
		"6. NVIDIA Quadro M2000(10DE 1430 - 10DE 1190)",
		"7. NVIDIA Quadro M2000(10DE 1430 - 10DE 1190)",
		"8. Microsoft Basic Render Driver(1414 8C - 0 0)",
		"9. Microsoft Basic Render Driver(1414 8C - 0 0)" };
	_gpus = new ofxDatGuiDropdown("GPUs", gpus_opts);
	_gpus->setTheme(_theme);
	_gpus->setWidth(350, 100);
	_gpus->setOpacity(.5f);
	_gpus->setPosition(pos_x, pos_y);
	_gpus->setLabelUpperCase(false);
	for (int32_t index = 0; index < gpus_opts.size(); index++)
	{
		ofxDatGuiDropdownOption * opt = _gpus->getChildAt(index);
		opt->setLabelAlignment(ofxDatGuiAlignment::RIGHT);
	}

	pos_y = _logo.getHeight();
	_cpu_usage = new ofxDatGuiSlider("CPU Usage", 0.f, 100.f);
	_cpu_usage->setTheme(_theme);
	_cpu_usage->setWidth(350, 100);
	_cpu_usage->setOpacity(.5f);
	_cpu_usage->setPosition(ofGetWidth() - _cpu_usage->getWidth() - 25, pos_y);
	_cpu_usage->setLabelUpperCase(false);
	pos_x = _cpu_usage->getX();
	pos_y += _cpu_usage->getHeight();

	_break_cpu_usage = new ofxDatGuiBreak();
	_break_cpu_usage->setTheme(_theme);
	_break_cpu_usage->setWidth(350, 70);
	_break_cpu_usage->setHeight(1);
	_break_cpu_usage->setOpacity(.5f);
	_break_cpu_usage->setPosition(pos_x, pos_y);
	_break_cpu_usage->setBackgroundColor({ 0x55, 0x55, 0x55 });
	pos_y += _break_cpu_usage->getHeight();

	_memory_usage = new ofxDatGuiSlider("Memory Usage", 0.f, 100.f);
	_memory_usage->setTheme(_theme);
	_memory_usage->setWidth(350, 100);
	_memory_usage->setOpacity(.5f);
	_memory_usage->setPosition(pos_x, pos_y);
	_memory_usage->setLabelUpperCase(false);

	pos_x = 25;
	_sssion_no = new ofxDatGuiScrollView("No", 10);
	_sssion_no->setTheme(_theme);
	_sssion_no->setWidth(50);
	_sssion_no->setLabelAlignment(ofxDatGuiAlignment::CENTER);
	_sssion_no->setOpacity(.5f);
	_sssion_no->setPosition(pos_x, ofGetHeight() - (_sssion_no->getHeight() + 25));
	_sssion_no->add("1");
	_sssion_no->add("2");
	_sssion_no->add("3");
	_sssion_no->add("4");
	_sssion_no->add("5");
	_sssion_no->add("6");
	_sssion_no->add("7");
	_sssion_no->add("8");
	_sssion_no->add("9");
	_sssion_no->add("10");
	_sssion_no->add("11");
	_sssion_no->add("12");
	_sssion_no->add("13");
	_sssion_no->add("14");
	_sssion_no->add("15");
	_sssion_no->add("16");
	_sssion_no->add("17");
	_sssion_no->add("18");
	_sssion_no->add("19");
	_sssion_no->add("20");
	_sssion_no->add("21");
	_sssion_no->add("22");
	_sssion_no->add("23");
	_sssion_no->add("24");
	_sssion_no->add("25");
	_sssion_no->add("26");
	//_sssion_no->setLabelUpperCase(false);
	pos_x += _sssion_no->getWidth();
	pos_x += 10;
	pos_y = ofGetHeight() - (_sssion_no->getHeight() + 25);

	_sssion_client = new ofxDatGuiScrollView("Client", 10);
	_sssion_client->setTheme(_theme_close);
	_sssion_client->setWidth(100);
	_sssion_client->setLabelAlignment(ofxDatGuiAlignment::CENTER);
	_sssion_client->setOpacity(.5f);
	_sssion_client->setPosition(pos_x, pos_y);
	_sssion_client->add("connected");
	_sssion_client->add("connected");
	_sssion_client->add("connected");
	_sssion_client->add("connected");
	_sssion_client->add("connected");
	_sssion_client->add("connected");
	_sssion_client->add("connected");
	_sssion_client->add("connected");
	_sssion_client->add("connected");
	_sssion_client->add("connected");
	_sssion_client->add("connected");
	_sssion_client->add("connected");
	_sssion_client->add("connected");
	_sssion_client->add("connected");
	_sssion_client->add("connected");
	_sssion_client->add("connected");
	_sssion_client->add("connected");
	_sssion_client->add("connected");
	_sssion_client->add("connected");
	_sssion_client->add("connected");
	_sssion_client->add("connected");
	_sssion_client->add("connected");
	_sssion_client->add("connected");
	_sssion_client->add("connected");
	_sssion_client->add("connected");
	_sssion_client->add("connected");
	//_sssion_client->setLabelUpperCase(false);
	pos_x += _sssion_client->getWidth();
	pos_x += 10;

	_sssion_slot = new ofxDatGuiScrollView("attendant", 10);
	_sssion_slot->setTheme(_theme_close);
	_sssion_slot->setWidth(100);
	_sssion_slot->setLabelAlignment(ofxDatGuiAlignment::CENTER);
	_sssion_slot->setOpacity(.5f);
	_sssion_slot->setPosition(pos_x, pos_y);
	_sssion_slot->add("connected");
	_sssion_slot->add("connected");
	_sssion_slot->add("connected");
	_sssion_slot->add("connected");
	_sssion_slot->add("connected");
	_sssion_slot->add("connected");
	_sssion_slot->add("connected");
	_sssion_slot->add("connected");
	_sssion_slot->add("connected");
	_sssion_slot->add("connected");
	_sssion_slot->add("connected");
	_sssion_slot->add("connected");
	_sssion_slot->add("connected");
	_sssion_slot->add("connected");
	_sssion_slot->add("connected");
	_sssion_slot->add("connected");
	_sssion_slot->add("connected");
	_sssion_slot->add("connected");
	_sssion_slot->add("connected");
	_sssion_slot->add("connected");
	_sssion_slot->add("connected");
	_sssion_slot->add("connected");
	_sssion_slot->add("connected");
	_sssion_slot->add("connected");
	_sssion_slot->add("connected");
	_sssion_slot->add("connected");
	//_sssion_slot->setLabelUpperCase(false);
	pos_x += _sssion_slot->getWidth();
	pos_x += 10;

	_sssion_state = new ofxDatGuiScrollView("State", 10);
	_sssion_state->setTheme(_theme_close);
	_sssion_state->setWidth(150);
	_sssion_state->setLabelAlignment(ofxDatGuiAlignment::CENTER);
	_sssion_state->setOpacity(.5f);
	_sssion_state->setPosition(pos_x, pos_y);
	_sssion_state->add("running");
	_sssion_state->add("running");
	_sssion_state->add("running");
	_sssion_state->add("running");
	_sssion_state->add("running");
	_sssion_state->add("running");
	_sssion_state->add("running");
	_sssion_state->add("running");
	_sssion_state->add("running");
	_sssion_state->add("running");
	_sssion_state->add("running");
	_sssion_state->add("running");
	_sssion_state->add("running");
	_sssion_state->add("running");
	_sssion_state->add("running");
	_sssion_state->add("running");
	_sssion_state->add("running");
	_sssion_state->add("running");
	_sssion_state->add("running");
	_sssion_state->add("running");
	_sssion_state->add("running");
	_sssion_state->add("running");
	_sssion_state->add("running");
	_sssion_state->add("running");
	_sssion_state->add("running");
	_sssion_state->add("running");
	//_sssion_state->setLabelUpperCase(false);
	pos_x += _sssion_state->getWidth();
	pos_x += 10;

	_sssion_time = new ofxDatGuiScrollView("Time", 10);
	_sssion_time->setTheme(_theme_close);
	_sssion_time->setWidth(100);
	_sssion_time->setLabelAlignment(ofxDatGuiAlignment::CENTER);
	_sssion_time->setOpacity(.5f);
	_sssion_time->setPosition(pos_x, pos_y);
	_sssion_time->add("22::19::31");
	_sssion_time->add("22::19::32");
	_sssion_time->add("22::19::33");
	_sssion_time->add("22::19::34");
	_sssion_time->add("22::19::35");
	_sssion_time->add("22::19::36");
	_sssion_time->add("22::19::37");
	_sssion_time->add("22::19::38");
	_sssion_time->add("22::19::39");
	_sssion_time->add("22::19::40");
	_sssion_time->add("22::19::41");
	_sssion_time->add("22::19::42");
	_sssion_time->add("22::19::43");
	_sssion_time->add("22::19::44");
	_sssion_time->add("22::19::45");
	_sssion_time->add("22::19::46");
	_sssion_time->add("22::19::47");
	_sssion_time->add("22::19::48");
	_sssion_time->add("22::19::49");
	_sssion_time->add("22::19::50");
	_sssion_time->add("22::19::51");
	_sssion_time->add("22::19::52");
	_sssion_time->add("22::19::53");
	_sssion_time->add("22::19::54");
	_sssion_time->add("22::19::55");
	_sssion_time->add("22::19::56");
	//_sssion_time->setLabelUpperCase(false);
	pos_x += _sssion_time->getWidth();
	pos_x += 10;

	_sssion_attendant_uuid = new ofxDatGuiScrollView("attendant uuid", 10);
	_sssion_attendant_uuid->setTheme(_theme_close);
	_sssion_attendant_uuid->setWidth(300);
	_sssion_attendant_uuid->setLabelAlignment(ofxDatGuiAlignment::CENTER);
	_sssion_attendant_uuid->setOpacity(.5f);
	_sssion_attendant_uuid->setPosition(pos_x, pos_y);
	_sssion_attendant_uuid->add("FE311234-13D31-3DFE5-F1334F-EF");
	_sssion_attendant_uuid->add("FE311234-13D31-3DFE5-F1334F-EF");
	_sssion_attendant_uuid->add("FE311234-13D31-3DFE5-F1334F-EF");
	_sssion_attendant_uuid->add("FE311234-13D31-3DFE5-F1334F-EF");
	_sssion_attendant_uuid->add("FE311234-13D31-3DFE5-F1334F-EF");
	_sssion_attendant_uuid->add("FE311234-13D31-3DFE5-F1334F-EF");
	_sssion_attendant_uuid->add("FE311234-13D31-3DFE5-F1334F-EF");
	_sssion_attendant_uuid->add("FE311234-13D31-3DFE5-F1334F-EF");
	_sssion_attendant_uuid->add("FE311234-13D31-3DFE5-F1334F-EF");
	_sssion_attendant_uuid->add("FE311234-13D31-3DFE5-F1334F-EF");
	_sssion_attendant_uuid->add("FE311234-13D31-3DFE5-F1334F-EF");
	_sssion_attendant_uuid->add("FE311234-13D31-3DFE5-F1334F-EF");
	_sssion_attendant_uuid->add("FE311234-13D31-3DFE5-F1334F-EF");
	_sssion_attendant_uuid->add("FE311234-13D31-3DFE5-F1334F-EF");
	_sssion_attendant_uuid->add("FE311234-13D31-3DFE5-F1334F-EF");
	_sssion_attendant_uuid->add("FE311234-13D31-3DFE5-F1334F-EF");
	_sssion_attendant_uuid->add("FE311234-13D31-3DFE5-F1334F-EF");
	_sssion_attendant_uuid->add("FE311234-13D31-3DFE5-F1334F-EF");
	_sssion_attendant_uuid->add("FE311234-13D31-3DFE5-F1334F-EF");
	_sssion_attendant_uuid->add("FE311234-13D31-3DFE5-F1334F-EF");
	_sssion_attendant_uuid->add("FE311234-13D31-3DFE5-F1334F-EF");
	_sssion_attendant_uuid->add("FE311234-13D31-3DFE5-F1334F-EF");
	_sssion_attendant_uuid->add("FE311234-13D31-3DFE5-F1334F-EF");
	_sssion_attendant_uuid->add("FE311234-13D31-3DFE5-F1334F-EF");
	_sssion_attendant_uuid->add("FE311234-13D31-3DFE5-F1334F-EF");
	_sssion_attendant_uuid->add("FE311234-13D31-3DFE5-F1334F-EF");
	//_sssion_attendant_uuid->setLabelUpperCase(false);
	pos_x += _sssion_attendant_uuid->getWidth();
	pos_x += 10;

	_sssion_device_id = new ofxDatGuiScrollView("Device ID", 10);
	_sssion_device_id->setTheme(_theme_close);
	_sssion_device_id->setWidth(300);
	_sssion_device_id->setLabelAlignment(ofxDatGuiAlignment::CENTER);
	_sssion_device_id->setOpacity(.5f);
	_sssion_device_id->setPosition(pos_x, pos_y);
	_sssion_device_id->add("aa:bb:cc:ee:ff");
	_sssion_device_id->add("aa:bb:cc:ee:ff");
	_sssion_device_id->add("aa:bb:cc:ee:ff");
	_sssion_device_id->add("aa:bb:cc:ee:ff");
	_sssion_device_id->add("aa:bb:cc:ee:ff");
	_sssion_device_id->add("aa:bb:cc:ee:ff");
	_sssion_device_id->add("aa:bb:cc:ee:ff");
	_sssion_device_id->add("aa:bb:cc:ee:ff");
	_sssion_device_id->add("aa:bb:cc:ee:ff");
	_sssion_device_id->add("aa:bb:cc:ee:ff");
	_sssion_device_id->add("aa:bb:cc:ee:ff");
	_sssion_device_id->add("aa:bb:cc:ee:ff");
	_sssion_device_id->add("aa:bb:cc:ee:ff");
	_sssion_device_id->add("aa:bb:cc:ee:ff");
	_sssion_device_id->add("aa:bb:cc:ee:ff");
	_sssion_device_id->add("aa:bb:cc:ee:ff");
	_sssion_device_id->add("aa:bb:cc:ee:ff");
	_sssion_device_id->add("aa:bb:cc:ee:ff");
	_sssion_device_id->add("aa:bb:cc:ee:ff");
	_sssion_device_id->add("aa:bb:cc:ee:ff");
	_sssion_device_id->add("aa:bb:cc:ee:ff");
	_sssion_device_id->add("aa:bb:cc:ee:ff");
	_sssion_device_id->add("aa:bb:cc:ee:ff");
	_sssion_device_id->add("aa:bb:cc:ee:ff");
	_sssion_device_id->add("aa:bb:cc:ee:ff");
	_sssion_device_id->add("aa:bb:cc:ee:ff");
	//_sssion_device_id->setLabelUpperCase(false);
	pos_x += _sssion_device_id->getWidth();
	pos_x += 10;

	//;
	_sssion_cpu_usage = new ofxDatGuiScrollView("CPU Usage", 10);
	_sssion_cpu_usage->setTheme(_theme_close);
	_sssion_cpu_usage->setWidth(ofGetWidth() - pos_x - 25);
	_sssion_cpu_usage->setLabelAlignment(ofxDatGuiAlignment::CENTER);
	_sssion_cpu_usage->setOpacity(.5f);
	_sssion_cpu_usage->setPosition(pos_x, pos_y);
	_sssion_cpu_usage->add("0.21");
	_sssion_cpu_usage->add("0.21");
	_sssion_cpu_usage->add("0.21");
	_sssion_cpu_usage->add("0.21");
	_sssion_cpu_usage->add("0.21");
	_sssion_cpu_usage->add("0.21");
	_sssion_cpu_usage->add("0.21");
	_sssion_cpu_usage->add("0.21");
	_sssion_cpu_usage->add("0.21");
	_sssion_cpu_usage->add("0.21");
	_sssion_cpu_usage->add("0.21");
	_sssion_cpu_usage->add("0.21");
	_sssion_cpu_usage->add("0.21");
	_sssion_cpu_usage->add("0.21");
	_sssion_cpu_usage->add("0.21");
	_sssion_cpu_usage->add("0.21");
	_sssion_cpu_usage->add("0.21");
	_sssion_cpu_usage->add("0.21");
	_sssion_cpu_usage->add("0.21");
	_sssion_cpu_usage->add("0.21");
	_sssion_cpu_usage->add("0.21");
	_sssion_cpu_usage->add("0.21");
	_sssion_cpu_usage->add("0.21");
	_sssion_cpu_usage->add("0.21");
	_sssion_cpu_usage->add("0.21");
	_sssion_cpu_usage->add("0.21");

	ofxDatGuiScrollView * mouse_scroll_listeners[8];
	mouse_scroll_listeners[0] = _sssion_no;
	mouse_scroll_listeners[1] = _sssion_client;
	mouse_scroll_listeners[2] = _sssion_slot;
	mouse_scroll_listeners[3] = _sssion_state;
	mouse_scroll_listeners[4] = _sssion_time;
	mouse_scroll_listeners[5] = _sssion_attendant_uuid;
	mouse_scroll_listeners[6] = _sssion_device_id;
	mouse_scroll_listeners[7] = _sssion_cpu_usage;

	_sssion_no->addMouseScrollListener(mouse_scroll_listeners, 8);
	_sssion_client->addMouseScrollListener(mouse_scroll_listeners, 8);
	_sssion_slot->addMouseScrollListener(mouse_scroll_listeners, 8);
	_sssion_state->addMouseScrollListener(mouse_scroll_listeners, 8);
	_sssion_time->addMouseScrollListener(mouse_scroll_listeners, 8);
	_sssion_attendant_uuid->addMouseScrollListener(mouse_scroll_listeners, 8);
	_sssion_device_id->addMouseScrollListener(mouse_scroll_listeners, 8);
	_sssion_cpu_usage->addMouseScrollListener(mouse_scroll_listeners, 8);

	pos_x = 25;
	_session_no_label = new ofxDatGuiLabel("No");
	_session_no_label->setTheme(_theme);
	_session_no_label->setWidth(50);
	_session_no_label->setLabelAlignment(ofxDatGuiAlignment::CENTER);
	_session_no_label->setOpacity(.5f);
	_session_no_label->setPosition(pos_x, ofGetHeight() - (_session_no_label->getHeight() + _sssion_no->getHeight() + 27));
	_session_no_label->setLabelUpperCase(false);
	pos_x += _session_no_label->getWidth();
	pos_x += 10;
	pos_y = ofGetHeight() - (_session_no_label->getHeight() + _sssion_no->getHeight() + 27);

	_sssion_client_label = new ofxDatGuiLabel("Client");
	_sssion_client_label->setTheme(_theme_close);
	_sssion_client_label->setWidth(100);
	_sssion_client_label->setLabelAlignment(ofxDatGuiAlignment::CENTER);
	_sssion_client_label->setOpacity(.5f);
	_sssion_client_label->setPosition(pos_x, pos_y);
	_sssion_client_label->setLabelUpperCase(false);
	pos_x += _sssion_client_label->getWidth();
	pos_x += 10;

	_sssion_attendant_label = new ofxDatGuiLabel("attendant");
	_sssion_attendant_label->setTheme(_theme_close);
	_sssion_attendant_label->setWidth(100);
	_sssion_attendant_label->setLabelAlignment(ofxDatGuiAlignment::CENTER);
	_sssion_attendant_label->setOpacity(.5f);
	_sssion_attendant_label->setPosition(pos_x, pos_y);
	_sssion_attendant_label->setLabelUpperCase(false);
	pos_x += _sssion_client_label->getWidth();
	pos_x += 10;

	_sssion_state_label = new ofxDatGuiLabel("State");
	_sssion_state_label->setTheme(_theme_close);
	_sssion_state_label->setWidth(150);
	_sssion_state_label->setLabelAlignment(ofxDatGuiAlignment::CENTER);
	_sssion_state_label->setOpacity(.5f);
	_sssion_state_label->setPosition(pos_x, pos_y);
	_sssion_state_label->setLabelUpperCase(false);
	pos_x += _sssion_state_label->getWidth();
	pos_x += 10;

	_sssion_time_label = new ofxDatGuiLabel("Time");
	_sssion_time_label->setTheme(_theme_close);
	_sssion_time_label->setWidth(100);
	_sssion_time_label->setLabelAlignment(ofxDatGuiAlignment::CENTER);
	_sssion_time_label->setOpacity(.5f);
	_sssion_time_label->setPosition(pos_x, pos_y);
	_sssion_time_label->setLabelUpperCase(false);
	pos_x += _sssion_time_label->getWidth();
	pos_x += 10;

	_sssion_attendant_uuid_label = new ofxDatGuiLabel("attendant uuid");
	_sssion_attendant_uuid_label->setTheme(_theme_close);
	_sssion_attendant_uuid_label->setWidth(300);
	_sssion_attendant_uuid_label->setLabelAlignment(ofxDatGuiAlignment::CENTER);
	_sssion_attendant_uuid_label->setOpacity(.5f);
	_sssion_attendant_uuid_label->setPosition(pos_x, pos_y);
	_sssion_attendant_uuid_label->setLabelUpperCase(false);
	pos_x += _sssion_attendant_uuid_label->getWidth();
	pos_x += 10;

	_sssion_device_id_label = new ofxDatGuiLabel("Device ID");
	_sssion_device_id_label->setTheme(_theme_close);
	_sssion_device_id_label->setWidth(300);
	_sssion_device_id_label->setLabelAlignment(ofxDatGuiAlignment::CENTER);
	_sssion_device_id_label->setOpacity(.5f);
	_sssion_device_id_label->setPosition(pos_x, pos_y);
	_sssion_device_id_label->setLabelUpperCase(false);
	pos_x += _sssion_device_id_label->getWidth();
	pos_x += 10;

	_sssion_cpu_usage_label = new ofxDatGuiLabel("CPU Usage");
	_sssion_cpu_usage_label->setTheme(_theme_close);
	_sssion_cpu_usage_label->setWidth(ofGetWidth() - pos_x - 25);
	_sssion_cpu_usage_label->setLabelAlignment(ofxDatGuiAlignment::CENTER);
	_sssion_cpu_usage_label->setOpacity(.5f);
	_sssion_cpu_usage_label->setPosition(pos_x, pos_y);
	_sssion_cpu_usage_label->setLabelUpperCase(false);

	/*
	ofxDatGuiScrollView *
	ofxDatGuiScrollView *
	ofxDatGuiScrollView *
	ofxDatGuiScrollView *
	ofxDatGuiScrollView *
	ofxDatGuiScrollView *	;
	ofxDatGuiScrollView *	;
	ofxDatGuiScrollView *	;
	*/

	/*
	gui = new ofxDatGui(ofxDatGuiAnchor::TOP_RIGHT);

	// add some components //
	gui->addTextInput("message", "# open frameworks #");

	gui->addFRM();
	gui->addBreak();

	// add a folder to group a few components together //
	ofxDatGuiFolder* folder = gui->addFolder("white folder", ofColor::white);
	folder->addTextInput("** input", "nested input field");
	folder->addSlider("** slider", 0, 100);
	folder->addToggle("** toggle");
	folder->addColorPicker("** picker", ofColor::fromHex(0xFFD00B));
	// let's have it open by default. note: call this only after you're done adding items //
	folder->expand();

	gui->addBreak();

	// add a couple range sliders //
	gui->addSlider("position X", 0, 120, 75);
	gui->addSlider("position Y", -40, 240, 200);
	gui->addSlider("position Z", -80, 120, -40);

	// and a slider to adjust the gui opacity //
	gui->addSlider("datgui opacity", 0, 100, 100);

	// and a colorpicker //
	gui->addColorPicker("color picker", ofColor::fromHex(0xeeeeee));

	// add a wave monitor //
	// take a look inside example-TimeGraph for more examples of this component and the value plotter //
	gui->addWaveMonitor("wave\nmonitor", 3, .2);

	gui->addBreak();

	// add a dropdown menu //
	vector<string> opts = { "option - 1", "option - 2", "option - 3", "option - 4" };
	gui->addDropdown("select option", opts);
	gui->addBreak();

	// add a 2d pad //
	ofxDatGui2dPad* pad = gui->add2dPad("2d pad");

	// a button matrix //
	gui->addMatrix("matrix", 21, true);

	// and a couple of simple buttons //
	gui->addButton("click");
	gui->addToggle("toggle fullscreen", true);

	// adding the optional header allows you to drag the gui around //
	gui->addHeader(":: drag me to reposition ::");

	// adding the optional footer allows you to collapse/expand the gui //
	gui->addFooter();

	// once the gui has been assembled, register callbacks to listen for component specific events //
	gui->onButtonEvent(this, &ofApp::onButtonEvent);
	gui->onToggleEvent(this, &ofApp::onToggleEvent);
	gui->onSliderEvent(this, &ofApp::onSliderEvent);
	gui->onTextInputEvent(this, &ofApp::onTextInputEvent);
	gui->on2dPadEvent(this, &ofApp::on2dPadEvent);
	gui->onDropdownEvent(this, &ofApp::onDropdownEvent);
	gui->onColorPickerEvent(this, &ofApp::onColorPickerEvent);
	gui->onMatrixEvent(this, &ofApp::onMatrixEvent);

	gui->setOpacity(gui->getSlider("datgui opacity")->getScale());
	//  gui->setLabelAlignment(ofxDatGuiAlignment::CENTER);


	// finally let's load up the stock themes, press spacebar to cycle through them //
	themes = { new ofxDatGuiTheme(true),
	new ofxDatGuiThemeSmoke(),
	new ofxDatGuiThemeWireframe(),
	new ofxDatGuiThemeMidnight(),
	new ofxDatGuiThemeAqua(),
	new ofxDatGuiThemeCharcoal(),
	new ofxDatGuiThemeEntrix(),
	new ofxDatGuiThemeAutumn(),
	new ofxDatGuiThemeCandy() };
	tIndex = 0;
	*/
	// launch the app //
	//mFullscreen = true;
	//refreshWindow();
}

void ofApp::exit(void)
{
	//ofRemoveListener(ofEvents().mouseDragged, this, &ofApp::onMouseDragged);
}

void ofApp::onButtonEvent(ofxDatGuiButtonEvent e)
{
	cout << "onButtonEvent: " << e.target->getLabel() << endl;
}

void ofApp::onToggleEvent(ofxDatGuiToggleEvent e)
{
	if (e.target->is("toggle fullscreen")) toggleFullscreen();
	cout << "onToggleEvent: " << e.target->getLabel() << " " << e.checked << endl;
}

void ofApp::onSliderEvent(ofxDatGuiSliderEvent e)
{
	cout << "onSliderEvent: " << e.target->getLabel() << " "; e.target->printValue();
	//if (e.target->is("datgui opacity")) gui->setOpacity(e.scale);
}

void ofApp::onTextInputEvent(ofxDatGuiTextInputEvent e)
{
	cout << "onTextInputEvent: " << e.target->getLabel() << " " << e.target->getText() << endl;
}

void ofApp::on2dPadEvent(ofxDatGui2dPadEvent e)
{
	cout << "on2dPadEvent: " << e.target->getLabel() << " " << e.x << ":" << e.y << endl;
}

void ofApp::onDropdownEvent(ofxDatGuiDropdownEvent e)
{
	cout << "onDropdownEvent: " << e.target->getLabel() << " Selected" << endl;
}

void ofApp::onColorPickerEvent(ofxDatGuiColorPickerEvent e)
{
	cout << "onColorPickerEvent: " << e.target->getLabel() << " " << e.target->getColor() << endl;
	ofSetBackgroundColor(e.color);
}

void ofApp::onMatrixEvent(ofxDatGuiMatrixEvent e)
{
	cout << "onMatrixEvent " << e.child << " : " << e.enabled << endl;
	cout << "onMatrixEvent " << e.target->getLabel() << " : " << e.target->getSelected().size() << endl;
}

void ofApp::draw(void)
{
	//ofBackgroundGradient({ 255, 122, 0 }, { 234, 0, 44 });
	ofBackground({ 35, 31, 32 });
	ofSetHexColor(0xffffff);
	//ofFill();
	//ofRect(1, 1, ofGetWidth()-1, 10 - 1 + _logo.getHeight() / 2);
	_logo.draw(0, 0, _logo.getWidth(), _logo.getHeight());
	//_key_visual.draw((ofGetWidth() / 2) - (_key_visual.getWidth() / 2), (ofGetHeight() / 2) - (_key_visual.getHeight() / 2));

	ofSetColor({ 0xEE, 0xEE, 0xEE });
	_font.drawString("Arbitrator", _logo.getWidth(), _logo.getHeight() / 2 + 10);

	_session_no_label->draw();
	_sssion_client_label->draw();
	_sssion_attendant_label->draw();
	_sssion_state_label->draw();
	_sssion_time_label->draw();
	_sssion_attendant_uuid_label->draw();
	_sssion_device_id_label->draw();
	_sssion_cpu_usage_label->draw();

	_sssion_no->draw();
	_sssion_client->draw();
	_sssion_slot->draw();
	_sssion_state->draw();
	_sssion_time->draw();
	_sssion_attendant_uuid->draw();
	_sssion_device_id->draw();
	_sssion_cpu_usage->draw();

	_close->draw();
	_uuid->draw();
	_break_uuid->draw();
	_port->draw();
	_break_port->draw();
	_display->draw();
	_break_display->draw();
	_config->draw();
	_break_config->draw();
	_start->draw();
	_break_start->draw();
	_stop->draw();

	_cpu->draw();
	_break_cpu->draw();
	_memory->draw();
	_break_memory->draw();
	_gpus->draw();

	_cpu_usage->draw();
	_break_cpu_usage->draw();
	_memory_usage->draw();

	ofPushMatrix();
	ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);

	//Draw points

	ofSetColor(160, 160, 160, fabs(130 - _bg_transparent));
	ofFill();
	for (int i = 0; i<count_of_cloud_point; i++)
	{
		ofCircle(_cloud_point[i], 2);
	}

	//Draw lines between near points
	float dist = 40;	//Threshold parameter of distance
	for (int j = 0; j<count_of_cloud_point; j++)
	{
		for (int k = j + 1; k<count_of_cloud_point; k++)
		{
			if (ofDist(_cloud_point[j].x, _cloud_point[j].y, _cloud_point[k].x, _cloud_point[k].y) < dist)
			{
				ofLine(_cloud_point[j], _cloud_point[k]);
			}
		}
	}
	ofPopMatrix();
}

void ofApp::update(void)
{
	//Update sound engine
	ofSoundUpdate();
	//Get current spectrum with N bands
	float * val = ofSoundGetSpectrum(count_of_spectrum);
	//We should not release memory of val,
	//because it is managed by sound engine

	//Update our smoothed spectrum,
	//by slowly decreasing its values and getting maximum with val
	//So we will have slowly falling peaks in spectrum
	for (int i = 0; i<count_of_spectrum; i++)
	{
		_spectrum[i] *= 0.97;	//Slow decreasing
		_spectrum[i] = max(_spectrum[i], val[i]);
	}

	//Update particles using spectrum values

	//Computing dt as a time between the last
	//and the current calling of update()
	float time = ofGetElapsedTimef();
	float dt = time - _time0;
	dt = ofClamp(dt, 0.0, 0.1);
	_time0 = time; //Store the current time

				   //Update Rad and Vel from spectrum
				   //Note, the parameters in ofMap's were tuned for best result
				   //just for current music track
	_radius = ofMap(_spectrum[radius_band_index_in_spectrum], 1, 3, 400, 800, true);
	_velocity = ofMap(_spectrum[velocity_band_index_in_spectrum], 0, 0.1, 0.05, 0.5);

	//Update particles positions
	for (int j = 0; j<count_of_cloud_point; j++)
	{
		_tx[j] += _velocity * dt;	//move offset
		_ty[j] += _velocity * dt;	//move offset
									//Calculate Perlin's noise in [-1, 1] and
									//multiply on Rad
		_cloud_point[j].x = ofSignedNoise(_tx[j]) * _radius;
		_cloud_point[j].y = ofSignedNoise(_ty[j]) * _radius;
	}

	//if (_bg_transparent > 0) 
	//{
	//	_bg_transparent = 255 - time * 3.5;
	//}
	//else 
	//{
	//	_bg_transparent = 0;
	//}

	_bg_transparent = 140;

	_close->update();
	_uuid->update();
	_break_uuid->update();
	_port->update();
	_break_port->update();
	_display->update();
	_break_display->update();
	_config->update();
	_break_config->update();
	_start->update();
	_break_start->update();
	_stop->update();

	_cpu->update();
	_break_cpu->update();
	_memory->update();
	_break_memory->update();
	_gpus->update();

	_cpu_usage->update();
	_break_cpu_usage->update();
	_memory_usage->update();

	_session_no_label->update();
	_sssion_client_label->update();
	_sssion_attendant_label->update();
	_sssion_state_label->update();
	_sssion_time_label->update();
	_sssion_attendant_uuid_label->update();
	_sssion_device_id_label->update();
	_sssion_cpu_usage_label->update();

	_sssion_no->update();
	_sssion_client->update();
	_sssion_slot->update();
	_sssion_state->update();
	_sssion_time->update();
	_sssion_attendant_uuid->update();
	_sssion_device_id->update();
	_sssion_cpu_usage->update();
}

void ofApp::keyPressed(int key)
{
	if (key == 'f')
	{
		toggleFullscreen();
	}
	else if (key == 32)
	{
		//tIndex = tIndex < themes.size() - 1 ? tIndex + 1 : 0;
		//gui->setTheme(themes[tIndex]);
		//_server->setTheme(themes[tIndex]);
		//_information->setTheme(themes[tIndex]);
		//_usage->setTheme(themes[tIndex]);
	}
}

void ofApp::toggleFullscreen()
{
	//mFullscreen = !mFullscreen;
	//gui->getToggle("toggle fullscreen")->setChecked(mFullscreen);
	//refreshWindow();
}

void ofApp::refreshWindow()
{
	ofSetFullscreen(mFullscreen);
	//if (!mFullscreen) 
	//{
	//	ofSetWindowShape(ofApp::window_width, ofApp::window_height);
	//	ofSetWindowPosition((ofGetScreenWidth() / 2) - (ofApp::window_width / 2), 0);
	//}
}
