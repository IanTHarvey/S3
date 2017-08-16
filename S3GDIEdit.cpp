// S3Edit.cpp : implementation file
//

#include "stdafx.h"

#include "S3DataModel.h"

#ifdef S3_AGENT
#include "S3Agent/S3AgentDlg.h"
#else
#include "S3ControllerDlg.h"
#endif

extern COLORREF RGBw(BYTE r, BYTE g, BYTE b);

// CS3Edit

IMPLEMENT_DYNAMIC(CS3Edit, CEdit)

CS3Edit::CS3Edit(CWnd* pParent = NULL)
{
	m_Parent = (CS3GDIScreenMain *)pParent;

	m_clrText = RGBw(0, 0, 0);
}

CS3Edit::~CS3Edit()
{
}


BEGIN_MESSAGE_MAP(CS3Edit, CEdit)
	ON_EN_CHANGE(S3GDI_TEXT_EDIT, &CS3Edit::OnUpdate)
	ON_CONTROL_REFLECT(EN_UPDATE, &CS3Edit::OnEnUpdate)
	ON_CONTROL_REFLECT(EN_KILLFOCUS, &CS3Edit::OnEnKillFocus)
	ON_CONTROL_REFLECT(EN_SETFOCUS, &CS3Edit::OnEnSetFocus)
	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()

// CS3Edit message handlers

void CS3Edit::OnUpdate()
{
	CString txt;

	GetWindowText(txt);
	// m_Parent->S3GDITextSupplied(m_txt);
}

void CS3Edit::OnEnUpdate()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CEdit::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_UPDATE flag ORed into the lParam mask.

	// TODO:  Add your control notification handler code here
	CString txt;

	GetWindowText(txt);
	m_Parent->S3GDITextSupplied((const wchar_t *)txt);
}

void CS3Edit::OnEnKillFocus()
{
#ifdef TRIZEPS
	SipShowIM(SIPF_OFF);
#endif
}

void CS3Edit::OnEnSetFocus()
{
#ifdef TRIZEPS
	SipShowIM(SIPF_ON);
#endif
}

// ----------------------------------------------------------------------------

HBRUSH CS3Edit::CtlColor(CDC* pDC, UINT nCtlColor)
{
	pDC->SetTextColor(m_clrText);	// Text
	// pDC->SetBkColor(m_clrBkgnd);	// Text bkgnd
	pDC->SetBkColor(m_Parent->m_crMenuBGLight);	// Text bkgnd
	
	return m_Parent->m_hBrushMenuBGLight; // m_brBkgnd;		// Control bkgnd
}

// ----------------------------------------------------------------------------

BOOL CS3Edit::SetWindowPos(CRect &rect)
{
	return CWnd::SetWindowPos(
		&m_Parent->wndTop,
		rect.left,		rect.top,
		rect.Width(),	rect.Height(), SWP_SHOWWINDOW);
}

// ----------------------------------------------------------------------------

void CS3Edit::Edit(CRect &rect, CString &str)
{
	SetWindowPos(rect);
	SetWindowText(str);
	SetFocus();
	SetSel(0, -1);
}

// ----------------------------------------------------------------------------
