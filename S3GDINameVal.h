// ----------------------------------------------------------------------------

class NameVal
{
public:
	NameVal(		int xref, int yref, int xright,
					CString lbl, CString val,
					bool editable);
	void Draw(HDC hdcMem, HFONT hFont, HFONT hFontB);
	CRect RectEdit(HDC hdcMem, HFONT hFont);
	void SetValue(CString val);
	void AttachEditor(HDC hdcMem, CS3NumEdit *editor);

private:
	int		m_xref;
	int		m_yref;
	int		m_xright;
	CString	m_lbl;
	CString m_val;
	bool	m_editable;
	CRect	m_EditRect;
	CRect	m_EditOpenRect;
	CS3NumEdit	*m_Editor;
};

// ----------------------------------------------------------------------------