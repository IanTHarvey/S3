// ----------------------------------------------------------------------------
#pragma once

// See:
// https://msdn.microsoft.com/en-us/library/eeah46xd%28v=vs.90%29.aspx

// CS3Edit
class CS3GDIScreenMain;

class CS3NumEdit : public CEdit
{
	DECLARE_DYNAMIC(CS3NumEdit)
		
	CS3GDIScreenMain	*m_Parent;
	CString				m_sVerified;

	COLORREF			m_clrText;

	bool				m_UpdateImmediate;
	bool				m_UpdateLoseFocus;

	BOOL				SetWindowPos(CRect &rect);
	void				Edit(CRect &rect, CString &str);

public:
	CS3NumEdit(CWnd* pParent);
	virtual ~CS3NumEdit();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdate(void);
	afx_msg void OnEnUpdate();
	afx_msg void OnEnKillFocus();
	afx_msg void OnEnSetFocus();
	afx_msg void OnChange();
public:
	afx_msg HBRUSH CtlColor(CDC* /*pDC*/, UINT /*nCtlColor*/);
};

// ----------------------------------------------------------------------------

