// ----------------------------------------------------------------------------
// S3AgentDlg.h : header file
//

#ifndef CS3AGENTDLG_H_
#define CS3AGENTDLG_H_

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "defines.h"
// #include <windows.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "afxdialogex.h"


//S3 Agent
#include "S3Agent.h"
#include "S3USBVCP.h"
#include "S3DataGathering.h"
#include "S3Comms.h"
#include "S3LogCopyDlg.h"
//Sentinel 3 Emulated GUI
#include "../S3DataModel.h"
#include "../S3GDIScreenMain.h"

class CS3GDIScreenMain;
class CS3LogCopyDlg;



// CS3AgentDlg dialog
class CS3AgentDlg : public CDialog
{
// Construction
public:
	CS3AgentDlg(CWnd* pParent = NULL);	// standard constructor
    CDialog m_Dialog;

// Dialog Data
	enum { IDD = IDD_S3AGENT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
    virtual BOOL CS3AgentDlg::PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	CFont m_font;

private:
	CWinThread		*m_ScriptThread;
    CWinThread      *m_ConnectToSentinel3Thread;
public:
    CWinThread      *m_DataUpdateThread;
    CS3LogCopyDlg *m_LogCopyDlg;

    afx_msg	void OnTimer(UINT_PTR TimerVal);
	void OnGUIUpdateTimer(void);

    void ExitS3RemController(void);
    void AboutDlg(void);
    afx_msg void OnBnClickedS3ConnectButton(void);
    afx_msg void OnBnClickedS3Eth(void);
    afx_msg void OnBnClickedS3USB(void);
    afx_msg void OnBnClickedS3USBSerial(void);
    afx_msg void OnBnClickedS3USBGPIB(void);
    afx_msg void OnBnClickedSendS3MsgButton(void);
    afx_msg void OnBnClickedChooseScriptButton(void);
    afx_msg void OnBnClickedRunScriptButton(void);
    afx_msg void OnUpdateS3RemGUIBtnClick(void);
    afx_msg void OnAutoUpdateGUIBtnClick(void);
    afx_msg void OnRemoteLocalPermissionsBtnClick(void);
    afx_msg void OnBnClickedAutoSyncTime(void);
    afx_msg void OnBnClickedManualEntry(void);
    afx_msg void OnBnClickedLogFileCopy(void);
    afx_msg LRESULT OnLogCopyFinished(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnDisconnect(WPARAM wParam, LPARAM lParam);
    void UpdateSentinelGUI(void);
    void ResizeWindow(char size);
    int EnumCOMPorts();

    //UI Controls    
    CEdit m_IPv4AddrEdit;
    CEdit m_PortEdit;
    CEdit m_MessageEdit;
    CComboBox m_SerialDropDown;
	CButton m_OpenUSB;
	CButton m_CloseUSB;
	CButton m_SendUSB;
	CButton m_StartStopScriptButton;
    CButton m_ConnectS3Button;
    CButton m_SendManCMDButton;
    CButton m_EthSelRdoButton;
    CButton m_USBSelRdoButton;
    CButton m_SerialSelRdoButton;
    CButton m_GPIBSelRdoButton;
    CButton m_AutoUpdateGUITick;
    CButton m_RemoteLocalCheck;
    CStatic m_ScriptNameStatic;
    CEdit m_CommandEdit;
    CStatic m_IMGStatic;
    CS3GDIScreenMain	m_GDIStatic;
    CToolTipCtrl m_ToolTip;
    CStatic m_ManualGroupBox;

    char m_TestScriptName[MAX_SCRIPT_LEN];
	
	bool		m_SWUpdateScheduled;
	bool		m_AppUpdateScheduled;

    bool	m_ScriptThreadRun; // = true; // False to terminate IP thread
    bool	m_ScriptStop;
    bool    AutoCheckS3Data;
    bool    ManualDataBundleUpdateInProgress;
    bool    ManualCommandEntryVisible;
    bool    AutoSyncTimesOnConnect;
    
	HDC		GetDrawable()	{return m_GDIStatic.GetDrawable(); };
	HFONT	GetDefFont()	{return m_GDIStatic.GetDefFont(); };

// For use by the Sentinel 3 GUI emulation
    char	m_AnimateState; // Toggled by timer
    pS3DataModel	m_S3Data; // THE model

};

//Script Processing Thread
extern UINT ScriptProcessThread(LPVOID pParam);
//Initial connection to Sentinel 3
extern UINT AttemptConnectToSentinel3(LPVOID pParam);
//Get current status data from Sentinel 3
extern int GetAllSentinelData(void);
extern UINT AutoUpdateSentinelDataBundleThread(LPVOID pParam);
//Copy log file from the Sentinel 3
extern bool LogCopyInProgress;

//Global vairables
//Data bundle of all the data we need for the UI
extern Sentinel3DataBundle Sentinel3;
//Mutexes to ensure the threads can operate safely
// extern boost::mutex CommsMUTEX;

//Connection/messages subroutines
extern void CloseConnectUSB(void);
extern CString SendSentinel3Message(CString message);

extern CString LastUpdateDateStr; 
extern CString LastUpdateTimeStr;
// ----------------------------------------------------------------------------
#endif