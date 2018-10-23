#include "sirius_scsp_receiver.h"

sirius::library::unified::scsp::receiver::receiver(sirius::library::unified::client * front)
	: sirius::library::unified::receiver(front)
	, _recv_video(false)
{

}

sirius::library::unified::scsp::receiver::~receiver(void)
{

}

void sirius::library::unified::scsp::receiver::play(const char * url, int32_t port, int32_t recv_option, bool reconnect, bool keepalive, int32_t keepalive_timeout)
{
	sirius::library::net::scsp::client::play(url, port, recv_option, reconnect, keepalive, keepalive_timeout);
}

void sirius::library::unified::scsp::receiver::stop(void)
{
	sirius::library::net::scsp::client::stop();
}

void sirius::library::unified::scsp::receiver::on_begin_video(int32_t codec, int32_t width, int32_t height, int32_t block_width, int32_t block_height)
{
	_recv_video = true;

	if (_front)
		_front->on_begin_video(codec, width, height, block_width, block_height);
}

void sirius::library::unified::scsp::receiver::on_recv_video(int32_t codec, const uint8_t * data, int32_t length, long long dts, long long cts)
{
	if (!_recv_video)
		return;

	if (_front)
		_front->on_recv_video(codec, data, length, dts, cts);
}

void sirius::library::unified::scsp::receiver::on_recv_video(int32_t codec, int32_t count, int32_t * index, uint8_t ** data, int32_t * length, long long dts, long long cts)
{
	if (!_recv_video)
		return;

	if (_front)
		_front->on_recv_video(codec, count, index, data, length, dts, cts);
}

void sirius::library::unified::scsp::receiver::on_recv_video(int32_t codec, int32_t count, int16_t * x, int16_t * y, int16_t * width, int16_t * height, uint8_t ** data, int32_t * length, long long dts, long long cts)
{
	if (!_recv_video)
		return;

	if (_front)
		_front->on_recv_video(codec, count, x, y, width, height, data, length, dts, cts);
}

void sirius::library::unified::scsp::receiver::on_end_video(void)
{
	_recv_video = false;

	if (_front)
		_front->on_end_video();
}