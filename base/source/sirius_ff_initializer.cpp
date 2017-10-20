#include "sirius_ff_initializer.h"

#if defined(_WIN32)
#include <windows.h>
#endif

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
	#include <libavutil/opt.h>
	#include <libavutil/mathematics.h>
}

#if defined(_WIN32)
int ff_lock_manager(void ** mutex, enum AVLockOp op)
{
	CRITICAL_SECTION ** lock = (CRITICAL_SECTION**)mutex;
	switch (op)
	{
	case AV_LOCK_CREATE:
		(*lock) = new CRITICAL_SECTION();
		InitializeCriticalSection((*lock));
		break;
	case AV_LOCK_OBTAIN:
		EnterCriticalSection((*lock));
		break;
	case AV_LOCK_RELEASE:
		LeaveCriticalSection((*lock));
		break;
	case AV_LOCK_DESTROY:
		DeleteCriticalSection((*lock));
		delete (*lock);
		break;
	}
	return 0;
}
#endif

sirius::ff::initializer::initializer(void)
{
	av_register_all();
#if defined(_WIN32)
	av_lockmgr_register(ff_lock_manager);
#endif
}

sirius::ff::initializer::~initializer(void)
{
#if defined(_WIN32)
	av_lockmgr_register(0);
#endif
}

static sirius::ff::initializer ffmpeg;