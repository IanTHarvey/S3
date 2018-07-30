// ----------------------------------------------------------------------------
// S3GDIListWindow.cpp : implementation file
//
// Single instance (m_NumericPad) gets attached to a CNumEdit when popped up.
//
// TODO: Set permissable characters
//
// ----------------------------------------------------------------------------

#include "stdafx.h"

#include "S3DataModel.h"

#ifdef S3_AGENT
#include "S3Agent/S3AgentDlg.h"
#else
#include "S3ControllerDlg.h"
#endif
#include "S3GDIInfoPopup.h"
#include "S3GDIListWindow.h"

extern COLORREF RGBw(BYTE r, BYTE g, BYTE b);

// Vertices for <|X|
extern POINT DeleteKey[9];
extern POINT DeleteKeyTrans[9];

// CS3ListWindow

IMPLEMENT_DYNAMIC(CS3ListWindow, CEdit)

// ----------------------------------------------------------------------------

CS3ListWindow::CS3ListWindow(CWnd* pParent = NULL)
{
	m_Parent = (CS3GDIScreenMain *)pParent;

	// m_clrText = m_Parent->m_crMenuTxtActive;

	m_UpdateImmediate = true;
	m_UpdateLoseFocus = true;

	m_PoppedUp = false;

	m_Constraints = 0;
	m_MaxChars = 0;

	char k = 0;
	for(char i = 0; i < 5; i++)
	{
		for(char j = 0; j < 3; j++)
		{
			m_RectButtons[k].left = S3_LW_BORDER +
				j * (S3_LW_KEY_W + S3_LW_KEY_SP);
			m_RectButtons[k].right = m_RectButtons[k].left + S3_LW_KEY_W;

			m_RectButtons[k].top = S3_LW_BORDER + // S3_LW_DISP_H +
				i * (S3_LW_KEY_H + S3_LW_KEY_SP);

			m_RectButtons[k].bottom = m_RectButtons[k].top + S3_LW_KEY_H;

			k++;
		}
	}

	m_RectPad.left = m_RectPad.top = 0;

	m_RectPad.right = m_RectPad.left +
		2 * S3_LW_KEY_SP +
		3 * S3_LW_KEY_W +
		2 * S3_LW_BORDER;

	m_RectPad.bottom = m_RectPad.top +
		4 * S3_LW_KEY_SP +
		5 * S3_LW_KEY_H + 
		2 * S3_LW_BORDER;

	// Get resources
	m_Parent->GetPopUpColours(m_hBrushBG, m_hBrushButton, m_hBrushDispBG);
	m_Parent->GetPopUpFont(m_hFont, m_clrText, m_clrBG);

}

CS3ListWindow::~CS3ListWindow()
{

}


BEGIN_MESSAGE_MAP(CS3ListWindow, CEdit)
	// ON_EN_CHANGE(S3GDI_NUM_EDIT, &CS3ListWindow::OnUpdate)
	// ON_CONTROL_REFLECT(EN_UPDATE, &CS3ListWindow::OnEnUpdate)
	// ON_CONTROL_REFLECT(EN_KILLFOCUS, &CS3ListWindow::OnEnKillFocus)
	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()

// CS3Edit message handlers

// ----------------------------------------------------------------------------
// This is never called (?)
void CS3ListWindow::OnUpdate()
{
	CString txt;

	GetWindowText(txt);
	// m_Parent->S3GDITextSupplied(m_txt);
}

// ----------------------------------------------------------------------------

void CS3ListWindow::OnEnUpdate()
{
	if (!m_UpdateImmediate)
		return;

	CString txt;

	GetWindowText(txt);

	CString	sTest;
    TCHAR *pTail = NULL;

    sTest = txt + '0';		// What? Figure it out.
    _tcstod(sTest, &pTail);	// Is it a real #

    if (*pTail == '\0')
    {
        m_sVerified = txt;  // Save it
    }
    else
    {
        long nLen = m_sVerified.GetLength();
        SetWindowText(m_sVerified);    // Reset it.
        SetSel(nLen, nLen);
    }

	m_Parent->S3GDITextSupplied((LPCWSTR)txt);
}

// ----------------------------------------------------------------------------

void CS3ListWindow::OnEnKillFocus()
{
	if (!m_UpdateLoseFocus)
		return;

	CString txt;

	GetWindowText(txt);

	CString	sTest;
    TCHAR *pTail = NULL;

    sTest = txt + '0';		// What? Figure it out.
    _tcstod(sTest, &pTail);	// Is it a real #

    if (*pTail == '\0')
    {
        m_sVerified = txt;  // Save it
    }
    else
    {
        long nLen = m_sVerified.GetLength();
        SetWindowText(m_sVerified);    // Reset it.
        SetSel(nLen, nLen);
    }

	m_Parent->S3GDITextSupplied(txt);
}
// ----------------------------------------------------------------------------

void CS3ListWindow::OnChange()
{
/*
	CString sText, sTest;
    TCHAR *pTail = NULL;

    GetWindowText(sText);
    sTest = sText + "0";            // What? Figure it out.
    _tcstod(sTest, &pTail);         // Is it a real #

    if (*pTail == '\0')
    {
        m_sVerified = sText;           // Save it
    }
    else
    {
        long nLen = m_sVerified.GetLength();
        SetWindowText(m_sVerified);    // Reset it.
        SetSel(nLen, nLen);
    }
	*/
}

// ----------------------------------------------------------------------------
// Edit panel colours
HBRUSH CS3ListWindow::CtlColor(CDC* pDC, UINT nCtlColor)
{
	pDC->SetTextColor(m_clrText);	// Text
	pDC->SetBkColor(m_clrBG);	// Text bkgnd
	
	return m_Parent->m_hBrushMenuBGLight; // m_brBkgnd;				// Control bkgnd
}

// ----------------------------------------------------------------------------

void CS3ListWindow::Draw()
{
	if (!m_PoppedUp)
		return;

	HGDIOBJ obj = SelectObject(m_HDC, m_hBrushBG);

	// S3_RECT(m_HDC, m_RectPad);
	S3_RECTR(m_HDC, m_RectPad, 5);

//	SelectObject(m_HDC, m_hBrushDispBG);
//	S3_RECT(m_HDC, m_RectDisplay);

	SelectObject(m_HDC, m_hBrushButton);
	SetTextColor(m_HDC, m_clrText);
	
	CString str;

	for(char j = 0; j < S3_LW_N_KEYS; j++)
	{
		// S3_RECT(m_HDC, m_RectButtons[j]);
		S3_RECTR(m_HDC, m_RectButtons[j], 5);

		if (j < 9)
		{
			str.Format(_T("%d"), j + 1);
			DrawText(m_HDC, str, -1, & m_RectButtons[j], DT_CENTER);
		}
		else if (j == S3_LW_PLUS_MINUS_KEY)
		{
			if (!(m_Constraints & S3_LW_POSITIVE))
				DrawText(m_HDC, _T("\u00b1"), -1, & m_RectButtons[j], DT_CENTER);
		}
		else if (j == S3_LW_ZERO_KEY)
			DrawText(m_HDC, _T("0"), -1, & m_RectButtons[j], DT_CENTER);
		else if (j == S3_LW_DECIMAL_KEY)
		{
			if (!(m_Constraints & S3_LW_INTEGER))
				DrawText(m_HDC, _T("."), -1, & m_RectButtons[j], DT_CENTER);
		}
		else if (j == S3_LW_DELETE_KEY)
		{
			// Not in main character set and not found on iPAN - found
			// on W8, maybe in Segoe symbol font??
			// DrawText(m_HDC, _T("\u232b"), -1, & m_RectButtons[j], DT_CENTER);
			HGDIOBJ o = SelectObject(m_HDC, GetStockObject(WHITE_PEN));
			Polyline(m_HDC, DeleteKeyTrans, 9);
			SelectObject(m_HDC, o);
		}
		else if (j == S3_LW_LEFT_KEY)
			DrawText(m_HDC, _T("\u2190"), -1, & m_RectButtons[j], DT_CENTER);
		else if (j == S3_LW_RIGHT_KEY)
			DrawText(m_HDC, _T("\u2192"), -1, & m_RectButtons[j], DT_CENTER);
	}

	SelectObject(m_HDC, obj);
}

// ----------------------------------------------------------------------------

void CS3ListWindow::PopUp(HDC hdc, int xref, int yref,
						 unsigned char constraints, 
						 unsigned char max_chars)
{
	m_Constraints = constraints;
	m_MaxChars = max_chars;

	if (m_PoppedUp)
		return;

	// SHSipPreference(m_hWnd, SIP_FORCEDOWN);
	
	m_HDC = hdc;

	m_PoppedUp = true;

	m_xref = xref;
	m_yref = yref;

	CString str;

	// TODO: Check boundaries
	CRect RectEditor;

	m_Parent->ScreenToClient(RectEditor);

	if (RectEditor.right + m_RectPad.Width() < m_Parent->S3GDIRectScreen().right &&
		(m_yref + m_RectPad.Height()) < m_Parent->S3GDIRectScreen().bottom)
	{
		// Just a move to the right required...
		m_xref = RectEditor.right + S3_LW_BORDER;
	}
	else
	{
		if (RectEditor.right + m_RectPad.Width() > m_Parent->S3GDIRectScreen().right)
		{
			// Make x fit
			m_xref -= RectEditor.right + m_RectPad.Width() - m_Parent->S3GDIRectScreen().right;

			// Move below if room
			if ((RectEditor.bottom + m_RectPad.Height()) < m_Parent->S3GDIRectScreen().bottom)
				m_yref = RectEditor.bottom + S3_LW_BORDER;
		}
		else
		{
			// TODO: Consider the other possibilties...
			if (m_yref + m_RectPad.Height() > m_Parent->S3GDIRectScreen().bottom)
				m_yref = m_Parent->S3GDIRectScreen().bottom - m_RectPad.Height();
		}
	}

	m_RectPad.OffsetRect(m_xref, m_yref);

	for(char i = 0; i < S3_LW_N_KEYS; i++)
	{
		m_RectButtons[i].OffsetRect(m_xref, m_yref);
	}

	for(char i = 0; i < 9; i++)
	{
		DeleteKeyTrans[i].x = DeleteKey[i].x +
			m_RectButtons[S3_LW_DELETE_KEY].left;
		DeleteKeyTrans[i].y = DeleteKey[i].y +
			m_RectButtons[S3_LW_DELETE_KEY].top;
	}
}

// ----------------------------------------------------------------------------

void CS3ListWindow::PopDown()
{
	if (!m_PoppedUp)
		return;
		
	m_PoppedUp = false;

	// TODO: Is this actually any better than recalculating layout on-the-fly?

	m_RectPad.OffsetRect(-m_xref, -m_yref);
	// m_RectDisplay.OffsetRect(-m_xref, -m_yref);

	for(char j = 0; j < S3_LW_N_KEYS; j++)
	{
		m_RectButtons[j].OffsetRect(-m_xref, -m_yref);
	}

	// This will crash if Create() not called - as asserted must be a
	// window - which the base CEdit class will not be. As the base
	// is not actually shown:
	// a) Is there any point in hiding the window? 
	// b) Is there any point in basing this class at all?
	ShowWindow(SW_HIDE);
}

// ----------------------------------------------------------------------------

char CS3ListWindow::Find(POINT pt)
{
	if (!m_PoppedUp)
		return -1;

	// Was popped up
	return -3;
}

// ----------------------------------------------------------------------------
