// ----------------------------------------------------------------------------
// S3GDINumEdit.cpp : implementation file
//

#include "stdafx.h"

#include "S3DataModel.h"

#ifdef S3_AGENT
#include "S3Agent/S3AgentDlg.h"
#else
#include "S3ControllerDlg.h"
#endif

extern COLORREF RGBw(BYTE r, BYTE g, BYTE b);

// CS3NumEdit

IMPLEMENT_DYNAMIC(CS3NumEdit, CEdit)

CS3NumEdit::CS3NumEdit(CWnd* pParent = NULL)
{
	m_Parent = (CS3GDIScreenMain *)pParent;

	m_clrText = RGBw( 0, 0, 0 );

	m_UpdateImmediate = true;
	m_UpdateLoseFocus = true;
	// m_clrBkgnd = RGBw( 255, 200, 80 );
	// m_brBkgnd.CreateSolidBrush( m_clrBkgnd );
}

// ----------------------------------------------------------------------------

CS3NumEdit::~CS3NumEdit()
{
}


BEGIN_MESSAGE_MAP(CS3NumEdit, CEdit)
	ON_EN_CHANGE(S3GDI_NUM_EDIT, &CS3NumEdit::OnUpdate)
	ON_CONTROL_REFLECT(EN_UPDATE, &CS3NumEdit::OnEnUpdate)
	ON_CONTROL_REFLECT(EN_CHANGE, &CS3NumEdit::OnChange)
	ON_CONTROL_REFLECT(EN_KILLFOCUS, &CS3NumEdit::OnEnKillFocus)
	ON_CONTROL_REFLECT(EN_SETFOCUS, &CS3NumEdit::OnEnSetFocus)
	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()

// CS3Edit message handlers

// ----------------------------------------------------------------------------
// This is never called (?)
void CS3NumEdit::OnUpdate()
{
	CString txt;

	GetWindowText(txt);
	// m_Parent->S3GDITextSupplied(m_txt);
}

// ----------------------------------------------------------------------------

void CS3NumEdit::OnEnSetFocus()
{
	// SHSipPreference(m_hWnd, SIP_UP);
	// SipShowIM(SIPF_ON);
}

void CS3NumEdit::OnEnUpdate()
{
	// Filter out all except '0' - '9' & '.'
	int cs, ce;
	CString	txt;

	GetWindowText(txt);
	GetSel(cs, ce);

	if (cs == ce && cs > 0)
	{
		TCHAR NewChar = txt[cs - 1];
		if ((NewChar < '0' || NewChar > '9') && NewChar != '.')
		{
			CString txt2 = txt.Left(cs - 1) + txt.Right(txt.GetLength() - cs);
			SetWindowText(txt2);
			SetSel(cs - 1, ce - 1);
			return;
		}
	}
	
	if (!m_UpdateImmediate)
		return;

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

void CS3NumEdit::OnEnKillFocus()
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

void CS3NumEdit::OnChange()
{
	// SHSipPreference(m_hWnd, SIP_FORCEDOWN);

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

HBRUSH CS3NumEdit::CtlColor(CDC* pDC, UINT nCtlColor)
{
	pDC->SetTextColor(m_clrText);	// Text
	// pDC->SetBkColor(m_clrBkgnd);	// Text bkgnd
	pDC->SetBkColor(m_Parent->m_crMenuBGLight);	// Text bkgnd
	
	return m_Parent->m_hBrushMenuBGLight; // m_brBkgnd;				// Control bkgnd
}

// ----------------------------------------------------------------------------

BOOL CS3NumEdit::SetWindowPos(CRect &rect)
{
	return CWnd::SetWindowPos(
		&m_Parent->wndTop,
		rect.left,		rect.top,
		rect.Width(),	rect.Height(), SWP_SHOWWINDOW);
}

// ----------------------------------------------------------------------------

void CS3NumEdit::Edit(CRect &rect, CString &str)
{
	SetWindowPos(rect);
	SetWindowText(str);
	SetFocus();
	SetSel(0, -1);
}

// ----------------------------------------------------------------------------