
// S3GDIDraw.cpp : GDI drawing implementation file
//
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


#include "windows.h"
#include "stdafx.h"
#include "afxpriv.h"
// #include "afxdialogex.h"

#include <GdiPlusEnums.h>

#include "S3DataModel.h"
// #include "S3GPIB.h"
// #include "S3I2C.h"

#include "S3Monitor.h"

#include "S3PrefsDlg.h"
#include "ConfigSysTab.h"
#include "ConfigRxTab.h"
#include "ConfigTxTab.h"
#include "ConfigIPTab.h"

#include "MainTabCtrl.h"

#include "PreviewStatic.h"
#include "S3ControllerDlg.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "mathdefs.h"

void CS3ControllerDlg::S3GDIInit()
{
	// Horizontal
	lf.lfHeight=36;
	lf.lfWidth=0;
	lf.lfEscapement=0;
	lf.lfOrientation=0;
	lf.lfWeight=FW_NORMAL;
	lf.lfItalic=0;
	lf.lfUnderline=0;
	lf.lfStrikeOut=0;
	lf.lfCharSet=ANSI_CHARSET;
	lf.lfOutPrecision=OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision=CLIP_DEFAULT_PRECIS;
	lf.lfQuality=FW_DONTCARE;
	lf.lfPitchAndFamily=DEFAULT_PITCH | FF_DONTCARE;
	// wcscpy_s(lf.lfFaceName, 32, _T("Digital Readout Thick Upright"));
	wcscpy_s(lf.lfFaceName, 32, _T("Segoe UI"));

	m_hFont = CreateFontIndirect(&lf);

	HBRUSH		hBGBrush, hLiveIPBrush, hIPBrush;
	
	HWND hWnd = ::GetDlgItem(m_hWnd, IDC_GDI_STATIC);

	CRect lpRect;

	m_GDIStatic.GetWindowRect(&lpRect);

	m_ndv = lpRect.bottom - lpRect.top;
	m_ndh = lpRect.right - lpRect.left;

	COLORREF crBG = RGB(190, 220, 255);
	COLORREF crTx = RGB(255, 140, 0);
	
	COLORREF crIP = RGB(200, 200, 200);
	COLORREF crLiveIPFill = RGB(0, 200, 0);

	COLORREF crFill = RGB(0xFF, 0, 0);
	COLORREF crBlack = RGB(0, 0, 0);

	hBGBrush = CreateSolidBrush(crBG);
	m_hBGPen = ::CreatePen(PS_SOLID, 1, crBG);
	
	hLiveIPBrush = CreateSolidBrush(crLiveIPFill);
	m_hLiveIPPen = ::CreatePen(PS_SOLID, 2, crBlack);
	
	m_hOffIPPen = ::CreatePen(PS_SOLID, 1, crBlack);
			
	m_hTxBrush = CreateSolidBrush(crTx);
	hIPBrush = CreateSolidBrush(crIP);

	m_hAlarmBrush = CreateSolidBrush(crFill);
	m_hAlarmPen = ::CreatePen(PS_SOLID, 3, crFill);

	m_hNoPen = ::CreatePen(PS_NULL, 0, crBlack);
}

void CS3ControllerDlg::S3GDIRedraw()
{
	CRect lpRect;
	m_GDIStatic.GetWindowRect(&lpRect);

	ScreenToClient(lpRect);

	HWND hWnd = ::GetDlgItem(m_hWnd, IDC_GDI_STATIC);
	// ::InvalidateRect(hWnd, lpRect, true);
	InvalidateRect(lpRect, false);

	// m_GDIStatic.UpdateWindow();
	// Invalidate();
	// UpdateWindow(); // NULL, NULL, RDW_INVALIDATE);
}

void S3DrawCircle(HDC hDC, int xo, int yo, int r)
{
	Ellipse(hDC, xo - r, yo - r, xo + r, yo + r);
}

void S3DrawTx(char Rx, char Tx)
{
	// Tie in to S3DataModel
}

void CS3ControllerDlg::S3DrawGDITx()
{
	unsigned char	TxType;
	char			ActiveIP;

	ActiveIP = S3GetActiveInput(m_CurrentNode[0] - 1, m_CurrentNode[1] - 1);
	TxType = S3GetTxType(m_CurrentNode[0] - 1, m_CurrentNode[1] - 1);

	HDC hDC;
	PAINTSTRUCT	Ps;
	HBRUSH		hBGBrush, hLiveIPBrush, hIPBrush, hAlarmBrush;

	HWND hWnd = ::GetDlgItem(m_hWnd, IDC_GDI_STATIC);

	COLORREF crBG = RGB(190, 220, 255);
	COLORREF crTx = RGB(255, 140, 0);
	
	COLORREF crIP = RGB(200, 200, 200);
	COLORREF crLiveIPFill = RGB(0, 200, 0);

	COLORREF crFill = RGB(0xFF, 0, 0);
	COLORREF crBlack = RGB(0, 0, 0);

	hBGBrush = CreateSolidBrush(crBG);
	
	hLiveIPBrush = CreateSolidBrush(crLiveIPFill);
			
	hIPBrush = CreateSolidBrush(crIP);
	hAlarmBrush = CreateSolidBrush(crFill);

	hDC = ::BeginPaint(hWnd, &Ps);
	SelectObject(hDC, hBGBrush);

	Rectangle(hDC, 0, 0, m_ndh, m_ndv);

	int	xref = m_ndh / 2;
	int	yref = m_ndv / 2;

	int	dia = 60;
	int radTx = dia / 2;

	int start = -40;
	int div = (180) / (8 - 1); // Fenceposts!
	int radIP = 7;
	double posIP = 1.3 * radTx;

	SelectObject(hDC, hIPBrush);

	if (TxType == S3_Tx8)
	{
		for(int i = 0; i < 8; i++)
		{
			double th, xrad, yrad;

			th = start + i * div;

			xrad = posIP * cos(M_PI * th / 180.0);
			yrad = posIP * sin(M_PI * th / 180.0);


			if (i == ActiveIP)
			{
				SelectObject(hDC, m_hLiveIPPen);
				SelectObject(hDC, hLiveIPBrush);
			}
			else
			{
				SelectObject(hDC, m_hOffIPPen);
				SelectObject(hDC, hIPBrush);
			}
			
			if (S3GetIPAlarms(m_CurrentNode[0] - 1, m_CurrentNode[1] - 1, i))
			{
				SelectObject(hDC, m_hAlarmPen);
			}

			S3DrawCircle(hDC, xref - (int)xrad, yref + (int)yrad, radIP);
		}
	}
	else if (TxType == S3_Tx1)
	{
		SelectObject(hDC, m_hLiveIPPen);
		SelectObject(hDC, hLiveIPBrush);

		S3DrawCircle(hDC, xref, yref + (int)(1.3 * radTx), 7);
	}

	SelectObject(hDC, m_hOffIPPen);
	SelectObject(hDC, hIPBrush);

	MoveToEx(hDC, xref, 0, NULL);
	LineTo(hDC, xref, yref);

	SelectObject(hDC, m_hTxBrush);
	S3DrawCircle(hDC, xref, yref, radTx);

	// Applies to font only
	SetBkMode(hDC, TRANSPARENT);
	SetTextColor(hDC, RGB(50, 50, 50));

	RECT fntRc;
	fntRc.left = xref - 7;
	fntRc.top = yref - 20;
	fntRc.right = xref + 13;
	fntRc.bottom = yref + 10;

	SelectObject(hDC, m_hFont);
	CString str;
	str.Format(_T("%d"), m_CurrentNode[1]);
	DrawText(hDC, str, -1, &fntRc, DT_LEFT);

	::EndPaint(hWnd, &Ps);
}