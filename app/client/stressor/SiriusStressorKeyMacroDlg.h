#pragma once
#define KEY_MACRO_COUNT 10

// CSiriusStressorKeyMacroDlg ��ȭ �����Դϴ�.

class CSiriusStressorKeyMacroDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSiriusStressorKeyMacroDlg)

public:
	CSiriusStressorKeyMacroDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CSiriusStressorKeyMacroDlg();

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SIRIUS_STRESSOR_KEY_MACRO_DILALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	BOOL set_key_code(int nID, int keycode);

	void load_key_macro(void);
	void update_key_macro(int nID, int keycode);

	std::vector<int> _keys;
};
