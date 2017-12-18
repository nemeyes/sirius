#include "unified_client.h"
#include <sirius_stringhelper.h>
#include <sirius_locks.h>

#include <shlwapi.h>

sirius::library::unified::client::core::core(sirius::library::unified::client * front)
	: _front(front)
	, _state(sirius::library::unified::client::state_t::none)
	, _scsp_receiver(nullptr)
	, _casp_receiver(nullptr)
	, _file_receiver(nullptr)
	, _bfile(false)
{
	::InitializeCriticalSection(&_cs);
}

sirius::library::unified::client::core::~core(void)
{
	//stop();
	::DeleteCriticalSection(&_cs);
}

int32_t sirius::library::unified::client::core::state(void)
{
	return _state;
}

int32_t sirius::library::unified::client::core::open(wchar_t * url, int32_t port, int32_t recv_option, bool repeat)
{
	sirius::autolock mutex(&_cs);

	char * mb_url = 0;
	sirius::stringhelper::convert_wide2multibyte(url, &mb_url);
	if (mb_url && strlen(mb_url) > 0)
	{
		strncpy_s(_url, mb_url, sizeof(_url));
		if (PathFileExistsA(_url))
			_bfile = true;
		_port = port;
		_recv_option = recv_option;
		_repeat = repeat;
		free(mb_url);
	}
	return sirius::library::unified::client::err_code_t::success;
}

int32_t sirius::library::unified::client::core::play(void)
{
	sirius::autolock mutex(&_cs);

	if ((_state == sirius::library::unified::client::state_t::none) || (_state == sirius::library::unified::client::state_t::stopped))
	{
		if (_bfile)
		{
			_file_receiver = new sirius::library::unified::file::receiver(_front);
			_file_receiver->play(_url);
		}
		else
		{
#ifdef WITH_CASP
			_casp_receiver = new sirius::library::unified::casp::receiver(_front);
			_casp_receiver->play(_url, _port, _recv_option, _repeat);	
#else
			_scsp_receiver = new sirius::library::unified::scsp::receiver(_front);
			_scsp_receiver->play(_url, _port, _recv_option, _repeat);
#endif
		}
		_state = sirius::library::unified::client::state_t::running;
		return sirius::library::unified::client::err_code_t::success;
	}
	else
	{
		return sirius::library::unified::client::err_code_t::fail;
	}
}

int32_t sirius::library::unified::client::core::stop(void)
{
	sirius::autolock mutex(&_cs);

	if (_state == sirius::library::unified::client::state_t::running)
	{
		if (_bfile)
		{
			if (_file_receiver)
			{
				_file_receiver->stop();
				delete _file_receiver;
				_file_receiver = nullptr;
			}
		}
		else
		{
			if (_scsp_receiver)
			{
				_scsp_receiver->stop();
				delete _scsp_receiver;
				_scsp_receiver = nullptr;
			}

			if (_casp_receiver)
			{
				_casp_receiver->stop();
				delete _casp_receiver;
				_casp_receiver = nullptr;
			}
		}
		_state = sirius::library::unified::client::state_t::stopped;
		return sirius::library::unified::client::err_code_t::success;
	}
	else
	{
		return sirius::library::unified::client::err_code_t::fail;
	}
}