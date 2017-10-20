#include "sirius_file_receiver.h"
#include <process.h>

sirius::library::unified::file::receiver::receiver(sirius::library::unified::client * front)
	: sirius::library::unified::receiver(front)
	, _recv_video(false)
	, _run(false)
	, _thread(INVALID_HANDLE_VALUE)
{

}

sirius::library::unified::file::receiver::~receiver(void)
{

}

void sirius::library::unified::file::receiver::play(const char * url, int32_t port, int32_t recv_option, bool repeat)
{
	if (!url || strlen(url)<1)
		return;

	strncpy_s(_url, url, strlen(url));

	uint32_t thrdaddr;
	_run = true;
	_thread = (HANDLE)::_beginthreadex(NULL, 0, sirius::library::unified::file::receiver::process_cb, this, 0, &thrdaddr);
}

void sirius::library::unified::file::receiver::stop(void)
{
	if (_thread != NULL && _thread != INVALID_HANDLE_VALUE)
	{
		_run = false;
		if (::WaitForSingleObject(_thread, INFINITE) == WAIT_OBJECT_0)
		{
			::CloseHandle(_thread);
			_thread = NULL;
		}
	}
}

void sirius::library::unified::file::receiver::on_begin_video(int32_t smt, const uint8_t * data, size_t data_size, long long dts, long long cts)
{
	_recv_video = true;

	if (_front)
		_front->on_begin_video(smt, data, data_size, dts, cts);
}

void sirius::library::unified::file::receiver::on_recv_video(int32_t smt, const uint8_t * data, size_t data_size, long long dts, long long cts)
{
	if (!_recv_video)
		return;

	if (_front)
		_front->on_recv_video(smt, data, data_size, dts, cts);
}

void sirius::library::unified::file::receiver::on_end_video(void)
{
	_recv_video = false;

	if (_front)
		_front->on_end_video();
}

unsigned sirius::library::unified::file::receiver::process_cb(void * param)
{
	sirius::library::unified::file::receiver * self = static_cast<sirius::library::unified::file::receiver*>(param);
	self->process();
	return 0;
}

void sirius::library::unified::file::receiver::process(void)
{
	HANDLE f = ::CreateFileA(_url, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (f == NULL || f == INVALID_HANDLE_VALUE)
		return;

	char * slash = (char*)strrchr(_url, '\\');
	char * filename = slash + 1;

	long long fs = file_size(f);
	unsigned long fr = 0;
	uint8_t * fb = static_cast<uint8_t*>(malloc(fs));
	memset(fb, 0x00, fs);

	::SetFilePointer(f, 0, FILE_BEGIN, false);
	::ReadFile(f, fb, fs, &fr, NULL);

	::CloseHandle(f);

	on_begin_video(sirius::library::unified::client::video_submedia_type_t::png, fb, fs, 0, 0);
	while (_run)
	{
		on_recv_video(sirius::library::unified::client::video_submedia_type_t::png, fb, fs, 0, 0);
		::Sleep(66);
	}
	on_end_video();

	if (fb)
		free(fb);
	fr = NULL;
}

long long sirius::library::unified::file::receiver::file_size(HANDLE f)
{
	if (f == NULL || f == INVALID_HANDLE_VALUE)
		return 0;
	LARGE_INTEGER filesize = { 0 };
	::GetFileSizeEx(f, &filesize);
	long long estimated_filesize = 0;
	estimated_filesize = filesize.HighPart;
	estimated_filesize <<= 32;
	estimated_filesize |= filesize.LowPart;
	return estimated_filesize;
}