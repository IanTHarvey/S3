
#pragma once

#define PM_MAX_ITEMS		8
#define PM_MAX_ITEM_LEN		16

#define PM_HEIGHT_ITEM		35
#define PM_WIDTH_MENU		100

#define PM_MARGIN			8
#define PM_BOTTOM_MARGIN	10
#define PM_BOTTOM_CLEARANCE	10	// Bottom margin of pop-up menu at bottom of screen

class CParameterMenu
{
	CS3GDIScreenMain	*m_Parent;

public:
	CParameterMenu(CS3GDIScreenMain	*parent);
	~CParameterMenu();

	CRect	rect;

	HDC			m_hdc;
	HBRUSH		m_hBrushBG, m_hBrushBGSel;
	COLORREF	m_crBG, m_crBGSel;

	bool	Active;
	char	NumItems;
	char	SelectedItem;

	int		MaxWidth;
	int		MenuWidth;
	
	CString	ItemStrings[PM_MAX_ITEMS];
	CRect	ItemRect[PM_MAX_ITEMS];

	void	Init(HDC m_hdc, int x, int y);
	void	AddItem(CString str);
	void	SelectItem(char item) { SelectedItem = item; };
	bool	Activated() { return Active; };
	void	Draw();
	void	Clear();
	void	Activate();
	void	Adjust();
	char	FindSelect(POINT p);
};
