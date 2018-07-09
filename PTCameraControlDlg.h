// PTCameraControlDlg.h : header file
//

#pragma once

#pragma warning (disable : 4101)

#include "afxwin.h"
#include "CameraInstance.h"
#include "StreamWriter.h"
#include "DisplayImage.h"
#include "crc16.h"
#include "PelcoDController.h"
#include "ImageProcController.h"

#if ( _MSC_VER >= 1600 )
	#pragma comment( lib, "NeptuneClassLib_MD_VC100.lib" )
#elif ( _MSC_VER == 1500 )
	#pragma comment( lib, "NeptuneClassLib_MD_VC90.lib" )	
#elif ( _MSC_VER == 1400 )
	#pragma comment( lib, "NeptuneClassLib_MD_VC80.lib" )	
#endif

using namespace NeptuneClassLib;

// CPTCameraControlDlg dialog
class CPTCameraControlDlg : public CDialog
{
// Construction
public:
	CPTCameraControlDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_FACE_DETECTION_DIALOG };

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
	afx_msg LRESULT OnCommunication(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	static PelcoDController pelcoDController;
	static ImageProcController imageProcController;
	static bool run;
	static bool detectionOn;
	static bool trackingOn;
	static bool cursorTrackingOn;
	int m_iTrackingMethod;
	CComboBox m_cTrackingMethod;

	CComboBox m_cboCamera;
	CComboBox m_cboPixelFormat;
	CComboBox m_cboFrameRate;
	CComboBox m_cboFormat;
	CStatic m_picDisplay;

	CCameraInstance* m_pCamera;
	CStreamWriter* m_pRecord;
	CDisplayImage* m_pDraw;

	_char_t m_strSelectedCam[MAX_STRING_LENGTH];
	_char_t m_strPixelFormat[MAX_STRING_LENGTH];
	_char_t m_strFrameRate[MAX_STRING_LENGTH];
	_char_t m_strSavePath[_MAX_PATH];
	ENeptuneBoolean m_bCapture;
	ENeptuneBoolean m_bStop;
	ENeptuneDevType m_eDevType;
	NEPTUNE_CAM_INFO* m_pCamInfo;
	CWinThread* m_pAcqThread;

	CComboBox	m_cSerialPort;
	int			m_iSerialPort;
	int			m_iBaudRate;
	CComboBox	m_cBaudRate;
	int			m_iDataBit;
	CComboBox	m_cDataBit;
	int			m_iStopBit;
	CComboBox	m_cStopBit;
	int			m_iParity;
	CComboBox	m_cParity;
	CEdit		m_EditPanPos;
	CEdit		m_EditTiltPos;
	CEdit		m_EditPTSpeed;
	CEdit		m_EditZoomPos;
	CEdit		m_EditFocusPos;
	CEdit		m_EditPresetID;
	UINT		m_nPanSetPos;
	UINT		m_nTiltSetPos;
	UINT		m_nPTSpeed;
	UINT		m_nZoomSetPos;
	UINT		m_nFocusSetPos;
	UINT		m_nPresetID;

	CString		strAddress;
	CEdit		m_EditCommunicationReceive;
	CString		m_strCommunicationReceive;
	int			m_iSize;
	int			responseIdx;
	int			bSize;
	BYTE		bPbyte[12];
	int			b_Pflg;
	CString		result;

	BOOL GetFolder(CString *strSelectedFolder, const char *lpszTitle, const HWND hwndOwner, const char *strRootFolder, const char *strStartFolder);
	void InitDlgCtrl(void);
	void UpdateCameraList(void);
	void UpdatePixelFormat(void);
	void UpdateFrameRate(void);
	void ImageCapture(CFrameDataPtr& pData);
	// bool UpdateTriggerInfo(void);
	void onMouse(int event, int x, int y);
	static void onMouseStatic(int event, int x, int y, int flags, void* userdata);
	static UINT AcquisitionThread(void* pParam);
	
	afx_msg void OnCbnSelchangeComboCamera();
	afx_msg void OnCbnSelchangeComboPixelformat();
	afx_msg void OnCbnSelchangeComboFramerate();
	afx_msg void OnBnClickedButtonSetGigeFramerate();
	afx_msg void OnBnClickedButtonPlay();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedButtonShowControl();
	afx_msg void OnBnClickedButtonCaptureDir();
	afx_msg void OnBnClickedButtonCaptureStart();
	afx_msg void OnBnClickedButtonCaptureStop();
	afx_msg void OnClose();

	afx_msg void OnPelcoDDlgEnable(BOOL bEnable, int num);
	afx_msg void OnBnClickedButtonOpen();
	afx_msg CString byIndexComPort(int xPort);
	afx_msg void OnSelchangeComboPort();
	afx_msg DWORD byIndexBaud(int xBaud);
	afx_msg void OnSelchangeComboBaudrate();
	afx_msg BYTE byIndexData(int xData);
	afx_msg void OnSelchangeComboDatabit();
	afx_msg BYTE byIndexStop(int xStop);
	afx_msg BYTE byIndexParity(int xParity);
	afx_msg void OnSelchangeComboStopbit();
	afx_msg void OnSelchangeComboParity();
	afx_msg void OnBnClickedButtonClose();

	afx_msg void OnBnClickedDetection();
	afx_msg void OnBnClickedTracking();
	afx_msg void OnBnClickedCursorTracking();

	afx_msg void PelcoDComm(BYTE byte[12]);

	afx_msg void OnTimer(UINT_PTR nIDEvent);

	afx_msg void OnBnClickedButtonZoompositionmove();
	afx_msg void OnBnClickedButtonLenszeroset();
	afx_msg void OnBnClickedButtonZoomstop();
	afx_msg void OnBnClickedButtonZoomwide();
	afx_msg void OnBnClickedButtonZoomtele();

	afx_msg void OnBnClickedButtonFocuspositionmove();
	afx_msg void OnBnClickedButtonFocuszeroset();
	afx_msg void OnBnClickedButtonFocusstop();
	afx_msg void OnBnClickedButtonFocusfar();
	afx_msg void OnBnClickedButtonFocusnear();

	afx_msg void OnBnClickedButtonPanpositionmove();
	afx_msg void OnBnClickedButtonPansetzero();
	
	afx_msg void OnBnClickedButtonTiltpositionmove();
	afx_msg void OnBnClickedButtonTiltsetzero();
	afx_msg void OnBnClickedButtonPtzfreset();

	afx_msg void Delay(DWORD dwMilsec);

	afx_msg void OnBnClickedButtonPresetgoto();
	afx_msg void OnBnClickedButtonPresetset();
	afx_msg void OnBnClickedButtonPresetclear();
	afx_msg void OnBnClickedButtonPtzfconfirmpos();

	afx_msg void OnBnClickedButtonPanstop();
	afx_msg void OnBnClickedButtonTiltstop();

	afx_msg void OnBnClickedButtonPtstop();
	afx_msg void OnBnClickedButtonPtup();
	afx_msg void OnBnClickedButtonPtdown();
	afx_msg void OnBnClickedButtonPtright();
	afx_msg void OnBnClickedButtonPtleft();
	afx_msg void OnBnClickedButtonPtleftup();
	afx_msg void OnBnClickedButtonPtleftdown();
	afx_msg void OnBnClickedButtonPtrightup();
	afx_msg void OnBnClickedButtonPtrightdown();

	afx_msg void Clear();
};
