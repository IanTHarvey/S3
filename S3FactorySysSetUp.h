
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
	CEdit m_ScrnOffsetX;
	CEdit m_ScrnOffsetY;
	CStatic m_StatusMsgStatic;
	CEdit m_FactSNEdit;
	CEdit m_FactPNEdit;
	CButton m_SealBatteryButton;

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void PostNcDestroy();
	afx_msg void OnBnClickedScrnOffsetSetButton();
	afx_msg void OnBnClickedFactSnSetButton();
	afx_msg void OnBnClickedFactPnSetButton();
	afx_msg void OnBnClickedTestButton();
	afx_msg void OnBnClickedSealButton();

	void Init();
	void Update();
	afx_msg void OnBnClickedUnsealButton();
	CEdit m_BatterySNEdit;
	afx_msg void OnBnClickedBattSnSetButton();
	CComboBox m_BatteryTypeCombo;
	CButton m_UnsealBatteryButton;
};


