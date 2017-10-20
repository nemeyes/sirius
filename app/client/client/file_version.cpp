#include "stdafx.h"
#include <stdio.h>
#include <assert.h>

#include "file_version.h"

#pragma comment(lib, "Version.lib")

BOOL get_module_file_version( LPCTSTR module_name, LPTSTR file_ver )
{
	if ( (module_name ==NULL ) || (file_ver ==NULL) )
		return FALSE;

	cap_file_version fv;

	if ( !fv.open(module_name) )
		return FALSE;

	if ( (file_ver ==NULL ) )
		return FALSE;

#ifdef _UNICODE
	wcscpy_s(file_ver,MAX_PATH,  fv.get_file_version());
#else
	strcpy_s(file_ver, MAX_PATH, fv.get_file_version());
#endif

	return TRUE;
}

cap_file_version::cap_file_version() :_version_data(NULL)
							 , _language_charset(0)
							 ,_file_ver_valid(FALSE)
{ 
	ZeroMemory(_temp_value,MAX_PATH);
}

cap_file_version::~cap_file_version() 
{ 
    close();
} 

/*-----------------------------------------------------------------------------
  class methods definitions
-----------------------------------------------------------------------------*/

void cap_file_version::close()
{
    delete[] _version_data; 
    _language_charset = 0;
}

bool cap_file_version::open(LPCTSTR module_name)
{
    if ( (module_name == NULL ) || _version_data != nullptr )
        return FALSE;

    // Get the version information size for allocate the buffer
    DWORD handle;     
    DWORD data_size = ::GetFileVersionInfoSize((LPTSTR)module_name, &handle);
    if (data_size == 0 )
        return FALSE;

    // Allocate buffer and retrieve version information
	_version_data = new BYTE[data_size];
    if (!::GetFileVersionInfo((LPTSTR)module_name, handle, data_size,
                                  (void**)_version_data) )
    {
        close();
        return FALSE;
    }

	UINT   uLen    = 0;
	LPVOID vs_ffi = NULL;

	if ( ::VerQueryValue(_version_data, _T( "\\" ), (LPVOID*)&vs_ffi, &uLen ) )
	{
		::CopyMemory( &_vsffi, vs_ffi, sizeof( VS_FIXEDFILEINFO ) );
		_file_ver_valid = ( _vsffi.dwSignature == VS_FFI_SIGNATURE );
	}

    // Retrieve the first language and character-set identifier
    UINT query_size;
    DWORD* trans_table;
    if (!::VerQueryValue(_version_data, _T("\\VarFileInfo\\Translation"),
                         (void **)&trans_table, &query_size) )
    {
		close();
        return FALSE;
    }

    // Swap the words to have lang-charset in the correct format
    _language_charset = MAKELONG(HIWORD(trans_table[0]), LOWORD(trans_table[0]));

    return TRUE;
}


LPCTSTR cap_file_version::get_query_value(LPCTSTR value_name, DWORD language_charset /* = 0*/)
{
    if ( ( value_name == NULL ) || (_version_data == nullptr) )
        return FALSE;

    // Must call Open() first
	
    // If no lang-charset specified use default
    if ( language_charset == 0 )
        language_charset = language_charset;
	
    // Query version information value
    UINT nQuerySize;
    LPVOID lpData;
	TCHAR strBlockName[MAX_PATH];
#ifdef _UNICODE
	swprintf_s(strBlockName, MAX_PATH, _T("\\StringFileInfo\\%08lx\\%s"),
		language_charset, value_name);

	if (::VerQueryValue((void **)_version_data, strBlockName,
		&lpData, &nQuerySize)) {
		::wcsncpy_s(_temp_value, MAX_PATH,(LPTSTR)lpData, MAX_PATH);
}
#else
    sprintf_s(strBlockName, MAX_PATH, "\\StringFileInfo\\%08lx\\%s",
		language_charset, value_name);

	if (::VerQueryValue((void **)_version_data, strBlockName,
		&lpData, &nQuerySize)) {
		::strncpy_s(_temp_value, MAX_PATH, (LPSTR)lpData, MAX_PATH);
	}
#endif


	
    return _temp_value;
}

LPCTSTR cap_file_version::get_file_version()
{
	assert(_file_ver_valid);

	UINT major,minor,build,qfe;

	major = HIWORD( _vsffi.dwFileVersionMS );
	minor = LOWORD( _vsffi.dwFileVersionMS );
	build = HIWORD( _vsffi.dwFileVersionLS );
	qfe = LOWORD( _vsffi.dwFileVersionLS );
#ifdef _UNICODE
	::swprintf_s(_temp_value, MAX_PATH,
		_T("%d.%d.%d.%d"),
		major,
		minor,
		build,
		qfe
	);
#else
	::sprintf_s( _temp_value, MAX_PATH,
		"%d.%d.%d.%d",
		major,
		minor,
		build,
		qfe 
	);
#endif
	return _temp_value;
}

LPCTSTR cap_file_version::get_product_version()
{
	assert(_file_ver_valid);

	UINT major,minor,build,qfe;

	major = HIWORD( _vsffi.dwProductVersionMS );
	minor = LOWORD( _vsffi.dwProductVersionMS );
	build = HIWORD( _vsffi.dwProductVersionLS );
	qfe = LOWORD( _vsffi.dwProductVersionLS );
#ifdef _UNICODE
	::swprintf_s(_temp_value, MAX_PATH,
		_T("%d.%d.%d.%d"),
		major,
		minor,
		build,
		qfe
	);
#else
	::sprintf_s( _temp_value, MAX_PATH,
		"%d.%d.%d.%d",
		major,
		minor,
		build,
		qfe 
		);
#endif
	return _temp_value;
}
