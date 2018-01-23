
// SiriusStressorDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "SiriusStressor.h"
#include "SiriusStressorDlg.h"
#include "afxdialogex.h"

#include <thread>
#include <fstream>

#include "sirius_stringhelper.h"
#include "json\json.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ���� ���α׷� ������ ���Ǵ� CAboutDlg ��ȭ �����Դϴ�.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

// �����Դϴ�.
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


// CSiriusStressorDlg ��ȭ ����



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
	ON_EN_CHANGE(IDC_EDIT_PORT, &CSiriusStressorDlg::OnEnChangeEditPort)
	ON_NOTIFY(IPN_FIELDCHANGED, IDC_IPADDRESS_SERVER, &CSiriusStressorDlg::OnIpnFieldchangedIpaddressServer)
	ON_EN_CHANGE(IDC_EDIT_CLIENT_ID, &CSiriusStressorDlg::OnEnChangeEditClientId)
	ON_EN_CHANGE(IDC_EDIT_CONNECT_COUNT, &CSiriusStressorDlg::OnEnChangeEditConnectCount)
	ON_EN_CHANGE(IDC_EDIT_CONNECT_INTERVAL, &CSiriusStressorDlg::OnEnChangeEditConnectInterval)
END_MESSAGE_MAP()


// Csirius_stressorDlg �޽��� ó����

BOOL CSiriusStressorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �ý��� �޴��� "����..." �޴� �׸��� �߰��մϴ�.

	// IDM_ABOUTBOX�� �ý��� ��� ������ �־�� �մϴ�.
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

	// �� ��ȭ ������ �������� �����մϴ�.  ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.

	_attendant_list.InsertColumn(0, _T("no."), LVCFMT_CENTER, 50);
	_attendant_list.InsertColumn(1, _T("ip address"), LVCFMT_CENTER, 130);
	_attendant_list.InsertColumn(2, _T("server connect"), LVCFMT_CENTER, 120);
	_attendant_list.InsertColumn(3, _T("stream connect"), LVCFMT_CENTER, 120);
	_attendant_list.InsertColumn(4, _T("stream state"), LVCFMT_CENTER, 100);
	_attendant_list.InsertColumn(5, _T("frame count"), LVCFMT_CENTER, 90);
	_attendant_list.InsertColumn(6, _T("latency(ms)"), LVCFMT_CENTER, 100);
	_attendant_list.ModifyStyle(LVS_TYPEMASK, LVS_REPORT);

	_attendant_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	
	load_config();
	
	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
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

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�.  ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CSiriusStressorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
HCURSOR CSiriusStressorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSiriusStressorDlg::OnBnClickedButtonConnect()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	_connect_button.EnableWindow(FALSE);
	_disconnect_button.EnableWindow(FALSE);

	unsigned int connect_thread_id = 0;
	_connect_thread = (HANDLE)_beginthreadex(NULL, 0, CSiriusStressorDlg::connect_proc_cb, this, 0, &connect_thread_id);	
}

void CSiriusStressorDlg::OnBnClickedButtonDisconnect()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	_connect_button.EnableWindow(FALSE);
	_disconnect_button.EnableWindow(FALSE);
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
	CString str_server_address;
	CString str_server_port;
	CString str_connect_count;
	CString str_connect_interval;

	_connect_count.GetWindowTextW(str_connect_count);
	_connect_interval.GetWindowTextW(str_connect_interval);
	_ip_address.GetWindowTextW(str_server_address);
	_port.GetWindowTextW(str_server_port);

	wchar_t server_address[16];
	wcsncpy_s(server_address, (LPCWSTR)str_server_address, str_server_address.GetLength() + 1);
	int32_t connect_count = _ttoi(str_connect_count);
	int32_t connect_interval = _ttoi(str_connect_interval);
	int32_t server_port = _ttoi(str_server_port);

	int32_t client_count = _vec_client.size();
	for (int32_t i = 0; i < connect_count; ++i)
	{
		int index = client_count + i;

		CString strItem = _T("");
		strItem.Format(_T("%d"), index);
		_attendant_list.InsertItem(index, strItem);
		_attendant_list.SetItem(index, 1, LVIF_TEXT, server_address, 0, 0, 0, NULL);
		_attendant_list.SetItem(index, 2, LVIF_TEXT, _T("disconnected"), 0, 0, 0, NULL);
		_attendant_list.SetItem(index, 3, LVIF_TEXT, _T("disconnected"), 0, 0, 0, NULL);
		_attendant_list.SetItem(index, 4, LVIF_TEXT, _T("none"), 0, 0, 0, NULL);
		_attendant_list.SetItem(index, 5, LVIF_TEXT, _T("0"), 0, 0, 0, NULL);
		_attendant_list.SetItem(index, 6, LVIF_TEXT, _T("0"), 0, 0, 0, NULL);
		_attendant_list.Update(index);
	}
		
	for (int i = 0; i < connect_count; 	i++)
	{
		int index = client_count + i;
		stressor_controller* client = new stressor_controller(this, index, true, false);
		client->connect(server_address, server_port, false);
		_vec_client.push_back(client);

		if (connect_count > 1)
			::Sleep(connect_interval);
	}
	_connect_button.EnableWindow(TRUE);
	_disconnect_button.EnableWindow(TRUE);
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

	_connect_button.EnableWindow(TRUE);
	_disconnect_button.EnableWindow(TRUE);
}

unsigned CSiriusStressorDlg::disconnect_proc_inner(void* param)
{
	stressor_controller * client = static_cast<stressor_controller*>(param);	

	if (client->state() != sirius::app::client::proxy::state_t::disconnected)
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

LRESULT CSiriusStressorDlg::OnClientConnectingMsg(WPARAM wParam, LPARAM lParam)
{
	int index = wParam;
	_attendant_list.SetItem(index, 2, LVIF_TEXT, _T("connecting"), 0, 0, 0, NULL);
	_attendant_list.Update(index);

	return 0;
}

LRESULT CSiriusStressorDlg::OnClientConnectedMsg(WPARAM wParam, LPARAM lParam)
{
	int index = wParam;
	_attendant_list.SetItem(index, 2, LVIF_TEXT, _T("connected"), 0, 0, 0, NULL);
	_attendant_list.Update(index);

	return 0;
}

LRESULT CSiriusStressorDlg::OnClientDisconnectingMsg(WPARAM wParam, LPARAM lParam)
{
	int index = wParam;
	_attendant_list.SetItem(index, 2, LVIF_TEXT, _T("disconnecting"), 0, 0, 0, NULL);
	_attendant_list.Update(index);

	return 0;
}

LRESULT CSiriusStressorDlg::OnClientDisconnectedMsg(WPARAM wParam, LPARAM lParam)
{
	int index = wParam;
	_attendant_list.SetItem(index, 2, LVIF_TEXT, _T("disconnected"), 0, 0, 0, NULL);
	_attendant_list.Update(index);

	return 0;
}

LRESULT CSiriusStressorDlg::OnStreamConnectingMsg(WPARAM wParam, LPARAM lParam)
{
	int index = wParam;
	_attendant_list.SetItem(index, 3, LVIF_TEXT, _T("connecting"), 0, 0, 0, NULL);
	_attendant_list.Update(index);

	return 0;
}

LRESULT CSiriusStressorDlg::OnStreamConnectedMsg(WPARAM wParam, LPARAM lParam)
{
	int index = wParam;
	_attendant_list.SetItem(index, 3, LVIF_TEXT, _T("connected"), 0, 0, 0, NULL);
	_attendant_list.Update(index);

	return 0;
}

LRESULT CSiriusStressorDlg::OnStreamDisconnectingMsg(WPARAM wParam, LPARAM lParam)
{
	int index = wParam;
	_attendant_list.SetItem(index, 3, LVIF_TEXT, _T("disconnecting"), 0, 0, 0, NULL);
	_attendant_list.Update(index);

	return 0;
}

LRESULT CSiriusStressorDlg::OnStreamDisconnectedMsg(WPARAM wParam, LPARAM lParam)
{
	int index = wParam;
	_attendant_list.SetItem(index, 3, LVIF_TEXT, _T("disconnected"), 0, 0, 0, NULL);
	_attendant_list.Update(index);

	return 0;
}

LRESULT CSiriusStressorDlg::OnStreamCountMsg(WPARAM wParam, LPARAM lParam)
{
	int index = wParam;
	int stream_count = lParam;

	CString str_stream_count;
	str_stream_count.Format(_T("%d"), stream_count);

	_attendant_list.SetItem(index, 5, LVIF_TEXT, str_stream_count, 0, 0, 0, NULL);
	_attendant_list.Update(index);

	return 0;
}


LRESULT CSiriusStressorDlg::OnStreamStateNoneMsg(WPARAM wParam, LPARAM lParam)
{
	int index = wParam;
	_attendant_list.SetItem(index, 4, LVIF_TEXT, _T("none"), 0, 0, 0, NULL);
	_attendant_list.Update(index);

	return 0;
}

LRESULT CSiriusStressorDlg::OnStreamStateRunningMsg(WPARAM wParam, LPARAM lParam)
{
	int index = wParam;
	_attendant_list.SetItem(index, 4, LVIF_TEXT, _T("running"), 0, 0, 0, NULL);
	_attendant_list.Update(index);

	return 0;
}

LRESULT CSiriusStressorDlg::OnStreamStatePausedMsg(WPARAM wParam, LPARAM lParam)
{
	int index = wParam;
	_attendant_list.SetItem(index, 4, LVIF_TEXT, _T("paused"), 0, 0, 0, NULL);
	_attendant_list.Update(index);

	return 0;
}

LRESULT CSiriusStressorDlg::OnStreamStateStoppedMsg(WPARAM wParam, LPARAM lParam)
{
	int index = wParam;
	_attendant_list.SetItem(index, 4, LVIF_TEXT, _T("stopped"), 0, 0, 0, NULL);
	_attendant_list.Update(index);

	return 0;
}

LRESULT CSiriusStressorDlg::OnStreamLatencyMsg(WPARAM wParam, LPARAM lParam)
{
	int index = wParam;
	int latency = lParam;

	CString str_latency;
	str_latency.Format(_T("%d"), latency);

	_attendant_list.SetItem(index, 6, LVIF_TEXT, str_latency, 0, 0, 0, NULL);
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

	//�Է��� edit_control ������ json���Ϸ� ����

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

void CSiriusStressorDlg::OnIpnFieldchangedIpaddressServer(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
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
