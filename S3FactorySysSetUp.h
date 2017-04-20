
#pragma once

#include "afxwin.h"
#include "afxcmn.h"

// CS3FactorySysSetUp form view

class CS3GDIScreenMain;

class CS3FactorySysSetUp : public CDialog
{
	DECLARE_DYNAMIC(CS3FactorySysSetUp)

	CS3ControllerDlg	*m_Parent;

public:
	CS3FactorySysSetUp(CWnd* pParent = NULL);
	virtual ~CS3FactorySysSetUp();

	virtual BOOL OnInitDialog();

public:
	enum { IDD = IDD_FACTORY_SYS_DIALOG };
#ifdef _DEBUG
	virtual void AssertValid() const;
/*
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
*/
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_RxCalEdit;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void PostNcDestroy();
	CEdit m_ScrnOffsetX;
	CEdit m_ScrnOffsetY;
	afx_msg void OnBnClickedScrnOffsetSetButton();
	CStatic m_StatusMsgStatic;
	CEdit m_FactSNEdit;
	afx_msg void OnBnClickedFactSnSetButton();

	void Init();
	CEdit m_FactPNEdit;
	afx_msg void OnBnClickedFactPnSetButton();
	afx_msg void OnBnClickedPeakThrButton();
	CStatic m_PeakThrStatic;
	CEdit m_PeakThrEdit;
	afx_msg void OnBnClickedTestButton();
	afx_msg void OnBnClickedSealButton();
	afx_msg void OnBnClickedSelfTestButton();
};


