#pragma once
#include "afxcmn.h"


// progress_dlg dialog

class progress_dlg : public CDialogEx
{
	DECLARE_DYNAMIC(progress_dlg)

public:
	progress_dlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~progress_dlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PROGRESS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CProgressCtrl _progress;
};
