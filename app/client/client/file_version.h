#ifndef _CAP_FILE_VER_H__
#define _CAP_FILE_VER_H__

#include <string>
#include "stdafx.h"

#ifdef __cplusplus
  extern "C" {
#endif

class cap_file_version
{ 
/* constructor/destructor */
public: 
    cap_file_version();
    virtual ~cap_file_version(); 

/* methods */
public: 
    bool    open(LPCTSTR module_name);
    void    close();

    LPCTSTR get_query_value(LPCTSTR value_name, DWORD language_charset = 0);
    LPCTSTR get_file_desc()  {return get_query_value(_T("FileDescription")); };
    LPCTSTR get_file_version();
    LPCTSTR get_internal_name()     {return get_query_value(_T("InternalName"));    };
    LPCTSTR get_company_name()      {return get_query_value(_T("CompanyName"));     };
    LPCTSTR get_legal_copyright()   {return get_query_value(_T("LegalCopyright"));  };
    LPCTSTR get_org_filename() {return get_query_value(_T("OriginalFilename"));};
    LPCTSTR get_product_name()      {return get_query_value(_T("ProductName"));     };
    LPCTSTR get_product_version();

protected:
    LPBYTE		_version_data; 
    int32_t	_language_charset; 
	TCHAR	_temp_value[MAX_PATH];

	VS_FIXEDFILEINFO _vsffi;
	bool		_file_ver_valid;		// Version info is loaded	

}; 

#ifdef __cplusplus
    }
#endif

BOOL get_module_file_version( LPCTSTR szModuleName, LPTSTR szFileVer );

#endif
