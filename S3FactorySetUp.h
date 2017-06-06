
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
	char	m_Tx;			// Selected transmitter

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
	CEdit m_Rx1CalEdit;
	CEdit m_Rx2CalEdit;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void PostNcDestroy();
	CEdit m_TxCalEdit;
	afx_msg void OnBnClickedTxCalSetButton();
	CComboBox m_RFPathCombo;
	afx_msg void OnCbnSelchangeRfPathCombo();
	afx_msg void OnCbnDropdownRfPathCombo();
	afx_msg void OnBnClickedRx1CalSetButton();
	afx_msg void OnBnClickedRx2CalSetButton();
	CStatic m_StatusMsgStatic;

	void Init();
	void Update();
	void Enable(BOOL enable);
	CEdit m_TxOptCalEdit;
	afx_msg void OnBnClickedTxOptCalSetButton();
	CButton m_RFCalSetButton;
	CButton m_Rx1CalSetButton;
	CButton m_Rx2CalSetButton;
	CButton m_TxOptCalSetButton;
	CTabCtrl m_Tabs;
	afx_msg void OnTcnSelchangeFactoryTab(NMHDR *pNMHDR, LRESULT *pResult);
	
	CStatic m_DbgDSAsStatic;
	afx_msg void OnBnClickedDumpDiagButton();
	CComboBox m_FactoryRxDroplist;
	CComboBox m_FactoryTxDroplist;
	afx_msg void OnCbnSelchangeRxCombo();
	afx_msg void OnCbnSelchangeTxCombo();
	afx_msg void OnStnClickedRx1CalStatic();

	void UpdateRxTxCombos();
	void UpdateInfoStatic();

	CStatic m_RxTxInfoStatic;
	afx_msg void OnBnClickedFactTestButton();
};


