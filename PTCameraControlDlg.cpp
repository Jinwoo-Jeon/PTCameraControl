// PTCameraControlDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PTCameraControl.h"
#include "PTCameraControlDlg.h"
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/tracking.hpp>
#include <iostream>
#include <string>

using namespace std;
using namespace cv;

#ifdef _DEBUG

 
//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#define new DEBUG_NEW
#endif

String face_cascade_name = "haarcascade/haarcascade_profileface.xml";
CascadeClassifier face_cascade;  
Ptr<Tracker> tracker;
static Rect2d trackerBB;
static bool selectObject = false;
static bool trackerInit = false;
static bool mouseDown = false;

PelcoDController CPTCameraControlDlg::pelcoDController;
bool CPTCameraControlDlg::run;
bool CPTCameraControlDlg::detectionOn;
bool CPTCameraControlDlg::trackingOn;
bool CPTCameraControlDlg::cursorTrackingOn;

HWND hCommWnd;
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CPTCameraControlDlg dialog




CPTCameraControlDlg::CPTCameraControlDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPTCameraControlDlg::IDD, pParent)
	, m_nPanSetPos(0)
	, m_nTiltSetPos(0)
	, m_nPTSpeed(0)
	, m_nZoomSetPos(0)
	, m_nFocusSetPos(0)
	, m_strCommunicationReceive(_T(""))
	, m_nPresetID(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_pCamera = NULL;
	m_pDraw = NULL;
	m_pRecord = NULL;

	memset( m_strSelectedCam, NULL, MAX_STRING_LENGTH );
	memset( m_strPixelFormat, NULL, MAX_STRING_LENGTH );
	memset( m_strFrameRate, NULL, MAX_STRING_LENGTH );
	memset( m_strSavePath, NULL, _MAX_PATH );
	m_bCapture = NEPTUNE_BOOL_FALSE;
	m_bStop = NEPTUNE_BOOL_TRUE;
	m_eDevType = NEPTUNE_DEV_TYPE_UNKNOWN;
	m_pCamInfo = NULL;
	m_pAcqThread = NULL;

	m_iSerialPort	= 2; // COM3
	m_iBaudRate		= 1; // 9600
	m_iDataBit		= 3; // 8 Bit
	m_iStopBit		= 0; // 1 Bit
	m_iParity		= 0; // None

	strAddress		= "01"; // Pelco Address
	pelcoDController.strAddress = strAddress;
	run = 0;
	detectionOn = 0;
	trackingOn = 0;
	m_iTrackingMethod = 4;
	m_nPanSetPos	= 33756;
	m_nTiltSetPos	= 12559;
	m_nPTSpeed		= 50;
	m_nZoomSetPos	= 149;
	m_nFocusSetPos	= 177;
	m_nPresetID		= 0;

	m_iSize			= 0;
	responseIdx		= 0;
	bSize			= 0;

	memset(bPbyte, NULL, 12);

	b_Pflg			= FALSE;
	result			= "";
}

void CPTCameraControlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_CAMERA, m_cboCamera);
	DDX_Control(pDX, IDC_COMBO_PIXELFORMAT, m_cboPixelFormat);
	DDX_Control(pDX, IDC_COMBO_FRAMERATE, m_cboFrameRate);
	DDX_Control(pDX, IDC_COMBO_CAPTURE_FORMAT, m_cboFormat);
	DDX_Control(pDX, IDC_PICTURE_DISPLAY, m_picDisplay);

	DDX_Control(pDX, IDC_COMBO_TRACKING_METHOD, m_cTrackingMethod);
	DDX_CBIndex(pDX, IDC_COMBO_TRACKING_METHOD, m_iTrackingMethod);
	DDX_Control(pDX, IDC_COMBO_PORT, m_cSerialPort);
	DDX_CBIndex(pDX, IDC_COMBO_PORT, m_iSerialPort);
	DDX_Control(pDX, IDC_COMBO_BAUDRATE, m_cBaudRate);
	DDX_CBIndex(pDX, IDC_COMBO_BAUDRATE, m_iBaudRate);
	DDX_Control(pDX, IDC_COMBO_DATABIT, m_cDataBit);
	DDX_CBIndex(pDX, IDC_COMBO_DATABIT, m_iDataBit);
	DDX_Control(pDX, IDC_COMBO_STOPBIT, m_cStopBit);
	DDX_CBIndex(pDX, IDC_COMBO_STOPBIT, m_iStopBit);
	DDX_Control(pDX, IDC_COMBO_PARITY, m_cParity);
	DDX_CBIndex(pDX, IDC_COMBO_PARITY, m_iParity);
	DDX_Control(pDX, IDC_EDIT_PANPOSITIONPARAM, m_EditPanPos);
	DDX_Text(pDX, IDC_EDIT_PANPOSITIONPARAM, m_nPanSetPos);
	//DDV_MinMaxInt(pDX, m_nPanSetPos, 25896, 40001);
	DDX_Control(pDX, IDC_EDIT_TILTPOSITIONPARAM, m_EditTiltPos);
	DDX_Text(pDX, IDC_EDIT_TILTPOSITIONPARAM, m_nTiltSetPos);
	//DDV_MinMaxInt(pDX, m_nTiltSetPos, 9999, 17287);
	DDX_Control(pDX, IDC_EDIT_PTSPEEDPARAM, m_EditPTSpeed);
	DDX_Text(pDX, IDC_EDIT_PTSPEEDPARAM, m_nPTSpeed);
	//DDV_MinMaxInt(pDX, m_nPTSpeed, 0, 63);
	DDX_Control(pDX, IDC_EDIT_ZOOMPOSITIONPARAM, m_EditZoomPos);
	DDX_Text(pDX, IDC_EDIT_ZOOMPOSITIONPARAM, m_nZoomSetPos);
	//DDV_MinMaxInt(pDX, m_nZoomSetPos, 30, 994);
	DDX_Control(pDX, IDC_EDIT_FOCUSPOSITIONPARAM, m_EditFocusPos);
	DDX_Text(pDX, IDC_EDIT_FOCUSPOSITIONPARAM, m_nFocusSetPos);
	//DDV_MinMaxInt(pDX, m_nFocusSetPos, 24, 992);
	DDX_Control(pDX, IDC_EDIT_COMMUNICATION_RECEIVE, m_EditCommunicationReceive);
	DDX_Text(pDX, IDC_EDIT_COMMUNICATION_RECEIVE, m_strCommunicationReceive);
	DDX_Control(pDX, IDC_EDIT_COMMUNICATION_SEND, pelcoDController.m_EditCommunicationSend);
	DDX_Control(pDX, IDC_EDIT_PTPRESETPARAM, m_EditPresetID);
	DDX_Text(pDX, IDC_EDIT_PTPRESETPARAM, m_nPresetID);
	//DDV_MinMaxInt(pDX, m_nPresetID, 0, 254);
}

BEGIN_MESSAGE_MAP(CPTCameraControlDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_COMM_READ , OnCommunication) //추가
	//}}AFX_MSG_MAP
	ON_CBN_SELCHANGE(IDC_COMBO_CAMERA, &CPTCameraControlDlg::OnCbnSelchangeComboCamera)
	ON_CBN_SELCHANGE(IDC_COMBO_PIXELFORMAT, &CPTCameraControlDlg::OnCbnSelchangeComboPixelformat)
	ON_CBN_SELCHANGE(IDC_COMBO_FRAMERATE, &CPTCameraControlDlg::OnCbnSelchangeComboFramerate)
	ON_BN_CLICKED(IDC_BUTTON_SET_GIGE_FRAMERATE, &CPTCameraControlDlg::OnBnClickedButtonSetGigeFramerate)
	ON_BN_CLICKED(IDC_BUTTON_PLAY, &CPTCameraControlDlg::OnBnClickedButtonPlay)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CPTCameraControlDlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_SHOW_CONTROL, &CPTCameraControlDlg::OnBnClickedButtonShowControl)
	ON_BN_CLICKED(IDC_BUTTON_CAPTURE_DIR, &CPTCameraControlDlg::OnBnClickedButtonCaptureDir)
	ON_BN_CLICKED(IDC_BUTTON_CAPTURE_START, &CPTCameraControlDlg::OnBnClickedButtonCaptureStart)
	ON_BN_CLICKED(IDC_BUTTON_CAPTURE_STOP, &CPTCameraControlDlg::OnBnClickedButtonCaptureStop)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_DETECTION, &CPTCameraControlDlg::OnBnClickedDetection)
	ON_BN_CLICKED(IDC_BUTTON_TRACKING, &CPTCameraControlDlg::OnBnClickedTracking)
	ON_BN_CLICKED(IDC_BUTTON_CURSOR_TRACKING, &CPTCameraControlDlg::OnBnClickedCursorTracking)	
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CPTCameraControlDlg::OnBnClickedButtonOpen)
	ON_CBN_SELCHANGE(IDC_COMBO_PORT, &CPTCameraControlDlg::OnSelchangeComboPort)
	ON_CBN_SELCHANGE(IDC_COMBO_BAUDRATE, &CPTCameraControlDlg::OnSelchangeComboBaudrate)
	ON_CBN_SELCHANGE(IDC_COMBO_DATABIT, &CPTCameraControlDlg::OnSelchangeComboDatabit)
	ON_CBN_SELCHANGE(IDC_COMBO_STOPBIT, &CPTCameraControlDlg::OnSelchangeComboStopbit)
	ON_CBN_SELCHANGE(IDC_COMBO_PARITY, &CPTCameraControlDlg::OnSelchangeComboParity)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CPTCameraControlDlg::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_ZOOMWIDE, &CPTCameraControlDlg::OnBnClickedButtonZoomwide)
	ON_BN_CLICKED(IDC_BUTTON_ZOOMTELE, &CPTCameraControlDlg::OnBnClickedButtonZoomtele)
	ON_BN_CLICKED(IDC_BUTTON_ZOOMPOSITIONMOVE, &CPTCameraControlDlg::OnBnClickedButtonZoompositionmove)
	ON_BN_CLICKED(IDC_BUTTON_ZOOMZEROSET, &CPTCameraControlDlg::OnBnClickedButtonLenszeroset)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_ZOOMSTOP, &CPTCameraControlDlg::OnBnClickedButtonZoomstop)
	ON_BN_CLICKED(IDC_BUTTON_FOCUSPOSITIONMOVE, &CPTCameraControlDlg::OnBnClickedButtonFocuspositionmove)
	ON_BN_CLICKED(IDC_BUTTON_FOCUSZEROSET, &CPTCameraControlDlg::OnBnClickedButtonFocuszeroset)
	ON_BN_CLICKED(IDC_BUTTON_FOCUSSTOP, &CPTCameraControlDlg::OnBnClickedButtonFocusstop)
	ON_BN_CLICKED(IDC_BUTTON_FOCUSFAR, &CPTCameraControlDlg::OnBnClickedButtonFocusfar)
	ON_BN_CLICKED(IDC_BUTTON_FOCUSNEAR, &CPTCameraControlDlg::OnBnClickedButtonFocusnear)
	ON_BN_CLICKED(IDC_BUTTON_PANSTOP, &CPTCameraControlDlg::OnBnClickedButtonPanstop)
	ON_BN_CLICKED(IDC_BUTTON_TILTSTOP, &CPTCameraControlDlg::OnBnClickedButtonTiltstop)
	ON_BN_CLICKED(IDC_BUTTON_PANPOSITIONMOVE, &CPTCameraControlDlg::OnBnClickedButtonPanpositionmove)
	ON_BN_CLICKED(IDC_BUTTON_PANSETZERO, &CPTCameraControlDlg::OnBnClickedButtonPansetzero)
	ON_BN_CLICKED(IDC_BUTTON_TILTPOSITIONMOVE, &CPTCameraControlDlg::OnBnClickedButtonTiltpositionmove)
	ON_BN_CLICKED(IDC_BUTTON_TILTSETZERO, &CPTCameraControlDlg::OnBnClickedButtonTiltsetzero)
	ON_BN_CLICKED(IDC_BUTTON_PTSTOP, &CPTCameraControlDlg::OnBnClickedButtonPtstop)
	ON_BN_CLICKED(IDC_BUTTON_PTUP, &CPTCameraControlDlg::OnBnClickedButtonPtup)
	ON_BN_CLICKED(IDC_BUTTON_PTDOWN, &CPTCameraControlDlg::OnBnClickedButtonPtdown)
	ON_BN_CLICKED(IDC_BUTTON_PTRIGHT, &CPTCameraControlDlg::OnBnClickedButtonPtright)
	ON_BN_CLICKED(IDC_BUTTON_PTLEFT, &CPTCameraControlDlg::OnBnClickedButtonPtleft)
	ON_BN_CLICKED(IDC_BUTTON_PTZFRESET, &CPTCameraControlDlg::OnBnClickedButtonPtzfreset)
	ON_BN_CLICKED(IDC_BUTTON_PRESETGOTO, &CPTCameraControlDlg::OnBnClickedButtonPresetgoto)
	ON_BN_CLICKED(IDC_BUTTON_PRESETSET, &CPTCameraControlDlg::OnBnClickedButtonPresetset)
	ON_BN_CLICKED(IDC_BUTTON_PRESETCLEAR, &CPTCameraControlDlg::OnBnClickedButtonPresetclear)
	ON_BN_CLICKED(IDC_BUTTON_PTZFCONFIRMPOS, &CPTCameraControlDlg::OnBnClickedButtonPtzfconfirmpos)
	ON_BN_CLICKED(IDC_BUTTON_PTLEFTUP, &CPTCameraControlDlg::OnBnClickedButtonPtleftup)
	ON_BN_CLICKED(IDC_BUTTON_PTLEFTDOWN, &CPTCameraControlDlg::OnBnClickedButtonPtleftdown)
	ON_BN_CLICKED(IDC_BUTTON_PTRIGHTUP, &CPTCameraControlDlg::OnBnClickedButtonPtrightup)
	ON_BN_CLICKED(IDC_BUTTON_PTRIGHTDOWN, &CPTCameraControlDlg::OnBnClickedButtonPtrightdown)
END_MESSAGE_MAP()


// CPTCameraControlDlg message handlers

BOOL CPTCameraControlDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
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
	hCommWnd = m_hWnd;

	// CPTCameraControl dialog control initialize.
	InitDlgCtrl();

	// Initialize Neptune class library.
	::InitLibrary();

	// Show the camera list information to combobox control.
	UpdateCameraList();

	GetCurrentDirectory( _MAX_PATH, m_strSavePath ); 
	SetDlgItemText( IDC_EDIT_CAPTURE_PATH, m_strSavePath );

	SetDlgItemText( IDC_STATIC_CAPTURE, "Capture : OFF" );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPTCameraControlDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CPTCameraControlDlg::OnPaint()
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
HCURSOR CPTCameraControlDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	TCHAR szDir[MAX_PATH];	
	CString strTmpPath;
	switch(uMsg)
	{
	case BFFM_INITIALIZED:
		if(lpData)
		{
			strcpy(szDir, strTmpPath.GetBuffer(strTmpPath.GetLength()));
			SendMessage(hwnd, BFFM_SETSELECTION,TRUE, (LPARAM) szDir);
		}
		break;
	case BFFM_SELCHANGED: 
		if(SHGetPathFromIDList((LPITEMIDLIST) lParam, szDir))
			SendMessage(hwnd, BFFM_SETSTATUSTEXT, 0, (LPARAM) szDir);
		break;
	default:
		break;
	}
	return 0;
}

BOOL CPTCameraControlDlg::GetFolder(CString *strSelectedFolder, const char *lpszTitle, const HWND hwndOwner, const char *strRootFolder, const char *strStartFolder)
{
	char pszDisplayName[MAX_PATH];
	CString strTmpPath;

	LPITEMIDLIST lpID;
	BROWSEINFOA bi;
	bi.hwndOwner = hwndOwner;
	if(strStartFolder == NULL)
		bi.pidlRoot = NULL;
	else
	{
		LPITEMIDLIST pIdl = NULL;
		IShellFolder* pDesktopFolder;
		char szPath[MAX_PATH];
		OLECHAR olePath[MAX_PATH];
		ULONG chEaten,dwAttributes;

		strcpy(szPath,(LPCTSTR)strRootFolder);
		if(SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder)))
		{
			MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,szPath,-1,olePath,MAX_PATH);
			pDesktopFolder->ParseDisplayName(NULL,NULL,olePath,&chEaten,&pIdl,&dwAttributes);
			pDesktopFolder->Release();
		}
		bi.pidlRoot = pIdl;
	}
	bi.pszDisplayName = pszDisplayName;
	bi.lpszTitle = lpszTitle;
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT | 64;
	bi.lpfn = BrowseCallbackProc;
	if(strStartFolder == NULL)
		bi.lParam = FALSE;
	else
	{
		strTmpPath.Format("%s",strStartFolder);
		bi.lParam = TRUE;
	}

	bi.iImage = NULL;
	lpID = SHBrowseForFolderA(&bi);
	if(lpID != NULL)
	{
		if(SHGetPathFromIDList(lpID, pszDisplayName))
		{
			strSelectedFolder->Format("%s", pszDisplayName);
			return TRUE;
		}
	}
	else
		strSelectedFolder->Empty();

	return FALSE;
}

void CPTCameraControlDlg::InitDlgCtrl(void)
{
	GetDlgItem( IDC_COMBO_CAMERA )->EnableWindow( TRUE );
	GetDlgItem( IDC_COMBO_PIXELFORMAT )->EnableWindow( FALSE );
	GetDlgItem( IDC_COMBO_FRAMERATE )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_GIGE_FRAMERATE )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTON_SET_GIGE_FRAMERATE )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTON_PLAY )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTON_STOP )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTON_SHOW_CONTROL )->EnableWindow( FALSE );
	GetDlgItem( IDC_COMBO_CAPTURE_FORMAT )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_CAPTURE_PATH )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTON_CAPTURE_DIR )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTON_CAPTURE_START )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTON_CAPTURE_STOP )->EnableWindow( FALSE );

	OnPelcoDDlgEnable(FALSE, 1);
	OnPelcoDDlgEnable(FALSE, 2);
	OnPelcoDDlgEnable(FALSE, 3);
	OnPelcoDDlgEnable(FALSE, 4);
	OnPelcoDDlgEnable(FALSE, 5);

	m_cboCamera.SetCurSel( -1 );
	m_cboPixelFormat.ResetContent();
	m_cboFrameRate.ResetContent();
	m_cboFormat.SetCurSel( -1 );

	CheckDlgButton( IDC_CHECK_TRIGGER_ONOFF, BST_UNCHECKED );

	SetDlgItemText( IDC_EDIT_GIGE_FRAMERATE, _T("") );
	SetDlgItemText( IDC_EDIT_PARAMETER, _T("") );
	SetDlgItemText( IDC_STATIC_PARAMRANGE, _T("(Range)") );

	Invalidate( TRUE );	
}

void CPTCameraControlDlg::UpdateCameraList(void)
{
	_char_t strDevice[MAX_STRING_LENGTH];

	if( m_pCamInfo  )
	{
		delete m_pCamInfo;
		m_pCamInfo = NULL;
	}	
	
	// Get number of cameras connected to the system.
	_uint32_t nCam = CDeviceManager::GetInstance().GetTotalCamera();
	
	if( nCam )
	{
		m_pCamInfo = new NEPTUNE_CAM_INFO[nCam];

		// Get cameras list from connected to the system.
		if( CDeviceManager::GetInstance().GetCameraList( m_pCamInfo, nCam ) == NEPTUNE_ERR_Success )
		{
			m_cboCamera.ResetContent();

			// Add to camera list. 
			for( _uint32_t i=0; i<nCam; i++ )
			{
				// Camera list format is "vendor name:model name:serial number"	
				sprintf_s( strDevice, MAX_STRING_LENGTH, "%s : %s S/N: %s", 
					m_pCamInfo[i].strVendor, m_pCamInfo[i].strModel, m_pCamInfo[i].strSerial );
				m_cboCamera.AddString( strDevice );
			}
		}
	}
}

void CPTCameraControlDlg::UpdatePixelFormat(void)
{
	ENeptuneError eErr;
	_char_t** pPixelFormatList = NULL;
	_uint32_t nCount = 0;
	_uint32_t nLen = 0;

	// Get the count of pixel format list.
	eErr = m_pCamera->GetPixelFormatList( pPixelFormatList, &nCount );

	if( nCount > 0 )
	{
		pPixelFormatList = new _char_t*[nCount];
		for( _uint32_t i=0; i<nCount; i++ )
			pPixelFormatList[i] = new _char_t[MAX_STRING_LENGTH];

		// Get the supported pixel format list of a camera.
		eErr = m_pCamera->GetPixelFormatList( pPixelFormatList, &nCount );
	}
	
	if( eErr != NEPTUNE_ERR_Success )
	{
		AfxMessageBox( _T("Pixel format list get error!") );
		return;
	}
	
	// Get the pixel format from a camera.
	eErr = m_pCamera->GetPixelFormat( m_strPixelFormat, &nLen );
	
	if( eErr != NEPTUNE_ERR_Success )
	{
		AfxMessageBox( _T("Pixel format get error!") );
		return;
	}

	// Add to pixel format list. 
	for( _uint32_t i=0; i<nCount; i++ )
	{
		m_cboPixelFormat.AddString( pPixelFormatList[i] );
		if( !memcmp( pPixelFormatList[i], m_strPixelFormat, strlen( m_strPixelFormat ) ) )
			m_cboPixelFormat.SetCurSel( i );
	}

	if( pPixelFormatList )
	{
		for( _uint32_t i=0; i<nCount; i++ )
			delete pPixelFormatList[i];
		delete pPixelFormatList;
		pPixelFormatList = NULL;
	}

	GetDlgItem( IDC_COMBO_PIXELFORMAT )->EnableWindow( TRUE );
}

void CPTCameraControlDlg::UpdateFrameRate(void)
{
	ENeptuneError eErr;
	_uint32_t nLen = 0;

	if( m_eDevType == NEPTUNE_DEV_TYPE_1394 )  
	{
		_char_t** pFrameRateList = NULL;
		_uint32_t nCount = 0;

		SetDlgItemText( IDC_EDIT_GIGE_FRAMERATE, _T("") );
		GetDlgItem( IDC_EDIT_GIGE_FRAMERATE )->EnableWindow( FALSE );
		GetDlgItem( IDC_BUTTON_SET_GIGE_FRAMERATE )->EnableWindow( FALSE );
		GetDlgItem( IDC_COMBO_FRAMERATE )->EnableWindow( FALSE );

		// Get the count of frame rate list.
		eErr = m_pCamera->GetFrameRateList( pFrameRateList, &nCount );

		if( nCount > 0 )
		{
			pFrameRateList = new _char_t*[nCount];
			for( _uint32_t i=0; i<nCount; i++ )
				pFrameRateList[i] = new _char_t[MAX_STRING_LENGTH];

			// Get the supported frame rate list from a 1394 camera.
			eErr= m_pCamera->GetFrameRateList( pFrameRateList, &nCount );

			if( eErr == NEPTUNE_ERR_Fail )
			{
				AfxMessageBox( _T("1394 frame rate list get error!") );
				return;
			} 
			else
			{
				if( eErr != NEPTUNE_ERR_NotSupport )
				{
					memset( m_strFrameRate, NULL, MAX_STRING_LENGTH	);

					// Get the frame rate from a 1394 camera.
					eErr = m_pCamera->GetFrameRate( m_strFrameRate, &nLen );
					if( eErr != NEPTUNE_ERR_Success )
					{
						AfxMessageBox( _T("1394 farmerate get error!") );
						return;
					}

					// Add to frame rate list.
					for( _uint32_t i=0; i<nCount; i++ )
					{

						m_cboFrameRate.AddString( pFrameRateList[i] );
						if( !memcmp( pFrameRateList[i], m_strFrameRate, strlen( m_strFrameRate ) ) )
							m_cboFrameRate.SetCurSel( i );
					}

					GetDlgItem( IDC_COMBO_FRAMERATE )->EnableWindow( TRUE );
				}
			}

			if( pFrameRateList )
			{
				for( _uint32_t i=0; i<nCount; i++ )
					delete pFrameRateList[i];
				delete pFrameRateList;
				pFrameRateList = NULL;
			}
		}
	}
	else if( m_eDevType == NEPTUNE_DEV_TYPE_GIGE || m_eDevType == NEPTUNE_DEV_TYPE_USB3 )
	{
		memset( m_strFrameRate, NULL, MAX_STRING_LENGTH );
		// Get the frame rate from a GigE camera.
		if( m_pCamera->GetFrameRate( m_strFrameRate, &nLen ) != NEPTUNE_ERR_Success )
		{
			AfxMessageBox( _T("GigE framerate get error!"));
			return;
		}
		SetDlgItemText( IDC_EDIT_GIGE_FRAMERATE, m_strFrameRate );
		GetDlgItem( IDC_EDIT_GIGE_FRAMERATE )->EnableWindow( TRUE );
		GetDlgItem( IDC_BUTTON_SET_GIGE_FRAMERATE )->EnableWindow( TRUE );
		// 1394 Frame rate control disable.
		GetDlgItem( IDC_COMBO_FRAMERATE )->EnableWindow( FALSE );
	}
}

void CPTCameraControlDlg::ImageCapture(CFrameDataPtr& pData)
{
	if ( m_bCapture )
	{
		CString file;
		SYSTEMTIME sysTime;
		GetLocalTime( &sysTime );

		CString cstrFormat;
		m_cboFormat.GetLBText( m_cboFormat.GetCurSel(), cstrFormat );

		// Save to raw data.
		if (cstrFormat == _T("AVI"))
		{
			if (m_pRecord)
			{
				m_pRecord->WriteRawFrame(pData);
			}
		}
		else if ( cstrFormat == _T("RAW") )
		{
			file.Format("%s\\%02d_%02d(%02d_%02d_%02d_%03d).raw", m_strSavePath, sysTime.wMonth, sysTime.wDay, sysTime.wHour,
				sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds );

			CString mode;

			FILE *fp = NULL;
			fopen_s( &fp, file, "wb" );
			if(fp == NULL)
			{
				AfxMessageBox( _T("Unable to open image file.") );
				m_bCapture = NEPTUNE_BOOL_FALSE;
				return;
			}

			fwrite(pData->GetBufferPtr(), pData->GetBufferSize(), 1, fp);
			fclose(fp);
		}
		else		
		{
			// Save image file for extension *.bmp
			if (  cstrFormat == _T("BMP") )
			{
				file.Format( "%s\\%02d_%02d(%02d_%02d_%02d_%03d).bmp", m_strSavePath, sysTime.wMonth, sysTime.wDay, sysTime.wHour,
					sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds );
				m_pCamera->SaveImage( LPSTR( LPCTSTR( file ) ), pData );
			}
			// Save image file for extension *.jpg
			else
			{
				file.Format( "%s\\%02d_%02d(%02d_%02d_%02d_%03d).jpg", m_strSavePath, sysTime.wMonth, sysTime.wDay, sysTime.wHour,
					sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds );
				m_pCamera->SaveImage( LPSTR( LPCTSTR( file ) ), pData );
			}
		}
	}
}

void CPTCameraControlDlg::onMouse(int event, int x, int y)
{	
	switch (event)
	{
	case EVENT_LBUTTONDOWN:
		trackerBB.x = x;
		trackerBB.y = y;
		trackerBB.width = trackerBB.height = 0;
		mouseDown = true;
		break;
	case EVENT_LBUTTONUP:
		trackerBB.width = std::abs(x - trackerBB.x);
		trackerBB.height = std::abs(y - trackerBB.y);
		selectObject = true;
		mouseDown = false;
		if (cursorTrackingOn)
		{
			trackerBB.x = -1;
			trackerBB.y = -1;
		}
		break;
	case EVENT_MOUSEMOVE:
		if (cursorTrackingOn && mouseDown)
		{
			trackerBB.x = x;
			trackerBB.y = y;
		}
		break;
	}
}

void CPTCameraControlDlg::onMouseStatic(int event, int x, int y, int flags, void* userdata)
{
	CPTCameraControlDlg* self = static_cast<CPTCameraControlDlg*>(userdata);
	self->onMouse(event, x, y);
}

double clockToMilliseconds(clock_t ticks) {
	// units/(units/time) => time (seconds) * 1000 = milliseconds
	return (ticks / (double)CLOCKS_PER_SEC)*1000.0;
}

UINT CPTCameraControlDlg::AcquisitionThread(void* pParam)
{
	CPTCameraControlDlg* pDlg = (CPTCameraControlDlg*)pParam;

	CFrameDataPtr pData;

	clock_t beginTime = clock(), endTime = clock(), deltaTime = 0;
	unsigned int frames = 0;
	double frameRate = 0;
	char frameRateChar[6];

	while( !pDlg->m_bStop )
	{
		beginTime = clock();
		if( pDlg->m_pCamera->WaitEventDataStream( pData, 1000 ) != NEPTUNE_ERR_Success )
			continue;

		CString cstrPixelFormat;
		cstrPixelFormat = pDlg->m_strPixelFormat;

		// when capture mode of PTCameraControl's dialog is ON, triggered image is saved.
		pDlg->ImageCapture( pData );

		if (detectionOn || trackingOn || cursorTrackingOn) 
		{
			int downScale = 10;
			Point screenCntrPoint = Point(pData->GetWidth() / downScale / 2, pData->GetHeight() / downScale / 2);
			Point faceCntrPoint = Point(pData->GetWidth() / downScale / 2, pData->GetHeight() / downScale / 2);
			int dist_x = 0;
			int dist_y = 0;
			int dist = 0;
			int vel_x = 0;
			int vel_y = 0;
			int vel = 0;
			int centerMargin = 10;
			Mat m;
			_uint32_t nRGBBuffer = pData->GetWidth() * pData->GetHeight() * 3;			_char_t* pRGBBuffer = new _char_t[nRGBBuffer];
			if (cstrPixelFormat == _T("BayerGR8"))
			{
				m = Mat(pData->GetHeight(), pData->GetWidth(), CV_8UC1, (uchar*)pData->GetBufferPtr());
				cvtColor(m, m, COLOR_BayerGR2RGB);
			}
			else if (cstrPixelFormat == _T("BayerGR12")) 
			{
				m = Mat(pData->GetHeight(), pData->GetWidth(), CV_16UC1, (uchar*)pData->GetBufferPtr());
				cvtColor(m, m, COLOR_BayerGR2RGB);
				m.convertTo(m, CV_8UC3, 1.0/16);
			}
			else if (cstrPixelFormat == _T("YUV411Packed"))
			{				pDlg->m_pCamera->ConvertYUVToRGB24(pRGBBuffer, nRGBBuffer, pData);
				m = Mat(pData->GetHeight(), pData->GetWidth(), CV_8UC3, pRGBBuffer);
			}
			else if (cstrPixelFormat == _T("YUV422Packed"))
			{				pDlg->m_pCamera->ConvertYUVToRGB24(pRGBBuffer, nRGBBuffer, pData);
				m = Mat(pData->GetHeight(), pData->GetWidth(), CV_8UC3, pRGBBuffer);
			}
			else
			{
				m = Mat(pData->GetHeight(), pData->GetWidth(), CV_8U, (uchar*)pData->GetBufferPtr());
			}
			resize(m, m, Size(pData->GetWidth() / downScale, pData->GetHeight() / downScale), 0, 0, INTER_CUBIC);
			Mat m_copy;
			m.copyTo(m_copy);

			if (detectionOn)
			{
				std::vector<Rect> faces;
				face_cascade.detectMultiScale(m, faces, 1.1, 2);
				if (faces.size() > 0)
				{
					faceCntrPoint = Point(faces[0].x + faces[0].width / 2, faces[0].y + faces[0].height / 2);
					rectangle(m_copy, faces[0], Scalar(255, 0, 0), 5);
				}
			}
			else if (trackingOn)
			{
				if (selectObject)
				{
					if (!trackerInit)
					{
						if (!tracker->init(m, trackerBB))
						{
							ATLTRACE("***Could not initialize tracker...***\r\n");
						}
						trackerInit = true;
					}
					else
					{
						tracker->update(m, trackerBB);
						faceCntrPoint = Point(int(trackerBB.x + trackerBB.width / 2), int(trackerBB.y + trackerBB.height / 2));
						rectangle(m_copy, trackerBB, Scalar(255, 0, 0), 5);
					}
				}
			}
			else
			{
				if (!(trackerBB.x == -1 && trackerBB.y == -1))
				{
					faceCntrPoint = Point(int(trackerBB.x), int(trackerBB.y));
				}
			}
			arrowedLine(m_copy, screenCntrPoint, faceCntrPoint, Scalar(0, 200, 0), 2);

			dist_x = faceCntrPoint.x - screenCntrPoint.x;
			dist_y = faceCntrPoint.y - screenCntrPoint.y;
			dist = int(sqrt(dist_x*dist_x + dist_y*dist_y));
			vel = int(abs(dist) * 24 / sqrt(screenCntrPoint.x*screenCntrPoint.x + screenCntrPoint.y*screenCntrPoint.y) + 40);
			if (dist_x > centerMargin)
			{
				if (dist_y > centerMargin)
				{
					pelcoDController.PTMove(PTDir::RIGHTDOWN, vel);
				}
				else if (dist_y < -centerMargin)
				{
					pelcoDController.PTMove(PTDir::RIGHTUP, vel);
				}
				else
				{
					pelcoDController.PTMove(PTDir::RIGHT, vel);
				}
			}
			else if (dist_x < -centerMargin)
			{
				if (dist_y > centerMargin)
				{
					pelcoDController.PTMove(PTDir::LEFTDOWN, vel);
				}
				else if (dist_y < -centerMargin)
				{
					pelcoDController.PTMove(PTDir::LEFTUP, vel);
				}
				else
				{
					pelcoDController.PTMove(PTDir::LEFT, vel);
				}
			}
			else
			{
				if (dist_y > centerMargin)
				{
					pelcoDController.PTMove(PTDir::DOWN, vel);
				}
				else if (dist_y < -centerMargin)
				{
					pelcoDController.PTMove(PTDir::UP, vel);
				}
				else
				{
					pelcoDController.PTMove(PTDir::STOP);
				}
			}
			endTime = clock();
			deltaTime += endTime - beginTime;
			frames++;
			if (clockToMilliseconds(deltaTime)>1000.0) { 
				frameRate = (double)frames*0.5 + frameRate*0.5; //more stable
				frames = 0;
				deltaTime -= CLOCKS_PER_SEC;
			}
			sprintf(frameRateChar, "%.1f", frameRate);
			putText(m_copy, frameRateChar, Point(5, pData->GetHeight() / downScale - 5), FONT_HERSHEY_SIMPLEX, 2.0, Scalar(0, 255, 0), 5);

			namedWindow("Output window", WINDOW_AUTOSIZE);
			imshow("Output window", m_copy);
			delete pRGBBuffer;
			waitKey(1);
			
		}
		else
		{
			if (cstrPixelFormat.Left(5) == _T("Bayer"))
			{
				
				_uint32_t nBufSize = pData->GetWidth() * pData->GetHeight() * 3;
				_uint8_t* pnConvertData = new _uint8_t[nBufSize];
				pDlg->m_pCamera->SetBayer(pnConvertData, nBufSize, pData, (ENeptuneBayerLayout)NEPTUNE_BAYER_GR_BG,
				(ENeptuneBayerMethod)NEPTUNE_BAYER_METHOD_NEAREST, 0);
				pDlg->m_pDraw->DrawConvertImage(pnConvertData, nBufSize, pData->GetWidth(),	pData->GetHeight());
				delete pnConvertData;
			}
			else if (cstrPixelFormat.Left(3) == _T("YUV"))
			{
				pDlg->m_pDraw->DrawRawImage(pData);
			}
			else
			{
				pDlg->m_pDraw->DrawRawImage(pData);
			}
		}
		// The data is removed from a queue.
		pDlg->m_pCamera->QueueBufferDataStream( pData->GetBufferIndex() );
	}
	
	return TRUE;
}

void CPTCameraControlDlg::OnCbnSelchangeComboCamera()
{
	// TODO: Add your control notification handler code here

	// Camera is already opened. first, try to close opened camera. 
	if( m_pCamera != NULL )
	{
		delete m_pCamera;
		m_pCamera = NULL;
	}

	memset( m_strSelectedCam, NULL, MAX_STRING_LENGTH );
	m_cboCamera.GetLBText( m_cboCamera.GetCurSel(), m_strSelectedCam );

	m_cboPixelFormat.ResetContent();
	m_cboFrameRate.ResetContent();
	SetDlgItemText( IDC_EDIT_GIGE_FRAMERATE, _T("") );
	
	_int32_t nIndex = m_cboCamera.GetCurSel();
	// Get INeptuneDevice interface object. (serial base)
	INeptuneDevice* pDevice = CDeviceManager::GetInstance().GetDeviceFromSerial( m_pCamInfo[nIndex].strSerial );
	if( pDevice == NULL )	
		return;
	// Get the camera's interface type.
	m_eDevType = pDevice->GetDeviceType();

	try
	{
		// Create camera object.
		m_pCamera = new CCameraInstance( pDevice );
	}
	catch ( ENeptuneError e )
	{
		AfxMessageBox( _T("Can not select camera!") );
		// Delete camera object.
		delete m_pCamera;
		// PTCameraControl dialog control initialize.
		InitDlgCtrl();
		return;
	}

	// Show the pixel format list information to combobox control.
	UpdatePixelFormat();

	// Show the frame rate information to control.
	UpdateFrameRate();	

	GetDlgItem(IDC_COMBO_CAPTURE_FORMAT)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_CAPTURE_PATH)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_CAPTURE_DIR)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_CAPTURE_START)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_CAPTURE_STOP)->EnableWindow(FALSE);

	// Create image display window.
	if( !m_pDraw )
		m_pDraw = new CDisplayImage( m_picDisplay.m_hWnd );

	// Play / Stop control set.
	GetDlgItem( IDC_BUTTON_PLAY )->EnableWindow( TRUE );
	GetDlgItem( IDC_BUTTON_STOP )->EnableWindow( FALSE );

	// Control dialog enable.
	GetDlgItem( IDC_BUTTON_SHOW_CONTROL )->EnableWindow( TRUE );
}

void CPTCameraControlDlg::OnCbnSelchangeComboPixelformat()
{
	// TODO: Add your control notification handler code here

	ENeptuneBoolean bRestart = NEPTUNE_BOOL_FALSE;
	if( !m_bStop )
	{
		OnBnClickedButtonStop();
		bRestart = NEPTUNE_BOOL_TRUE;
	}
	
	_int32_t nIndex = m_cboPixelFormat.GetCurSel();
	memset( m_strPixelFormat, NULL, MAX_STRING_LENGTH );
	m_cboPixelFormat.GetLBText( nIndex, m_strPixelFormat );
	
	// Set the pixel format to a camera.
	if( m_pCamera->SetPixelFormat( m_strPixelFormat ) != NEPTUNE_ERR_Success )
	{
		AfxMessageBox( _T("pixelformat set error!") );
		return;
	}

	if( m_eDevType == NEPTUNE_DEV_TYPE_1394 )
	{
		CString cstrPixelFormat;
		cstrPixelFormat = m_strPixelFormat;
		// When camera's pixel format is Format7, frame rate is set to fixed value.
		if( cstrPixelFormat.Left( 7 ) == _T("Format7") )
		{
			m_cboFrameRate.ResetContent();
			GetDlgItem( IDC_COMBO_FRAMERATE )->EnableWindow( FALSE );
		}
		else
		{			
			m_cboFrameRate.ResetContent();
			
			// Show the frame rate information to control.
			UpdateFrameRate();

			// Set the frame rate to a 1394 camera.
			if( m_pCamera->SetFrameRate( m_strFrameRate ) != NEPTUNE_ERR_Success )
			{
				AfxMessageBox( _T("1394 framerate set error!") );
				return;
			}
		}
	}

	// Restart to camera acquisition.
	if( bRestart )
		OnBnClickedButtonPlay();
}

void CPTCameraControlDlg::OnCbnSelchangeComboFramerate()
{
	// TODO: Add your control notification handler code here

	// When camera acquisition is starting, 
	ENeptuneBoolean bRestart = NEPTUNE_BOOL_FALSE;
	if( !m_bStop )
	{
		OnBnClickedButtonStop();
		bRestart = NEPTUNE_BOOL_TRUE;
	}

	memset( m_strFrameRate, NULL, MAX_STRING_LENGTH );
	m_cboFrameRate.GetLBText( m_cboFrameRate.GetCurSel(), m_strFrameRate );
	
	// Set the frame rate to a 1394 camera.
	ENeptuneError eErr = m_pCamera->SetFrameRate( m_strFrameRate );
	if( eErr != NEPTUNE_ERR_Success )
	{
		AfxMessageBox( _T("1394 frame rate set error!") );
		return;
	}

	// Restart to camera acquisition.
	if( bRestart )
		OnBnClickedButtonPlay();
}

void CPTCameraControlDlg::OnBnClickedButtonSetGigeFramerate()
{
	// TODO: Add your control notification handler code here

	// When camera acquisition is starting,
	ENeptuneBoolean bRestart = NEPTUNE_BOOL_FALSE;
	if( !m_bStop )
	{
		OnBnClickedButtonStop();
		bRestart = NEPTUNE_BOOL_TRUE;
	}

	GetDlgItemText( IDC_EDIT_GIGE_FRAMERATE, m_strFrameRate, MAX_STRING_LENGTH );
	
	// Set the frame rate to a GigE camera.
	if( m_pCamera->SetFrameRate( m_strFrameRate ) != NEPTUNE_ERR_Success )
	{
		AfxMessageBox( _T("GigE framerate set error!") );
		return;
	}

	// Restart to camera acquisition.
	if( bRestart )
		OnBnClickedButtonPlay();
}

void CPTCameraControlDlg::OnBnClickedButtonPlay()
{
	// TODO: Add your control notification handler code here

	// Set Acquisition start to a camera.
	if( m_pCamera->AcquisitionStart() != NEPTUNE_ERR_Success )
	{
		AfxMessageBox( _T("Acquisition start error!") );
		return;
	}

	m_bStop = NEPTUNE_BOOL_FALSE;
	if (!face_cascade.load(face_cascade_name)) 
	{ 
		ATLTRACE("--(!)Error loading\r\n"); 
	}

	if( !m_pAcqThread )
	{
		// Create to acquisition thread.
		m_pAcqThread = AfxBeginThread( AcquisitionThread, this, 0, 0, CREATE_SUSPENDED, 0 );
		m_pAcqThread->m_bAutoDelete = FALSE;
		m_pAcqThread->ResumeThread();
	}

	GetDlgItem( IDC_BUTTON_PLAY )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTON_STOP )->EnableWindow( TRUE );
	GetDlgItem( IDC_COMBO_CAMERA )->EnableWindow( FALSE );
	if( m_pCamera->GetCameraType() == NEPTUNE_DEV_TYPE_GIGE || m_pCamera->GetCameraType() == NEPTUNE_DEV_TYPE_USB3 )
	{
		GetDlgItem( IDC_EDIT_GIGE_FRAMERATE )->EnableWindow( FALSE );
		GetDlgItem( IDC_BUTTON_SET_GIGE_FRAMERATE )->EnableWindow( FALSE );
	}
}

void CPTCameraControlDlg::OnBnClickedButtonStop()
{
	// TODO: Add your control notification handler code here

	m_bStop = NEPTUNE_BOOL_TRUE;
	
	if( m_pAcqThread )
	{
		DWORD dwExitCode = 0;
		GetExitCodeThread( m_pAcqThread->m_hThread, &dwExitCode );
		if( dwExitCode == STILL_ACTIVE )
		{
			// Wait for AcquisitionThread is exited.
			ATLTRACE(_T("WaitForSingleObject\r\n"));
			WaitForSingleObject( m_pAcqThread->m_hThread, INFINITE );
		}
		ATLTRACE(_T("delete m_pAcqThread\r\n"));
		delete m_pAcqThread;
		m_pAcqThread = NULL;
	}
	
	// Set Acquisition stop to a camera.
	if ( m_pCamera->AcquisitionStop() != NEPTUNE_ERR_Success )
	{
		ATLTRACE( _T("Acquisition stop error!\r\n") );
		return;
	}

	GetDlgItem( IDC_BUTTON_PLAY )->EnableWindow( TRUE );
	GetDlgItem( IDC_BUTTON_STOP )->EnableWindow( FALSE );
	GetDlgItem( IDC_COMBO_CAMERA )->EnableWindow( TRUE );
	if( m_pCamera->GetCameraType() == NEPTUNE_DEV_TYPE_GIGE || m_pCamera->GetCameraType() == NEPTUNE_DEV_TYPE_USB3 )
	{
		GetDlgItem( IDC_EDIT_GIGE_FRAMERATE )->EnableWindow( TRUE );
		GetDlgItem( IDC_BUTTON_SET_GIGE_FRAMERATE )->EnableWindow( TRUE );
	}
}

void CPTCameraControlDlg::OnBnClickedButtonShowControl()
{
	// TODO: Add your control notification handler code here

	// Open Neptune control dialog.
	m_pCamera->ShowControlDialog();
}

void CPTCameraControlDlg::OnBnClickedButtonCaptureDir()
{
	// TODO: Add your control notification handler code here

	CString strCurPath;

	if ( GetFolder( &strCurPath, "select image save folder.", this->m_hWnd, NULL, NULL ) )
	{
		strcpy_s( m_strSavePath, _MAX_PATH, (LPTSTR)(LPCTSTR)strCurPath );
		SetDlgItemText( IDC_EDIT_CAPTURE_PATH, m_strSavePath );
	}
}

void CPTCameraControlDlg::OnBnClickedButtonCaptureStart()
{
	// TODO: Add your control notification handler code here

	if( m_cboFormat.GetCurSel() < 0 )
	{
		AfxMessageBox( _T("Please select the file format first.") );
		return;
	}

	m_bCapture = NEPTUNE_BOOL_TRUE;

	CString cstrFormat;
	m_cboFormat.GetLBText(m_cboFormat.GetCurSel(), cstrFormat);
	if (cstrFormat == _T("AVI") && !m_pRecord)
	{
		SYSTEMTIME sysTime;
		GetLocalTime(&sysTime);

		m_pRecord = new CStreamWriter(m_pCamera);
		_char_t file[MAX_PATH];
		sprintf_s(file, "%s\\%02d_%02d(%02d_%02d_%02d_%03d).avi", m_strSavePath, sysTime.wMonth, sysTime.wDay, sysTime.wHour,
				sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);
		if (m_pRecord->StartStreamCapture(file, NEPTUNE_BOOL_FALSE, NEPTUNE_BOOL_FALSE, 1000) != NEPTUNE_ERR_Success)
		{
			AfxMessageBox(_T("AVI capture initialize error!"));
			delete m_pRecord;
			m_pRecord = NULL;
			return;
		}
	}
	SetDlgItemText( IDC_STATIC_CAPTURE, "Capture : ON" );
	
	GetDlgItem( IDC_COMBO_CAPTURE_FORMAT )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_CAPTURE_PATH )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTON_CAPTURE_DIR )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTON_CAPTURE_START )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTON_CAPTURE_STOP )->EnableWindow( TRUE );
}

void CPTCameraControlDlg::OnBnClickedButtonCaptureStop()
{
	// TODO: Add your control notification handler code here

	m_bCapture = NEPTUNE_BOOL_FALSE;


	CString cstrFormat;
	m_cboFormat.GetLBText(m_cboFormat.GetCurSel(), cstrFormat);
	if (cstrFormat == _T("AVI"))
	{
		if (m_pRecord)
		{
			// Stop AVI stream capture;
			m_pRecord->StopStreamCapture();
			delete m_pRecord;
			m_pRecord = NULL;
		}
	}


	SetDlgItemText( IDC_STATIC_CAPTURE, "Capture : OFF" );

	GetDlgItem( IDC_COMBO_CAPTURE_FORMAT )->EnableWindow( TRUE );
	GetDlgItem( IDC_EDIT_CAPTURE_PATH )->EnableWindow( TRUE );
	GetDlgItem( IDC_BUTTON_CAPTURE_DIR )->EnableWindow( TRUE );
	GetDlgItem( IDC_BUTTON_CAPTURE_START )->EnableWindow( TRUE );
	GetDlgItem( IDC_BUTTON_CAPTURE_STOP )->EnableWindow( FALSE );
}

void CPTCameraControlDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	if( !m_bStop )
		OnBnClickedButtonStop();

	// Delete camera object.
	if( m_pCamera != NULL )
		delete m_pCamera;

	// Delete image window object.
	if( m_pDraw )
		delete m_pDraw;
	if (m_pRecord)
		delete m_pRecord;
	if( m_pCamInfo )
		delete m_pCamInfo;

	// Clear Neptune class library.
	::UninitLibrary();

	CDialog::OnClose();
}

void CPTCameraControlDlg::OnPelcoDDlgEnable(BOOL bEnable, int num)
{
	switch (num)
	{
		case 1:
			GetDlgItem(IDC_BUTTON_PANSETZERO)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_PANPOSITIONMOVE)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_PTUP)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_PTDOWN)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_PTRIGHT)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_PTLEFT)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_PANSTOP)->EnableWindow(bEnable);
			GetDlgItem(IDC_EDIT_PANPOSITIONPARAM)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_PTLEFTUP)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_PTLEFTDOWN)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_PTRIGHTUP)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_PTRIGHTDOWN)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_PTSTOP)->EnableWindow(bEnable);
			GetDlgItem(IDC_EDIT_PTSPEEDPARAM)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_DETECTION)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_TRACKING)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_CURSOR_TRACKING)->EnableWindow(bEnable);
			break;

		case 2:
			GetDlgItem(IDC_BUTTON_TILTSETZERO)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_TILTPOSITIONMOVE)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_PTUP)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_PTDOWN)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_PTRIGHT)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_PTLEFT)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_TILTSTOP)->EnableWindow(bEnable);
			GetDlgItem(IDC_EDIT_TILTPOSITIONPARAM)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_PTLEFTUP)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_PTLEFTDOWN)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_PTRIGHTUP)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_PTRIGHTDOWN)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_PTSTOP)->EnableWindow(bEnable);
			GetDlgItem(IDC_EDIT_PTSPEEDPARAM)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_DETECTION)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_TRACKING)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_CURSOR_TRACKING)->EnableWindow(bEnable);
			break;

		case 3:
			GetDlgItem(IDC_BUTTON_ZOOMZEROSET)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_ZOOMPOSITIONMOVE)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_ZOOMWIDE)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_ZOOMTELE)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_ZOOMSTOP)->EnableWindow(bEnable);
			GetDlgItem(IDC_EDIT_ZOOMPOSITIONPARAM)->EnableWindow(bEnable);
			break;

		case 4:
			GetDlgItem(IDC_BUTTON_FOCUSZEROSET)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_FOCUSPOSITIONMOVE)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_FOCUSFAR)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_FOCUSNEAR)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_FOCUSSTOP)->EnableWindow(bEnable);
			GetDlgItem(IDC_EDIT_FOCUSPOSITIONPARAM)->EnableWindow(bEnable);
			break;

		case 5:
			GetDlgItem(IDC_BUTTON_PRESETGOTO)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_PRESETSET)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_PRESETCLEAR)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_PTZFRESET)->EnableWindow(bEnable);
			GetDlgItem(IDC_BUTTON_PTZFCONFIRMPOS)->EnableWindow(bEnable);
			GetDlgItem(IDC_EDIT_PTPRESETPARAM)->EnableWindow(bEnable);
			break;
	}
}

void CPTCameraControlDlg::OnBnClickedButtonOpen()
{
	UpdateData(TRUE);
	CString PortName;
	PortName.Format("OPEN PORT: %s\r\n", byIndexComPort(m_iSerialPort));
	if (pelcoDController.m_ComuPort.m_bConnected == FALSE)//포트가 닫혀 있을 경우에만 포트를 열기 위해
	{
		if (pelcoDController.m_ComuPort.OpenPort(byIndexComPort(m_iSerialPort), byIndexBaud(m_iBaudRate), byIndexData(m_iDataBit), byIndexStop(m_iStopBit), byIndexParity(m_iParity)) == TRUE)
		{
			OnPelcoDDlgEnable(TRUE, 1);
			OnPelcoDDlgEnable(TRUE, 2);
			OnPelcoDDlgEnable(TRUE, 3);
			OnPelcoDDlgEnable(TRUE, 4);
			OnPelcoDDlgEnable(TRUE, 5);
			
			OnBnClickedButtonPtzfconfirmpos();
		}
	}
	else
	{
		AfxMessageBox("Already Port open");
	}
}

CString CPTCameraControlDlg::byIndexComPort(int xPort)
{
	CString PortName;
	switch (xPort)
	{
	case 0:		PortName = "COM1"; 			break;

	case 1:		PortName = "COM2";			break;

	case 2:		PortName = "COM3"; 			break;

	case 3:		PortName = "COM4";			break;

	case 4:		PortName = "\\\\.\\COM5"; 	break;

	case 5:		PortName = "\\\\.\\COM6";	break;

	case 6:		PortName = "\\\\.\\COM7"; 	break;

	case 7:		PortName = "\\\\.\\COM8";	break;

	case 8:		PortName = "\\\\.\\COM9"; 	break;

	case 9:		PortName = "\\\\.\\COM10";	break;
	}



	return PortName;
}

void CPTCameraControlDlg::OnSelchangeComboPort()
{
	UpdateData(TRUE);
	CString change, str;
	m_iSerialPort = m_cSerialPort.GetCurSel();
	m_cSerialPort.GetLBText(m_iSerialPort, str);
	change.Format("ComPort change: %s \r\n", str);
	//m_EditControl.SetSel(-1, 0);
	//m_EditControl.ReplaceSel(change);
}

DWORD CPTCameraControlDlg::byIndexBaud(int xBaud)
{
	DWORD dwBaud;
	switch (xBaud)
	{
	case 0:		dwBaud = CBR_4800;		break;

	case 1:		dwBaud = CBR_9600;		break;

	case 2:		dwBaud = CBR_14400;		break;

	case 3:		dwBaud = CBR_19200;		break;

	case 4:		dwBaud = CBR_38400;		break;

	case 5:		dwBaud = CBR_56000;		break;

	case 6:		dwBaud = CBR_57600;		break;

	case 7:		dwBaud = CBR_115200;	break;

	case 9:		dwBaud = 921600;		break;
	}

	return dwBaud;
}

void CPTCameraControlDlg::OnSelchangeComboBaudrate()
{
	UpdateData(TRUE);
	CString change, str;
	m_iBaudRate = m_cBaudRate.GetCurSel();
	if (pelcoDController.m_ComuPort.m_bConnected == TRUE)
	{
		pelcoDController.m_ComuPort.ClosePort();
		pelcoDController.m_ComuPort.OpenPort(byIndexComPort(m_iSerialPort), byIndexBaud(m_iBaudRate), byIndexData(m_iDataBit), byIndexStop(m_iStopBit), byIndexParity(m_iParity));
	}
	m_cBaudRate.GetLBText(m_iBaudRate, str);
	change.Format("BaudRate change: %s \r\n", str);
	//m_EditControl.SetSel(-1, 0);
	//m_EditControl.ReplaceSel(change);
}

BYTE CPTCameraControlDlg::byIndexData(int xData)
{
	BYTE byData;
	switch (xData)
	{
	case 0:	byData = 5;			break;

	case 1:	byData = 6;			break;

	case 2:	byData = 7;			break;

	case 3:	byData = 8;			break;
	}
	return byData;
}

void CPTCameraControlDlg::OnSelchangeComboDatabit()
{
	UpdateData(TRUE);
	CString change, str;
	m_iDataBit = m_cDataBit.GetCurSel();
	if (pelcoDController.m_ComuPort.m_bConnected == TRUE)
	{
		pelcoDController.m_ComuPort.ClosePort();
		pelcoDController.m_ComuPort.OpenPort(byIndexComPort(m_iSerialPort), byIndexBaud(m_iBaudRate), byIndexData(m_iDataBit), byIndexStop(m_iStopBit), byIndexParity(m_iParity));
	}
	m_cDataBit.GetLBText(m_iDataBit, str);
	change.Format("DataBit change: %s \r\n", str);
	//m_EditControl.SetSel(-1, 0);
	//m_EditControl.ReplaceSel(change);
}

BYTE CPTCameraControlDlg::byIndexStop(int xStop)
{
	BYTE byStop;
	if (xStop == 0)
	{
		byStop = ONESTOPBIT;
	}
	else
	{
		byStop = TWOSTOPBITS;
	}
	return byStop;
}

void CPTCameraControlDlg::OnSelchangeComboStopbit()
{
	UpdateData(TRUE);
	CString change, str;
	m_iStopBit = m_cStopBit.GetCurSel();
	if (pelcoDController.m_ComuPort.m_bConnected == TRUE)
	{
		pelcoDController.m_ComuPort.ClosePort();
		pelcoDController.m_ComuPort.OpenPort(byIndexComPort(m_iSerialPort), byIndexBaud(m_iBaudRate), byIndexData(m_iDataBit), byIndexStop(m_iStopBit), byIndexParity(m_iParity));
	}
	m_cStopBit.GetLBText(m_iStopBit, str);
	change.Format("StopBit change: %s \r\n", str);
	//m_EditControl.SetSel(-1, 0);
	//m_EditControl.ReplaceSel(change);
}

BYTE CPTCameraControlDlg::byIndexParity(int xParity)
{
	BYTE byParity;
	switch (xParity)
	{
	case 0:	byParity = NOPARITY;	break;

	case 1:	byParity = ODDPARITY;	break;

	case 2:	byParity = EVENPARITY;	break;
	}

	return byParity;
}

void CPTCameraControlDlg::OnSelchangeComboParity()
{
	UpdateData(TRUE);
	CString change, str;
	m_iParity = m_cParity.GetCurSel();
	if (pelcoDController.m_ComuPort.m_bConnected == TRUE)
	{
		pelcoDController.m_ComuPort.ClosePort();
		pelcoDController.m_ComuPort.OpenPort(byIndexComPort(m_iSerialPort), byIndexBaud(m_iBaudRate), byIndexData(m_iDataBit), byIndexStop(m_iStopBit), byIndexParity(m_iParity));
	}
	m_cParity.GetLBText(m_iParity, str);
	change.Format("Parity change: %s \r\n", str);
	//m_EditControl.SetSel(-1, 0);
	//m_EditControl.ReplaceSel(change);
}

void CPTCameraControlDlg::OnBnClickedButtonClose()
{
	UpdateData(TRUE);
	CString PortName;
	if (pelcoDController.m_ComuPort.m_bConnected == TRUE)
	{
		pelcoDController.m_ComuPort.ClosePort();
		PortName.Format("CLOSE PORT: %s \r\n", byIndexComPort(m_iSerialPort));
		//m_EditControl.SetSel(-1, 0);
		//m_EditControl.ReplaceSel(PortName);

		OnPelcoDDlgEnable(FALSE, 1);
		OnPelcoDDlgEnable(FALSE, 2);
		OnPelcoDDlgEnable(FALSE, 3);
		OnPelcoDDlgEnable(FALSE, 4);
		OnPelcoDDlgEnable(FALSE, 5);
	}
	else
	{
		PortName.Format("%s Port not yet open \r\n", byIndexComPort(m_iSerialPort));
		//m_EditControl.SetSel(-1, 0);
		//m_EditControl.ReplaceSel(PortName);

	}
}

LRESULT CPTCameraControlDlg::OnCommunication(WPARAM wParam, LPARAM lParam)
{
	UpdateData(TRUE);//받는 데이터 타입을 알기 위해

	CString str = "";
	BYTE aByte;	 

	int iSize = (pelcoDController.m_ComuPort.m_QueueRead).GetSize();
	for (int i = 0; i < iSize; i++)
	{
		(pelcoDController.m_ComuPort.m_QueueRead).GetByte(&aByte);
		str.Format("%02X ", aByte);
		result += str;
	}
	//ATLTRACE("result " + result + "\r\n");
	int syncByte = result.Find("FF "+ strAddress);
	//ATLTRACE("syncByte %d\r\n", syncByte);
	if (syncByte != -1)
	{
		if ((strlen(result) - syncByte)/3 >= 7)
		{
			result = result.Mid(syncByte, 21);
			//ATLTRACE("result 2 " + result + "\r\n");
			result.Replace(" ", "");
			result.Replace("\r\n", "");
			m_EditCommunicationReceive.SetSel(-1, 0);
			m_EditCommunicationReceive.ReplaceSel("\r\n");
			m_EditCommunicationReceive.SetSel(-1, 0);
			for (unsigned int i = 0; i < strlen(result); i += 2) {
				CString byteString = result.Mid(i, 2);
				m_EditCommunicationReceive.ReplaceSel(byteString + " ");
				bPbyte[i / 2] = HexString2Int(byteString);
			}
			m_EditCommunicationReceive.ReplaceSel("\r\n");
			PelcoDComm(bPbyte);
			result = "";			
		}	
	}
	return 1;
}

void CPTCameraControlDlg::PelcoDComm(BYTE byte[12])
{
	BYTE dByte[12];
	BYTE CByte;
	CString str;
	UINT m_nPanGetPos, m_nTiltGetPos;
	UINT m_nZoomGetPos, m_nFocusGetPos;

	for (int i = 0; i < 12; i++){
		dByte[i] = byte[i];
	}

	if (dByte[0] == 0xFF){
		if (dByte[1] == 0x01){
			if (dByte[3] == 0x59){
				CByte = dByte[1] + dByte[2] + dByte[3] + dByte[4] + dByte[5];
				if (dByte[6] == CByte){
					m_EditPanPos.SetSel(0, -1, TRUE);
					m_EditPanPos.Clear();

					m_nPanGetPos = ((dByte[4] & 0xFFFF) << 8) + (dByte[5] & 0xFFFF);
					str.Format(_T("%d"), m_nPanGetPos);

					m_EditPanPos.SetSel(-1, 0);
					m_EditPanPos.ReplaceSel(str);
					m_EditCommunicationReceive.SetSel(-1, 0);
					m_EditCommunicationReceive.ReplaceSel("Pan Get Pos "+ str + "\r\n");
				}
				else{
					//m_EditCommunication.SetSel(-1, 0);
					//m_EditCommunication.ReplaceSel("Checksum Error\r\n");
					ATLTRACE("Checksum Error\r\n");
					Clear();
				}
			}

			if (dByte[3] == 0x5B){
				CByte = dByte[1] + dByte[2] + dByte[3] + dByte[4] + dByte[5];
				if (dByte[6] == CByte){
					m_EditTiltPos.SetSel(0, -1, TRUE);
					m_EditTiltPos.Clear();

					m_nTiltGetPos = ((dByte[4] & 0xFFFF) << 8) + (dByte[5] & 0xFFFF);
					str.Format(_T("%d"), m_nTiltGetPos);

					m_EditTiltPos.SetSel(-1, 0);
					m_EditTiltPos.ReplaceSel(str);
					m_EditCommunicationReceive.SetSel(-1, 0);
					m_EditCommunicationReceive.ReplaceSel("Tilt Get Pos " + str + "\r\n");
				}
				else{
					ATLTRACE("Checksum Error\r\n");
					//m_EditCommunication.SetSel(-1, 0);
					//m_EditCommunication.ReplaceSel("Checksum Error\r\n");
					Clear();
				}
			}

			if (dByte[3] == 0x5D){
				CByte = dByte[1] + dByte[2] + dByte[3] + dByte[4] + dByte[5];
				if (dByte[6] == CByte){
					m_EditZoomPos.SetSel(0, -1, TRUE);
					m_EditZoomPos.Clear();

					m_nZoomGetPos = ((dByte[4] & 0xFFFF) << 8) + (dByte[5] & 0xFFFF);
					str.Format(_T("%d"), m_nZoomGetPos);

					m_EditZoomPos.SetSel(-1, 0);
					m_EditZoomPos.ReplaceSel(str);
					m_EditCommunicationReceive.SetSel(-1, 0);
					m_EditCommunicationReceive.ReplaceSel("Zoom Get Pos " + str + "\r\n");
				}
				else{
					ATLTRACE("Checksum Error\r\n");
					//m_EditCommunication.SetSel(-1, 0);
					//m_EditCommunication.ReplaceSel("Checksum Error\r\n");
					Clear();
				}
			}

			if (dByte[3] == 0x63){
				CByte = dByte[1] + dByte[2] + dByte[3] + dByte[4] + dByte[5];
				if (dByte[6] == CByte){
					m_EditFocusPos.SetSel(0, -1, TRUE);
					m_EditFocusPos.Clear();

					m_nFocusGetPos = ((dByte[4] & 0xFFFF) << 8) + (dByte[5] & 0xFFFF);
					str.Format(_T("%d"), m_nFocusGetPos);

					m_EditFocusPos.SetSel(-1, 0);
					m_EditFocusPos.ReplaceSel(str);
					m_EditCommunicationReceive.SetSel(-1, 0);
					m_EditCommunicationReceive.ReplaceSel("Focus Get Pos " + str + "\r\n");
				}
				else{
					ATLTRACE("Checksum Error\r\n");
					//m_EditCommunication.SetSel(-1, 0);
					//m_EditCommunication.ReplaceSel("Checksum Error\r\n");
					Clear();
				}
			}
			//else{
			//	AfxMessageBox(_T("Command Error"));
			//}
		}
		else{
			ATLTRACE("Camera Address Error\r\n");
			//m_EditCommunication.SetSel(-1, 0);
			//m_EditCommunication.ReplaceSel("Camera Address Error\r\n");
			Clear();
		}
	}
	else{
		ATLTRACE("Sync Error\r\n");
		//m_EditCommunication.SetSel(-1, 0);
		//m_EditCommunication.ReplaceSel("Sync Error\r\n");
		Clear();
	}

	memset(dByte, NULL, 12);
}

void CPTCameraControlDlg::OnTimer(UINT_PTR nIDEvent)
{
	CDialog::OnTimer(nIDEvent);
}


void CPTCameraControlDlg::OnBnClickedDetection()
{
	if (detectionOn)
	{
		detectionOn = false;
		GetDlgItem(IDC_BUTTON_DETECTION)->SetWindowText("Detection Start");

		GetDlgItem(IDC_BUTTON_TRACKING)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_CURSOR_TRACKING)->EnableWindow(TRUE);
	}
	else
	{
		detectionOn = true;
		GetDlgItem(IDC_BUTTON_DETECTION)->SetWindowText("Detection Stop");

		GetDlgItem(IDC_BUTTON_TRACKING)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_CURSOR_TRACKING)->EnableWindow(FALSE);
	}
}

void CPTCameraControlDlg::OnBnClickedTracking()
{
	if (trackingOn)
	{
		trackingOn = false;
		GetDlgItem(IDC_BUTTON_TRACKING)->SetWindowText("Tracking Start");

		GetDlgItem(IDC_BUTTON_DETECTION)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_CURSOR_TRACKING)->EnableWindow(TRUE);
	}
	else
	{
		trackingOn = true;
		GetDlgItem(IDC_BUTTON_TRACKING)->SetWindowText("Tracking Stop");

		GetDlgItem(IDC_BUTTON_DETECTION)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_CURSOR_TRACKING)->EnableWindow(FALSE);

		trackerInit = false;
		selectObject = false;
		trackerBB = Rect2d(0, 0, 0, 0);

		m_iTrackingMethod = m_cTrackingMethod.GetCurSel();
		//ATLTRACE("%d", m_iTrackingMethod);
		switch (m_iTrackingMethod) {
		case 0:
			//error
			tracker = TrackerBoosting::create();
			break;
		case 1:
			tracker = TrackerCSRT::create();
			break;
		case 2:
			//error
			tracker = TrackerGOTURN::create();
			break;
		case 3:
			tracker = TrackerKCF::create();
			break;
		case 4:
			tracker = TrackerMedianFlow::create();
			break;
		case 5:
			tracker = TrackerMIL::create();
			break;
		case 6:
			tracker = TrackerMOSSE::create();
			break;
		case 7:
			tracker = TrackerTLD::create();
			break;
		}
		if (tracker == NULL)
		{
			ATLTRACE("***Error in the instantiation of the tracker...***\r\n");
		}
		namedWindow("Output window", WINDOW_AUTOSIZE);
		setMouseCallback("Output window", CPTCameraControlDlg::onMouseStatic, (void*)this);
	}
}

void CPTCameraControlDlg::OnBnClickedCursorTracking()
{
	if (cursorTrackingOn)
	{
		cursorTrackingOn = false;
		GetDlgItem(IDC_BUTTON_CURSOR_TRACKING)->SetWindowText("Cursor Tracking Start");

		GetDlgItem(IDC_BUTTON_TRACKING)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_DETECTION)->EnableWindow(TRUE);
		//destroyWindow("Detection window");
	}
	else
	{
		cursorTrackingOn = true;
		GetDlgItem(IDC_BUTTON_CURSOR_TRACKING)->SetWindowText("Cursor Tracking Stop");

		GetDlgItem(IDC_BUTTON_TRACKING)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_DETECTION)->EnableWindow(FALSE);

		trackerBB = Rect2d(-1,-1, 0, 0);

		namedWindow("Output window", WINDOW_AUTOSIZE);
		setMouseCallback("Output window", CPTCameraControlDlg::onMouseStatic, (void*)this);
	}
}

void CPTCameraControlDlg::OnBnClickedButtonZoompositionmove()
{
	UpdateData(TRUE);

	BYTE ch[10] = { 0, };
	CString str0, str2, str3, str4, str5, strCrc1, byGetDataT;

	str0 = "FF";
	ch[0] = HexString2Int(str0);

	ch[1] = HexString2Int(strAddress);

	str2 = "00";
	ch[2] = HexString2Int(str2);

	str3 = "4F";
	ch[3] = HexString2Int(str3);

	str4.Format(_T("%02X"), (m_nZoomSetPos & 0xFF00) >> 8);
	ch[4] = HexString2Int(str4);

	str5.Format(_T("%02X"), m_nZoomSetPos & 0x00FF);
	ch[5] = HexString2Int(str5);

	ch[6] = ch[1] + ch[2] + ch[3] + ch[4] + ch[5];

	strCrc1.Format("%01X", ch[6]);
	byGetDataT = str0 + strAddress + str2 + str3 + str4 + str5 + strCrc1;
	pelcoDController.OnWriteComm(byGetDataT);
}

void CPTCameraControlDlg::OnBnClickedButtonLenszeroset()
{
	UpdateData(TRUE);

	BYTE ch[10] = { 0, };
	CString str0, str2, str3, str4, str5, strCrc1, byGetDataT;

	str0 = "FF";
	ch[0] = HexString2Int(str0);

	ch[1] = HexString2Int(strAddress);

	str2 = "00";
	ch[2] = HexString2Int(str2);

	str3 = "4F";
	ch[3] = HexString2Int(str3);

	str4 = "00";
	ch[4] = HexString2Int(str4);

	str5 = "95";
	ch[5] = HexString2Int(str5);

	ch[6] = ch[1] + ch[2] + ch[3] + ch[4] + ch[5];

	strCrc1.Format("%01X", ch[6]);
	byGetDataT = str0 + strAddress + str2 + str3 + str4 + str5 + strCrc1;
	pelcoDController.OnWriteComm(byGetDataT);
}

void CPTCameraControlDlg::OnBnClickedButtonZoomstop()
{
	pelcoDController.PTMove(PTDir::STOP);
	pelcoDController.PTQueryPosition(PTPos::ZOOM);
}

void CPTCameraControlDlg::OnBnClickedButtonZoomwide()
{
	UpdateData(TRUE);

	BYTE ch[10] = { 0, };
	CString str0, str2, str3, str4, str5, strCrc1, byGetDataT;

	if (run != 0)
	{
		OnPelcoDDlgEnable(TRUE, 3);
		GetDlgItem(IDC_BUTTON_ZOOMWIDE)->SetWindowText("Wide");

		pelcoDController.PTMove(PTDir::STOP);
		pelcoDController.PTQueryPosition(PTPos::ZOOM);
		run = 0;
	}
	else
	{
		OnPelcoDDlgEnable(FALSE, 3);
		GetDlgItem(IDC_BUTTON_ZOOMWIDE)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_ZOOMWIDE)->SetWindowText("Stop");

		str0 = "FF";
		ch[0] = HexString2Int(str0);

		ch[1] = HexString2Int(strAddress);

		str2 = "00";
		ch[2] = HexString2Int(str2);

		str3 = "40";
		ch[3] = HexString2Int(str3);

		str4 = "00";
		ch[4] = HexString2Int(str4);

		str5 = "00";
		ch[5] = HexString2Int(str5);

		ch[6] = ch[1] + ch[2] + ch[3] + ch[4] + ch[5];

		strCrc1.Format("%01X", ch[6]);
		byGetDataT = str0 + strAddress + str2 + str3 + str4 + str5 + strCrc1;
		pelcoDController.OnWriteComm(byGetDataT);
		run = 1;
	}
}

void CPTCameraControlDlg::OnBnClickedButtonZoomtele()
{
	UpdateData(TRUE);

	BYTE ch[10] = { 0, };
	CString str0, str2, str3, str4, str5, strCrc1, byGetDataT;

	if (run != 0)
	{
		OnPelcoDDlgEnable(TRUE, 3);
		GetDlgItem(IDC_BUTTON_ZOOMTELE)->SetWindowText("Tele");

		pelcoDController.PTMove(PTDir::STOP);
		pelcoDController.PTQueryPosition(PTPos::ZOOM);
		run = 0;
	}
	else
	{
		OnPelcoDDlgEnable(FALSE, 3);
		GetDlgItem(IDC_BUTTON_ZOOMTELE)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_ZOOMTELE)->SetWindowText("Stop");

		str0 = "FF";
		ch[0] = HexString2Int(str0);

		ch[1] = HexString2Int(strAddress);

		str2 = "00";
		ch[2] = HexString2Int(str2);

		str3 = "20";
		ch[3] = HexString2Int(str3);

		str4 = "00";
		ch[4] = HexString2Int(str4);

		str5 = "00";
		ch[5] = HexString2Int(str5);

		ch[6] = ch[1] + ch[2] + ch[3] + ch[4] + ch[5];

		strCrc1.Format("%01X", ch[6]);
		byGetDataT = str0 + strAddress + str2 + str3 + str4 + str5 + strCrc1;
		pelcoDController.OnWriteComm(byGetDataT);
		run = 1;
	}
}

void CPTCameraControlDlg::OnBnClickedButtonFocuspositionmove()
{
	UpdateData(TRUE);

	BYTE ch[10] = { 0, };
	CString str0, str2, str3, str4, str5, strCrc1, byGetDataT;

	str0 = "FF";
	ch[0] = HexString2Int(str0);

	ch[1] = HexString2Int(strAddress);

	str2 = "00";
	ch[2] = HexString2Int(str2);

	str3 = "5F";
	ch[3] = HexString2Int(str3);

	str4.Format(_T("%02X"), (m_nFocusSetPos & 0xFF00) >> 8);
	ch[4] = HexString2Int(str4);

	str5.Format(_T("%02X"), m_nFocusSetPos & 0x00FF);
	ch[5] = HexString2Int(str5);

	ch[6] = ch[1] + ch[2] + ch[3] + ch[4] + ch[5];

	strCrc1.Format("%01X", ch[6]);
	byGetDataT = str0 + strAddress + str2 + str3 + str4 + str5 + strCrc1;
	pelcoDController.OnWriteComm(byGetDataT);
}

void CPTCameraControlDlg::OnBnClickedButtonFocuszeroset()
{
	UpdateData(TRUE);

	BYTE ch[10] = { 0, };
	CString str0, str2, str3, str4, str5, strCrc1, byGetDataT;

	str0 = "FF";
	ch[0] = HexString2Int(str0);

	ch[1] = HexString2Int(strAddress);

	str2 = "00";
	ch[2] = HexString2Int(str2);

	str3 = "5F";
	ch[3] = HexString2Int(str3);

	str4 = "00";
	ch[4] = HexString2Int(str4);

	str5 = "B1";
	ch[5] = HexString2Int(str5);

	ch[6] = ch[1] + ch[2] + ch[3] + ch[4] + ch[5];

	strCrc1.Format("%01X", ch[6]);
	byGetDataT = str0 + strAddress + str2 + str3 + str4 + str5 + strCrc1;
	pelcoDController.OnWriteComm(byGetDataT);
}

void CPTCameraControlDlg::OnBnClickedButtonFocusstop()
{
	pelcoDController.PTMove(PTDir::STOP);
	pelcoDController.PTQueryPosition(PTPos::FOCUS);
}

void CPTCameraControlDlg::OnBnClickedButtonFocusfar()
{
	UpdateData(TRUE);

	BYTE ch[10] = { 0, };
	CString str0, str2, str3, str4, str5, strCrc1, byGetDataT;

	if (run != 0)
	{
		OnPelcoDDlgEnable(TRUE, 4);
		GetDlgItem(IDC_BUTTON_FOCUSFAR)->SetWindowText("Far");

		pelcoDController.PTMove(PTDir::STOP);
		pelcoDController.PTQueryPosition(PTPos::FOCUS);
		run = 0;
	}
	else
	{
		OnPelcoDDlgEnable(FALSE, 4);
		GetDlgItem(IDC_BUTTON_FOCUSFAR)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_FOCUSFAR)->SetWindowText("Stop");

		str0 = "FF";
		ch[0] = HexString2Int(str0);

		ch[1] = HexString2Int(strAddress);

		str2 = "00";
		ch[2] = HexString2Int(str2);

		str3 = "80";
		ch[3] = HexString2Int(str3);

		str4 = "00";
		ch[4] = HexString2Int(str4);

		str5 = "00";
		ch[5] = HexString2Int(str5);

		ch[6] = ch[1] + ch[2] + ch[3] + ch[4] + ch[5];

		strCrc1.Format("%01X", ch[6]);
		byGetDataT = str0 + strAddress + str2 + str3 + str4 + str5 + strCrc1;
		pelcoDController.OnWriteComm(byGetDataT);
		run = 1;
	}
}

void CPTCameraControlDlg::OnBnClickedButtonFocusnear()
{
	UpdateData(TRUE);

	BYTE ch[10] = { 0, };
	CString str0, str2, str3, str4, str5, strCrc1, byGetDataT;

	if (run != 0)
	{
		OnPelcoDDlgEnable(TRUE, 4);
		GetDlgItem(IDC_BUTTON_FOCUSNEAR)->SetWindowText("Near");

		pelcoDController.PTMove(PTDir::STOP);
		pelcoDController.PTQueryPosition(PTPos::FOCUS);
		run = 0;
	}
	else
	{
		OnPelcoDDlgEnable(FALSE, 4);
		GetDlgItem(IDC_BUTTON_FOCUSNEAR)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_FOCUSNEAR)->SetWindowText("Stop");

		str0 = "FF";
		ch[0] = HexString2Int(str0);

		ch[1] = HexString2Int(strAddress);

		str2 = "01";
		ch[2] = HexString2Int(str2);

		str3 = "00";
		ch[3] = HexString2Int(str3);

		str4 = "00";
		ch[4] = HexString2Int(str4);

		str5 = "00";
		ch[5] = HexString2Int(str5);

		ch[6] = ch[1] + ch[2] + ch[3] + ch[4] + ch[5];

		strCrc1.Format("%01X", ch[6]);
		byGetDataT = str0 + strAddress + str2 + str3 + str4 + str5 + strCrc1;
		pelcoDController.OnWriteComm(byGetDataT);
		run = 1;
	}
}

void CPTCameraControlDlg::OnBnClickedButtonPanpositionmove()
{
	UpdateData(TRUE);

	BYTE ch[10] = { 0, };
	CString str0, str2, str3, str4, str5, strCrc1, byGetDataT;

	str0 = "FF";
	ch[0] = HexString2Int(str0);

	ch[1] = HexString2Int(strAddress);

	str2 = "00";
	ch[2] = HexString2Int(str2);

	str3 = "4B";
	ch[3] = HexString2Int(str3);

	str4.Format(_T("%02X"), (m_nPanSetPos & 0xFF00) >> 8);
	ch[4] = HexString2Int(str4);

	str5.Format(_T("%02X"), m_nPanSetPos & 0x00FF);
	ch[5] = HexString2Int(str5);

	ch[6] = ch[1] + ch[2] + ch[3] + ch[4] + ch[5];

	strCrc1.Format("%01X", ch[6]);
	byGetDataT = str0 + strAddress + str2 + str3 + str4 + str5 + strCrc1;
	pelcoDController.OnWriteComm(byGetDataT);
}

void CPTCameraControlDlg::OnBnClickedButtonPansetzero()
{
	UpdateData(TRUE);

	BYTE ch[10] = { 0, };
	CString str0, str2, str3, str4, str5, strCrc1, byGetDataT;

	str0 = "FF";
	ch[0] = HexString2Int(str0);

	ch[1] = HexString2Int(strAddress);

	str2 = "00";
	ch[2] = HexString2Int(str2);

	str3 = "4B";
	ch[3] = HexString2Int(str3);

	str4 = "83";
	ch[4] = HexString2Int(str4);

	str5 = "DC";
	ch[5] = HexString2Int(str5);

	ch[6] = ch[1] + ch[2] + ch[3] + ch[4] + ch[5];

	strCrc1.Format("%01X", ch[6]);
	byGetDataT = str0 + strAddress + str2 + str3 + str4 + str5 + strCrc1;
	pelcoDController.OnWriteComm(byGetDataT);
}

void CPTCameraControlDlg::OnBnClickedButtonTiltpositionmove()
{
	UpdateData(TRUE);

	BYTE ch[10] = { 0, };
	CString str0, str2, str3, str4, str5, strCrc1, byGetDataT;

	str0 = "FF";
	ch[0] = HexString2Int(str0);

	ch[1] = HexString2Int(strAddress);

	str2 = "00";
	ch[2] = HexString2Int(str2);

	str3 = "4D";
	ch[3] = HexString2Int(str3);

	str4.Format(_T("%02X"), (m_nTiltSetPos & 0xFF00) >> 8);
	ch[4] = HexString2Int(str4);

	str5.Format(_T("%02X"), m_nTiltSetPos & 0x00FF);
	ch[5] = HexString2Int(str5);

	ch[6] = ch[1] + ch[2] + ch[3] + ch[4] + ch[5];

	strCrc1.Format("%01X", ch[6]);
	byGetDataT = str0 + strAddress + str2 + str3 + str4 + str5 + strCrc1;
	pelcoDController.OnWriteComm(byGetDataT);
}

void CPTCameraControlDlg::OnBnClickedButtonTiltsetzero()
{
	UpdateData(TRUE);

	BYTE ch[10] = { 0, };
	CString str0, str2, str3, str4, str5, strCrc1, byGetDataT;

	str0 = "FF";
	ch[0] = HexString2Int(str0);

	ch[1] = HexString2Int(strAddress);

	str2 = "00";
	ch[2] = HexString2Int(str2);

	str3 = "4D";
	ch[3] = HexString2Int(str3);

	str4 = "31";
	ch[4] = HexString2Int(str4);

	str5 = "0F";
	ch[5] = HexString2Int(str5);

	ch[6] = ch[1] + ch[2] + ch[3] + ch[4] + ch[5];

	strCrc1.Format("%01X", ch[6]);
	byGetDataT = str0 + strAddress + str2 + str3 + str4 + str5 + strCrc1;
	pelcoDController.OnWriteComm(byGetDataT);
}

void CPTCameraControlDlg::OnBnClickedButtonPtzfreset()
{
	UpdateData(TRUE);

	BYTE ch[10] = { 0, };
	CString str0, str2, str3, str4, str5, strCrc1, byGetDataT;

	str0 = "FF";
	ch[0] = HexString2Int(str0);

	ch[1] = HexString2Int(strAddress);

	str2 = "00";
	ch[2] = HexString2Int(str2);

	str3 = "0F";
	ch[3] = HexString2Int(str3);

	str4 = "00";
	ch[4] = HexString2Int(str4);

	str5 = "00";
	ch[5] = HexString2Int(str5);

	ch[6] = ch[1] + ch[2] + ch[3] + ch[4] + ch[5];

	strCrc1.Format("%01X", ch[6]);
	byGetDataT = str0 + strAddress + str2 + str3 + str4 + str5 + strCrc1;
	pelcoDController.OnWriteComm(byGetDataT);
}

void CPTCameraControlDlg::Delay(DWORD dwMillisecond)
{
	MSG msg;
	DWORD dwStart;
	dwStart = GetTickCount();

	while (GetTickCount() - dwStart < dwMillisecond)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

void CPTCameraControlDlg::OnBnClickedButtonPresetgoto()
{
	UpdateData(TRUE);

	BYTE ch[10] = { 0, };
	CString str0, str2, str3, str4, str5, strCrc1, byGetDataT;

	str0 = "FF";
	ch[0] = HexString2Int(str0);

	ch[1] = HexString2Int(strAddress);

	str2 = "00";
	ch[2] = HexString2Int(str2);

	str3 = "07";
	ch[3] = HexString2Int(str3);

	str4 = "00";
	ch[4] = HexString2Int(str4);

	str5.Format(_T("%02X"), m_nPresetID & 0x00FF);
	ch[5] = HexString2Int(str5);

	ch[6] = ch[1] + ch[2] + ch[3] + ch[4] + ch[5];

	strCrc1.Format("%01X", ch[6]);
	byGetDataT = str0 + strAddress + str2 + str3 + str4 + str5 + strCrc1;
	pelcoDController.OnWriteComm(byGetDataT);
}

void CPTCameraControlDlg::OnBnClickedButtonPresetset()
{
	UpdateData(TRUE);

	BYTE ch[10] = { 0, };
	CString str0, str2, str3, str4, str5, strCrc1, byGetDataT;

	str0 = "FF";
	ch[0] = HexString2Int(str0);

	ch[1] = HexString2Int(strAddress);

	str2 = "00";
	ch[2] = HexString2Int(str2);

	str3 = "03";
	ch[3] = HexString2Int(str3);

	str4 = "00";
	ch[4] = HexString2Int(str4);

	str5.Format(_T("%02X"), m_nPresetID & 0x00FF);
	ch[5] = HexString2Int(str5);

	ch[6] = ch[1] + ch[2] + ch[3] + ch[4] + ch[5];

	strCrc1.Format("%01X", ch[6]);
	byGetDataT = str0 + strAddress + str2 + str3 + str4 + str5 + strCrc1;
	pelcoDController.OnWriteComm(byGetDataT);
}

void CPTCameraControlDlg::OnBnClickedButtonPresetclear()
{
	UpdateData(TRUE);

	BYTE ch[10] = { 0, };
	CString str0, str2, str3, str4, str5, strCrc1, byGetDataT;

	str0 = "FF";
	ch[0] = HexString2Int(str0);

	ch[1] = HexString2Int(strAddress);

	str2 = "00";
	ch[2] = HexString2Int(str2);

	str3 = "05";
	ch[3] = HexString2Int(str3);

	str4 = "00";
	ch[4] = HexString2Int(str4);

	str5.Format(_T("%02X"), m_nPresetID & 0x00FF);
	ch[5] = HexString2Int(str5);

	ch[6] = ch[1] + ch[2] + ch[3] + ch[4] + ch[5];

	strCrc1.Format("%01X", ch[6]);
	byGetDataT = str0 + strAddress + str2 + str3 + str4 + str5 + strCrc1;
	pelcoDController.OnWriteComm(byGetDataT);
}

void CPTCameraControlDlg::OnBnClickedButtonPtzfconfirmpos()
{
	pelcoDController.PTQueryPosition(PTPos::PAN);

	Delay(100);

	pelcoDController.PTQueryPosition(PTPos::TILT);

	Delay(100);

	pelcoDController.PTQueryPosition(PTPos::ZOOM);

	Delay(100);

	pelcoDController.PTQueryPosition(PTPos::FOCUS);
}

void CPTCameraControlDlg::OnBnClickedButtonPanstop()
{
	pelcoDController.PTMove(PTDir::STOP);
	pelcoDController.PTQueryPosition(PTPos::PAN);
}

void CPTCameraControlDlg::OnBnClickedButtonTiltstop()
{
	pelcoDController.PTMove(PTDir::STOP);
	pelcoDController.PTQueryPosition(PTPos::TILT);
}
void CPTCameraControlDlg::OnBnClickedButtonPtstop()
{
	//ATLTRACE("-------------PTSTOP-------------\r\n");
	pelcoDController.PTMove(PTDir::STOP);
	pelcoDController.PTQueryPosition(PTPos::PAN);
	//Delay(100);
	pelcoDController.PTQueryPosition(PTPos::TILT);
	run = 0;
}
void CPTCameraControlDlg::OnBnClickedButtonPtup()
{
	UpdateData(TRUE);
	pelcoDController.PTMove(PTDir::UP, m_nPTSpeed);
	run = 1;
}

void CPTCameraControlDlg::OnBnClickedButtonPtdown()
{
	UpdateData(TRUE);
	pelcoDController.PTMove(PTDir::DOWN, m_nPTSpeed);
	run = 1;
}

void CPTCameraControlDlg::OnBnClickedButtonPtright()
{
	UpdateData(TRUE);
	pelcoDController.PTMove(PTDir::RIGHT, m_nPTSpeed);
	run = 1;
}

void CPTCameraControlDlg::OnBnClickedButtonPtleft()
{
	UpdateData(TRUE);
	pelcoDController.PTMove(PTDir::LEFT, m_nPTSpeed);
	run = 1;
}

void CPTCameraControlDlg::OnBnClickedButtonPtleftup()
{
	UpdateData(TRUE);
	pelcoDController.PTMove(PTDir::LEFTUP, m_nPTSpeed);
	run = 1;
}

void CPTCameraControlDlg::OnBnClickedButtonPtleftdown()
{
	UpdateData(TRUE);
	pelcoDController.PTMove(PTDir::LEFTDOWN, m_nPTSpeed);
	run = 1;
}

void CPTCameraControlDlg::OnBnClickedButtonPtrightup()
{
	UpdateData(TRUE);
	pelcoDController.PTMove(PTDir::RIGHTUP, m_nPTSpeed);
	run = 1;
}

void CPTCameraControlDlg::OnBnClickedButtonPtrightdown()
{
	UpdateData(TRUE);
	pelcoDController.PTMove(PTDir::RIGHTDOWN, m_nPTSpeed);
	run = 1;
}

void CPTCameraControlDlg::Clear()
{
	ATLTRACE("Clear");
	memset(bPbyte, NULL, 12);
	(pelcoDController.m_ComuPort.m_QueueRead).Clear();
	result = "";
	b_Pflg = FALSE;
	responseIdx = 0;
}
