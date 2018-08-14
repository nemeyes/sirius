#include "attendant_proxy.h"
#include "sirius_attendant_proxy.h"

#include <sirius_stringhelper.h>
#include <sirius_log4cplus_logger.h>
#include <string>
#include <map>


bool sirius::app::attendant::proxy::parse_argument(int32_t argc, wchar_t * argv[], sirius::app::attendant::proxy::context_t * context)
{
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
	if (param.end() != (iter = param.find(L"uuid")))
	{
		value = iter->second;
		wcscpy_s(context->uuid, value.c_str());
	}
	if (param.end() != (iter = param.find(L"control_server_portnumber")))
	{
		value = iter->second;
		context->controller_portnumber = _wtoi(value.c_str());
	}
	if (param.end() != (iter = param.find(L"streaming_server_portnumber")))
	{
		value = iter->second;
		context->streamer_portnumber = _wtoi(value.c_str());
	}
	if (param.end() != (iter = param.find(L"id")))
	{
		value = iter->second;
		context->id = _wtoi(value.c_str());
	}
	if (param.end() != (iter = param.find(L"reconnect")))
	{
		value = iter->second;
		if (!_wcsicmp(value.c_str(), L"true"))
			context->reconnect = true;
		else
			context->reconnect = false;
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
	if (param.end() != (iter = param.find(L"video_compression_level")))
	{
		value = iter->second;
		context->video_compression_level = _wtoi(value.c_str());
	}
	if (param.end() != (iter = param.find(L"video_quantization_posterization")))
	{
		value = iter->second;
		if (!_wcsicmp(value.c_str(), L"true"))
			context->video_quantization_posterization = true;
		else
			context->video_quantization_posterization = false;
	}
	if (param.end() != (iter = param.find(L"video_quantization_dither_map")))
	{
		value = iter->second;
		if (!_wcsicmp(value.c_str(), L"true"))
			context->video_quantization_dither_map = true;
		else
			context->video_quantization_dither_map = false;
	}
	if (param.end() != (iter = param.find(L"video_quantization_contrast_maps")))
	{
		value = iter->second;
		if (!_wcsicmp(value.c_str(), L"true"))
			context->video_quantization_contrast_maps = true;
		else
			context->video_quantization_contrast_maps = false;
	}
	if (param.end() != (iter = param.find(L"video_quantization_colors")))
	{
		value = iter->second;
		context->video_quantization_colors = _wtoi(value.c_str());
	}
	if (param.end() != (iter = param.find(L"enable_invalidate4client")))
	{
		value = iter->second;
		if (!_wcsicmp(value.c_str(), L"true"))
			context->invalidate4client = true;
		else
			context->invalidate4client = false;
	}
	if (param.end() != (iter = param.find(L"enable_indexed_mode")))
	{
		value = iter->second;
		if (!_wcsicmp(value.c_str(), L"true"))
			context->indexed_mode = true;
		else
			context->indexed_mode = false;
	}
	if (param.end() != (iter = param.find(L"nthread")))
	{
		value = iter->second;
		context->nthread = _wtoi(value.c_str());
	}
	if (param.end() != (iter = param.find(L"enable_present")))
	{
		value = iter->second;
		if (!_wcsicmp(value.c_str(), L"true"))
			context->present = true;
		else
			context->present = false;
	}
	if (param.end() != (iter = param.find(L"enable_keepalive")))
	{
		value = iter->second;
		if (!_wcsicmp(value.c_str(), L"true"))
			context->keepalive = true;
		else
			context->keepalive = false;
	}
	if (param.end() != (iter = param.find(L"keepalive_timeout")))
	{
		value = iter->second;
		context->keepalive_timeout = _wtoi(value.c_str());
	}
	if (param.end() != (iter = param.find(L"enable_tls")))
	{
		value = iter->second;
		if (!_wcsicmp(value.c_str(), L"true"))
			context->tls = true;
		else
			context->tls = false;
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
	sirius::app::attendant::proxy::context_t * context = this->context();
	sirius::library::log::log4cplus::logger::create("configuration\\sirius_log_configuration.ini", SLNSC, "");

	char * mb_uuid = nullptr;
	sirius::stringhelper::convert_wide2multibyte((wchar_t*)context->uuid, &mb_uuid);

	if (mb_uuid && strlen(mb_uuid) > 0)
	{
		_core = new sirius::app::attendant::proxy::core(this, mb_uuid, context->keepalive, context->keepalive_timeout, context->tls);
	}

	if (mb_uuid)
		free(mb_uuid);

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

void sirius::app::attendant::proxy::connect_attendant_callback(int32_t code)
{
	if (_core)
		_core->connect_attendant_callback(code);
}

void sirius::app::attendant::proxy::disconnect_attendant_callback(void)
{
	if (_core)
		_core->disconnect_attendant_callback();
}

void sirius::app::attendant::proxy::start_attendant_callback(const char * client_uuid, const char * client_id)
{
	if (_core)
		_core->start_attendant_callback(client_uuid, client_id);
}

void sirius::app::attendant::proxy::stop_attendant_callback(const char * client_uuid)
{
	if (_core)
		_core->stop_attendant_callback(client_uuid);
}

void sirius::app::attendant::proxy::destroy_callback(void)
{
	if (_core)
		_core->destroy_callback();
}

void sirius::app::attendant::proxy::key_up_callback(int8_t type, int32_t key)
{
	if (_core)
	{
		if (context()->play_after_connect == false)
		{
			_key_pressed = FALSE;
		}
		_core->key_up_callback(type, key);
	}
}

void sirius::app::attendant::proxy::key_down_callback(int8_t type, int32_t key)
{
	if (_core)
	{
/*
		if (context()->play_after_connect == false)
		{
			if (_key_pressed == FALSE)
			{
				_key_pressed = TRUE;
				switch (key)
				{
				//case VK_NUMPAD4 :	if(_core) _core->Seek(-10);		break;
				//case VK_NUMPAD6 :	if(_core) _core->Seek(10);		break;
				//case VK_NUMPAD3 :	if(_core) _core->forward();		break;
				//case VK_NUMPAD1 :	if(_core) _core->backward();	break;
				//case VK_NUMPAD2 :	if(_core) _core->reverse();		break;
				//case VK_NUMPAD0 :	if(_core) _core->play_toggle();	break;
				}
			}
		}		
#*/
		_core->key_down_callback(type, key);
	}
}

void sirius::app::attendant::proxy::mouse_move_callback(int32_t pos_x, int32_t pos_y)
{
	if (_core)
		_core->mouse_move_callback(pos_x, pos_y);
}

void sirius::app::attendant::proxy::mouse_lbd_callback(int32_t pos_x, int32_t pos_y)
{
	if (_core)
		_core->mouse_lbd_callback(pos_x, pos_y);
}

void sirius::app::attendant::proxy::mouse_lbu_callback(int32_t pos_x, int32_t pos_y)
{
	if (_core)
		_core->mouse_lbu_callback(pos_x, pos_y);
}

void sirius::app::attendant::proxy::mouse_rbd_callback(int32_t pos_x, int32_t pos_y)
{
	if (_core)
		_core->mouse_rbd_callback(pos_x, pos_y);
}

void sirius::app::attendant::proxy::mouse_rbu_callback(int32_t pos_x, int32_t pos_y)
{
	if (_core)
		_core->mouse_rbu_callback(pos_x, pos_y);
}

void sirius::app::attendant::proxy::mouse_lb_dclick_callback(int32_t pos_x, int32_t pos_y)
{
	if (_core)
		_core->mouse_lb_dclick_callback(pos_x, pos_y);
}

void sirius::app::attendant::proxy::mouse_rb_dclick_callback(int32_t pos_x, int32_t pos_y)
{
	if (_core)
		_core->mouse_rb_dclick_callback(pos_x, pos_y);
}

void sirius::app::attendant::proxy::mouse_wheel_callback(int32_t pos_x, int32_t pos_y, int32_t wheel_z)
{
	if (_core)
		_core->mouse_wheel_callback(pos_x, pos_y, wheel_z);
}

void sirius::app::attendant::proxy::app_to_attendant(uint8_t * packet, int32_t len)
{
	if (_core)
		_core->app_to_attendant(packet, len);
}

void sirius::app::attendant::proxy::attendant_to_app_callback(uint8_t * packet, int32_t len)
{
	sirius::library::log::log4cplus::logger::make_info_log(SLNS, "%s, %d", __FUNCTION__, __LINE__);
	if (_core)
	{
		_core->attendant_to_app_callback(packet, len);
		sirius::library::log::log4cplus::logger::make_info_log(SLNS, "%s, %d", __FUNCTION__, __LINE__);
	}
}

void sirius::app::attendant::proxy::set_attendant_cb(FuncPtrCallback fncallback)
{
	if (_core)
		_core->set_attendant_cb(fncallback);
}