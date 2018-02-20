
// sp_comparison_client_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "sp_compare_client.h"
#include "sp_comparison_client_dlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
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

CAboutDlg::CAboutDlg() : CDialog(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// sp_comparison_client_dlg dialog



sp_comparison_client_dlg::sp_comparison_client_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_SP_COMPARE_CLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void sp_comparison_client_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_ARODNAP_ADDRESS, _arodnap_address);
	DDX_Control(pDX, IDC_EDIT_ARODNAP_PORTNUMBER, _arodnap_portnumber);
	DDX_Control(pDX, IDC_EDIT_SIRIUS_ADDRESS, _sirius_address);
	DDX_Control(pDX, IDC_EDIT_SIRIUS_PORTNUMBER, _sirius_portnumber);
}

BEGIN_MESSAGE_MAP(sp_comparison_client_dlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &sp_comparison_client_dlg::OnBnClickedButtonConnect)
	ON_BN_CLICKED(IDC_BUTTON_DISCONNECT, &sp_comparison_client_dlg::OnBnClickedButtonDisconnect)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// sp_comparison_client_dlg message handlers

BOOL sp_comparison_client_dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

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
	//GetDlgItem()
	_arodnap_address.SetWindowTextW(L"10.90.180.25");
	_arodnap_portnumber.SetWindowTextW(L"3390");
	_arodnap_client = new arodnap_client(1024*1024*2);


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void sp_comparison_client_dlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
	if (_arodnap_client)
		delete _arodnap_client;
	_arodnap_client = nullptr;
}

void sp_comparison_client_dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void sp_comparison_client_dlg::OnPaint()
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
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR sp_comparison_client_dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void sp_comparison_client_dlg::OnBnClickedButtonConnect()
{
	// TODO: Add your control notification handler code here
	char *	address = nullptr;
	int		portnumber = 0;
	{
		CString arodnap_address;
		CString arodnap_portnumber;
		_arodnap_address.GetWindowTextW(arodnap_address);
		_arodnap_portnumber.GetWindowTextW(arodnap_portnumber);


		sirius::stringhelper::convert_wide2multibyte((LPWSTR)(LPCWSTR)arodnap_address, &address);
		portnumber = _wtoi(arodnap_portnumber);

		_arodnap_client->start(address, portnumber);

		if (address)
			free(address);
		address = nullptr;
	}
}


void sp_comparison_client_dlg::OnBnClickedButtonDisconnect()
{
	// TODO: Add your control notification handler code here
	{
		_arodnap_client->stop();
	}
}

