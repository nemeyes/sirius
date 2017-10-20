#include "sirius_scsp_receiver.h"

sirius::library::unified::scsp::receiver::receiver(sirius::library::unified::client * front)
	: sirius::library::unified::receiver(front)
	, _recv_video(false)
{

}

sirius::library::unified::scsp::receiver::~receiver(void)
{

}

void sirius::library::unified::scsp::receiver::play(const char * url, int32_t port, int32_t recv_option, bool repeat)
{
	sirius::library::net::scsp::client::play(url, port, recv_option, repeat);
}

void sirius::library::unified::scsp::receiver::stop(void)
{
	sirius::library::net::scsp::client::stop();
}

void sirius::library::unified::scsp::receiver::on_begin_video(int32_t smt, const uint8_t * data, size_t data_size, long long dts, long long cts)
{
	_recv_video = true;

	if (_front)
		_front->on_begin_video(smt, data, data_size, dts, cts);
}

void sirius::library::unified::scsp::receiver::on_recv_video(int32_t smt, const uint8_t * data, size_t data_size, long long dts, long long cts)
{
	if (!_recv_video)
		return;

	if (_front)
		_front->on_recv_video(smt, data, data_size, dts, cts);
}

void sirius::library::unified::scsp::receiver::on_end_video(void)
{
	_recv_video = false;

	if (_front)
		_front->on_end_video();
}