
// S3ControllerDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"

#define IDT_UPDATE_TIMER		(WM_USER + 200)
#define IDT_ALARM_TIMER			(WM_USER + 201)
#define IDT_S3_RX_POLL_TIMER	(WM_USER + 202)

// m_CurrentTab
#define CONFIG_TAB	0
#define MONITOR_TAB	1
#define SYSTEM_TAB	2

#define TREE_DEPTH	3

// CS3MonitorDlg dialog
class CS3ControllerDlg : public CDialog
{
	int				m_DialogResourceId[2];
	CDialog			m_Dialog;
	CS3PrefsDlg		m_PrefsDlg;

	u_char			m_CurrentTab;

	// CFont	m_nonCleartypeFont;

	pS3DataModel	m_S3Data; // THE model
	char			m_Remote;
	
	// TODO: Test only
	int				m_ActiveIP;

	CString			m_OSStr;
	void			OSDetect(void);

	// Socket stuff
	int				InitSocket(void);
	int				SendMsg(const char *pMsg);
	void			WriteWindow(void);

	CWinThread		*m_IPThread;

	// Each tree control node's data pointer points into this and provides the
	// Rx/Tx/IP address of the node. Indexing is 1-based, zero is a non-existent
	// reference (eg node Rx 6 = [6 0 0]).
	u_char		m_NodeData[1 + 6 + (6 * 6) + (6 * 6 * 8)][TREE_DEPTH];
	u_char		m_CurrentNode[TREE_DEPTH]; // Selected node 1-indexed (NOT 0)

	u_char		m_AlarmNode[TREE_DEPTH]; // Experimental, not used
	u_char		m_AlarmState;

	HTREEITEM	m_PreviousSelection;
	bool		m_DontRespond, m_DontRespondDeleteAll;
	bool		m_ExternalCommand;

	HTREEITEM	SetSelection(u_char *Node);

	void	UpdateDateTime(void);
	void	UpdateCtrlTemps(void);

	void	S3EventNotification(void);
	void	UpdateEventLog(void);

	int		ReadTree(void);

	int		ReadForm(void);
	int		WriteForm(void);
	
	void	RemoteClose();
	void	SetS3Data(pS3DataModel m) { m_S3Data = m; };

	CButton m_ApplyPushButton;
	CButton m_ApplyButton;
	CButton m_DiscardButton;
	CButton m_AlarmSimButton;
	CButton m_GDITest;
	CStatic m_IncomingMsgStatic;
	CEdit m_IPAddrEdit;
	CStatic m_NodeTitleStatic;
	CStatic m_DateTimeStatic;

	CTreeCtrl m_MainTree;
	CMainTabCtrl m_PropertyTab;
	CS3GDIScreenMain m_GDIStatic;

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
	afx_msg LRESULT OnUpdateControl(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedTestButton();
	afx_msg void OnBnClickedLoadButton();
	afx_msg void OnBnClickedSaveButton();
	// afx_msg void OnTvnBeginlabeleditControllerHierarchyTree(NMHDR *pNMHDR, LRESULT *pResult);
	// afx_msg void OnTvnEndlabeleditControllerHierarchyTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedOk();
	afx_msg void OnTvnSelchangingControllerHierarchyTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedApplyButton();
	afx_msg void OnBnClickedDiscardButton();
	afx_msg void OnNMRClickControllerHierarchyTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg	void OnTimer(UINT TimerVal);
	afx_msg void OnBnClickedPushButton();
	afx_msg void OnBnClickedAlarmTrigButton();
	afx_msg void OnStnClickedGdiStatic();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedResetButton();
	afx_msg void OnTcnSelchangePropsTab(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawControllerHierarchyTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnSelchangedControllerHierarchyTree(NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()

public:
	CS3ControllerDlg(CWnd* pParent = NULL);	// standard constructor

	void			S3GDIRedraw(void);

	// Dialog Data
	enum { IDD = IDD_S3CONTROLLER_DIALOG };

	void S3DrawGDITxSel(void);
	// TODO: Ensure in-line (and any others)
	char Rx() { return m_CurrentNode[0] - 1; };
	char Tx() { return m_CurrentNode[1] - 1; };
	char IP() { return m_CurrentNode[2] - 1; };

	int		RedrawTree(void);
	
	int		SetRemote(char r);
	void	TabChanged(u_char tab);

	void	ChangeSelection(const u_char *cData);
	void	ChangeSelection(char Rx, char Tx, char IP, bool RedrawTree = true);

	void	GetTimeStr(CString &str);

	char	m_AnimateState; // Toggled by timer
	bool	m_IPThreadRun; // = true; // False to terminate IP thread

	int		ParseMsg(const char *pMsg);

	FILE	*m_Log;	// Socket errors/info only
};

// ----------------------------------------------------------------------------
