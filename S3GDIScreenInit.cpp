// ----------------------------------------------------------------------------
// S3GDIScreenMain.cpp : implementation file
//

#include "stdafx.h"

#include <stdio.h>
#include <math.h>
#include "mathdefs.h"
#include "S3DataModel.h"

#ifdef S3_AGENT
#include "S3Agent/S3AgentDlg.h"
#else
#include "S3ControllerDlg.h"
#endif

extern int hTxIPRows[];


// Relevant to simulate early 16-bit displays only. New iPAN7 displays are 18-bit.
COLORREF RGBw(BYTE r, BYTE g, BYTE b)
{
#ifndef TRIZEPS
	//r = (r & 0xF8) + 7;
	//g = (g & 0xFC) + 3;
	//b = (b & 0xF8) + 7;
#endif
	// r = (r + 1) * 8 - 1;
	// g = (g + 1) * 4 - 1;
	// b = (b + 1) * 8 - 1;

	return (COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16));
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3GDIInit()
{
	CDC				*cc = GetDC();
	HDC				hh = *cc;

	// ---------------------------------------------------------------------------
	// Set up resources
	// Horizontal

	// Base font spec
	m_lf.lfWidth = 0;
	m_lf.lfEscapement = 0;
	m_lf.lfOrientation = 0;
	m_lf.lfWeight = FW_NORMAL;
	m_lf.lfItalic = 0;
	m_lf.lfUnderline = 0;
	m_lf.lfStrikeOut = 0;
	m_lf.lfCharSet = ANSI_CHARSET;
	m_lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	m_lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	m_lf.lfQuality = FW_DONTCARE;
	m_lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

	wcscpy_s(m_lf.lfFaceName, 32, _T("Segoe UI"));

	// Font variants
	m_lf.lfHeight = 36;
	m_hFontL = CreateFontIndirect(&m_lf);
	m_cFontL.CreateFontIndirect(&m_lf);

	m_lf.lfWeight = FW_SEMIBOLD;
	m_hFontLB = CreateFontIndirect(&m_lf);

	m_lf.lfWeight = FW_NORMAL;
	m_lf.lfHeight = 28;
	m_hFontM = CreateFontIndirect(&m_lf);

	m_lf.lfHeight = 20;
	m_hFontS = CreateFontIndirect(&m_lf);
	m_cFontS.CreateFontIndirect(&m_lf);

	m_lf.lfWeight = FW_SEMIBOLD;
	m_hFontSB = CreateFontIndirect(&m_lf);
	m_lf.lfUnderline = 0;

	m_lf.lfWeight = FW_NORMAL;
	m_lf.lfHeight = 16;
	m_hFontVS = CreateFontIndirect(&m_lf);

	// ---------------------------------------------------------------------------
	// Colours

#ifdef BLUESCHEME
	// Bluey BGs
	 m_crBG1 =		RGBw(190, 220, 255);
	 m_crBG2 =		RGBw(180, 210, 255);
	 m_crBG3 =		RGBw(170, 200, 255);
	 m_crBG4 =		RGBw(255, 255, 255);
#else
	// Greyey BGs
	m_crBG1 =			RGBw(255, 255, 255);
	m_crBG2 =			RGBw(240, 240, 240);
	m_crBG3 =			RGBw(230, 230, 230);
	m_crBG4 =			RGBw(220, 220, 220);
#endif

	// Blue scheme - keep relative
	m_crMenuBGLight	=		RGBw(100,	110,	250);
	m_crMenuBGMed =			RGBw(100 - 40,	110 - 40,	250 - 40);
	m_crMenuBGDark =		RGBw(100 - 80,	110 - 80,	250 - 80);

	m_crMenuTxtActive	=	RGBw(255,	255,	255);
	m_crMenuTxtGreyed =		RGBw(160,	160,	160);

	// Orange scheme
	// m_crMenuBGLight	=	RGBw(255, 165 + 20, 20);
	// m_crMenuBGMed =		RGBw(255, 165, 0);
	// m_crMenuBGDark =		RGBw(255 - 20, 165 - 20, 0);

	// m_crMenuTxtActive =	RGBw( 30,  30,  30);
	// m_crMenuTxtGreyed =	RGBw(150, 150, 150);

	m_crSleep =			RGBw( 40,  40, 255);

	m_crTx =			RGBw(255,  79,   0); // International orange
	// m_crTxSleep =		RGBw(155, 155, 155);
	m_crTxSleep =		RGBw(  0,   0, 255);
	m_crTxInactive =	RGBw(200, 200, 200);
	m_crTxTest =		RGBw(  0, 255, 255);

	m_crIP =			RGBw(200, 200, 200);
	m_crLiveIPFill =	RGBw(  0, 200,   0);

	m_crGainBg =		RGBw(150, 150, 150);
	m_crGainOn =		RGBw(150, 150, 255);
	m_crGainOnUnsel =	RGBw(120, 120, 180); // TOBE: Obsolete
	m_crGainOff =		RGBw(130, 130, 130);

	m_crRLLBg =			RGBw(150, 150, 150);
	m_crRLLOn =			RGBw(150, 150, 255); // Match m_crFOL
	m_crRLLOnUnsel =	RGBw(150, 150, 150); // TOBE: Obsolete
	m_crRLLOff =		RGBw(130, 130, 130);
	
	m_crAlarmFill =		RGBw(255,   0,   0);

	m_crFOL =			RGBw(150, 150, 255);
	
	// Generic colours
	m_crRed =			RGBw(255,   0,   0);
	m_crAmber =			RGBw(255, 180,   0);
	m_crGreen =			RGBw(0,   200,   0);

	m_hBrushRed =		CreateSolidBrush(m_crRed);
	m_hBrushAmber =		CreateSolidBrush(m_crAmber);
	m_hBrushGreen =		CreateSolidBrush(m_crGreen);

	m_crBlack =			RGBw(  0,   0,   0);

	m_crWhite =			RGBw(255, 255, 255);
	m_crLightGrey =		RGBw(218, 218, 218);	// Battery
	m_crMediumGrey =	RGBw(180, 180, 180);
	m_crDarkGrey =		RGBw( 80,  80,  80);

	m_crTextNorm =		RGBw( 30,  30,  30);
	m_crTextGrey =		RGBw(150, 150, 150);

	m_crSel =			RGBw( 50,  50,  50); // General selection colour
	m_hBrushSel = CreateSolidBrush(m_crSel);

	// ---------------------------------------------------------------------------
	// GDI objects

	// INSIDEFRAME or width != 1 causes solid pen...
	// m_hPenSel = CreatePen(PS_DOT | PS_INSIDEFRAME, 3, m_crSel);
	
	// Use ::CreatePen() to disambiguate:
	// C:\Program Files (x86)\Windows CE Tools\SDKs\Trizeps-VII-2015-Q2\Include\Armv4i\wingdi.h(613): could be 'HPEN CreatePen(int,int,COLORREF)'
	// C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\ce7\atlmfc\include\atlosapice.h(754): or 'HPEN ATL::CreatePen(int,int,COLORREF)'
	
#ifdef TRIZEPS
	m_hPenSel = ::CreatePen(PS_DASH, 1, m_crSel);
#else
	m_hPenSel = ::CreatePen(PS_DOT, 1, m_crSel);
#endif

	m_hBrushMenuBGDark =	CreateSolidBrush(m_crMenuBGDark);
	m_hBrushMenuBGMed =		CreateSolidBrush(m_crMenuBGMed);
	m_hBrushMenuBGLight =	CreateSolidBrush(m_crMenuBGLight);

	m_hBrushBG1 = CreateSolidBrush(m_crBG1);
	m_hBrushBG2 = CreateSolidBrush(m_crBG2);
	m_hBrushBG3 = CreateSolidBrush(m_crBG3);
	m_hBrushBG4 = CreateSolidBrush(m_crBG4);

	m_hBrushSleep = CreateSolidBrush(m_crSleep);

	m_hBrushLightGrey =		CreateSolidBrush(m_crLightGrey);
	m_hBrushMediumGrey =	CreateSolidBrush(m_crMediumGrey);
	m_hBrushDarkGrey =		CreateSolidBrush(m_crDarkGrey);

	m_hPenBG =				::CreatePen(PS_SOLID, 1, m_crBG1);

	m_hLiveIPBrush =		CreateSolidBrush(m_crLiveIPFill);
	m_hPenIPLive =			::CreatePen(PS_SOLID, 1, m_crBlack);

	m_hPenIPOff =			::CreatePen(PS_SOLID, 1, m_crBlack);
	m_hPenIPSelected =		::CreatePen(PS_DOT, 1, m_crBlack);

	m_hPenDetected =		::CreatePen(PS_SOLID, 3, m_crGreen);
	m_hPenSleep =			::CreatePen(PS_SOLID, 3, m_crTxSleep);

	m_hBrushIP =			CreateSolidBrush(m_crIP);

	m_hBrushTx =			CreateSolidBrush(m_crTx);
	m_hBrushTxSleep =		CreateSolidBrush(m_crTxSleep);
	m_hBrushTxInactive =	CreateSolidBrush(m_crTxInactive);
	m_hBrushTxTest =		CreateSolidBrush(m_crTxTest);

	m_hBrushAlarm =			CreateSolidBrush(m_crAlarmFill);
	m_hPenAlarm =			::CreatePen(PS_SOLID, 3, m_crAlarmFill);

	m_hPenNone =			::CreatePen(PS_NULL, 0, m_crBlack);

	m_hBrushWhite =			CreateSolidBrush(m_crWhite);

	// Receiver objects
	m_crRx = RGBw(100, 100, 100);
	m_hBrushRx = CreateSolidBrush(m_crRx);

	LOGBRUSH lb; // Not required to be in scope
	lb.lbStyle = BS_SOLID;
	lb.lbColor = m_crFOL;
	lb.lbHatch = 0;

	// m_hPenFOL1 = CreatePen(PS_SOLID | PS_GEOMETRIC | PS_ENDCAP_FLAT, 2, m_crFOL);
	// m_hPenFOL2 = CreatePen(PS_SOLID | PS_GEOMETRIC | PS_ENDCAP_FLAT, 3, m_crFOL);
	// m_hPenFOL6 = CreatePen(PS_SOLID | PS_GEOMETRIC | PS_ENDCAP_FLAT, 4, m_crFOL);

#ifdef TRIZEPS
	m_PenFOL1.CreatePen(PS_SOLID, 2, m_crFOL);
	m_hPenFOL1 = (HPEN)m_PenFOL1;

	m_PenFOL2.CreatePen(PS_SOLID, 3, m_crFOL);
	m_hPenFOL2 = (HPEN)m_PenFOL2;
	
	m_PenFOL6.CreatePen(PS_SOLID, 4, m_crFOL);
	m_hPenFOL6 = (HPEN)m_PenFOL6;
#else
	m_PenFOL1.CreatePen(PS_SOLID | PS_GEOMETRIC | PS_ENDCAP_FLAT, 2, &lb);
	m_hPenFOL1 = (HPEN)m_PenFOL1;

	m_PenFOL2.CreatePen(PS_SOLID | PS_GEOMETRIC | PS_ENDCAP_FLAT, 3, &lb);
	m_hPenFOL2 = (HPEN)m_PenFOL2;
	
	m_PenFOL6.CreatePen(PS_SOLID | PS_GEOMETRIC | PS_ENDCAP_FLAT, 4, &lb);
	m_hPenFOL6 = (HPEN)m_PenFOL6;
#endif

	lb.lbColor = m_crLightGrey;

#ifdef TRIZEPS
	m_PenFOL1Dark.CreatePen(PS_SOLID, 2, m_crMediumGrey);
	m_hPenFOL1Dark = (HPEN)m_PenFOL1Dark;

	m_PenFOL2Dark.CreatePen(PS_SOLID, 3, m_crMediumGrey);
	m_hPenFOL2Dark = (HPEN)m_PenFOL2Dark;

	m_PenFOL6Dark.CreatePen(PS_SOLID, 4, m_crMediumGrey);
	m_hPenFOL6Dark = (HPEN)m_PenFOL6Dark;
#else
	m_PenFOL1Dark.CreatePen(PS_SOLID | PS_GEOMETRIC | PS_ENDCAP_FLAT, 2, &lb);
	m_hPenFOL1Dark = (HPEN)m_PenFOL1Dark;

	m_PenFOL2Dark.CreatePen(PS_SOLID | PS_GEOMETRIC | PS_ENDCAP_FLAT, 3, &lb);
	m_hPenFOL2Dark = (HPEN)m_PenFOL2Dark;

	m_PenFOL6Dark.CreatePen(PS_SOLID | PS_GEOMETRIC | PS_ENDCAP_FLAT, 4, &lb);
	m_hPenFOL6Dark = (HPEN)m_PenFOL6Dark;
#endif

	m_hBrushFOL =			CreateSolidBrush(m_crFOL);

	m_hBrushRxRLLBg =		CreateSolidBrush(m_crRLLBg);
	m_hBrushRxRLLOn =		CreateSolidBrush(m_crRLLOn);
	m_hBrushRxRLLOnUnsel =	CreateSolidBrush(m_crRLLOnUnsel);
	m_hBrushRxRLLOff =		CreateSolidBrush(m_crRLLOff);

	m_hBrushGainBg =		CreateSolidBrush(m_crGainBg);
	m_hBrushGainOn =		CreateSolidBrush(m_crGainOn);
	m_hBrushGainOnUnsel =	CreateSolidBrush(m_crGainOnUnsel);
	m_hBrushGainOff =		CreateSolidBrush(m_crGainOff);

	m_hBrushRxNum =			CreateSolidBrush(m_crBG4);

	// ---------------------------------------------------------------------------
	// Bitmaps

	// Create battery bitmap DCs
	int BattBmpIds[7] = {
		IDB_TXBATT5V_BITMAP,
		IDB_TXBATT4V_BITMAP,
		IDB_TXBATT3V_BITMAP,
		IDB_TXBATT2V_BITMAP,
		IDB_TXBATT1GV_BITMAP,
		IDB_TXBATT1AV_BITMAP,
		IDB_TXBATT1RV_BITMAP
	};

	// For unselected Txs
	int BattUBmpIds[7] = {
		IDB_TXBATT5UV_BITMAP,
		IDB_TXBATT4UV_BITMAP,
		IDB_TXBATT3UV_BITMAP,
		IDB_TXBATT2UV_BITMAP,
		IDB_TXBATT1GUV_BITMAP,
		IDB_TXBATT1AUV_BITMAP,
		IDB_TXBATT1RUV_BITMAP
	};

	unsigned char	i;
	
	for (i = 0; i < S3_N_BATT_SEGS + 2; i++)
	{
		m_hbmpTxBatt[i] =		CreateBitmap(hh, BattBmpIds[i]);
		m_hbmpTxUBatt[i] =		CreateBitmap(hh, BattUBmpIds[i]);
	}

	// BOOTPPMTPC.bmp
	m_hbmpPPMBG = CreateBitmap(hh, IDB_PPMBG_BITMAP);

	// TODO: Implement the rest like these, get rid of Mem suffix

	m_hbmpSysWarn =				CreateBitmap(hh, IDB_SYS_WARN_BITMAP);
	m_hbmpSysError =			CreateBitmap(hh, IDB_SYS_ERROR_BITMAP);
	m_hbmpSysInfo =				CreateBitmap(hh, IDB_SYS_INFO_BITMAP);

	m_hbmpBattCharging =		CreateBitmap(hh, IDB_CHARGING_BITMAP);
	m_hbmpBattExclam =			CreateBitmap(hh, IDB_CH_BATT_EXCLAM_BITMAP);
	m_hbmpBattFail =			CreateBitmap(hh, IDB_CH_BATT_FAIL_BITMAP);
	m_hbmpTxSelBattInvalid =	CreateBitmap(hh, IDB_TX_SEL_BATT_INVALID_BITMAP);
	m_hbmpTxUnselBattInvalid =	CreateBitmap(hh, IDB_TX_UNSEL_BATT_INVALID_BITMAP);
	m_hbmpSettings =			CreateBitmap(hh, IDB_SETTINGS_BITMAP);

	m_hbmpShutdown =			CreateBitmap(hh, IDB_SHUTDOWN_BITMAP);
	m_hbmpRxBackBtn =			CreateBitmap(hh, IDB_RX_BACK_BTN_BITMAP);

	m_hbmpTxSelAct =			CreateBitmap(hh, IDB_TX_SEL_ACT_BITMAP);
	m_hbmpTxUnselAct =			CreateBitmap(hh, IDB_TX_UNSEL_ACT_BITMAP);
	m_hbmpTxUnselInact =		CreateBitmap(hh, IDB_TX_UNSEL_INACT_BITMAP);
	m_hbmpTxUnselSleep =		CreateBitmap(hh, IDB_TX_UNSEL_SLEEP_BITMAP);
	m_hbmpTxSelInact =			CreateBitmap(hh, IDB_TX_SEL_INACT_BITMAP);
	m_hbmpTxSelSleep =			CreateBitmap(hh, IDB_TX_SEL_SLEEP_BITMAP);
	m_hbmpTxLrgAlarm =			CreateBitmap(hh, IDB_TX_LRG_ALARM_BITMAP);
	m_hbmpTxSmlAlarm =			CreateBitmap(hh, IDB_TX_SML_ALARM_BITMAP);
	m_hbmpTxLrgEmpty =			CreateBitmap(hh, IDB_TX_LRG_EMPTY_BITMAP);
	m_hbmpTxSmlEmpty =			CreateBitmap(hh, IDB_TX_SML_EMPTY_BITMAP);

	m_hbmpRxMainRx1 =			CreateBitmap(hh, IDB_RX_MAIN_BITMAP);
	m_hbmpRxMainRx2 =			CreateBitmap(hh, IDB_RX_MAIN_RX2_BITMAP);
	m_hbmpRxMainEmpty =			CreateBitmap(hh, IDB_RX_MAIN_EMPTY_BITMAP);

	m_hbmpFOL =					CreateBitmap(hh, IDB_FOL_BITMAP);
	m_hbmpFOLDark =				CreateBitmap(hh, IDB_FOL_DARK_BITMAP);

	// ---------------------------------------------------------------------------

	m_hbmpTxSelIPAct =			CreateBitmap(hh, IDB_TX_SEL_IP_ACT_BITMAP);
	m_hbmpTxSelIPInact =		CreateBitmap(hh, IDB_TX_SEL_IP_INACT_BITMAP);
	m_hbmpTxUnselIPAct =		CreateBitmap(hh, IDB_TX_UNSEL_IP_ACT_BITMAP);
	m_hbmpTxUnselIPInact =		CreateBitmap(hh, IDB_TX_UNSEL_IP_INACT_BITMAP);

	m_hbmpRxEmptyBar =			CreateBitmap(hh, IDB_RX_EMPTY_BAR_BITMAP);

	// ---------------------------------------------------------------------------

	m_hbmpTxSelIPActAlrm =		CreateBitmap(hh, IDB_TX_SEL_IP_ACT_ALRM_BITMAP);
	m_hbmpTxSelIPInactAlrm =	CreateBitmap(hh, IDB_TX_SEL_IP_INACT_ALRM_BITMAP);
	m_hbmpTxUnselIPActAlrm =	CreateBitmap(hh, IDB_TX_UNSEL_IP_ACT_ALRM_BITMAP);
	m_hbmpTxUnselIPInactAlrm =	CreateBitmap(hh, IDB_TX_UNSEL_IP_INACT_ALRM_BITMAP);

	// ---------------------------------------------------------------------------

	m_hbmpTxTxBattHot =			CreateBitmap(hh, IDB_TXTX_BATT_HOT_BITMAP);
	m_hbmpTxTxBattCold =		CreateBitmap(hh, IDB_TXTX_BATT_COLD_BITMAP);

	// ---------------------------------------------------------------------------

	m_hbmpRedLED =				CreateBitmap(hh, IDB_RED_LED_BITMAP);
	m_hbmpBlkLED =				CreateBitmap(hh, IDB_BLK_LED_BITMAP);
	m_hbmpGrnLED =				CreateBitmap(hh, IDB_GRN_LED_BITMAP);

	// ---------------------------------------------------------------------------

	m_hbmpInfoButton =			CreateBitmap(hh, IDB_INFO_BUTTON_BITMAP);
	m_hbmpInfoButtonGrey =		CreateBitmap(hh, IDB_INFO_BUTTON_GREY_BITMAP);
	m_hbmpBlueButton =			CreateBitmap(hh, IDB_BLUE_BUTTON_BITMAP);
	m_hbmpGreyButton =			CreateBitmap(hh, IDB_GREY_BUTTON_BITMAP);
	m_hbmpRedButton =			CreateBitmap(hh, IDB_RED_BUTTON_BITMAP);
//	m_hbmpBootPPM =				CreateBitmap(hh, IDB_BLUE_BUTTON_BITMAP);

	// ---------------------------------------------------------------------------
	
	m_hbmpRx1RLLOn[4] =			CreateBitmap(hh, IDB_RX1_RLL5_ON_BITMAP);
	m_hbmpRx1RLLOn[3] =			CreateBitmap(hh, IDB_RX1_RLL4_ON_BITMAP);
	m_hbmpRx1RLLOn[2] =			CreateBitmap(hh, IDB_RX1_RLL3_ON_BITMAP);
	m_hbmpRx1RLLOn[1] =			CreateBitmap(hh, IDB_RX1_RLL2_ON_BITMAP);
	m_hbmpRx1RLLOn[0] =			CreateBitmap(hh, IDB_RX1_RLL1_ON_BITMAP);
	m_hbmpRx1RLL1Red =			CreateBitmap(hh, IDB_RX1_RLL1_RED_BITMAP);
	m_hbmpRx1RLL5Red =			CreateBitmap(hh, IDB_RX1_RLL5_RED_BITMAP);

	// ---------------------------------------------------------------------------

	m_hbmpRx2RLLOn[4] =			CreateBitmap(hh, IDB_RX2_RLL5_ON_BITMAP);
	m_hbmpRx2RLLOn[3] =			CreateBitmap(hh, IDB_RX2_RLL4_ON_BITMAP);
	m_hbmpRx2RLLOn[2] =			CreateBitmap(hh, IDB_RX2_RLL3_ON_BITMAP);
	m_hbmpRx2RLLOn[1] =			CreateBitmap(hh, IDB_RX2_RLL2_ON_BITMAP);
	m_hbmpRx2RLLOn[0] =			CreateBitmap(hh, IDB_RX2_RLL1_ON_BITMAP);
	m_hbmpRx2RLL1Red =			CreateBitmap(hh, IDB_RX2_RLL1_RED_BITMAP);
	m_hbmpRx2RLL5Red =			CreateBitmap(hh, IDB_RX2_RLL5_RED_BITMAP);

	// ---------------------------------------------------------------------------
	// Set up layout

	GetWindowRect(&m_RectPhysicalScreen);
	ScreenToClient(m_RectPhysicalScreen);

	m_RectScreen = m_RectPhysicalScreen;
	
	// Make 4-pixel border
	m_RectScreen.DeflateRect(4, 4);
	
	// Add any factory offsets
	short x, y;
	S3GetScreenOffsets(&x, &y);
	m_RectScreen.left += x;
	m_RectScreen.top += y;

	m_ndv = m_RectPhysicalScreen.Height();
	m_ndh = m_RectPhysicalScreen.Width();

	// Create a compatible DC. This is the in-memory main drawing buffer.
	// Everything gets blitted onto this. 
	m_HDC = CreateCompatibleDC(hh);
	m_hBitmap = CreateCompatibleBitmap(hh, m_ndh, m_ndv);
	SelectObject(m_HDC, m_hBitmap);

	m_radRatioPosIP = 1.1;
	m_radRatioPosActIP = 1.2;

	m_posIP = m_radRatioPosIP * m_radTxSel;
	m_posActiveIP = m_radRatioPosActIP * m_radTxSel;

	m_posIPUnsel = m_radRatioPosIP * m_radTxUnsel;
	m_posActiveIPUnsel = m_radRatioPosActIP * m_radTxUnsel;

	// Pre-calc Tx8 input angles 
	double div = (M_PI) / (S3_MAX_IPS - 1); // Fenceposts!
	double start = (double)m_ThStart * M_PI / 180.0;

	for (i = 0; i < S3_MAX_IPS; i++)
		m_th_l[i] = start + i * div;

	for (i = 0; i < S3_MAX_IPS; i++)
		m_th_r[i] = start + M_PI / 2 + i * div;

	// Define 4 horizontal regions (some arbitrary y references)
	m_HeadH = 50;
	m_RxH = 90;
	m_FOLH = 230;
	m_TxH = m_RectScreen.bottom - m_HeadH - m_RxH - m_FOLH;

	m_RxSep = 130;

	m_RectHeader = m_RectScreen;
	m_RectHeader.bottom = m_RectHeader.top + m_HeadH;

	m_RectRx = m_RectScreen;
	m_RectRx.top = m_RectHeader.bottom;
	m_RectRx.bottom = m_RectRx.top + m_RxH;

	m_RectFOL = m_RectScreen;
	m_RectFOL.top = m_RectRx.bottom;
	m_RectFOL.bottom = m_RectFOL.top + m_FOLH;

	m_RectTx = m_RectScreen;
	m_RectTx.top = m_RectFOL.bottom;

	m_TxTop = ((m_RectTx.bottom + m_RectTx.top) / 2) - m_radTxSel;

	m_RxYref = (m_RectRx.bottom + m_RectRx.top) / 2;
	m_RxBottom = m_RxYref + m_hRx / 2 - 2; // -2 fiddle-factor

	m_RectCharger = m_RectScreen;
	m_RectCharger.right = 350; // m_RectScreen.right / 2;
	m_RectCharger.bottom = m_RectHeader.bottom;

	m_RectInfo = m_RectCharger;
	m_RectInfo.left = m_RectCharger.right;
	m_RectInfo.right = m_RectInfo.left + 180;

	// TODO: Should reference settings button
	m_RectShutdownButton = m_RectHeader;
	m_RectShutdownButton.right -= 12;
	m_RectShutdownButton.left = m_RectShutdownButton.right - 48;

	m_RectSettingsButton = m_RectShutdownButton;
	m_RectSettingsButton.left -= 48 + 12;
	m_RectSettingsButton.right -= 48;

	m_RectBackButton = m_RectSettingsButton;
	m_RectBackButton.left -= 48 + 12;
	m_RectBackButton.right -= 48;

	// Use all space between charger and buttons
	m_RectInfo = m_RectCharger;
	m_RectInfo.left = m_RectCharger.right;
	m_RectInfo.right = m_RectBackButton.left;

	int wCharger = (m_RectCharger.right - m_RectCharger.left) / S3_N_CHARGERS;
	int sep = (m_RectScreen.right - m_RectScreen.left) / S3_N_CHARGERS;

	m_InfoStr = _T("");

	// Global text parameters
	SetBkMode(m_HDC, TRANSPARENT);

	S3InitGDIRx();

	S3InitSettingsScreen();
	S3InitGDIRxScreen();
	S3InitGDITxScreen();
	S3InitGDIChScreen();
#ifndef S3_AGENT
	S3InitGDIFactoryScreen();
#endif
	S3InitGDIShutdownScreen();
	S3InitGDISWUpdateScreen();
	S3InitGDIAppUpdateScreen();
	S3InitGDILogCopyScreen();

	m_NumericPad = new CS3NumberPad(this);
	m_NumericPad->Create(WS_CHILD, CRect(0, 0, 0, 0), this, S3GDI_NUM_EDIT);

	m_ParaMenu = new CParameterMenu(this);

	m_Screen = m_PrevScreen = S3_OVERVIEW_SCREEN;

	if (S3OSUpdateFail())
		m_Screen = S3_OS_UPDATE_SCREEN;
}

// ----------------------------------------------------------------------------