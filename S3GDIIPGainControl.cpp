// ----------------------------------------------------------------------------
// Tx gain control drawing

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

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

CRect	m_IPGainRect, m_IPGainRectText, m_IPGainRectText2,
		m_IPGainRectUp1, m_IPGainRectUp2, m_IPGainRectDown1, m_IPGainRectDown2,
		m_IPGainRectIncr1, m_IPGainRectIncr2;

#define S3_GAIN_SMALL_INC_INSET_X	16
#define S3_GAIN_SMALL_INC_INSET_Y	10

#define S3_GAIN_LARGE_INC_INSET_X	8
#define S3_GAIN_LARGE_INC_INSET_Y	5

#define S3_GAIN_CTRL_W				100
#define S3_GAIN_CTRL_H				120

// ----------------------------------------------------------------------------

// Xref is the LHS of the IP column
void CS3GDIScreenMain::S3InitGDIIPGain(char Rx, char Tx, char IP,
	int xref, int yref)
{
	xref -= S3_GAIN_CTRL_W - 10; // Push left but leave a bit of overlap

	m_IPGainRect.left = xref;
	m_IPGainRect.top = yref;
	m_IPGainRect.right = xref + S3_GAIN_CTRL_W;
	m_IPGainRect.bottom = yref + S3_GAIN_CTRL_H;

	if (m_IPGainRect.right > m_RectScreen.right)
		m_IPGainRect.OffsetRect(m_RectScreen.Width() - m_IPGainRect.right, 0);

	m_IPGainRectText = m_IPGainRect;
	m_IPGainRectText.top = m_IPGainRect.top;
	m_IPGainRectText.bottom = m_IPGainRectText.top + 40;
	
	m_IPGainRectText2 = m_IPGainRectText;

	m_IPGainRectText.right = m_IPGainRectText.right - 40;
	m_IPGainRectText2.left = m_IPGainRectText.right;
	
	m_IPGainRectUp1.left = m_IPGainRect.left;
	m_IPGainRectUp1.top = m_IPGainRectText.bottom;
	m_IPGainRectUp1.right = m_IPGainRectUp1.left + m_IPGainRect.Width() / 2;
	m_IPGainRectUp1.bottom = m_IPGainRectUp1.top + 30;

	m_IPGainRectUp2 = m_IPGainRectUp1;
	m_IPGainRectUp2.left = m_IPGainRectUp1.right;
	m_IPGainRectUp2.right = m_IPGainRect.right;

	m_IPGainRectIncr1 = m_IPGainRectUp1;
	m_IPGainRectIncr1.top = m_IPGainRectUp1.bottom;
	m_IPGainRectIncr1.bottom = m_IPGainRectIncr1.top + 20;

	m_IPGainRectIncr2 = m_IPGainRectUp2;
	m_IPGainRectIncr2.top = m_IPGainRectUp2.bottom;
	m_IPGainRectIncr2.bottom = m_IPGainRectIncr2.top + 20;

	m_IPGainRectDown1 = m_IPGainRectIncr1;
	m_IPGainRectDown1.top = m_IPGainRectIncr1.bottom;
	m_IPGainRectDown1.bottom = m_IPGainRect.bottom;

	m_IPGainRectDown2 = m_IPGainRectIncr2;
	m_IPGainRectDown2.top = m_IPGainRectIncr2.bottom;
	m_IPGainRectDown2.bottom = m_IPGainRect.bottom;

	// ---------------------------------------------------------------------------

	mIPGainTriangleUp1[0].x = m_IPGainRectUp1.left + S3_GAIN_SMALL_INC_INSET_X;
	mIPGainTriangleUp1[0].y = m_IPGainRectUp1.bottom - S3_GAIN_SMALL_INC_INSET_Y;

	mIPGainTriangleUp1[1].x = m_IPGainRectUp1.right - S3_GAIN_SMALL_INC_INSET_X;
	mIPGainTriangleUp1[1].y = m_IPGainRectUp1.bottom - S3_GAIN_SMALL_INC_INSET_Y;

	mIPGainTriangleUp1[2].x = (m_IPGainRectUp1.left + m_IPGainRectUp1.right) / 2;
	mIPGainTriangleUp1[2].y = m_IPGainRectUp1.top + S3_GAIN_SMALL_INC_INSET_Y;

	mIPGainTriangleDown1[0].x = m_IPGainRectDown1.left + S3_GAIN_SMALL_INC_INSET_X;
	mIPGainTriangleDown1[0].y = m_IPGainRectDown1.top + S3_GAIN_SMALL_INC_INSET_Y;

	mIPGainTriangleDown1[1].x = m_IPGainRectDown1.right - S3_GAIN_SMALL_INC_INSET_X;
	mIPGainTriangleDown1[1].y = m_IPGainRectDown1.top + S3_GAIN_SMALL_INC_INSET_Y;

	mIPGainTriangleDown1[2].x = (m_IPGainRectDown1.left + m_IPGainRectDown1.right) / 2;
	mIPGainTriangleDown1[2].y = m_IPGainRectDown1.bottom - S3_GAIN_SMALL_INC_INSET_Y;

	// ---------------------------------------------------------------------------

	mIPGainTriangleUp2[0].x = m_IPGainRectUp2.left + S3_GAIN_LARGE_INC_INSET_X;
	mIPGainTriangleUp2[0].y = m_IPGainRectUp2.bottom - S3_GAIN_LARGE_INC_INSET_Y;

	mIPGainTriangleUp2[1].x = m_IPGainRectUp2.right - S3_GAIN_LARGE_INC_INSET_X;
	mIPGainTriangleUp2[1].y = m_IPGainRectUp2.bottom - S3_GAIN_LARGE_INC_INSET_Y;

	mIPGainTriangleUp2[2].x = (m_IPGainRectUp2.left + m_IPGainRectUp2.right) / 2;
	mIPGainTriangleUp2[2].y = m_IPGainRectUp2.top + 3;

	mIPGainTriangleDown2[0].x = m_IPGainRectDown2.left + S3_GAIN_LARGE_INC_INSET_X;
	mIPGainTriangleDown2[0].y = m_IPGainRectDown2.top + S3_GAIN_LARGE_INC_INSET_Y;

	mIPGainTriangleDown2[1].x = m_IPGainRectDown2.right - S3_GAIN_LARGE_INC_INSET_X;
	mIPGainTriangleDown2[1].y = m_IPGainRectDown2.top + S3_GAIN_LARGE_INC_INSET_Y;

	mIPGainTriangleDown2[2].x = (m_IPGainRectDown2.left + m_IPGainRectDown2.right) / 2;
	mIPGainTriangleDown2[2].y = m_IPGainRectDown2.bottom - S3_GAIN_LARGE_INC_INSET_Y;

	m_IPGainIsUp = true;
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDIIPGain(char Rx, char Tx, char IP)
{
	if (!m_IPGainIsUp)
		return;

	SelectObject(m_HDC, m_hBrushMenuBGLight);
	SelectObject(m_HDC, m_hPenNone);
	COLORREF cr = SetTextColor(m_HDC, m_crWhite);

	RoundRect(m_HDC, m_IPGainRect.left - 4, m_IPGainRect.top - 4,
		m_IPGainRect.right + 4, m_IPGainRect.bottom + 4, 10, 10);

	SelectObject(m_HDC, m_hBrushMenuBGMed);
	RoundRect(m_HDC, m_IPGainRectUp1.left, m_IPGainRectUp1.top,
		m_IPGainRectUp1.right - 1, m_IPGainRectUp1.bottom - 1, 5, 5);

	RoundRect(m_HDC, m_IPGainRectDown1.left, m_IPGainRectDown1.top + 1,
		m_IPGainRectDown1.right - 1, m_IPGainRectDown1.bottom, 5, 5);

	RoundRect(m_HDC, m_IPGainRectUp2.left + 1, m_IPGainRectUp2.top,
		m_IPGainRectUp2.right, m_IPGainRectUp2.bottom - 1, 5, 5);

	RoundRect(m_HDC, m_IPGainRectDown2.left + 1, m_IPGainRectDown2.top + 1,
		m_IPGainRectDown2.right, m_IPGainRectDown2.bottom, 5, 5);

	SelectObject(m_HDC, m_hBrushMenuBGLight);
	Polygon(m_HDC, mIPGainTriangleUp1, 3);
	Polygon(m_HDC, mIPGainTriangleUp2, 3);
	Polygon(m_HDC, mIPGainTriangleDown1, 3);
	Polygon(m_HDC, mIPGainTriangleDown2, 3);

	CString str;
	int gain = S3IPGetGain(Rx, Tx, IP);
	if (gain == 0)
		str = _T("-0");
	else str.Format(_T("%+d"), gain);

	SelectObject(m_HDC, m_hFontL);
	DrawText(m_HDC, str, -1, &m_IPGainRectText, DT_RIGHT);

	str = "dB";
	DrawText(m_HDC, str, -1, &m_IPGainRectText2, DT_LEFT);

	// Not required with medium font
	//m_IPGainRectUp1.top -= 9;
	//m_IPGainRectDown1.top -= 5;
	//m_IPGainRectUp2.top -= 9;
	//m_IPGainRectDown2.top -= 5;

	m_IPGainRectIncr1.top -= 7;
	m_IPGainRectIncr2.top -= 7;

	SelectObject(m_HDC, m_hFontM);

	/*
	DrawText(m_HDC, _T("+"), -1, &m_IPGainRectUp1, DT_CENTER);
	DrawText(m_HDC, _T("+"), -1, &m_IPGainRectUp2, DT_CENTER);

	DrawText(m_HDC, _T("-"), -1, &m_IPGainRectDown1, DT_CENTER);
	DrawText(m_HDC, _T("-"), -1, &m_IPGainRectDown2, DT_CENTER);
	*/

	DrawText(m_HDC, _T("1"), -1, &m_IPGainRectIncr1, DT_CENTER);
	DrawText(m_HDC, _T("5"), -1, &m_IPGainRectIncr2, DT_CENTER);

	//m_IPGainRectUp1.top += 9;
	//m_IPGainRectDown1.top += 5;
	//m_IPGainRectUp2.top += 9;
	//m_IPGainRectDown2.top += 5;

	m_IPGainRectIncr1.top += 7;
	m_IPGainRectIncr2.top += 7;

	m_NumericPad->Draw();

	SetTextColor(m_HDC, cr);

}

// ----------------------------------------------------------------------------

int CS3GDIScreenMain::S3GDIIPGainProcess(POINT p)
{
	if (!m_IPGainIsUp)
		return 0;

	if (m_NumericPad->Find(p) >= 0)
		return 1;
	else
		m_NumericPad->PopDown();

	if (m_IPGainRect.PtInRect(p))
	{
		// Process up/down...
		char Rx, Tx, IP;

		S3GetSelected(&Rx, &Tx, &IP);
		
		char	gain = S3IPGetGain(Rx, Tx, IP);
		
		if (m_IPGainRectUp1.PtInRect(p))
		{	
			m_GDIGainEdit->ShowWindow(false);
			S3SetGain(Rx, Tx, IP, gain + 1);
		}
		else if (m_IPGainRectDown1.PtInRect(p))
		{
			m_GDIGainEdit->ShowWindow(false);
			S3SetGain(Rx, Tx, IP, gain - 1);
		}
		else if (m_IPGainRectUp2.PtInRect(p))
		{	
			m_GDIGainEdit->ShowWindow(false);
			S3SetGain(Rx, Tx, IP, gain + 5);
		}
		else if (m_IPGainRectDown2.PtInRect(p))
		{
			m_GDIGainEdit->ShowWindow(false);
			S3SetGain(Rx, Tx, IP, gain - 5);
		}
		else if (m_IPGainRectText.PtInRect(p))
		{
			m_GDIGainEdit->SetFont(&m_cFontL);

			m_GDIGainEdit->SetWindowPos(&this->wndTop,
				m_IPGainRectText.left, m_IPGainRectText.top,
				m_IPGainRectText.Width(), m_IPGainRectText.Height(),
				SWP_SHOWWINDOW);

			CString str;
			if (gain == 0)
				str = _T("-0");
			else str.Format(_T("%+d"), gain);
			m_GDIGainEdit->SetWindowText(str);
			m_GDIGainEdit->SetFocus();
			m_GDIGainEdit->SetSel(0, -1); // select all text and move cursor at the end
			// m_GDIGainEdit->SetSel(-1); //  remove selection

			m_NumericPad->PopUp(m_HDC, p.x, p.y, m_GDIGainEdit,
				S3_NP_INTEGER, 5);

		}

		return 1;
	}

	m_GDIGainEdit->SetFont(&m_cFontS);
	m_GDIGainEdit->ShowWindow(false);
	m_IPGainIsUp = false;

	// m_NumericPad->PopDown();

	return 2;
}

// ----------------------------------------------------------------------------
// Called on screen change only

void CS3GDIScreenMain::S3GDIIPGainClose()
{
	m_IPGainIsUp = false;
	m_GDIGainEdit->ShowWindow(false);

	m_NumericPad->PopDown();
}

// ----------------------------------------------------------------------------