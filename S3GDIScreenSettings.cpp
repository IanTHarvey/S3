// S3GDIScreenMain.cpp : implementation file
//

//
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef TRIZEPS
#include "S3ControllerX86/targetver.h"
#else
#define WINVER _WIN32_WCE
#include <ceconfig.h>
#endif

#include "afxpriv.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mathdefs.h"

#include "S3DataModel.h"
#include "S3GPIB.h"

#include "S3ControllerDlg.h"
#include "S3GDIInfoPopup.h"

#define S3_MAX_SELECTABLES	18

CRect	Selectable[S3_MAX_SELECTABLES];
char	NSelect;

// 3 vertical panels
CRect	m_RectSettingsScreen,
			m_RectSettingsRemote,
				m_RectRemoteHeader, m_RectEthernet, m_RectUSB,
			m_RectSysParas,
				m_RectParasHeader, m_RectSettingsSysWide, m_RectSettingsDefaults,
			m_RectSettingsSystem,
				m_RectSystemHeader, m_RectSystemDateTime, m_RectIdent, m_RectSettingsInfo;

// ----------------------------------------------------------------------------

char FindSelectable(POINT p)
{
	for(char i = 0; i < NSelect; i++)
		if (Selectable[i].PtInRect(p))
			return i;

	return -1;
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3InitSettingsScreen(void)
{
	CRect rect(0, 0, 80, 40);
		
	m_GDIIPPortEdit = new CS3NumEdit(this);

	m_GDIIPPortEdit->Create(WS_CHILD | ES_LEFT | ES_NOHIDESEL | ES_AUTOHSCROLL,
												rect, this, S3GDI_NUM_EDIT);
	m_GDIIPPortEdit->SetFont(&m_cFontL);
	m_GDIIPPortEdit->ShowWindow(false);
	m_GDIIPPortEdit->m_UpdateImmediate = false;

	m_GDIDefaultGainEdit = new CS3NumEdit(this);

	m_GDIDefaultGainEdit->Create(WS_CHILD | ES_LEFT, 
												rect, this, S3GDI_NUM2_EDIT);
	m_GDIDefaultGainEdit->SetFont(&m_cFontL);
	m_GDIDefaultGainEdit->ShowWindow(false);
	m_GDIDefaultGainEdit->m_UpdateImmediate = false;

	m_GDIDateEdit = new CS3Edit(this);
	m_GDIDateEdit->Create(WS_CHILD | ES_LEFT, rect, this, S3GDI_DATE_EDIT);
	m_GDIDateEdit->SetFont(&m_cFontL);
	m_GDIDateEdit->ShowWindow(false);

	m_GDITimeEdit = new CS3Edit(this);
	m_GDITimeEdit->Create(WS_CHILD | ES_LEFT, rect, this, S3GDI_TIME_EDIT);
	m_GDITimeEdit->SetFont(&m_cFontL);
	m_GDITimeEdit->ShowWindow(false);

	m_RectSettingsScreen = m_RectScreen;
	m_RectSettingsScreen.top = m_RectHeader.bottom;

	int WCol = (m_RectScreen.right - m_RectScreen.left) / 3;

	// ------- Remote -------
	m_RectSettingsRemote = m_RectSettingsScreen;
	m_RectSettingsRemote.right = WCol;

	m_RectRemoteHeader = m_RectEthernet = m_RectUSB = m_RectSettingsRemote;
	m_RectRemoteHeader.bottom = m_RectRemoteHeader.top + HEAD_ROW;

	// Get the rect to position the edit box
	m_SettingsAccess = new CS3NameValue(	m_RectRemoteHeader.left, 
								m_RectRemoteHeader.bottom, WCol,
								_T("Access"), _T("Remote"), true);
	// m_SettingsAccess->RectEdit(m_HDC, m_hFontSB);

	m_RectEthernet.top = m_RectRemoteHeader.bottom + PARA_ROW;
	m_RectEthernet.bottom = m_RectEthernet.top +
		SUBHEAD_ROW + 4 * PARA_ROW + BMARGIN;

	// Get the rect to position the edit box
	m_SettingsIPAddr = new CS3NameValue(	m_RectEthernet.left, 
								m_RectEthernet.top + SUBHEAD_ROW + 0 * PARA_ROW, WCol,
								_T("IP Address"), _T("000.000.000.000"), false);
	m_SettingsIPAddr->RectEdit(m_HDC, m_hFontSB);

	m_SettingsGateway = new CS3NameValue(	m_RectEthernet.left, 
								m_RectEthernet.top + SUBHEAD_ROW + 1 * PARA_ROW, WCol,
								_T("Gateway mask"), _T("000.000.000.000"), false);
	m_SettingsGateway->RectEdit(m_HDC, m_hFontSB);

	m_SettingsPort = new CS3NameValue(	m_RectEthernet.left, 
								m_RectEthernet.top + SUBHEAD_ROW + 2 * PARA_ROW, WCol,
								_T("Port"), _T("555555"), true);

	m_SettingsMAC = new CS3NameValue(	m_RectEthernet.left, 
								m_RectEthernet.top + SUBHEAD_ROW + 3 * PARA_ROW, WCol,
								_T("MAC Address"), _T("00:00:00:00:00:00"), false);
	m_SettingsMAC->RectEdit(m_HDC, m_hFontSB);

	m_RectUSB = m_RectEthernet;
	m_RectUSB.top = m_RectEthernet.bottom;
	m_RectUSB.bottom = m_RectScreen.bottom;

	m_SettingsUSBPort = new CS3NameValue(	m_RectUSB.left, 
								m_RectUSB.top + SUBHEAD_ROW + 0 * PARA_ROW, WCol,
								_T("Port"), _T("Disabled"), true);

	m_SettingsUSBDriver = new CS3NameValue(	m_RectUSB.left, 
								m_RectUSB.top + SUBHEAD_ROW + 1 * PARA_ROW, WCol,
								_T("Driver type"), _T("UnknownX"), false);
	m_SettingsUSBDriver->RectEdit(m_HDC, m_hFontS);

	// ---- System parameters ------------
	m_RectSysParas = m_RectSettingsScreen;
	m_RectSysParas.left = m_RectSettingsRemote.right;
	m_RectSysParas.right = m_RectSysParas.left + WCol;

	m_RectParasHeader = m_RectSettingsSysWide = m_RectSettingsDefaults = m_RectSysParas;
	m_RectParasHeader.bottom = m_RectParasHeader.top + HEAD_ROW;

	m_RectSettingsSysWide.top = m_RectParasHeader.bottom;

	// --- System-wide settings ---
	unsigned char RowCnt = 0;

	m_SettingsContTComp = new CS3NameValue(	m_RectSettingsSysWide.left, 
								m_RectSettingsSysWide.top + SUBHEAD_ROW + RowCnt++ * PARA_ROW, WCol,
								_T("\u03F4 Compensation"), _T("Gain Change"), true);

	m_SettingsRxAGC = new CS3NameValue(	m_RectSettingsSysWide.left, 
								m_RectSettingsSysWide.top + SUBHEAD_ROW + RowCnt++ * PARA_ROW, WCol,
								_T("Receiver AGC"), _T("Gain Change"), true);

	m_SettingsUnits = new CS3NameValue(	m_RectSettingsDefaults.left, 
								m_RectSettingsSysWide.top + SUBHEAD_ROW + RowCnt++ * PARA_ROW, WCol,
								_T("Power units"), _T("ddddBuV"), true);

	m_SettingsTxStart = new CS3NameValue(	m_RectSettingsDefaults.left, 
								m_RectSettingsSysWide.top + SUBHEAD_ROW + RowCnt++ * PARA_ROW, WCol,
								_T("Tx start state"), _T("Sleep"), true);

	m_SettingsTxSelfTest = new CS3NameValue(	m_RectSettingsDefaults.left, 
								m_RectSettingsSysWide.top + SUBHEAD_ROW + RowCnt++ * PARA_ROW, WCol,
								_T("Tx self test"), _T("Off"), true);

	m_RectSettingsSysWide.bottom = m_RectSettingsSysWide.top +
		SUBHEAD_ROW + RowCnt * PARA_ROW + BMARGIN;
	
	// --------- Defaults ---------
	
	m_RectSettingsDefaults = m_RectSettingsScreen;
	m_RectSettingsDefaults.top = m_RectSettingsSysWide.bottom;
	m_RectSettingsDefaults.left = m_RectSettingsRemote.right;
	m_RectSettingsDefaults.right = m_RectSettingsDefaults.left + WCol;

	RowCnt = 0;

	m_SettingsGain = new CS3NameValue(	m_RectSettingsDefaults.left, 
								m_RectSettingsDefaults.top + SUBHEAD_ROW + RowCnt++ * PARA_ROW, WCol,
								_T("Gain (dB)"), _T("-55.55"), true);

	// Leave stub for now to maintain indexing
	m_SettingsSigTau = new CS3NameValue(0, 0, 0,
								_T(""), _T(""), false);
	/*
	m_SettingsSigTau = new CS3NameValue(m_RectSettingsDefaults.left, 
								m_RectSettingsDefaults.top + SUBHEAD_ROW + RowCnt++ * PARA_ROW, WCol,
								_T("\u222B\u03a4 (\u03bcS)"), _T("0.000"), true);

	*/

	m_SettingsImp = new CS3NameValue(	m_RectSettingsDefaults.left, 
								m_RectSettingsDefaults.top + SUBHEAD_ROW + RowCnt++ * PARA_ROW, WCol,
								_T("Input Z (\u03a9)"), _T("1MMMMM"), true);

// TODO: Make optional
	m_SettingsLowNoise = new CS3NameValue(	m_RectSettingsDefaults.left, 
								m_RectSettingsDefaults.top + SUBHEAD_ROW + RowCnt++ * PARA_ROW, WCol,
								_T("Low noise mode"), _T("OFFFFF"), true);


	// ------- System -------

	m_RectSettingsSystem = m_RectSettingsScreen;
	m_RectSettingsSystem.left = m_RectSettingsDefaults.right;
	m_RectSettingsSystem.right = m_RectScreen.right;

	m_RectSystemHeader = m_RectSystemDateTime = m_RectIdent = 
		m_RectSettingsInfo = m_RectSettingsSystem;

	m_RectSystemHeader.bottom = m_RectSystemHeader.top + HEAD_ROW;

	m_RectSystemDateTime.top = m_RectSystemHeader.bottom;

	int Start = m_RectSystemDateTime.top;

	m_SettingsDate = new CS3NameValue(m_RectSystemDateTime.left,
			Start + 0 * PARA_ROW, WCol, _T("Date"), _T("YYYY-MM-DD"), true);
	m_SettingsDate->RectEdit(m_HDC, m_hFontSB);

	m_SettingsTime = new CS3NameValue(m_RectSystemDateTime.left,
			Start + 1 * PARA_ROW, WCol, _T("Time"), _T("hhhh:mm:ss"), true);
	m_SettingsTime->RectEdit(m_HDC, m_hFontSB);

	m_RectSystemDateTime.bottom = m_RectSystemDateTime.top + 2 * PARA_ROW + BMARGIN;

	m_RectIdent.top = m_RectSystemDateTime.bottom;

	Start = m_RectIdent.top + SUBHEAD_ROW;
	CString str = _T("SN00000000000");
	RowCnt = 0;

	m_SettingsModel = new CS3NameValue(m_RectIdent.left,
						Start + RowCnt++ * PARA_ROW, WCol, _T("Model"), str, false);
	m_SettingsModel->RectEdit(m_HDC, m_hFontSB);

	m_SettingsSN = new CS3NameValue(m_RectIdent.left,
						Start + RowCnt++ * PARA_ROW, WCol, _T("S/N"), str, false);
	m_SettingsSN->RectEdit(m_HDC, m_hFontSB);

	m_SettingsPN = new CS3NameValue(m_RectIdent.left,
						Start + RowCnt++ * PARA_ROW, WCol, _T("P/N"), str, false);
	m_SettingsPN->RectEdit(m_HDC, m_hFontSB);

	m_SettingsSW = new CS3NameValue(m_RectIdent.left,
						Start + RowCnt++ * PARA_ROW, WCol, _T("S/W"), str, true);
	m_SettingsSW->RectEdit(m_HDC, m_hFontSB);

	m_SettingsImageDate = new CS3NameValue(m_RectIdent.left,
		Start + RowCnt++ * PARA_ROW, WCol, _T("OS Image"), _T("DD/MM/YY:hh:mm"), true);
	m_SettingsImageDate->RectEdit(m_HDC, m_hFontSB);

	m_SettingsBuildNum = new CS3NameValue(m_RectIdent.left,
		Start + RowCnt++ * PARA_ROW, WCol, _T("Application"), _T("DD/MM/YY:hh:mm"), false);
	m_SettingsBuildNum->RectEdit(m_HDC, m_hFontSB);

	m_RectIdent.bottom = m_RectIdent.top +
		SUBHEAD_ROW + RowCnt++ * PARA_ROW + BMARGIN;

	m_RectSettingsInfo.top = m_RectIdent.bottom;
	
	Start = m_RectSettingsInfo.top + SUBHEAD_ROW;

	m_SettingsCfg = new CS3NameValue(m_RectSettingsInfo.left,
						Start, WCol, _T("Config file"), _T("CfgFileName"), false);
	m_SettingsCfg->RectEdit(m_HDC, m_hFontSB);

	m_SettingsLog = new CS3NameValue(m_RectSettingsInfo.left,
						Start + PARA_ROW, WCol, _T("Log file"), _T("LogFileName"), true);
	m_SettingsLog->RectEdit(m_HDC, m_hFontSB);
	
	// Set-up selectable regions for pop-ups and text edits
	// NSelect number is returned by FindSelectable called in S3FindSettingsScreen
	// Check < S3_MAX_SELECTABLES
	Selectable[NSelect++] = m_SettingsPort->RectEdit(m_HDC, m_hFontSB);
	Selectable[NSelect++] = m_SettingsUSBPort->RectEdit(m_HDC, m_hFontSB);
	Selectable[NSelect++] = m_SettingsUnits->RectEdit(m_HDC, m_hFontSB);
	Selectable[NSelect++] = m_SettingsGain->RectEdit(m_HDC, m_hFontSB);
	// TODO: Need to keep in place to maintain indexing
	Selectable[NSelect++] = m_SettingsSigTau->RectEdit(m_HDC, m_hFontSB);
	Selectable[NSelect++] = m_SettingsImp->RectEdit(m_HDC, m_hFontSB);
	// TODO: Need to keep in place to maintain indexing
	Selectable[NSelect++] = m_SettingsLowNoise->RectEdit(m_HDC, m_hFontSB);
	Selectable[NSelect++] = m_SettingsSW->RectEdit(m_HDC, m_hFontSB);
	Selectable[NSelect++] = m_SettingsContTComp->RectEdit(m_HDC, m_hFontSB);
	Selectable[NSelect++] = m_SettingsLog->RectEdit(m_HDC, m_hFontSB);
	Selectable[NSelect++] = m_SettingsDate->RectEdit(m_HDC, m_hFontSB);
	Selectable[NSelect++] = m_SettingsTime->RectEdit(m_HDC, m_hFontSB);
	Selectable[NSelect++] = m_SettingsAccess->RectEdit(m_HDC, m_hFontSB);
	Selectable[NSelect++] = m_SettingsTxStart->RectEdit(m_HDC, m_hFontSB);
	Selectable[NSelect++] = m_SettingsRxAGC->RectEdit(m_HDC, m_hFontSB);
	Selectable[NSelect++] = m_SettingsImageDate->RectEdit(m_HDC, m_hFontSB);
	Selectable[NSelect++] = m_SettingsTxSelfTest->RectEdit(m_HDC, m_hFontSB);

	// Attach an S3NumEdit editors to the settings
	m_SettingsPort->AttachEditor(m_HDC, m_GDIIPPortEdit);
	m_SettingsGain->AttachEditor(m_HDC, m_GDIDefaultGainEdit);
	m_SettingsDate->AttachEditor(m_HDC, m_GDIDateEdit);
	m_SettingsTime->AttachEditor(m_HDC, m_GDITimeEdit);
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3CloseSettingsScreen(void)
{
	delete m_GDIIPPortEdit;
	delete m_GDIDefaultGainEdit;

	delete m_GDIDateEdit;
	delete m_GDITimeEdit;

	delete m_SettingsAccess;

	delete m_SettingsPort;
	delete m_SettingsIPAddr;
	delete m_SettingsGateway;
	delete m_SettingsMAC;

	delete m_SettingsUSBPort;
	delete m_SettingsUSBDriver;

	delete m_SettingsContTComp;
	delete m_SettingsRxAGC;
	delete m_SettingsUnits;
	delete m_SettingsGain;
	delete m_SettingsSigTau;
	delete m_SettingsImp;
	delete m_SettingsLowNoise;
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
	
	Rectangle(m_HDC, fntRc.left, fntRc.top,
		fntRc.right, fntRc.bottom);

	fntRc.left += LHMARGIN;
	DrawText(m_HDC, _T("Ethernet"), -1, &fntRc, DT_LEFT);
	
	fntRc = m_RectUSB;
	fntRc.bottom = fntRc.top + SUBHEAD_ROW;

	Rectangle(m_HDC, fntRc.left - LHMARGIN, fntRc.top,
		fntRc.right, fntRc.bottom);
	
	fntRc.left += LHMARGIN;
	DrawText(m_HDC, _T("USB Serial"), -1, &fntRc, DT_LEFT);

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

	S3GetIPAddrStr(Addr);
	str.Format(_T("%S"), Addr);

	m_SettingsIPAddr->SetValue(str);
	m_SettingsIPAddr->Draw(m_HDC, m_hFontS, m_hFontSB);

	S3GetIPMaskStr(Addr);
	str.Format(_T("%S"), Addr);

	m_SettingsGateway->SetValue(str);
	m_SettingsGateway->Draw(m_HDC, m_hFontS, m_hFontSB);

	unsigned short Port = S3GetIPPort();
	str.Format(_T("%d"), Port);
	
	m_SettingsPort->SetEditable(!S3GetRemote());
	m_SettingsPort->SetValue(str);
	m_SettingsPort->Draw(m_HDC, m_hFontS, m_hFontSB);

	S3GetMACAddrStr(Addr);
	str.Format(_T("%S"), Addr);

	m_SettingsMAC->SetValue(str);
	m_SettingsMAC->Draw(m_HDC, m_hFontS, m_hFontSB);

	SelectObject(m_HDC, m_hBrushBG4);

	m_SettingsUSBPort->SetEditable(!S3GetRemote());
	m_SettingsUSBPort->SetValue(m_Parent->GetUSBPortName());
	m_SettingsUSBPort->Draw(m_HDC, m_hFontS, m_hFontSB);

	m_SettingsUSBDriver->SetValue(m_Parent->GetUSBDriverType());
	m_SettingsUSBDriver->Draw(m_HDC, m_hFontS, m_hFontSB);
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
	
	Rectangle(m_HDC, fntRc.left, fntRc.top,
		fntRc.right, fntRc.bottom);

	fntRc.left += LHMARGIN;
	DrawText(m_HDC, _T("System"), -1, &fntRc, DT_LEFT);

	fntRc = m_RectSettingsDefaults;
	fntRc.bottom = fntRc.top + SUBHEAD_ROW;

	Rectangle(m_HDC, fntRc.left, fntRc.top,
		fntRc.right, fntRc.bottom);

	unsigned mode = S3GetTCompMode();

	// mode = (mode < 100) ? mode : mode - 100;

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

	m_SettingsUnits->SetValue(S3GetUnitString());
	m_SettingsUnits->Draw(m_HDC, m_hFontS, m_hFontSB);

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

	fntRc.left += LHMARGIN;
	DrawText(m_HDC, _T("Defaults"), -1, &fntRc, DT_LEFT);

	str.Format(_T("%+d"), S3IPGetGain(-1, -1, -1));
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
	
	Rectangle(m_HDC, fntRc.left, fntRc.top,
		fntRc.right, fntRc.bottom);

	fntRc.left += LHMARGIN;
	DrawText(m_HDC, _T("Identification"), -1, &fntRc, DT_LEFT);

	fntRc = m_RectSettingsInfo;
	fntRc.bottom = fntRc.top + SUBHEAD_ROW;

	Rectangle(m_HDC, fntRc.left, fntRc.top,
		fntRc.right, fntRc.bottom);

	m_Parent->GetDateStr(str);
	m_SettingsDate->SetValue(str);
	m_SettingsDate->Draw(m_HDC, m_hFontS, m_hFontSB);

	m_Parent->GetTimeStr(str);
	m_SettingsTime->SetValue(str);
	m_SettingsTime->Draw(m_HDC, m_hFontS, m_hFontSB);

	str.Format(_T("%S"), S3SysGetSN());
	m_SettingsSN->SetValue(str);
	m_SettingsSN->Draw(m_HDC, m_hFontS, m_hFontSB);
	
	str.Format(_T("%S"), S3SysGetPN());
	m_SettingsPN->SetValue(str);
	m_SettingsPN->Draw(m_HDC, m_hFontS, m_hFontSB);

	str.Format(_T("%S"), S3SysGetSW());
	m_SettingsSW->SetValue(str);
	m_SettingsSW->SetEditable(!S3GetRemote());
	m_SettingsSW->Draw(m_HDC, m_hFontS, m_hFontSB);
	
	str.Format(_T("%S"), S3SysGetModel());
	m_SettingsModel->SetValue(str);  
	m_SettingsModel->Draw(m_HDC, m_hFontS, m_hFontSB);

	str.Format(_T("%S %S"), S3SysGetImageDate(), S3SysGetImageTime());
	m_SettingsImageDate->SetValue(str);
	m_SettingsImageDate->SetEditable(!S3GetRemote());
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

	m_GDIDateEdit->ShowWindow(false);
	m_GDITimeEdit->ShowWindow(false);

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
		char Para = S3GetSelectedPara(-1, -1, -1);

		if (Para == S3_USB_ENABLE)
		{
			if (m_Parent->GetUSBEnabled())
				m_Parent->SetUSBEnabled(false);
			else
				m_Parent->SetUSBEnabled(true);
		}
		else if  (Para == S3_T_COMP_MODE)
		{
			if (S3GetTCompGainOption())
			{
				if (menu_item == 3)
					S3DoComp(-1, -1);
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
			if (menu_item == 0)
				S3SetRemote(false);
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
				S3SetAGC(S3_AGC_OFF + 100);
			else if (menu_item == 1)
				S3SetAGC(S3_AGC_CONT + 100);
			else if (menu_item == 2)
				S3SetAGC(S3_AGC_GAIN + 100);
		}
		else if  (Para == S3_TX_SELF_TEST)
		{
			if (menu_item == 0)
				S3SetTxSelfTest(true);
			else if (menu_item == 1)
				S3SetTxSelfTest(false);
		}

		return 0;
	}

	// Set up the menu and indicate the parameter to be edited to
	// be processed by S3SetParaValue()
	char s = FindSelectable(p); 
	if (s != -1)
	{
		if (s == 0) // Ethernet port edit
		{
			if (S3GetRemote())
				return 0;
			
			unsigned short Port = S3GetIPPort();

			CString str;

			str.Format(_T("%d"), Port);

			m_GDIIPPortEdit->SetWindowText(str);
			m_GDIIPPortEdit->ShowWindow(true);

			m_NumericPad->PopUp(m_HDC, p.x, p.y, m_GDIIPPortEdit,
				S3_NP_POSITIVE | S3_NP_INTEGER, 5);

			// m_GDIIPPortEdit->SetSel(2, 2);
			// Indicate that a system parameter is being edited
			return S3SetSelectedPara(-1, -1, -1, S3_IP_PORT);
		}
		else if (s == 1) // USB enable/disable toggle
		{
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
		}
		else if (s == 2) // Power units selector
		{
			m_ParaMenu->Init(m_HDC, p.x, p.y);
		
			m_ParaMenu->AddItem(S3GetUnitStrings(S3_UNITS_DBM));
			m_ParaMenu->AddItem(S3GetUnitStrings(S3_UNITS_DBUV));
			m_ParaMenu->AddItem(S3GetUnitStrings(S3_UNITS_MV));

			// TODO: Assumes the ordering of units
			m_ParaMenu->SelectItem(S3GetUnits() - 1);
				
			m_ParaMenu->Activate();

			return S3SetSelectedPara(-1, -1, -1, S3_IP_POWER_UNITS);
		}
		else if (s == 3)
		{
			char Gain = S3IPGetGain(-1, -1, -1);

			CString str;

			str.Format(_T("%d"), Gain);

			m_GDIDefaultGainEdit->SetWindowText(str);
			m_GDIDefaultGainEdit->ShowWindow(true);

			m_NumericPad->PopUp(m_HDC, p.x, p.y, m_GDIDefaultGainEdit,
				S3_NP_INTEGER, 3);

			// Indicate that a system parameter is being edited
			return S3SetSelectedPara(-1, -1, -1, S3_GAIN);
		}
		else if (s == 4) // Integrator time constant
		{
			/*
			// This is defined for each transmitter, so we can't offer
			// a default setting
			m_ParaMenu->Init(m_HDC, p.x, p.y);
		
			m_ParaMenu->AddItem(_T("Off"));
			m_ParaMenu->AddItem(_T("0.1"));
			m_ParaMenu->AddItem(_T("1.0"));
			m_ParaMenu->AddItem(_T("10.0"));

			switch(S3GetSigmaTau(-1, -1, -1))
			{
				case TNone: m_ParaMenu->SelectItem(0); break;
				case TauLo: m_ParaMenu->SelectItem(1); break;
				case TauMd: m_ParaMenu->SelectItem(2); break;
				case TauHi: m_ParaMenu->SelectItem(3); break;
				default: m_ParaMenu->SelectItem(-1);
			};
				
			m_ParaMenu->Activate();

			return S3SetSelectedPara(-1, -1, -1, S3_SIGMA_TAU);
			*/

			return 0; // TODO: Sensible?
		}
		else if (s == 5) // Input z
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
		else if (s == 6) // Low noise mode
		{
#ifdef S3LOWNOISE
			m_ParaMenu->Init(m_HDC, p.x, p.y);
		
			m_ParaMenu->AddItem(_T("On"));
			m_ParaMenu->AddItem(_T("Off"));

			switch(S3GetLowNoiseMode(-1, -1, -1))
			{
				case true: m_ParaMenu->SelectItem(0); break;
				case false: m_ParaMenu->SelectItem(1); break;
			};
				
			m_ParaMenu->Activate();

			return S3SetSelectedPara(-1, -1, -1, S3_LOW_NOISE);
#endif
		}
		else if (s == 7) // OS version
		{
			if (S3GetRemote())
				return 0;

			m_ParaMenu->Init(m_HDC, p.x, p.y);
		
			m_ParaMenu->AddItem(_T("Update App"));

			m_ParaMenu->Activate();

			return S3SetSelectedPara(-1, -1, -1, S3_APP_UPDATE);
		}
		else if (s == 8) // Temp compensation mode
		{
			if (S3GetRemote())
				return 0;

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
		else if (s == 9) // Copy log to USB
		{
			m_ParaMenu->Init(m_HDC, p.x, p.y);
		
			m_ParaMenu->AddItem(_T("Copy to USB"));

			m_ParaMenu->Activate();

			return S3SetSelectedPara(-1, -1, -1, S3_LOG_COPY_USB);
		}
		else if (s == 10) // Date edit
		{
			CString str;

			m_Parent->GetDateStr(str);

			m_GDIDateEdit->SetWindowText(str);
			m_GDIDateEdit->ShowWindow(true);

			// m_NumericPad->PopUp(m_HDC, p.x, p.y, m_GDIIPPortEdit,
			//	S3_NP_POSITIVE | S3_NP_INTEGER, 5);

			return S3SetSelectedPara(-1, -1, -1, S3_DATE_EDIT);
		}
		else if (s == 11) // Time edit
		{
			CString str;

			m_Parent->GetTimeStr(str);

			m_GDITimeEdit->SetWindowText(str);
			m_GDITimeEdit->ShowWindow(true);

			// m_NumericPad->PopUp(m_HDC, p.x, p.y, m_GDIIPPortEdit,
			//	S3_NP_POSITIVE | S3_NP_INTEGER, 5);

			return S3SetSelectedPara(-1, -1, -1, S3_TIME_EDIT);
		}
		else if (s == 12) // Access
		{
			m_ParaMenu->Init(m_HDC, p.x, p.y);
		
			if (S3GetRemote())
			{
				m_ParaMenu->AddItem(_T("Local"));
				// m_ParaMenu->SelectItem(0);

				m_ParaMenu->Activate();

				return S3SetSelectedPara(-1, -1, -1, S3_ACCESS);
			}
			// else do nothing
		}
		else if (s == 13) // Tx start state
		{
			m_ParaMenu->Init(m_HDC, p.x, p.y);
		
			m_ParaMenu->AddItem(_T("User"));
			m_ParaMenu->AddItem(_T("Sleep"));
			m_ParaMenu->AddItem(_T("On"));
				
			m_ParaMenu->SelectItem(S3GetTxStartState());
			m_ParaMenu->Activate();

			return S3SetSelectedPara(-1, -1, -1, S3_TX_START_STATE);
		}
		else if (s == 14) // Global Rx AGC setting
		{
			if (S3GetRemote())
				return 0;

			m_ParaMenu->Init(m_HDC, p.x, p.y);
		
			m_ParaMenu->AddItem(_T("Off"));
			m_ParaMenu->AddItem(_T("Continuous"));
			m_ParaMenu->AddItem(_T("Gain Change"));

			m_ParaMenu->SelectItem(S3GetAGC());
			m_ParaMenu->Activate();

			return S3SetSelectedPara(-1, -1, -1, S3_GLOBAL_AGC);
		}
		else if (s == 15) // App version
		{
			if (S3GetRemote())
				return 0;

			m_ParaMenu->Init(m_HDC, p.x, p.y);
		
			m_ParaMenu->AddItem(_T("Update Image"));

			m_ParaMenu->Activate();

			return S3SetSelectedPara(-1, -1, -1, S3_OS_UPDATE);
		}
		else if (s == 16) // Tx self test
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