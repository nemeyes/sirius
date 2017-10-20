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

void ofApp::on_close_event(ofxDatGuiButtonEvent e)
{
	HWND hwnd = ofGetWin32Window();
	::SendMessage(hwnd, WM_KEYDOWN, VK_ESCAPE, NULL);
	::SendMessage(hwnd, WM_KEYUP, VK_ESCAPE, NULL);
}

void ofApp::on_reconnect_change_event(ofxDatGuiToggleEvent e)
{
	make_command_line();
}

void ofApp::on_inject_hook_change_event(ofxDatGuiToggleEvent e)
{
	make_command_line();
}

void ofApp::on_play_after_connect_change_event(ofxDatGuiToggleEvent e)
{
	make_command_line();
}

void ofApp::on_url_change_event(ofxDatGuiTextInputEvent e)
{
	make_command_line();
}

void ofApp::on_uuid_change_event(ofxDatGuiTextInputEvent e)
{
	make_command_line();
}

void ofApp::on_client_uuid_change_event(ofxDatGuiTextInputEvent e)
{
	make_command_line();
}

void ofApp::on_ctrlsrv_addr_change_event(ofxDatGuiTextInputEvent e)
{
	make_command_line();
}

void ofApp::on_ctrlsrv_port_change_event(ofxDatGuiTextInputEvent e)
{
	make_command_line();
}

void ofApp::on_strmsrv_addr_change_event(ofxDatGuiTextInputEvent e)
{
	make_command_line();
}

void ofApp::on_strmsrv_port_change_event(ofxDatGuiTextInputEvent e)
{
	make_command_line();
}

void ofApp::on_video_width_change_event(ofxDatGuiTextInputEvent e)
{
	make_command_line();
}

void ofApp::on_video_height_change_event(ofxDatGuiTextInputEvent e)
{
	make_command_line();
}

void ofApp::on_video_fps_change_event(ofxDatGuiTextInputEvent e)
{
	make_command_line();
}

void ofApp::on_video_present_change_event(ofxDatGuiToggleEvent e)
{
	make_command_line();
}

void ofApp::on_video_repeat_change_event(ofxDatGuiToggleEvent e)
{
	make_command_line();
}

void ofApp::retreieve_gpu_info(gpudesc_t * adapters, size_t capacity, size_t & count)
{
	IDXGIFactory3 * dxgi_factory = NULL;
	IDXGIAdapter1 * dxgi_adapter = NULL;
	HRESULT result = CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**)&dxgi_factory);
	if (FAILED(result))
		return;

	size_t index = 0;
	for (size_t i = 0; (dxgi_factory->EnumAdapters1(i, &dxgi_adapter) != DXGI_ERROR_NOT_FOUND) && index<capacity; i++)
	{
		DXGI_ADAPTER_DESC1 desc;
		dxgi_adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;

		char * description = nullptr;
		sirius::stringhelper::convert_wide2multibyte(desc.Description, &description);
		if (description)
		{
			strncpy_s(adapters[index].description, description, sizeof(adapters[index].description));
			free(description);
			description = nullptr;
		}
		adapters[index].adaptorIndex = i;
		adapters[index].vendorId = desc.VendorId;
		adapters[index].subsysId = desc.SubSysId;
		adapters[index].deviceId = desc.DeviceId;
		adapters[index].revision = desc.Revision;

		IDXGIOutput * output = NULL;
		if (DXGI_ERROR_NOT_FOUND != dxgi_adapter->EnumOutputs(0, &output))
		{
			DXGI_OUTPUT_DESC output_desc;
			HRESULT hr = output->GetDesc(&output_desc);
			adapters[index].coordLeft = output_desc.DesktopCoordinates.left;
			adapters[index].coordTop = output_desc.DesktopCoordinates.top;
			adapters[index].coordRight = output_desc.DesktopCoordinates.right;
			adapters[index].coordBottom = output_desc.DesktopCoordinates.bottom;

			SIRIUS_SAFE_RELEASE(output);
		}
		index++;

		if (dxgi_adapter)
			dxgi_adapter->Release();
		dxgi_adapter = NULL;
	}
	count = index;

	if (dxgi_factory)
		dxgi_factory->Release();
	dxgi_factory = NULL;
}

void ofApp::on_gen_uuid_event(ofxDatGuiButtonEvent e)
{
	UUID gen_uuid;
	::ZeroMemory(&gen_uuid, sizeof(UUID));
	::UuidCreate(&gen_uuid);

	char * new_uuid = nullptr;
	::UuidToStringA(&gen_uuid, (RPC_CSTR*)&new_uuid);

	std::string ret_uuid = (char *)new_uuid;
	std::transform(ret_uuid.begin(), ret_uuid.end(), ret_uuid.begin(), toupper);
	memcpy(new_uuid, ret_uuid.c_str(), ret_uuid.length());
	if (new_uuid)
	{
		_uuid->setText(new_uuid);
		::RpcStringFreeA((RPC_CSTR*)&new_uuid);
	}

	make_command_line();
}

void ofApp::on_gen_client_uuid_event(ofxDatGuiButtonEvent e)
{
	UUID gen_uuid;
	::ZeroMemory(&gen_uuid, sizeof(UUID));
	::UuidCreate(&gen_uuid);

	char * new_uuid = nullptr;
	::UuidToStringA(&gen_uuid, (RPC_CSTR*)&new_uuid);

	std::string ret_uuid = (char *)new_uuid;
	std::transform(ret_uuid.begin(), ret_uuid.end(), ret_uuid.begin(), toupper);
	memcpy(new_uuid, ret_uuid.c_str(), ret_uuid.length());
	if (new_uuid)
	{
		_client_uuid->setText(new_uuid);
		::RpcStringFreeA((RPC_CSTR*)&new_uuid);
	}

	make_command_line();
}

void ofApp::on_create_process_suspend_event(ofxDatGuiButtonEvent e)
{
	std::string app_path = _app_path->getText();
	if (app_path.length()<1)
	{
		//::MessageBox("Select Application Filename");
		return;
	}

	std::string command_line = _command_line_str;
	if (command_line.length() < 1)
	{
		//AfxMessageBox("Set Command Line Option");
		make_command_line();
		command_line = _command_line_str;
		if (command_line.length() < 1)
		{
			return;
		}
	}

	STARTUPINFOA startup_info;
	memset(&startup_info, 0, sizeof(STARTUPINFOA));
	memset(&_process_info, 0, sizeof(PROCESS_INFORMATION));
	startup_info.cb = sizeof(STARTUPINFOA);

	HINSTANCE module_handle = ::GetModuleHandleA("sirius_attendant_launcher.exe");
	char module_path[MAX_PATH] = { 0 };
	char * module_name = module_path;
	module_name += GetModuleFileNameA(module_handle, module_name, (sizeof(module_path) / sizeof(*module_path)) - (module_name - module_path));

	if (module_name != module_path)
	{
		CHAR * slash = strrchr(module_path, '\\');
		if (slash != NULL)
		{
			module_name = slash + 1;
			_strset_s(module_name, strlen(module_name) + 1, 0);
		}
		else
		{
			_strset_s(module_path, strlen(module_path) + 1, 0);
		}
	}

	// Spawn a process in SUSPENDED mode
	CHAR command[1500] = { 0 };
	_snprintf_s(command, sizeof(command) - 1, "%s %s", app_path.c_str(), command_line.c_str());
	if (!::CreateProcessA(NULL, (LPSTR)(LPCTSTR)command, NULL, NULL, false, CREATE_SUSPENDED, NULL, module_path, &startup_info, &_process_info))
	{
		//CString szError;
		//szError.Format(_T("Failed : %s"), command);
		//AfxMessageBox(szError);
		return;
	}
}

void ofApp::on_resume_event(ofxDatGuiButtonEvent e)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	// Get the id of the spawned process
	DWORD procId = _process_info.dwProcessId;
	bool do_inject = _inject_hook->getChecked();
	if (do_inject)
	{
		if (!inject(procId, "sirius_attendant_proxy.dll"))
		{
			//CString szError;
			//szError.Format(_T("Failed DLL Injection : %s"), INJECTION_DLL);
			//AfxMessageBox(szError);
		}
	}

	if (_process_info.hThread != nullptr)
	{
		// Resume the process
		ResumeThread(_process_info.hThread);

		_attendant_process_handle = _process_info.hProcess;
		_attendant_process_id = _process_info.dwProcessId;
	}

	CloseHandle(_attendant_process_handle);
	CloseHandle(_process_info.hThread);
}

void ofApp::on_unload_hook_dll_event(ofxDatGuiButtonEvent e)
{

}

void ofApp::setup(void)
{
	_radius = 500.f;
	_velocity = 0.1f;
	_time0 = 0;
	_bg_transparent = 255;

	_logo.load("img/company.png");
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
		_spectrum[i] = 0.0f;

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
	_close->onButtonEvent(this, &ofApp::on_close_event);
	_close->setLabelUpperCase(false);
	_close->setPosition(ofGetWidth() - _close->getWidth(), 0);

	_app_path = new ofxDatGuiTextInput("Application Path", "sirius_web_attendant.exe");
	_app_path->setTheme(_theme);
	_app_path->setWidth(1230, 150);
	_app_path->setPosition(pos_x, pos_y);
	_app_path->setLabelUpperCase(false);
	pos_y += _app_path->getHeight();

	_break_app_path = new ofxDatGuiBreak();
	_break_app_path->setTheme(_theme);
	_break_app_path->setWidth(1230, 150);
	_break_app_path->setHeight(1);
	_break_app_path->setPosition(pos_x, pos_y);
	_break_app_path->setBackgroundColor({ 0x55, 0x55, 0x55 });
	pos_y += _break_app_path->getHeight();

	_command_line = new ofxDatGuiTextInput("Command Line", "");
	_command_line->setTheme(_theme);
	_command_line->setWidth(1230, 150);
	_command_line->setPosition(pos_x, pos_y);
	_command_line->setLabelUpperCase(false);
	_command_line->setEnabled(false);
	pos_y += _command_line->getHeight();

	pos_y += 30;
	_controller_streamer_option = new ofxDatGui(pos_x, pos_y);
	_controller_streamer_option->setTheme(_theme);
	_controller_streamer_option->setWidth(500, 200);
	_controller_streamer_option->addHeader("Controller & Streamer");

	_reconnect = _controller_streamer_option->addToggle("Reconnect", true);
	_reconnect->onToggleEvent(this, &ofApp::on_reconnect_change_event);

	_inject_hook = _controller_streamer_option->addToggle("Inject & Hook", false);
	_inject_hook->onToggleEvent(this, &ofApp::on_inject_hook_change_event);

	_play_after_connect = _controller_streamer_option->addToggle("Play After Connect", false);
	_play_after_connect->onToggleEvent(this, &ofApp::on_play_after_connect_change_event);
	
	_url = _controller_streamer_option->addTextInput("URL", "https://www.naver.com");
	_url->onTextInputEvent(this, &ofApp::on_url_change_event);
	
	_uuid = _controller_streamer_option->addTextInput("UUID", "9701AE27-7655-41CA-ADEA-F8AEA3BD645C");
	_uuid->onTextInputEvent(this, &ofApp::on_uuid_change_event);

	_client_uuid = _controller_streamer_option->addTextInput("Client UUID", "3701AE27-7655-41CA-ADEA-F8AEA3BD645C");
	_client_uuid->onTextInputEvent(this, &ofApp::on_client_uuid_change_event);

	_ctrlsrv_addr = _controller_streamer_option->addTextInput("Control Server Address", "127.0.0.1");
	_ctrlsrv_addr->onTextInputEvent(this, &ofApp::on_ctrlsrv_addr_change_event);

	_ctrlsrv_port = _controller_streamer_option->addTextInput("Control Server PortNumber", "5000");
	_ctrlsrv_port->onTextInputEvent(this, &ofApp::on_ctrlsrv_port_change_event);

	_strmsrv_addr = _controller_streamer_option->addTextInput("Streamer Address", "127.0.0.1");
	_strmsrv_addr->onTextInputEvent(this, &ofApp::on_strmsrv_addr_change_event);

	_strmsrv_port = _controller_streamer_option->addTextInput("Streamer PortNumber", "7000");
	_strmsrv_port->onTextInputEvent(this, &ofApp::on_strmsrv_port_change_event);

	_controller_streamer_option->setOpacity(.5f);
	pos_y += _controller_streamer_option->getHeight();

	_video_gpu_option = new ofxDatGui(pos_x + 270 + _controller_streamer_option->getWidth(), _controller_streamer_option->getPosition().y);
	_video_gpu_option->setTheme(_theme);
	_video_gpu_option->setWidth(460, 200);
	_video_gpu_option->addHeader("Video & GPU");
	_video_width = _video_gpu_option->addTextInput("Width", "1280");
	_video_width->onTextInputEvent(this, &ofApp::on_video_width_change_event);

	_video_height = _video_gpu_option->addTextInput("Height", "720");
	_video_height->onTextInputEvent(this, &ofApp::on_video_height_change_event);
	
	_video_fps = _video_gpu_option->addTextInput("FPS", "15");
	_video_fps->onTextInputEvent(this, &ofApp::on_video_fps_change_event);

	_video_present = _video_gpu_option->addToggle("Present", true);
	_video_present->onToggleEvent(this, &ofApp::on_video_present_change_event);

	_video_repeat = _video_gpu_option->addToggle("Repeat", false);
	_video_repeat->onToggleEvent(this, &ofApp::on_video_repeat_change_event);

	std::vector<std::string> gpus_opts;
	gpudesc_t adapters[16];
	size_t nadapter = 0;
	retreieve_gpu_info(adapters, 16, nadapter);
	for (int32_t index = 0; index < nadapter; index++)
	{
		char description[260] = { 0 };
		_snprintf_s(description, sizeof(description), "%d : %s", index, adapters[index].description);
		gpus_opts.push_back(description);
	}

	_gpus = _video_gpu_option->addDropdown("GPUS", gpus_opts);
	_gpus->setLabelAlignment(ofxDatGuiAlignment::RIGHT);
	for (int32_t index = 0; index < gpus_opts.size(); index++)
	{
		ofxDatGuiDropdownOption * opt = _gpus->getChildAt(index);
		opt->setLabelAlignment(ofxDatGuiAlignment::RIGHT);
	}
	_video_gpu_option->setOpacity(.5f);

	_gen_uuid_btn = new ofxDatGuiButton("Generate UUID");
	_gen_uuid_btn->setTheme(_theme);
	_gen_uuid_btn->setWidth(150);
	_gen_uuid_btn->setPosition(530, _uuid->getY());
	_gen_uuid_btn->setOpacity(0.9f);
	_gen_uuid_btn->setLabelAlignment(ofxDatGuiAlignment::CENTER);
	_gen_uuid_btn->onButtonEvent(this, &ofApp::on_gen_uuid_event);

	_gen_client_uuid_btn = new ofxDatGuiButton("Generate Client UUID");
	_gen_client_uuid_btn->setTheme(_theme);
	_gen_client_uuid_btn->setWidth(150);
	_gen_client_uuid_btn->setPosition(530, _client_uuid->getY());
	_gen_client_uuid_btn->setOpacity(0.9f);
	_gen_client_uuid_btn->setLabelAlignment(ofxDatGuiAlignment::CENTER);
	_gen_client_uuid_btn->onButtonEvent(this, &ofApp::on_gen_client_uuid_event);

	pos_x = 650;
	pos_y = 670;
	_create_process_btn = new ofxDatGuiButton("Create Process & Suspend");
	_create_process_btn->setTheme(_theme);
	_create_process_btn->setWidth(200);
	_create_process_btn->setPosition(pos_x, pos_y);
	_create_process_btn->setLabelAlignment(ofxDatGuiAlignment::CENTER);
	_create_process_btn->onButtonEvent(this, &ofApp::on_create_process_suspend_event);
	pos_x += _create_process_btn->getWidth();


	_resume_btn = new ofxDatGuiButton("Resume");
	_resume_btn->setTheme(_theme);
	_resume_btn->setWidth(200);
	_resume_btn->setPosition(pos_x, pos_y);
	_resume_btn->setLabelUpperCase(false);
	_resume_btn->setLabelAlignment(ofxDatGuiAlignment::CENTER);
	_resume_btn->onButtonEvent(this, &ofApp::on_resume_event);
	pos_x += _resume_btn->getWidth();

	_unload_hook_dll_btn = new ofxDatGuiButton("Unload Hook DLL");
	_unload_hook_dll_btn->setTheme(_theme);
	_unload_hook_dll_btn->setWidth(200);
	_unload_hook_dll_btn->setPosition(pos_x, pos_y);
	_unload_hook_dll_btn->setLabelUpperCase(false);
	_unload_hook_dll_btn->setLabelAlignment(ofxDatGuiAlignment::CENTER);
	_unload_hook_dll_btn->onButtonEvent(this, &ofApp::on_unload_hook_dll_event);
}

void ofApp::exit(void)
{
	//ofRemoveListener(ofEvents().mouseDragged, this, &ofApp::onMouseDragged);
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
	_font.drawString("Launcher", _logo.getWidth(), _logo.getHeight() / 2 + 10);

	_close->draw();

	_app_path->draw();
	_break_app_path->draw();
	_command_line->draw();

	_controller_streamer_option->draw();
	_video_gpu_option->draw();

	_gen_uuid_btn->draw();
	_gen_client_uuid_btn->draw();

	_create_process_btn->draw();
	_resume_btn->draw();
	_unload_hook_dll_btn->draw();


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
	_bg_transparent = 140;

	_close->update();
	
	_app_path->update();
	_break_app_path->update();
	_command_line->update();

	_controller_streamer_option->update();
	_video_gpu_option->update();

	_gen_uuid_btn->update();
	_gen_client_uuid_btn->update();
	_create_process_btn->update();
	_resume_btn->update();
	_unload_hook_dll_btn->update();
}

void ofApp::make_command_line(void)
{
	std::string type = "web";
	std::string reconnect = _reconnect->getChecked() ? "true" : "false";
	std::string inject_hook = _inject_hook->getChecked() ? "true" : "false";
	std::string play_after_connect = _play_after_connect->getChecked() ? "true" : "false";
	std::string url = _url->getText();
	std::string uuid = _uuid->getText();
	std::string client_uuid = _client_uuid->getText();
	std::string ctrlsrv_addr = _ctrlsrv_addr->getText();
	std::string ctrlsrv_port = _ctrlsrv_port->getText();
	std::string strmsrv_addr = _strmsrv_addr->getText();
	std::string strmsrv_port = _strmsrv_port->getText();
	std::string video_width = _video_width->getText();
	std::string video_height = _video_height->getText();
	std::string video_fps = _video_fps->getText();
	std::string present = _video_present->getChecked() ? "true" : "false";
	std::string repeat = _video_repeat->getChecked() ? "true" : "false";

	char command_line[1500] = { 0 };
	_snprintf(command_line, sizeof(command_line), "--reconnect=%s --attendant_type=\"%s\" --url=\"%s\" --video_codec=\"png\" --video_width=%s --video_height=%s --video_fps=%s --gpu_index=%d --enable_present=%s --enable_repeat=%s --uuid=\"%s\" --control_server_address=\"%s\" --control_server_portnumber=%s --streamer_address=\"%s\" --streamer_portnumber=%s --play_after_connect=%s --client_uuid=\"%s\"  --ignore-gpu-blacklist --force-compositing-mode --enable-gpu --multi-gpu",
		reconnect.c_str(),
		type.c_str(),
		url.c_str(),
		video_width.c_str(),
		video_height.c_str(),
		video_fps.c_str(),
		0,
		present.c_str(),
		repeat.c_str(),
		uuid.c_str(),
		ctrlsrv_addr.c_str(),
		ctrlsrv_port.c_str(),
		strmsrv_addr.c_str(),
		strmsrv_port.c_str(),
		play_after_connect.c_str(),
		client_uuid.c_str());

	_command_line->setText(command_line);
	_command_line_str = command_line;
}

HMODULE ofApp::inject(DWORD ProcessID, char * dllName)
{
	HANDLE Proc;
	HANDLE Thread;
	char buf[50] = { 0 };
	LPVOID RemoteString, LoadLibAddy;
	HMODULE hModule = NULL;
	DWORD dwOut;

	if (!ProcessID)
		return false;

#if 1
	// XP 에서는 PROCESS_ALL_ACCESS 범위 값이 커서 인식 하지 못함.
	Proc = OpenProcess(PROCESS_ALL_ACCESS, 0, ProcessID);
#else
	Proc = OpenProcess(MAXIMUM_ALLOWED, 0, ProcessID);
#endif

	if (!Proc)
	{
		sprintf_s(buf, "OpenProcess() failed: %d", GetLastError());
		//AfxMessageBox(buf);
		return false;
	}

	LoadLibAddy = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
	if (!LoadLibAddy) 
	{
		return false;
	}

	RemoteString = (LPVOID)VirtualAllocEx(Proc, NULL, strlen(dllName), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!RemoteString) 
	{
		return false;
	}

	if (!WriteProcessMemory(Proc, (LPVOID)RemoteString, dllName, strlen(dllName), NULL)) 
	{
		return false;
	}

	Thread = CreateRemoteThread(Proc, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibAddy, (LPVOID)RemoteString, NULL, NULL);
	if (!Thread) 
	{
		return false;
	}
	else 
	{
		while (GetExitCodeThread(Thread, &dwOut)) 
		{
			if (dwOut != STILL_ACTIVE) 
			{
				hModule = (HMODULE)dwOut;
				break;
			}
		}
	}

	CloseHandle(Thread);
	CloseHandle(Proc);

	return hModule;
}