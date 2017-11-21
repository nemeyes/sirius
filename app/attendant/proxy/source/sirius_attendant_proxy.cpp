#include "sirius_attendant_proxy.h"

#include <sirius_stringhelper.h>
#include <sirius_log4cplus_logger.h>
#include <string>
#include <map>

#include "attendant_proxy.h"

bool sirius::app::attendant::proxy::parse_argument(int32_t argc, wchar_t * argv[])
{
	sirius::app::attendant::proxy::context_t * context = sirius::app::attendant::proxy::instance().context();

	wchar_t * pargv;
	std::map<std::wstring, std::wstring> param;
	for (int32_t i = 1; i < argc; i++)
	{
		pargv = argv[i];
		if (wcsncmp(pargv, L"--", 2) == 0)
		{
			const wchar_t *p = wcschr(pargv + 2, L'=');
			if (p)
			{
				const wchar_t *f = pargv + 2;
				std::wstring name(f, p);
				std::wstring val(p + 1);
				param.insert(std::make_pair(name, val));
			}
			else
			{
				std::wstring name(pargv + 2);
				std::wstring val;
				val.clear();
				param.insert(std::make_pair(name, val));
			}
		}
		else
		{
			continue;
		}
	}

	std::map<std::wstring, std::wstring>::iterator iter;
	std::wstring value;
	if (param.end() != (iter = param.find(L"client_uuid")))
	{
		value = iter->second;
		wcscpy_s(context->client_uuid, value.c_str());
	}
	if (param.end() != (iter = param.find(L"uuid")))
	{
		value = iter->second;
		wcscpy_s(context->uuid, value.c_str());
	}
	if (param.end() != (iter = param.find(L"client_id")))
	{
		value = iter->second;
		wcscpy_s(context->client_id, value.c_str());
	}
	if (param.end() != (iter = param.find(L"control_server_portnumber")))
	{
		value = iter->second;
		context->controller_portnumber = _wtoi(value.c_str());
	}
	if (param.end() != (iter = param.find(L"streamer_portnumber")))
	{
		value = iter->second;
		context->streamer_portnumber = _wtoi(value.c_str());
	}
	if (param.end() != (iter = param.find(L"reconnect")))
	{
		value = iter->second;
		context->reconnect = _wtoi(value.c_str());
	}
	if (param.end() != (iter = param.find(L"attendant_type")))
	{
		value = iter->second;
		if (!_wcsicmp(value.c_str(), L"web"))
		{
			context->type = sirius::app::attendant::proxy::attendant_type_t::web;
		}
	}
	if (param.end() != (iter = param.find(L"url")))
	{
		std::map<std::wstring, std::wstring>::iterator iter2;
		value = iter->second;
		wcscpy_s(context->url, MAX_PATH - 1, value.c_str());
	}
	if (param.end() != (iter = param.find(L"video_codec")))
	{
		value = iter->second;
		if (!_wcsicmp(value.c_str(), L"png"))
			context->video_codec = sirius::app::attendant::proxy::video_submedia_type_t::png;
		else if (!_wcsicmp(value.c_str(), L"jpeg"))
			context->video_codec = sirius::app::attendant::proxy::video_submedia_type_t::jpeg;
		else if (!_wcsicmp(value.c_str(), L"dxt"))
			context->video_codec = sirius::app::attendant::proxy::video_submedia_type_t::dxt;
		else
			context->video_codec = sirius::app::attendant::proxy::video_submedia_type_t::unknown;
	}
	if (param.end() != (iter = param.find(L"video_width")))
	{
		value = iter->second;
		context->video_width = _wtoi(value.c_str());
	}
	if (param.end() != (iter = param.find(L"video_height")))
	{
		value = iter->second;
		context->video_height = _wtoi(value.c_str());
	}
	if (param.end() != (iter = param.find(L"video_fps")))
	{
		value = iter->second;
		context->video_fps = _wtoi(value.c_str());
	}
	if (param.end() != (iter = param.find(L"video_buffer_count")))
	{
		value = iter->second;
		context->video_nbuffer = _wtoi(value.c_str());
	}
	if (param.end() != (iter = param.find(L"video_block_width")))
	{
		value = iter->second;
		context->video_block_width = _wtoi(value.c_str());
	}
	if (param.end() != (iter = param.find(L"video_block_height")))
	{
		value = iter->second;
		context->video_block_height = _wtoi(value.c_str());
	}
	if (param.end() != (iter = param.find(L"gpu_index")))
	{
		value = iter->second;
		context->gpuindex = _wtoi(value.c_str());
	}
	if (param.end() != (iter = param.find(L"enable_present")))
	{
		value = iter->second;
		if (!_wcsicmp(value.c_str(), L"true"))
			context->present = true;
		else
			context->present = false;
	}
	if (param.end() != (iter = param.find(L"play_after_connect")))
	{
		value = iter->second;
		if (!_wcsicmp(value.c_str(), L"true"))
			context->play_after_connect = true;
		else
			context->play_after_connect = false;
	}
	if (param.end() != (iter = param.find(L"off-screen-rendering-enabled")))
	{
		context->video_process_type = sirius::app::attendant::proxy::video_memory_type_t::host;
		if (param.end() != (iter = param.find(L"off-screen-frame-rate")))
		{
			value = iter->second;
			context->video_fps = _wtoi(value.c_str());
		}
	}
	else
	{
		context->video_process_type = sirius::app::attendant::proxy::video_memory_type_t::d3d11;
	}
	return true;
}

sirius::app::attendant::proxy & sirius::app::attendant::proxy::instance(void)
{
	static sirius::app::attendant::proxy _instance;
	return _instance;
}

sirius::app::attendant::proxy::context_t * sirius::app::attendant::proxy::context(void)
{
	return &_context;
}

bool sirius::app::attendant::proxy::is_initialized(void)
{
	return _initialized;
}

sirius::app::attendant::proxy::proxy(void)
	: _core(nullptr)
	, _key_pressed(FALSE)
	, _initialized(FALSE)
{

}

sirius::app::attendant::proxy::~proxy(void)
{
	//release();
}

int32_t sirius::app::attendant::proxy::initialize(void)
{
	sirius::app::attendant::proxy::context_t * context = sirius::app::attendant::proxy::instance().context();
	int str_size = WideCharToMultiByte(CP_ACP, 0, context->client_id, -1, NULL, 0, NULL, NULL);
	char* str_ptr = new char[str_size];
	WideCharToMultiByte(CP_ACP, 0, context->client_id, -1, str_ptr, str_size, 0, 0);
	sirius::library::log::log4cplus::logger::create("configuration\\log.ini", SLNS, str_ptr);
	if (str_ptr)
	{
		delete [] str_ptr;
		str_ptr = NULL;
	}
	char * mb_uuid = nullptr;
	char * mb_client_uuid = nullptr;
	sirius::stringhelper::convert_wide2multibyte((wchar_t*)context->uuid, &mb_uuid);
	sirius::stringhelper::convert_wide2multibyte((wchar_t*)context->client_uuid, &mb_client_uuid);

	if (mb_uuid && strlen(mb_uuid) > 0)
	{
		if (mb_client_uuid && strlen(mb_client_uuid) > 0)
		{
			_core = new sirius::app::attendant::proxy::core(this, mb_uuid, mb_client_uuid);
		}
		else
		{
			_core = new sirius::app::attendant::proxy::core(this, mb_uuid, nullptr);
		}
	}

	if (mb_uuid)
		free(mb_uuid);
	if (mb_client_uuid)
		free(mb_client_uuid);

	if (_core)
		return _core->initialize(context);
	return sirius::app::attendant::proxy::err_code_t::fail;
}


int32_t sirius::app::attendant::proxy::release(void)
{
	int32_t status = sirius::app::attendant::proxy::err_code_t::fail;
	if (_core)
	{
		status = _core->release();
		delete _core;
		_core = nullptr;
	}
	sirius::library::log::log4cplus::logger::destroy();
	return status;
}

int32_t sirius::app::attendant::proxy::connect(void)
{
	if (_core)
		return _core->connect();
	return sirius::app::attendant::proxy::err_code_t::fail;
}

int32_t sirius::app::attendant::proxy::disconnect(void)
{
	if (_core)
		return _core->disconnect();
	return sirius::app::attendant::proxy::err_code_t::fail;
}

int32_t sirius::app::attendant::proxy::play(void)
{
	if (_core)
		return _core->play();
	return sirius::app::attendant::proxy::err_code_t::fail;
}

int32_t sirius::app::attendant::proxy::stop(void)
{
	if (_core)
		return _core->stop();
	return sirius::app::attendant::proxy::err_code_t::fail;
}

int32_t sirius::app::attendant::proxy::play_toggle(void)
{
	if (_core)
		return _core->play_toggle();
	return sirius::app::attendant::proxy::err_code_t::fail;
}

int32_t sirius::app::attendant::proxy::backward(void)
{
	if (_core)
		return _core->backward();
	return sirius::app::attendant::proxy::err_code_t::fail;
}

int32_t sirius::app::attendant::proxy::forward(void)
{
	if (_core)
		return _core->forward();
	return sirius::app::attendant::proxy::err_code_t::fail;
}

int32_t sirius::app::attendant::proxy::reverse(void)
{
	if (_core)
		return _core->reverse();
	return sirius::app::attendant::proxy::err_code_t::fail;
}

void sirius::app::attendant::proxy::on_destroy(void)
{
	if (_core)
		_core->on_destroy();
}

void sirius::app::attendant::proxy::on_key_up(int8_t type, int32_t key)
{
	if (_core)
	{
		if (context()->play_after_connect == false)
		{
			_key_pressed = FALSE;
		}
		_core->on_key_up(type, key);
	}
}

void sirius::app::attendant::proxy::on_key_down(int8_t type, int32_t key)
{
	if (_core)
	{
#if defined(_DEBUG)
		if (context()->play_after_connect == false)
		{
			if (_key_pressed == FALSE)
			{
				_key_pressed = TRUE;
				switch (key)
				{
				//case VK_NUMPAD4 :	if(_core) _core->Seek(-10);		break;
				//case VK_NUMPAD6 :	if(_core) _core->Seek(10);		break;
				case VK_NUMPAD3 :	if(_core) _core->forward();		break;
				case VK_NUMPAD1 :	if(_core) _core->backward();	break;
				case VK_NUMPAD2 :	if(_core) _core->reverse();		break;
				case VK_NUMPAD0 :	if(_core) _core->play_toggle();	break;
				}
			}
		}		
#endif
		_core->on_key_down(type, key);
		//LOGGER::make_debug_log("keycontrol", "%s(), [key] = %d", __FUNCTION__, key);
	}
}

void sirius::app::attendant::proxy::on_mouse_move(int32_t pos_x, int32_t pos_y)
{
	if (_core)
		_core->on_mouse_move(pos_x, pos_y);
}

void sirius::app::attendant::proxy::on_mouse_lbd(int32_t pos_x, int32_t pos_y)
{
	if (_core)
		_core->on_mouse_lbd(pos_x, pos_y);
}

void sirius::app::attendant::proxy::on_mouse_lbu(int32_t pos_x, int32_t pos_y)
{
	if (_core)
		_core->on_mouse_lbu(pos_x, pos_y);
}

void sirius::app::attendant::proxy::on_mouse_rbd(int32_t pos_x, int32_t pos_y)
{
	if (_core)
		_core->on_mouse_rbd(pos_x, pos_y);
}

void sirius::app::attendant::proxy::on_mouse_rbu(int32_t pos_x, int32_t pos_y)
{
	if (_core)
		_core->on_mouse_rbu(pos_x, pos_y);
}

void sirius::app::attendant::proxy::on_mouse_lb_dclick(int32_t pos_x, int32_t pos_y)
{
	if (_core)
		_core->on_mouse_lb_dclick(pos_x, pos_y);
}

void sirius::app::attendant::proxy::on_mouse_rb_dclick(int32_t pos_x, int32_t pos_y)
{
	if (_core)
		_core->on_mouse_rb_dclick(pos_x, pos_y);
}

void sirius::app::attendant::proxy::on_mouse_wheel(int32_t pos_x, int32_t pos_y, int32_t wheel_z)
{
	if (_core)
		_core->on_mouse_wheel(pos_x, pos_y, wheel_z);
}

void sirius::app::attendant::proxy::on_gyro(float x, float y, float z)
{
	if (_core)
		_core->on_gyro(x, y, z);
}

void sirius::app::attendant::proxy::on_pinch_zoom(float delta)
{
	if (_core)
		_core->on_pinch_zoom(delta);
}

void sirius::app::attendant::proxy::on_gyro_attitude(float x, float y, float z, float w)
{
	if (_core)
		_core->on_gyro_attitude(x, y, z, w);
}

void sirius::app::attendant::proxy::on_gyro_gravity(float x, float y, float z)
{
	if (_core)
		_core->on_gyro_gravity(x, y, z);
}

void sirius::app::attendant::proxy::on_gyro_rotation_rate(float x, float y, float z)
{
	if (_core)
		_core->on_gyro_rotation_rate(x, y, z);
}

void sirius::app::attendant::proxy::on_gyro_rotation_rate_unbiased(float x, float y, float z)
{
	if (_core)
		_core->on_gyro_rotation_rate_unbiased(x, y, z);
}

void sirius::app::attendant::proxy::on_gyro_user_acceleration(float x, float y, float z)
{
	if (_core)
		_core->on_gyro_user_acceleration(x, y, z);
}

void sirius::app::attendant::proxy::on_gyro_enabled_attitude(bool state)
{
	if (_core)
		_core->on_gyro_enabled_attitude(state);
}

void sirius::app::attendant::proxy::on_gyro_enabled_gravity(bool state)
{
	if (_core)
		_core->on_gyro_enabled_gravity(state);
}

void sirius::app::attendant::proxy::on_gyro_enabled_rotation_rate(bool state)
{
	if (_core)
		_core->on_gyro_enabled_rotation_rate(state);
}

void sirius::app::attendant::proxy::on_gyro_enabled_rotation_rate_unbiased(bool state)
{
	if (_core)
		_core->on_gyro_enabled_rotation_rate_unbiased(state);
}

void sirius::app::attendant::proxy::on_gyro_enabled_user_acceleration(bool state)
{
	if (_core)
		_core->on_gyro_enabled_user_acceleration(state);
}

void sirius::app::attendant::proxy::on_gyro_update_interval(float interval)
{
	if (_core)
		_core->on_gyro_update_interval(interval);
}

void sirius::app::attendant::proxy::on_ar_view_mat(float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23,
	float m30, float m31, float m32, float m33)
{
	if (_core)
		_core->on_ar_view_mat(m00, m01, m02, m03,
			m10, m11, m12, m13,
			m20, m21, m22, m23,
			m30, m31, m32, m33);
}

void sirius::app::attendant::proxy::on_ar_proj_mat(float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23,
	float m30, float m31, float m32, float m33)
{
	if (_core)
		_core->on_ar_proj_mat(m00, m01, m02, m03,
			m10, m11, m12, m13,
			m20, m21, m22, m23,
			m30, m31, m32, m33);
}

void sirius::app::attendant::proxy::send(char * packet, int len)
{
	if (_core)
		_core->on_app_to_container_xml(packet, len);
}

void sirius::app::attendant::proxy::on_container_to_app(char * packet, int len)
{
	if (_core)
		_core->on_container_to_app(packet, len);
}

void sirius::app::attendant::proxy::set_webcontainer_callback(FuncPtrCallback fncallback)
{
	if (_core)
		_core->set_callback(fncallback);
}