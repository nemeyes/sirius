
// SiriusStressorDlg.h : 헤더 파일
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"

#include <vector>

#include "stressor_controller.h"

#define WM_CLIENT_CONNECTING_MSG		WM_USER+900
#define WM_CLIENT_CONNECTED_MSG			WM_USER+901
#define WM_CLIENT_DISCONNECTING_MSG		WM_USER+902
#define WM_CLIENT_DISCONNECTED_MSG		WM_USER+903

#define WM_STREAM_CONNECTING_MSG		WM_USER+904
#define WM_STREAM_CONNECTED_MSG			WM_USER+905
#define WM_STREAM_DISCONNECTING_MSG		WM_USER+906
#define WM_STREAM_DISCONNECTED_MSG		WM_USER+907

#define WM_STREAM_STATE_NONE_MSG		WM_USER+908
#define WM_STREAM_STATE_RUNNING_MSG		WM_USER+909
#define WM_STREAM_STATE_PAUSED_MSG		WM_USER+910
#define WM_STREAM_STATE_STOPPED_MSG		WM_USER+911

#define WM_STREAM_COUNT_MSG				WM_USER+912
#define WM_STREAM_LATENCY_MSG			WM_USER+913

// CSiriusStressorDlg 대화 상자
class CSiriusStressorDlg : public CDialogEx
{
// 생성입니다.
public:
	CSiriusStressorDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SIRIUS_STRESSOR_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	std::vector<stressor_controller*> _vec_client;
public:
	afx_msg LRESULT OnClientConnectingMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnClientConnectedMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnClientDisconnectingMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnClientDisconnectedMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStreamConnectingMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStreamConnectedMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStreamDisconnectingMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStreamDisconnectedMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStreamStateNoneMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStreamStateRunningMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStreamStatePausedMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStreamStateStoppedMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStreamCountMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStreamLatencyMsg(WPARAM wParam, LPARAM lParam);
public:
	CListCtrl _attendant_list;
	CButton _connect_button;
	CButton _disconnect_button;
	CIPAddressCtrl _ip_address;
	CEdit _port;
	CEdit _client_id;
	CEdit _connect_count;
	CEdit _connect_interval;
	int _key_interval;
	bool _key_loop;
public:
	afx_msg void OnBnClickedButtonConnect();
	afx_msg void OnBnClickedButtonDisconnect();		
	afx_msg void OnEnChangeEditPort();
	afx_msg void OnIpnFieldchangedIpaddressServer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeEditClientId();
	afx_msg void OnEnChangeEditConnectCount();
	afx_msg void OnEnChangeEditConnectInterval();
	afx_msg void OnBnClickedCheckAuto();
	afx_msg void OnBnClickedCheckKeyEvent();
	afx_msg void OnCustomdrawList(NMHDR* pNMHDR, LRESULT* pResult);
	
	HANDLE _connect_thread;
	void close_connect_thread_wait();
	static unsigned __stdcall connect_proc_cb(void* param);
	void connect_proc();	

	HANDLE _disconnect_thread;
	static unsigned __stdcall disconnect_proc_cb(void* param);
	void disconnect_proc();
	static unsigned __stdcall disconnect_proc_inner(void * param);
	void close_disconnect_thread_wait();
	
	bool _auto_mode_run;
	HANDLE _auto_mode_thread;
	void close_auto_mode_thread_wait();
	static unsigned __stdcall auto_mode_proc_cb(void* param);
	void auto_mode_proc();

	void load_config(void);
	void update_config(void);	
	afx_msg void OnBnClickedButtonAutoStart();
	afx_msg void OnBnClickedButtonAutoStop();
	afx_msg void OnBnClickedButtonKeySetting();
};
