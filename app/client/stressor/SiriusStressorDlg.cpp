
// SiriusStressorDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "SiriusStressor.h"
#include "SiriusStressorDlg.h"
#include "SiriusStressorKeyMacroDlg.h"
#include "afxdialogex.h"

#include <thread>
#include <fstream>

#include "sirius_stringhelper.h"
#include "json\json.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSiriusStressorDlg 대화 상자



CSiriusStressorDlg::CSiriusStressorDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SIRIUS_STRESSOR_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSiriusStressorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_ATTENDANT, _attendant_list);
	DDX_Control(pDX, IDC_BUTTON_CONNECT, _connect_button);
	DDX_Control(pDX, IDC_BUTTON_DISCONNECT, _disconnect_button);
	DDX_Control(pDX, IDC_IPADDRESS_SERVER, _ip_address);
	DDX_Control(pDX, IDC_EDIT_PORT, _port);
	DDX_Control(pDX, IDC_EDIT_CLIENT_ID, _client_id);
	DDX_Control(pDX, IDC_EDIT_CONNECT_COUNT, _connect_count);
	DDX_Control(pDX, IDC_EDIT_CONNECT_INTERVAL, _connect_interval);
	DDX_Control(pDX, IDC_EDIT_KEEPALIVE_TIMEMOUT, _keepalive_timeout);
}

BEGIN_MESSAGE_MAP(CSiriusStressorDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CSiriusStressorDlg::OnBnClickedButtonConnect)
	ON_BN_CLICKED(IDC_BUTTON_DISCONNECT, &CSiriusStressorDlg::OnBnClickedButtonDisconnect)
	ON_MESSAGE(WM_CLIENT_CONNECTING_MSG, &CSiriusStressorDlg::OnClientConnectingMsg)
	ON_MESSAGE(WM_CLIENT_CONNECTED_MSG, &CSiriusStressorDlg::OnClientConnectedMsg)
	ON_MESSAGE(WM_CLIENT_DISCONNECTING_MSG, &CSiriusStressorDlg::OnClientDisconnectingMsg)
	ON_MESSAGE(WM_CLIENT_DISCONNECTED_MSG, &CSiriusStressorDlg::OnClientDisconnectedMsg)
	ON_MESSAGE(WM_STREAM_CONNECTING_MSG, &CSiriusStressorDlg::OnStreamConnectingMsg)
	ON_MESSAGE(WM_STREAM_CONNECTED_MSG, &CSiriusStressorDlg::OnStreamConnectedMsg)
	ON_MESSAGE(WM_STREAM_DISCONNECTING_MSG, &CSiriusStressorDlg::OnStreamDisconnectingMsg)
	ON_MESSAGE(WM_STREAM_DISCONNECTED_MSG, &CSiriusStressorDlg::OnStreamDisconnectedMsg)
	ON_MESSAGE(WM_STREAM_COUNT_MSG, &CSiriusStressorDlg::OnStreamCountMsg)
	ON_MESSAGE(WM_STREAM_STATE_NONE_MSG, &CSiriusStressorDlg::OnStreamStateNoneMsg)
	ON_MESSAGE(WM_STREAM_STATE_RUNNING_MSG, &CSiriusStressorDlg::OnStreamStateRunningMsg)
	ON_MESSAGE(WM_STREAM_STATE_PAUSED_MSG, &CSiriusStressorDlg::OnStreamStatePausedMsg)
	ON_MESSAGE(WM_STREAM_STATE_STOPPED_MSG, &CSiriusStressorDlg::OnStreamStateStoppedMsg)
	ON_MESSAGE(WM_STREAM_LATENCY_MSG, &CSiriusStressorDlg::OnStreamLatencyMsg)
	ON_MESSAGE(WM_STREAM_PORT_MSG, &CSiriusStressorDlg::OnStreamPortMsg)
	ON_EN_CHANGE(IDC_EDIT_PORT, &CSiriusStressorDlg::OnEnChangeEditPort)
	ON_NOTIFY(IPN_FIELDCHANGED, IDC_IPADDRESS_SERVER, &CSiriusStressorDlg::OnIpnFieldchangedIpaddressServer)
	ON_EN_CHANGE(IDC_EDIT_CLIENT_ID, &CSiriusStressorDlg::OnEnChangeEditClientId)
	ON_EN_CHANGE(IDC_EDIT_CONNECT_COUNT, &CSiriusStressorDlg::OnEnChangeEditConnectCount)
	ON_EN_CHANGE(IDC_EDIT_CONNECT_INTERVAL, &CSiriusStressorDlg::OnEnChangeEditConnectInterval)
	ON_BN_CLICKED(IDC_CHECK_AUTO, &CSiriusStressorDlg::OnBnClickedCheckAuto)
	ON_BN_CLICKED(IDC_CHECK_KEY_EVENT, &CSiriusStressorDlg::OnBnClickedCheckKeyEvent)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_START, &CSiriusStressorDlg::OnBnClickedButtonAutoStart)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_STOP, &CSiriusStressorDlg::OnBnClickedButtonAutoStop)
	ON_BN_CLICKED(IDC_BUTTON_KEY_SETTING, &CSiriusStressorDlg::OnBnClickedButtonKeySetting)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_ATTENDANT, OnCustomdrawList)
END_MESSAGE_MAP()


// Csirius_stressorDlg 메시지 처리기

BOOL CSiriusStressorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	
	GetDlgItem(IDC_BUTTON_CONNECT)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_DISCONNECT)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_AUTO_START)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_AUTO_STOP)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_KEY_SETTING)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO_KEY_LOOP_ON)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO_KEY_LOOP_OFF)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_KEY_INTERVAL)->EnableWindow(FALSE);

	CButton * pLoopCheckOn = (CButton*)GetDlgItem(IDC_RADIO_KEY_LOOP_ON);
	pLoopCheckOn->SetCheck(true);
	SetDlgItemText(IDC_EDIT_KEY_INTERVAL, L"5");
	SetDlgItemText(IDC_EDIT_KEEPALIVE_TIMEMOUT, L"5000");
	
	_attendant_list.InsertColumn(0, _T("no."), LVCFMT_CENTER, 30);
	_attendant_list.InsertColumn(1, _T("ip address"), LVCFMT_CENTER, 130);
	_attendant_list.InsertColumn(2, _T("port"), LVCFMT_CENTER, 70);
	_attendant_list.InsertColumn(3, _T("server connect"), LVCFMT_CENTER, 120);
	_attendant_list.InsertColumn(4, _T("stream connect"), LVCFMT_CENTER, 120);
	_attendant_list.InsertColumn(5, _T("stream state"), LVCFMT_CENTER, 100);
	_attendant_list.InsertColumn(6, _T("frame count"), LVCFMT_CENTER, 90);
	//_attendant_list.InsertColumn(7, _T("latency(ms)"), LVCFMT_CENTER, 100);
	_attendant_list.ModifyStyle(LVS_TYPEMASK, LVS_REPORT);

	_attendant_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	
	load_config();
	
	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CSiriusStressorDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CSiriusStressorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CSiriusStressorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSiriusStressorDlg::OnBnClickedButtonConnect()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	GetDlgItem(IDC_BUTTON_CONNECT)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_DISCONNECT)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK_AUTO)->EnableWindow(FALSE);

	GetDlgItem(IDC_CHECK_KEY_EVENT)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_KEY_SETTING)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO_KEY_LOOP_ON)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO_KEY_LOOP_OFF)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_KEY_INTERVAL)->EnableWindow(FALSE);

	unsigned int connect_thread_id = 0;
	_connect_thread = (HANDLE)_beginthreadex(NULL, 0, CSiriusStressorDlg::connect_proc_cb, this, 0, &connect_thread_id);	
}

void CSiriusStressorDlg::OnBnClickedButtonDisconnect()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	GetDlgItem(IDC_BUTTON_CONNECT)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_DISCONNECT)->EnableWindow(FALSE);

	unsigned int disconnect_thread_id = 0;
	_disconnect_thread = (HANDLE)_beginthreadex(NULL, 0, CSiriusStressorDlg::disconnect_proc_cb, this, 0, &disconnect_thread_id);
}

unsigned CSiriusStressorDlg::connect_proc_cb(void* param)
{
	CSiriusStressorDlg * self = static_cast<CSiriusStressorDlg*>(param);
	self->connect_proc();
	return 0;
}

void CSiriusStressorDlg::connect_proc()
{
	CString str_key_interval;
	GetDlgItem(IDC_EDIT_KEY_INTERVAL)->GetWindowTextW(str_key_interval);
	_key_interval = _ttoi(str_key_interval);
	_key_loop = ((CButton*)GetDlgItem(IDC_RADIO_KEY_LOOP_ON))->GetCheck();

	CString str_server_address;
	CString str_server_port;
	CString str_connect_count;
	CString str_connect_interval;
	CString str_keepalive_timeout;

	_connect_count.GetWindowTextW(str_connect_count);
	_connect_interval.GetWindowTextW(str_connect_interval);
	_ip_address.GetWindowTextW(str_server_address);
	_port.GetWindowTextW(str_server_port);
	_keepalive_timeout.GetWindowTextW(str_keepalive_timeout);

	wchar_t server_address[16];
	wcsncpy_s(server_address, (LPCWSTR)str_server_address, str_server_address.GetLength() + 1);
	int32_t connect_count = _ttoi(str_connect_count);
	int32_t connect_interval = _ttoi(str_connect_interval);
	int32_t server_port = _ttoi(str_server_port);
	int32_t keepalive_timeout = _ttoi(str_keepalive_timeout);

	int32_t client_count = _vec_client.size();
	for (int32_t i = 0; i < connect_count; ++i)
	{
		int index = client_count + i;

		CString strItem = _T("");
		strItem.Format(_T("%d"), index);
		_attendant_list.InsertItem(index, strItem);
		_attendant_list.SetItem(index, 1, LVIF_TEXT, server_address, 0, 0, 0, NULL);
		_attendant_list.SetItem(index, 2, LVIF_TEXT, _T("-"), 0, 0, 0, NULL);
		_attendant_list.SetItem(index, 3, LVIF_TEXT, _T("disconnected"), 0, 0, 0, NULL);
		_attendant_list.SetItem(index, 4, LVIF_TEXT, _T("disconnected"), 0, 0, 0, NULL);
		_attendant_list.SetItem(index, 5, LVIF_TEXT, _T("none"), 0, 0, 0, NULL);
		_attendant_list.SetItem(index, 6, LVIF_TEXT, _T("0"), 0, 0, 0, NULL);
		_attendant_list.SetItem(index, 7, LVIF_TEXT, _T("0"), 0, 0, 0, NULL);
		_attendant_list.Update(index);
	}
		
	for (int i = 0; i < connect_count; 	i++)
	{
		int index = client_count + i;
		stressor_controller* client = new stressor_controller(this, index, keepalive_timeout > 0 ? true:false, keepalive_timeout, IsDlgButtonChecked(IDC_CHECK_USE_TLS));
		client->connect(server_address, server_port, IsDlgButtonChecked(IDC_CHECK_RECONNECTION));
		_vec_client.push_back(client);

		if (connect_count > 1)
			::Sleep(connect_interval);
	}

	if (!IsDlgButtonChecked(IDC_CHECK_AUTO))
	{
		GetDlgItem(IDC_BUTTON_CONNECT)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_DISCONNECT)->EnableWindow(TRUE);
	}
}

void CSiriusStressorDlg::close_connect_thread_wait()
{
	if (_connect_thread != INVALID_HANDLE_VALUE)
	{
		if (::WaitForSingleObject(_connect_thread, INFINITE) == WAIT_OBJECT_0)
		{
			::CloseHandle(_connect_thread);
		}
		_connect_thread = INVALID_HANDLE_VALUE;
	}
}

unsigned CSiriusStressorDlg::disconnect_proc_cb(void* param)
{
	CSiriusStressorDlg * self = static_cast<CSiriusStressorDlg*>(param);
	self->disconnect_proc();
	return 0;
}

void CSiriusStressorDlg::disconnect_proc()
{
	CString str_connect_interval;
	_connect_interval.GetWindowTextW(str_connect_interval);
	int32_t connect_interval = _ttoi(str_connect_interval);	

	std::vector<std::thread> vec_disconnect_inner_thread;
	if (_vec_client.size() > 0)
	{
		for (UINT i = 0; i < _vec_client.size(); i++)
		{	
			stressor_controller * client = _vec_client[i];
			if (client)
			{			
				vec_disconnect_inner_thread.push_back(std::thread(&CSiriusStressorDlg::disconnect_proc_inner, client));
				::Sleep(20);
			}
			else
			{
				vec_disconnect_inner_thread.push_back(std::thread());
			}
			::Sleep(connect_interval);
		}
	}
	//wait disconnect thread
	for (int i = 0; i < vec_disconnect_inner_thread.size(); ++i)
	{
		if (vec_disconnect_inner_thread[i].joinable())
			vec_disconnect_inner_thread[i].join();
	}
	_vec_client.clear();
	vec_disconnect_inner_thread.clear();
	_attendant_list.DeleteAllItems();

	::Sleep(1000);
	
	if (!IsDlgButtonChecked(IDC_CHECK_AUTO))
	{
		GetDlgItem(IDC_BUTTON_CONNECT)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_DISCONNECT)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_AUTO)->EnableWindow(TRUE);

		if (IsDlgButtonChecked(IDC_CHECK_KEY_EVENT))
		{
			GetDlgItem(IDC_CHECK_KEY_EVENT)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_KEY_SETTING)->EnableWindow(TRUE);
			GetDlgItem(IDC_RADIO_KEY_LOOP_ON)->EnableWindow(TRUE);
			GetDlgItem(IDC_RADIO_KEY_LOOP_OFF)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_KEY_INTERVAL)->EnableWindow(TRUE);
		}
		else
		{
			GetDlgItem(IDC_CHECK_KEY_EVENT)->EnableWindow(TRUE);
		}
	}	
}

unsigned CSiriusStressorDlg::disconnect_proc_inner(void* param)
{
	stressor_controller * client = static_cast<stressor_controller*>(param);	
	client->disconnect();	
	delete client;
	client = nullptr;
	return 0;
}

void CSiriusStressorDlg::close_disconnect_thread_wait()
{
	if (_disconnect_thread != INVALID_HANDLE_VALUE)
	{
		if (::WaitForSingleObject(_disconnect_thread, INFINITE) == WAIT_OBJECT_0)
		{
			::CloseHandle(_disconnect_thread);
		}
		_disconnect_thread = INVALID_HANDLE_VALUE;
	}
}



unsigned CSiriusStressorDlg::auto_mode_proc_cb(void* param)
{
	CSiriusStressorDlg * self = static_cast<CSiriusStressorDlg*>(param);
	self->auto_mode_proc();
	return 0;
}

void CSiriusStressorDlg::auto_mode_proc()
{
	unsigned int disconnect_thread_id = 0;
	_disconnect_thread = (HANDLE)_beginthreadex(NULL, 0, CSiriusStressorDlg::disconnect_proc_cb, this, 0, &disconnect_thread_id);
	close_disconnect_thread_wait();

	while (_auto_mode_run)
	{
		// connect
		unsigned int connect_thread_id = 0;
		_connect_thread = (HANDLE)_beginthreadex(NULL, 0, CSiriusStressorDlg::connect_proc_cb, this, 0, &connect_thread_id);
		close_connect_thread_wait();
		// !connect
		
		::Sleep(1000 * 3);

		// disconnect		
		_disconnect_thread = (HANDLE)_beginthreadex(NULL, 0, CSiriusStressorDlg::disconnect_proc_cb, this, 0, &disconnect_thread_id);
		close_disconnect_thread_wait();
		//!disconnect
	}
	GetDlgItem(IDC_BUTTON_AUTO_START)->EnableWindow(TRUE);
	GetDlgItem(IDC_CHECK_AUTO)->EnableWindow(TRUE);

	if (IsDlgButtonChecked(IDC_CHECK_KEY_EVENT))
	{
		GetDlgItem(IDC_CHECK_KEY_EVENT)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_KEY_SETTING)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_KEY_LOOP_ON)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_KEY_LOOP_OFF)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_KEY_INTERVAL)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_CHECK_KEY_EVENT)->EnableWindow(TRUE);
	}
}

void CSiriusStressorDlg::close_auto_mode_thread_wait()
{
	if (_auto_mode_thread != INVALID_HANDLE_VALUE)
	{
		if (::WaitForSingleObject(_auto_mode_thread, INFINITE) == WAIT_OBJECT_0)
		{
			::CloseHandle(_auto_mode_thread);
		}
		_auto_mode_thread = INVALID_HANDLE_VALUE;
	}
}


void CSiriusStressorDlg::OnBnClickedButtonAutoStart()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	GetDlgItem(IDC_CHECK_AUTO)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_AUTO_START)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_AUTO_STOP)->EnableWindow(TRUE);

	GetDlgItem(IDC_CHECK_KEY_EVENT)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_KEY_SETTING)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO_KEY_LOOP_ON)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO_KEY_LOOP_OFF)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_KEY_INTERVAL)->EnableWindow(FALSE);

	_auto_mode_run = true;
	unsigned int auto_mode_thread_id = 0;
	_auto_mode_thread = (HANDLE)_beginthreadex(NULL, 0, CSiriusStressorDlg::auto_mode_proc_cb, this, 0, &auto_mode_thread_id);	
}


void CSiriusStressorDlg::OnBnClickedButtonAutoStop()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	GetDlgItem(IDC_BUTTON_AUTO_START)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_AUTO_STOP)->EnableWindow(FALSE);
	
	_auto_mode_run = false;
}


LRESULT CSiriusStressorDlg::OnClientConnectingMsg(WPARAM wParam, LPARAM lParam)
{
	int index = wParam;
	_attendant_list.SetItem(index, 3, LVIF_TEXT, _T("connecting"), 0, 0, 0, NULL);
	_attendant_list.Update(index);

	return 0;
}

LRESULT CSiriusStressorDlg::OnClientConnectedMsg(WPARAM wParam, LPARAM lParam)
{
	int index = wParam;
	_attendant_list.SetItem(index, 3, LVIF_TEXT, _T("connected"), 0, 0, 0, NULL);
	_attendant_list.Update(index);

	return 0;
}

LRESULT CSiriusStressorDlg::OnClientDisconnectingMsg(WPARAM wParam, LPARAM lParam)
{
	int index = wParam;
	_attendant_list.SetItem(index, 3, LVIF_TEXT, _T("disconnecting"), 0, 0, 0, NULL);
	_attendant_list.Update(index);

	return 0;
}

LRESULT CSiriusStressorDlg::OnClientDisconnectedMsg(WPARAM wParam, LPARAM lParam)
{
	int index = wParam;
	_attendant_list.SetItem(index, 3, LVIF_TEXT, _T("disconnected"), 0, 0, 0, NULL);
	_attendant_list.Update(index);

	return 0;
}

LRESULT CSiriusStressorDlg::OnStreamConnectingMsg(WPARAM wParam, LPARAM lParam)
{
	int index = wParam;
	_attendant_list.SetItem(index, 4, LVIF_TEXT, _T("connecting"), 0, 0, 0, NULL);
	_attendant_list.Update(index);

	return 0;
}

LRESULT CSiriusStressorDlg::OnStreamConnectedMsg(WPARAM wParam, LPARAM lParam)
{
	int index = wParam;
	_attendant_list.SetItem(index, 4, LVIF_TEXT, _T("connected"), 0, 0, 0, NULL);
	_attendant_list.Update(index);

	return 0;
}

LRESULT CSiriusStressorDlg::OnStreamDisconnectingMsg(WPARAM wParam, LPARAM lParam)
{
	int index = wParam;
	_attendant_list.SetItem(index, 4, LVIF_TEXT, _T("disconnecting"), 0, 0, 0, NULL);
	_attendant_list.Update(index);

	return 0;
}

LRESULT CSiriusStressorDlg::OnStreamDisconnectedMsg(WPARAM wParam, LPARAM lParam)
{
	int index = wParam;
	_attendant_list.SetItem(index, 4, LVIF_TEXT, _T("disconnected"), 0, 0, 0, NULL);
	_attendant_list.Update(index);

	return 0;
}

LRESULT CSiriusStressorDlg::OnStreamCountMsg(WPARAM wParam, LPARAM lParam)
{
	int index = wParam;
	int stream_count = lParam;

	CString str_stream_count;
	str_stream_count.Format(_T("%d"), stream_count);

	_attendant_list.SetItem(index, 6, LVIF_TEXT, str_stream_count, 0, 0, 0, NULL);
	_attendant_list.Update(index);

	return 0;
}


LRESULT CSiriusStressorDlg::OnStreamStateNoneMsg(WPARAM wParam, LPARAM lParam)
{
	int index = wParam;
	_attendant_list.SetItem(index, 5, LVIF_TEXT, _T("none"), 0, 0, 0, NULL);
	_attendant_list.Update(index);

	return 0;
}

LRESULT CSiriusStressorDlg::OnStreamStateRunningMsg(WPARAM wParam, LPARAM lParam)
{
	int index = wParam;
	_attendant_list.SetItem(index, 5, LVIF_TEXT, _T("running"), 0, 0, 0, NULL);
	_attendant_list.Update(index);

	return 0;
}

LRESULT CSiriusStressorDlg::OnStreamStatePausedMsg(WPARAM wParam, LPARAM lParam)
{
	int index = wParam;
	_attendant_list.SetItem(index, 5, LVIF_TEXT, _T("paused"), 0, 0, 0, NULL);
	_attendant_list.Update(index);

	return 0;
}

LRESULT CSiriusStressorDlg::OnStreamStateStoppedMsg(WPARAM wParam, LPARAM lParam)
{
	int index = wParam;
	_attendant_list.SetItem(index, 5, LVIF_TEXT, _T("stopped"), 0, 0, 0, NULL);
	_attendant_list.Update(index);

	return 0;
}

LRESULT CSiriusStressorDlg::OnStreamLatencyMsg(WPARAM wParam, LPARAM lParam)
{
	int index = wParam;
	int latency = lParam;

	CString str_latency;
	str_latency.Format(_T("%d"), latency);

	_attendant_list.SetItem(index, 7, LVIF_TEXT, str_latency, 0, 0, 0, NULL);
	_attendant_list.Update(index);

	return 0;
}


LRESULT CSiriusStressorDlg::OnStreamPortMsg(WPARAM wParam, LPARAM lParam)
{
	int index = wParam;
	int port = lParam;

	CString str_port;
	str_port.Format(_T("%d"), port);

	_attendant_list.SetItem(index, 2, LVIF_TEXT, str_port, 0, 0, 0, NULL);
	_attendant_list.Update(index);

	return 0;
}


void CSiriusStressorDlg::load_config(void)
{
	std::ifstream fin;
	fin.open(L"config.json");
	const int bufferLength = 1024;
	char readBuffer[bufferLength] = { 0, };
	fin.read(readBuffer, bufferLength);
	fin.close();

	std::string config_doc = readBuffer;

	Json::Value root;
	Json::Reader reader;
	if (reader.parse(config_doc, root) == false)
	{
		_ip_address.SetWindowTextW(L"127.0.0.1");
		_port.SetWindowTextW(L"5000");
		_client_id.SetWindowTextW(L"aa:bb:cc:dd");
		_connect_count.SetWindowTextW(L"1");
		_connect_interval.SetWindowTextW(L"1000");
	}
	else
	{
		std::string ip = root.get("ip", "127.0.0.1").asString();
		std::string port = root.get("port", "5000").asString();
		std::string client_id = root.get("client_id", "aa:bb:cc:dd").asString();
		std::string connect_count = root.get("connect_count", "1").asString();
		std::string connect_interval = root.get("connect_interval", "1000").asString();
	
		wchar_t* w_ip = nullptr;
		wchar_t* w_port = nullptr;
		wchar_t* w_client_id = nullptr;
		wchar_t* w_connect_count = nullptr;
		wchar_t* w_connect_interval = nullptr;

		sirius::stringhelper::convert_multibyte2wide((char*)ip.c_str(), &w_ip);
		sirius::stringhelper::convert_multibyte2wide((char*)port.c_str(), &w_port);
		sirius::stringhelper::convert_multibyte2wide((char*)client_id.c_str(), &w_client_id);
		sirius::stringhelper::convert_multibyte2wide((char*)connect_count.c_str(), &w_connect_count);
		sirius::stringhelper::convert_multibyte2wide((char*)connect_interval.c_str(), &w_connect_interval);

		_ip_address.SetWindowTextW(w_ip);
		_port.SetWindowTextW(w_port);
		_client_id.SetWindowTextW(w_client_id);
		_connect_count.SetWindowTextW(w_connect_count);
		_connect_interval.SetWindowTextW(w_connect_interval);
	
		SysFreeString(w_ip);
		SysFreeString(w_port);
		SysFreeString(w_client_id);
		SysFreeString(w_connect_count);
		SysFreeString(w_connect_interval);
	}
}

void CSiriusStressorDlg::update_config(void)
{
	CString str_ip_addr;
	CString str_port;
	CString str_client_id;
	CString str_connect_count;
	CString str_connect_interval;
	//CString str_connect_num_at_once;

	_ip_address.GetWindowTextW(str_ip_addr);
	_port.GetWindowTextW(str_port);
	_client_id.GetWindowTextW(str_client_id);
	_connect_count.GetWindowTextW(str_connect_count);
	_connect_interval.GetWindowTextW(str_connect_interval);

	char* ip_address = nullptr;
	char* port = nullptr;
	char* client_id = nullptr;
	char* connect_count = nullptr;
	char* connect_interval = nullptr;

	sirius::stringhelper::convert_wide2multibyte((LPWSTR)(LPCWSTR)str_ip_addr, &ip_address);
	sirius::stringhelper::convert_wide2multibyte((LPWSTR)(LPCWSTR)str_port, &port);
	sirius::stringhelper::convert_wide2multibyte((LPWSTR)(LPCWSTR)str_client_id, &client_id);
	sirius::stringhelper::convert_wide2multibyte((LPWSTR)(LPCWSTR)str_connect_count, &connect_count);
	sirius::stringhelper::convert_wide2multibyte((LPWSTR)(LPCWSTR)str_connect_interval, &connect_interval);

	//입력한 edit_control 데이터 json파일로 저장

	Json::Value Jroot;
	Jroot["ip"] = ip_address;
	Jroot["port"] = port;
	Jroot["client_id"] = client_id;
	Jroot["connect_count"] = connect_count;
	Jroot["connect_interval"] = connect_interval;
	
	Json::StyledWriter writer;
	std::string outputConfig = writer.write(Jroot);

	std::ofstream file(L"config.json", std::ios_base::out | std::ios_base::trunc);
	file.write(outputConfig.c_str(), outputConfig.length());
	file.close();

	free(ip_address);
	free(port);	
	free(client_id);
	free(connect_count);
	free(connect_interval);

}

void CSiriusStressorDlg::OnBnClickedCheckAuto()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (IsDlgButtonChecked(IDC_CHECK_AUTO))
	{
		GetDlgItem(IDC_BUTTON_CONNECT)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_DISCONNECT)->EnableWindow(FALSE);

		GetDlgItem(IDC_BUTTON_AUTO_START)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_AUTO_STOP)->EnableWindow(FALSE);		
	}
	else
	{
		GetDlgItem(IDC_BUTTON_CONNECT)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_DISCONNECT)->EnableWindow(FALSE);

		GetDlgItem(IDC_BUTTON_AUTO_START)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_AUTO_STOP)->EnableWindow(FALSE);
	}	
}


void CSiriusStressorDlg::OnBnClickedCheckKeyEvent()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (IsDlgButtonChecked(IDC_CHECK_KEY_EVENT))
	{
		GetDlgItem(IDC_BUTTON_KEY_SETTING)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_KEY_LOOP_ON)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_KEY_LOOP_OFF)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_KEY_INTERVAL)->EnableWindow(TRUE);	
	}
	else
	{
		GetDlgItem(IDC_BUTTON_KEY_SETTING)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_KEY_LOOP_ON)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_KEY_LOOP_OFF)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_KEY_INTERVAL)->EnableWindow(FALSE);

	}
}

void CSiriusStressorDlg::OnBnClickedButtonKeySetting()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CSiriusStressorKeyMacroDlg sirius_stressor_key_macro_dlg;
	sirius_stressor_key_macro_dlg.DoModal();
	sirius_stressor_key_macro_dlg.DestroyWindow();
}

void CSiriusStressorDlg::OnIpnFieldchangedIpaddressServer(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	update_config();
	*pResult = 0;
}

void CSiriusStressorDlg::OnEnChangeEditPort()
{
	update_config();
}

void CSiriusStressorDlg::OnEnChangeEditClientId()
{
	update_config();
}


void CSiriusStressorDlg::OnEnChangeEditConnectCount()
{
	update_config();
}


void CSiriusStressorDlg::OnEnChangeEditConnectInterval()
{
	update_config();
}

void CSiriusStressorDlg::OnCustomdrawList(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
	*pResult = 0;
	if (CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if (CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		if ((pLVCD->nmcd.dwItemSpec % 2) == 0)
		{
			pLVCD->clrText = RGB(0, 0, 0);
			pLVCD->clrTextBk = RGB(230, 230, 230);
		}
		else
		{
			pLVCD->clrText = RGB(0, 0, 0);
			pLVCD->clrTextBk = RGB(255, 255, 255);
		}
		*pResult = CDRF_DODEFAULT;
	}
}