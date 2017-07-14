#include "stdafx.h"

#include "S3DataModel.h"

#ifdef S3_AGENT
#include "S3Agent/S3AgentDlg.h"
#else
#include "S3ControllerDlg.h"
#endif

extern COLORREF RGBw(BYTE r, BYTE g, BYTE b);

// ----------------------------------------------------------------------------

CParameterMenu::CParameterMenu(CS3GDIScreenMain *parent)
{
	m_Parent = parent;
	
	NumItems = 0;
	MaxWidth = 0;

	rect.bottom = rect.top = rect.left = rect.right = 0;
	Active = false;
	SelectedItem = -1; // Nothing
}

// ----------------------------------------------------------------------------

CParameterMenu::~CParameterMenu()
{
}

// ----------------------------------------------------------------------------
// Adjust to keep bottom and right of menu on screen

void CParameterMenu::Adjust()
{
	CRect rectscreen = m_Parent->S3GDIRectScreen();

	if (rect.right > rectscreen.right)
	{
		int shift = rectscreen.right - rect.right;
		
		rect.OffsetRect(shift, 0);
	
		for(char i = 0; i < NumItems; i++)
		{
			ItemRect[i].OffsetRect(shift, 0);
		}
	}

	if (rect.bottom > rectscreen.bottom)
	{
		int shift = rectscreen.bottom - rect.bottom;
		rect.OffsetRect(0, shift);

		for(char i = 0; i < NumItems; i++)
		{
			ItemRect[i].OffsetRect(0, shift);
		}
	}
}

// ----------------------------------------------------------------------------

void CParameterMenu::AddItem(CString str)
{
	if (NumItems < PM_MAX_ITEMS)
	{
		CRect str_rect;

		SelectObject(m_hdc, m_Parent->m_hFontLB);
		DrawText(m_hdc, str, -1, &str_rect, DT_CALCRECT | DT_CENTER);

		if (str_rect.Width() > MaxWidth)
		{
			MaxWidth = str_rect.Width();
			MenuWidth = MaxWidth + 2 * PM_MARGIN;
			rect.right = rect.left + MenuWidth;
		}

		ItemStrings[NumItems] = str;

		ItemRect[NumItems].top = rect.top + NumItems * PM_HEIGHT_ITEM;
		
		rect.bottom += PM_HEIGHT_ITEM;
		
		ItemRect[NumItems].left = rect.left;
		ItemRect[NumItems].right = rect.left + MenuWidth;
		ItemRect[NumItems].bottom = rect.bottom;

		NumItems++;
	}
}

// ----------------------------------------------------------------------------

void CParameterMenu::Init(HDC hdc, int x, int y)
{
	m_hdc = hdc;

	MaxWidth = 0;

	// ITH: Need to check boundaries
	rect.left = x;
	rect.top = y;
	rect.bottom = y + PM_BOTTOM_MARGIN;
	rect.right = x + MaxWidth;
}

// ----------------------------------------------------------------------------

void CParameterMenu::Activate()
{
	Adjust();
	Active = true;
}

// ----------------------------------------------------------------------------

void CParameterMenu::Clear()
{
	Active = false;
	NumItems = 0;
	SelectedItem = -1; // Nothing
}

// ----------------------------------------------------------------------------

void CParameterMenu::Draw()
{
	if (!Active)
		return;
	
	// HGDIOBJ objBrush = SelectObject(m_hdc, m_hBrushBG);
	HGDIOBJ objBrush = SelectObject(m_hdc, m_Parent->m_hBrushMenuBGMed);
	HGDIOBJ objPen = SelectObject(m_hdc, GetStockObject(NULL_PEN));
	SelectObject(m_hdc, m_Parent->m_hFontL);

	S3_RECTR(m_hdc,	rect, 10);

	for(char i = 0; i < NumItems; i++)
	{
		if (i == SelectedItem)
		{
			SelectObject(m_hdc, m_Parent->m_hFontL);
			SetTextColor(m_hdc, m_Parent->m_crMenuTxtGreyed);
		}
		else
		{
			SelectObject(m_hdc, m_Parent->m_hFontLB);
			SetTextColor(m_hdc, m_Parent->m_crMenuTxtActive);
		}

		DrawText(m_hdc, ItemStrings[i], -1, &ItemRect[i], DT_CENTER);
	}

	SetTextColor(m_hdc, m_Parent->m_crMenuTxtActive);

	SelectObject(m_hdc, objBrush);
	SelectObject(m_hdc, objPen);
}

// ----------------------------------------------------------------------------

char	CParameterMenu::FindSelect(POINT p)
{
	if (!Active)
		return -1;

	if (rect.PtInRect(p))
	{
		for(char i = 0; i < NumItems; i++)
		{
			if (ItemRect[i].PtInRect(p))
			{
				if (i == SelectedItem)
				{
					// Indicate selection is made but we don't
					// want to do anything about it
					return -2;
				}
				else
					return i;

				// Active = false;
			}
		}
	}

	return -1;
}

// ----------------------------------------------------------------------------
