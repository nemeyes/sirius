// progress_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "sirius_warbitrator.h"
#include "progress_dlg.h"
#include "afxdialogex.h"


// progress_dlg dialog

IMPLEMENT_DYNAMIC(progress_dlg, CDialogEx)

progress_dlg::progress_dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_PROGRESS, pParent)
{

}

progress_dlg::~progress_dlg()
{
}

void progress_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS_INIT, _progress);
}


BEGIN_MESSAGE_MAP(progress_dlg, CDialogEx)
END_MESSAGE_MAP()


// progress_dlg message handlers
