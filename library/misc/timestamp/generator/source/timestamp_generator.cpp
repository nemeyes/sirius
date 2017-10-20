#include "timestamp_generator.h"

sirius::library::misc::timestamp::generator::core::core(void)
	: _begin(false)
{

}

sirius::library::misc::timestamp::generator::core::~core(void)
{

}

void sirius::library::misc::timestamp::generator::core::begin_elapsed_time(void)
{
	if (!_begin)
	{
		::QueryPerformanceFrequency(&_frequency);
		::QueryPerformanceCounter(&_begin_elapsed_microseconds);

		_begin = true;
	}
}

long long sirius::library::misc::timestamp::generator::core::elapsed_microseconds(void)
{
	LARGE_INTEGER elapsed_microseconds;
	LARGE_INTEGER now;
	::QueryPerformanceCounter(&now);
	elapsed_microseconds.QuadPart = now.QuadPart - _begin_elapsed_microseconds.QuadPart;
	elapsed_microseconds.QuadPart *= 1000000;
	if (_frequency.QuadPart > 0)
		elapsed_microseconds.QuadPart /= _frequency.QuadPart;
	else
		return 0;
	return elapsed_microseconds.QuadPart;
}

long long sirius::library::misc::timestamp::generator::core::elapsed_milliseconds(void)
{
	LARGE_INTEGER elapsed_microseconds;
	LARGE_INTEGER now;
	::QueryPerformanceCounter(&now);
	elapsed_microseconds.QuadPart = now.QuadPart - _begin_elapsed_microseconds.QuadPart;
	elapsed_microseconds.QuadPart *= 1000;
	if (_frequency.QuadPart > 0)
		elapsed_microseconds.QuadPart /= _frequency.QuadPart;
	else
		return 0;
	return elapsed_microseconds.QuadPart;
}