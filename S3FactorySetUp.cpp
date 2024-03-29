// S3FactorySetUp.cpp : implementation file
//
#include "stdafx.h"

#include "S3DataModel.h"
#include "S3GPIB.h"
#include "S3USBVCP.h"
#include "S3I2C.h"

#include "S3ControllerDlg.h"
#include "S3FactorySetUp.h"

#ifdef TRIZEPS
#include "drvLib_app.h"
#endif

extern int S3I2CTxGetRFCalGain(char Rx, char Tx);

extern unsigned char	Dbg_RF1_DSA;
extern short			Dbg_TxOpt;	// Gain soft set 0xA8-9
extern short			Dbg_RxOpt;	// Gain soft set 0xA4-5
extern short			Dbg_RLL;
extern char				Dbg_Path;
extern short PeakThTable[];


// CS3FactorySetUp

IMPLEMENT_DYNAMIC(CS3FactorySetUp, CDialog)

// ----------------------------------------------------------------------------

CS3FactorySetUp::CS3FactorySetUp(CWnd* pParent)
	: CDialog(CS3FactorySetUp::IDD)
{
	m_Parent = (CS3ControllerDlg *)pParent;
	m_RFPath = 0;
	m_CurSel = 0;
	m_Rx = -1;
	m_Tx = -1;
	m_IP = 0; // TODO: Get selected?
	
	for(char i = 0; i < S3_TX_N_RF_PATH; i++)
		m_RFCalVals[i] = -999.0;
}

// ----------------------------------------------------------------------------

CS3FactorySetUp::~CS3FactorySetUp()
{
}

// ----------------------------------------------------------------------------

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
	DDX_Control(pDX, IDC_TX_OPT_CAL_SET_BUTTON, m_TxOptCalSetButton);
	DDX_Control(pDX, IDC_DBG_DSAS_STATIC, m_DbgDSAsStatic);
	DDX_Control(pDX, IDC_RX_COMBO, m_FactoryRxDroplist);
	DDX_Control(pDX, IDC_TX_COMBO, m_FactoryTxDroplist);
	DDX_Control(pDX, IDC_RXTX_INFO_STATIC, m_RxTxInfoStatic);
	DDX_Control(pDX, IDC_PEAK_THR_STATIC, m_PeakThrStatic);
	DDX_Control(pDX, IDC_PEAK_THR_EDIT, m_PeakThrEdit);
	DDX_Control(pDX, IDC_PEAK_THR_BUTTON, m_PeakThrSetButton);
	DDX_Control(pDX, IDC_SELF_TEST_BUTTON, m_TxSelfTestButton);
	DDX_Control(pDX, IDC_TX8_SOAK_TEST_BUTTON, m_Tx8SoakTestButton);
	DDX_Control(pDX, IDC_DUMP_DIAG_BUTTON, m_DumpDiagsButton);
	DDX_Control(pDX, IDC_CHECK1, Check1);
	DDX_Control(pDX, IDC_CHECK2, Check2);
	DDX_Control(pDX, IDC_CHECK3, Check3);
	DDX_Control(pDX, IDC_CHECK4, Check4);
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
	ON_BN_CLICKED(IDC_DUMP_DIAG_BUTTON, &CS3FactorySetUp::OnBnClickedDumpDiagButton)
	ON_CBN_SELCHANGE(IDC_RX_COMBO, &CS3FactorySetUp::OnCbnSelchangeRxCombo)
	ON_CBN_SELCHANGE(IDC_TX_COMBO, &CS3FactorySetUp::OnCbnSelchangeTxCombo)
	ON_STN_CLICKED(IDC_RX1_CAL_STATIC, &CS3FactorySetUp::OnStnClickedRx1CalStatic)
	ON_BN_CLICKED(IDC_FACT_TEST_BUTTON, &CS3FactorySetUp::OnBnClickedFactTestButton)
	ON_BN_CLICKED(IDC_SELF_TEST_BUTTON, &CS3FactorySetUp::OnBnClickedSelfTestButton)
	ON_BN_CLICKED(IDC_PEAK_THR_BUTTON, &CS3FactorySetUp::OnBnClickedPeakThrButton)
	ON_CBN_DROPDOWN(IDC_RX_COMBO, &CS3FactorySetUp::OnCbnDropdownRxCombo)
	ON_BN_CLICKED(IDC_TX8_SOAK_TEST_BUTTON, &CS3FactorySetUp::OnBnClickedTx8SoakTestButton)
	ON_BN_CLICKED(IDC_CHECK1, &CS3FactorySetUp::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_CHECK2, &CS3FactorySetUp::OnBnClickedCheck2)
	ON_BN_CLICKED(IDC_CHECK3, &CS3FactorySetUp::OnBnClickedCheck3)
	ON_BN_CLICKED(IDC_CHECK4, &CS3FactorySetUp::OnBnClickedCheck4)
END_MESSAGE_MAP()

// ----------------------------------------------------------------------------

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

	m_FactoryRxDroplist.SetCurSel(m_Rx);

	return TRUE;  // return TRUE  unless you set the focus to a control
}


// ----------------------------------------------------------------------------

void CS3FactorySetUp::Init()
{
	S3EventLogAdd("Entering factory calibration screen", 3, -1, -1, -1);

	S3SetSIPRegKey(0); // Enable SIP pop-up
	
	int err = S3I2CTxGetRFCalGain(m_Rx, m_Tx);
	err = S3I2CTxGetOptCalGain(m_Rx, m_Tx);

	m_RFPathCombo.SetCurSel(m_RFPath);

	m_FactoryRxDroplist.SetCurSel(m_Rx);
	m_FactoryTxDroplist.SetCurSel(m_Tx);

	S3SetFactoryMode(m_Rx, m_Tx, true);

	Update();
}

// ----------------------------------------------------------------------------

void CS3FactorySetUp::Update()
{
	CString	tmp;

	if (m_Tx >= 0) // m_Rx must be valid
	{
		for(char RFPath = 0; RFPath < S3_TX_N_RF_PATH; RFPath++)
			m_RFCalVals[RFPath] = (double)S3TxGetCalRF(m_Rx, m_Tx, RFPath) / 100.0;

		tmp.Format(_T("%.2f"), m_RFCalVals[m_RFPath]);
		m_TxCalEdit.SetWindowText(tmp);

		if (	S3TxGetType(m_Rx, m_Tx) != S3_TxUnconnected &&
				S3TxGetPowerStat(m_Rx, m_Tx) == S3_TX_ON)
		{
			tmp.Format(_T("%.2f"), (double)S3TxGetCalOpt(m_Rx, m_Tx) / 100.0);
			m_TxOptCalEdit.SetWindowText(tmp);

			short LinkGain = -100 * Dbg_RF1_DSA + Dbg_TxOpt + Dbg_RxOpt;
			tmp.Format(_T("Path: %d; RFDSA: %d; TxSoftGain: %d;")
								_T("RxSoftGain: %d; LinkGain: %d (100mdB)\nRLL: %d"),
			Dbg_Path, Dbg_RF1_DSA, Dbg_TxOpt, Dbg_RxOpt, LinkGain, Dbg_RLL);
			m_DbgDSAsStatic.SetWindowText(tmp);


			tmp.Format(_T("Peak thresh [%d] (mVp-p):"), S3IPGetPathSent(m_Rx, m_Tx, m_IP));
			m_PeakThrStatic.SetWindowText(tmp);
	
			tmp.Format(_T("%d"), PeakThTable[S3IPGetPathSent(m_Rx, m_Tx, m_IP) - 1]);
			m_PeakThrEdit.SetWindowText(tmp);
		}
		else
		{
			m_TxOptCalEdit.SetWindowText(_T("N/A"));
			m_DbgDSAsStatic.SetWindowText(_T("N/A"));

			m_PeakThrStatic.SetWindowText(_T("Peak thresh [N/A] (mVp-p):"));
			m_PeakThrEdit.SetWindowText(_T("N/A"));
		}
	}
	else
	{
		for(char RFPath = 0; RFPath < S3_TX_N_RF_PATH; RFPath++)
			m_RFCalVals[RFPath] = 0.0;

		m_TxCalEdit.SetWindowText(_T("N/A"));
		m_Rx1CalEdit.SetWindowText(_T("N/A"));

		m_TxOptCalEdit.SetWindowText(_T("N/A"));
		m_DbgDSAsStatic.SetWindowText(_T("N/A"));

		m_PeakThrStatic.SetWindowText(_T("Peak thresh [N/A] (mVp-p):"));
		m_PeakThrEdit.SetWindowText(_T("N/A"));
	}

	if (m_Rx >= 0)
	{
		tmp.Format(_T("%.2f"), (double)S3RxGetCalGain(m_Rx, 0) / 100.0);
		m_Rx1CalEdit.SetWindowText(tmp);

		if (S3RxGetType(m_Rx) == S3_Rx2)
		{
			tmp.Format(_T("%.2f"), (double)S3RxGetCalGain(m_Rx, 1) / 100.0);
			m_Rx2CalEdit.SetWindowText(tmp);
		}
		else
		{
			m_Rx2CalEdit.SetWindowText(_T("N/A"));
		}
	}
	else
	{
		for(char Tx = m_FactoryTxDroplist.GetCount() - 1; Tx >= 0; Tx--)
			m_FactoryTxDroplist.DeleteString(Tx);

		m_Rx1CalEdit.SetWindowText(_T("N/A"));
		m_Rx2CalEdit.SetWindowText(_T("N/A"));
	}

	UpdateRxTxCombos();
	UpdateInfoStatic();
	UpdateChargerEnables();

	Enable(TRUE);
}

// ----------------------------------------------------------------------------

void CS3FactorySetUp::Enable(BOOL enable)
{
	// Disable everything
	m_FactoryRxDroplist.EnableWindow(enable);
	m_FactoryTxDroplist.EnableWindow(enable);

	if (enable)
	{
		m_FactoryRxDroplist.EnableWindow(enable);

		// Conditionally enable
		if (S3RxGetType(m_Rx) == S3_RxEmpty)
		{
			m_FactoryTxDroplist.EnableWindow(FALSE);

			m_Rx1CalEdit.EnableWindow(FALSE);
			m_Rx1CalSetButton.EnableWindow(FALSE);

			m_Rx2CalEdit.EnableWindow(FALSE);
			m_Rx2CalSetButton.EnableWindow(FALSE);

			m_DumpDiagsButton.EnableWindow(FALSE);
		}
		else
		{
			m_Rx1CalEdit.EnableWindow(TRUE);
			m_Rx1CalSetButton.EnableWindow(TRUE);

			m_DumpDiagsButton.EnableWindow(TRUE);

			if (S3RxGetType(m_Rx) == S3_Rx2)
			{
				m_Rx2CalEdit.EnableWindow(TRUE);
				m_Rx2CalSetButton.EnableWindow(TRUE);
			}
			else
			{
				m_Rx2CalEdit.EnableWindow(FALSE);
				m_Rx2CalSetButton.EnableWindow(FALSE);
			}
		}

		if (	S3RxGetType(m_Rx) == S3_RxEmpty ||
				S3TxGetType(m_Rx, m_Tx) == S3_TxUnconnected)
		{
			m_RFPathCombo.EnableWindow(FALSE);
			m_TxCalEdit.EnableWindow(FALSE);
			m_RFCalSetButton.EnableWindow(FALSE);

			m_TxOptCalSetButton.EnableWindow(FALSE);
			m_TxOptCalEdit.EnableWindow(FALSE);

			m_TxSelfTestButton.EnableWindow(FALSE);
			m_Tx8SoakTestButton.EnableWindow(FALSE);
			m_PeakThrEdit.EnableWindow(FALSE);
			m_PeakThrSetButton.EnableWindow(FALSE);
		}
		else
		{
			m_RFPathCombo.EnableWindow(TRUE);
			m_TxCalEdit.EnableWindow(TRUE);
			m_RFCalSetButton.EnableWindow(TRUE);

			m_PeakThrEdit.EnableWindow(TRUE);
			m_PeakThrSetButton.EnableWindow(TRUE);
			
			if (S3TxGetPowerStat(m_Rx, m_Tx) == S3_TX_ON)
			{
				m_TxSelfTestButton.EnableWindow(TRUE);
				m_Tx8SoakTestButton.EnableWindow(TRUE);
				m_TxOptCalSetButton.EnableWindow(TRUE);
				m_TxOptCalEdit.EnableWindow(TRUE);
			}
			else
			{
				m_TxSelfTestButton.EnableWindow(FALSE);
				m_Tx8SoakTestButton.EnableWindow(FALSE);
				m_TxOptCalSetButton.EnableWindow(FALSE);
				m_TxOptCalEdit.EnableWindow(FALSE);
			}
		}
	}
	else
	{
		m_FactoryRxDroplist.EnableWindow(enable);
		m_FactoryTxDroplist.EnableWindow(enable);

		m_Rx1CalEdit.EnableWindow(enable);
		m_Rx1CalSetButton.EnableWindow(enable);

		m_Rx2CalEdit.EnableWindow(enable);
		m_Rx2CalSetButton.EnableWindow(enable);

		m_RFPathCombo.EnableWindow(enable);
		m_TxCalEdit.EnableWindow(enable);
		m_RFCalSetButton.EnableWindow(enable);

		m_TxOptCalSetButton.EnableWindow(enable);
		m_TxCalEdit.EnableWindow(enable);

		m_RFPathCombo.EnableWindow(enable);
		m_TxCalEdit.EnableWindow(enable);
		m_RFCalSetButton.EnableWindow(enable);

		m_DumpDiagsButton.EnableWindow(enable);
	}

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

	double RxCalVal = _tcstod(RxCal, &e);

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
	S3SetSIPRegKey(1); // Disable SIP pop-up

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

	double TxCalVal = _tcstod(TxCal, &e);

	if (*e != '\0')
	{
		m_StatusMsgStatic.SetWindowText(_T("ERROR: Invalid calibration value"));
	}
	else
	{
		int err = S3I2CTxSetRFCalibration(m_Rx, m_Tx, m_RFPath, TxCalVal);

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

	tmp.Format(_T("%.2f"), (double)S3TxGetCalRF(m_Rx, m_Tx, m_RFPath) / 100.0);
	m_TxCalEdit.SetWindowText(tmp);
}

// ----------------------------------------------------------------------------

void CS3FactorySetUp::OnCbnSelchangeRfPathCombo()
{
	CString tmp;
	m_TxCalEdit.GetWindowText(tmp);

	wchar_t	*e;
	double CalVal = _tcstod(tmp, &e);

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

	m_RFCalVals[cs] = _tcstod(TxCal, &e);
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

	double CalVal = _tcstod(RxCal, &e);

	if (*e != '\0')
	{
		m_StatusMsgStatic.SetWindowText(_T("ERROR: Invalid calibration value"));
	}
	else
	{
		int err = S3I2CRxSetCalibration(m_Rx, m_Tx, CalVal);

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
	
	tmp.Format(_T("%.2f"), (double)S3RxGetCalGain(m_Rx, m_Tx) / 100.0);
	m_Rx1CalEdit.SetWindowText(tmp);

	Update();
}

// ----------------------------------------------------------------------------

void CS3FactorySetUp::OnBnClickedRx2CalSetButton()
{
	m_StatusMsgStatic.SetWindowText(_T("Wait..."));
	Enable(FALSE);

	CString	RxCal;
	wchar_t	*e;

	m_Rx2CalEdit.GetWindowText(RxCal);

	double CalVal = _tcstod(RxCal, &e);

	if (*e != '\0')
	{
		m_StatusMsgStatic.SetWindowText(_T("ERROR: Invalid calibration value"));
	}
	else
	{
		int err = S3I2CRxSetCalibration(m_Rx, 1, CalVal);

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
	
	tmp.Format(_T("%.2f"), (double)S3RxGetCalGain(m_Rx, 1) / 100.0);
	m_Rx2CalEdit.SetWindowText(tmp);
	
	Update();
}

// ----------------------------------------------------------------------------

void CS3FactorySetUp::OnBnClickedTxOptCalSetButton()
{
	m_StatusMsgStatic.SetWindowText(_T("Wait..."));
	Enable(FALSE);

	CString	TxCal;
	wchar_t	*e;

	m_TxOptCalEdit.GetWindowText(TxCal);

	double CalVal = _tcstod(TxCal, &e);

	if (*e != '\0')
	{
		m_StatusMsgStatic.SetWindowText(_T("ERROR: Invalid calibration value"));
	}
	else
	{
		int err = S3I2CTxSetOptCalibration(m_Rx, m_Tx, CalVal);

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
	
	tmp.Format(_T("%.2f"), (double)S3TxGetCalOpt(m_Rx, m_Tx) / 100.0);
	m_TxOptCalEdit.SetWindowText(tmp);
}

// ----------------------------------------------------------------------------

void CS3FactorySetUp::OnTcnSelchangeFactoryTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
}

// ----------------------------------------------------------------------------

void CS3FactorySetUp::OnBnClickedDumpDiagButton()
{
	int err;

	err = S3I2CTxDumpCtrlConfig(m_Rx, m_Tx);
	err = S3I2CTxDumpOptConfig(m_Rx, m_Tx);

	err = S3I2CRxDumpCtrlConfig(m_Rx);
	err = S3I2CRxDumpOptConfig(m_Rx);
}

// ----------------------------------------------------------------------------

void CS3FactorySetUp::OnCbnSelchangeRxCombo()
{
	m_Rx = m_FactoryRxDroplist.GetCurSel();
	m_Tx = -1;

	if (S3RxExistQ(m_Rx))
	{
		S3SetFactoryMode(m_Rx, m_Tx, true);
	}
	else
		m_Rx = m_Tx = -1;

	Update();
}

// ----------------------------------------------------------------------------

void CS3FactorySetUp::OnCbnSelchangeTxCombo()
{
	m_Tx = m_FactoryTxDroplist.GetCurSel();
	
	if (S3TxExistQ(m_Rx, m_Tx))
	{
		S3SetFactoryMode(m_Rx, m_Tx, true);
	}
	else m_Tx = -1;
		
	Update();
}

// ----------------------------------------------------------------------------

void CS3FactorySetUp::OnStnClickedRx1CalStatic()
{
	// TODO: Add your control notification handler code here
}

// ----------------------------------------------------------------------------

void CS3FactorySetUp::UpdateRxTxCombos()
{
	CString tmp;

	for(char Rx = m_FactoryRxDroplist.GetCount() - 1; Rx >= 0; Rx--)
		m_FactoryRxDroplist.DeleteString(Rx);

	for(char Rx = 0; Rx < S3_MAX_RXS; Rx++)
	{
		if (S3RxExistQ(Rx))
			tmp.Format(_T("%d"), Rx + 1);
		else
			tmp.Format(_T("%d N/A"), Rx + 1);
			
		m_FactoryRxDroplist.AddString(tmp);
	}

	m_FactoryRxDroplist.SetCurSel(m_Rx);

	for(char Tx = m_FactoryTxDroplist.GetCount() - 1; Tx >= 0; Tx--)
		m_FactoryTxDroplist.DeleteString(Tx);

	if (m_Rx >= 0)
	{
		for(char Tx= 0; Tx < S3RxGetNTx(m_Rx); Tx++)
		{
			if (S3TxExistQ(m_Rx, Tx))
				tmp.Format(_T("%d"), Tx + 1);
			else
				tmp.Format(_T("%d N/A"), Tx + 1);

			m_FactoryTxDroplist.AddString(tmp);
		}
	}

	if (m_Tx >= 0)
		m_FactoryTxDroplist.SetCurSel(m_Tx);
}

// ----------------------------------------------------------------------------

void CS3FactorySetUp::UpdateInfoStatic()
{
	CString tmp;

	if (m_Rx == -1)
		tmp = _T("No Rx selected; ");
	else
		tmp.Format(_T("Rx%d; "), S3RxGetType(m_Rx));

	if (m_Tx == -1)
		tmp.Append(_T("No Tx selected; "));
	else
	{
		tmp.AppendFormat(_T("Tx%d; "), S3TxGetType(m_Rx, m_Tx));

		if (S3TxGetPowerStat(m_Rx, m_Tx) == S3_TX_ON)
			tmp.Append(_T("Awake; "));
		else
			tmp.Append(_T("Asleep; "));

		if (S3RxGetType(m_Rx) == S3_Rx6)
		{
			if (S3RxIsActiveTx(m_Rx, m_Tx))
				tmp.Append(_T("Active; "));
			else
				tmp.Append(_T("NOT Active; "));
		}
	}

	m_RxTxInfoStatic.SetWindowText(tmp);

}

// ----------------------------------------------------------------------------

void CS3FactorySetUp::OnBnClickedFactTestButton()
{
	int err;
	unsigned char rcfg, wcfg;

	err = S3I2CReadSerialByte(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_CFG + 1, &rcfg);

	if (rcfg & 0x01)
		wcfg = rcfg & ~0x01;
	else
		wcfg = rcfg | 0x01;
	
	err = S3I2CWriteSerialByte(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_CFG + 1, wcfg);
}

// ----------------------------------------------------------------------------

extern int S3I2CTxSelfTest(short *v1, short *v2, char Rx, char Tx);

void CS3FactorySetUp::OnBnClickedSelfTestButton()
{
	CString tmp;

	tmp.Format(_T("SelfTest:"));
	m_StatusMsgStatic.SetWindowText(tmp);
	m_StatusMsgStatic.Invalidate();
	m_StatusMsgStatic.UpdateWindow();

	char Rx = 0, Tx = 0;
	short v1, v2;

	S3TxSetSelfTestPending(m_Rx, m_Tx, true);
	int err = S3I2CTxSelfTest(&v1, &v2, m_Rx, m_Tx);

	tmp.Format(_T("SelfTest: %d %d; Err: %d"), v1, v2, err);
	m_StatusMsgStatic.SetWindowText(tmp);
}

// ----------------------------------------------------------------------------

void CS3FactorySetUp::OnBnClickedPeakThrButton()
{
	CString tmp;

	m_PeakThrEdit.GetWindowText(tmp);
	short thr = (short)_wtoi(tmp);

	char path = S3IPGetPathSent(0, 0, 0);

	PeakThTable[path - 1] = thr;

	char Rx = 0, Tx = 0;

	if (S3I2CTxSetPeakThresh(Rx, Tx, path))
	{
		m_StatusMsgStatic.SetWindowText(_T("Failed to set threshold"));
	}
	else
		m_StatusMsgStatic.SetWindowText(_T("Threshold set OK"));
}

// ----------------------------------------------------------------------------

void CS3FactorySetUp::OnCbnDropdownRxCombo()
{
	// In factory mode, so won't pick up any changes (Rx insert/remove)
	// UpdateRxTxCombos();
}

extern int S3I2CTx8SelfTest(short *v1, short *v2, char Rx, char Tx);
extern int S3I2CTx8SoakTest(FILE *fid, short *v1, short *v2, char Rx, char Tx);

// ----------------------------------------------------------------------------

void CS3FactorySetUp::OnBnClickedTx8SoakTestButton()
{
	if (m_Rx < 0 || m_Tx < 0)
		return;

	FILE *fd;
	
	errno_t ferr = fopen_s(&fd, "\\Flashdisk\\Tx8SoakLog.txt", "w");

	CString tmp;

	if (ferr)
	{
		tmp.Format(_T("Tx8SoakTest: File fail"));
		m_StatusMsgStatic.SetWindowText(tmp);
		m_StatusMsgStatic.Invalidate();
		m_StatusMsgStatic.UpdateWindow();
		return;
	}

	tmp.Format(_T("Tx8SoakTest:"));
	m_StatusMsgStatic.SetWindowText(tmp);
	m_StatusMsgStatic.Invalidate();
	m_StatusMsgStatic.UpdateWindow();

	short v1, v2;

	int err = 0;
	int cnt = 0;
	
	while(!err)
	{
		fprintf(fd, "-------------- %04d ---------------\n", cnt++);

		err = S3I2CTx8SoakTest(fd, &v1, &v2, m_Rx, m_Tx);
	}

	if (err)
		fprintf(fd, "Finished with err: %d\n", err);

	fclose(fd);

	tmp.Format(_T("SelfTest: %d %d; Err: %d"), v1, v2, err);
	m_StatusMsgStatic.SetWindowText(tmp);
}

// -----------------------------------------------------------------------------

void CS3FactorySetUp::OnBnClickedCheck1()
{
	if (Check1.GetCheck() == BST_CHECKED)
		S3I2CChEn(0, true);
	else
		S3I2CChEn(0, false);
}

void CS3FactorySetUp::OnBnClickedCheck2()
{
	if (Check2.GetCheck() == BST_CHECKED)
		S3I2CChEn(1, true);
	else
		S3I2CChEn(1, false);
}

void CS3FactorySetUp::OnBnClickedCheck3()
{
	if (Check3.GetCheck() == BST_CHECKED)
		S3I2CChEn(2, true);
	else
		S3I2CChEn(2, false);
}

void CS3FactorySetUp::OnBnClickedCheck4()
{
	if (Check4.GetCheck() == BST_CHECKED)
		S3I2CChEn(3, true);
	else
		S3I2CChEn(3, false);
}

// -----------------------------------------------------------------------------

void CS3FactorySetUp::UpdateChargerEnables()
{
	// Lo is enabled
#ifdef TRIZEPS
	int pins = I2C_ReadRandom(S3I2C_EXPANDER_ADDR, 0x04);

	Check1.SetCheck((pins & EN_BAT_4) ? 0 : 1);
	Check2.SetCheck((pins & EN_BAT_3) ? 0 : 1);
	Check3.SetCheck((pins & EN_BAT_2) ? 0 : 1);
	Check4.SetCheck((pins & EN_BAT_1) ? 0 : 1);
#endif
}

// -----------------------------------------------------------------------------