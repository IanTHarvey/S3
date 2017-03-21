// S3FactorySetUp.cpp : implementation file
//
#ifndef TRIZEPS
#include "S3ControllerX86/targetver.h"
#else
#define WINVER _WIN32_WCE
#include <ceconfig.h>
#endif

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <stdio.h>
#include "S3DataModel.h"
#include "S3USBVCP.h"
#include "S3I2C.h"

#include "S3ControllerDlg.h"
#include "S3FactorySetUp.h"

extern int S3I2CTxGetRFCalGain(char Rx, char Tx);

// CS3FactorySetUp

IMPLEMENT_DYNAMIC(CS3FactorySetUp, CDialog)

CS3FactorySetUp::CS3FactorySetUp(CWnd* pParent)
	: CDialog(CS3FactorySetUp::IDD)
{
	m_Parent = (CS3ControllerDlg *)pParent;
	m_RFPath = 0;
	m_CurSel = 0;
	m_Rx = -1;
	
	for(char i = 0; i < 7; i++)
		m_RFCalVals[i] = -999.0;
}

CS3FactorySetUp::~CS3FactorySetUp()
{
}

void CS3FactorySetUp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RX1_CAL_EDIT, m_Rx1CalEdit);
	DDX_Control(pDX, IDC_RX2_CAL_EDIT, m_Rx2CalEdit);
	DDX_Control(pDX, IDC_TX_CAL_EDIT, m_TxCalEdit);
	DDX_Control(pDX, IDC_TX_OPT_CAL_EDIT, m_TxOptCalEdit);
	DDX_Control(pDX, IDC_RF_PATH_COMBO, m_RFPathCombo);
	DDX_Control(pDX, IDC_STATUS_MSG_STATIC, m_StatusMsgStatic);
	DDX_Control(pDX, IDC_CAL_SET_BUTTON, m_RFCalSetButton);
	DDX_Control(pDX, IDC_RX1_CAL_SET_BUTTON, m_Rx1CalSetButton);
	DDX_Control(pDX, IDC_RX2_CAL_SET_BUTTON, m_Rx2CalSetButton);
	DDX_Control(pDX, IDC_TX_OPT_CAL_SET_BUTTON, m_TxCalSetButton);
	
	
}

BEGIN_MESSAGE_MAP(CS3FactorySetUp, CDialog)
	ON_BN_CLICKED(IDOK, &CS3FactorySetUp::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CS3FactorySetUp::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_CAL_SET_BUTTON, &CS3FactorySetUp::OnBnClickedTxCalSetButton)
	ON_CBN_SELCHANGE(IDC_RF_PATH_COMBO, &CS3FactorySetUp::OnCbnSelchangeRfPathCombo)
	ON_CBN_DROPDOWN(IDC_RF_PATH_COMBO, &CS3FactorySetUp::OnCbnDropdownRfPathCombo)
	ON_BN_CLICKED(IDC_RX1_CAL_SET_BUTTON, &CS3FactorySetUp::OnBnClickedRx1CalSetButton)
	ON_BN_CLICKED(IDC_RX2_CAL_SET_BUTTON, &CS3FactorySetUp::OnBnClickedRx2CalSetButton)
	ON_BN_CLICKED(IDC_TX_OPT_CAL_SET_BUTTON, &CS3FactorySetUp::OnBnClickedTxOptCalSetButton)
END_MESSAGE_MAP()

BOOL CS3FactorySetUp::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_RFPathCombo.AddString(_T("1"));
	m_RFPathCombo.AddString(_T("2 N/A"));
	m_RFPathCombo.AddString(_T("3"));
	m_RFPathCombo.AddString(_T("4"));
	m_RFPathCombo.AddString(_T("5"));
	m_RFPathCombo.AddString(_T("6"));
	m_RFPathCombo.AddString(_T("7"));

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// ----------------------------------------------------------------------------

int SetSIPRegKey(DWORD data)
{
#ifdef TRIZEPS

	HKEY	hKey;
	int		err;

	err = RegOpenKeyEx(HKEY_CURRENT_USER, _T("ControlPanel\\Sip"), 0, 
											KEY_SET_VALUE, &hKey);

	if (err == ERROR_SUCCESS)
	{
		err = RegSetValueEx(hKey, _T("TurnOffAutoDeploy"), 0,
			REG_DWORD, (BYTE *)(&data), sizeof(DWORD));

		if (err != ERROR_SUCCESS)
		{
			S3EventLogAdd("Failed to set SIP registry key", 1, -1, -1, -1);
			return 1;
		}
	}
	else
	{
		S3EventLogAdd("Failed to open SIP registry key", 1, -1, -1, -1);
		return 1;
	}

#endif

	return 0;
}

// ----------------------------------------------------------------------------

void CS3FactorySetUp::Init()
{
	S3EventLogAdd("Entering factory set-up screen", 3, -1, -1, -1);

	SetSIPRegKey(0); // Enable SIP pop-up
	
	int err = S3I2CTxGetRFCalGain(0, 0);
	err = S3I2CTxGetOptCalGain(0, 0);

	for(char i = 0; i < 7; i++)
		m_RFCalVals[i] = (double)S3TxGetCalRF(0, 0, i) / 100.0;

	m_RFPathCombo.SetCurSel(m_RFPath);

	CString	tmp;

	tmp.Format(_T("%.2f"), (double)S3TxGetCalRF(0, 0, m_RFPath) / 100.0);
	m_TxCalEdit.SetWindowText(tmp);

	tmp.Format(_T("%.2f"), (double)S3RxGetCalGain(0, 0) / 100.0);
	m_Rx1CalEdit.SetWindowText(tmp);

	tmp.Format(_T("%.2f"), (double)S3RxGetCalGain(0, 1) / 100.0);
	m_Rx2CalEdit.SetWindowText(tmp);

	tmp.Format(_T("%.2f"), (double)S3TxGetCalOpt(0, 0) / 100.0);
	m_TxOptCalEdit.SetWindowText(tmp);

	Update();
}

// ----------------------------------------------------------------------------

void CS3FactorySetUp::Update()
{
	if (S3RxGetType(0) ==  S3_RxEmpty)
	{
		m_Rx1CalEdit.EnableWindow(FALSE);
		m_Rx1CalSetButton.EnableWindow(FALSE);

		m_Rx2CalEdit.EnableWindow(FALSE);
		m_Rx2CalSetButton.EnableWindow(FALSE);
	}
	else
	{
		m_Rx1CalEdit.EnableWindow(TRUE);
		m_Rx1CalSetButton.EnableWindow(TRUE);

		m_Rx2CalEdit.EnableWindow(TRUE);
		m_Rx2CalSetButton.EnableWindow(TRUE);
	}

	if (S3RxGetType(0) == S3_RxEmpty || S3TxGetType(0, 0) ==  S3_TxUnconnected)
	{
		m_RFPathCombo.EnableWindow(FALSE);
		m_TxOptCalEdit.EnableWindow(FALSE);
		m_TxCalEdit.EnableWindow(FALSE);

		m_RFCalSetButton.EnableWindow(FALSE);
		m_TxCalSetButton.EnableWindow(FALSE);
	}
	else
	{
		m_RFPathCombo.EnableWindow(TRUE);
		m_TxOptCalEdit.EnableWindow(TRUE);
		m_TxCalEdit.EnableWindow(TRUE);

		m_RFCalSetButton.EnableWindow(TRUE);
		m_TxCalSetButton.EnableWindow(TRUE);
	}
}

// ----------------------------------------------------------------------------

void CS3FactorySetUp::Enable(BOOL enable)
{
	m_Rx1CalEdit.EnableWindow(enable);
	m_Rx1CalSetButton.EnableWindow(enable);

	m_Rx2CalEdit.EnableWindow(enable);
	m_Rx2CalSetButton.EnableWindow(enable);

	m_RFPathCombo.EnableWindow(enable);
	m_TxOptCalEdit.EnableWindow(enable);
	m_TxCalEdit.EnableWindow(enable);

	m_RFCalSetButton.EnableWindow(enable);
	m_TxCalSetButton.EnableWindow(enable);

	UpdateWindow();
}

// ----------------------------------------------------------------------------
// CS3FactorySetUp diagnostics

#ifdef _DEBUG
void CS3FactorySetUp::AssertValid() const
{
	CDialog::AssertValid();
}

/*
#ifndef _WIN32_WCE
void CS3FactorySetUp::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
*/
#endif //_DEBUG


// ----------------------------------------------------------------------------
// CS3FactorySetUp message handlers

void CS3FactorySetUp::OnBnClickedOk()
{
	/*
	CString RxCal, TxCal;
	wchar_t *e;

	m_RxCalEdit.GetWindowText(RxCal);

	double RxCalVal = wcstod(RxCal, &e);

	if (*e != '\0')
	{
		// Error
	}
	else
		S3I2CRxSetCalibration(0, RxCalVal);

	m_TxCalEdit.GetWindowText(TxCal);

	double TxCalVal = _tstof(TxCal);

	if (*e != '\0')
	{
		// Error
	}
	else
	{
		S3I2CTxSetRFCalibration(m_RFPath + 1, TxCalVal);

	}
	*/
}

// ----------------------------------------------------------------------------

void CS3FactorySetUp::OnBnClickedCancel()
{
	SetSIPRegKey(1); // Disable SIP pop-up

	m_Parent->HideFactory();
	OnCancel();
}

// ----------------------------------------------------------------------------
// See:
// http://stackoverflow.com/questions/944033/is-this-a-memory-leak-in-mfc/944190#944190

void CS3FactorySetUp::PostNcDestroy() 
{
    CDialog::PostNcDestroy();
    delete this;
}

// ----------------------------------------------------------------------------

void CS3FactorySetUp::OnBnClickedTxCalSetButton()
{
	m_StatusMsgStatic.SetWindowText(_T("Wait..."));
	Enable(FALSE);
	// m_StatusMsgStatic.UpdateWindow();
	
	CString	TxCal;
	wchar_t	*e;

	m_TxCalEdit.GetWindowText(TxCal);

	double TxCalVal = wcstod(TxCal, &e);

	if (*e != '\0')
	{
		m_StatusMsgStatic.SetWindowText(_T("ERROR: Invalid calibration value"));
	}
	else
	{
		int err = S3I2CTxSetRFCalibration(m_RFPath, TxCalVal);

		if (err == -1)
		{
			m_StatusMsgStatic.SetWindowText(_T("ERROR: No receiver in slot 1"));
		}
		else if (err == -2)
		{
			m_StatusMsgStatic.SetWindowText(_T("ERROR: No transmitter attached"));
		}
		else if (err)
		{
			CString tmp;

			tmp.Format(_T("ERROR: calibration failed: %d"), err);
			m_StatusMsgStatic.SetWindowText(tmp);
		}
		else
		{
			m_StatusMsgStatic.SetWindowText(_T("Calibration factor updated"));
		}
	}

	Enable(TRUE);

	CString tmp;

	tmp.Format(_T("%.2f"), (double)S3TxGetCalRF(0, 0, m_RFPath) / 100.0);
	m_TxCalEdit.SetWindowText(tmp);
}

// ----------------------------------------------------------------------------

void CS3FactorySetUp::OnCbnSelchangeRfPathCombo()
{
	CString tmp;
	m_TxCalEdit.GetWindowText(tmp);

	wchar_t	*e;

	m_TxCalEdit.GetWindowText(tmp);

	double CalVal = wcstod(tmp, &e);

	if (*e != '\0')
	{
		m_StatusMsgStatic.SetWindowText(_T("ERROR: Invalid calibration value"));
		return;
	}

	m_RFCalVals[m_CurSel] = CalVal;
	
	m_CurSel = m_RFPathCombo.GetCurSel();
	
	m_RFPath = m_CurSel;

	CString	TxCal;

	TxCal.Format(_T("%.2f"), m_RFCalVals[m_RFPath]);
	m_TxCalEdit.SetWindowText(TxCal);
}

// ----------------------------------------------------------------------------

void CS3FactorySetUp::OnCbnDropdownRfPathCombo()
{
	/*
	int cs = m_RFPathCombo.GetCurSel();

	CString	TxCal;
	wchar_t	*e;

	m_TxCalEdit.GetWindowText(TxCal);

	m_RFCalVals[cs] = wcstod(TxCal, &e);
	*/
}

// ----------------------------------------------------------------------------

void CS3FactorySetUp::OnBnClickedRx1CalSetButton()
{
	m_StatusMsgStatic.SetWindowText(_T("Wait..."));
	Enable(FALSE);

	CString	RxCal;
	wchar_t	*e;

	m_Rx1CalEdit.GetWindowText(RxCal);

	double CalVal = wcstod(RxCal, &e);

	if (*e != '\0')
	{
		m_StatusMsgStatic.SetWindowText(_T("ERROR: Invalid calibration value"));
	}
	else
	{
		int err = S3I2CRxSetCalibration(0, -1, CalVal);

		if (err == -1)
		{
			m_StatusMsgStatic.SetWindowText(_T("ERROR: No receiver in slot 1"));
		}
		else if (err)
		{
			CString tmp;

			tmp.Format(_T("ERROR: calibration failed: %d"), err);
			m_StatusMsgStatic.SetWindowText(tmp);
		}
		else
		{
			m_StatusMsgStatic.SetWindowText(_T("Calibration factor updated"));
		}
	}

	Enable(TRUE);

	CString tmp;
	
	tmp.Format(_T("%.2f"), (double)S3RxGetCalGain(0, 0) / 100.0);
	m_Rx1CalEdit.SetWindowText(tmp);
}

// ----------------------------------------------------------------------------

void CS3FactorySetUp::OnBnClickedRx2CalSetButton()
{
	m_StatusMsgStatic.SetWindowText(_T("Wait..."));
	Enable(FALSE);

	CString	RxCal;
	wchar_t	*e;

	m_Rx2CalEdit.GetWindowText(RxCal);

	double CalVal = wcstod(RxCal, &e);

	if (*e != '\0')
	{
		m_StatusMsgStatic.SetWindowText(_T("ERROR: Invalid calibration value"));
	}
	else
	{
		int err = S3I2CRxSetCalibration(0, 1, CalVal);

		if (err == -1)
		{
			m_StatusMsgStatic.SetWindowText(_T("ERROR: No receiver in slot 1"));
		}
		else if (err)
		{
			CString tmp;

			tmp.Format(_T("ERROR: calibration failed: %d"), err);
			m_StatusMsgStatic.SetWindowText(tmp);
		}
		else
		{
			m_StatusMsgStatic.SetWindowText(_T("Calibration factor updated"));
		}
	}

	Enable(TRUE);

	CString tmp;
	
	tmp.Format(_T("%.2f"), (double)S3RxGetCalGain(0, 1) / 100.0);
	m_Rx2CalEdit.SetWindowText(tmp);
	
}

// ----------------------------------------------------------------------------

void CS3FactorySetUp::OnBnClickedTxOptCalSetButton()
{
	m_StatusMsgStatic.SetWindowText(_T("Wait..."));
	Enable(FALSE);

	CString	TxCal;
	wchar_t	*e;

	m_TxOptCalEdit.GetWindowText(TxCal);

	double CalVal = wcstod(TxCal, &e);

	if (*e != '\0')
	{
		m_StatusMsgStatic.SetWindowText(_T("ERROR: Invalid calibration value"));
	}
	else
	{
		int err = S3I2CTxSetOptCalibration(CalVal);

		if (err == -1)
		{
			m_StatusMsgStatic.SetWindowText(_T("ERROR: No receiver in slot 1"));
		}
		else if (err)
		{
			CString tmp;

			tmp.Format(_T("ERROR: calibration failed: %d"), err);
			m_StatusMsgStatic.SetWindowText(tmp);
		}
		else
		{
			m_StatusMsgStatic.SetWindowText(_T("Optical calibration factor updated"));
		}
	}

	Enable(TRUE);

	CString tmp;
	
	tmp.Format(_T("%.2f"), (double)S3TxGetCalOpt(0, 0) / 100.0);
	m_TxOptCalEdit.SetWindowText(tmp);
}

// ----------------------------------------------------------------------------

void CS3FactorySetUp::OnTcnSelchangeFactoryTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

// ----------------------------------------------------------------------------


