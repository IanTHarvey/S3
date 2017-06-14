
// S3MonitorDlg.cpp : implementation file
//
#include "stdafx.h"

#include <afxsock.h>		// MFC socket extensions
#include "afxpriv.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifdef TRIZEPS
#include "drvLib_app.h"
#endif

#include "S3DataModel.h"
#include "S3GPIB.h"
#include "S3I2C.h"
#include "S3GPIO.h"

#include "S3ControllerDlg.h"

#define NO_USE_TREE_ICONS

// ----------------------------------------------------------------------------
// High precision timer, see:
// http://stackoverflow.com/questions/1739259/how-to-use-queryperformancecounter

#ifdef S3_DIAG_TIMING
double PCFreq = 0.0;
__int64 CounterStart[8] = {0, 0, 0, 0, 0, 0, 0, 0};
FILE	*TimerLog = NULL;
#define S3_DIAG_TIMER_FILE	"TimeLog.log"
char S3DiagTimerFileName[S3_MAX_FILENAME_LEN];
#endif

int S3TimerInit(CS3ControllerDlg *dlg)
{
#ifdef S3_DIAG_TIMING
	
	// Write to root as writing to \flashdisk significantly affects timings
	sprintf_s(S3DiagTimerFileName, S3_MAX_FILENAME_LEN, "%s",
			S3_DIAG_TIMER_FILE);

	// Open and clear file
	int err = fopen_s(&TimerLog, S3DiagTimerFileName, "w");
	if (!err)
	{
		char str[S3_DATETIME_LEN];
		dlg->GetDateTimeStrA(str);

		fprintf(TimerLog, "%s\n", str);
		fclose(TimerLog);
	}
#endif
	return 0;
}

int S3TimerStart(unsigned char Tid)
{
#ifdef S3_DIAG_TIMING
	
	LARGE_INTEGER li;
    if(!QueryPerformanceFrequency(&li))
		return 1;

    PCFreq = double(li.QuadPart)/1000.0;

    QueryPerformanceCounter(&li);
    CounterStart[Tid] = li.QuadPart;
#endif
	return 0;
}

int S3TimerStop(unsigned char Tid)
{
#ifdef S3_DIAG_TIMING
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
	double t = (double)(li.QuadPart - CounterStart[Tid]) / PCFreq;

	int err = fopen_s(&TimerLog, S3DiagTimerFileName, "a");
	if (!err)
	{
		fprintf(TimerLog, "%d: %.2fms\n", Tid, t);
		fclose(TimerLog);
	}
	else return 1;
#endif
    return 0;
}

// ----------------------------------------------------------------------------

// CAboutDlg dialog used for App About

CS3ControllerDlg::CS3ControllerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CS3ControllerDlg::IDD, pParent)
{
	// _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

#if DBGLOG == 1
	// If in WEC7 root, will disappear on reboot - put in \Flashdisk if want
	// non-volatile.
	S3DbgLog = NULL;
	errno_t err = fopen_s(&S3DbgLog, "S3GUI.log", "w");
#endif

	debug_print("CS3ControllerDlg: ctor start\n");

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	// m_AlarmState = 0;
	m_AnimateState = 0;

	m_IPThreadRun = false;
	m_EthEnabled = true;
	m_EthInactivityTimer = S3_ETH_ACTIVITY_TIMEOUT;

	m_USBEnabled = true; // false; // TEST:
	// m_DemoMode = false;

	m_SWUpdateScheduled = false;
	m_AppUpdateScheduled = false;

	OSDetect();
	
	debug_print("CS3ControllerDlg: ctor end\n");
}

// ----------------------------------------------------------------------------

void CS3ControllerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GDI_STATIC, m_GDIStatic);
}

BEGIN_MESSAGE_MAP(CS3ControllerDlg, CDialog)
	//{{AFX_MSG(CS3MonitorDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CALLBACK TimerProc(void* lpParametar, BOOLEAN TimerOrWaitFired)
{
    // This is used only to call QueueTimerHandler
    // Typically, this function is static member of CTimersDlg
    CS3ControllerDlg* obj = (CS3ControllerDlg*) lpParametar;
    // obj->QueueTimerHandler();
}


// CS3MonitorDlg message handlers
// CViaLiteMonDlg message handlers
BOOL CS3ControllerDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
		pMsg->wParam = VK_TAB;

	return CDialog::PreTranslateMessage(pMsg);;
}

bool GetBuildNum(void);

// extern int S3I2CTxAuthenticate();
extern unsigned char S3I2CCurTxOptAddr;
extern unsigned char S3I2CCurRxOptAddr;

extern unsigned char S3I2CRxOptAddr[];
extern unsigned char S3I2CTxOptAddr[];

BOOL CS3ControllerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	errno_t err;

	debug_print("OnInitDialog: Starting\n");

	// Set the icon for this dialog.  The framework does this automatically
	// when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_S3Data = S3Init(false);

	unsigned char CompMode = S3GetTCompMode();

	if (S3GetTCompGainOption() && CompMode == S3_TCOMP_GAIN)
		CompMode = S3_TCOMP_CONT;

	S3SetTCompMode(CompMode + 100); // Force update

	debug_print("OnInitDialog: Initialised model\n");
	// See comments in S3EventLog.cpp
	// S3EventAddNotifyFn(&S3EventNotification);

	m_Remote = S3GetRemote();

	UpdateDateTime();
	S3TimerInit(this);

	S3GPIOInit();
	S3OSInit();
	S3I2CInit();

	// TODO: Actually any use?
	GetBuildNum();
	
	// Set GDI
	m_GDIStatic.SetWindowPos(&wndTop, 0, 0, M_SCREEN_WIDTH, M_SCREEN_HEIGHT,
								SWP_NOMOVE); // | SWP_NOSIZE);

	m_GDIStatic.m_Parent = this;
	m_GDIStatic.S3GDIInit();

	m_FactoryDlg = new CS3FactorySetUp(this);
	m_FactoryDlg->Create(IDD_FACTORY_DIALOG, this);
	m_FactoryDlg->SetWindowPos(&wndTop, 0, 0, M_SCREEN_WIDTH, M_SCREEN_HEIGHT,
								SWP_NOMOVE);

	m_FactorySysDlg = new CS3FactorySysSetUp(this);
	m_FactorySysDlg->Create(IDD_FACTORY_SYS_DIALOG, this);
	m_FactorySysDlg->SetWindowPos(&wndTop, 0, 0, M_SCREEN_WIDTH, M_SCREEN_HEIGHT,
								SWP_NOMOVE);

	// Initialize Winsock
	int		iResult;
	WORD	VersionReqd;
	WSADATA wsaData;
	
	m_ListenSocket = INVALID_SOCKET;

	VersionReqd = MAKEWORD(2, 2);

	unsigned char retries = 0;
	while(retries++ < 10)
	{
		iResult = WSAStartup(VersionReqd, &wsaData);

		if (!iResult)
		{
			retries = 0;
			break;
		}

		if (iResult != WSASYSNOTREADY)
		{
			debug_print("OnInitDialog: WSAStartup failed: %d\n",
													iResult);
			return 1;
		}
	}

	if (retries >= 10)
	{
		debug_print("OnInitDialog: WSAStartup failed: %d (retries: %d)\n",
													iResult, retries);
		return 1;
	}

	err = RemoteOpenEth();
	
	if (err == 1)
	{
		debug_print("OnInitDialog: RemoteOpenEth failed: %d\n", err);
		S3EventLogAdd("Failed to initialise socket stream", 1, -1, -1, -1);
	}

	err = RemoteOpenUSB();
	
	if (err == 2)
	{
		debug_print("OnInitDialog: RemoteOpenUSB failed: %d\n", err);

		char msg[S3_EVENTS_LINE_LEN];
		sprintf_s(msg, S3_EVENTS_LINE_LEN, "Failed to open USB port: %S", 
			GetUSBPortName());

		S3EventLogAdd(msg, 1, -1, -1, -1);
	}
	else if (err == 3)
	{
		S3EventLogAdd("No available USB ports found", 1, -1, -1, -1);
	}

#ifdef WINCE
	ShowWindow(SW_SHOWMAXIMIZED);
#endif

	// char Filename[S3_MAX_FILENAME_LEN];

	// sprintf_s(Filename, S3_MAX_FILENAME_LEN, "%s\\%s",
	//	S3_ROOT_DIR, S3_DEF_CONFIG_FILENAME);

	// ...now kick everything off
	SetTimer(IDT_S3_GUI_UPDATE_TIMER,	S3_GUI_UPDATE_INTERVAL,	NULL);
	SetTimer(IDT_S3_RX_POLL_TIMER,		S3_RX_POLL_INTERVAL,	NULL);
	SetTimer(IDT_S3_COMM_POLL_TIMER,	S3_COMM_POLL_INTERVAL,	NULL);

    //StartCounter();
    //Sleep(1000);
    //double t = GetCounter();

	return TRUE;  // return TRUE  unless you set the focus to a control
} // OnInitDialog



// ----------------------------------------------------------------------------

void CS3ControllerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	/*
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	*/
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}


// ----------------------------------------------------------------------------
// If you add a minimize button to your dialog, you will need the code below
// to draw the icon.  For MFC applications using the document/view model,
// this is automatically done for you by the framework.

void CS3ControllerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		// Not CE
		// SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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
		// if (PaintForm)
			CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
// the minimized window.
HCURSOR CS3ControllerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CS3ControllerDlg::OnShutdownRequest(WPARAM wParam, LPARAM lParam)
{
    SysShutdown();
    return 0;
}

extern void SystemShutdown(void);

void CS3ControllerDlg::SysShutdown()
{
	if (S3DbgLog) fclose(S3DbgLog);

	KillTimer(IDT_S3_GUI_UPDATE_TIMER);
	KillTimer(IDT_S3_RX_POLL_TIMER);
	KillTimer(IDT_S3_COMM_POLL_TIMER);

	S3Save(NULL);
	S3I2CClose();
	S3GPIOClose();
	RemoteClose();

	S3End(); // Do last as S3Data required by others to shut down cleanly

	SystemShutdown();

	OnCancel();
}

// ----------------------------------------------------------------------------


void CS3ControllerDlg::AppShutdown()
{
	m_GDIStatic.S3GDIRemoteCmd();

	if (S3DbgLog) fclose(S3DbgLog);

	BOOL err;

	// KillTimer doesn't remove queued messages
	err = KillTimer(IDT_S3_GUI_UPDATE_TIMER);
	err = KillTimer(IDT_S3_RX_POLL_TIMER);
	err = KillTimer(IDT_S3_COMM_POLL_TIMER);

	S3Save(NULL);
	S3I2CClose();
	S3GPIOClose();
	RemoteClose();

	S3End(); // Do last as S3Data required by others to shut down cleanly
    WSACleanup();

	OnCancel();
}

// ----------------------------------------------------------------------------

void CS3ControllerDlg::OSDetect()
{
	// Now properly deprecated - see sysinfoapi
	/*
	OSVERSIONINFO OSversion;

	OSversion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	::GetVersionEx(&OSversion);

	switch (OSversion.dwPlatformId)
	{
	case VER_PLATFORM_WIN32s:
		m_OSStr.Format(_T("Windows %d.%d"), OSversion.dwMajorVersion,
			OSversion.dwMinorVersion);
		break;
	case VER_PLATFORM_WIN32_WINDOWS:
		if (OSversion.dwMinorVersion == 0)
			m_OSStr = "Windows	95";
		else if (OSversion.dwMinorVersion == 10)
			m_OSStr = "Windows	98";
		else if (OSversion.dwMinorVersion == 90)
			m_OSStr = "Windows	Me";
		break;
	case VER_PLATFORM_WIN32_NT:
		if (OSversion.dwMajorVersion == 5 && OSversion.dwMinorVersion == 0)
			m_OSStr.Format(_T("Windows 2000 With %s"), OSversion.szCSDVersion);
		else if (OSversion.dwMajorVersion == 5 && OSversion.dwMinorVersion == 1)
			m_OSStr.Format(_T("Windows XP %s"), OSversion.szCSDVersion);
		else if (OSversion.dwMajorVersion <= 4)
			m_OSStr.Format(_T("Windows NT %d.%d with %s"), OSversion.dwMajorVersion,
			OSversion.dwMinorVersion, OSversion.szCSDVersion);
		else
			// for unknown windows/newest windows version	  
			m_OSStr.Format(_T("Windows %d.%d"), OSversion.dwMajorVersion,
			OSversion.dwMinorVersion);
	}
	*/
}

// ----------------------------------------------------------------------------

void CS3ControllerDlg::OnGUIUpdateTimer(void)
{    
	UpdateDateTime();

	m_AnimateState = !m_AnimateState;

	// TODO: Redraw animated items only
	m_GDIStatic.S3GDIRedraw();

	return;
}

// ----------------------------------------------------------------------------

#ifdef S3_TX_BATT_LOG
int battlogcnt = 0;
#endif

void CS3ControllerDlg::OnI2CPollTimer(void)
{
	// I2C_Init();
	if (m_SWUpdateScheduled)
	{
		int err = S3OSImageUpdate();

		if (err)
		{
			m_SWUpdateScheduled = false;
		}
		else
		{
			// We've got a few seconds
			AppShutdown();
		}
	}
	else if (m_AppUpdateScheduled)
	{
		int err = S3OSAppUpdate();

		if (err)
		{
			m_AppUpdateScheduled = false;
		}
		else
		{
			// We've got a few seconds
			AppShutdown();
		}
	}

	if (S3GetPowerDownPending())
	{
		m_GDIStatic.S3GDIChangeScreen(S3_OVERVIEW_SCREEN);
	}

	if (S3GetCloseAppPending())
	{
		m_GDIStatic.S3GDIChangeScreen(S3_CLOSED_SCREEN);
	}

	if (S3Poll(this))
	{	
	}

#ifdef S3_TX_BATT_LOG
	battlogcnt++;

	// TEST: 5 min interval
	if (battlogcnt == (int)(5 * 60.0 * 1000.0 / (double)S3_RX_POLL_INTERVAL))
	{
		battlogcnt = 0;
		for(char Rx = 0; Rx < 6; Rx++)
		{
			for(char Tx = 0; Tx < 6; Tx++)
			{
				if (S3TxGetType(Rx, Tx) != S3_TxUnconnected)
					TxLogBatt(Rx, Tx);
			}
		}
	}

#endif

}

// ----------------------------------------------------------------------------

void CS3ControllerDlg::OnCommPollTimer(void)
{
		// TODO: Do not attempt this until found reliable
		// connection failure detection method
		RemoteOpenEth();

		// Limit just to prevent wraparound
		if (m_EthInactivityTimer < 100)
			m_EthInactivityTimer++;

		RemoteOpenUSB();
		
		/*
		if (m_USBEnabled)
		{
			if (!m_COMPort->IsPortOpen())
			{	
				// Try to open one
				if (m_COMPort->OpenPort())
				{
					CString tmp;
					m_COMPort->GetOpenPortName(tmp);
				}
			}
		}

		if (m_EthEnabled)
		{
			if (!m_COMPort->IsPortOpen())
			{	
				// Try to open one
				if (m_COMPort->OpenPort())
				{
					CString tmp;
					m_COMPort->GetOpenPortName(tmp);
				}
			}
			else
			{
				// Check it's still there
				if (m_COMPort->EnumCOMPorts(true) == 0)
				{
					CString tmp = _T("");
				}
			}
		}
		*/
}
// ----------------------------------------------------------------------------

char m_TimerPhase = 0;

char m_TimerFn[8] = {0, 1, 0, 2, 0, 1, 0, 3};

void CS3ControllerDlg::OnTimer(UINT nIDEvent)
{
	/*
	if (nIDEvent == IDT_S3_GUI_UPDATE_TIMER)
	{
		switch(	m_TimerFn[m_TimerPhase++])
		{
			case 0: OnGUIUpdateTimer(); break;
			case 1: OnI2CPollTimer(); break;
			case 2: OnCommPollTimer(); break;
		}

		if (m_TimerPhase == 8)
			m_TimerPhase = 0;
		
	}
	*/	
	
	if (nIDEvent == IDT_S3_GUI_UPDATE_TIMER)
	{		
		OnGUIUpdateTimer();
	}
	else if (nIDEvent == IDT_S3_RX_POLL_TIMER)
	{
		OnI2CPollTimer();
	}
	else if (nIDEvent == IDT_S3_COMM_POLL_TIMER)
	{
		OnCommPollTimer();
	}

	CDialog::OnTimer(nIDEvent);
}
// ----------------------------------------------------------------------------
// Experimental, not used

/*
void CS3ControllerDlg::OnBnClickedAlarmTrigButton()
{
	if (m_AlarmState == 0)
	{
		for (u_char i = 0; i < TREE_DEPTH; i++)
			m_AlarmNode[i] = m_CurrentNode[i];

		m_AlarmState = 10;

		m_AlarmSimButton.SetWindowText(_T("Alarm Cancel"));

		// Not CE
		// m_MainTree.SetBkColor(0x000000FF); // Red

		SetTimer(IDT_ALARM_TIMER, 1000, 0);
	}
	else
	{
		for (u_char i = 0; i < 4; i++)
			m_AlarmNode[i] = 0;

		m_AlarmState = 0;

		m_AlarmSimButton.SetWindowText(_T("Alarm Trig"));

		// Not CE
		// m_MainTree.SetBkColor(-1); // -1 = background

		KillTimer(IDT_ALARM_TIMER);
	}
}
*/ 

// ----------------------------------------------------------------------------
// Static control must have SS_NOTIFY style enabled for this to be called.

/*
void CS3ControllerDlg::OnStnClickedGdiStatic()
{
	POINT p;

	GetCursorPos(&p);

	// ::ScreenToClient(m_hWnd, &p);
	
	// ADC_GetLastTouchValues not in drvlib_app.lib (in header tho')
	// AC97_Alloc(0);
	// unsigned int RawX, RawY, RawZ;
	// ADC_GetLastTouchValues(&RawX, &RawY, &RawZ);

	m_GDIStatic.S3Find(p);

	// m_GdiStatic.SetCurrentCoord(p);
}
*/

// ----------------------------------------------------------------------------

void CS3ControllerDlg::UpdateDateTime(void)
{
	CTime CurrentTime = CTime::GetCurrentTime();

	m_SysTime.wDayOfWeek = 0;
	m_SysTime.wHour = CurrentTime.GetHour();
	m_SysTime.wMinute = CurrentTime.GetMinute();
	m_SysTime.wSecond = CurrentTime.GetSecond();

	m_SysTime.wYear = CurrentTime.GetYear();
	m_SysTime.wMonth = CurrentTime.GetMonth();
	m_SysTime.wDay = CurrentTime.GetDay();

	S3SetDateTime(m_SysTime.wHour, m_SysTime.wMinute, m_SysTime.wSecond,
		m_SysTime.wYear, m_SysTime.wMonth, m_SysTime.wDay);

}

// ----------------------------------------------------------------------------

void CS3ControllerDlg::GetDateTimeStr(CString &str)
{
	// ISO 8601 standard format
	str.Format(_T("%02d-%02d-%02d %02d:%02d:%02d"),
		m_SysTime.wYear, m_SysTime.wMonth, m_SysTime.wDay,
		m_SysTime.wHour, m_SysTime.wMinute, m_SysTime.wSecond);
}

void CS3ControllerDlg::GetDateStr(CString &str)
{
	str.Format(_T("%02d-%02d-%02d"),
		m_SysTime.wYear, m_SysTime.wMonth, m_SysTime.wDay);
}

void CS3ControllerDlg::GetTimeStr(CString &str)
{
	str.Format(_T("%02d:%02d:%02d"),
		m_SysTime.wHour, m_SysTime.wMinute, m_SysTime.wSecond);
}

// ----------------------------------------------------------------------------

void CS3ControllerDlg::GetDateTimeStrA(char *str)
{
	// ISO 8601 standard format
	sprintf_s(str, S3_DATETIME_LEN, "%02d-%02d-%02d %02d:%02d:%02d",
		m_SysTime.wYear, m_SysTime.wMonth, m_SysTime.wDay,
		m_SysTime.wHour, m_SysTime.wMinute, m_SysTime.wSecond);
}

// ----------------------------------------------------------------------------

void CS3ControllerDlg::GetTimeStrA(char *str)
{
	sprintf_s(str, S3_DATETIME_LEN, "%02d:%02d:%02d",
		m_SysTime.wHour, m_SysTime.wMinute, m_SysTime.wSecond);
}

// ----------------------------------------------------------------------------

int CS3ControllerDlg::SetSysTime(short h, short m, short s)
{
	m_SysTime.wHour = h;
	m_SysTime.wMinute = m;
	m_SysTime.wSecond = s;
	m_SysTime.wMilliseconds = 0;

	BOOL ok = SetSystemTime(&m_SysTime);
	
	return (ok == 0);
}

// ----------------------------------------------------------------------------

int CS3ControllerDlg::SetSysDate(short y, short m, short d)
{
	m_SysTime.wYear = y;
	m_SysTime.wMonth = m;
	m_SysTime.wDay = d;

	BOOL ok = SetSystemTime(&m_SysTime);
	
	return (ok == 0);
}

// ----------------------------------------------------------------------------
// A very basic time parser
int CS3ControllerDlg::SetSysTimeStr(CString &str)
{
	wchar_t	*eptr;
	char h = (char)_tcstol(str, &eptr, 10);

	if (*eptr != ':')
		return 1;

	char m = (char)_tcstol(eptr + 1, &eptr, 10);

	if (*eptr != ':')
		return 1;

	char s = (char)_tcstol(eptr + 1, &eptr, 10);

	if (*eptr != '\0')
		return 1;

	return SetSysTime(h, m, s);
}

// ----------------------------------------------------------------------------
// A very basic date parser
int CS3ControllerDlg::SetSysDateStr(CString &str)
{
	wchar_t	*eptr;
	short y = (short)_tcstol(str, &eptr, 10);

	if (*eptr != '-')
		return 1;

	short m = (short)_tcstol(eptr + 1, &eptr, 10);

	if (*eptr != '-')
		return 1;

	short d = (short)_tcstol(eptr + 1, &eptr, 10);

	if (*eptr != '\0')
		return 1;

	return SetSysDate(y, m, d);
}

// ----------------------------------------------------------------------------

void CS3ControllerDlg::S3GDIReInitialise()
{
	m_GDIStatic.S3GDIReInitialise();
}

// ----------------------------------------------------------------------------

void CS3ControllerDlg::S3GDIRedraw()
{
	m_GDIStatic.S3GDIRedraw();
}

// ----------------------------------------------------------------------------

/*
void CS3ControllerDlg::OnStnDblclickGdiStatic()
{
	POINT p;

	GetCursorPos(&p);

	// ::ScreenToClient(m_hWnd, &p);

	m_GDIStatic.S3Find(p);
}
*/

// ----------------------------------------------------------------------------
// This have to be so ugly?

bool GetBuildNum(void)
{
    // Get the filename of the executable containing the version resource

	char Msg[S3_EVENTS_LINE_LEN];

    TCHAR szFilename[MAX_PATH + 1] = {0};
    if (GetModuleFileName(NULL, szFilename, MAX_PATH) == 0)
    {
		sprintf_s(Msg, S3_EVENTS_LINE_LEN, "GetModuleFileName: Error %d\n", GetLastError());
		S3EventLogAdd(Msg, 1, -1, -1, -1);
        return false;
    }

    // Allocate memory for the version info
    DWORD dummy;
    DWORD dwSize = GetFileVersionInfoSize(szFilename, &dummy);
    if (dwSize == 0)
    {
		sprintf_s(Msg, S3_EVENTS_LINE_LEN, "GetFileVersionInfoSize: Error %d\n", GetLastError());
		S3EventLogAdd(Msg, 1, -1, -1, -1);
        return false;
    }
    
	DWORD *data = new DWORD[dwSize];

    // Get the version info
    if (!GetFileVersionInfo(szFilename, NULL, dwSize, &data[0]))
    {
		sprintf_s(Msg, S3_EVENTS_LINE_LEN, "GetFileVersionInfo: Error %d\n", GetLastError());
		S3EventLogAdd(Msg, 1, -1, -1, -1);
        return false;
    }

    // Get the root version string
    LPVOID pvProductVersion = NULL;
    unsigned int iProductVersionLen = 0;

	VerQueryValue(&data[0], _T("\\"), &pvProductVersion, &iProductVersionLen);

	VS_FIXEDFILEINFO *fixedinfo;

	fixedinfo = (VS_FIXEDFILEINFO *)pvProductVersion;
	unsigned short *ver = (unsigned short *)&fixedinfo->dwProductVersionMS;

	char str[S3_MAX_BUILD_NUM_LEN];

	sprintf_s(str, S3_MAX_BUILD_NUM_LEN, "%d.%d.%d.%d",
		ver[1], ver[0], ver[3], ver[2]);

	S3SysSetBuildNum(str);

    // Replace "040904e4" with the language ID of your resources
    /*
	if (!VerQueryValue(&data[0], _T("\\StringFileInfo\\040904e4\\ProductName"), &pvProductName, &iProductNameLen) ||
        !VerQueryValue(&data[0], _T("\\StringFileInfo\\040904e4\\ProductVersion"), &pvProductVersion, &iProductVersionLen))
    {
        printf("Can't obtain ProductName and ProductVersion from resources\n");
        return false;
    }
	*/

	delete data;

    return true;
}

void CS3ControllerDlg::ShowFactory(char screen)
{
	if (screen == S3_FACTORY_SCREEN)
	{
		if (S3SetFactoryMode(-1, -1, true))
			return;

		m_FactoryDlg->Init();
		m_FactoryDlg->ShowWindow(SW_SHOWMAXIMIZED);
		m_GDIStatic.S3GDIChangeScreen(S3_CALIBRATE_SCREEN);
	}
	else if (screen == S3_FACTORY_SYS_SCREEN)
	{
		if (S3SetFactoryMode(-1, -1, true))
			return;

		m_FactorySysDlg->Init();
		m_FactorySysDlg->ShowWindow(SW_SHOWMAXIMIZED);
		m_GDIStatic.S3GDIChangeScreen(S3_FACTORY_SYS_SCREEN);
	}
}

// ---------------------------------------------------------------------------

void CS3ControllerDlg::HideFactory(void)
{
	S3SetFactoryMode(0, 0, false);

	if (m_GDIStatic.S3GDIGetScreen() == S3_CALIBRATE_SCREEN)
		m_FactoryDlg->ShowWindow(false);
	else if (m_GDIStatic.S3GDIGetScreen() == S3_FACTORY_SYS_SCREEN)
		m_FactorySysDlg->ShowWindow(false);

	m_GDIStatic.S3GDIChangeScreen(S3_PREVIOUS_SCREEN);
}

// ---------------------------------------------------------------------------
// TEST: Test only - file will grow indefinitely

int CS3ControllerDlg::TxLogBatt(char Rx, char Tx)
{
	if (Rx == -1 || Tx == -1)
		return -1;

	FILE	*fid = NULL;

	char tmp[S3_MAX_FILENAME_LEN];
	sprintf_s(tmp, S3_MAX_FILENAME_LEN, "%s\\%s_%d_%d.s3l",
		m_S3Data->m_EventLogPath, "TXBatt", Rx, Tx);

	int err = fopen_s(&fid, tmp, "a");

	if (err)
		return err;

	char t[S3_DATETIME_LEN];
	GetTimeStrA(t);

	t[5] = '\0';

	fprintf(fid, "%s:\t%03d\t%03d\t%3d\t%5.1f\n",
		t,
		S3TxGetBattSoC(Rx, Tx),
		S3TxGetATTE(Rx, Tx),
		S3TxGetBattI(Rx, Tx),
		(double)S3TxGetBattTemp(Rx, Tx) / 10.0);

	fflush(fid);
	fclose(fid);

	return 0;
}

// ------------------------------ The End -------------------------------------
