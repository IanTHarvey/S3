
// ----------------------------------------------------------------------------
// A pop-up window, attached to a small info-page icon, for secondary 
// information.

#ifndef TRIZEPS
#include "S3ControllerX86/targetver.h"
#else
#define WINVER _WIN32_WCE
#include <ceconfig.h>
#endif

#include "afxpriv.h"

#include "S3DataModel.h"
#include "S3ControllerDlg.h"
#include "S3GDIInfoPopup.h"

extern COLORREF RGBw(BYTE r, BYTE g, BYTE b);

extern int KeepInRect(CRect outer, CRect &in);

CS3GDIInfoPopup::CS3GDIInfoPopup(CS3GDIScreenMain *parent, HDC hdc, HDC hInfoButton)
{
	m_Parent = parent;

	m_NumItems = 0;

	m_rect.bottom = m_rect.top = m_rect.left = m_rect.right = 0;
	m_rect.bottom = 5;

	m_PoppedUp = false;
	m_Disabled = false;
	m_SelectedItem = -1; // Nothing

	m_MaxWidth = 0;

	// m_crTextNorm =	m_Parent->m_crTextNorm;		//	RGBw(30, 30, 30);
	m_crWhite =	m_Parent->m_crWhite;

	m_hFontS = m_Parent->m_hFontS;

	m_HDC = hdc;

	m_Parent->GetDCDims(hInfoButton, &m_w, &m_h);
}

// ----------------------------------------------------------------------------

CS3GDIInfoPopup::~CS3GDIInfoPopup()
{
}

// ----------------------------------------------------------------------------
// Adjust to keep all on screen

void CS3GDIInfoPopup::Adjust()
{
	if (m_rect.right > m_Parent->S3GDIRectScreen().right)
	{
		int shift = m_Parent->S3GDIRectScreen().right - m_rect.right;
		
		m_rect.OffsetRect(shift, 0);
	
		for(char i = 0; i < m_NumItems; i++)
			ItemRect[i].OffsetRect(shift, 0);
	}

	// TODO: Frigged to add 20-up to get above task-bar - remove when
	// task-bar removed.
	if (m_rect.bottom > m_Parent->S3GDIRectScreen().bottom)
	{
		int shift = m_Parent->S3GDIRectScreen().bottom - m_rect.bottom;
		m_rect.OffsetRect(0, shift - 20);

		for(char i = 0; i < m_NumItems; i++)
			ItemRect[i].OffsetRect(0, shift - 20);
	}
}

// ----------------------------------------------------------------------------
// 

void CS3GDIInfoPopup::AddItem(CString str1, CString str2)
{
	CRect rect1, rect2;

	if (m_NumItems < PM_MAX_ITEMS)
	{
		ItemStrings1[m_NumItems] = str1;
		ItemStrings2[m_NumItems] = str2;

		HGDIOBJ objFont = SelectObject(m_HDC, m_hFontS);

		DrawText(m_HDC, str1, -1, &rect1, DT_CALCRECT | DT_RIGHT);
		DrawText(m_HDC, str2, -1, &rect2, DT_CALCRECT | DT_RIGHT);

		SelectObject(m_HDC, objFont);

		ItemRect[m_NumItems].top = m_rect.bottom + IP_TOP_MARGIN;
		
		m_rect.bottom += PARA_ROW;
		
		ItemRect[m_NumItems].left = m_rect.left + IP_MARGIN;
		ItemRect[m_NumItems].right = m_rect.left +
								rect1.Width() + IP_GAP + rect2.Width();
		ItemRect[m_NumItems].bottom = m_rect.bottom;

		// Adjust menu width if necessary
		int w = ItemRect[m_NumItems].Width() + 2 * IP_MARGIN;

		if (w > m_MaxWidth)
		{
			m_MaxWidth = w;
			m_rect.right = m_rect.left + w;
		}

		m_NumItems++;

		for(char i = 0; i < m_NumItems; i++)
			ItemRect[i].right = ItemRect[i].left + m_MaxWidth - 2 * IP_MARGIN;
	}
}

// ----------------------------------------------------------------------------

void CS3GDIInfoPopup::Init(  int xref, int yref)
{
	m_IconRect.left = xref;
	m_IconRect.top = yref;
	m_IconRect.right = xref + m_w;
	m_IconRect.bottom = yref + m_h;

	// Make touch area larger
	m_SelectRect = m_IconRect;
	m_SelectRect.left -= m_w;
	m_SelectRect.right += 3 * m_w / 2;
	m_SelectRect.bottom += 3 * m_h / 2;

	m_rect.left = m_IconRect.left;
	m_rect.top = m_IconRect.top;
	m_rect.bottom = m_IconRect.top;
	m_rect.right = m_IconRect.left + PM_WIDTH_MENU;
}

// ----------------------------------------------------------------------------

void	CS3GDIInfoPopup::Disable(bool disable)
{
	// if (disable)
	//	Clear();
	
	m_Disabled = disable;
}

// ----------------------------------------------------------------------------

void	CS3GDIInfoPopup::PopDown()
{
	m_PoppedUp = false;
}

// ----------------------------------------------------------------------------

void	CS3GDIInfoPopup::PopUp()
{
	m_Parent->S3ParaMenuClear();
	Adjust();
	m_PoppedUp = true;
}

// ----------------------------------------------------------------------------

void	CS3GDIInfoPopup::Clear()
{
	m_PoppedUp = false;
	m_NumItems = 0;
	m_SelectedItem = -1; // Nothing
	m_MaxWidth = 0;

	m_rect.left = m_IconRect.left;
	m_rect.top = m_IconRect.top;
	m_rect.bottom = m_IconRect.top;
	m_rect.right = m_IconRect.left + PM_WIDTH_MENU;
}

// ----------------------------------------------------------------------------

void CS3GDIInfoPopup::Draw()
{
	if (m_Disabled)
		S3BLT(m_Parent->m_hbmpInfoButtonGrey, m_IconRect.left, m_IconRect.top,
			m_IconRect.Width(), m_IconRect.Height());
	else
		S3BLT(m_Parent->m_hbmpInfoButton, m_IconRect.left, m_IconRect.top,
			m_IconRect.Width(), m_IconRect.Height());

	if (!m_PoppedUp)
		return;
	
	HGDIOBJ objBrush =	SelectObject(m_HDC, m_Parent->m_hBrushMenuBGMed);
	HGDIOBJ objPen =	SelectObject(m_HDC, GetStockObject(NULL_PEN));
	HGDIOBJ objFont =	SelectObject(m_HDC, m_hFontS);

	RoundRect(m_HDC,	m_rect.left,	m_rect.top,
						m_rect.right,	m_rect.bottom + PM_BOTTOM_MARGIN,
						10, 10);

	// SetTextColor(m_HDC, m_Parent->m_crTextNorm);
	SetTextColor(m_HDC, m_Parent->m_crMenuTxtActive);

	for(char i = 0; i < m_NumItems; i++)
	{
		DrawText(m_HDC, ItemStrings1[i], -1, &ItemRect[i], DT_LEFT);
		DrawText(m_HDC, ItemStrings2[i], -1, &ItemRect[i], DT_RIGHT);
	}

	SelectObject(m_HDC, objBrush);
	SelectObject(m_HDC, objPen);
	SelectObject(m_HDC, objFont);

	SetTextColor(m_HDC, m_Parent->m_crTextNorm);
}

// ----------------------------------------------------------------------------

char CS3GDIInfoPopup::FindSelect(POINT p)
{
	if (m_Disabled)
		return 0;

	if (m_SelectRect.PtInRect(p))
	{
		PopUp();
		return 1;
	}

	m_PoppedUp = false;

	return 0;
}

// ----------------------------------------------------------------------------
