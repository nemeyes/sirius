#pragma once
#define KEY_MACRO_COUNT 10

// CSiriusStressorKeyMacroDlg 대화 상자입니다.

class CSiriusStressorKeyMacroDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSiriusStressorKeyMacroDlg)

public:
	CSiriusStressorKeyMacroDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CSiriusStressorKeyMacroDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SIRIUS_STRESSOR_KEY_MACRO_DILALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	BOOL set_key_code(int nID, int keycode);

	void load_key_macro(void);
	void update_key_macro(int nID, int keycode);

	std::vector<int> _keys;
};
