// S3GDIScreenLogCopy.cpp : implementation file
//

#include "stdafx.h"

#include <math.h>

#include "S3DataModel.h"

#ifdef S3_AGENT
#include "S3Agent/S3AgentDlg.h"
#else
#include "S3ControllerDlg.h"
#endif

extern pS3DataModel S3Data;

// Map S3GDI table row to parameter type, which is set in S3Data
// structures and then used to pop-up appropriate editor, menu etc

// TODO: These need better mapping...

#ifndef S3_SHOW_P1DB_MODES
char S3GDI_RowParaMap[S3GDI_MAX_IP_PARAS] = {
								S3_ACTIVE_INPUT,
								S3_TXIP_NODENAME,
								S3_GAIN,
								-1, // S3_MAX_INPUT,
								S3_ALARM_LED,
								S3_SIGMA_TAU,
								S3_INPUT_IMP,
								S3_TEST_TONE,
								-1, -1, -1, -1, -1, -1, -1, -1};
#else
char S3GDI_RowParaMap[S3GDI_MAX_IP_PARAS] = {
								S3_ACTIVE_INPUT,
								S3_TXIP_NODENAME,
								S3_GAIN,
								-1, // S3_MAX_INPUT,
								-1, // Sensitivity
								S3_ALARM_LED,
								S3_SIGMA_TAU,
								S3_INPUT_IMP,
								S3_TEST_TONE,
								-1, -1, -1, -1, -1, -1, -1};
#endif

char S3GDI_RxRowParaMap[S3GDI_MAX_TX_PARAS] = {
								S3_ACTIVE_TX,
								S3_RXTX_NODENAME,
								S3_RLL,
								-1, -1, -1, -1, -1};

extern int hTxIPRows[];

// #define RGB(r,g,b)	((COLORREF)(((BYTE)(r & 0xF8)|((WORD)((BYTE)(g & 0xFC))<<8))|(((DWORD)(BYTE)(b & 0xF8))<<16)))

IMPLEMENT_DYNAMIC(CS3GDIScreenMain, CStatic)
CS3GDIScreenMain::CS3GDIScreenMain()
{
	// Double-click behaviour on Rx or Tx of main screen
	m_DoubleClickRequired = false;

	m_ParaMenu = NULL;
	m_NumericPad = NULL;
}

// ----------------------------------------------------------------------------

CS3GDIScreenMain::~CS3GDIScreenMain()
{
	S3CloseGDIMainScreen();
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3CloseGDIMainScreen()
{
	// From CS3GDIScreenInit
	delete m_ParaMenu;
	delete m_NumericPad;

	DeleteObject(m_hFontL);
	m_cFontL.DeleteObject();
	
	DeleteObject(m_hFontLB);
	DeleteObject(m_hFontM);
	
	DeleteObject(m_hFontS);
	m_cFontS.DeleteObject();
	
	DeleteObject(m_hFontSB);
	DeleteObject(m_hFontVS);

	S3CloseSettingsScreen();
	S3CloseGDIRxScreen();
	S3CloseGDITxScreen();
	S3CloseGDIChScreen();
	S3CloseGDIShutdownScreen();
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3GDIReInitialise()
{
	// From CS3GDIScreenInit
	S3CloseGDIMainScreen();
	S3GDIInit();
}

// ----------------------------------------------------------------------------

// This is not defined in:
// C:\Program Files (x86)\Windows CE Tools\SDKs\Trizeps-VII-2015-Q2\Include\Armv4i\winuser.h
// for some reason? A touch screen thing?

#ifdef TRIZEPS
#define STN_DBLCLK          1
#endif

BEGIN_MESSAGE_MAP(CS3GDIScreenMain, CStatic)
		ON_WM_PAINT()
		// ON_STN_CLICKED(IDC_GDI_STATIC, &CS3GDIScreenMain::OnStnClickedGdiStatic)
		ON_CONTROL_REFLECT(STN_CLICKED, &CS3GDIScreenMain::OnStnClicked)
		ON_CONTROL_REFLECT(STN_DBLCLK, &CS3GDIScreenMain::OnStnDblclick)
END_MESSAGE_MAP()

// ----------------------------------------------------------------------------

bool dialog_open = false;

void CS3GDIScreenMain::OnPaint()
{
	HDC			hDC;
	PAINTSTRUCT	Ps;

	hDC = ::BeginPaint(m_hWnd, &Ps);

	if (m_Screen != S3_CALIBRATE_SCREEN && m_Screen != S3_FACTORY_SYS_SCREEN)
	{

	// Paint a background
	SelectObject(m_HDC, m_hPenNone);
	SelectObject(m_HDC, m_hBrushWhite);
	
	if (0)
	{
		// Logical regions
		S3_RECT(m_HDC, m_RectPhysicalScreen);

		SelectObject(m_HDC, m_hBrushBG1);
		Rectangle(m_HDC, m_RectScreen.left, m_RectScreen.top,
							m_RectHeader.right, m_RectHeader.bottom + 1);

		SelectObject(m_HDC, m_hBrushBG2);
		Rectangle(m_HDC, m_RectScreen.left, m_RectRx.top, m_RectRx.right, m_RectRx.bottom + 1);

		SelectObject(m_HDC, m_hBrushBG3);
		Rectangle(m_HDC, m_RectScreen.left, m_RectFOL.top, m_RectFOL.right, m_RectFOL.bottom + 1);

		SelectObject(m_HDC, m_hBrushBG4);
		Rectangle(m_HDC, m_RectScreen.left, m_RectTx.top, m_RectTx.right, m_RectTx.bottom);
		S3_RECT(m_HDC, m_RectCharger);
	}
	else
	{
		if (!S3GetLocked())
			SelectObject(m_HDC, m_hBrushRed);
		else
			SelectObject(m_HDC, m_hBrushWhite);

		S3_RECT_N(m_HDC, m_RectPhysicalScreen);

		SelectObject(m_HDC, m_hBrushBG1);
		S3_RECT_N(m_HDC, m_RectScreen);
	}

	S3DrawGDIHeader();

	// Draw the current screen
	if (m_Screen == S3_OVERVIEW_SCREEN)
	{
		S3DrawGDIRack();
	}
	else if (m_Screen == S3_RX_SCREEN)
	{
		S3DrawGDIRxScreen();

		m_ParaMenu->Draw();
	}
	else if (m_Screen == S3_TX_SCREEN)
	{
		S3DrawGDITxScreen();
		
		m_ParaMenu->Draw();
	}
	else if (m_Screen == S3_CH_SCREEN)
	{
		S3DrawGDIChScreen();
	}
	else if (m_Screen == S3_SETTINGS_SCREEN)
	{
		S3DrawGDISettingsScreen();
	}
#ifndef S3_AGENT
	else if (m_Screen == S3_FACTORY_SCREEN)
	{
		S3DrawGDIFactoryScreen();
	}
#endif
	else if (m_Screen == S3_SHUTDOWN_SCREEN)
	{
		S3DrawGDIShutdownScreen();
	}
	else if (m_Screen == S3_SLEEP_SCREEN)
	{
		S3DrawGDISleepScreen();
	}
	else if (m_Screen == S3_OS_UPDATE_SCREEN)
	{
		S3DrawGDISWUpdateScreen();
	}
#ifndef S3_AGENT
	else if (m_Screen == S3_APP_UPDATE_SCREEN)
	{
		S3DrawGDIAppUpdateScreen();
	}
#endif
	else if (m_Screen == S3_LOG_COPY_SCREEN)
	{
		S3DrawGDILogCopyScreen();
	}
#ifndef S3_AGENT
	else if (m_Screen == S3_CLOSED_SCREEN)
	{
		S3DrawGDIClosedScreen();
	}
#endif

	// Blt the changes to the screen DC.
	BitBlt(	hDC, 0, 0, m_ndh, m_ndv,
			m_HDC, 0, 0,
			SRCCOPY);
	}

	::EndPaint(m_hWnd, &Ps);
}

// ----------------------------------------------------------------------------
// ITH: This is not called... 
void CS3GDIScreenMain::OnInitialUpdate()
{
	S3GDIInit();
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDIHeader()
{
	S3DrawGDIBattChargers();
	S3DrawGDIInfo();
	S3DrawGDISettings();
	S3DrawGDIShutdown();
}

// ----------------------------------------------------------------------------

char CS3GDIScreenMain::S3GDIGetScreen()
{
	return m_Screen;
}

// ----------------------------------------------------------------------------

int CS3GDIScreenMain::S3GDIChangeScreen(char screen)
{
	if (screen == S3_PREVIOUS_SCREEN)
		screen = m_PrevScreen;

		// TODO: Any other factory notifications?
#ifndef S3_AGENT
	if (m_Screen == S3_FACTORY_SCREEN)
		S3LeaveFactoryScreen();

	if (m_Screen == S3_SHUTDOWN_SCREEN)
		S3LeaveShutdownScreen();
#endif
	
	if (m_Screen == screen)
		return 0;

	m_PrevScreen = m_Screen;

	// Tx Screen orphans
	m_GDINodeNameEdit->ShowWindow(SW_HIDE);
	// m_GDIMaxPowerEdit->ShowWindow(SW_HIDE);
	S3GDIIPGainClose();

	// Settings Screen orphans
	m_GDIIPPortEdit->ShowWindow(SW_HIDE);
	m_GDIDefaultGainEdit->ShowWindow(SW_HIDE);
	m_GDIDateEdit->ShowWindow(SW_HIDE);
	m_GDITimeEdit->ShowWindow(SW_HIDE);

	// Shutdown screen orphan
#ifndef S3_AGENT
	m_GDIMaintKeyEdit->ShowWindow(SW_HIDE);
#endif

	m_NumericPad->PopDown();
	m_ParaMenu->Clear();

	// Should this apply to any screen change?
	if (screen == S3_SETTINGS_SCREEN)
	{
		S3SetSelected(-1, -1, -1);
	}
	else if (screen == S3_TX_SCREEN)
	{
		S3GDITxNewTx();
	}

	m_Screen = screen;

	return 1;
}

// ----------------------------------------------------------------------------
// Draw battery with continuous (1% interval) bar.

void CS3GDIScreenMain::S3DrawGDIBattCharge(char Ch, int xref, int yref)
{
	// Draw with primitives
	SelectObject(m_HDC, m_hPenIPOff);

	if (!S3ChOccupied(Ch))
		SelectObject(m_HDC, m_hBrushBG1);
	else
		SelectObject(m_HDC, m_hBrushWhite);

	// Body & +ve button
	Rectangle(m_HDC, xref, yref, xref + m_lChChBatt, yref + m_wChChBatt);
	Rectangle(m_HDC, xref + m_lChChBatt - 1, yref + (int)(0.25 * m_wChChBatt),
		xref + m_lChChBatt + m_lChChBattBtn, yref + (int)(0.75 * m_wChChBatt));

	SelectObject(m_HDC, m_hFontS);
	CString str;

	char ChLevel = S3ChGetSoC(Ch);

	if (!S3ChOccupied(Ch))
		return;

	SelectObject(m_HDC, m_hPenNone);
	SelectObject(m_HDC, m_hBrushLightGrey);

	Rectangle(m_HDC, xref + 3, yref + 3, xref + m_lChChBatt - 2, yref + m_wChChBatt - 2);

	if (!S3ChBattValidated(Ch))
	{
		S3BLT(m_hbmpBattExclam,
			xref + m_lChChBatt / 2 - 24, yref + m_wChChBatt / 2 - 24, 48, 48);

		return;
	}
	else if (ChLevel < 0) // Failed
	{
		S3BLT(m_hbmpBattFail, xref + m_lChChBatt / 2 - 10, yref + m_wChChBatt / 2 - 16, 14, 32);
	}
	else if (S3ChGetAlarms(Ch) & S3_CH_BATT_COLD)
	{
		S3BLT(m_hbmpTxTxBattCold,
			xref + m_lChChBatt / 2 - 8 / 2, yref + m_wChChBatt / 2 - 28 / 2, 8, 28);

		return; // Suppress charge display below
	}
	else if (S3ChGetAlarms(Ch) & S3_CH_BATT_HOT)
	{
		S3BLT(m_hbmpTxTxBattHot,
			xref + m_lChChBatt / 2 - 8 / 2, yref + m_wChChBatt / 2 - 28 / 2, 8, 28);

		return;
	}
	else // Charged or charging
	{
		unsigned char MeterBar;

		MeterBar = (unsigned char)floor(((double)ChLevel * (m_lChChBatt - 5)) / 100.0);

		SelectObject(m_HDC, m_hLiveIPBrush);

		Rectangle(m_HDC,	xref + 3, yref + 3,
							xref + 3 + MeterBar, yref + m_wChChBatt - 2);

		// if (!S3ChFullyCharged(Ch) && 
		if (S3ChGetBattI(Ch) > 0)
		{
			S3BLT(m_hbmpChBattCharging,
				xref + m_lChChBatt / 2 - 16, yref + m_wChChBatt / 2 - 24, 32, 48);
		}
	}

	RECT fntRc;
	fntRc.left = xref + m_lChChBatt / 5;
	fntRc.top = yref + 6;
	fntRc.right = xref + m_lChChBatt - m_lChChBatt / 5;
	fntRc.bottom = yref + m_wChChBatt;

	if (ChLevel < 0)
	{
		fntRc.left -= 5;
		str.Format(_T("Failed"));
		DrawText(m_HDC, str, -1, &fntRc, DT_LEFT);
	}
	else if (m_Screen == S3_CH_SCREEN)
	{
		str.Format(_T("%d%c"), ChLevel, '%');
		DrawText(m_HDC, str, -1, &fntRc, DT_CENTER);
	}

}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::GetPopUpColours(	HBRUSH &BrushBG,
						HBRUSH &BrushButton,
						HBRUSH &BrushDispBG)
{
	BrushBG = m_hBrushMenuBGMed;
	BrushButton = m_hBrushMenuBGDark;
	BrushDispBG = m_hBrushMenuBGLight;
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::GetPopUpFont(	HFONT		&hFontL,
										COLORREF	&crText,
										COLORREF	&crTextBG)
{
	hFontL = m_hFontL;
	crText = m_crMenuTxtActive;
	crTextBG = m_crMenuBGDark;
}

// ----------------------------------------------------------------------------
// Segmented version of above.
// The battery size/shape should be kept aligned with the Tx screen battery  

void CS3GDIScreenMain::S3DrawGDIBattChargers(void)
{
	if (m_Screen == S3_CH_SCREEN)
	{
		HGDIOBJ hfobj = SelectObject(m_HDC, m_hFontL);

		DrawText(m_HDC, _T("Battery chargers"), -1, &m_RectCharger, DT_LEFT);

		SelectObject(m_HDC, hfobj);

		return;
	}

	int w = (m_RectCharger.right - m_RectCharger.left) / S3_N_CHARGERS;
	int M = (w - m_lChBatt - m_lChBattBtn) / 2;

	int yref = m_RectCharger.top + (m_RectCharger.Height() - m_wChBatt) / 2;

	for (char Ch = 0; Ch < S3_N_CHARGERS; Ch++)
	{
		S3DrawGDIBattChargeSeg(Ch, m_RectCharger.left + w * Ch + M, yref);
	}
}

// ----------------------------------------------------------------------------
// Draw battery with blocky (20% interval) bar.
void CS3GDIScreenMain::S3DrawGDIBattChargeSeg(char Ch, int xref, int yref)
{
	// Draw with primitives
	SelectObject(m_HDC, m_hPenIPOff);

	if (!S3ChOccupied(Ch))
		SelectObject(m_HDC, m_hBrushBG1);
	else
		SelectObject(m_HDC, m_hBrushWhite);

	// Body & +ve button
	Rectangle(m_HDC, xref, yref, xref + m_lChBatt, yref + m_wChBatt);
	Rectangle(m_HDC, xref + m_lChBatt - 1, yref + (int)(0.25 * m_wChBatt),
		xref + m_lChBatt + m_lChBattBtn, yref + (int)(0.75 * m_wChBatt));

	SelectObject(m_HDC, m_hFontS);
	CString str;

	char SoC = S3ChGetSoC(Ch);

	if (!S3ChOccupied(Ch))
		return;

	SelectObject(m_HDC, m_hPenNone);
	SelectObject(m_HDC, m_hBrushLightGrey);

	if (!S3ChBattValidated(Ch))
	{
		S3BLT(m_hbmpBattExclam,
			xref + m_lChBatt / 2 - 24, yref + m_wChBatt / 2 - 24, 48, 48);

		return;
	}
	else if (SoC < 0) // Failed
	{
		S3BLT(m_hbmpBattFail, xref + m_lChBatt / 2 - 10, yref + m_wChBatt / 2 - 16, 14, 32);
	}
	else if (S3ChGetAlarms(Ch) & S3_CH_BATT_COLD)
	{
		S3BLT(m_hbmpTxTxBattCold,
			xref + m_lChBatt / 2 - 8 / 2, yref + m_wChBatt / 2 - 28 / 2, 8, 28);

		return; // Suppress charge display below
	}
	else if (S3ChGetAlarms(Ch) & S3_CH_BATT_HOT)
	{
		S3BLT(m_hbmpTxTxBattHot,
			xref + m_lChBatt / 2 - 8 / 2, yref + m_wChBatt / 2 - 28 / 2, 8, 28);

		return;
	}
	else // Charged or charging
	{
		unsigned char MeterBar;

		MeterBar = (m_lChBatt - ((S3_N_BATT_SEGS + 1) * S3_CHARGER_SEG_GAP)) /
			S3_N_BATT_SEGS;

		// 5 bars, <10% show no (useful) charge
		for (char i = 0; i < S3_N_BATT_SEGS; i++)
		{
			if (SoC > 10 && SoC > i * 100 / S3_N_BATT_SEGS)
				SelectObject(m_HDC, m_hBrushGreen);
			else
				SelectObject(m_HDC, m_hBrushLightGrey);

			RoundRect(m_HDC,
				xref + S3_CHARGER_SEG_GAP + i * (MeterBar + S3_CHARGER_SEG_GAP),
				yref + S3_CHARGER_SEG_GAP,
				xref + S3_CHARGER_SEG_GAP + (i + 1) * (MeterBar + S3_CHARGER_SEG_GAP) - 1,
				yref + m_wChBatt - S3_CHARGER_SEG_GAP + 1,
				1, 1);
		}

		if (!S3ChFullyCharged(Ch))
		{
			S3BLT(m_hbmpBattCharging,
				xref + m_lChBatt / 2 - 16, yref + m_wChBatt / 2 - 16, 32, 32);
		}
	}

	RECT fntRc;
	fntRc.left = xref + m_lChBatt / 5;
	fntRc.top = yref + 2;
	fntRc.right = xref + m_lChBatt - m_lChBatt / 5;
	fntRc.bottom = yref + m_wChBatt;

	if (SoC < 0)
	{
		fntRc.left -= 5;
		str.Format(_T("Failed"));
		DrawText(m_HDC, str, -1, &fntRc, DT_LEFT);
	}
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3GDIRemoteCmd(void)
{
	InvalidateRect(m_RectPhysicalScreen, false);
	UpdateWindow();
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDIInfo(void)
{

#ifndef S3_AGENT
    CT2A ascii1(m_Parent->GetUSBPortName());
    strcpy_s(S3Data->m_DisplayedUSBPort, S3_MAX_USB_DRIVER_LEN, ascii1.m_psz);
    CT2A ascii(m_Parent->GetUSBDriverType());
    strcpy_s(S3Data->m_DisplayedUSBDriver, S3_MAX_USB_DRIVER_LEN, ascii.m_psz);
#endif

	if (S3GetRemote())
	{
		S3DrawGDIInfoRemote();
		return;
	}

	CString tmp;
#ifdef S3_AGENT
    tmp.Format(_T("%s %s"), LastUpdateDateStr.GetString(), LastUpdateTimeStr.GetString());
#else
	m_Parent->GetDateTimeStr(tmp);
#endif

	RECT fntRc = m_RectInfo;
	fntRc.left += 20;
	fntRc.bottom = (m_RectInfo.top + m_RectInfo.bottom) / 2;
	
	SetTextColor(m_HDC, m_crBlack);
	SelectObject(m_HDC, m_hFontS);
	DrawText(m_HDC, tmp, -1, &fntRc, DT_LEFT);

	fntRc.top = fntRc.bottom;
	fntRc.bottom = m_RectInfo.bottom;

	tmp = m_InfoStr;

	DrawText(m_HDC, tmp, -1, &fntRc, DT_LEFT);
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDIInfoRemote(void)
{
	CString tmp;
	
	RECT fntRc = m_RectInfo;
	fntRc.left += 5;
	fntRc.bottom = (m_RectInfo.top + m_RectInfo.bottom) / 2;

	HGDIOBJ br = SelectObject(m_HDC, m_hBrushAlarm);
	HGDIOBJ pn = SelectObject(m_HDC, m_hPenNone);
	Rectangle(m_HDC,	m_RectInfo.left, fntRc.top,
						m_RectInfo.right, fntRc.bottom);

	COLORREF cr = SetTextColor(m_HDC, m_crWhite);

	const char *TestName = S3GetTestName();
	
	if (*TestName == '\0')
		tmp.Format(_T("Remote commands only"));
	else
		tmp.Format(_T("Remote: %S"), TestName);

	DrawText(m_HDC, tmp, -1, &fntRc, DT_LEFT);

	SetTextColor(m_HDC, cr);

	const char *CmdBuf =  S3GetPrevRxedMsg();//"CMDHERE";//S3GPIBGetCmdBuf();
	
	if (S3GetPrevRemoteSrc() == S3_USB)
		tmp.Format(_T("USB: %S"), CmdBuf);
	else
		tmp.Format(_T("ETH: %S"), CmdBuf);

	fntRc.top = fntRc.bottom;
	fntRc.bottom = m_RectInfo.bottom;

	SelectObject(m_HDC, m_hFontS);
	DrawText(m_HDC, tmp, -1, &fntRc, DT_LEFT);
	
	br = SelectObject(m_HDC, br);
	pn = SelectObject(m_HDC, pn);
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDIBackButton(void)
{
	int yref = (m_RectBackButton.top + m_RectBackButton.bottom - 48) / 2;
	int	xref = (m_RectBackButton.left + m_RectBackButton.right - 48) / 2;

	S3BLT(m_hbmpRxBackBtn, xref, yref, 48, 48);
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDISettings(void)
{
	if (m_Screen == S3_SETTINGS_SCREEN)
		return;

	int yref = (m_RectSettingsButton.top + m_RectSettingsButton.bottom - 48) / 2;
	int	xref = (m_RectSettingsButton.left + m_RectSettingsButton.right - 48) / 2;

	S3BLT(m_hbmpSettings, xref, yref, 48, 48);
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDIShutdown(void)
{
	int yref = (m_RectShutdownButton.top + m_RectShutdownButton.bottom - 48) / 2;
	int	xref = (m_RectShutdownButton.left + m_RectShutdownButton.right - 48) / 2;

	S3BLT(m_hbmpShutdown, xref, yref, 48, 48);
}

// ----------------------------------------------------------------------------

//void CS3GDIScreenMain::OnDraw(CDC* dc)
//{
//    // CMemDC pDC(dc);
//
//    // TODO: add draw code for native data here - use pDC 
//     //as the device context to draw to
//}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3GDIRedraw()
{
	// GetWindowRect(&m_RectScreen);
	// ScreenToClient(m_RectScreen);

	// HWND hWnd = ::GetDlgItem(m_hWnd, IDC_GDI_STATIC);
	// ::InvalidateRect(hWnd, m_RectScreen, true);
	InvalidateRect(m_RectPhysicalScreen, false);

	// m_GDIStatic.UpdateWindow();
	// Invalidate();
	// UpdateWindow(); // NULL, NULL, RDW_INVALIDATE);
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3GDIForceRedraw()
{
	RedrawWindow(m_RectPhysicalScreen, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDIRack()
{
	for(char i = S3_MAX_RXS - 1; i >= 0; i--)
		S3DrawGDIRx(i);

	S3DrawShuttingDown();

	// S3DrawGDIDbg();
}

// ----------------------------------------------------------------------------
// A little scratch diagnostic box
void CS3GDIScreenMain::S3DrawGDIDbg()
{
	CRect m_RectDbg;

	m_RectDbg.top = 210;
	m_RectDbg.left = 0;
	m_RectDbg.bottom = 370;
	m_RectDbg.right = 60;

	SelectObject(m_HDC, m_hBrushBG4);
	Rectangle(m_HDC, 0, m_RectDbg.top, m_RectDbg.right, m_RectDbg.bottom);

	CString tmp;

#ifndef S3_AGENT
	if (S3GetUSBOpen())
	{
		tmp = m_Parent->GetUSBPortName();
	}
	else
#endif
		tmp = _T("---");

	CRect fntRc = m_RectDbg;
	
	// fntRc.right = m_RectDbg.right + 20;
	fntRc.bottom = m_RectDbg.top + 30;

	SelectObject(m_HDC, m_hFontS);
	DrawText(m_HDC, tmp, -1, &fntRc, DT_LEFT);
}

// ----------------------------------------------------------------------------
// Pointer functions

int CS3GDIScreenMain::S3Find(POINT p)
{
	::ScreenToClient(m_hWnd, &p);

	if (S3FindHeader(p))
		return 1;

	if (m_Screen == S3_OVERVIEW_SCREEN)
	{
		if (S3FindOverviewScreen(p))
			return 1;
	}
	else if (m_Screen == S3_RX_SCREEN)
	{
		if (S3FindRxScreen(p))
			return 1;
	}
	else if (m_Screen == S3_TX_SCREEN)
	{
		if (S3FindTxScreen(p))
			return 1;
	}
	else if (m_Screen == S3_CH_SCREEN)
	{
		if (S3FindChScreen(p))
			return 1;
	}
	else if (m_Screen == S3_SETTINGS_SCREEN)
	{
		if (S3FindSettingsScreen(p))
			return 1;
	}
#ifndef S3_AGENT
	else if (m_Screen == S3_FACTORY_SCREEN)
	{
		if (S3FindFactoryScreen(p))
			return 1;
	}
#endif
	else if (m_Screen == S3_SHUTDOWN_SCREEN)
	{
		if (S3FindShutdownScreen(p))
			return 1;
	}
	else if (m_Screen == S3_OS_UPDATE_SCREEN)
	{
		if (S3FindSWUpdateScreen(p))
			return 1;
	}
#ifndef S3_AGENT
	else if (m_Screen == S3_APP_UPDATE_SCREEN)
	{
		if (S3FindAppUpdateScreen(p))
			return 1;
	}
#endif
	else if (m_Screen == S3_LOG_COPY_SCREEN)
	{
		if (S3FindLogCopyScreen(p))
			return 1;
	}
	else if (m_Screen == S3_SLEEP_SCREEN)
	{
		m_Screen = S3_OVERVIEW_SCREEN;
	}


	return 0;
}

// ----------------------------------------------------------------------------

int CS3GDIScreenMain::S3FindHeader(POINT p)
{
	char ChBattId = -1;

	if (p.y < m_RectHeader.bottom)
	{
		if (m_RectSettingsButton.PtInRect(p))
		{
			if (m_Screen != S3_SETTINGS_SCREEN)
			{
				S3GDIChangeScreen(S3_SETTINGS_SCREEN);

				return 1;
			}
		}

		if (m_RectShutdownButton.PtInRect(p))
		{
#ifndef S3_AGENT
			if (!S3GetLocked())
				S3GDIChangeScreen(S3_FACTORY_SCREEN);
			else
#endif
				S3GDIChangeScreen(S3_SHUTDOWN_SCREEN);

			return 1;
		}
		else if (m_RectCharger.PtInRect(p))
		{
			if (S3ChGetNOnCharge() > 0)
				S3GDIChangeScreen(S3_CH_SCREEN);
		}
	}

	return 0;
}

// ----------------------------------------------------------------------------

int CS3GDIScreenMain::S3FindOverviewScreen(POINT p)
{
	int		d, min = INT_MAX;
	int		x, y;
	int		crx = -1, ctx = -1;

	if (p.y > m_RectRx.top && p.y < m_RectRx.bottom)
	{
		for (char Rx = 0; Rx < S3_MAX_RXS; Rx++)
		{
			S3RxGetCoords(Rx, &x, &y);

			if (x != -1)
			{
				d = (p.x - x) * (p.x - x) + (p.y - y) * (p.y - y);

				if (d < min)
				{
					crx = Rx;
					min = d;
				}
			}
		}

		// 40 pixels from reference point
		if (min < 40 * 40)
		{
			if (S3GetType(crx, -1) == S3_RxEmpty)
				return 0;
			
			char SelRx = S3GetSelectedRx();

			// Crude double click
			if (!m_DoubleClickRequired)
				S3GDIChangeScreen(S3_RX_SCREEN);
			else if (SelRx == crx)
				S3GDIChangeScreen(S3_RX_SCREEN);

			S3SetSelected(crx, -1, -1);
			SelectionChanged();
		}
		else
		{
			S3SetSelected(-1, -1, -1);
			SelectionChanged();
		}

		return 0;
	}

	for (char Rx = 0; Rx < S3_MAX_RXS; Rx++)
	{
		for (char Tx = 0; Tx < S3_MAX_TXS; Tx++)
		{
			S3TxGetCoords(Rx, Tx, &x, &y);

			if (x != -1)
			{
				d = (p.x - x) * (p.x - x) + (p.y - y) * (p.y - y);

				if (d < min)
				{
					crx = Rx;
					ctx = Tx;
					min = d;
				}
			}
		}
	}

	// 30 pixels from reference point
	if (min < 30 * 30)
	{
		if (S3GetType(crx, ctx) == S3_TxUnconnected)
			return 0;

		S3SetSelected(crx, ctx, -1);

		// If already selected, this is a second click
		if (!m_DoubleClickRequired)
			S3GDIChangeScreen(S3_TX_SCREEN);
		else if (S3IsSelected(crx, ctx, -1))
			S3GDIChangeScreen(S3_TX_SCREEN);

		SelectionChanged();

		// Just moves Rx's Tx to big one on the bottom - only meaningful
		// for Rx6
		if (ctx != -1)
			S3RxSetHighlightedTx(crx, ctx);

		return 1;
	}
	else
	{
		S3SetSelected(-1, -1, -1);
		SelectionChanged();
	}

	return 0;
}

// ----------------------------------------------------------------------------
// Selection changed by GDI control
void CS3GDIScreenMain::SelectionChanged()
{
	char	*tmp2;

	S3GetSelPathStr(&tmp2);

	m_InfoStr.Format(_T("%S: %S"), tmp2, S3GetSelNodeName());
}

// ----------------------------------------------------------------------------
// Selection changed outside GDI control
void CS3GDIScreenMain::SetSelection(bool goToOverview = false)
{
	char Rx, Tx, IP;
	char level = S3GetSelected(&Rx, &Tx, &IP);

    if(goToOverview)
    {
        S3GDIChangeScreen(S3_OVERVIEW_SCREEN);
    }
    else
    {
	switch(level)
	{
	// If nothing selected, don't change screen
	case 0:	break; // S3GDIChangeScreen(S3_OVERVIEW_SCREEN); break;
	case 1:	S3GDIChangeScreen(S3_RX_SCREEN); break;
	case 2:	S3GDIChangeScreen(S3_TX_SCREEN); break;
	case 3:	S3GDIChangeScreen(S3_TX_SCREEN); break;
	}
    }
	SelectionChanged();
}

// ----------------------------------------------------------------------------
// TODO: Not used

void CS3GDIScreenMain::ResetCoords()
{
	for(char Rx = 0; Rx < S3_MAX_RXS; Rx++)
	{
		S3RxSetCoords(Rx, -1, -1);

		for(char Tx = 0; Tx < S3_MAX_RXS; Tx++)
			S3TxSetCoords(Rx, Tx, -1, -1);
	}
}

// ----------------------------------------------------------------------------
// Deprecated for CreateBitmap()

HBITMAP CS3GDIScreenMain::S3LoadBitmap(const int &nBitmapID)
{
	HINSTANCE	hInstResource = NULL;
	HBITMAP		hBitmap;

	// Find resource handle
	hInstResource = AfxFindResourceHandle(MAKEINTRESOURCE(nBitmapID), RT_BITMAP);

	// Load the image from resource
	hBitmap = (HBITMAP)::LoadImage(hInstResource, MAKEINTRESOURCE(nBitmapID),
		IMAGE_BITMAP, 0, 0, 0);

	if (hBitmap == NULL)
	{
		char msg[S3_EVENTS_LINE_LEN];
		sprintf_s(msg, S3_EVENTS_LINE_LEN,
			"S3LoadBitmap: Error loading bitmap. ID: %d", nBitmapID);
		S3EventLogAdd(msg, 3, -1, -1, -1);
		return NULL;
	}

	return hBitmap;
}

// ----------------------------------------------------------------------------

HDC CS3GDIScreenMain::CreateBitmap(HDC ParentDC, const int &nBitmapID)
{
	HINSTANCE	hInstResource = NULL;
	HBITMAP		hBitmap;

	// Find resource handle
	hInstResource = AfxFindResourceHandle(MAKEINTRESOURCE(nBitmapID), RT_BITMAP);

	// Load the image from resource
	hBitmap = (HBITMAP)::LoadImage(hInstResource, MAKEINTRESOURCE(nBitmapID),
		IMAGE_BITMAP, 0, 0, 0);

	if (hBitmap == NULL)
	{
		char msg[S3_EVENTS_LINE_LEN];
		sprintf_s(msg, S3_EVENTS_LINE_LEN,
			"CreateBitmap: Error loading bitmap. ID: %d", nBitmapID);
		S3EventLogAdd(msg, 3, -1, -1, -1);
		
		return NULL;
	}

	HDC hDC;
	
	hDC = CreateCompatibleDC(ParentDC);
	SelectObject(hDC, hBitmap);

	return hDC;
}

// ----------------------------------------------------------------------------
// Ellipse for circle wrapper, origin xo, yo

void CS3GDIScreenMain::S3DrawCircle(int xo, int yo, int r)
{
	Ellipse(m_HDC, xo - r, yo - r, xo + r, yo + r);
}

// ----------------------------------------------------------------------------
// Can only use this if we have a reference to the original bitmap - which
// isn't required for anything else. So use GetDCDims() instead.

void CS3GDIScreenMain::GetBitmapDims(HBITMAP hBmp, int *w, int *h)
{
    BITMAP bm;
    ::GetObject( hBmp, sizeof( bm ), &bm );
    *w = bm.bmWidth;
    *h = bm.bmHeight;
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::GetDCDims(HDC hdc, int *w, int *h)
{
	HGDIOBJ hBitmap = GetCurrentObject(hdc, OBJ_BITMAP);
	
	BITMAP bm;
	GetObject(hBitmap, sizeof(BITMAP), &bm);

	*w = bm.bmWidth;
	*h = bm.bmHeight;
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::OnStnClicked()
{
	// TODO: Add your control notification handler code here
	POINT p;

	GetCursorPos(&p);

	S3Find(p);

	InvalidateRect(NULL, false);
}

// ----------------------------------------------------------------------------
// Need to catch double-click, else very (unacceptably) sluggish.
void CS3GDIScreenMain::OnStnDblclick()
{
	// TODO: Add your control notification handler code here
	POINT p;

	GetCursorPos(&p);

	S3Find(p);

	InvalidateRect(m_RectScreen, false);
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawShuttingDown()
{
	if (!S3GetPowerDownPending())
		return;
	
	SelectObject(m_HDC, m_hPenNone);
	SelectObject(m_HDC, m_hBrushSleep);
	SelectObject(m_HDC, m_hFontL);
	COLORREF cr = SetTextColor(m_HDC, m_crWhite);

	CRect rect(	m_RectScreen.left,	m_RectScreen.top + 100,
				m_RectScreen.right, m_RectScreen.bottom - 240);

	Rectangle(m_HDC,	rect.left, rect.top + 100,
						rect.right, rect.bottom - 240);

	CString tmp;

	if (S3GetPowerDownFailed())
		tmp = _T("Failed to shut down all transmitters.\nPlease disconnect manually.");
	else
		tmp = _T("Shutting down transmitters.\nPlease wait.");
		
	DrawText(m_HDC, tmp, -1, &rect, DT_CENTER);
}

// ----------------------------------------------------------------------------