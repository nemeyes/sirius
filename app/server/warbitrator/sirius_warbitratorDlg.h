
// sirius_warbitratorDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include <sirius_arbitrator_proxy.h>
#include "progress_dlg.h"
// sirius_warbitrator_dlg dialog
class sirius_warbitrator_dlg
	: public CDialogEx
	, public sirius::app::server::arbitrator::proxy::handler
{
// Construction
public:
	sirius_warbitrator_dlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SIRIUS_WARBITRATOR_DIALOG };
#endif

	typedef struct _status_t
	{
		static const int32_t initialized = 0;
		static const int32_t started = 1;
		static const int32_t stopped = 2;
		static const int32_t released = 3;
	} status_t;

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedButtonUuidGenerate();
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedButtonUpdate();

	void initialize_gpus(void);
	void release_gpus(void);

	virtual void on_initialize(const char * uuid, const char * url, int32_t attendant_instance, int32_t attendant_creation_delay, int32_t controller_portnumber, int32_t streamer_portnumber, int32_t video_codec, int32_t video_width, int32_t video_height, int32_t video_fps, int32_t video_buffer_count, int32_t video_block_width, int32_t video_block_height, int32_t video_png_compression_level, bool video_png_quantization_posterization, bool video_png_quantization_dither_map, bool video_png_quantization_contrast_maps, int32_t video_png_quantization_colors, float video_webp_quality, int32_t video_webp_method, bool invalidate4client, bool indexed_mode, int32_t nthread, bool enable_tls, bool enable_keepalive, int32_t keepalive_timeout, bool enable_present, bool enable_auto_start, bool enable_caching, bool clean_attendant, char * cpu, char * memory, const char * app_session_app);
	virtual void on_attendant_create(double percent);
	virtual void on_system_monitor_info(double cpu_usage, double memory_usage);
	virtual void on_start(void);
	virtual void on_stop(void);
	virtual void on_release(void);

private:
	BOOL					_about_to_destory;
	CRITICAL_SECTION		_cs;

	progress_dlg		*	_progress;

	CStatic			_label_arbitrator_uuid;
	CStatic			_label_arbitrator_portnumber;
	CStatic			_label_cpu_name;
	CStatic			_label_memory_info;
	CProgressCtrl	_progress_cpu;
	CProgressCtrl	_progress_memory;
	CEdit			_arbitrator_uuid;
	CEdit			_url;
	CEdit			_attendant_instance;
	CEdit			_attendant_creation_delay;
	CEdit			_arbitrator_control_portnumber;
	CEdit			_streamer_portnumber;
	CEdit			_video_fps;
	CEdit			_video_buffer_count;
	CEdit			_video_block_width;
	CEdit			_video_block_height;

	CComboBox		_video_codec;

	CComboBox		_video_png_compression_level;
	CButton			_video_png_quantization_posterization;
	CButton			_video_png_quantization_dither_map;
	CButton			_video_png_quantization_contrast_maps;
	CComboBox		_video_png_quantization_colors;

	CEdit			_video_webp_quality;

	CButton			_use_indexed_mode;
	CEdit			_nthread;

	CButton			_enable_invalidate4client;
	CButton			_use_tls;
	CButton			_enable_present;
	CButton			_enable_auto_start;
	CListCtrl		_attendants;
	CButton			_enable_keepalive;
	CEdit			_keepalive_timeout;
	bool			_auto_start;
	CButton			_clean_attendant;
	int32_t			_status;

	sirius::app::server::arbitrator::proxy::context_t * _proxy_ctx;
	sirius::app::server::arbitrator::proxy * _proxy;

public:
	afx_msg void OnDestroy();

};
