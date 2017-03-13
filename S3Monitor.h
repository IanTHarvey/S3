
// S3Monitor.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CS3MonitorApp:
// See S3Monitor.cpp for the implementation of this class
//

class CS3MonitorApp : public CWinApp
{
public:
	CS3MonitorApp();

	int ReadForm();
	int WriteForm();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CS3MonitorApp theApp;