// S3Controller.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

// #ifdef STANDARDSHELL_UI_MODEL
#include "resource.h"
// #endif

// CS3ControllerApp:
// See S3Controller.cpp for the implementation of this class
//

class CS3ControllerApp : public CWinApp
{
public:
	CS3ControllerApp();
	
// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CS3ControllerApp theApp;
