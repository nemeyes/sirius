
// sirius_warbitratorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "sirius_warbitrator.h"
#include "sirius_warbitratorDlg.h"
#include "afxdialogex.h"

#include <sirius_stringhelper.h>
#include <sirius_uuid.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
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


// sirius_warbitrator_dlg dialog



sirius_warbitrator_dlg::sirius_warbitrator_dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SIRIUS_WARBITRATOR_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void sirius_warbitrator_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_ARBITRATOR_UUID, _label_arbitrator_uuid);
	DDX_Control(pDX, IDC_STATIC_ARBITRATOR_PORTNUMBER, _label_arbitrator_portnumber);
	DDX_Control(pDX, IDC_STATIC_CPU_NAME, _label_cpu_name);
	DDX_Control(pDX, IDC_STATIC_MEMORY_INFO, _label_memory_info);
	DDX_Control(pDX, IDC_PROGRESS_GPU, _progress_cpu);
	DDX_Control(pDX, IDC_PROGRESS_MEMORY, _progress_memory);
	DDX_Control(pDX, IDC_EDIT_ARBITRATOR_UUID, _arbitrator_uuid);
	DDX_Control(pDX, IDC_EDIT_URL, _url);
	DDX_Control(pDX, IDC_EDIT_ATTENDANT_INSTANCE, _attendant_instance);
	DDX_Control(pDX, IDC_EDIT_ATTENDANT_CREATION_DELAY, _attendant_creation_delay);
	DDX_Control(pDX, IDC_EDIT_ARBITRATOR_PORTNUMBER, _arbitrator_control_portnumber);
	DDX_Control(pDX, IDC_EDIT_VIDEO_FPS, _video_fps);
	DDX_Control(pDX, IDC_COMBO_VIDEO_COMPRESION_LEVEL, _video_compression_level);
	DDX_Control(pDX, IDC_COMBO_VIDEO_QUANTIZATION_COLORS, _video_quantization_colors);
	DDX_Control(pDX, IDC_CHECK_TLS, _use_tls);
	DDX_Control(pDX, IDC_CHECK_PRESENT, _enable_present);
	DDX_Control(pDX, IDC_CHECK_AUTOSTART, _enable_auto_start);
	DDX_Control(pDX, IDC_LIST_ATTENDANTS, _attendants);
}

BEGIN_MESSAGE_MAP(sirius_warbitrator_dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_UUID_GENERATE, &sirius_warbitrator_dlg::OnBnClickedButtonUuidGenerate)
	ON_BN_CLICKED(IDC_BUTTON_START, &sirius_warbitrator_dlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &sirius_warbitrator_dlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE, &sirius_warbitrator_dlg::OnBnClickedButtonUpdate)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// sirius_warbitrator_dlg message handlers

BOOL sirius_warbitrator_dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	::InitializeCriticalSection(&_cs);
	initialize_gpus();
	
	GetDlgItem(IDC_BUTTON_START)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);
	_status = status_t::released;

	_progress_cpu.SetRange(0, 100);
	_progress_cpu.SetPos(0);
	_progress_memory.SetRange(0, 100);
	_progress_memory.SetPos(0);

	_proxy_ctx = new sirius::app::server::arbitrator::proxy::context_t();
	_proxy_ctx->handler = this;
	_proxy = new sirius::app::server::arbitrator::proxy();
	set_proxy(_proxy);
	_proxy->initialize(_proxy_ctx);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void sirius_warbitrator_dlg::OnDestroy()
{
	::EnterCriticalSection(&_cs);
	_about_to_destory = TRUE;
	::LeaveCriticalSection(&_cs);

	__super::OnDestroy();

	// TODO: Add your message handler code here
	if (_status == status_t::started)
	{
		_proxy->stop();
	}

	if (_proxy)
	{
		_proxy->release();
		delete _proxy;
		_proxy = nullptr;

		delete _proxy_ctx;
		_proxy_ctx = nullptr;
	}

	if (_progress)
		delete _progress;
	_progress = nullptr;

	::DeleteCriticalSection(&_cs);
}

void sirius_warbitrator_dlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void sirius_warbitrator_dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR sirius_warbitrator_dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void sirius_warbitrator_dlg::OnBnClickedButtonUuidGenerate()
{
	// TODO: Add your control notification handler code here
	sirius::uuid uuidgen;
	uuidgen.create();

	wchar_t * uuid = nullptr;
	sirius::stringhelper::convert_multibyte2wide(uuidgen.c_str(), &uuid);

	_arbitrator_uuid.SetWindowTextW(uuid);

	if (uuid)
		::SysFreeString(uuid);
	uuid = nullptr;
}

void sirius_warbitrator_dlg::OnBnClickedButtonStart()
{
	// TODO: Add your control notification handler code here
	if (_progress)
		delete _progress;
	_progress = new progress_dlg();
	_progress->Create(IDD_DIALOG_PROGRESS, GetActiveWindow());
	_progress->CenterWindow(GetActiveWindow());
	_progress->ShowWindow(SW_SHOW);

	GetDlgItem(IDC_BUTTON_START)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);
	_proxy->start();
}


void sirius_warbitrator_dlg::OnBnClickedButtonStop()
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_BUTTON_START)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);
	_proxy->stop();
}


void sirius_warbitrator_dlg::OnBnClickedButtonUpdate()
{
	// TODO: Add your control notification handler code here
	CString wuuid;
	CString wurl;
	CString wattendant_instance;
	CString wattendant_creation_delay;
	CString wportnumber;

	CString wvideo_fps;
	CString wvideo_quantization_colors;

	char * uuid = nullptr;
	char * url = nullptr;
	int32_t attendant_instance;
	int32_t attendant_creation_delay = 2000;
	int32_t portnumber;
	int32_t video_fps = 0;
	int32_t video_compression_level = 5;
	int32_t video_quantization_colors = 128;

	bool enable_tls = false;
	bool enable_present = false;
	bool enable_auto_start = false;

	_arbitrator_uuid.GetWindowTextW(wuuid);
	_url.GetWindowTextW(wurl);
	_attendant_instance.GetWindowTextW(wattendant_instance);
	_attendant_creation_delay.GetWindowTextW(wattendant_creation_delay);
	_arbitrator_control_portnumber.GetWindowTextW(wportnumber);

	_video_fps.GetWindowTextW(wvideo_fps);
	video_compression_level = _video_compression_level.GetCurSel() + 1;
	if (_video_quantization_colors.GetCurSel() == 0)
		video_quantization_colors = 32;
	else if (_video_quantization_colors.GetCurSel() == 1)
		video_quantization_colors = 64;
	else if (_video_quantization_colors.GetCurSel() == 2)
		video_quantization_colors = 128;
	else if (_video_quantization_colors.GetCurSel() == 3)
		video_quantization_colors = 256;

	sirius::stringhelper::convert_wide2multibyte((LPTSTR)(LPCTSTR)wuuid, &uuid);
	sirius::stringhelper::convert_wide2multibyte((LPTSTR)(LPCTSTR)wurl, &url);

	attendant_instance = _wtoi(wattendant_instance);
	attendant_creation_delay = _wtoi(wattendant_creation_delay);
	portnumber = _wtoi(wportnumber);
	video_fps = _wtoi(wvideo_fps);

	if (_use_tls.GetCheck())
		enable_tls = true;
	if (_enable_present.GetCheck())
		enable_present = true;
	if (_enable_auto_start.GetCheck())
		enable_auto_start = true;
	update(uuid, url, attendant_instance, attendant_creation_delay, portnumber, sirius::app::server::arbitrator::proxy::video_submedia_type_t::png, 1280, 720, video_fps, 128, 72, video_compression_level, video_quantization_colors, enable_tls, false, enable_present, enable_auto_start, true, false, false);

	if (uuid)
		free(uuid);
	if (url)
		free(url);
	uuid = nullptr;
	url = nullptr;
}


void sirius_warbitrator_dlg::on_initialize(const char * uuid, const char * url, int32_t attendant_instance, int32_t attendant_creation_delay, int32_t portnumber, int32_t video_codec, int32_t video_width, int32_t video_height, int32_t video_fps, int32_t video_block_width, int32_t video_block_height, int32_t video_compression_level, int32_t video_quantization_colors, bool enable_tls, bool enable_gpu, bool enable_present, bool enable_auto_start, bool enable_quantization, bool enable_caching, bool enable_crc, char * cpu, char * memory)
{
	wchar_t * wuuid = nullptr;
	wchar_t * wurl = nullptr;
	wchar_t	wportnumber[MAX_PATH] = { 0 };
	wchar_t wattendant_instance[MAX_PATH] = { 0 };
	wchar_t wattendant_creation_delay[MAX_PATH] = { 0 };
	wchar_t wvideo_fps[MAX_PATH] = { 0 };
	wchar_t * wcpu = nullptr;
	wchar_t * wmemory = nullptr;

	sirius::stringhelper::convert_multibyte2wide((char*)uuid, &wuuid);
	sirius::stringhelper::convert_multibyte2wide((char*)url, &wurl);
	_itow_s(attendant_instance, wattendant_instance, 10);
	_itow_s(attendant_creation_delay, wattendant_creation_delay, 10);
	_itow_s(portnumber, wportnumber, 10);
	_itow_s(video_fps, wvideo_fps, 10);

	_label_arbitrator_uuid.SetWindowText(wuuid);
	_label_arbitrator_portnumber.SetWindowTextW(wportnumber);

	sirius::stringhelper::convert_multibyte2wide((char*)cpu, &wcpu);
	_label_cpu_name.SetWindowTextW(wcpu);

	sirius::stringhelper::convert_multibyte2wide((char*)memory, &wmemory);
	wchar_t wmemory2[MAX_PATH] = { 0 };
	_snwprintf_s(wmemory2, sizeof(wmemory2), L"Memory Installed : %s", wmemory);
	_label_memory_info.SetWindowTextW(wmemory2);


	_arbitrator_uuid.SetWindowTextW(wuuid);
	_url.SetWindowTextW(wurl);
	_attendant_instance.SetWindowTextW(wattendant_instance);
	_attendant_creation_delay.SetWindowTextW(wattendant_creation_delay);
	_arbitrator_control_portnumber.SetWindowTextW(wportnumber);

	_video_fps.SetWindowTextW(wvideo_fps);
	_video_compression_level.SetCurSel(video_compression_level - 1);

	if (video_quantization_colors == 32)
		_video_quantization_colors.SetCurSel(0);
	else if (video_quantization_colors == 64)
		_video_quantization_colors.SetCurSel(1);
	else if (video_quantization_colors == 128)
		_video_quantization_colors.SetCurSel(2);
	else if (video_quantization_colors == 256)
		_video_quantization_colors.SetCurSel(3);

	_use_tls.SetCheck(enable_tls ? 1 : 0);
	_enable_present.SetCheck(enable_present ? 1 : 0);
	_enable_auto_start.SetCheck(enable_auto_start ? 1 : 0);

	_auto_start = enable_auto_start;
	_status = sirius_warbitrator_dlg::status_t::initialized;
	if (_auto_start)
	{
		GetDlgItem(IDC_BUTTON_START)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);
		_proxy->start();
	}

	if (wuuid)
		::SysFreeString(wuuid);
	if (wurl)
		::SysFreeString(wurl);
	if (wcpu)
		::SysFreeString(wcpu);
	if (wmemory)
		::SysFreeString(wmemory);

	wuuid = nullptr;
	wurl = nullptr;
	wcpu = nullptr;
	wmemory = nullptr;

	if (_progress)
	{
		delete _progress;
		_progress = nullptr;
	}

	if (attendant_instance > 0)
	{
		_progress = new progress_dlg();
		_progress->Create(IDD_DIALOG_PROGRESS, GetActiveWindow());
		_progress->CenterWindow(GetActiveWindow());
		_progress->ShowWindow(SW_SHOW);
	}
}

void sirius_warbitrator_dlg::on_attendant_create(double percent)
{
	if (_progress)
		_progress->_progress.SetPos(percent);

	if (percent >= 100)
	{
		::Sleep(1000);
		if(_progress)
			_progress->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BUTTON_START)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(TRUE);
	}
}

void sirius_warbitrator_dlg::on_system_monitor_info(double cpu_usage, double memory_usage)
{
	::EnterCriticalSection(&_cs);
	if (!_about_to_destory)
	{
		_progress_cpu.SetPos(cpu_usage);
		_progress_memory.SetPos(memory_usage);
	}
	::LeaveCriticalSection(&_cs);
}

void sirius_warbitrator_dlg::on_start(void)
{
	_status = sirius_warbitrator_dlg::status_t::started;
}

void sirius_warbitrator_dlg::on_stop(void)
{
	_status = sirius_warbitrator_dlg::status_t::stopped;
}

void sirius_warbitrator_dlg::on_release(void)
{
	_status = sirius_warbitrator_dlg::status_t::released;
}

void sirius_warbitrator_dlg::initialize_gpus(void)
{
	int iItem = 0;

	_attendants.InsertColumn(iItem++, _T("No"), LVCFMT_CENTER, 44, 1);
	_attendants.InsertColumn(iItem++, _T("Attendant Id"), LVCFMT_CENTER, 150, 2);
	_attendants.InsertColumn(iItem++, _T("Client Id"), LVCFMT_CENTER, 150, 3);
	_attendants.InsertColumn(iItem++, _T("Status"), LVCFMT_CENTER, 200, 4);
	_attendants.InsertColumn(iItem++, _T("Time"), LVCFMT_CENTER, 100, 5);
	_attendants.InsertColumn(iItem++, _T("PID"), LVCFMT_CENTER, 100, 6);
	_attendants.InsertColumn(iItem++, _T("CPU"), LVCFMT_CENTER, 100, 7);
	_attendants.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES | LVS_EX_DOUBLEBUFFER | LVS_SORTASCENDING);
}

void sirius_warbitrator_dlg::release_gpus(void)
{

}
