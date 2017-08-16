// ----------------------------------------------------------------------------

#pragma once

#define NV_LMARGIN	15	// For parameters (additional to LHMARGIN)
#define RMARGIN		15	// For parameters (additional to LHMARGIN)
#define LHMARGIN	5	// For headers
#define BMARGIN		10

#define S3_MAX_SELECTABLES	24

// class CS3AgentDlg;

class CS3NameValue
{
public:
	CS3NameValue(	int xref, int yref,
					int xright,		// Right justification position
					CString lbl,	// Left-hand label
					CString val,	// String representing the longest reasonable input
					bool editable,	// Displays m_val bold if editable
					char ID = -1);
	
	// Allow alarms to be displayed (flashing text)
	CS3NameValue(
#ifdef S3_AGENT
	CS3AgentDlg *Parent,
#else
	CS3ControllerDlg *Parent,
#endif
				   int xref, int yref, int xright,
				   CString lbl, CString val,
				   bool editable, char ID = -1);

	void	Draw(HDC hdc, HFONT hFont, HFONT hFontB);
	CRect	RectEdit(HDC hdc, HFONT hFont);
	void	SetValue(CString val);
	void	AttachEditor(HDC hdc, CEdit *editor);
	void	SetAlarm(bool alarm) { m_Alarm = alarm; };
	void	SetEditable(bool editable);
	bool	GetEditable() { return m_Editable; };
	CRect	GetEditRect() { return m_EditRect; };
	char	GetID() { return m_ID; };
	char	FindSelect(POINT p);

	static char	FindSelectable(POINT p);
	int		AddSelectable();

private:
#ifdef S3_AGENT
			CS3AgentDlg *m_Parent;
#else
			CS3ControllerDlg *m_Parent;
#endif
	bool	m_Alarm;

	char	m_ID;

	int		m_xref;
	int		m_yref;
	int		m_xright;
	CString	m_lbl;
	CString m_val;
	bool	m_Editable;
	CRect	m_EditRect;
	CRect	m_EditOpenRect;
	CEdit	*m_Editor;

	static CS3NameValue	*m_Selectable[];
	static char			m_NSelectable;
};


// ----------------------------------------------------------------------------