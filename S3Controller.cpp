// ----------------------------------------------------------------------------
// S3Controller.cpp : Defines the class behaviors for the application.
//

#ifndef TRIZEPS
#include "S3ControllerX86/targetver.h"
#else
#define WINVER _WIN32_WCE
#include <ceconfig.h>
#endif

#include <afxsock.h>		// MFC socket extensions
#include "afxpriv.h"



#include "S3DataModel.h"
#include "S3ControllerDlg.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif


// CS3ControllerApp

BEGIN_MESSAGE_MAP(CS3ControllerApp, CWinApp)
END_MESSAGE_MAP()


// CS3ControllerApp construction
CS3ControllerApp::CS3ControllerApp()
	: CWinApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CS3ControllerApp object
CS3ControllerApp theApp;

// CS3ControllerApp initialization

BOOL CS3ControllerApp::InitInstance()
{

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

/*
	DWORD LaunchNo = (DWORD)_wtoi(m_lpCmdLine);

	FILE *fid;

	int err = fopen_s(&fid, "\\Flashdisk\\args.txt", "w");

	if (!err)
	{
		fwprintf(fid, _T("\n->%s<- (%d)\n"), m_lpCmdLine, LaunchNo);
		fclose(fid);
	}
*/

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("PPM-Sentinel3"));

	CS3ControllerDlg dlg;
	// CS3CtrlDlg guidlg;

	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
