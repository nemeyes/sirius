#include "StdAfx.h"
#include "stop_watch.h"

#if defined(STREAMING_BLOCKING_MODE)

stop_watch::stop_watch(void)
{
	time_init();
}

stop_watch::~stop_watch(void)
{
	timeEndPeriod(1);
}

void stop_watch::time_init()
{
	timeBeginPeriod(1);
	_initial = clock();
	_interval_time = _initial;
}

clock_t stop_watch::get_interval()
{
	clock_t time = clock();
	clock_t ret_value;
	ret_value = time - _interval_time;
	_interval_time = time;

#if 0
	DWORD ret;
	elapsed_time = timeGetTime();
	ret = elapsed_time;
	begin_time = elapsed_time;

	return ret;
#endif
	return ret_value;
}

clock_t stop_watch::get_elapsed_time()
{
	clock_t time = clock();
	clock_t ret_value;
	ret_value = time - _initial;
	return ret_value;
}

void stop_watch::begin_elapsed_time(void)
{
#if 1
	begin_time = timeGetTime();
#else
	::QueryPerformanceFrequency(&_frequency);
	::QueryPerformanceCounter(&_begin_elapsed_microseconds);
#endif
}

//long long stop_watch::elapsed_microseconds(void)
DWORD stop_watch::elapsed_microseconds(void)
{
#if 1
	DWORD ret;
	elapsed_time = timeGetTime();
	ret = elapsed_time - begin_time;
	return ret;
#else
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
#endif
}

#endif