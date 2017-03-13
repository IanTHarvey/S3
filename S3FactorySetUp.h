
#pragma once

#include "afxwin.h"
#include "afxcmn.h"

// CS3FactorySetUp form view

class CS3GDIScreenMain;

class CS3FactorySetUp : public CDialog
{
	DECLARE_DYNAMIC(CS3FactorySetUp)

	CS3ControllerDlg	*m_Parent;

	char	m_RFPath;		// 0-6
	double	m_RFCalVals[7]; // Display values
	char	m_CurSel;		// Selected RF path on dropdown
	char	m_Rx;			// Selected receiver slot

public:
	CS3FactorySetUp(CWnd* pParent = NULL);
	virtual ~CS3FactorySetUp();

	virtual BOOL OnInitDialog();

public:
	enum { IDD = IDD_FACTORY_DIALOG };
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
	CEdit m_TxCalEdit;
	afx_msg void OnBnClickedTxCalSetButton();
	CComboBox m_RFPathCombo;
	afx_msg void OnCbnSelchangeRfPathCombo();
	afx_msg void OnCbnDropdownRfPathCombo();
	afx_msg void OnBnClickedRxCalSetButton();
	CEdit m_ScrnOffsetX;
	CEdit m_ScrnOffsetY;
	afx_msg void OnBnClickedScrnOffsetSetButton();
	CStatic m_StatusMsgStatic;
	CEdit m_FactSNEdit;
	afx_msg void OnBnClickedFactSnSetButton();

	void Init();
	void Update();
	void Enable(BOOL enable);
	CEdit m_TxOptCalEdit;
	afx_msg void OnBnClickedTxOptCalSetButton();
	CEdit m_FactPNEdit;
	afx_msg void OnBnClickedFactPnSetButton();
	CButton m_RFCalSetButton;
	CButton m_RxCalSetButton;
	CButton m_TxCalSetButton;
	CTabCtrl m_Tabs;
	afx_msg void OnTcnSelchangeFactoryTab(NMHDR *pNMHDR, LRESULT *pResult);
};


