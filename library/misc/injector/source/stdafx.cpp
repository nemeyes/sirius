// stdafx.cpp : 표준 포함 파일만 들어 있는 소스 파일입니다.
// inject_helper.pch는 미리 컴파일된 헤더가 됩니다.
// stdafx.obj에는 미리 컴파일된 형식 정보가 포함됩니다.

#include "stdafx.h"
#include <stdio.h>
#include <memory>

#ifdef UNICODE
#define tstring std::wstring
#define tostringstream std::wostringstream
#else
#define tstring std::string                      
#define tostringstream std::ostringstream
#endif

#define FMTDBGSTR(stream)  ((tostringstream&)(tostringstream() << tstring() << stream)).str().c_str() 


void trace(const wchar_t* format, ...)
{
#if 1
	wchar_t *buffer = NULL;
	va_list args;

	va_start(args, format);
	int size = _vscwprintf_p(format, args);
	std::wstring result(++size, 0);
	_vsnwprintf_s((wchar_t*)result.data(), size, _TRUNCATE, format, args);
	va_end(args);

	free(buffer);

	OutputDebugString(FMTDBGSTR("[" << GetCurrentThreadId() << "] " << result));
#endif // _DEBUG
}