
// sirius_warbitrator.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// sirius_warbitrator_app:
// See sirius_warbitrator.cpp for the implementation of this class
//

class sirius_warbitrator_app : public CWinApp
{
public:
	sirius_warbitrator_app();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern sirius_warbitrator_app theApp;