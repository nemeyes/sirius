
// sirius_stressor.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CSiriusStressorApp:
// �� Ŭ������ ������ ���ؼ��� SiriusStressor.cpp�� �����Ͻʽÿ�.
//

class CSiriusStressorApp : public CWinApp
{
public:
	CSiriusStressorApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CSiriusStressorApp theApp;