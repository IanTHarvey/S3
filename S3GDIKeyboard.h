// ----------------------------------------------------------------------------
#pragma once

// See:
// https://msdn.microsoft.com/en-us/library/eeah46xd%28v=vs.90%29.aspx

// CS3Edit
class CS3GDIScreenMain;

#define S3_NP_N_KEYS			15
#define S3_NP_PLUS_MINUS_KEY	11
#define S3_NP_ZERO_KEY			10
#define S3_NP_DECIMAL_KEY		9
#define S3_NP_DELETE_KEY		12
#define S3_NP_LEFT_KEY			13
#define S3_NP_RIGHT_KEY			14

#define S3_NP_KEY_H				30
#define S3_NP_KEY_W				30
#define S3_NP_BORDER			6
#define S3_NP_KEY_SP			6
// #define S3_NP_DISP_H			30

// Constraints
#define S3_NP_INTEGER			1
#define S3_NP_POSITIVE			2

class CS3GDIKeyboard : public CEdit
{
	DECLARE_DYNAMIC(CS3GDIKeyboard)
		
	int					m_xref, m_yref;
	
	CS3GDIScreenMain	*m_Parent;
	CString				m_sVerified;

	COLORREF			m_clrText;

	bool				m_UpdateImmediate;
	bool				m_UpdateLoseFocus;

	bool				m_PoppedUp;
	CRect				m_RectButtons[S3_NP_N_KEYS];	// [L->R; T->B][. 0 -]
	CRect				m_RectPad;			// Background
//	CRect				m_RectDisplay;		// On-pad display

	HDC					m_hdc;
	HBRUSH				m_hBrushBG;
	HBRUSH				m_hBrushButton;
	HBRUSH				m_hBrushDispBG;
	HFONT				m_hFont;

	unsigned char		m_Constraints;
	unsigned char		m_MaxChars;

	CS3NumEdit			*m_Editor;

	// COLORREF	m_clrBkgnd;
	// CBrush	m_brBkgnd;

public:
	CS3GDIKeyboard(CWnd* pParent);
	virtual ~CS3GDIKeyboard();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdate(void);
	afx_msg void OnEnUpdate();
	afx_msg void OnEnKillFocus();
	afx_msg void OnChange();
public:
	afx_msg HBRUSH CtlColor(CDC* /*pDC*/, UINT /*nCtlColor*/);

	void	Draw();
	void	PopUp(HDC hdc, int xref, int yref, CS3NumEdit *editor,
											unsigned char constraints = 0,
											unsigned char max_chars = 0);
	void	PopDown();
	char	Find(POINT pt);
};

// ----------------------------------------------------------------------------

