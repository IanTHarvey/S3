// ----------------------------------------------------------------------------
// S3GDIKeyboard.cpp : implementation file
//

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

#include "S3USBVCP.h"

#include "S3Edit.h"
#include "S3NumEdit.h"
#include "S3NumberPad.h"
#include "S3GDIKeyboard.h"
#include "S3CheckBox.h"
#include "ParameterMenu.h"
#include "S3GDIScreenMain.h"
#include "S3ControllerDlg.h"

extern COLORREF RGBw(BYTE r, BYTE g, BYTE b);

#define S3_RECT(A, B)		Rectangle(A, B.left, B.top, B.right, B.bottom)
#define S3_RECTR(A, B, C)	RoundRect(A, B.left, B.top, B.right, B.bottom, C, C)

// See S3NumberPad
extern POINT DeleteKey[9];
extern POINT DeleteKeyTrans[9];

// CS3GDIKeyboard

IMPLEMENT_DYNAMIC(CS3GDIKeyboard, CEdit)

typedef struct _key {
	char def;
	char shift;
	char symbol;
} Key;

#define S3_KB_ROWS	5
#define S3_KB_COLS	14

Key kb[S3_KB_ROWS][S3_KB_COLS]; // = {};

// ----------------------------------------------------------------------------

CS3GDIKeyboard::CS3GDIKeyboard(CWnd* pParent = NULL)
{
	m_Parent = (CS3GDIScreenMain *)pParent;

	m_clrText = RGBw( 0, 0, 0 );

	m_UpdateImmediate = true;
	m_UpdateLoseFocus = true;

	m_PoppedUp = false;

	m_Constraints = 0;
	m_MaxChars = 0;
	m_Editor = NULL;

	char k = 0;
	for(char i = 0; i < 5; i++)
	{
		for(char j = 0; j < 3; j++)
		{
			m_RectButtons[k].left = S3_NP_BORDER +
				j * (S3_NP_KEY_W + S3_NP_KEY_SP);
			m_RectButtons[k].right = m_RectButtons[k].left + S3_NP_KEY_W;

			m_RectButtons[k].top = S3_NP_BORDER + // S3_NP_DISP_H +
				i * (S3_NP_KEY_H + S3_NP_KEY_SP);

			m_RectButtons[k].bottom = m_RectButtons[k].top + S3_NP_KEY_H;

			k++;
		}
	}

	m_RectPad.left = m_RectPad.top = 0;

	m_RectPad.right = m_RectPad.left +
		13 * S3_NP_KEY_SP +
		14 * S3_NP_KEY_W +
		2 * S3_NP_BORDER;

	m_RectPad.bottom = m_RectPad.top +
		4 * S3_NP_KEY_SP +
		5 * S3_NP_KEY_H + 
		2 * S3_NP_BORDER;

	// Get resources
	m_Parent->GetPopUpColours(m_hBrushBG, m_hBrushButton, m_hBrushDispBG);
	m_Parent->GetPopUpFont(m_hFont);

}

CS3GDIKeyboard::~CS3GDIKeyboard()
{

}


BEGIN_MESSAGE_MAP(CS3GDIKeyboard, CEdit)
	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()

// CS3Edit message handlers

// ----------------------------------------------------------------------------
// This is never called (?)
void CS3GDIKeyboard::OnUpdate()
{
	CString txt;

	GetWindowText(txt);
}

// ----------------------------------------------------------------------------

void CS3GDIKeyboard::OnEnUpdate()
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

	m_Parent->S3GDITextSupplied(txt);
}

// ----------------------------------------------------------------------------

void CS3GDIKeyboard::OnEnKillFocus()
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

void CS3GDIKeyboard::OnChange()
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

HBRUSH CS3GDIKeyboard::CtlColor(CDC* pDC, UINT nCtlColor)
{
	pDC->SetTextColor(m_clrText);	// Text
	// pDC->SetBkColor(m_clrBkgnd);	// Text bkgnd
	pDC->SetBkColor(m_Parent->m_crOrangeLight);	// Text bkgnd
	
	return m_Parent->m_hBrushOrLight; // m_brBkgnd;				// Control bkgnd
}

// ----------------------------------------------------------------------------

void CS3GDIKeyboard::Draw()
{
	if (!m_PoppedUp)
		return;

	HGDIOBJ obj = SelectObject(m_hdc, m_hBrushBG);

	S3_RECTR(m_hdc, m_RectPad, 5);

	SelectObject(m_hdc, m_hBrushButton);
	
	CString str;

	for(char j = 0; j < S3_NP_N_KEYS; j++)
	{
		S3_RECTR(m_hdc, m_RectButtons[j], 5);

		if (j < 9)
		{
			str.Format(_T("%d"), j + 1);
			DrawText(m_hdc, str, -1, & m_RectButtons[j], DT_CENTER);
		}
		else if (j == S3_NP_PLUS_MINUS_KEY)
			DrawText(m_hdc, _T("\u00b1"), -1, & m_RectButtons[j], DT_CENTER);
		else if (j == S3_NP_ZERO_KEY)
			DrawText(m_hdc, _T("0"), -1, & m_RectButtons[j], DT_CENTER);
		else if (j == S3_NP_DECIMAL_KEY)
			DrawText(m_hdc, _T("."), -1, & m_RectButtons[j], DT_CENTER);
		else if (j == S3_NP_DELETE_KEY)
		{
			// Not in main character set and not found on iPAN - found
			// on W8, maybe in Segoe symbol font??
			// DrawText(m_hdc, _T("\u232b"), -1, & m_RectButtons[j], DT_CENTER);
			HGDIOBJ o = SelectObject(m_hdc, GetStockObject(BLACK_PEN));
			Polyline(m_hdc, DeleteKeyTrans, 9);
			SelectObject(m_hdc, o);
		}
		else if (j == S3_NP_LEFT_KEY)
			DrawText(m_hdc, _T("\u2190"), -1, & m_RectButtons[j], DT_CENTER);
		else if (j == S3_NP_RIGHT_KEY)
			DrawText(m_hdc, _T("\u2192"), -1, & m_RectButtons[j], DT_CENTER);
	}

	SelectObject(m_hdc, obj);
}

// ----------------------------------------------------------------------------

void CS3GDIKeyboard::PopUp(HDC hdc, int xref, int yref, CS3NumEdit *editor, 
						 unsigned char constraints, 
						 unsigned char max_chars)
{
	m_Constraints = constraints;
	m_MaxChars = max_chars;

	if (m_PoppedUp)
		return;
	
	m_hdc = hdc;

	m_PoppedUp = true;

	m_xref = xref;
	m_yref = yref;

	m_Editor = editor;

	CString str;

	m_Editor->SetFocus(); // To show caret
	m_Editor->SetSel(-1);

	// TODO: Check boundaries
	CRect RectEditor;
	m_Editor->GetWindowRect(RectEditor);

	m_Parent->ScreenToClient(RectEditor);

	if (RectEditor.right + m_RectPad.Width() < m_Parent->S3GDIRectScreen().right &&
		(m_yref + m_RectPad.Height()) < m_Parent->S3GDIRectScreen().bottom)
	{
		// Just a move to the right required...
		m_xref = RectEditor.right + S3_NP_BORDER;
	}
	else
	{
		if (RectEditor.right + m_RectPad.Width() > m_Parent->S3GDIRectScreen().right)
		{
			// Make x fit
			m_xref -= RectEditor.right + m_RectPad.Width() - m_Parent->S3GDIRectScreen().right;

			// Move below if room
			if ((RectEditor.bottom + m_RectPad.Height()) < m_Parent->S3GDIRectScreen().bottom)
				m_yref = RectEditor.bottom + S3_NP_BORDER;
		}
		else
		{
			// TODO: and the other possibilties...
		}
	}

	m_RectPad.OffsetRect(m_xref, m_yref);

	for(char i = 0; i < S3_NP_N_KEYS; i++)
	{
		m_RectButtons[i].OffsetRect(m_xref, m_yref);
	}

	for(char i = 0; i < 9; i++)
	{
		DeleteKeyTrans[i].x = DeleteKey[i].x +
			m_RectButtons[S3_NP_DELETE_KEY].left;
		DeleteKeyTrans[i].y = DeleteKey[i].y +
			m_RectButtons[S3_NP_DELETE_KEY].top;
	}
}

// ----------------------------------------------------------------------------

void CS3GDIKeyboard::PopDown()
{
	if (!m_PoppedUp)
		return;
		
	m_PoppedUp = false;

	// TODO: Is this actually any better than recalculating layout on-the-fly?

	m_RectPad.OffsetRect(-m_xref, -m_yref);

	for(char j = 0; j < S3_NP_N_KEYS; j++)
	{
		m_RectButtons[j].OffsetRect(-m_xref, -m_yref);
	}

	m_Editor->ShowWindow(false);
	
	// This will crash if Create() not called - as asserted must be a
	// window - which the base CEdit class will not be. As the base
	// is not actually shown:
	// a) Is there any point in hiding the window? 
	// b) Is there any point in basing this class at all?
	ShowWindow(false);
}


// ----------------------------------------------------------------------------

char CS3GDIKeyboard::Find(POINT pt)
{
	if (!m_PoppedUp)
		return -1;

	if (m_RectPad.PtInRect(pt))
	{
		int cs, ce;
		CString	txt;
		int	len;

		m_Editor->GetWindowText(txt);
		m_Editor->GetSel(cs, ce);

		len = txt.GetLength();
		bool CanInsert = (len < m_MaxChars);

		for(char j = 0; j < S3_NP_N_KEYS; j++)
		{
			if (m_RectButtons[j].PtInRect(pt))
			{
				if (j == S3_NP_LEFT_KEY)
				{
					if (cs > 0)
					{
						cs--;
					}
				}
				else if (j == S3_NP_RIGHT_KEY)
				{
						cs++;
				}
				else if (j == S3_NP_DELETE_KEY)
				{
					if (cs > 0)
					{
						txt.Delete(cs - 1);
						cs--;
					}
				}
				else if (CanInsert)
				{
					if (j == S3_NP_PLUS_MINUS_KEY)
					{
						if (!(S3_NP_POSITIVE & m_Constraints))
						{
							if (ce == cs && cs == 0)
							{
								txt.Insert(ce, _T('-'));
								cs++;
							}
						}
					}
					else if (j == S3_NP_ZERO_KEY)
					{
						if (ce == cs)
						{
							txt.Insert(ce, _T('0'));
							cs++;
						}
					}
					else if (j == S3_NP_DECIMAL_KEY)
					{
						if (!(S3_NP_INTEGER & m_Constraints))
						{
							if (ce == cs)
							{
								txt.Insert(ce, _T('.'));
								cs++;
							}
						}
					}
					else
					{
						TCHAR c = TCHAR(j + 0x30 + 1);

						if (ce == cs)
						{
							txt.Insert(ce, c);
							cs++;
						}
					}
				}

				m_Editor->SetWindowText(txt);
				m_Editor->SetSel(cs, cs);
				// m_Editor->ShowWindow(true);

				return j;
			}
		}
		
		return -2;
	}

	// Was popped up
	return -3;
}

// ----------------------------------------------------------------------------
