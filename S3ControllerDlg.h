
// S3ControllerDlg.h : header file
//

#pragma once

#include <afxsock.h>		// MFC socket extensions
#include <afxcmn.h>
#include <afxwin.h>

#include "resource.h"
#include "S3GPIB.h"
#include "S3USBVCP.h"
#include "S3GDIScreenMain.h"
#include "S3FactorySetUp.h"
#include "S3FactorySysSetUp.h"

#include "S3Controller.h"

#define IDT_S3_GUI_UPDATE_TIMER	(WM_USER + 200)	// Update display
#define IDT_S3_ALARM_TIMER		(WM_USER + 201)	// Not used
#define IDT_S3_RX_POLL_TIMER	(WM_USER + 202)	// Scan receivers
#define IDT_S3_COMM_POLL_TIMER	(WM_USER + 203) // Re-acquire ethernet and USB comms

#define S3_GUI_UPDATE_INTERVAL	500	// ms
#define S3_RX_POLL_INTERVAL		500	// ms
#define S3_COMM_POLL_INTERVAL	1225	// ms
#define S3_ETH_ACTIVITY_TIMEOUT 20

#define M_SCREEN_WIDTH			800
#define M_SCREEN_HEIGHT			480

// CS3MonitorDlg dialog
class CS3ControllerDlg : public CDialog
{
	int				m_DialogResourceId[2];
	CDialog			m_Dialog;

	// CFont	m_nonCleartypeFont;

	PHANDLE			m_timerHandle;

	pS3DataModel	m_S3Data; // THE model
	char			m_Remote;
	
	CString			m_OSStr;
	void			OSDetect(void);


	// Remote stuff
	int				RemoteOpenEth(void);
	
	// USB stuff
	int				RemoteOpenUSB(void);
	int				InitUSB(void);

	// Socket stuff
	int				InitSocket(void);
	int				CloseSocket(void);
	int				SendMsg(const char *pMsg);
	void			WriteWindow(void);

	bool			m_EthEnabled; 

	// bool			m_DemoMode;
	unsigned int	m_EthInactivityTimer; // ms

	// USB virtual COM port stuff
	bool			m_USBEnabled;
	CS3USBVCP		m_COMPort;

	SYSTEMTIME		m_SysTime;
	__time64_t		m_PosixTime;

	void	UpdateDateTime(void);
	void	UpdateCtrlTemps(void);

	void	S3EventNotification(void);
	void	UpdateEventLog(void);

	int		ReadTree(void);

	int		ReadForm(void);
	int		WriteForm(void);
	
	void	RemoteClose();
	void	SetS3Data(pS3DataModel m) { m_S3Data = m; };

 	CS3GDIScreenMain	m_GDIStatic;
	CS3FactorySetUp		*m_FactoryDlg;
	CS3FactorySysSetUp	*m_FactorySysDlg;
	CS3FactorySetUp		*m_FactoryCalDlg;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Implementation
	HICON m_hIcon;

	// -------------------------------------------------------
	// Generated message map functions
	virtual BOOL OnInitDialog();
	BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg	void OnTimer(UINT TimerVal);
	afx_msg LRESULT OnShutdownRequest(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

public:
	CWinThread	*m_IPThread;
	char		m_AnimateState; // Toggled by timer
	bool		m_IPThreadRun; // = true; // False to terminate IP thread
	SOCKET		m_ListenSocket;

	CS3ControllerDlg(CWnd* pParent = NULL);	// standard constructor

	void			S3GDIReInitialise(void);
	void			S3GDIRedraw(void);

	// Dialog Data
	enum { IDD = IDD_S3CONTROLLER_DIALOG };

	bool		m_SWUpdateScheduled;
	bool		m_AppUpdateScheduled;

	void S3DrawGDITxSel(void);
	void ShowFactory(char screen);
	void HideFactory(void);

	// Time sub-functions
	void OnGUIUpdateTimer(void);
	void OnI2CPollTimer(void);
	void OnCommPollTimer(void);

	void ForceRedraw(void);

	int TxLogBatt(char Rx, char Tx);

	void	GetDateTimeStr(	CString &str);
	void	GetTimeStr(		CString &str);
	void	GetDateStr(		CString &str);

	__time64_t GetPosixTime();

	void	GetDateTimeStrA(char *str);
	void	GetTimeStrA(	char *str);

	int		SetSysTimeStr(		const wchar_t *str);
	int		SetSysDateStr(		const wchar_t *str);
	int		SetSysTime(short h, short m, short s);
	int		SetSysDate(short y, short m, short d);

	int		ParseMsg(const char *pMsg, char MsgSrc);
	
	void	AppShutdown();
	void	SysShutdown();

	CString		GetUSBPortName(void);
	CString		GetUSBDriverType(void);
	
	bool		GetUSBEnabled(void);
	void		SetUSBEnabled(bool enable);

	int			ResetSocket(void);

	HDC		GetDrawable()	{return m_GDIStatic.GetDrawable(); };
	HFONT	GetDefFont()	{return m_GDIStatic.GetDefFont(); };

	// FILE	*m_DbgLog;	// Socket errors/info only
	// afx_msg void OnStnDblclickGdiStatic();
};

// ----------------------------------------------------------------------------
