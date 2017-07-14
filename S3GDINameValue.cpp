// ----------------------------------------------------------------------------
// S3GDINameValue.cpp
//
// Label-value pair class. The value rectangle (m_EditRect) is available for
// selection detection (eg pop-up menu) or for over-laying with another control
// (eg text edit control).
//
// Label is left-justified to xref, value is right-justified to xright.
//
// If editable, a different font can be used for the value, and this used to
// provide the editor with an overlay rectangle (m_EditOpenRect).
//
// -----------------------------------------------------------------------------

#pragma once

#include "stdafx.h"

#include "S3DataModel.h"

#ifdef S3_AGENT
#include "S3Agent/S3AgentDlg.h"
#else
#include "S3ControllerDlg.h"
#endif

// ----------------------------------------------------------------------------
// String val should provide a hint to the size of actual values for future use.

CS3NameValue::CS3NameValue(int xref, int yref, int xright,
						   CString lbl, CString val, bool editable)
{
	m_Parent = NULL;
	m_Alarm = false;
	m_xref = xref;			// Left justification
	m_yref = yref;
	m_xright = xright;		// Right justification
	m_lbl = lbl + ':';
	m_val = val;
	m_Editable = editable;	// If editable, use a different font/face for value.
	
	m_Editor = NULL;
	m_EditRect.SetRectEmpty();
	m_EditOpenRect.SetRectEmpty();
}

// ----------------------------------------------------------------------------
// String val should provide a hint to the size of actual values for future use.

CS3NameValue::CS3NameValue(
#ifdef S3_AGENT
						   CS3AgentDlg *Parent,
#else
						   CS3ControllerDlg *Parent,
#endif						   
						   int xref, int yref, int xright,
						   CString lbl, CString val, bool editable)
{
	m_Parent = Parent;
	m_Alarm = false;
	m_xref = xref;			// Left justification
	m_yref = yref;
	m_xright = xright;		// Right justification
	m_lbl = lbl + ':';
	m_val = val;
	m_Editable = editable;	// If editable, use a different font/face for value.
	
	m_Editor = NULL;
	m_EditRect.SetRectEmpty();
	m_EditOpenRect.SetRectEmpty();
}

// ----------------------------------------------------------------------------
// FontB for drawing the editable value if required.

void CS3NameValue::Draw(HDC hdc, HFONT hFont, HFONT hFontB)
{
	CRect fntRc;
	fntRc.left = m_xref + NV_LMARGIN; 
	fntRc.top = m_yref;
	fntRc.right = m_xref + m_xright;
	fntRc.bottom = fntRc.top + 30;

	HGDIOBJ fobj = SelectObject(hdc, hFont);
	COLORREF fcol;
	if (m_Parent && m_Parent->m_AnimateState && m_Alarm)
		fcol = SetTextColor(hdc, RGB(255, 0, 0));
	else
		fcol = SetTextColor(hdc, RGB(0, 0, 0));

	DrawText(hdc, m_lbl, -1, &fntRc, DT_LEFT);

	if (m_Editable)
		SelectObject(hdc, hFontB);

	DrawText(hdc, m_val, -1, &m_EditRect, DT_RIGHT);

	SetTextColor(hdc, fcol);
	SelectObject(hdc, fobj);
}

// ----------------------------------------------------------------------------
// Update and return the editable (and clickable) rectangle - requires the
// font to 'draw' the text.

CRect CS3NameValue::RectEdit(HDC hdc, HFONT hFont) // , HFONT hFontLarge)
{
	HGDIOBJ fobj = SelectObject(hdc, hFont);
	
	DrawText(hdc, m_val, -1, &m_EditRect, DT_CALCRECT | DT_RIGHT);

	m_EditRect.MoveToXY(m_xref + m_xright - m_EditRect.Width() - RMARGIN, m_yref);

	return m_EditRect;
}

// ----------------------------------------------------------------------------
// 

void CS3NameValue::SetValue(CString val)
{
	m_val = val;
}

// ----------------------------------------------------------------------------
// Provide a rectangle for the editor window which may use a bigger font than
// for just displaying.

void CS3NameValue::AttachEditor(HDC hdc, CEdit *editor)
{
	m_Editor = editor;

	HFONT hFont =  (HFONT)m_Editor->GetFont()->GetSafeHandle();

	HGDIOBJ fobj = SelectObject(hdc, hFont);
	
	DrawText(hdc, m_val, -1, &m_EditOpenRect, DT_CALCRECT | DT_RIGHT);
	m_EditOpenRect.MoveToXY(m_xref + m_xright - m_EditOpenRect.Width(), m_yref);

	SelectObject(hdc, fobj);

	m_Editor->SetWindowPos(&CWnd::wndTop,
			m_EditOpenRect.left, m_EditOpenRect.top,
			m_EditOpenRect.Width(), m_EditOpenRect.Height(), SWP_HIDEWINDOW); 
}

// ----------------------------------------------------------------------------

void CS3NameValue::SetEditable(bool editable)
{
	m_Editable = editable;
}

// ----------------------------------------------------------------------------

char CS3NameValue::FindSelect(POINT p)
{
	if (!m_Editable)
		return 0;

	if (m_EditRect.PtInRect(p))
		return 1;

	return 0;
}

// ----------------------------------------------------------------------------