#pragma once

// CS3Edit
class CS3GDIScreenMain;

class CS3Edit : public CEdit
{
	DECLARE_DYNAMIC(CS3Edit)
		
	CS3GDIScreenMain	*m_Parent;

	COLORREF			m_clrText;
	BOOL				SetWindowPos(CRect &rect);
	void				Edit(CRect &rect, CString &str);

public:
	CS3Edit(CWnd* pParent);
	virtual ~CS3Edit();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdate(void);
public:
	afx_msg void OnEnUpdate();
	afx_msg void OnEnKillFocus();
	afx_msg void OnEnSetFocus();
	afx_msg HBRUSH CtlColor(CDC* /*pDC*/, UINT /*nCtlColor*/);
};

// ----------------------------------------------------------------------------
