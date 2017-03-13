#pragma once
#include "afxwin.h"


// CTextInputPopup dialog

class CTextInputPopup : public CDialog
{
	DECLARE_DYNAMIC(CTextInputPopup)

	int		m_x, m_y;
	CString	m_txt;
	CS3GDIScreenMain	*m_Parent;

public:
	CTextInputPopup(int x, int y, CString txt, CWnd* pParent = NULL);   // standard constructor
	virtual ~CTextInputPopup();

// Dialog Data
	enum { IDD = IDD_TEXT_IP_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CEdit m_TextEdit;
};
