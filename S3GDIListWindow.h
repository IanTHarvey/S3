// ----------------------------------------------------------------------------
#pragma once

// See:
// https://msdn.microsoft.com/en-us/library/eeah46xd%28v=vs.90%29.aspx

// CS3Edit
class CS3GDIScreenMain;

#define S3_LW_N_KEYS			15
#define S3_LW_PLUS_MINUS_KEY	11
#define S3_LW_ZERO_KEY			10
#define S3_LW_DECIMAL_KEY		9
#define S3_LW_DELETE_KEY		12
#define S3_LW_LEFT_KEY			13
#define S3_LW_RIGHT_KEY			14

#define S3_LW_KEY_H				30
#define S3_LW_KEY_W				30
#define S3_LW_BORDER			6
#define S3_LW_KEY_SP			6

// Constraints
#define S3_LW_INTEGER			0x01
#define S3_LW_POSITIVE			0x02
#define S3_LW_DECIMAL			0x04 // Not used - why?

class CS3ListWindow : public CEdit
{
	DECLARE_DYNAMIC(CS3ListWindow)
		
	int					m_xref, m_yref;
	
	CS3GDIScreenMain	*m_Parent;
	CString				m_sVerified;

	COLORREF			m_clrText, m_clrBG;

	bool				m_UpdateImmediate;	// Update-while-typing
	bool				m_UpdateLoseFocus;	// Only when focus lost

	bool				m_PoppedUp;	// Current state
	CRect				m_RectButtons[S3_LW_N_KEYS];	// [L->R; T->B][. 0 -]
	CRect				m_RectPad;			// Background

	HDC					m_HDC;
	HBRUSH				m_hBrushBG;
	HBRUSH				m_hBrushButton;
	HBRUSH				m_hBrushDispBG;
	HFONT				m_hFont;

	unsigned char		m_Constraints;
	unsigned char		m_MaxChars;

public:
	CS3ListWindow(CWnd* pParent);
	virtual ~CS3ListWindow();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdate(void);
	afx_msg void OnEnUpdate();
	afx_msg void OnEnKillFocus();
	afx_msg void OnChange();
public:
	afx_msg HBRUSH CtlColor(CDC* /*pDC*/, UINT /*nCtlColor*/);

	void	Draw();
	void	PopUp(HDC hdc, int xref, int yref,
											unsigned char constraints = 0,
											unsigned char max_chars = 0);
	void	PopDown();
	char	Find(POINT pt);
};

// ----------------------------------------------------------------------------

