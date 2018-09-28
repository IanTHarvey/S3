// S3GDIScreenMain.cpp : implementation file
//

#include "stdafx.h"

#include "S3DataModel.h"

#ifdef S3_AGENT
#include "S3Agent/S3AgentDlg.h"
#else
#include "S3ControllerDlg.h"
#endif

#include "S3GDIInfoPopup.h"

// 3 vertical panels
CRect	m_RectSettingsScreen,
			m_RectSettingsRemote,
				m_RectRemoteHeader,
				m_RectEthernet,
				m_RectUSB,
				m_RectOther,
			m_RectSysParas,
				m_RectParasHeader,
				m_RectSettingsSysWide,
				m_RectSettingsLinkPara,
				m_RectSettingsDefaults,
			m_RectSettingsSystem,
				m_RectSystemHeader,
				m_RectSystemDateTime,
				m_RectIdent,
				m_RectSettingsInfo;

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3InitSettingsScreen(void)
{
	CRect rect(0, 0, 80, 40); // Over-ridden if editor attached

	m_GDIIPAddrEdit = new CS3NumEdit(this);

	m_GDIIPAddrEdit->Create(WS_CHILD | ES_LEFT | ES_NOHIDESEL | ES_AUTOHSCROLL,
												rect, this, S3GDI_NUM_EDIT);
	m_GDIIPAddrEdit->SetFont(&m_cFontL);
	m_GDIIPAddrEdit->ShowWindow(SW_HIDE);
	m_GDIIPAddrEdit->m_UpdateImmediate = false;

	m_GDIIPSubnetEdit = new CS3NumEdit(this);

	m_GDIIPSubnetEdit->Create(WS_CHILD | ES_LEFT | ES_NOHIDESEL | ES_AUTOHSCROLL,
												rect, this, S3GDI_NUM_EDIT);
	m_GDIIPSubnetEdit->SetFont(&m_cFontL);
	m_GDIIPSubnetEdit->ShowWindow(SW_HIDE);
	m_GDIIPSubnetEdit->m_UpdateImmediate = false;

	m_GDIIPPortEdit = new CS3NumEdit(this);

	m_GDIIPPortEdit->Create(WS_CHILD | ES_LEFT | ES_NOHIDESEL | ES_AUTOHSCROLL,
												rect, this, S3GDI_NUM_EDIT);
	m_GDIIPPortEdit->SetFont(&m_cFontL);
	m_GDIIPPortEdit->ShowWindow(SW_HIDE);
	m_GDIIPPortEdit->m_UpdateImmediate = false;

	m_GDIDefaultGainEdit = new CS3NumEdit(this);

	m_GDIDefaultGainEdit->Create(WS_CHILD | ES_LEFT, 
												rect, this, S3GDI_NUM2_EDIT);
	m_GDIDefaultGainEdit->SetFont(&m_cFontL);
	m_GDIDefaultGainEdit->ShowWindow(SW_HIDE);
	m_GDIDefaultGainEdit->m_UpdateImmediate = false;

	m_GDIDateEdit = new CS3Edit(this);
	m_GDIDateEdit->Create(WS_CHILD | ES_LEFT, rect, this, S3GDI_DATE_EDIT);
	m_GDIDateEdit->SetFont(&m_cFontL);
	m_GDIDateEdit->ShowWindow(SW_HIDE);

	m_GDITimeEdit = new CS3Edit(this);
	m_GDITimeEdit->Create(WS_CHILD | ES_LEFT, rect, this, S3GDI_TIME_EDIT);
	m_GDITimeEdit->SetFont(&m_cFontL);
	m_GDITimeEdit->ShowWindow(SW_HIDE);

	m_RectSettingsScreen = m_RectScreen;
	m_RectSettingsScreen.top = m_RectHeader.bottom;

	int WCol = (m_RectScreen.right - m_RectScreen.left) / 3;

	// ------- Remote -------
	m_RectSettingsRemote = m_RectSettingsScreen;
	m_RectSettingsRemote.right = WCol;

	m_RectRemoteHeader = m_RectEthernet = m_RectUSB = m_RectSettingsRemote;
	m_RectRemoteHeader.bottom = m_RectRemoteHeader.top + HEAD_ROW;

	// Get the rect to position the edit box
	m_SettingsAccess = new CS3NameValue(	m_Parent, m_RectRemoteHeader.left, 
								m_RectRemoteHeader.bottom, WCol,
								_T("Access"), _T("Remote"), true, S3_ACCESS);
	// m_SettingsAccess->RectEdit(m_HDC, m_hFontSB);

	m_RectEthernet.top = m_RectRemoteHeader.bottom + PARA_ROW;
	m_RectEthernet.bottom = m_RectEthernet.top +
		SUBHEAD_ROW + 4 * PARA_ROW + BMARGIN;

	// Get the rect to position the edit box
	m_SettingsIPAddr = new CS3NameValue(	m_Parent, m_RectEthernet.left, 
								m_RectEthernet.top + SUBHEAD_ROW + 0 * PARA_ROW, WCol,
								_T("IP Address"), _T("255.255.255.2555"), true, S3_IP_ADDRESS);
	m_SettingsIPAddr->RectEdit(m_HDC, m_hFontSB);

	m_SettingsIPSubnet = new CS3NameValue(	m_Parent, m_RectEthernet.left, 
								m_RectEthernet.top + SUBHEAD_ROW + 1 * PARA_ROW, WCol,
								_T("Subnet mask"), _T("000.000.000.0000"), true, S3_IP_SUBNET);
	m_SettingsIPSubnet->RectEdit(m_HDC, m_hFontSB);

	m_SettingsPort = new CS3NameValue(	m_Parent, m_RectEthernet.left, 
								m_RectEthernet.top + SUBHEAD_ROW + 2 * PARA_ROW, WCol,
								_T("Port"), _T("555555"), true, S3_IP_PORT);

	m_SettingsMAC = new CS3NameValue(	m_Parent, m_RectEthernet.left, 
								m_RectEthernet.top + SUBHEAD_ROW + 3 * PARA_ROW, WCol,
								_T("MAC Address"), _T("00:00:00:00:00:00"), false);
	m_SettingsMAC->RectEdit(m_HDC, m_hFontSB);

	m_RectUSB = m_RectEthernet;
	m_RectUSB.top = m_RectEthernet.bottom;
	m_RectUSB.bottom = m_RectUSB.top + SUBHEAD_ROW + 2 * PARA_ROW + BMARGIN;


	m_SettingsUSBPort = new CS3NameValue(	m_Parent, m_RectUSB.left, 
						m_RectUSB.top + SUBHEAD_ROW + 0 * PARA_ROW, WCol,
						_T("Port"), _T("Disabled"), true, S3_USB_ENABLE);

	m_SettingsUSBDriver = new CS3NameValue(	m_Parent, m_RectUSB.left, 
						m_RectUSB.top + SUBHEAD_ROW + 1 * PARA_ROW, WCol,
						_T("Driver type"), _T("UnknownX"), false);
	m_SettingsUSBDriver->RectEdit(m_HDC, m_hFontS);

	m_RectOther = m_RectUSB;
	m_RectOther.top = m_RectUSB.bottom;
	m_RectOther.bottom = m_RectSettingsScreen.bottom;

	m_SettingsTerminator = new CS3NameValue(	m_Parent, m_RectOther.left, 
								m_RectOther.top + SUBHEAD_ROW + 0 * PARA_ROW, WCol,
								_T("Terminator"), _T("None"), true, S3_TERMINATOR);

	// ---- System parameters ------------
	m_RectSysParas = m_RectSettingsScreen;
	m_RectSysParas.left = m_RectSettingsRemote.right;
	m_RectSysParas.right = m_RectSysParas.left + WCol;

	m_RectParasHeader = m_RectSettingsSysWide = m_RectSettingsLinkPara =
		m_RectSettingsDefaults = 
		m_RectSysParas;
	m_RectParasHeader.bottom = m_RectParasHeader.top + HEAD_ROW;

	int yref = m_RectSettingsSysWide.top = m_RectParasHeader.bottom;

	// --- System-wide settings ---
	unsigned char RowCnt = 0;

	m_SettingsContTComp = new CS3NameValue(	m_Parent, m_RectSettingsSysWide.left, 
					yref + SUBHEAD_ROW + RowCnt++ * PARA_ROW, WCol,
					_T("\u03F4 Compensation"), _T("Gain Change"), true, S3_T_COMP_MODE);

	m_SettingsRxAGC = new CS3NameValue(	m_Parent, m_RectSettingsSysWide.left, 
					yref + SUBHEAD_ROW + RowCnt++ * PARA_ROW, WCol,
					_T("Receiver AGC"), _T("Gain Change"), true, S3_GLOBAL_AGC);

#ifndef S3_SHOW_P1DB_MODES
	m_SettingsUnits = new CS3NameValue(	m_Parent, m_RectSettingsDefaults.left, 
					yref + SUBHEAD_ROW + RowCnt++ * PARA_ROW, WCol,
					_T("Units"), _T("dBuV"), true, S3_IP_POWER_UNITS);
#endif

	m_SettingsTxStart = new CS3NameValue(	m_Parent, m_RectSysParas.left, 
					yref + SUBHEAD_ROW + RowCnt++ * PARA_ROW, WCol,
					_T("Tx start state"), _T("Sleep"), true, S3_TX_START_STATE);

	m_SettingsTxSelfTest = new CS3NameValue(	m_Parent, m_RectSysParas.left, 
					yref + SUBHEAD_ROW + RowCnt++ * PARA_ROW, WCol,
					_T("Tx self test"), _T("Off"), true, S3_TX_SELF_TEST);
	m_SettingsTxSelfTest->SetEditable(false);

	m_RectSettingsSysWide.bottom = m_RectSettingsSysWide.top +
		SUBHEAD_ROW + RowCnt * PARA_ROW + BMARGIN;

#ifdef S3_SHOW_P1DB_MODES
	// --------- Link display parameters ---------

	yref = m_RectSettingsLinkPara.top = m_RectSettingsSysWide.bottom;

	RowCnt = 0;

	m_SettingsSize = new CS3NameValue(	m_Parent, m_RectSysParas.left, 
					yref + SUBHEAD_ROW + RowCnt++ * PARA_ROW, WCol,
					_T("Signal magnitude"), _T("Small"), true, S3_IP_SIG_SIZE);

	m_SettingsScale = new CS3NameValue(	m_Parent, m_RectSysParas.left, 
					yref + SUBHEAD_ROW + RowCnt++ * PARA_ROW, WCol,
					_T("Scale"), _T("Log"), true, S3_IP_POWER_SCALE);

	m_SettingsUnits = new CS3NameValue(	m_Parent, m_RectSysParas.left, 
					yref + SUBHEAD_ROW + RowCnt++ * PARA_ROW, WCol,
					_T("Units"), _T("Watts"), true, S3_IP_POWER_UNITS);

	m_Settings3PCLinearity = new CS3NameValue(	m_Parent, m_RectSysParas.left, 
					yref + SUBHEAD_ROW + RowCnt++ * PARA_ROW, WCol,
					_T("3% Linearity"), _T("Off"), true, S3_3PC_LINEARITY);

	m_RectSettingsLinkPara.bottom = m_RectSettingsLinkPara.top +
		SUBHEAD_ROW + RowCnt * PARA_ROW + BMARGIN;
#else
	m_SettingsSize = NULL;
	m_SettingsScale = NULL;
	m_Settings3PCLinearity = NULL;
#endif // S3_SHOW_P1DB_MODES

	// --------- Defaults ---------
	

#ifdef S3_SHOW_P1DB_MODES
	yref = m_RectSettingsDefaults.top = m_RectSettingsLinkPara.bottom;
#else
	yref = m_RectSettingsDefaults.top = m_RectSettingsSysWide.bottom;
#endif

//	m_RectSettingsDefaults.left = m_RectSettingsRemote.right;
//	m_RectSettingsDefaults.right = m_RectSettingsDefaults.left + WCol;

/*
RowCnt = 0;

	m_SettingsGain = new CS3NameValue(	m_Parent, m_RectSettingsDefaults.left, 
					yref + SUBHEAD_ROW + RowCnt++ * PARA_ROW, WCol,
					_T("Gain (dB)"), _T("-55.55"), true, S3_DEF_GAIN);

	m_SettingsImp = new CS3NameValue(	m_Parent, m_RectSettingsDefaults.left, 
					yref + SUBHEAD_ROW + RowCnt++ * PARA_ROW, WCol,
					_T("Input Z (\u03a9)"), _T("1MMMMM"), true, S3_DEF_IMP);

#ifdef S3LOWNOISE
	m_SettingsLowNoise = new CS3NameValue(	m_RectSettingsDefaults.left, 
					yref + SUBHEAD_ROW + RowCnt++ * PARA_ROW, WCol,
					_T("Low noise mode"), _T("OFFFFF"), true, S3_DEF_LOW_NOISE);
#endif
*/

	// ------- System -------

	m_RectSettingsSystem = m_RectSettingsScreen;
	m_RectSettingsSystem.left = m_RectSysParas.right;
	m_RectSettingsSystem.right = m_RectScreen.right;

	m_RectSystemHeader = m_RectSystemDateTime = m_RectIdent = 
		m_RectSettingsInfo = m_RectSettingsSystem;

	m_RectSystemHeader.bottom = m_RectSystemHeader.top + HEAD_ROW;

	m_RectSystemDateTime.top = m_RectSystemHeader.bottom;

	yref = m_RectSystemDateTime.top;

#ifndef S3_AGENT
	m_SettingsDate = new CS3NameValue(m_RectSystemDateTime.left,
			yref + 0 * PARA_ROW, WCol, _T("Date"), _T("YYYY-MM-DD"), true, S3_DATE_EDIT);
	m_SettingsDate->RectEdit(m_HDC, m_hFontSB);

	m_SettingsTime = new CS3NameValue(m_RectSystemDateTime.left,
			yref + 1 * PARA_ROW, WCol, _T("Time"), _T("hhhh:mm:ss"), true, S3_TIME_EDIT);
	m_SettingsTime->RectEdit(m_HDC, m_hFontSB);
#else
	m_SettingsDate = new CS3NameValue(m_RectSystemDateTime.left,
			yref + 0 * PARA_ROW, WCol, _T("Last Update Date"), _T("YYYY-MM-DD"), false);
	m_SettingsDate->RectEdit(m_HDC, m_hFontSB);

	m_SettingsTime = new CS3NameValue(m_RectSystemDateTime.left,
			yref + 1 * PARA_ROW, WCol, _T("Last Update Time"), _T("hhhh:mm:ss"), false);
	m_SettingsTime->RectEdit(m_HDC, m_hFontSB);
#endif

	m_RectSystemDateTime.bottom = m_RectSystemDateTime.top + 2 * PARA_ROW + BMARGIN;

	m_RectIdent.top = m_RectSystemDateTime.bottom;

	yref = m_RectIdent.top + SUBHEAD_ROW;
	CString str = _T("SN00000000000");
	RowCnt = 0;

	m_SettingsModel = new CS3NameValue(m_RectIdent.left,
				yref + RowCnt++ * PARA_ROW, WCol, _T("Model"), str, false);
	m_SettingsModel->RectEdit(m_HDC, m_hFontSB);

	m_SettingsSN = new CS3NameValue(m_RectIdent.left,
				yref + RowCnt++ * PARA_ROW, WCol, _T("S/N"), str, false);
	m_SettingsSN->RectEdit(m_HDC, m_hFontSB);

	m_SettingsPN = new CS3NameValue(m_RectIdent.left,
				yref + RowCnt++ * PARA_ROW, WCol, _T("P/N"), str, false);
	m_SettingsPN->RectEdit(m_HDC, m_hFontSB);

	m_SettingsSW = new CS3NameValue(m_RectIdent.left,
				yref + RowCnt++ * PARA_ROW, WCol, _T("S/W"), str, true, S3_APP_UPDATE);
	m_SettingsSW->RectEdit(m_HDC, m_hFontSB);

	m_SettingsImageDate = new CS3NameValue(m_RectIdent.left,
		yref + RowCnt++ * PARA_ROW, WCol, _T("OS Image"), _T("DD/MM/YY:hh:mm"), true, S3_OS_UPDATE);
	m_SettingsImageDate->RectEdit(m_HDC, m_hFontSB);

	m_SettingsBuildNum = new CS3NameValue(m_RectIdent.left,
		yref + RowCnt++ * PARA_ROW, WCol, _T("Application"), _T("DD/MM/YY:hh:mm"), false);
	m_SettingsBuildNum->RectEdit(m_HDC, m_hFontSB);

	m_RectIdent.bottom = m_RectIdent.top +
		SUBHEAD_ROW + RowCnt++ * PARA_ROW + BMARGIN;

	m_RectSettingsInfo.top = m_RectIdent.bottom;
	
	yref = m_RectSettingsInfo.top + SUBHEAD_ROW;

	m_SettingsCfg = new CS3NameValue(m_RectSettingsInfo.left,
					yref, WCol, _T("Config file"), _T("CfgFileName"), false);
	m_SettingsCfg->RectEdit(m_HDC, m_hFontSB);

	m_SettingsLog = new CS3NameValue(m_RectSettingsInfo.left,
					yref + PARA_ROW, WCol, _T("Log file"), _T("LogFileName"), true, S3_LOG_COPY_USB);
	m_SettingsLog->RectEdit(m_HDC, m_hFontSB);
	
	// Attach an S3NumEdit editors to the settings
	m_SettingsIPAddr->AttachEditor(m_HDC, m_GDIIPAddrEdit);
	m_SettingsIPSubnet->AttachEditor(m_HDC, m_GDIIPSubnetEdit);
	m_SettingsPort->AttachEditor(m_HDC, m_GDIIPPortEdit);
	// m_SettingsGain->AttachEditor(m_HDC, m_GDIDefaultGainEdit);
	m_SettingsDate->AttachEditor(m_HDC, m_GDIDateEdit);
	m_SettingsTime->AttachEditor(m_HDC, m_GDITimeEdit);
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3CloseSettingsScreen(void)
{
	delete m_GDIIPAddrEdit;
	delete m_GDIIPSubnetEdit;
	delete m_GDIIPPortEdit;
	delete m_GDIDefaultGainEdit;

	delete m_GDIDateEdit;
	delete m_GDITimeEdit;

	delete m_SettingsAccess;

	delete m_SettingsPort;
	delete m_SettingsIPAddr;
	delete m_SettingsIPSubnet;
	delete m_SettingsMAC;

	delete m_SettingsUSBPort;
	delete m_SettingsUSBDriver;

	delete m_SettingsTerminator;

	delete m_SettingsContTComp;
	delete m_SettingsRxAGC;

	delete m_SettingsUnits;
#ifdef S3_SHOW_P1DB_MODES
	delete m_SettingsScale;
	delete m_SettingsSize;
	delete m_Settings3PCLinearity;
#endif

	// delete m_SettingsGain;
	// delete m_SettingsImp;
#ifdef S3LOWNOISE
	delete m_SettingsLowNoise;
#endif
	delete m_SettingsTxStart;
	delete m_SettingsTxSelfTest;

	delete m_SettingsDate;
	delete m_SettingsTime;

	delete m_SettingsSN;
	delete m_SettingsPN;
	delete m_SettingsSW;
	delete m_SettingsModel;
	delete m_SettingsImageDate;
	delete m_SettingsBuildNum;

	delete m_SettingsCfg;
	delete m_SettingsLog;
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDISettingsScreen(void)
{
	// Paint a background
	SelectObject(m_HDC, m_hPenNone);

	SelectObject(m_HDC, m_hBrushBG1);
	S3_RECT(m_HDC, m_RectSettingsRemote);

	SelectObject(m_HDC, m_hBrushBG3);
	S3_RECT(m_HDC, m_RectSysParas);

	SelectObject(m_HDC, m_hBrushBG2);
	S3_RECT(m_HDC, m_RectSettingsSystem);

	int xref = m_RectHeader.left;
	int	yref = m_RectHeader.bottom;

	S3DrawGDISettingsRemote();
	S3DrawGDISettingsDefaults();
	S3DrawGDISettingsSystem();

	m_ParaMenu->Draw();
	
	S3DrawGDIBackButton();

	m_NumericPad->Draw();
}

// ----------------------------------------------------------------------------

extern pS3DataModel S3Data;

void CS3GDIScreenMain::S3DrawGDISettingsRemote(void)
{
	SetTextColor(m_HDC, m_crTextNorm);
	SelectObject(m_HDC, m_hFontL);
	
	RECT fntRc = m_RectRemoteHeader;
	
	CString str;
	str.Format(_T("Remote"));

	DrawText(m_HDC, str, -1, &fntRc, DT_CENTER);

	fntRc.top = fntRc.bottom + PARA_ROW;
	fntRc.bottom = fntRc.top + SUBHEAD_ROW;

	SelectObject(m_HDC, m_hFontM);
	SelectObject(m_HDC, m_hBrushBG4);
	
	S3_RECT(m_HDC, fntRc);

	fntRc.left += LHMARGIN;
	
	if (S3GetDHCP())
		DrawText(m_HDC, _T("Ethernet (DHCP)"), -1, &fntRc, DT_LEFT);
	else
		DrawText(m_HDC, _T("Ethernet (Static)"), -1, &fntRc, DT_LEFT);
	
	fntRc = m_RectUSB;
	fntRc.bottom = fntRc.top + SUBHEAD_ROW;

	S3_RECT(m_HDC, fntRc);
	
	fntRc.left += LHMARGIN;
	DrawText(m_HDC, _T("USB Serial"), -1, &fntRc, DT_LEFT);

	fntRc = m_RectOther;
	fntRc.bottom = fntRc.top + SUBHEAD_ROW;

	S3_RECT(m_HDC, fntRc);
	
	fntRc.left += LHMARGIN;
	DrawText(m_HDC, _T("Other"), -1, &fntRc, DT_LEFT);

	SelectObject(m_HDC, m_hFontS);
	SelectObject(m_HDC, m_hBrushBG1);

	int Start = m_RectEthernet.top + HEAD_ROW;

	if (S3GetRemote())
		str = _T("Remote");
	else
		str = _T("Local");

	m_SettingsAccess->SetValue(str);
	m_SettingsAccess->Draw(m_HDC, m_hFontS, m_hFontSB);
	
	char Addr[S3_MAX_IP_ADDR_LEN];

	// S3GetIPAddrStr(Addr);
	str.Format(_T("%S"), S3GetIPAddrStr());

	m_SettingsIPAddr->SetValue(str);
	m_SettingsIPAddr->Draw(m_HDC, m_hFontS, m_hFontSB);

	// S3GetIPSubnetStr(Addr);
	str.Format(_T("%S"), S3GetIPSubnetStr());

	m_SettingsIPSubnet->SetValue(str);
	m_SettingsIPSubnet->Draw(m_HDC, m_hFontS, m_hFontSB);

	unsigned short Port = S3GetIPPort();
	str.Format(_T("%d"), Port);
	
	m_SettingsPort->SetEditable(!S3GetRemote());
	m_SettingsPort->SetValue(str);
	m_SettingsPort->Draw(m_HDC, m_hFontS, m_hFontSB);

	S3GetMACAddrStr(Addr);
	str.Format(_T("%S"), Addr);

	m_SettingsMAC->SetValue(str);
	m_SettingsMAC->Draw(m_HDC, m_hFontS, m_hFontSB);

	m_SettingsTerminator->SetValue(CString(S3GetTerminatorStr()));
	m_SettingsTerminator->Draw(m_HDC, m_hFontS, m_hFontSB);

	SelectObject(m_HDC, m_hBrushBG4);

#ifndef S3_AGENT
	m_SettingsUSBPort->SetEditable(!S3GetRemote());
	m_SettingsUSBPort->SetValue(m_Parent->GetUSBPortName());
	m_SettingsUSBPort->Draw(m_HDC, m_hFontS, m_hFontSB);

	m_SettingsUSBDriver->SetValue(m_Parent->GetUSBDriverType());
	m_SettingsUSBDriver->Draw(m_HDC, m_hFontS, m_hFontSB);
#else
	m_SettingsUSBPort->SetEditable(!S3GetRemote());
	// m_SettingsUSBPort->SetValue(m_Parent->GetUSBPortName());
	m_SettingsUSBPort->Draw(m_HDC, m_hFontS, m_hFontSB);

	m_SettingsUSBDriver->SetValue(CString(S3Data->m_DisplayedUSBDriver));
	m_SettingsUSBDriver->Draw(m_HDC, m_hFontS, m_hFontSB);
#endif
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDISettingsDefaults(void)
{
	SelectObject(m_HDC, m_hFontL);
	CString str;
	str.Format(_T("Settings"));
	DrawText(m_HDC, str, -1, &m_RectParasHeader, DT_CENTER);

	RECT fntRc = m_RectParasHeader;
	fntRc.top = fntRc.bottom;
	fntRc.bottom = fntRc.top + SUBHEAD_ROW;

	SelectObject(m_HDC, m_hFontM);
	SelectObject(m_HDC, m_hBrushBG4);
	
	S3_RECT(m_HDC, fntRc);

	fntRc.left += LHMARGIN;
	DrawText(m_HDC, _T("System"), -1, &fntRc, DT_LEFT);

	unsigned mode = S3GetTCompMode();

	if (S3GetTCompGainOption())
	{
		switch (S3GetTCompMode())
		{
			case S3_TCOMP_OFF: m_SettingsContTComp->SetValue(_T("Off")); break;
			case S3_TCOMP_CONT: m_SettingsContTComp->SetValue(_T("Continuous")); break;
			case S3_TCOMP_GAIN: m_SettingsContTComp->SetValue(_T("Gain Change")); break;
			default: m_SettingsContTComp->SetValue(_T("Error")); break;
		}
	}
	else
	{
		switch (S3GetTCompMode())
		{
			case S3_TCOMP_OFF: m_SettingsContTComp->SetValue(_T("Off")); break;
			case S3_TCOMP_CONT: m_SettingsContTComp->SetValue(_T("On")); break;
			default: m_SettingsContTComp->SetValue(_T("Error")); break;
		}
	}

	m_SettingsContTComp->SetEditable(!S3GetRemote());
	m_SettingsContTComp->Draw(m_HDC, m_hFontS, m_hFontSB);

	switch (S3GetAGC())
	{
		case S3_AGC_OFF: m_SettingsRxAGC->SetValue(_T("Off")); break;
		case S3_AGC_CONT: m_SettingsRxAGC->SetValue(_T("Continuous")); break;
		case S3_AGC_GAIN: m_SettingsRxAGC->SetValue(_T("Gain Change")); break;
		default: m_SettingsRxAGC->SetValue(_T("Error")); break;
	}

	m_SettingsRxAGC->SetEditable(!S3GetRemote());
	m_SettingsRxAGC->Draw(m_HDC, m_hFontS, m_hFontSB);

#ifndef S3_SHOW_P1DB_MODES
	m_SettingsUnits->SetValue(S3GetUnitString());
	m_SettingsUnits->Draw(m_HDC, m_hFontS, m_hFontSB);
#endif

	switch (S3GetTxStartState())
	{
		case S3_TXSTART_USER:	m_SettingsTxStart->SetValue(_T("User")); break;
		case S3_TXSTART_SLEEP:	m_SettingsTxStart->SetValue(_T("Sleep")); break;
		case S3_TXSTART_ON:		m_SettingsTxStart->SetValue(_T("On")); break;
		default:				m_SettingsTxStart->SetValue(_T("Error")); break;
	}

	m_SettingsTxStart->Draw(m_HDC, m_hFontS, m_hFontSB);

	if (S3GetTxSelfTest())
		m_SettingsTxSelfTest->SetValue(_T("On"));
	else
		m_SettingsTxSelfTest->SetValue(_T("Off"));

	m_SettingsTxSelfTest->Draw(m_HDC, m_hFontS, m_hFontSB);

#ifdef S3_SHOW_P1DB_MODES
	fntRc = m_RectSettingsLinkPara;
	fntRc.bottom = fntRc.top + SUBHEAD_ROW;

	S3_RECT(m_HDC, fntRc);

	fntRc.left += LHMARGIN;
	DrawText(m_HDC, _T("Link Display Parameters"), -1, &fntRc, DT_LEFT);

	m_SettingsUnits->SetValue(S3GetUnitString());
	m_SettingsUnits->Draw(m_HDC, m_hFontS, m_hFontSB);

	m_SettingsScale->SetValue(S3GetScaleString());
	m_SettingsScale->Draw(m_HDC, m_hFontS, m_hFontSB);

	m_SettingsSize->SetValue(S3GetSigSizeString());
	m_SettingsSize->Draw(m_HDC, m_hFontS, m_hFontSB);

	if (S3Get3PCLinearity())
		m_Settings3PCLinearity->SetValue(_T("On"));
	else
		m_Settings3PCLinearity->SetValue(_T("Off"));
	m_Settings3PCLinearity->Draw(m_HDC, m_hFontS, m_hFontSB);
#endif
	
/*
	fntRc = m_RectSettingsDefaults;
	fntRc.bottom = fntRc.top + SUBHEAD_ROW;

	S3_RECT(m_HDC, fntRc);

	fntRc.left += LHMARGIN;
	DrawText(m_HDC, _T("Defaults"), -1, &fntRc, DT_LEFT);

	int gain = S3IPGetGain(-1, -1, -1);
	if (gain == 0)
		str = _T("-0");
	else str.Format(_T("%+d"), gain);
	m_SettingsGain->SetValue(str);
	m_SettingsGain->Draw(m_HDC, m_hFontS, m_hFontSB);

	switch(S3GetImpedance(-1, -1, -1))
	{
		case W50: str = _T("50"); break;
		case W1M: str = _T("1M"); break;
		default: str = _T("Unknown");
	};

	m_SettingsImp->SetValue(str);
	m_SettingsImp->Draw(m_HDC, m_hFontS, m_hFontSB);
	
#ifdef S3LOWNOISE
	switch(S3GetLowNoiseMode(-1, -1, -1))
	{
		case true: str = _T("On"); break;
		case false: str = _T("Off"); break;
	};

	m_SettingsLowNoise->SetValue(str);
	m_SettingsLowNoise->Draw(m_HDC, m_hFontS, m_hFontSB);
#endif
*/
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDISettingsSystem(void)
{
	SelectObject(m_HDC, m_hFontL);
	CString str;
	str.Format(_T("System"));
	DrawText(m_HDC, str, -1, &m_RectSystemHeader, DT_CENTER);

	RECT fntRc = m_RectIdent;
	fntRc.bottom = fntRc.top + SUBHEAD_ROW;

	SelectObject(m_HDC, m_hFontM);
	SelectObject(m_HDC, m_hBrushBG4);
	
	S3_RECT(m_HDC, fntRc);

	fntRc.left += LHMARGIN;
	DrawText(m_HDC, _T("Identification"), -1, &fntRc, DT_LEFT);

	fntRc = m_RectSettingsInfo;
	fntRc.bottom = fntRc.top + SUBHEAD_ROW;

	S3_RECT(m_HDC, fntRc);

#ifndef S3_AGENT
	m_Parent->GetDateStr(str);
	m_SettingsDate->SetValue(str);
	m_SettingsDate->Draw(m_HDC, m_hFontS, m_hFontSB);

	m_Parent->GetTimeStr(str);
	m_SettingsTime->SetValue(str);
	m_SettingsTime->Draw(m_HDC, m_hFontS, m_hFontSB);
#else
    str = LastUpdateDateStr;
	m_SettingsDate->SetValue(str);
	m_SettingsDate->Draw(m_HDC, m_hFontS, m_hFontSB);

    str = LastUpdateTimeStr;
	m_SettingsTime->SetValue(str);
	m_SettingsTime->Draw(m_HDC, m_hFontS, m_hFontSB);
#endif

	str.Format(_T("%S"), S3SysGetSN());
	m_SettingsSN->SetValue(str);
	m_SettingsSN->Draw(m_HDC, m_hFontS, m_hFontSB);
	
	str.Format(_T("%S"), S3SysGetPN());
	m_SettingsPN->SetValue(str);
	m_SettingsPN->Draw(m_HDC, m_hFontS, m_hFontSB);

	str.Format(_T("%S"), S3SysGetSW());
	m_SettingsSW->SetValue(str);
#ifndef S3_AGENT
	m_SettingsSW->SetEditable(!S3GetRemote());
#endif
	m_SettingsSW->Draw(m_HDC, m_hFontS, m_hFontSB);
	
	str.Format(_T("%S"), S3SysGetModel());
	m_SettingsModel->SetValue(str);  
	m_SettingsModel->Draw(m_HDC, m_hFontS, m_hFontSB);

	str.Format(_T("%S %S"), S3SysGetImageDate(), S3SysGetImageTime());
	m_SettingsImageDate->SetValue(str);
#ifndef S3_AGENT
	m_SettingsImageDate->SetEditable(!S3GetRemote());
#endif
	m_SettingsImageDate->Draw(m_HDC, m_hFontS, m_hFontSB);

	if (1)
		str.Format(_T("%S"), S3SysGetAppDateTime());
	else
		str.Format(_T("%S"), S3SysGetBuildNum());

	m_SettingsBuildNum->SetValue(str);  
	m_SettingsBuildNum->Draw(m_HDC, m_hFontS, m_hFontSB);
	
	fntRc.left += LHMARGIN;
	DrawText(m_HDC, _T("Information"), -1, &fntRc, DT_LEFT);

	str.Format(_T("%S"), S3GetConfigName());
	m_SettingsCfg->SetValue(str);
	m_SettingsCfg->Draw(m_HDC, m_hFontS, m_hFontSB);
	
	str.Format(_T("%S"), S3GetEventLogName());
	m_SettingsLog->SetValue(str);
	m_SettingsLog->Draw(m_HDC, m_hFontS, m_hFontSB);
}

// ----------------------------------------------------------------------------

CRect CS3GDIScreenMain::DrawNameVal(int xref, int yref,
								   CString Name, CString Val)
{
	CRect fntRc;
	fntRc.left = xref; 
	fntRc.top = yref;

	HGDIOBJ fobj = SelectObject(m_HDC, m_hFontS);

	CString tmp;
	tmp.Format(_T("%s: %s"), Name, Val);

	CRect rect(0, 0, 0, 0);
	
	DrawText(m_HDC, tmp, -1, &rect, DT_CALCRECT);

	fntRc = rect;
	fntRc.MoveToXY(xref, yref);

	DrawText(m_HDC, tmp, -1, &fntRc, DT_LEFT);

	return fntRc;
}

// ----------------------------------------------------------------------------
// Returns the rectangle of the Val string - useful if to be overwritten by
// an edit control and for detecting clickage.

CRect CS3GDIScreenMain::InitNameVal(int xref, int yref,
								   CString Name, CString Val)
{
	CRect fntRc;
	fntRc.left = xref; 
	fntRc.top = yref;

	HGDIOBJ fobj = SelectObject(m_HDC, m_hFontS);

	CString tmp;
	tmp.Format(_T("%s: "), Name);

	CRect rect(0, 0, 0, 0);
	
	DrawText(m_HDC, tmp, -1, &rect, DT_CALCRECT);

	fntRc = rect;
	fntRc.MoveToXY(xref, yref);

	int right = fntRc.right; 

	tmp.Format(_T("%s"), Val);
	rect.SetRectEmpty();
	DrawText(m_HDC, tmp, -1, &rect, DT_CALCRECT);

	fntRc = rect;
	fntRc.MoveToXY(right, yref);

	SelectObject(m_HDC, fobj);

	return fntRc;
}

// ----------------------------------------------------------------------------

int CS3GDIScreenMain::S3FindSettingsScreen(POINT p)
{
	if (m_RectBackButton.PtInRect(p))
	{
		S3GDIChangeScreen(S3_OVERVIEW_SCREEN);

		return 1;
	}
	
	// Is menu up and pointed at?
	// menu_item =	-1: Nothing found
	//				-2: Selection disabled (greyed)
	char menu_item = m_ParaMenu->FindSelect(p);

	m_ParaMenu->Clear();
	m_TxBattInfoPopup->Clear();

	m_GDIDateEdit->ShowWindow(SW_HIDE);
	m_GDITimeEdit->ShowWindow(SW_HIDE);

	// Check for keypad press
	char number = m_NumericPad->Find(p);
	if (number == -3)
		m_NumericPad->PopDown();

	if (number >= 0)
	{
		char DoSomething  = number;
	}
	else if (menu_item != -1 && menu_item != -2)
	{
		// Something already selected, so handle this click accordingly
		char Para = S3GetSelectedPara(-1, -1, -1);

		if (Para == S3_USB_ENABLE)
		{
#ifndef S3_AGENT
			if (m_Parent->GetUSBEnabled())
				m_Parent->SetUSBEnabled(false);
			else
				m_Parent->SetUSBEnabled(true);
#endif
		}
		else if  (Para == S3_T_COMP_MODE)
		{
			if (S3GetTCompGainOption())
			{
				if (menu_item == 3)
#ifndef S3_AGENT
				{
					S3DoComp(-1, -1);
				}
#else
                {
				    CString Command, Args, Response;
                    Command = L"TCOMP";

                    Args.Format(_T(" All"));

                    Command.Append(Args);

                    Response = SendSentinel3Message(Command);
                }
#endif
				else if (menu_item == 0)
					S3SetTCompMode(S3_TCOMP_OFF);
				else if (menu_item == 1)
					S3SetTCompMode(S3_TCOMP_CONT);
				else if (menu_item == 2)
					S3SetTCompMode(S3_TCOMP_GAIN);
			}
			else
			{
				if (menu_item == 0)
					S3SetTCompMode(S3_TCOMP_OFF);
				else if (menu_item == 1)
					S3SetTCompMode(S3_TCOMP_CONT);
			}
		}
		else if  (Para == S3_IP_POWER_UNITS)
		{
			// TODO: Assumes the ordering of units
			S3SetUnits(menu_item + 1);
		}
		/*
		else if (Para == S3_SIGMA_TAU)
		{
			switch(menu_item)
			{
			case 0: S3SetSigmaTau(-1, -1, -1, TauNone); break;
			case 1: S3SetSigmaTau(-1, -1, -1, TauLo); break;
			case 2: S3SetSigmaTau(-1, -1, -1, TauMd); break;
			case 3: S3SetSigmaTau(-1, -1, -1, TauHi); break;
			//	default:
			//		TERMINAL:
			//		S3SetSigmaTau(-1, -1, -1, TUnknown);
			}
		}
		*/
		else if (Para == S3_INPUT_IMP)
		{
			if (menu_item == 0)
				S3SetImpedance(-1, -1, -1, W50);
			else
				S3SetImpedance(-1, -1, -1, W1M);
		}
#ifdef S3LOWNOISE
		else if (Para == S3_LOW_NOISE)
		{
			if (menu_item == 0)
				S3SetLowNoiseMode(-1, -1, -1, true);
			else
				S3SetLowNoiseMode(-1, -1, -1, false);
		}
#endif
		else if (Para == S3_OS_UPDATE)
		{
			// m_MsgID = S3OSSWUpdateRequest();
			m_Screen = S3_OS_UPDATE_SCREEN;
		}
		else if (Para == S3_APP_UPDATE)
		{
			// m_MsgID = S3OSSWUpdateRequest();
			m_Screen = S3_APP_UPDATE_SCREEN;
		}
		else if (Para == S3_LOG_COPY_USB)
		{
			m_Screen = S3_LOG_COPY_SCREEN;
		}
		else if  (Para == S3_ACCESS)
		{
#ifndef S3_AGENT
			if (menu_item == 0)
				S3SetRemote(false);
#else
            CString Command, Response;
			if(S3GetRemote())
            {
                Command = L"LOCAL";
            }
            else
            {
                Command = L"REMOTE";
            }
            Response = SendSentinel3Message(Command);
#endif
		}
		else if  (Para == S3_TX_START_STATE)
		{
			if (menu_item == 0)
				S3SetTxStartState(S3_TXSTART_USER);
			else if (menu_item == 1)
				S3SetTxStartState(S3_TXSTART_SLEEP);
			else if (menu_item == 2)
				S3SetTxStartState(S3_TXSTART_ON);
		}
		else if  (Para == S3_GLOBAL_AGC)
		{
			if (menu_item == 0)
				S3SetAGC(S3_AGC_OFF + S3_PENDING);
			else if (menu_item == 1)
				S3SetAGC(S3_AGC_CONT + S3_PENDING);
			else if (menu_item == 2)
				S3SetAGC(S3_AGC_GAIN + S3_PENDING);
		}
		else if  (Para == S3_TX_SELF_TEST)
		{
			if (menu_item == 0)
				S3SetTxSelfTest(true);
			else if (menu_item == 1)
				S3SetTxSelfTest(false);
		}
#ifdef S3_SHOW_P1DB_MODES
		else if  (Para == S3_IP_POWER_SCALE)
		{
			// TODO: Assumes the ordering of units
			S3SetScale(menu_item + 1);
		}
		else if  (Para == S3_IP_SIG_SIZE)
		{
			// TODO: Assumes the ordering of units
			S3SetSigSize(menu_item + 1);
		}
		else if (Para == S3_3PC_LINEARITY)
		{
			// TODO: Assumes the ordering of units
			S3Set3PCLinearity(menu_item == 1);
		}
#endif
		else if (Para == S3_TERMINATOR)
		{	// TODO: Assumes the ordering
			S3SetTerminator(menu_item);
		}

		return 0;
	}

	// Has something been selected with this click.
	// Set up the menu and indicate the parameter to be edited to
	// be processed by S3SetParaValue()
	char s = CS3NameValue::FindSelectable(p); 
	if (s != -1)
	{
		if (s == S3_IP_PORT) // Ethernet port edit
		{
#ifndef S3_AGENT
            //S3Agent Remote application shouldn't be able to modify the IP PORT

			if (S3GetRemote())
				return 0;
			
			unsigned short Port = S3GetIPPort();

			CString str;
			str.Format(_T("%d"), Port);

			m_GDIIPPortEdit->SetWindowText(str);
			m_GDIIPPortEdit->ShowWindow(SW_SHOWNORMAL);

			m_NumericPad->PopUp(m_HDC, p.x, p.y, m_GDIIPPortEdit,
				S3_NP_POSITIVE | S3_NP_INTEGER, S3_MAX_IP_PORT_LEN);
#endif
			return S3SetSelectedPara(-1, -1, -1, S3_IP_PORT);
		}
		else if (s == S3_USB_ENABLE) // USB enable/disable toggle
		{
#ifndef S3_AGENT
			// S3Agent Remote application shouldn't be able to modify the USB enabled/disabled
			if (S3GetRemote())
				return 0;

			m_ParaMenu->Init(m_HDC, p.x, p.y);
		
			if (m_Parent->GetUSBEnabled())
				m_ParaMenu->AddItem(_T("Disable"));
			else
				m_ParaMenu->AddItem(_T("Enable"));
		
			m_ParaMenu->SelectItem(-1);
			m_ParaMenu->Activate();

			return S3SetSelectedPara(-1, -1, -1, S3_USB_ENABLE);
#endif
		}
		else if (s == S3_IP_POWER_UNITS) // Power units selector
		{
			m_ParaMenu->Init(m_HDC, p.x, p.y);
		
			m_ParaMenu->AddItem(S3GetUnitStrings(S3_UNITS_DBM));
			m_ParaMenu->AddItem(S3GetUnitStrings(S3_UNITS_DBUV));
#ifndef S3_SHOW_P1DB_MODES
			m_ParaMenu->AddItem(S3GetUnitStrings(S3_UNITS_MV));
#endif
			// TODO: Assumes the ordering of units
			m_ParaMenu->SelectItem(S3GetUnits() - 1);
			m_ParaMenu->Activate();

			return S3SetSelectedPara(-1, -1, -1, S3_IP_POWER_UNITS);
		}
		else if (s == S3_DEF_GAIN) // Default gain
		{
			char Gain = S3IPGetGain(-1, -1, -1);

			CString str;

			str.Format(_T("%d"), Gain);

			m_GDIDefaultGainEdit->SetWindowText(str);
			m_GDIDefaultGainEdit->ShowWindow(SW_SHOWNORMAL);

			m_NumericPad->PopUp(m_HDC, p.x, p.y, m_GDIDefaultGainEdit,
				S3_NP_INTEGER, 3);

			// Indicate that a system parameter is being edited
			return S3SetSelectedPara(-1, -1, -1, S3_GAIN);
		}
		else if (s == S3_DEF_IMP) // Input z
		{
			m_ParaMenu->Init(m_HDC, p.x, p.y);
		
			m_ParaMenu->AddItem(_T("50"));
			m_ParaMenu->AddItem(_T("1M"));

			switch(S3GetImpedance(-1, -1, -1))
			{
				case W50: m_ParaMenu->SelectItem(0); break;
				case W1M: m_ParaMenu->SelectItem(1); break;
				default: m_ParaMenu->SelectItem(-1);
			};
				
			m_ParaMenu->Activate();

			return S3SetSelectedPara(-1, -1, -1, S3_INPUT_IMP);
		}
		else if (s == S3_APP_UPDATE) // App version
		{
#ifndef S3_AGENT
			if (S3GetRemote())
				return 0;
#endif

			m_ParaMenu->Init(m_HDC, p.x, p.y);
			m_ParaMenu->AddItem(_T("Update App"));
			m_ParaMenu->Activate();

			return S3SetSelectedPara(-1, -1, -1, S3_APP_UPDATE);
		}
		else if (s == S3_T_COMP_MODE) // Temp compensation mode
		{
#ifndef S3_AGENT
			if (S3GetRemote())
				return 0;
#endif
			m_ParaMenu->Init(m_HDC, p.x, p.y);
		
			if (S3GetTCompGainOption())
			{
				m_ParaMenu->AddItem(_T("Off"));
				m_ParaMenu->AddItem(_T("Continuous"));
				m_ParaMenu->AddItem(_T("Gain Change"));

				// TODO: Assumes the ordering of items
				if (S3GetTCompMode() == S3_TCOMP_OFF)
				{
					m_ParaMenu->SelectItem(0);
				}
				else if (S3GetTCompMode() == S3_TCOMP_CONT)
				{
					m_ParaMenu->SelectItem(1);
				}
				else if (S3GetTCompMode() == S3_TCOMP_GAIN)
				{
					m_ParaMenu->SelectItem(2);
					m_ParaMenu->AddItem(_T("Do now"));
				}
			}
			else
			{
				m_ParaMenu->AddItem(_T("Off"));
				m_ParaMenu->AddItem(_T("On"));

				// TODO: Assumes the ordering of items
				if (S3GetTCompMode() == S3_TCOMP_OFF)
					m_ParaMenu->SelectItem(0);
				else if (S3GetTCompMode() == S3_TCOMP_CONT)
					m_ParaMenu->SelectItem(1);
			}
				
			m_ParaMenu->Activate();

			return S3SetSelectedPara(-1, -1, -1, S3_T_COMP_MODE);
		}
		else if (s == S3_LOG_COPY_USB) // Copy log to USB
		{
			m_ParaMenu->Init(m_HDC, p.x, p.y);
			m_ParaMenu->AddItem(_T("Copy to USB"));
			m_ParaMenu->Activate();

			return S3SetSelectedPara(-1, -1, -1, S3_LOG_COPY_USB);
		}
		else if (s == S3_DATE_EDIT) // Date edit
		{
			// Disallow the user from changing the date & time remotely
			// this can still be done using the manual SYSSETTIME command
#ifndef S3_AGENT
			CString str;

			m_Parent->GetDateStr(str);

			m_GDIDateEdit->SetWindowText(str);
			m_GDIDateEdit->ShowWindow(SW_SHOWNORMAL);

			return S3SetSelectedPara(-1, -1, -1, S3_DATE_EDIT);
#endif
		}
		else if (s == S3_TIME_EDIT) // Time edit
		{
			// Disallow the user from changing the date & time remotely
            // this can still be done using the manual SYSSETTIME command
#ifndef S3_AGENT
			CString str;

			m_Parent->GetTimeStr(str);

			m_GDITimeEdit->SetWindowText(str);
			m_GDITimeEdit->ShowWindow(SW_SHOWNORMAL);

			return S3SetSelectedPara(-1, -1, -1, S3_TIME_EDIT);
#endif
		}
		else if (s == S3_ACCESS) // Access
		{
			m_ParaMenu->Init(m_HDC, p.x, p.y);
		
			// Only change remote->local here
			if (S3GetRemote())
			{
				m_ParaMenu->AddItem(_T("Local"));

				m_ParaMenu->Activate();

				return S3SetSelectedPara(-1, -1, -1, S3_ACCESS);
			}
		}
		else if (s == S3_TX_START_STATE) // Tx start state
		{
			m_ParaMenu->Init(m_HDC, p.x, p.y);
		
			m_ParaMenu->AddItem(_T("User"));
			m_ParaMenu->AddItem(_T("Sleep"));
			m_ParaMenu->AddItem(_T("On"));
				
			m_ParaMenu->SelectItem(S3GetTxStartState());
			m_ParaMenu->Activate();

			return S3SetSelectedPara(-1, -1, -1, S3_TX_START_STATE);
		}
		else if (s == S3_GLOBAL_AGC) // Global Rx AGC setting
		{
#ifndef S3_AGENT		
			if (S3GetRemote())
				return 0;
#endif

			m_ParaMenu->Init(m_HDC, p.x, p.y);
		
			m_ParaMenu->AddItem(_T("Off"));
			m_ParaMenu->AddItem(_T("Continuous"));
			m_ParaMenu->AddItem(_T("Gain Change"));

			m_ParaMenu->SelectItem(S3GetAGC());
			m_ParaMenu->Activate();

			return S3SetSelectedPara(-1, -1, -1, S3_GLOBAL_AGC);
		}
		else if (s == S3_OS_UPDATE) // OS version
		{
			if (S3GetRemote())
				return 0;

			m_ParaMenu->Init(m_HDC, p.x, p.y);
		
			m_ParaMenu->AddItem(_T("Update Image"));
			m_ParaMenu->Activate();

			return S3SetSelectedPara(-1, -1, -1, S3_OS_UPDATE);
		}
		else if (s == S3_TX_SELF_TEST) // Tx self test
		{
			m_ParaMenu->Init(m_HDC, p.x, p.y);
		
			m_ParaMenu->AddItem(_T("On"));
			m_ParaMenu->AddItem(_T("Off"));
		
			if (S3GetTxSelfTest())
				m_ParaMenu->SelectItem(0);
			else
				m_ParaMenu->SelectItem(1);

			m_ParaMenu->Activate();

			return S3SetSelectedPara(-1, -1, -1, S3_TX_SELF_TEST);
		}
#ifdef S3_SHOW_P1DB_MODES
		else if (s == S3_IP_POWER_SCALE) // Units scale
		{
			m_ParaMenu->Init(m_HDC, p.x, p.y);
		
			m_ParaMenu->AddItem(ScaleStrings[0]);
			m_ParaMenu->AddItem(ScaleStrings[1]);

			// TODO: Assumes the ordering of units
			m_ParaMenu->SelectItem(S3GetScale() - 1);
			m_ParaMenu->Activate();

			return S3SetSelectedPara(-1, -1, -1, S3_IP_POWER_SCALE);
		}
		else if (s == S3_IP_SIG_SIZE) // Signal size
		{
			m_ParaMenu->Init(m_HDC, p.x, p.y);
		
			m_ParaMenu->AddItem(SigSizeStrings[0]);
			m_ParaMenu->AddItem(SigSizeStrings[1]);

			// TODO: Assumes the ordering of units
			m_ParaMenu->SelectItem(S3GetSigSize() - 1);
			m_ParaMenu->Activate();

			return S3SetSelectedPara(-1, -1, -1, S3_IP_SIG_SIZE);
		}
		else if (s == S3_3PC_LINEARITY) // Show 3PC linearity valuse
		{
			m_ParaMenu->Init(m_HDC, p.x, p.y);
		
			m_ParaMenu->AddItem(_T("Off"));
			m_ParaMenu->AddItem(_T("On"));

			// TODO: Assumes the ordering of units
			if (S3Get3PCLinearity())
				m_ParaMenu->SelectItem(1);
			else
				m_ParaMenu->SelectItem(0);
				
			m_ParaMenu->Activate();

			return S3SetSelectedPara(-1, -1, -1, S3_3PC_LINEARITY);
		}
#endif // S3_SHOW_P1DB_MODES
		else if (s == S3_TERMINATOR) // Remote command response terminator
		{
			m_ParaMenu->Init(m_HDC, p.x, p.y);
			m_ParaMenu->AddItem(_T("\\n"));
			m_ParaMenu->AddItem(_T("\\0"));
			m_ParaMenu->AddItem(_T("None"));
		
			switch(S3GetTerminator())
			{
				case 0: m_ParaMenu->SelectItem(0); break;
				case 1: m_ParaMenu->SelectItem(1); break;
				case 2: m_ParaMenu->SelectItem(2); break;
			}

			m_ParaMenu->Activate();

			return S3SetSelectedPara(-1, -1, -1, S3_TERMINATOR);
		}
		else if (s == S3_IP_ADDRESS) // Ethernet address edit
		{
#ifndef S3_AGENT
            //S3Agent Remote application shouldn't be able to modify the IP PORT

			if (S3GetRemote())
				return 0;
			
			// unsigned short Port = S3GetIPPort();

			CString str;
			// char cStr[S3_MAX_IP_ADDR_LEN];
			// S3GetIPAddrStr(cStr);
			str.Format(_T("%S"), S3GetIPAddrStr());

			if (!wcscmp(str, _T("No Ethernet")))
				str = _T("");

			m_GDIIPAddrEdit->SetWindowText(str);
			m_GDIIPAddrEdit->ShowWindow(SW_SHOWNORMAL);

			m_NumericPad->PopUp(m_HDC, p.x, p.y, m_GDIIPAddrEdit,
				S3_NP_POSITIVE, S3_MAX_IP_ADDRESS_LEN - 1);
#endif
			return S3SetSelectedPara(-1, -1, -1, S3_IP_ADDRESS);
		}
		else if (s == S3_IP_SUBNET) // Ethernet port edit
		{
#ifndef S3_AGENT
            //S3Agent Remote application shouldn't be able to modify the IP PORT

			if (S3GetRemote())
				return 0;
			
			// unsigned short Port = S3GetIPPort();

			CString str;
			//char cStr[S3_MAX_IP_ADDR_LEN];;
			//S3GetIPSubnetStr(cStr);
			str.Format(_T("%S"), S3GetIPSubnetStr());

			m_GDIIPSubnetEdit->SetWindowText(str);
			m_GDIIPSubnetEdit->ShowWindow(SW_SHOWNORMAL);

			m_NumericPad->PopUp(m_HDC, p.x, p.y, m_GDIIPSubnetEdit,
				S3_NP_POSITIVE, S3_MAX_IP_ADDRESS_LEN - 1);
#endif
			return S3SetSelectedPara(-1, -1, -1, S3_IP_SUBNET);
		}

		return 0;
	}
	else if (menu_item == -2)
	{
		// This is a click on the menu that we don't want to process
	}
	else if (m_RectSettingsSystem.PtInRect(p))
	{
		// m_NumericPad->PopUp(m_HDC, p.x, p.y, NULL);
	}

	return 0;
}

// ----------------------------------------------------------------------------