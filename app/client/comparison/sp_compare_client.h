
// sp_compare_client.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// sp_comparison_client_app:
// See sp_compare_client.cpp for the implementation of this class
//

class sp_comparison_client_app : public CWinApp
{
public:
	sp_comparison_client_app();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern sp_comparison_client_app theApp;