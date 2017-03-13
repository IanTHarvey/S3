// TextInputPopup.cpp : implementation file
//

#include "stdafx.h"

#include "S3DataModel.h"
#include "S3ControllerDlg.h"
#include "S3GDITextInputPopup.h"

// CTextInputPopup dialog

IMPLEMENT_DYNAMIC(CTextInputPopup, CDialog)

CTextInputPopup::CTextInputPopup(int x, int y, CString txt, CWnd* pParent /*=NULL*/)
	: CDialog(CTextInputPopup::IDD, pParent)
{
	m_x = x;
	m_y = y;
	m_txt = txt;

	m_Parent = (CS3GDIScreenMain *)pParent;
}

CTextInputPopup::~CTextInputPopup()
{
}

void CTextInputPopup::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TEXT_IP_EDIT, m_TextEdit);
}


BEGIN_MESSAGE_MAP(CTextInputPopup, CDialog)
	ON_BN_CLICKED(IDOK, &CTextInputPopup::OnBnClickedOk)
END_MESSAGE_MAP()


// CTextInputPopup message handlers

BOOL CTextInputPopup::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowPos(&this->wndTop, m_x, m_y, 0, 0, SWP_NOSIZE);

	m_TextEdit.SetWindowTextW(m_txt);

	// SetWindowPos(HWND_TOP, 500, 500, 0, 0, SWP_NOSIZE);
	return TRUE;  // return TRUE  unless you set the focus to a control
} // OnInitDialog

void CTextInputPopup::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here

	m_TextEdit.GetWindowText(m_txt);
	m_Parent->S3GDITextSupplied(m_txt);

	OnOK();
}
