
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
	DDX_Control(pDX, IDC_COMBO_VIDEO_PNG_COMPRESION_LEVEL, _video_png_compression_level);
	DDX_Control(pDX, IDC_COMBO_VIDEO_PNG_QUANTIZATION_COLORS, _video_png_quantization_colors);
	DDX_Control(pDX, IDC_CHECK_TLS, _use_tls);
	DDX_Control(pDX, IDC_CHECK_PRESENT, _enable_present);
	DDX_Control(pDX, IDC_CHECK_AUTOSTART, _enable_auto_start);
	DDX_Control(pDX, IDC_LIST_ATTENDANTS, _attendants);
	DDX_Control(pDX, IDC_CHECK_KEEPALIVE, _enable_keepalive);
	DDX_Control(pDX, IDC_EDIT_STREAMER_PORTNUMBER, _streamer_portnumber);
	DDX_Control(pDX, IDC_EDIT_VIDEO_BUFFER_COUNT, _video_buffer_count);
	DDX_Control(pDX, IDC_EDIT_VIDEO_BLOCK_WIDTH, _video_block_width);
	DDX_Control(pDX, IDC_EDIT_VIDEO_BLOCK_HEIGHT, _video_block_height);
	DDX_Control(pDX, IDC_CHECK_INVALIDATE4CLIENT, _enable_invalidate4client);
	DDX_Control(pDX, IDC_CHECK_INDEXED_MODE, _use_indexed_mode);
	DDX_Control(pDX, IDC_EDIT_KEEPALIVE_TIMEOUT, _keepalive_timeout);
	DDX_Control(pDX, IDC_EDIT_THREAD_COUNT, _nthread);
	DDX_Control(pDX, IDC_CHECK_CLEAN_ATTENDANT, _clean_attendant);
	DDX_Control(pDX, IDC_CHECK_VIDEO_PNG_QUANTIZATION_POSTERIZATION, _video_png_quantization_posterization);
	DDX_Control(pDX, IDC_CHECK_VIDEO_PNG_QUANTIZATION_DITHER_MAP, _video_png_quantization_dither_map);
	DDX_Control(pDX, IDC_CHECK_VIDEO_PNG_QUANTIZATION_CONTRAST_MAPS, _video_png_quantization_contrast_maps);
	DDX_Control(pDX, IDC_EDIT_VIDEO_WEBP_QUALITY, _video_webp_quality);
	DDX_Control(pDX, IDC_COMBO_VIDEO_CODEC, _video_codec);
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

	_about_to_destory = FALSE;

	if (_proxy)
	{
		_proxy->initialize(_proxy_ctx);
		if (!_auto_start)
		{
			_proxy->start();
		}
	}
}


void sirius_warbitrator_dlg::OnBnClickedButtonStop()
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_BUTTON_START)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);

	_about_to_destory = TRUE;

	if (_proxy)
	{
		_proxy->stop();
		_proxy->release();
	}
}


void sirius_warbitrator_dlg::OnBnClickedButtonUpdate()
{
	// TODO: Add your control notification handler code here
	CString wuuid;
	CString wurl;
	CString wattendant_instance;
	CString wattendant_creation_delay;
	CString wcontroller_portnumber;
	CString wstreamer_portnumber;

	CString wvideo_fps;
	CString wvideo_buffer_count;
	CString wvideo_block_width;
	CString wvideo_block_height;

	CString wvideo_webp_quality;
	CString wvideo_webp_method;

	CString wkeepalive_timeout;
	CString wnthread;

	char * uuid = nullptr;
	char * url = nullptr;
	int32_t attendant_instance;
	int32_t attendant_creation_delay = 2000;
	int32_t controller_portnumber;
	int32_t streamer_portnumber;

	int32_t video_codec = sirius::app::server::arbitrator::proxy::video_submedia_type_t::png;

	int32_t video_fps = 0;
	int32_t video_buffer_count = 3;
	int32_t video_block_width = 128;
	int32_t video_block_height = 72;

	int32_t video_png_compression_level = 5;
	bool	video_png_quantization_posterization = true;
	bool	video_png_quantization_dither_map = false;
	bool	video_png_quantization_contrast_maps = false;
	int32_t video_png_quantization_colors = 128;

	float	video_webp_quality = 100.f;
	int32_t video_webp_method = 1;
	
	int32_t keepalive_timeout = 5000;
	int32_t nthread;

	bool invalidate4client = false;
	bool indexed_mode = false;
	bool enable_keepalive = false;
	bool enable_tls = false;
	bool enable_present = false;
	bool enable_auto_start = false;
	bool clean_attendant = false;

	_arbitrator_uuid.GetWindowTextW(wuuid);
	_url.GetWindowTextW(wurl);
	_attendant_instance.GetWindowTextW(wattendant_instance);
	_attendant_creation_delay.GetWindowTextW(wattendant_creation_delay);
	_arbitrator_control_portnumber.GetWindowTextW(wcontroller_portnumber);
	_streamer_portnumber.GetWindowTextW(wstreamer_portnumber);

	_video_fps.GetWindowTextW(wvideo_fps);
	_video_buffer_count.GetWindowTextW(wvideo_buffer_count);
	_video_block_width.GetWindowTextW(wvideo_block_width);
	_video_block_height.GetWindowTextW(wvideo_block_height);

	_video_webp_quality.GetWindowTextW(wvideo_webp_quality);
	_keepalive_timeout.GetWindowTextW(wkeepalive_timeout);
	_nthread.GetWindowTextW(wnthread);

	if (_video_codec.GetCurSel() == 0)
	{
		video_codec = sirius::app::server::arbitrator::proxy::video_submedia_type_t::png;
	}
	else if (_video_codec.GetCurSel() == 1)
	{
		video_codec = sirius::app::server::arbitrator::proxy::video_submedia_type_t::webp;
	}
	else
	{
		video_codec = sirius::app::server::arbitrator::proxy::video_submedia_type_t::png;
	}

	video_png_compression_level = _video_png_compression_level.GetCurSel() + 1;

	if (_video_png_quantization_posterization.GetCheck())
		video_png_quantization_posterization = true;
	else
		video_png_quantization_posterization = false;

	if (_video_png_quantization_dither_map.GetCheck())
		video_png_quantization_dither_map = true;
	else
		video_png_quantization_dither_map = false;

	if (_video_png_quantization_contrast_maps.GetCheck())
		video_png_quantization_contrast_maps = true;
	else
		video_png_quantization_contrast_maps = false;

	if (_video_png_quantization_colors.GetCurSel() == 0)
		video_png_quantization_colors = 0;
	else if (_video_png_quantization_colors.GetCurSel() == 1)
		video_png_quantization_colors = 8;
	else if (_video_png_quantization_colors.GetCurSel() == 2)
		video_png_quantization_colors = 16;
	else if (_video_png_quantization_colors.GetCurSel() == 3)
		video_png_quantization_colors = 32;
	else if (_video_png_quantization_colors.GetCurSel() == 4)
		video_png_quantization_colors = 64;
	else if (_video_png_quantization_colors.GetCurSel() == 5)
		video_png_quantization_colors = 128;
	else if (_video_png_quantization_colors.GetCurSel() == 6)
		video_png_quantization_colors = 256;



	sirius::stringhelper::convert_wide2multibyte((LPTSTR)(LPCTSTR)wuuid, &uuid);
	sirius::stringhelper::convert_wide2multibyte((LPTSTR)(LPCTSTR)wurl, &url);

	attendant_instance = _wtoi(wattendant_instance);
	attendant_creation_delay = _wtoi(wattendant_creation_delay);
	controller_portnumber = _wtoi(wcontroller_portnumber);
	streamer_portnumber = _wtoi(wstreamer_portnumber);
	video_fps = _wtoi(wvideo_fps);
	video_buffer_count = _wtoi(wvideo_buffer_count);
	video_block_width = _wtoi(wvideo_block_width);
	video_block_height = _wtoi(wvideo_block_height);
	video_webp_quality = _wtof(wvideo_webp_quality);
	keepalive_timeout = _wtoi(wkeepalive_timeout);
	nthread = _wtoi(wnthread);

	if (_enable_invalidate4client.GetCheck())
		invalidate4client = true;
	if (_use_indexed_mode.GetCheck())
		indexed_mode = true;
	if (_enable_keepalive.GetCheck())
		enable_keepalive = true;
	if (_use_tls.GetCheck())
		enable_tls = true;
	if (_enable_present.GetCheck())
		enable_present = true;
	if (_enable_auto_start.GetCheck())
		enable_auto_start = true;
	if (_clean_attendant.GetCheck())
		clean_attendant = true;
	update(uuid, url, attendant_instance, attendant_creation_delay, controller_portnumber, streamer_portnumber, video_codec, 1280, 720, video_fps, video_buffer_count, video_block_width, video_block_height, video_png_compression_level, video_png_quantization_posterization, video_png_quantization_dither_map, video_png_quantization_contrast_maps, video_png_quantization_colors, video_webp_quality, video_webp_method, invalidate4client, indexed_mode, nthread, enable_tls, enable_keepalive, keepalive_timeout, enable_present, enable_auto_start, false, clean_attendant, "");

	if (uuid)
		free(uuid);
	if (url)
		free(url);
	uuid = nullptr;
	url = nullptr;
}


void sirius_warbitrator_dlg::on_initialize(const char * uuid, const char * url, int32_t attendant_instance, int32_t attendant_creation_delay, int32_t controller_portnumber, int32_t streamer_portnumber, int32_t video_codec, int32_t video_width, int32_t video_height, int32_t video_fps, int32_t video_buffer_count, int32_t video_block_width, int32_t video_block_height, int32_t video_png_compression_level, bool video_png_quantization_posterization, bool video_png_quantization_dither_map, bool video_png_quantization_contrast_maps, int32_t video_png_quantization_colors, float video_webp_quality, int32_t video_webp_method, bool invalidate4client, bool indexed_mode, int32_t nthread, bool enable_tls, bool enable_keepalive, int32_t keepalive_timeout, bool enable_present, bool enable_auto_start, bool enable_caching, bool clean_attendant, char * cpu, char * memory, const char * app_session_app)
{
	wchar_t * wuuid = nullptr;
	wchar_t * wurl = nullptr;
	wchar_t	wcontroler_portnumber[MAX_PATH] = { 0 };
	wchar_t wstreamer_portnumber[MAX_PATH] = { 0 };
	wchar_t wattendant_instance[MAX_PATH] = { 0 };
	wchar_t wattendant_creation_delay[MAX_PATH] = { 0 };
	wchar_t wvideo_fps[MAX_PATH] = { 0 };
	wchar_t wvideo_buffer_count[MAX_PATH] = { 0 };
	wchar_t wvideo_block_width[MAX_PATH] = { 0 };
	wchar_t wvideo_block_height[MAX_PATH] = { 0 };
	wchar_t wvideo_webp_quality[MAX_PATH] = { 0 };
	wchar_t wkeepalive_timeout[MAX_PATH] = { 0 };
	wchar_t wnthread[MAX_PATH] = { 0 };
	wchar_t * wcpu = nullptr;
	wchar_t * wmemory = nullptr;

	sirius::stringhelper::convert_multibyte2wide((char*)uuid, &wuuid);
	sirius::stringhelper::convert_multibyte2wide((char*)url, &wurl);
	_itow_s(attendant_instance, wattendant_instance, 10);
	_itow_s(attendant_creation_delay, wattendant_creation_delay, 10);
	_itow_s(controller_portnumber, wcontroler_portnumber, 10);
	_itow_s(streamer_portnumber, wstreamer_portnumber, 10);
	_itow_s(video_fps, wvideo_fps, 10);
	_itow_s(video_buffer_count, wvideo_buffer_count, 10);
	_itow_s(video_block_width, wvideo_block_width, 10);
	_itow_s(video_block_height, wvideo_block_height, 10);
	
	
	_snwprintf_s(wvideo_webp_quality, sizeof(wvideo_webp_quality) / sizeof(wchar_t), L"%f", video_webp_quality);

	_itow_s(keepalive_timeout, wkeepalive_timeout, 10);
	_itow_s(nthread, wnthread, 10);

	_label_arbitrator_uuid.SetWindowText(wuuid);
	_label_arbitrator_portnumber.SetWindowTextW(wcontroler_portnumber);

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
	_arbitrator_control_portnumber.SetWindowTextW(wcontroler_portnumber);
	_streamer_portnumber.SetWindowTextW(wstreamer_portnumber);

	if (video_codec == sirius::base::video_submedia_type_t::png)
	{
		_video_codec.SetCurSel(0);
	}
	else if (video_codec == sirius::base::video_submedia_type_t::webp)
	{
		_video_codec.SetCurSel(1);
	}
	else
	{
		_video_codec.SetCurSel(0);
	}

	_video_fps.SetWindowTextW(wvideo_fps);
	_video_buffer_count.SetWindowTextW(wvideo_buffer_count);
	_video_block_width.SetWindowTextW(wvideo_block_width);
	_video_block_height.SetWindowTextW(wvideo_block_height);
	_video_png_compression_level.SetCurSel(video_png_compression_level - 1);

	_video_png_quantization_posterization.SetCheck(video_png_quantization_posterization ? 1 : 0);
	_video_png_quantization_dither_map.SetCheck(video_png_quantization_dither_map ? 1 : 0);
	_video_png_quantization_contrast_maps.SetCheck(video_png_quantization_contrast_maps ? 1 : 0);

	if (video_png_quantization_colors == 0)
		_video_png_quantization_colors.SetCurSel(0);
	else if (video_png_quantization_colors == 8)
		_video_png_quantization_colors.SetCurSel(1);
	else if (video_png_quantization_colors == 16)
		_video_png_quantization_colors.SetCurSel(2);
	else if (video_png_quantization_colors == 32)
		_video_png_quantization_colors.SetCurSel(3);
	else if (video_png_quantization_colors == 64)
		_video_png_quantization_colors.SetCurSel(4);
	else if (video_png_quantization_colors == 128)
		_video_png_quantization_colors.SetCurSel(5);
	else if (video_png_quantization_colors == 256)
		_video_png_quantization_colors.SetCurSel(6);

	_video_webp_quality.SetWindowTextW(wvideo_webp_quality);

	_keepalive_timeout.SetWindowTextW(wkeepalive_timeout);
	_nthread.SetWindowTextW(wnthread);

	_enable_invalidate4client.SetCheck(invalidate4client ? 1 : 0);
	_use_indexed_mode.SetCheck(indexed_mode ? 1 : 0);
	_enable_keepalive.SetCheck(enable_keepalive ? 1 : 0);
	_use_tls.SetCheck(enable_tls ? 1 : 0);
	_enable_present.SetCheck(enable_present ? 1 : 0);
	_enable_auto_start.SetCheck(enable_auto_start ? 1 : 0);
	_clean_attendant.SetCheck(clean_attendant ? 1 : 0);

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
