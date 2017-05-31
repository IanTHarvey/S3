
// S3Agent.h : main header file for the PROJECT_NAME application
//

#pragma once

//#define NTDDI_VERSION NTDDI_WINXP
//#define _WIN32_WINNT _WIN32_WINNT_WINXP

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

#define WM_LOGCOPYCLOSED	(WM_USER + 77)
#define S3_DISCONNECT       (WM_USER + 78)
// CS3AgentApp:
// See S3Agent.cpp for the implementation of this class
//

class CS3AgentApp : public CWinApp
{
public:
	CS3AgentApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CS3AgentApp theApp;
