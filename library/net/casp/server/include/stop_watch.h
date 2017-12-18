#ifndef _STOP_WATCH_H_
#define _STOP_WATCH_H_
#if defined(STREAMING_BLOCKING_MODE)

#include <time.h>
#include <mmsystem.h>


class stop_watch
{
public:
	stop_watch(void);
	~stop_watch(void);

	clock_t	_initial;
	clock_t	_interval_time;
	clock_t	_time_freq;

	void	time_init();
	clock_t	get_interval();
	clock_t	get_elapsed_time();
	void begin_elapsed_time();
	//long long elapsed_microseconds();
	DWORD elapsed_microseconds();

private:
	DWORD	begin_time;
	DWORD	elapsed_time;
	LARGE_INTEGER _frequency;
	LARGE_INTEGER _begin_elapsed_microseconds;




};

#endif
#endif