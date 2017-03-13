// ----------------------------------------------------------------------------
// Charger status displays
//
// m_Screen == S3_CH_SCREEN
// ----------------------------------------------------------------------------


#ifndef TRIZEPS
#include "S3ControllerX86/targetver.h"
#else
#define WINVER _WIN32_WCE
#include <ceconfig.h>
#endif

#include "afxpriv.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "mathdefs.h"

#include "S3DataModel.h"
#include "S3ControllerDlg.h"

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDISleepScreen(void)
{
	RECT	fntRc;
	CString str;

	SelectObject(m_HDC, m_hPenNone);
	SelectObject(m_HDC, m_hBrushSleep);
	SelectObject(m_HDC, m_hFontL);
	COLORREF cr = SetTextColor(m_HDC, m_crWhite);

	Rectangle(m_HDC,	m_RectScreen.left,	m_RectScreen.top,
						m_RectScreen.right,	m_RectScreen.bottom);

	// Battery info
	//fntRc.left = xref - 50;
	//fntRc.top = yref + 50;
	//fntRc.right = xref + 180;
	//fntRc.bottom = yref + 220;

	str.Format(_T("Sleep"));
	fntRc = m_RectScreen;

	DrawText(m_HDC, str, -1, &fntRc, DT_CENTER);

	SetTextColor(m_HDC, cr);

	// S3DrawGDIBackButton();
}

// ----------------------------------------------------------------------------

