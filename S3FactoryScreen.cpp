// S3FactoryScreen.cpp : implementation file
//

#include "stdafx.h"
#include "S3Controller.h"
#include "S3FactoryScreen.h"


// CS3FactoryScreen

IMPLEMENT_DYNCREATE(CS3FactoryScreen, CFormView)

CS3FactoryScreen::CS3FactoryScreen()
	: CFormView(CS3FactoryScreen::IDD)
{

}

CS3FactoryScreen::~CS3FactoryScreen()
{
}

void CS3FactoryScreen::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CS3FactoryScreen, CFormView)
	ON_BN_CLICKED(IDC_BUTTON1, &CS3FactoryScreen::OnBnClickedFinishButton)
END_MESSAGE_MAP()


// CS3FactoryScreen diagnostics

#ifdef _DEBUG
void CS3FactoryScreen::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CS3FactoryScreen::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CS3FactoryScreen message handlers

void CS3FactoryScreen::OnBnClickedFinishButton()
{
	// TODO: Add your control notification handler code here
}
