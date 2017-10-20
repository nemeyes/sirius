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
#define WM_CREATING_SLOT_BEGIN_MESSAGE			WM_USER + 5
#define WM_CREATING_SLOT_END_MESSAGE			WM_USER + 6
#define WM_SUB_APP_CONNECT_MESSAGE				WM_USER + 7

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
	CEdit		_ctrl_client_app_id;
	CEdit		_ctrl_client_device_id;
	CComboBox	_ctrl_protocol_type;
	CEdit		_ctrl_url;
	CEdit		_ctrl_port;
	CEdit		_ctrl_keystroke_interval;

	CComboBox	_ctrl_client_device_type;
	CComboBox	_ctrl_client_environment_type;
	CEdit		_ctrl_client_model_name;

	CComboBox	_ctrl_attendant_resolution;


	client_controller * _client;
	client_controller * _sub_app_client;
	client_controller * _prev_client;
	
	
	sirius::library::framework::client::base *	_framework;
	HMODULE _hmodule;

	bool _auto_start;
	int32_t _keystroke_interval;
	uint32_t _playing_time;

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
	afx_msg LRESULT OnCreatingSlotBegin(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCreatingSlotEnd(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSubAppConnect(WPARAM wParam, LPARAM lParam);

	afx_msg void OnBnClickedButtonOpen();
	afx_msg void OnBnClickedButtonPlay();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedButtonConnect();
	afx_msg void OnBnClickedButtonDisconnect();
	DECLARE_MESSAGE_MAP()
public:

	bool ParseArgument(int argc, wchar_t * argv[]);

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);

	void		ClientPointToServerPoint(CPoint point, CPoint * output);
	
	int32_t		_total_time;
	int32_t		_current_time;
	float		_current_rate;
	BOOL		_pause;

	bool _sub_app_connection;
	char _sub_app_id[MAX_PATH];

private:
	void		UpdateTimeInfo();	//Internal function
	int			_clicked_pos;
	int			_width;
	int			_height;
public:
	afx_msg void OnReleasedSeekBar(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedPlayToggle();
	
	afx_msg void OnBnClickedBackward();
	afx_msg void OnBnClickedForward();
	afx_msg void OnBnClickedReverse();
	afx_msg void OnBnClickedKeystroke();
	
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	afx_msg void OnBnClickedFindFileButton();
};
