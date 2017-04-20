// S3FactorySysSetUp.cpp : implementation file
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
#include "S3GPIB.h"
#include "S3USBVCP.h"
#include "S3I2C.h"

#include "S3ControllerDlg.h"
#include "S3FactorySysSetUp.h"


// CS3FactorySysSetUp

IMPLEMENT_DYNAMIC(CS3FactorySysSetUp, CDialog)

CS3FactorySysSetUp::CS3FactorySysSetUp(CWnd* pParent)
	: CDialog(CS3FactorySysSetUp::IDD)
{
	m_Parent = (CS3ControllerDlg *)pParent;
}

CS3FactorySysSetUp::~CS3FactorySysSetUp()
{
}

void CS3FactorySysSetUp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SCRN_OFFSET_X_EDIT, m_ScrnOffsetX);
	DDX_Control(pDX, IDC_SCRN_OFFSET_Y_EDIT, m_ScrnOffsetY);
	DDX_Control(pDX, IDC_STATUS_MSG_SYS_STATIC, m_StatusMsgStatic);
	DDX_Control(pDX, IDC_FACT_SN_EDIT, m_FactSNEdit);
	DDX_Control(pDX, IDC_FACT_PN_EDIT, m_FactPNEdit);
	DDX_Control(pDX, IDC_PEAK_THR_STATIC, m_PeakThrStatic);
	DDX_Control(pDX, IDC_PEAK_THR_EDIT, m_PeakThrEdit);
}

BEGIN_MESSAGE_MAP(CS3FactorySysSetUp, CDialog)
	ON_BN_CLICKED(IDOK, &CS3FactorySysSetUp::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CS3FactorySysSetUp::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_SCRN_OFFSET_SET_BUTTON, &CS3FactorySysSetUp::OnBnClickedScrnOffsetSetButton)
	ON_BN_CLICKED(IDC_FACT_SN_SET_BUTTON, &CS3FactorySysSetUp::OnBnClickedFactSnSetButton)
	ON_BN_CLICKED(IDC_FACT_PN_SET_BUTTON, &CS3FactorySysSetUp::OnBnClickedFactPnSetButton)
	ON_BN_CLICKED(IDC_PEAK_THR_BUTTON, &CS3FactorySysSetUp::OnBnClickedPeakThrButton)
	ON_BN_CLICKED(IDC_TEST_BUTTON, &CS3FactorySysSetUp::OnBnClickedTestButton)
	ON_BN_CLICKED(IDC_SEAL_BUTTON, &CS3FactorySysSetUp::OnBnClickedSealButton)
	ON_BN_CLICKED(IDC_SELF_TEST_BUTTON, &CS3FactorySysSetUp::OnBnClickedSelfTestButton)
END_MESSAGE_MAP()

BOOL CS3FactorySysSetUp::OnInitDialog()
{
	CDialog::OnInitDialog();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// ----------------------------------------------------------------------------

extern int SetSIPRegKey(DWORD data);
extern short PeakThTable[];

// ----------------------------------------------------------------------------

void CS3FactorySysSetUp::Init()
{
	S3EventLogAdd("Entering factory system set-up screen", 3, -1, -1, -1);

	SetSIPRegKey(0); // Enable SIP pop-up

	CString	tmp;

	short x, y;
	S3GetScreenOffsets(&x, &y);

	tmp.Format(_T("%d"), x);
	m_ScrnOffsetX.SetWindowText(tmp);

	tmp.Format(_T("%d"), y);
	m_ScrnOffsetY.SetWindowText(tmp);

	tmp.Format(_T("%S"), S3SysGetSN());
	m_FactSNEdit.SetWindowText(tmp);

	tmp.Format(_T("%S"), S3SysGetPN());
	m_FactPNEdit.SetWindowText(tmp);

	// Peak thresh
	tmp.Format(_T("Peak thresh [%d] (mVp-p):"), S3IPGetPathSent(0, 0, 0));
	m_PeakThrStatic.SetWindowText(tmp);

	tmp.Format(_T("%d"), PeakThTable[S3IPGetPathSent(0, 0, 0) - 1]);
	m_PeakThrEdit.SetWindowText(tmp);

	m_StatusMsgStatic.SetWindowText(_T(""));
}

// ----------------------------------------------------------------------------
// CS3FactorySysSetUp diagnostics

#ifdef _DEBUG
void CS3FactorySysSetUp::AssertValid() const
{
	CDialog::AssertValid();
}

/*
#ifndef _WIN32_WCE
void CS3FactorySysSetUp::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
*/
#endif //_DEBUG


// ----------------------------------------------------------------------------
// CS3FactorySysSetUp message handlers

void CS3FactorySysSetUp::OnBnClickedOk()
{
}

// ----------------------------------------------------------------------------

void CS3FactorySysSetUp::OnBnClickedCancel()
{
	SetSIPRegKey(1); // Disable SIP pop-up

	m_Parent->HideFactory();
	OnCancel();
}

// ----------------------------------------------------------------------------
// See:
// http://stackoverflow.com/questions/944033/is-this-a-memory-leak-in-mfc/944190#944190

void CS3FactorySysSetUp::PostNcDestroy() 
{
    CDialog::PostNcDestroy();
    delete this;
}

// ----------------------------------------------------------------------------

void CS3FactorySysSetUp::OnBnClickedScrnOffsetSetButton()
{
	short x, y;

	CString tmp;

	m_ScrnOffsetX.GetWindowText(tmp);
	x = (short)_wtoi(tmp);

	m_ScrnOffsetY.GetWindowText(tmp);
	y = (short)_wtoi(tmp);

	if (x < -16 || x > 16)
		m_StatusMsgStatic.SetWindowText(_T("Offsets must be > -16 & < 16"));
	else if (y < -16 || y > 16)
		m_StatusMsgStatic.SetWindowText(_T("Offsets must be > -16 & < 16"));
	else if (S3WriteScreenOffsets(x, y))
		m_StatusMsgStatic.SetWindowText(_T("Failed to save offset file"));
	else
	{
		m_StatusMsgStatic.SetWindowText(_T("Restart to take effect"));

		// TODO: Re-initialising the layout is not yet feasible:
		//	Separate layout reinitialisation from the rest
		//	Close and re-init screens - need to delete all GDI objects on close
		//	Re-start the app
		//
		// m_Parent->S3GDIReInitialise();
	}
}

// ----------------------------------------------------------------------------

void CS3FactorySysSetUp::OnBnClickedFactPnSetButton()
{
	CString PNTxt;
	m_FactPNEdit.GetWindowText(PNTxt);

	char PN[S3_MAX_PN_LEN];
	sprintf_s(PN, "%S", (LPCTSTR)PNTxt);

	S3SysSetPN(PN);
	
	if (S3SysWritePN())
	{	
		m_StatusMsgStatic.SetWindowText(_T("Failed to save part number"));
		S3SysSetPN("Unknown");
	}
	else
		m_StatusMsgStatic.SetWindowText(_T("Part number saved"));
}

// ----------------------------------------------------------------------------

void CS3FactorySysSetUp::OnBnClickedFactSnSetButton()
{
	CString SNTxt;
	m_FactSNEdit.GetWindowText(SNTxt);

	char SN[S3_MAX_SN_LEN];
	sprintf_s(SN, "%S", (LPCTSTR)SNTxt);

	S3SysSetSN(SN);
	
	if (S3SysWriteSN())
	{	
		m_StatusMsgStatic.SetWindowText(_T("Failed to save serial number"));
		S3SysSetSN("Unknown");
	}
	else
		m_StatusMsgStatic.SetWindowText(_T("Serial number saved"));
}

// ----------------------------------------------------------------------------

void CS3FactorySysSetUp::OnBnClickedPeakThrButton()
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

void CS3FactorySysSetUp::OnBnClickedTestButton()
{
	// int AuthCh = S3I2CChAuthenticate();
	int AuthTx = S3I2CTxAuthenticate();
}

// ----------------------------------------------------------------------------

void CS3FactorySysSetUp::OnBnClickedSealButton()
{
	char Ch = 0;

	if (S3ChGetBattStatus(Ch) & BQ_SS)
	{
		m_StatusMsgStatic.SetWindowText(_T("Battery already sealed"));
		return;
	}

	if (S3ChGetBattStatus(Ch) & BQ_FAS)
	{
		m_StatusMsgStatic.SetWindowText(_T("Full access required"));
		return;
	}

	int err = S3I2CChWriteSecKeys();

	if (err)
	{
		CString tmp;

		tmp.Format(_T("Failed to seal battery: Err %d"), err);
		m_StatusMsgStatic.SetWindowText(tmp);
	}
	else
	{
		if (S3I2CChAuthenticate())
			m_StatusMsgStatic.SetWindowText(_T("Authentication failed"));
		else if (S3I2CChSetBattSealed(Ch))
			m_StatusMsgStatic.SetWindowText(_T("Seal command failed"));
		else
			m_StatusMsgStatic.SetWindowText(_T("Battery sealed OK"));
	}
}

// ----------------------------------------------------------------------------

extern int S3I2CTxSelfTest(short *v1, short *v2, char Rx, char Tx);

void CS3FactorySysSetUp::OnBnClickedSelfTestButton()
{
	CString tmp;

	tmp.Format(_T("SelfTest:"));
	m_StatusMsgStatic.SetWindowText(tmp);
	m_StatusMsgStatic.Invalidate();
	m_StatusMsgStatic.UpdateWindow();

	char Rx = 0, Tx = 0;
	short v1, v2;

	int err = S3I2CTxSelfTest(&v1, &v2, Rx, Tx);

	tmp.Format(_T("SelfTest: %d %d; Err: %d"), v1, v2, err);
	m_StatusMsgStatic.SetWindowText(tmp);
}

// ----------------------------------------------------------------------------
