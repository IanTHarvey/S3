// S3FactorySysSetUp.cpp : implementation file
//

#include "stdafx.h"

#include "S3DataModel.h"

#ifdef TRIZEPS
#include "drvLib_app.h"
#endif

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
	DDX_Control(pDX, IDC_SEAL_BUTTON, m_SealBatteryButton);
	DDX_Control(pDX, IDC_BATTERY_SN_EDIT, m_BatterySNEdit);
	DDX_Control(pDX, IDC_BATTERY_TYPE_COMBO, m_BatteryTypeCombo);
	DDX_Control(pDX, IDC_UNSEAL_BUTTON, m_UnsealBatteryButton);
}

BEGIN_MESSAGE_MAP(CS3FactorySysSetUp, CDialog)
	ON_BN_CLICKED(IDOK, &CS3FactorySysSetUp::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CS3FactorySysSetUp::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_SCRN_OFFSET_SET_BUTTON, &CS3FactorySysSetUp::OnBnClickedScrnOffsetSetButton)
	ON_BN_CLICKED(IDC_FACT_SN_SET_BUTTON, &CS3FactorySysSetUp::OnBnClickedFactSnSetButton)
	ON_BN_CLICKED(IDC_FACT_PN_SET_BUTTON, &CS3FactorySysSetUp::OnBnClickedFactPnSetButton)
	ON_BN_CLICKED(IDC_TEST_BUTTON, &CS3FactorySysSetUp::OnBnClickedTestButton)
	ON_BN_CLICKED(IDC_SEAL_BUTTON, &CS3FactorySysSetUp::OnBnClickedSealButton)
	ON_BN_CLICKED(IDC_UNSEAL_BUTTON, &CS3FactorySysSetUp::OnBnClickedUnsealButton)
	ON_BN_CLICKED(IDC_BATT_SN_SET_BUTTON, &CS3FactorySysSetUp::OnBnClickedBattSnSetButton)
END_MESSAGE_MAP()

BOOL CS3FactorySysSetUp::OnInitDialog()
{
	CDialog::OnInitDialog();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// ----------------------------------------------------------------------------

void CS3FactorySysSetUp::Init()
{
	S3EventLogAdd("Entering factory system set-up screen", 3, -1, -1, -1);

	S3SetSIPRegKey(0); // Enable SIP pop-up

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

	m_StatusMsgStatic.SetWindowText(_T(""));

	Update();
}

// ----------------------------------------------------------------------------

void CS3FactorySysSetUp::Update()
{
	char Ch = 0;
	bool occupied = S3ChOccupied(Ch);

	m_SealBatteryButton.EnableWindow(occupied);
	m_UnsealBatteryButton.EnableWindow(occupied);
	m_SealBatteryButton.EnableWindow(occupied);
	m_BatterySNEdit.EnableWindow(occupied);
	m_BatteryTypeCombo.EnableWindow(occupied);

	if (occupied)
	{
		CString tmp;

		tmp.Format(_T("%S"), S3ChGetBattSN(Ch));

		m_BatterySNEdit.SetWindowText(tmp);

		if (S3ChGetBattType(Ch) == S3_BattUnknown)
			m_BatteryTypeCombo.SetCurSel(0);
		else if (S3ChGetBattType(Ch) == S3_Batt2S1P)
			m_BatteryTypeCombo.SetCurSel(1);
		else if (S3ChGetBattType(Ch) == S3_Batt2S2P)
			m_BatteryTypeCombo.SetCurSel(2);
	}
	else
	{
		m_BatterySNEdit.SetWindowText(_T(""));
		m_BatteryTypeCombo.SetCurSel(0);
	}

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
	S3SetSIPRegKey(1); // Disable SIP pop-up

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

void CS3FactorySysSetUp::OnBnClickedTestButton()
{
	CString tmp;

	tmp.Format(_T("Authenticate:"));
	m_StatusMsgStatic.SetWindowText(tmp);
	m_StatusMsgStatic.Invalidate();
	m_StatusMsgStatic.UpdateWindow();

	char Rx = 0, Tx = 0;
	// int AuthCh = S3I2CChAuthenticate();
	int AuthTx = S3I2CTxAuthenticate(Rx, Tx);

	tmp.Format(_T("Authenticate: Err: %d"), AuthTx);
	m_StatusMsgStatic.SetWindowText(tmp);
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

		tmp.Format(_T("Failed to secure battery: Err %d"), err);
		m_StatusMsgStatic.SetWindowText(tmp);
	}
	else
	{
		if (S3I2CChAuthenticate(Ch))
			m_StatusMsgStatic.SetWindowText(_T("Authentication failed"));
		else if (S3I2CChSetBattSealed(Ch))
			m_StatusMsgStatic.SetWindowText(_T("Seal command failed"));
		else
		{
			m_StatusMsgStatic.SetWindowText(_T("Battery sealed OK"));
			S3ChRemove(Ch);
		}
	}
}

// ----------------------------------------------------------------------------

void CS3FactorySysSetUp::OnBnClickedUnsealButton()
{
	char Ch = 0;

	if (S3I2CChSetBattUnseal())
	{
		m_StatusMsgStatic.SetWindowText(_T("Unseal failed"));
		return;
	}
	
	Sleep(100);
	
	if (S3I2CChSetBattFullAccess())
		m_StatusMsgStatic.SetWindowText(_T("Allow full access failed"));
	else
	{
		// Attempt to verify operation. Attempting this actually
		// causes a failure to unseal.
		if (0)
		{
			Sleep(1000);

#ifdef TRIZEPS
			// Do reset
			unsigned char cmd[3];
		
			cmd[0] = 0x00;
			cmd[1] = 0x41;
			cmd[2] = 0x00;
			int ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 3, NULL, 0);

			if (!ok)
			{
				m_StatusMsgStatic.SetWindowText(_T("Failed to reset battery"));
				return;
			}

			Sleep(1000);
#endif

			if (S3I2CChGetStatus(Ch))
			{
				m_StatusMsgStatic.SetWindowText(_T("Failed to read back battery status"));
				return;
			}

			if (S3ChGetBattStatus(Ch) & BQ_SS)
			{
				m_StatusMsgStatic.SetWindowText(_T("Battery seal flag not cleared"));
				return;
			}

			if (S3ChGetBattStatus(Ch) & BQ_FAS)
			{
				m_StatusMsgStatic.SetWindowText(_T("Full access flag not cleared"));
				return;
			}
		}

		m_StatusMsgStatic.SetWindowText(_T("Full access enabled"));
		S3ChRemove(Ch);
	}

	return;
}

// ----------------------------------------------------------------------------

void CS3FactorySysSetUp::OnBnClickedBattSnSetButton()
{
	char Ch = 0;
	if (!S3ChOccupied(Ch))
	{
		m_StatusMsgStatic.SetWindowText(_T("No battery on charge port 1"));
		return;
	}

	char SN[S3_MAX_SN_LEN], PN[S3_MAX_PN_LEN];

	CString tmp;

	m_BatterySNEdit.GetWindowText(tmp);

	if (tmp.GetLength() < 7)
	{
		m_StatusMsgStatic.SetWindowText(_T("Invalid serial number"));
		return;
	}

	sprintf_s(SN, S3_MAX_SN_LEN, "%S", (LPCTSTR)tmp);
	
	if (m_BatteryTypeCombo.GetCurSel() == 1)
		strcpy_s(PN, S3_MAX_PN_LEN, "S3-BAT-1P-00");
	else if (m_BatteryTypeCombo.GetCurSel() == 2)
		strcpy_s(PN, S3_MAX_PN_LEN, "S3-BAT-2P-00");
	else
	{
		m_StatusMsgStatic.SetWindowText(_T("Type not selected"));
		return;
	}

	S3I2CChMS(Ch);

	int err = S3I2CChWriteSNPN(Ch, SN, PN);

	if (err == 0)
	{
		m_StatusMsgStatic.SetWindowText(_T("Write succeeded"));
		S3ChRemove(Ch);
	}
	else if (err == 1 || err == 4)
	{
		m_StatusMsgStatic.SetWindowText(_T("Write failed"));
	}
	else if (err == 2)
	{
		m_StatusMsgStatic.SetWindowText(_T("Invalid serial or part number number"));
	}
	else if (err == 3)
	{
		m_StatusMsgStatic.SetWindowText(_T("Battery is sealed"));
	}
}

// ----------------------------------------------------------------------------
