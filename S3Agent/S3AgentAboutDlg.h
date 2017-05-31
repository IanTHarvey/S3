#include "stdafx.h"
#include "resource.h"

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
    CEdit m_AboutText;

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};