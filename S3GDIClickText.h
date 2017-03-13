
class CClickText 
{
private:
	CS3GDIScreenMain *m_Parent;
	CString m_s;
	CRect	m_r;
	bool	m_Enabled;
	int		m_Justification;
	unsigned char	m_Font; // 0: small, 1: large
	HDC		m_HDC;

public:
	CClickText(CS3GDIScreenMain *p, CRect r, HDC hdc,
		int Justification = DT_RIGHT, char font = 0)
		{	m_Parent = p;
			m_r = r;
			m_HDC = hdc;
			m_Enabled = true;
			m_Justification = Justification;
			m_Font = font; // Small
		};

	void SetString(CString s, bool redraw = true)
				{m_s = s; if (redraw) Draw(); };
	void Enable(bool en) { m_Enabled = en; };
	void Draw(void);
	BOOL Find(POINT p);
	CRect &Rect() { return m_r; };
};
