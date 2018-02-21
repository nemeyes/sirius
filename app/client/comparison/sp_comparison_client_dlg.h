
// sp_comparison_client_dlg.h : header file
//

#pragma once

#include <sirius_stringhelper.h>
#include "arodnap_client.h"
#include "afxwin.h"

// sp_comparison_client_dlg dialog
class sp_comparison_client_dlg : public CDialog
{
// Construction
public:
	sp_comparison_client_dlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SP_COMPARE_CLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


private:
	arodnap_client *	_arodnap_client;

	CEdit				_arodnap_address;
	CEdit				_arodnap_portnumber;
	CEdit				_sirius_address;
	CEdit				_sirius_portnumber;

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedButtonConnect();
	afx_msg void OnBnClickedButtonDisconnect();
	DECLARE_MESSAGE_MAP()

};
