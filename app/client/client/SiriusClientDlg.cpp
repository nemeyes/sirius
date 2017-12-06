
// CloudMediaEdgeClientDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "afxdialogex.h"
#include <sirius_stringhelper.h>
#include <sirius_log4cplus_logger.h>
#include <memory>
#include <string>
#include <map>

#include "SiriusClient.h"
#include "SiriusClientDlg.h"

#include <sirius_curl_client.h>
#include <sirius_string.h>
#include "configure.h"

#include <sirius_xml_parser.h>
#include <json/json.h>
#include "file_version.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define PROTOCOL_FILE	0
#define PROTOCOL_CASP	1
#define PROTOCOL_RTSP	2 
#define PROTOCOL_RTMP	3 
#define PROTOCOL_HLS	4
#define PROTOCOL_TS_UDP 5
#define PROTOCOL_TS_TCP 6
#define PROTOCOL_TS_MULTICAST 7
// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

#define TIMER_ID_PLAYING_TIME 1000

typedef sirius::library::framework::client::base* (*fpn_create_client_framework)();
typedef void(*fpn_destory_client_framework)(sirius::library::framework::client::base ** client_framework);

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


// CCloudMediaEdgeClientDlg 대화 상자


CSiriusClientDlg * clientDlg = nullptr;

CSiriusClientDlg::CSiriusClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CLOUDMEDIAEDGECLIENT_DIALOG, pParent)
	, _auto_start(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	clientDlg = this;
}

void CSiriusClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_ADDRESS, _ctrl_address);
	DDX_Control(pDX, IDC_EDIT_PORT_NUMBER, _ctrl_port_number);
	DDX_Control(pDX, IDC_EDIT_CLIENT_STB_ID, _ctrl_device_id);
	DDX_Control(pDX, IDC_EDIT_CLIENT_URL, _ctrl_url);
	DDX_Control(pDX, IDC_EDIT_CLIENT_PORT, _ctrl_port);
	DDX_Control(pDX, IDC_EDIT_KEYSTROKE_INTERVAL, _ctrl_keystroke_interval);
}

BEGIN_MESSAGE_MAP(CSiriusClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_CONNECTION_BEGIN_MESSAGE, &CSiriusClientDlg::OnConnectionBegin)
	ON_MESSAGE(WM_CONNECTION_END_MESSAGE, &CSiriusClientDlg::OnConnectionEnd)
	ON_MESSAGE(WM_DISCONNECTION_BEGIN_MESSAGE, &CSiriusClientDlg::OnDisconnectionBegin)
	ON_MESSAGE(WM_DISCONNECTION_END_MESSAGE, &CSiriusClientDlg::OnDisconnectionEnd)
	ON_MESSAGE(WM_CREATING_ATTENDANT_BEGIN_MESSAGE, &CSiriusClientDlg::OnCreatingAttendantBegin)
	ON_MESSAGE(WM_CREATING_ATTENDANT_END_MESSAGE, &CSiriusClientDlg::OnCreatingAttendantEnd)
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CSiriusClientDlg::OnBnClickedButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_PLAY, &CSiriusClientDlg::OnBnClickedButtonPlay)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CSiriusClientDlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CSiriusClientDlg::OnBnClickedButtonConnect)
	ON_BN_CLICKED(IDC_BUTTON_DISCONNECT, &CSiriusClientDlg::OnBnClickedButtonDisconnect)
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_HSCROLL()
	ON_WM_WINDOWPOSCHANGED()
	ON_BN_CLICKED(IDC_FIND_FILE_BUTTON, &CSiriusClientDlg::OnBnClickedFindFileButton)
END_MESSAGE_MAP()


// CCloudMediaEdgeClientDlg 메시지 처리기

BOOL CSiriusClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	sirius::library::log::log4cplus::logger::create("configuration\\log.properties", SAC, "");

	HINSTANCE hInstance = GetModuleHandleA(NULL);

	sirius::string title;
	title.format(_T("Sirius Client Ver : %s"), sirius::string::atow(GEN_VER_VERSION_STRING).c_str());

	SetWindowText(title.c_str());

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
	EnableConnectButton(TRUE);
	EnableDisconnectButton(FALSE);

	if (!Config.load()) 
	{
		MessageBox(_T("can't load sirius_client.ini. \r\n It must be the configuration folder."), _T("Error"), MB_OK);
		PostMessage(WM_CLOSE);
		return FALSE;
	}

	if (!_framework)
	{
		HINSTANCE module_handle = ::GetModuleHandleA("sirius_client.exe");
		char module_path[MAX_PATH] = { 0 };
		char * module_name = module_path;
		module_name += GetModuleFileNameA(module_handle, module_name, (sizeof(module_path) / sizeof(*module_path)) - (module_name - module_path));
		if (module_name != module_path)
		{
			CHAR * slash = strrchr(module_path, '\\');
			if (slash != NULL)
			{
				module_name = slash + 1;
				_strset_s(module_name, strlen(module_name) + 1, 0);
			}
			else
			{
				_strset_s(module_path, strlen(module_path) + 1, 0);
			}
		}
		if (strlen(module_path)>0)
		{
			SetDllDirectoryA(module_path);
			_hmodule = ::LoadLibraryA("sirius_native_client_framework.dll");
			if (_hmodule)
			{
				fpn_create_client_framework pfn_create = (fpn_create_client_framework)::GetProcAddress(_hmodule, "create_client_framework");
				if (pfn_create)
				{
					_framework = (pfn_create)();
					if (!_framework)
					{
						FreeLibrary(_hmodule);
						_hmodule = NULL;
					}
				}
				else
				{
					FreeLibrary(_hmodule);
					_hmodule = NULL;
				}
			}
		}
	}


	CString command = GetCommandLine();
	int32_t argc = 0;
	LPTSTR * argv = CommandLineToArgvW(command, &argc);
	
	ParseArgument(argc, argv);

	if (!_auto_start)
		_ctrl_address.SetWindowText(Config.get_server_address().c_str());; // _sirius_address.SetWindowText(L"10.202.142.53");

		
	_ctrl_port_number.SetWindowTextW(Config.get_server_port().c_str());
	CheckDlgButton(IDC_CHECK_RETRY_CONNECTION, TRUE);
	CheckDlgButton(IDC_CHECK_AUTO_RECONNECTION, TRUE);
	CheckDlgButton(IDC_CHECK_KEYSTROKE, FALSE);
	GetDlgItem(IDC_EDIT_KEYSTROKE_INTERVAL)->EnableWindow(FALSE);
	
	_ctrl_device_id.SetWindowTextW(Config.get_attendant_device_id().c_str());
	_ctrl_url.SetWindowTextW(Config.get_url().c_str());
	_ctrl_port.SetWindowTextW(Config.get_port().c_str());

	if(_auto_start)
		SendDlgItemMessage(IDC_BUTTON_CONNECT, BM_CLICK);

	
	//((CSliderCtrl*)GetDlgItem(IDC_SLIDER_SEEK_BAR))->SetRange(0, 100);
	//((CSliderCtrl*)GetDlgItem(IDC_SLIDER_SEEK_BAR))->SetPageSize(1);

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

BOOL CSiriusClientDlg::DestroyWindow()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (_client)
	{
		_client->disconnect();
		delete _client;
		_client = nullptr;
	}

	if (_hmodule)
	{
		fpn_destory_client_framework pfn_destroy = (fpn_destory_client_framework)::GetProcAddress(_hmodule, "destroy_client_framework");
		if (_framework)
		{
			(pfn_destroy)(&_framework);
			_framework = nullptr;
		}
		FreeLibrary(_hmodule);
		_hmodule = NULL;
	}

	CString server_address, server_port, attendant_app_id, attendant_device_id, protocol_type, url, port;
			
	_ctrl_address.GetWindowText(server_address);
	_ctrl_port_number.GetWindowText(server_port);
	_ctrl_device_id.GetWindowText(attendant_device_id);
	_ctrl_url.GetWindowText(url);
	_ctrl_port.GetWindowText(port);

	Config.set_server_address(sirius::string(server_address));
	Config.set_server_port(sirius::string(server_port));
	Config.set_attendant_app_id(sirius::string(attendant_app_id));
	Config.set_attendant_device_id(sirius::string(attendant_device_id));
	Config.set_url(sirius::string(url));
	Config.set_port(sirius::string(port));

	Config.save();
		
	sirius::library::log::log4cplus::logger::destroy();
	return CDialogEx::DestroyWindow();
}

void CSiriusClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CSiriusClientDlg::OnPaint()
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
		CBrush brush;
		brush.CreateSolidBrush(RGB(0, 0, 0));

		CRect rect;
		CWnd * video_view = GetDlgItem(IDC_STATIC_VIDEO_VIEW);
		CDC * video_view_dc = video_view->GetDC();
		video_view->GetClientRect(rect);
		video_view_dc->FillRect(rect, &brush);
		brush.DeleteObject();

		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CSiriusClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

afx_msg LRESULT CSiriusClientDlg::OnConnectionBegin(WPARAM wParam, LPARAM lParam)
{
	SetServerStatus(L"connecting.");
	CWnd * wnd_disconnect = GetDlgItem(IDC_BUTTON_CONNECT);
	if (wnd_disconnect)
		wnd_disconnect->EnableWindow(FALSE);

	return 0;
}

afx_msg LRESULT CSiriusClientDlg::OnConnectionEnd(WPARAM wParam, LPARAM lParam)
{
	SetServerStatus(L"connected.");
	CWnd * wnd_disconnect = GetDlgItem(IDC_BUTTON_DISCONNECT);
	if (wnd_disconnect)
		wnd_disconnect->EnableWindow(TRUE);

	return 0;
}

afx_msg LRESULT CSiriusClientDlg::OnDisconnectionBegin(WPARAM wParam, LPARAM lParam)
{
	SetServerStatus(L"disconnecting.");
	CWnd * wnd_disconnect = GetDlgItem(IDC_BUTTON_DISCONNECT);
	if (wnd_disconnect)
		wnd_disconnect->EnableWindow(FALSE);

	return 0;
}

afx_msg LRESULT CSiriusClientDlg::OnDisconnectionEnd(WPARAM wParam, LPARAM lParam)
{	
	SetServerStatus(L"disconnected.");
	CWnd * wnd_disconnect = GetDlgItem(IDC_BUTTON_CONNECT);
	if (wnd_disconnect)
		wnd_disconnect->EnableWindow(TRUE);

	wnd_disconnect = GetDlgItem(IDC_BUTTON_DISCONNECT);
	if (wnd_disconnect)
		wnd_disconnect->EnableWindow(FALSE);

	return 0;
}

afx_msg LRESULT CSiriusClientDlg::OnCreatingAttendantBegin(WPARAM wParam, LPARAM lParam)
{
	SetServerStatus(L"creating attendant.");

	return 0;
}

afx_msg LRESULT CSiriusClientDlg::OnCreatingAttendantEnd(WPARAM wParam, LPARAM lParam)
{
	SetServerStatus(L"created attendant.");

	return 0;
}

void CSiriusClientDlg::SetServerStatus(CString status)
{
	CWnd * wnd = GetDlgItem(IDC_STATIC_SERVER_STATUS);
	wnd->SetWindowText(status);
}

void CSiriusClientDlg::SetEncoderStatus(CString status)
{
	CWnd * wnd = GetDlgItem(IDC_STATIC_ENCODER_STATUS);
	wnd->SetWindowText(status);
}

void CSiriusClientDlg::EnableConnectButton(BOOL enable)
{
	CWnd * wnd = GetDlgItem(IDC_BUTTON_CONNECT);
	wnd->EnableWindow(enable);
}

void CSiriusClientDlg::EnableDisconnectButton(BOOL enable)
{
	CWnd * wnd = GetDlgItem(IDC_BUTTON_DISCONNECT);
	wnd->EnableWindow(enable);
}

void CSiriusClientDlg::OnBnClickedButtonOpen()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString url;
	CString port;

	bool reconnection = IsDlgButtonChecked(IDC_CHECK_AUTO_RECONNECTION)?true:false;
	_ctrl_url.GetWindowTextW(url);
	_ctrl_port.GetWindowTextW(port);
	if (_framework)
		_framework->open((LPWSTR)(LPCWSTR)url, _wtoi(port), sirius::base::media_type_t::video | sirius::base::media_type_t::audio, reconnection);
}

void CSiriusClientDlg::OnBnClickedButtonPlay()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	HWND hwnd = ::GetDlgItem(GetSafeHwnd(), IDC_STATIC_VIDEO_VIEW);
	if (_framework)
		_framework->play(hwnd);
}

void CSiriusClientDlg::OnBnClickedButtonStop()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (_framework)
		_framework->stop();
}

void CSiriusClientDlg::OnBnClickedButtonConnect()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (_client)
	{
		_client->disconnect();
		delete _client;
		_client = nullptr;
	}
	
	BOOL reconnection = IsDlgButtonChecked(IDC_CHECK_RETRY_CONNECTION);

	CString server_address;
	CString server_port_number;
	_ctrl_address.GetWindowTextW(server_address);
	_ctrl_port_number.GetWindowTextW(server_port_number);
	
	CString keystroke_interval;
	_ctrl_keystroke_interval.GetWindowTextW(keystroke_interval);
	_keystroke_interval = _ttoi(keystroke_interval);

	_client = new client_controller(this);
	_client->connect((LPWSTR)(LPCWSTR)server_address, _ttoi(server_port_number), reconnection);

	EnableConnectButton(FALSE);
}

void CSiriusClientDlg::OnBnClickedButtonDisconnect()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (_client)
	{
		_client->disconnect();
	}

	EnableDisconnectButton(FALSE);
}

bool CSiriusClientDlg::ParseArgument(int argc, wchar_t * argv[])
{
	wchar_t * pargv;
	std::map<std::wstring, std::wstring> param;

	for (int32_t i = 1; i < argc; i++)
	{
		pargv = argv[i];
		if (wcsncmp(pargv, L"--", 2) == 0)
		{
			const wchar_t *p = wcschr(pargv + 2, L'=');
			if (p)
			{
				const wchar_t *f = pargv + 2;
				std::wstring name(f, p);
				std::wstring val(p + 1);
				param.insert(std::make_pair(name, val));
			}
			else
			{
				std::wstring name(pargv + 2);
				std::wstring val;
				val.clear();
				param.insert(std::make_pair(name, val));
			}
		}
		else
		{
			continue;
		}
	}

	std::map<std::wstring, std::wstring>::iterator iter;
	std::wstring value;
	if (param.end() != (iter = param.find(L"address")))
	{
		value = iter->second;
		_ctrl_address.SetWindowTextW(value.c_str());
	}

	if (param.end() != (iter = param.find(L"auto_start")))
	{
		value = iter->second;
		if(wcscmp(L"true",value.c_str()) == 0)
			_auto_start = true;
	}

	return true;
}

void CSiriusClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TIMER_ID_PLAYING_TIME)
	{
		SendMessage(WM_CLOSE);
	}
	CDialogEx::OnTimer(nIDEvent);
}



void CSiriusClientDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (_client)
	{
		CPoint result;
		ClientPointToServerPoint(point, &result);

		if (result.x != -1 || result.y != -1)
		{
			_client->mouse_move(result.x, result.y);
		}
	}


	CDialogEx::OnMouseMove(nFlags, point);
}


BOOL CSiriusClientDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (_client)
	{
		CPoint result;
		ClientPointToServerPoint(pt, &result);

		if (result.x != -1 || result.y != -1)
		{
			_client->mouse_wheel(result.x, result.y, zDelta);
		}
	}

	return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}


void CSiriusClientDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (_client)
	{
		CPoint result;
		ClientPointToServerPoint(point, &result);

		if (result.x != -1 || result.y != -1)
		{
			_client->mouse_lb_double(result.x, result.y);
		}
	}

	CDialogEx::OnLButtonDblClk(nFlags, point);
}


void CSiriusClientDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (_client)
	{
		CPoint result;
		ClientPointToServerPoint(point, &result);

		if (result.x != -1 || result.y != -1)
		{
			_client->mouse_lb_down(result.x, result.y);
		}
	}

	CDialogEx::OnLButtonDown(nFlags, point);
}


void CSiriusClientDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (_client)
	{
		CPoint result;
		ClientPointToServerPoint(point, &result);

		if (result.x != -1 || result.y != -1)
		{
			_client->mouse_lb_up(result.x, result.y);
		}
	}

	CDialogEx::OnLButtonUp(nFlags, point);
}


void CSiriusClientDlg::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (_client)
	{
		CPoint result;
		ClientPointToServerPoint(point, &result);

		if (result.x != -1 || result.y != -1)
		{
			_client->mouse_rb_double(result.x, result.y);
		}
	}

	CDialogEx::OnRButtonDblClk(nFlags, point);
}


void CSiriusClientDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (_client)
	{
		CPoint result;
		ClientPointToServerPoint(point, &result);

		if (result.x != -1 || result.y != -1)
		{
			_client->mouse_rb_down(result.x, result.y);
		}
	}

	CDialogEx::OnRButtonDown(nFlags, point);
}


void CSiriusClientDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (_client)
	{
		CPoint result;
		ClientPointToServerPoint(point, &result);

		if (result.x != -1 || result.y != -1)
		{
			_client->mouse_rb_up(result.x, result.y);
		}
	}

	CDialogEx::OnRButtonUp(nFlags, point);
}

void CSiriusClientDlg::ClientPointToServerPoint(CPoint point, CPoint* output)
{
	RECT clientRC;
	GetDlgItem(IDC_STATIC_VIDEO_VIEW)->GetWindowRect(&clientRC);
	ScreenToClient(&clientRC);

	if (PtInRect(&clientRC, point))
	{
		int width = clientRC.right - clientRC.left;
		int height = clientRC.bottom - clientRC.top;

		float normX = (float)point.x / (float)width;
		float normY = (float)point.y / (float)height;

		int x = normX * 1280;// _width;
		int y = normY * 720;// _height;

		output->SetPoint(x, y);
	}
	else
	{
		output->SetPoint(-1, -1);
	}
}

void CSiriusClientDlg::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	CDialogEx::OnWindowPosChanged(lpwndpos);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	//Invalidate();
}

void CSiriusClientDlg::OnBnClickedFindFileButton()
{
	CFileDialog *dlg = new CFileDialog(TRUE, L"png", NULL, NULL, L"PNG Image Files (*.png)|*.png|");
	INT_PTR nResponse = dlg->DoModal();
	if (nResponse == IDOK)
	{
		CString filename;
		filename = dlg->GetPathName();
		_ctrl_url.SetWindowTextW(filename);
	}
}
