#pragma once


// CS3LogCopyDlg dialog

class CS3LogCopyDlg : public CDialog
{
	DECLARE_DYNAMIC(CS3LogCopyDlg)

public:
	CS3LogCopyDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CS3LogCopyDlg();

// Dialog Data
	enum { IDD = IDD_LOGFILECOPY_DIALOG };

    afx_msg void BrowseForFolder(void);
    afx_msg void BeginCopyLogFile(void);
    afx_msg void CancelTx(void);
    void PostNcDestroy(void);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

public:
    CButton m_CopyFileButton;
    CButton m_BrowseButton;
    CButton m_CancelButton;

    CEdit m_FileDirEdit;

    CProgressCtrl m_FileCopyProgressBar;
    CStatic m_PercentageStatic;

    CWinThread      *m_LogFileCopyThread;
    FILE *Logfile;
};

extern UINT AttemptLogFileCopyThread(LPVOID pParam);
extern bool LogCopyInProgress;