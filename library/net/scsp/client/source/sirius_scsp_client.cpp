#include "scsp_client.h"
#include "sirius_scsp_client.h"
#include "sirius_log4cplus_logger.h"

sirius::library::net::scsp::client::client(void)
	: _video_client(NULL)
{
	//_video_client = new sirius::library::net::scsp::client::core(this);
}

sirius::library::net::scsp::client::~client(void)
{
	/*
	if (_video_client)
	{
		_video_client->stop();
		delete _video_client;
		_video_client = nullptr;
	}
	*/
}

void sirius::library::net::scsp::client::play(const char * url, int32_t port, int32_t recv_option, bool reconnection, bool keepalive, int32_t keepalive_timeout)
{
	if (recv_option & sirius::library::net::scsp::client::media_type_t::video)
	{
		_video_client = new sirius::library::net::scsp::client::core(this, keepalive ? TRUE : FALSE, keepalive_timeout);
		if (_video_client)
		{
			_video_client->play(url, port, sirius::library::net::scsp::client::media_type_t::video, reconnection);
		}
	}
}

void sirius::library::net::scsp::client::stop(void)
{
	if (_video_client)
	{
		_video_client->stop();
		delete _video_client;
		_video_client = nullptr;
	}
}