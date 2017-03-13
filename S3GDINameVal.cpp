
#include "stdafx.h"

#include "S3DataModel.h"
#include "S3Monitor.h"

#include "ConfigSysTab.h"
#include "ConfigRxTab.h"
#include "ConfigTxTab.h"
#include "ConfigIPTab.h"

#include "ConfigTab.h"
#include "MainTabCtrl.h"
#include "S3PrefsDlg.h"

#include "S3Edit.h"
#include "S3NumEdit.h"
#include "S3CheckBox.h"
#include "ParameterMenu.h"
#include "S3GDIScreenMain.h"
#include "S3ControllerDlg.h"

#include "NameVal.h"

// ----------------------------------------------------------------------------

NameVal::NameVal(int xref, int yref, int xright, CString lbl, CString val, bool editable)
{
	m_xref = xref;
	m_yref = yref;
	m_xright = xright;
	m_lbl = lbl + ':';
	m_val = val;
	m_editable = editable;
	
	m_Editor = NULL;
	m_EditRect.IsRectNull();
	m_EditOpenRect.IsRectNull();
}

// ----------------------------------------------------------------------------

void NameVal::Draw(HDC hdcMem, HFONT hFont, HFONT hFontB)
{
	CRect fntRc;
	fntRc.left = m_xref; 
	fntRc.top = m_yref;
	fntRc.right = m_xref + m_xright;
	fntRc.bottom = fntRc.top + 30;

	HGDIOBJ fobj = SelectObject(hdcMem, hFont);

	DrawText(hdcMem, m_lbl, -1, &fntRc, DT_LEFT);

	if (m_editable)
		SelectObject(hdcMem, hFontB);

	fntRc = m_EditRect;
	fntRc.MoveToXY(m_xref + m_xright - m_EditRect.Width(), m_yref);

	DrawText(hdcMem, m_val, -1, &fntRc, DT_RIGHT);

	SelectObject(hdcMem, fobj);
}

// ----------------------------------------------------------------------------

CRect NameVal::RectEdit(HDC hdcMem, HFONT hFont) // , HFONT hFontLarge)
{
	HGDIOBJ fobj = SelectObject(hdcMem, hFont);
	
	DrawText(hdcMem, m_val, -1, &m_EditRect, DT_CALCRECT | DT_RIGHT);

	m_EditRect.MoveToXY(m_xref + m_xright - m_EditRect.Width(), m_yref);

	/*
	if (m_editable)
	{
		HGDIOBJ fobj = SelectObject(hdcMem, hFont);
	
		DrawText(hdcMem, m_val, -1, &m_EditRect, DT_CALCRECT | DT_RIGHT);

		m_EditRect.MoveToXY(m_xref + m_xright - m_EditRect.Width(), m_yref);
	}
	*/

	return m_EditRect;
}

// ----------------------------------------------------------------------------

void NameVal::SetValue(CString val)
{
	m_val = val;
}

// ----------------------------------------------------------------------------

void NameVal::AttachEditor(HDC hdcMem, CS3NumEdit *editor)
{
	m_Editor = editor;

	HFONT hFont = HFONT(m_Editor->GetFont()); // ->GetSafeHandle();

	HGDIOBJ fobj = SelectObject(hdcMem, hFont);
	
	DrawText(hdcMem, m_val, -1, &m_EditOpenRect, DT_CALCRECT | DT_RIGHT);
	m_EditOpenRect.MoveToXY(m_xref + m_xright - m_EditOpenRect.Width(), m_yref);

	SelectObject(hdcMem, fobj);

	m_Editor->SetWindowPos(&CWnd::wndTop,
			m_EditOpenRect.left, m_EditOpenRect.top,
			m_EditOpenRect.Width(), m_EditOpenRect.Height(), SWP_HIDEWINDOW); 
}


// ----------------------------------------------------------------------------

