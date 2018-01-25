// SiriusStressorKeyMacroDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "SiriusStressor.h"
#include "SiriusStressorDlg.h"
#include "SiriusStressorKeyMacroDlg.h"
#include "afxdialogex.h"

#include <fstream>

#include "sirius_stringhelper.h"
#include "json\json.h"

// CSiriusStressorKeyMacroDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CSiriusStressorKeyMacroDlg, CDialogEx)

CSiriusStressorKeyMacroDlg::CSiriusStressorKeyMacroDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SIRIUS_STRESSOR_KEY_MACRO_DILALOG, pParent)
{

}

CSiriusStressorKeyMacroDlg::~CSiriusStressorKeyMacroDlg()
{
}

void CSiriusStressorKeyMacroDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSiriusStressorKeyMacroDlg, CDialogEx)
END_MESSAGE_MAP()


// CSiriusStressorKeyMacroDlg 메시지 처리기입니다.

BOOL CSiriusStressorKeyMacroDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	load_key_macro();

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

BOOL CSiriusStressorKeyMacroDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)
	{
		int nID = GetFocus()->GetDlgCtrlID();	
		if(set_key_code(nID, pMsg->wParam))
			update_key_macro(nID, pMsg->wParam);
		else
			update_key_macro(nID, -1);

		pMsg->wParam = VK_TAB;	
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

BOOL CSiriusStressorKeyMacroDlg::set_key_code(int nID, int keycode)
{
	switch (keycode)
	{
	case VK_LEFT:
		SetDlgItemText(nID, L"←");
		break;
	case VK_RIGHT:
		SetDlgItemText(nID, L"→");
		break;
	case VK_UP:
		SetDlgItemText(nID, L"↑");
		break;
	case VK_DOWN:
		SetDlgItemText(nID, L"↓");
		break;
	case VK_RETURN:
		SetDlgItemText(nID, L"Enter");
		break;
	case VK_ESCAPE:
		SetDlgItemText(nID, L"Esc");
		break;
	case VK_SPACE:
		SetDlgItemText(nID, L"Space");
		break;
	case VK_F2:
		SetDlgItemText(nID, L"F2");
		break;
	case VK_F3:
		SetDlgItemText(nID, L"F3");
		break;
	case VK_F4:
		SetDlgItemText(nID, L"F4");
		break;
	case VK_F5:
		SetDlgItemText(nID, L"F5");
		break;
	case VK_F6:
		SetDlgItemText(nID, L"F6");
		break;
	case VK_F7:
		SetDlgItemText(nID, L"F7");
		break;
		
	default:
		SetDlgItemText(nID, L"null");	
		return false;
	}	
	return true;
}


void CSiriusStressorKeyMacroDlg::load_key_macro(void)
{
	std::ifstream fin;
	fin.open(L"key_macro.json");
	const int bufferLength = 1024;
	char readBuffer[bufferLength] = { 0, };
	fin.read(readBuffer, bufferLength);
	fin.close();

	std::string config_doc = readBuffer;

	Json::Value root;
	Json::Reader reader;
	if (reader.parse(config_doc, root) == false)
	{
		GetDlgItem(IDC_EDIT_KEY0)->SetWindowTextW(L"null");		
		GetDlgItem(IDC_EDIT_KEY1)->SetWindowTextW(L"null");
		GetDlgItem(IDC_EDIT_KEY2)->SetWindowTextW(L"null");
		GetDlgItem(IDC_EDIT_KEY3)->SetWindowTextW(L"null");
		GetDlgItem(IDC_EDIT_KEY4)->SetWindowTextW(L"null");
		GetDlgItem(IDC_EDIT_KEY5)->SetWindowTextW(L"null");
		GetDlgItem(IDC_EDIT_KEY6)->SetWindowTextW(L"null");
		GetDlgItem(IDC_EDIT_KEY7)->SetWindowTextW(L"null");
		GetDlgItem(IDC_EDIT_KEY8)->SetWindowTextW(L"null");
		GetDlgItem(IDC_EDIT_KEY9)->SetWindowTextW(L"null");
		for (int i = 0; i < KEY_MACRO_COUNT; i++)
		{
			_keys.push_back(-1);
		}
	}
	else
	{
		if (root["key_00"].isInt())
		{
			int value = root["key_00"].asInt();
			set_key_code(IDC_EDIT_KEY0, value);
			_keys.push_back(value);
		}
		if (root["key_01"].isInt())
		{
			int value = root["key_01"].asInt();
			set_key_code(IDC_EDIT_KEY1, value);
			_keys.push_back(value);
		}
		if (root["key_02"].isInt())
		{
			int value = root["key_02"].asInt();
			set_key_code(IDC_EDIT_KEY2, value);
			_keys.push_back(value);
		}
		if (root["key_03"].isInt())
		{
			int value = root["key_03"].asInt();
			set_key_code(IDC_EDIT_KEY3, value);
			_keys.push_back(value);
		}
		if (root["key_04"].isInt())
		{
			int value = root["key_04"].asInt();
			set_key_code(IDC_EDIT_KEY4, value);
			_keys.push_back(value);
		}
		if (root["key_05"].isInt())
		{
			int value = root["key_05"].asInt();
			set_key_code(IDC_EDIT_KEY5, value);
			_keys.push_back(value);
		}
		if (root["key_06"].isInt())
		{
			int value = root["key_06"].asInt();
			set_key_code(IDC_EDIT_KEY6, value);
			_keys.push_back(value);
		}
		if (root["key_07"].isInt())
		{
			int value = root["key_07"].asInt();
			set_key_code(IDC_EDIT_KEY7, value);
			_keys.push_back(value);
		}
		if (root["key_08"].isInt())
		{
			int value = root["key_08"].asInt();
			set_key_code(IDC_EDIT_KEY8, value);
			_keys.push_back(value);
		}
		if (root["key_09"].isInt())
		{
			int value = root["key_09"].asInt();
			set_key_code(IDC_EDIT_KEY9, value);
			_keys.push_back(value);
		}	
	}
}

void CSiriusStressorKeyMacroDlg::update_key_macro(int nID, int keycode)
{
	if (_keys.size() > 0)
	{
		Json::Value root;
		if (nID == IDC_EDIT_KEY0)
			_keys[0] = keycode;

		if (nID == IDC_EDIT_KEY1)
			_keys[1] = keycode;

		if (nID == IDC_EDIT_KEY2)
			_keys[2] = keycode;

		if (nID == IDC_EDIT_KEY3)
			_keys[3] = keycode;

		if (nID == IDC_EDIT_KEY4)
			_keys[4] = keycode;

		if (nID == IDC_EDIT_KEY5)
			_keys[5] = keycode;

		if (nID == IDC_EDIT_KEY6)
			_keys[6] = keycode;

		if (nID == IDC_EDIT_KEY7)
			_keys[7] = keycode;

		if (nID == IDC_EDIT_KEY8)
			_keys[8] = keycode;

		if (nID == IDC_EDIT_KEY9)
			_keys[9] = keycode;

		root["key_00"] = _keys[0];
		root["key_01"] = _keys[1];
		root["key_02"] = _keys[2];
		root["key_03"] = _keys[3];
		root["key_04"] = _keys[4];
		root["key_05"] = _keys[5];
		root["key_06"] = _keys[6];
		root["key_07"] = _keys[7];
		root["key_08"] = _keys[8];
		root["key_09"] = _keys[9];

		Json::StyledWriter writer;
		std::string outputConfig = writer.write(root);

		std::ofstream file(L"key_macro.json", std::ios_base::out | std::ios_base::trunc);
		file.write(outputConfig.c_str(), outputConfig.length());
		file.close();
	}
}
