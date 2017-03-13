// ----------------------------------------------------------------------------

#pragma once

#define PM_MAX_ITEMS		8
#define PM_MAX_ITEM_LEN		16

#define PM_HEIGHT_ITEM		35
#define PM_WIDTH_MENU		100

#define IP_TOP_MARGIN		5
#define IP_MARGIN			10
#define IP_GAP				20

class CS3GDIInfoPopup
{
	CS3GDIScreenMain	*m_Parent;

	CRect	m_rect, m_IconRect, m_SelectRect;

	HDC			m_HDC;
	HBRUSH		m_hBrushBG, m_hBrushBGSel;
	COLORREF	m_crBG, m_crBGSel;
	// COLORREF	m_crTextNorm;
	COLORREF	m_crWhite;
	HFONT		m_hFontS;
	int			m_w, m_h;

	bool	m_PoppedUp;
	bool	m_Disabled;
	char	m_NumItems;
	char	m_SelectedItem;
	int		m_MaxWidth;
	
	CString	ItemStrings1[PM_MAX_ITEMS], ItemStrings2[PM_MAX_ITEMS];
	CRect	ItemRect[PM_MAX_ITEMS];

public:
	CS3GDIInfoPopup(CS3GDIScreenMain	*parent, HDC hdc, HDC hInfoButton);
	~CS3GDIInfoPopup();

	void	Init(int xref, int yref);
	void	AddItem(CString str1, CString str2);
	void	SelectItem(char item) { m_SelectedItem = item; };
	void	Draw();
	void	Clear();
	void	PopUp();
	void	PopDown();
	void	Disable(bool disable = true);
	void	Adjust();
	char	FindSelect(POINT p);
	int		Width() { return m_w; };
	int		Height() { return m_h; };
};

// ----------------------------------------------------------------------------