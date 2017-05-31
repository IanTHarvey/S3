// S3LogCopyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "S3Agent.h"
#include "S3LogCopyDlg.h"
#include "S3AgentDlg.h"

TCHAR LogDir[MAX_PATH];
// CS3LogCopyDlg dialog

IMPLEMENT_DYNAMIC(CS3LogCopyDlg, CDialog)

CS3LogCopyDlg::CS3LogCopyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CS3LogCopyDlg::IDD, pParent)
{

}

CS3LogCopyDlg::~CS3LogCopyDlg()
{
}

void CS3LogCopyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_COPY, m_CopyFileButton);
    DDX_Control(pDX, IDC_BROWSE, m_BrowseButton);
    DDX_Control(pDX, IDCANCEL, m_CancelButton);

    DDX_Control(pDX, IDC_FILEDIR, m_FileDirEdit);

    DDX_Control(pDX, IDC_FILE_PROGRESS, m_FileCopyProgressBar);
    DDX_Control(pDX, IDC_PERCENTAGE, m_PercentageStatic);
}


BEGIN_MESSAGE_MAP(CS3LogCopyDlg, CDialog)

    ON_BN_CLICKED(IDC_BROWSE, &CS3LogCopyDlg::BrowseForFolder)

    ON_BN_CLICKED(IDC_COPY, &CS3LogCopyDlg::BeginCopyLogFile)
    ON_BN_CLICKED(IDCANCEL, &CS3LogCopyDlg::CancelTx)

END_MESSAGE_MAP()

BOOL CS3LogCopyDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    char Filename[MAX_PATH];
	FILE *fid;
	int err = 0;

    //Check if we have previously saved a "preferred" log file directory. If not, use the My Documents location. If that fails, use the root of the C drive
    _tcscpy_s(LogDir, MAX_PATH, DataLocStr);
    PathAppend(LogDir, TEXT("\\S3logdir.txt"));
    if(GetFileAttributes(LogDir) == INVALID_FILE_ATTRIBUTES)
    {
        sprintf_s(Filename, MAX_PATH, "%ls", LogDir);
        err = fopen_s(&fid, Filename, "w");
        if (!err)
        {
	        if(!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, 0, LogDir)))
            {
                 _tcscpy_s(LogDir, MAX_PATH, _T("C:\\"));
            }
            fprintf(fid, "%ls", LogDir);
            fclose(fid);
        }
    }
    else
    {
        sprintf_s(Filename, MAX_PATH, "%ls", LogDir);
        err = fopen_s(&fid, Filename, "r");
        if (!err)
        {
	        _fgetts(LogDir, MAX_PATH, fid);
            fclose(fid);
        }
        else
        {
	        if(!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, 0, LogDir)))
            {
                 _tcscpy_s(LogDir, MAX_PATH, _T("C:\\"));
            }
        }
    }
    PathAppend(LogDir,TEXT("Sentinel3LogFile.s3l"));
    m_FileDirEdit.SetWindowTextW(LogDir);
    return TRUE;
}

// CS3LogCopyDlg message handlers

void CS3LogCopyDlg::BrowseForFolder(void)
{
    TCHAR szFilters[] = _T("S3 Log files (*.s3l)|*.s3l|All Files (*.*)|*.*||");
    // Create an Save As dialog;
	CFileDialog fileDlg(FALSE, _T("s3l"), _T("Sentinel3LogFile.s3l"),
		OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, szFilters);

    fileDlg.m_ofn.lpstrInitialDir = LogDir;
	// Display the file dialog. When user clicks OK, fileDlg.DoModal()  
	// returns IDOK. 

	INT_PTR res = fileDlg.DoModal();

    if (res == IDOK)
	{
		CString pathName = fileDlg.GetPathName();
        m_FileDirEdit.SetWindowTextW(pathName);

        //Save the selected directory as the "preferred" location for future copies
        char Filename[MAX_PATH];
	    FILE *fid;
	    int err = 0;
        sprintf_s(Filename, MAX_PATH, "%ls\\S3logdir.txt", DataLocStr);
        err = fopen_s(&fid, Filename, "w");
        if (!err)
        {
            CString test = fileDlg.GetFileName();
            CString test2 = pathName.Left(pathName.GetLength() - test.GetLength());
            fprintf(fid, "%ls", pathName.Left(pathName.GetLength() - test.GetLength()));
            fclose(fid);
        }

        _tcscpy_s(LogDir, MAX_PATH, pathName);
	}
}

void CS3LogCopyDlg::BeginCopyLogFile(void)
{
    m_CopyFileButton.EnableWindow(false);
    m_BrowseButton.EnableWindow(false);
    m_FileDirEdit.EnableWindow(false);
    m_FileCopyProgressBar.EnableWindow(true);
    m_PercentageStatic.EnableWindow(true);

    if(LogCopyInProgress)
    {
        AfxMessageBox(_T("A Log File Copy operation is already in progress. Please wait until this has finihed before trying again."));
        m_CopyFileButton.EnableWindow(true);
        m_BrowseButton.EnableWindow(true);
        m_FileDirEdit.EnableWindow(true);
        m_FileCopyProgressBar.EnableWindow(false);
        m_PercentageStatic.EnableWindow(false);
    }
    else
    {
        //Start the log file copy thread.
        m_LogFileCopyThread = AfxBeginThread(AttemptLogFileCopyThread, this, 0, 0, CREATE_SUSPENDED, NULL);
        m_LogFileCopyThread->ResumeThread();
    }
}

void CS3LogCopyDlg::CancelTx(void)
{
    if(LogCopyInProgress)
    {
        LogCopyInProgress = false;
        m_CopyFileButton.EnableWindow(false);
        m_BrowseButton.EnableWindow(false);
        m_FileDirEdit.EnableWindow(false);
        m_FileCopyProgressBar.EnableWindow(false);
        m_PercentageStatic.EnableWindow(false);
        m_CancelButton.EnableWindow(false);
        m_CancelButton.SetWindowTextW(L"Cancelling...");


        //Allow the thread 5 seconds to close itself tidily.
        DWORD closethread = WaitForSingleObject(m_LogFileCopyThread->m_hThread, 5000);
        if(closethread == WAIT_TIMEOUT)
        {
            //Thread not closed yet, therefore we will force it to close.
            if(m_LogFileCopyThread != NULL)
            {
                DWORD exitcode;
                GetExitCodeThread(m_LogFileCopyThread->m_hThread, &exitcode);
                if(exitcode == STILL_ACTIVE)
                {
                    ::TerminateThread(m_LogFileCopyThread->m_hThread, 0);
                    CloseHandle(m_LogFileCopyThread->m_hThread);
                }
                m_LogFileCopyThread->m_hThread = NULL;
                m_LogFileCopyThread = NULL;
            }
        }

        //The log file may still be open. Close it just in case
        fclose(Logfile);
    }
    //CDialog::OnOK();
    if(UpdateData(true))
    {
        DestroyWindow();	
    }
}

void CS3LogCopyDlg::PostNcDestroy()
{
    //Alert the main window that this dialog is closed.
    //This will set the pointer to this dialog to NULL - which means another can be opened
    //(The main window will not open a logcopydlg until the pointer is null)
    AfxGetMainWnd()->PostMessage(WM_LOGCOPYCLOSED, 0, 0);
    CDialog::PostNcDestroy();
    delete this;
}