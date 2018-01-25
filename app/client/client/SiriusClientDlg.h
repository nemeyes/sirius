#pragma once

#include <sirius_stringhelper.h>
#include "client_controller.h"
#include <sirius_client_framework.h>
#include "afxwin.h"

/*
#define WM_AMADEUS_CLIENT_STATE_MESSAGE			WM_USER + 1
#define WM_ENABLE_BUTTON_MESSAGE				WM_USER + 2
*/

#define WM_CONNECTION_BEGIN_MESSAGE				WM_USER + 1
#define WM_CONNECTION_END_MESSAGE				WM_USER + 2
#define WM_DISCONNECTION_BEGIN_MESSAGE			WM_USER + 3
#define WM_DISCONNECTION_END_MESSAGE			WM_USER + 4
#define WM_CREATING_ATTENDANT_BEGIN_MESSAGE			WM_USER + 5
#define WM_CREATING_ATTENDANT_END_MESSAGE			WM_USER + 6

class CSiriusClientDlg : public CDialogEx
{
	friend class client_controller;
public:
	CSiriusClientDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.
	void SetServerStatus(CString status);
	void SetEncoderStatus(CString status);
// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CLOUDMEDIAEDGECLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.

private:
	CEdit		_ctrl_address;
	CEdit		_ctrl_port_number;
	CEdit		_ctrl_device_id;
	CEdit		_ctrl_url;
	CEdit		_ctrl_port;

	client_controller * _client;
		
	sirius::library::framework::client::base *	_framework;
	HMODULE _hmodule;
	bool	_auto_start;
	int32_t _video_width;
	int32_t _video_height;

public:
	char _version[MAX_PATH];
	
// 구현입니다.
protected:
	HICON m_hIcon;

	void EnableConnectButton(BOOL enable);
	void EnableDisconnectButton(BOOL enable);

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	virtual BOOL DestroyWindow();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnConnectionBegin(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnConnectionEnd(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDisconnectionBegin(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDisconnectionEnd(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCreatingAttendantBegin(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCreatingAttendantEnd(WPARAM wParam, LPARAM lParam);

	afx_msg void OnBnClickedButtonOpen();
	afx_msg void OnBnClickedButtonPlay();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedButtonConnect();
	afx_msg void OnBnClickedButtonDisconnect();
	afx_msg void OnBnClickedButtonToApp();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	afx_msg void OnBnClickedFindFileButton();


	bool ParseArgument(int argc, wchar_t * argv[]);
	void ClientPointToServerPoint(CPoint point, CPoint * output);

	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
