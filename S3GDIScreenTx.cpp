// ----------------------------------------------------------------------------
// Draws the transmitter subscreen consisting of transmitter information and a
// column for each input.
//
// Keep code aligned with S3GDIScreenRx.cpp.
//
// ----------------------------------------------------------------------------

#include "stdafx.h"

#include <math.h>

#include "S3DataModel.h"

#ifdef S3_AGENT
#include "S3Agent/S3AgentDlg.h"
#else
#include "S3ControllerDlg.h"
#endif

#include "S3GDIClickText.h"
#include "S3GDIInfoPopup.h"

#define S3dBm2mW(A)			(pow(10.0, (A) / 10.0))
#define	S3mVrms2dBVrms(A)	(20.0 * log10((A)))

#define	S3Vpk2dBmV(A)		(20.0 * log10(((A) * 1000.0)) - 10.0 * log10(2.0))
#define	S3mVpk2dBmV(A)		(20.0 * log10((A)) - 10.0 * log10(2.0))

#define	S3mVpk2dBmVpk(A)	(20.0 * log10((A)))

// ----------------------------------------------------------------------------

CClickText	*TxSigTau[S3_MAX_IPS], *TxHiZ[S3_MAX_IPS], *TxTestTone[S3_MAX_IPS];

#ifdef S3_SHOW_RF_POWER
CClickText			*TxScratch[S3_MAX_IPS];
#endif

CClickText	*TxAlarm, *TxTestToneAll;

extern char S3GDI_RxParaRowMap(char para);

#ifdef S3LOWNOISE
CClickText *TxLowNoise[S3_MAX_IPS];
#endif

#define S3_TX_TABLE_R_MARG	8

#define S3_GAIN_MARGIN_W	3
#define S3_GAIN_BORDER_W	2
#define S3_GAIN_L_OFFSET	10
#define S3_MESSAGE_H		50

int		wIPCol;

TRIVERTEX vertex_g[2] = {
	{0,	0,	0x0000, 0x8000, 0x0000, 0x0000},
	{0,	0,	0x0000, 0xD000, 0x0000, 0x0000}};

TRIVERTEX vertex_r[2] = {
	{0,	0,	0x8000, 0x0000, 0x0000, 0x0000},
	{0,	0,	0xD000, 0x0000, 0x0000, 0x0000}};

TRIVERTEX vertex_b[2] = {
	{0,	0,	0x0000, 0x0000, 0x8000, 0x0000},
	{0,	0,	0x0000, 0x0000, 0xD000, 0x0000}};

TRIVERTEX vertex_grey[2] = {
	{0,	0,	0x8000, 0x8000, 0x8000, 0x0000},
	{0,	0,	0xD000, 0xD000, 0xD000, 0x0000}};

TRIVERTEX vertex_dark_grey[2] = {
	{0,	0,	0x2000, 0x2000, 0x2000, 0x0000},
	{0,	0,	0x7000, 0x7000, 0x7000, 0x0000}};

GRADIENT_RECT gRect = {0, 1};

extern char S3GDI_RowParaMap[]; 

// extern int RxTxRowsCumSum();
extern int hRxTxRows[];
extern int pRxTxRows[];

#ifndef S3_SHOW_P1DB_MODES
#define S3_MAX_TXIP_PARAS	10
int hTxIPRows[S3_MAX_TXIP_PARAS] = {0, 35, 30, 91, 45, 40, 35, 30, 30, 30};
#else
#define S3_MAX_TXIP_PARAS	11
int hTxIPRows[S3_MAX_TXIP_PARAS] = {0, 35, 30, 91, 45, 30, 40, 35, 30, 30, 30};
#endif

int pTxIPRows[S3_MAX_TXIP_PARAS];

extern char S3GDI_ParaRowMap(char para);

// ----------------------------------------------------------------------------
// ITH: Add to initialisation
int TxIPRowsCumSum()
{
	int	CumSum = 0;

	for(char i = 0; i < S3_MAX_TXIP_PARAS; i++)
	{
		CumSum += hTxIPRows[i];
		pTxIPRows[i] = CumSum;
	}

	return 0;
}

// ----------------------------------------------------------------------------

int KeepInRect(CRect outer, CRect &in)
{
	int r = 0, d = 0; // Right and down shift to keep 'in' within 'out'
		
	if (in.left < outer.left)
		r = outer.left - in.left;

	if (in.right > outer.right)
		r = outer.right - in.right;

	if (in.top < outer.top )
		d = outer.top - in.top;

	if (in.bottom > outer.bottom)
		d = outer.bottom - in.bottom;

	in.OffsetRect(r, d);

	return (int)(r || d);
}

// ----------------------------------------------------------------------------
// Assumes gain *range* will not change

int		GainTickMarks[32];
char	NTick;

char CS3GDIScreenMain::CalcGainTickMarks()
{
	NTick = ((S3_MAX_GAIN - S3_MIN_GAIN) / 10);

	// Calculate 10dB tick-marks
	for(short g = 0; g < NTick; g++)
	{
		GainTickMarks[g] = (int)((g + 1) * 10 * m_dBperPix);
	}

	return NTick;
}

// ----------------------------------------------------------------------------
// TODO: Some of this might need to be done on Tx-change to allow for
// tx-specific capabilities, like peak-hold...

CRect m_RectTxTx, m_RectTxBatt, m_RectTxParaList, m_RectTxTable, m_RectTxMsg;

void CS3GDIScreenMain::S3InitGDITxScreen(void)
{
	TxIPRowsCumSum();

	m_GDINodeNameEdit = NULL;
	m_GDIGainEdit = NULL;
	m_GDIMaxPowerEdit = NULL;

	m_TxInfoPopup = NULL;
	m_TxBattInfoPopup = NULL;

	m_TxNodeName = NULL;
	m_TxType = NULL;
	m_TxSN = NULL;

	m_TxBattT = NULL;
	m_TxBattI = NULL;

	m_hIPGain = 81; // hTxIPRows[2]; // m_hIP;
	m_wGainBar = 20;

	m_wTxTx = 200;
	m_wTxIPPara = 90;

	m_dBperPix = (double)(m_hIPGain - 2 * S3_GAIN_MARGIN_W) / (S3_MAX_GAIN - S3_MIN_GAIN);

	m_RectTxTx = m_RectScreen;
	m_RectTxTx.top = m_RectHeader.bottom;
	m_RectTxTx.right = m_RectScreen.left + m_wTxTx;

	m_RectTxBatt = m_RectTxTx;
	m_RectTxBatt.bottom -= S3_HEIGHT_MSG_BAR;

	m_RectTxParaList = m_RectTxTx;
	m_RectTxParaList.left = m_RectTxTx.right;
	m_RectTxParaList.right = m_RectTxParaList.left + m_wTxIPPara;
	m_RectTxParaList.bottom = m_RectScreen.bottom - S3_MESSAGE_H;

	m_RectTxTable = m_RectTxParaList;
	m_RectTxTable.left = m_RectTxParaList.right;
	
	wIPCol = (m_RectScreen.right - m_RectTxTable.left) / S3_MAX_IPS;

	m_RectTxTable.right = m_RectScreen.right;
	m_RectTxTable.bottom = m_RectScreen.bottom - S3_HEIGHT_MSG_BAR;

	m_IPGainIsUp = false;

	CRect rect(0, 0, 80, 40);

	m_GDINodeNameEdit = new CS3Edit(this);

	// TODO: Define IDD in resource.h
	m_GDINodeNameEdit->Create(WS_CHILD | ES_LEFT | ES_AUTOHSCROLL,
		rect, this, S3GDI_TEXT_EDIT);
	m_GDINodeNameEdit->SetFont(&m_cFontL);
	m_GDINodeNameEdit->ShowWindow(SW_HIDE);
	m_GDINodeNameEdit->SetLimitText(8);

	m_GDIGainEdit = new CS3NumEdit(this);

	m_GDIGainEdit->Create(WS_CHILD | ES_RIGHT,
		rect, this, S3GDI_NUM_EDIT);
	m_GDIGainEdit->SetFont(&m_cFontL);
	m_GDIGainEdit->ShowWindow(SW_HIDE);

	m_GDIMaxPowerEdit = new CS3NumEdit(this);

	// TODO: Specify wrt font
	CRect rect2(0, 0, 100, 40);
	m_GDIMaxPowerEdit->Create(WS_CHILD | ES_RIGHT | ES_AUTOHSCROLL,
		rect2, this, S3GDI_NUM_EDIT);
	m_GDIMaxPowerEdit->SetFont(&m_cFontL);
	m_GDIMaxPowerEdit->ShowWindow(SW_HIDE);
	m_GDIMaxPowerEdit->SetLimitText(8);

	CalcGainTickMarks();

	char RowCnt = 0;

	m_TxNodeName = new CS3NameValue(	m_RectTxTx.left, 
				m_RectTxTx.top + HEAD_ROW + RowCnt++ * PARA_ROW, m_RectTxTx.Width(),
				_T("Name"), _T("12345678"), true);

	m_RectTxNodeName = m_TxNodeName->RectEdit(m_HDC, m_hFontL);
	
	m_TxType = new CS3NameValue(	m_RectTxTx.left, 
				m_RectTxTx.top + HEAD_ROW + RowCnt++ * PARA_ROW, m_RectTxTx.Width(),
				_T("Type"), _T("TxN"), false);
	m_RectTxType = m_TxType->RectEdit(m_HDC, m_hFontS);

	m_TxPowerMode = new CS3NameValue(	m_RectTxTx.left, 
				m_RectTxTx.top + HEAD_ROW + RowCnt++ * PARA_ROW, m_RectTxTx.Width(),
				_T("Power mode"), _T("SSleeping"), true);
	m_RectTxPowerMode = m_TxPowerMode->RectEdit(m_HDC, m_hFontS);

	m_TxLaserPow = new CS3NameValue(	m_Parent,
				m_RectTxTx.left, 
				m_RectTxTx.top + HEAD_ROW + RowCnt++ * PARA_ROW, m_RectTxTx.Width(),
				_T("Plaser (dBm)"), _T("-1288"), false);
	m_TxLaserPow->RectEdit(m_HDC, m_hFontS);

	// ------------------------------------------------------------------------

	m_TxPeakThresh = new CS3NameValue(	m_Parent,
				m_RectTxTx.left, 
				m_RectTxTx.top + HEAD_ROW + RowCnt++ * PARA_ROW, m_RectTxTx.Width(),
				_T("Peak thresh (mV)"), _T("-1288"), false);
	m_TxPeakThresh->RectEdit(m_HDC, m_hFontS);

		//m_TxPeakHold = new CS3NameValue(	m_Parent,
		//			m_RectTxTx.left, 
		//			m_RectTxTx.top + HEAD_ROW + Row++ * PARA_ROW, m_RectTxTx.Width(),
		//			_T("Peak hold (dBm)"), _T("-1288"), false);
		//m_TxPeakHold->RectEdit(m_HDC, m_hFontS);

	// ------------------------------------------------------------------------

	CString str;

	str.Format(_T("\u03F4%cC"), 0x00b0);

	m_TxTemp = new CS3NameValue(	m_Parent,
				m_RectTxTx.left, 
				m_RectTxTx.top + HEAD_ROW + RowCnt++ * PARA_ROW, m_RectTxTx.Width(),
				str, _T("-1288"), false);
	m_TxTemp->RectEdit(m_HDC, m_hFontS);

	str.Format(_T("\u03F4comp.%cC"), 0x00b0);

	m_TxTempComp = new CS3NameValue(	m_Parent,
				m_RectTxTx.left, 
				m_RectTxTx.top + HEAD_ROW + RowCnt++ * PARA_ROW, m_RectTxTx.Width(),
				str, _T("-1288"), true);
	m_RectTxDoComp = m_TxTempComp->RectEdit(m_HDC, m_hFontS);

	m_TxSN = new CS3NameValue(	m_RectTxTx.left, 
				m_RectTxTx.top + HEAD_ROW + RowCnt * PARA_ROW, m_RectTxTx.Width() - 16 - NV_LMARGIN,
				_T("S/N"), _T("SN1234567"), false);
	m_TxSN->RectEdit(m_HDC, m_hFontS);

	m_TxInfoPopup = new CS3GDIInfoPopup(this, m_HDC, m_hbmpInfoButton);
	m_TxInfoPopup->Init(
		m_RectTxTx.right - m_TxInfoPopup->Width() - NV_LMARGIN, m_RectTxTx.top + HEAD_ROW + RowCnt++ * PARA_ROW);

	m_RectTxTx.bottom = m_RectTxTx.top + HEAD_ROW + RowCnt * PARA_ROW;
	m_RectTxBatt.top = m_RectTxTx.bottom;

	// --------------------------------------------------------------------------

	RowCnt = 0;

	str.Format(_T("\u03F4 (%d - %d%cC)"),
		S3_BATT_DISCHG_MIN_T / 10, S3_BATT_DISCHG_MAX_T / 10, S3_SYM_DEGREE);

	m_TxBattT = new CS3NameValue(	m_RectTxBatt.left, 
				m_RectTxBatt.top + m_wChBatt + 3 + HEAD_ROW + RowCnt++ * PARA_ROW,
				m_RectTxBatt.Width(),
				str, _T("100"), false);
	m_TxBattT->RectEdit(m_HDC, m_hFontS);

	m_TxBattI = new CS3NameValue(	m_RectTxBatt.left, 
				m_RectTxBatt.top + m_wChBatt + 3 + HEAD_ROW + RowCnt++ * PARA_ROW,
				m_RectTxBatt.Width(),
				_T("I(mA)"), _T("100"), false);
	m_TxBattI->RectEdit(m_HDC, m_hFontS);

	m_TxBattInfoPopup = new CS3GDIInfoPopup(this, m_HDC, m_hbmpInfoButton);
	m_TxBattInfoPopup->Init(m_RectTxBatt.right - m_TxBattInfoPopup->Width() - NV_LMARGIN,
		m_RectTxBatt.top + 14);

	// ------------------------------------------------------------------------
	// Set up ClickTexts
	int xref = m_RectTxTable.left;
	int yref = m_RectTxTable.top;
	
	for (char IP = 0; IP < S3_MAX_IPS; IP++)
	{
		int xrefcol = xref + IP * wIPCol;
		int yrefrow = yref;
		CRect fntRc;

		RowCnt = S3_MAX_TXIP_PARAS - 5;

		fntRc.left = xrefcol;
		fntRc.right = fntRc.left + wIPCol - S3_TX_TABLE_R_MARG;

		fntRc.top = yref + pTxIPRows[RowCnt++];
		fntRc.bottom = yref + pTxIPRows[RowCnt];

		TxSigTau[IP] = new CClickText(this, fntRc, m_HDC);

		fntRc.top = yref + pTxIPRows[RowCnt++];
		fntRc.bottom = yref + pTxIPRows[RowCnt];

		TxHiZ[IP] = new CClickText(this, fntRc, m_HDC);

		fntRc.top = yref + pTxIPRows[RowCnt++];
		fntRc.bottom = yref + pTxIPRows[RowCnt];

		TxTestTone[IP] = new CClickText(this, fntRc, m_HDC);

		fntRc.top = yref + pTxIPRows[RowCnt++];
		fntRc.bottom = yref + pTxIPRows[RowCnt];

#ifdef S3_SHOW_RF_POWER
		TxScratch[IP] = new CClickText(this, fntRc, m_HDC);
		TxScratch[IP]->Enable(false);
#endif

#ifdef S3LOWNOISE
		fntRc.top = yref + pTxIPRows[RowCnt++];
		fntRc.bottom = yref + pTxIPRows[RowCnt];

		TxLowNoise[IP] = new CClickText(this, fntRc, m_HDC);
#endif
	}

	rect = S3RectGDITxIPRowName(m_RectTxParaList.left,
									m_RectTxParaList.top, RowCnt - 2, DT_RIGHT);

	TxTestToneAll = new CClickText(this, rect, m_HDC, DT_RIGHT, 0);
	TxTestToneAll->SetString(_T("Test tone"));

	m_RectTxMsg = m_RectScreen;
	m_RectTxMsg.top = m_RectTxMsg.bottom - S3_MESSAGE_H; // Always bottom row
	m_RectTxMsg.left += 50;

	TxAlarm = new CClickText(this, m_RectTxMsg, m_HDC, DT_LEFT, 1);
	
	m_RectTxMsg.left -= 50;

	m_TxPowerState = S3_TX_ON;
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3CloseGDITxScreen(void)
{
	delete m_GDINodeNameEdit;
	delete m_GDIGainEdit;
	delete m_GDIMaxPowerEdit;

	delete m_TxInfoPopup;
	delete m_TxBattInfoPopup;

	delete m_TxNodeName;
	delete m_TxPowerMode;
	delete m_TxLaserPow;
	
	delete m_TxPeakThresh;
	// delete m_TxPeakHold;

	delete m_TxType;
	delete m_TxTemp;
	delete m_TxTempComp;
	delete m_TxSN;

	delete m_TxBattT;
	delete m_TxBattI;

	for (char IP = 0; IP < S3_MAX_IPS; IP++)
	{
		delete TxSigTau[IP];
		delete TxHiZ[IP];
		delete TxTestTone[IP];
#ifdef S3_SHOW_RF_POWER
		delete TxScratch[IP];
#endif
#ifdef S3LOWNOISE
		delete TxLowNoise[IP];
#endif
	}

	delete TxAlarm;
	delete TxTestToneAll;
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDITxScreen(void)
{
	char Rx, Tx, IP;

	S3GetSelected(&Rx, &Tx, &IP);

	// Detect if parent Rx or Tx has been removed
	if (Rx == -1 || Tx == -1)
	{
		S3GDIChangeScreen(S3_OVERVIEW_SCREEN);
		return;
	}

	m_TxPowerState = S3TxGetPowerStat(Rx, Tx);

	S3DrawGDIBackButton();

	SetTextColor(m_HDC, m_crTextNorm);

	S3DrawGDITxTx(Rx, Tx);
	S3DrawGDITxIPTable(Rx, Tx);
	S3DrawGDIIPGain(Rx, Tx, IP);

	S3DrawGDITxMessage(Rx, Tx);

	m_TxBattInfoPopup->Draw();
	m_TxInfoPopup->Draw();


}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDITxMessage(char Rx, char Tx)
{
	SelectObject(m_HDC, m_hPenNone);
	SelectObject(m_HDC, m_hBrushBG4);
	HGDIOBJ fobj = SelectObject(m_HDC, m_hFontL);

	S3_RECT(m_HDC,	m_RectTxMsg);

	char S3AlarmString[S3_EVENTS_LINE_LEN];

	int NMsg = S3TxAlarmGetString(Rx, Tx, S3AlarmString, S3_EVENTS_LINE_LEN);

	// Prioritise alarm messages over stabilising message
	if (NMsg)
	{
		if (S3AlarmString[0] == 'E')
		{
			S3BLT(m_hbmpSysError, m_RectTxMsg.left + 10, m_RectTxMsg.top + 5,
					32, 32);
		}
		else if (S3AlarmString[0] == 'W')
		{
			S3BLT(m_hbmpSysWarn, m_RectTxMsg.left + 10, m_RectTxMsg.top + 5,
					32, 32);
		}
		else if (S3AlarmString[0] == 'I')
		{
			S3BLT(m_hbmpSysInfo, m_RectTxMsg.left + 10, m_RectTxMsg.top + 5,
					32, 32);
		}
		else return;
		
		CString str(S3AlarmString + 2);

		RECT fntRc = m_RectTxMsg;
		fntRc.top += 5;

		TxAlarm->Enable(true);
		TxAlarm->SetString(str);
	}
	else if (!S3RxIsActiveTx(Rx, Tx))
	{
		S3BLT(m_hbmpSysInfo, m_RectTxMsg.left + 10, m_RectTxMsg.top + 5,
					32, 32);

		CString str("Transmitter not active.");

		RECT fntRc = m_RectTxMsg;
		fntRc.top += 5;

		TxAlarm->Enable(true);
		TxAlarm->SetString(str);
	}
	else if (!S3TxRLLStable(Rx, Tx))
	{
		S3BLT(m_hbmpSysInfo, m_RectTxMsg.left + 10, m_RectTxMsg.top + 5,
					32, 32);

		CString str("Transmitter stabilising. Please wait.");

		RECT fntRc = m_RectTxMsg;
		fntRc.top += 5;

		TxAlarm->Enable(true);
		TxAlarm->SetString(str);
	}
	else
	{
		TxAlarm->Enable(false);
		TxAlarm->SetString(_T(""));
	}

}

// ----------------------------------------------------------------------------

int CS3GDIScreenMain::S3FindTxScreen(POINT p)
{
	if (m_RectBackButton.PtInRect(p))
	{
		S3GDIChangeScreen(S3_OVERVIEW_SCREEN);

		return 1;
	}

	// TxCtrl and battery states may be known in sleep mode

	// Don't even allow info pop-ups as they're potentially out
	// of date.
	if (1) // m_TxPowerState < S3_TX_SLEEP)
	{
		// TODO: Exclusivity should be handled by InfoPopup class
		if (m_TxInfoPopup->FindSelect(p) == 1)
		{
			S3GDIIPGainClose();
			m_ParaMenu->Clear();
			m_TxBattInfoPopup->PopDown();

			m_NumericPad->PopDown();

			return 1;
		}
		
		if (m_TxBattInfoPopup->FindSelect(p) == 1)
		{
			S3GDIIPGainClose();
			m_ParaMenu->Clear();
			m_TxInfoPopup->PopDown();

			m_NumericPad->PopDown();

			return 1;
		}
	}	

#ifndef S3_AGENT
	if (S3GetRemote())
		return 0;
#endif

	char menu_item = m_ParaMenu->FindSelect(p);

	m_ParaMenu->Clear();

	m_GDINodeNameEdit->ShowWindow(SW_HIDE);
	m_GDIMaxPowerEdit->ShowWindow(SW_HIDE);

	char Rx, Tx, IP, Para;
	S3GetSelected(&Rx, &Tx, &IP);

	if (m_RectTxPowerMode.PtInRect(p))
	{
		if (S3TxGetBattValidated(Rx, Tx))
		{
			S3SetSelected(Rx, Tx, -1);
			S3SetSelectedPara(Rx, Tx, -1, S3_TX_POWER_MODE);
			S3DrawGDIParaPopUp(p.x, p.y);
		}

		m_NumericPad->PopDown();

		return 1;
	}

	if (m_TxTempComp->FindSelect(p))
	{
		S3SetSelected(Rx, Tx, -1);
		S3SetSelectedPara(Rx, Tx, -1, S3_TX_DO_COMP);
		S3DrawGDIParaPopUp(p.x, p.y);

		m_NumericPad->PopDown();

		return 1;
	}

	if (S3TxGetType(Rx, Tx) != S3_Tx1 && TxTestToneAll->Find(p))
	{
		S3SetSelected(Rx, Tx, -1);
		S3SetSelectedPara(Rx, Tx, -1, S3_TX_TESTTONE_ALL);
		S3DrawGDIParaPopUp(p.x, p.y);

		m_NumericPad->PopDown();

		return 1;
	}

	// S3TxPwrMode PowerState = S3TxGetPowerStat(Rx, Tx);

	int pgRes = S3GDIIPGainProcess(p);

	if (pgRes == 1)
		return 1;

	if (menu_item != -1)
	{
		Para = S3GetSelectedPara(Rx, Tx, IP);
		
		if (m_TxPowerState >= S3_TX_SLEEP)
		{
			//#define S3_GAIN				0	// dB. Upper: ?; Lower: ?
			//#define S3_MAX_INPUT			1	// dBm 
			//#define S3_SIGMA_TAU			2	// uS
			//#define S3_INPUT_IMP			3	// W
			//#define S3_LOW_NOISE			4	//
			//#define S3_WIN_TRACK			5	// 
			//#define S3_PASSIVE_INT		6	// 
			//#define S3_ALARM_LED			7	//
			//#define S3_TXTX_NODENAME		8
			//#define S3_TEST_TONE			9

			if (Para == S3_TX_POWER_MODE)
			{
				S3SetParaValue(Rx, Tx, IP, Para, menu_item);
				S3SetSelectedPara(Rx, Tx, IP, -1);
			}
			else if (	Para == S3_GAIN ||
						// Para == S3_MAX_INPUT ||
						Para == S3_SIGMA_TAU ||
						Para == S3_INPUT_IMP ||
						Para == S3_TXTX_NODENAME ||
						Para == S3_ACTIVE_INPUT ||
						Para == S3_TEST_TONE)
			{
				S3SetParaValue(Rx, Tx, IP, Para, menu_item);
				S3SetSelectedPara(Rx, Tx, IP, -1);
			}
		}
		else
		{
			S3SetParaValue(Rx, Tx, IP, Para, menu_item);
			S3SetSelectedPara(Rx, Tx, IP, -1);
		}

		return 0;
	}
	// Removed to allow off-line set-up
	// else if (m_TxPowerState >= S3_TX_SLEEP || menu_item == -2)
	else if (menu_item == -2)
	{
		// This is a click on the menu that we don't want to process
		return 0;
	}
	else if (p.y > m_RectHeader.bottom)
	{
		if (TxAlarm->Find(p))
		{
			S3SetSelected(Rx, Tx, -1);
			S3SetSelectedPara(Rx, Tx, -1, S3_TX_CANCEL_ALARM);
			S3DrawGDIParaPopUp(p.x, p.y);

			return 1;
		}

		if (p.x < m_RectTxTx.Width() + m_RectTxParaList.Width())
		{
			S3SetSelected(Rx, Tx, -1);

			if (m_RectTxNodeName.PtInRect(p))
			{
				S3SetSelectedPara(Rx, Tx, -1, S3_TXTX_NODENAME);
				S3DrawGDIParaPopUp(p.x, p.y);

				return 1;
			}

			return 0;
		}
		else
		{
			// Which input?
			for (char IP = 0; IP < S3TxGetNIP(Rx, Tx); IP++)
			{
				if (p.x < m_RectTxTable.left + (IP + 1) * wIPCol)
				{
					char Rx, Tx, OldIP;
					
					S3GetSelected(&Rx, &Tx, &OldIP);

					// TODO: Should only do this when the gain parameter is
					// clicked, to allow immediate selection of another parameter
					// of the same input...
					if (pgRes == 2 && OldIP == IP)
					{
						S3SetSelected(Rx, Tx, -1);
						return 0;
					}

					S3SetSelected(Rx, Tx, IP);
					m_GDINodeNameEdit->ShowWindow(SW_HIDE);
					m_GDIMaxPowerEdit->ShowWindow(SW_HIDE);

					int found = S3FindTxScreenPara(p);

					if (pgRes == 2 && OldIP == IP)
						return 0;

					if (found)
					{
						S3DrawGDIParaPopUp(p.x, p.y);
					}

					return found;

				}
			} // for IP
		}
	}

	S3SetSelected(Rx, Tx, -1);

	return 0;
}

// ----------------------------------------------------------------------------

int CS3GDIScreenMain::S3FindTxScreenPara(POINT p)
{
	char Rx, Tx, IP;
					
	S3GetSelected(&Rx, &Tx, &IP);
			
	unsigned char i;
	// TODO: '5' needs to be replaced with something sensible
	for(i = 0; i < S3_MAX_TXIP_PARAS - 4; i++)
	{
		if (p.y < m_RectTxTable.top + pTxIPRows[i])
		{
			// Column header
			return S3SetSelectedPara(Rx, Tx, IP, S3GDI_RowParaMap[i - 1]);
		}
	}

	i--;

	if (TxSigTau[IP]->Find(p))
		return S3SetSelectedPara(Rx, Tx, IP, S3GDI_RowParaMap[i]);
	i++;

	if (TxHiZ[IP]->Find(p))
		return S3SetSelectedPara(Rx, Tx, IP, S3GDI_RowParaMap[i]);
	i++;

	if (TxTestTone[IP]->Find(p))
		return S3SetSelectedPara(Rx, Tx, IP, S3GDI_RowParaMap[i]);
	i++;

#ifdef S3LOWNOISE
	if (TxLowNoise[IP]->Find(p))
		return S3SetSelectedPara(Rx, Tx, IP, S3GDI_RowParaMap[S3_MAX_TXIP_PARAS - 2]);
#endif

	S3SetSelectedPara(Rx, Tx, IP, -1);

	return 0;
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDITxTx(char Rx, char Tx)
{
	// S3TxPwrMode PowerState = S3TxGetPowerStat(Rx, Tx);
	
	// Make sure we're not left on this screen if Tx has been
	// disconnected
	if (S3GetType(Rx, Tx) == S3_TxUnconnected)
	{
		S3GDIChangeScreen(S3_OVERVIEW_SCREEN);
		return;
	}

	if (!S3GetRemote())
		m_TxNodeName->SetEditable(true);
	else
		m_TxNodeName->SetEditable(false);

	if (!S3GetRemote() && S3TxGetBattValidated(Rx, Tx))
		m_TxPowerMode->SetEditable(true);
	else
		m_TxPowerMode->SetEditable(false);

	if (!S3TxGetBattValidated(Rx, Tx))
	{
		m_TxInfoPopup->Disable(true);
	}
	else
	{
		m_TxInfoPopup->Disable(false);
	}

	if (S3TxGetTCompMode(Rx, Tx) == S3_TCOMP_GAIN && !S3GetRemote() &&
		S3TxGetBattValidated(Rx, Tx) && m_TxPowerState < S3_TX_SLEEP)
		m_TxTempComp->SetEditable(true);
	else
		m_TxTempComp->SetEditable(false);


	SelectObject(m_HDC, m_hPenNone);
	SelectObject(m_HDC, m_hBrushBG2);

	S3_RECT_N(m_HDC, m_RectTxTx);

	CString str;

	RECT fntRc = m_RectTxTx;
	fntRc.left += NV_LMARGIN;
	fntRc.bottom = fntRc.top + HEAD_ROW;
	
	str.Format(_T("Transmitter: %d/%d\r\n"), Rx + 1, Tx + 1);
	
	SelectObject(m_HDC, m_hFontL);
	DrawText(m_HDC, str, -1, &fntRc, DT_LEFT);

	str.Format(_T("%S"), S3GetNodeName(Rx, Tx, -1));
	m_TxNodeName->SetValue(str);
	m_TxNodeName->Draw(m_HDC, m_hFontS, m_hFontSB);

	str.Format(_T("%S"), S3GetTypeStr(Rx, Tx));
	m_TxType->SetValue(str);
	m_TxType->Draw(m_HDC, m_hFontS, m_hFontSB);

	if (S3TxSelfTestPending(Rx, Tx))
	{
		str = _T("Testing");
		m_TxPowerMode->SetEditable(false);
	}
	else
	{
		switch (m_TxPowerState)
		{
		case S3_TX_ON:				str = _T("On"); break;
		case S3_TX_ON_PENDING:		str = _T("Waking"); break;
		case S3_TX_SLEEP_PENDING:	str = _T("Sleeping"); break;
		case S3_TX_SLEEP:			str = _T("Sleep"); break;
		default:					str = _T("Unknown");
		}
	}

	m_TxPowerMode->SetValue(str);
	m_TxPowerMode->Draw(m_HDC, m_hFontS, m_hFontSB);

	short LaserPow = S3TxGetLaserPow(Rx, Tx);

	if (LaserPow != SHRT_MIN)
		str.Format(_T("%.1f"), (double)LaserPow / 100.0);
	else
		str = _T("-.-");

	m_TxLaserPow->SetValue(str);
	m_TxLaserPow->Draw(m_HDC, m_hFontS, m_hFontSB);

	// ---------------------------------------------------------------------------
	// Temporary for development - this is a per-input measurement
	if (S3TxGetPeakHoldCap(Rx, Tx))
	{
		short PeakThresh = S3TxGetPeakThresh(Rx, Tx);

		if (PeakThresh != SHRT_MIN)
			str.Format(_T("%d"), PeakThresh / 10);
		else
			str = _T("--");

		m_TxPeakThresh->SetValue(str);
		m_TxPeakThresh->Draw(m_HDC, m_hFontS, m_hFontSB);

		/*
		short PeakHold = S3TxGetPeakHold(Rx, Tx);

		if (PeakHold != SHRT_MIN)
			str.Format(_T("%.1f"), (double)PeakHold / 100.0);
		else
			str = _T("-.-");

		m_TxPeakHold->SetValue(str);
		m_TxPeakHold->Draw(m_HDC, m_hFontS, m_hFontSB);
		*/
	}

	// ---------------------------------------------------------------------------

	str.Format(_T("%+d"), S3TxGetTemp(Rx, Tx));
	m_TxTemp->SetValue(str);
	m_TxTemp->Draw(m_HDC, m_hFontS, m_hFontSB);

	char t = S3TxGetTempComp(Rx, Tx);

	if (S3GetTCompMode() == S3_TCOMP_GAIN)
	{
		if (t == SCHAR_MIN)
			str.Format(_T("Wait"));
		else
			str.Format(_T("%+d"), S3TxGetTempComp(Rx, Tx));
	}
	else
		str.Format(_T("--"));

	m_TxTempComp->SetAlarm(
		(S3TxGetAlarms(Rx, Tx) & S3_TX_RECOMP_REQ) != 0);

	m_TxTempComp->SetValue(str);
	m_TxTempComp->Draw(m_HDC, m_hFontS, m_hFontSB);

	const char		*SN, *PN, *HWV, *FWV;
	S3TxGetInfo(Rx, Tx, &SN, &PN, &HWV, &FWV);

	str.Format(_T("%S"), SN);
	m_TxSN->SetValue(str);
	m_TxSN->Draw(m_HDC, m_hFontS, m_hFontSB);

	// -----------------------------------------------
	// Battery info
	S3DrawGDITxBattSeg(Rx, Tx, m_RectTxBatt.left + NV_LMARGIN, m_RectTxBatt.top + HEAD_ROW);

	m_TxBattT->SetAlarm(
		(S3TxBattGetAlarms(Rx, Tx) & S3_TX_BATT_HOT) || 
		(S3TxBattGetAlarms(Rx, Tx) & S3_TX_BATT_COLD));

	str.Format(_T("%+d"), S3TxGetBattTemp(Rx, Tx) / 10);
	m_TxBattT->SetValue(str);
	m_TxBattT->Draw(m_HDC, m_hFontS, m_hFontSB);

	str.Format(_T("%+d"), S3TxGetBattI(Rx, Tx));
	m_TxBattI->SetValue(str);
	m_TxBattI->Draw(m_HDC, m_hFontS, m_hFontSB);
}

// ----------------------------------------------------------------------------

#define S3_DIA_LED	24

void CS3GDIScreenMain::S3DrawGDITxIP(char Rx, char Tx, char IP,
												int xref, int yref)
{
	SelectObject(m_HDC, m_hPenNone);

	char ActiveIP = S3TxGetActiveIP(Rx, Tx);
	char Alarms = S3IPGetAlarms(Rx, Tx, IP);
	char SelectedPara = S3GetSelectedPara(Rx, Tx, IP);

	S3DrawGDITxIPParaSelect(xref, yref,	SelectedPara);

	char RowCnt = 0;
	COLORREF cr;

	if (S3IPGetAlarms(Rx, Tx, IP) & S3_IP_OVERDRIVE)
	{
		TxSigTau[IP]->Enable(false);
		TxHiZ[IP]->Enable(false);
		TxTestTone[IP]->Enable(false);
	}
	else
	{
		TxSigTau[IP]->Enable(true);
		TxHiZ[IP]->Enable(true);
		TxTestTone[IP]->Enable(true);
	}

	if (m_TxPowerState < S3_TX_SLEEP)
	{
		cr = SetTextColor(m_HDC, m_crWhite);

		// Contrast
		if (IP == ActiveIP && !S3TxGetActiveIPPending(Rx, Tx))
		{
			if ((Alarms & S3_IP_OVERDRIVE) && m_Parent->m_AnimateState)
			{
				vertex_r[0].x     = xref; // + 1;
				vertex_r[0].y     = yref; // + 1;

				vertex_r[1].x     = xref + wIPCol;
				vertex_r[1].y     = yref + hTxIPRows[RowCnt + 1];

				GradientFill(m_HDC, vertex_r, 2, &gRect, 1, GRADIENT_FILL_RECT_H);
			}
			else
			{
				vertex_g[0].x     = xref; // + 1;
				vertex_g[0].y     = yref; // + 1;

				vertex_g[1].x     = xref + wIPCol;
				vertex_g[1].y     = yref + hTxIPRows[RowCnt + 1];

				GradientFill(m_HDC, vertex_g, 2, &gRect, 1, GRADIENT_FILL_RECT_H);
			}
		}
		else
		{
			if (Alarms && m_Parent->m_AnimateState)
			{
				SetTextColor(m_HDC, m_crWhite);

				vertex_r[0].x     = xref; // + 1;
				vertex_r[0].y     = yref; // + 1;

				vertex_r[1].x     = xref + wIPCol;
				vertex_r[1].y     = yref + hTxIPRows[RowCnt + 1];

				GradientFill(m_HDC, vertex_r, 2, &gRect, 1, GRADIENT_FILL_RECT_H);
			}
			else
			{
				SetTextColor(m_HDC, cr);
				
				if (IP % 2)
					SelectObject(m_HDC, m_hBrushBG2);
				else
					SelectObject(m_HDC, m_hBrushBG3);

				Rectangle(m_HDC,
					xref, yref,
					xref + wIPCol, yref + hTxIPRows[RowCnt + 1]);
			}
		}
	}
	else
	{
		// Sleeping
		cr = SetTextColor(m_HDC, m_crTextNorm);

		if (IP == ActiveIP)
		{
			vertex_grey[0].x     = xref; // + 1;
			vertex_grey[0].y     = yref; // + 1;

			vertex_grey[1].x     = xref + wIPCol;
			vertex_grey[1].y     = yref + hTxIPRows[RowCnt + 1];

			GradientFill(m_HDC, vertex_grey, 2, &gRect, 1, GRADIENT_FILL_RECT_H);
		}
	}

	RECT fntRc;
	fntRc.left = xref;
	fntRc.top = yref - 4;
	fntRc.right = xref + wIPCol;
	fntRc.bottom = yref + hTxIPRows[RowCnt + 1];

	CString str;
	str.Format(_T("%d"), IP + 1);
	SelectObject(m_HDC, m_hFontL);
	DrawText(m_HDC, str, -1, &fntRc, DT_CENTER);

	SetTextColor(m_HDC, cr);

	// ------------------------------------------------------------------------
	RowCnt++;

	fntRc.top = yref + pTxIPRows[RowCnt];
	fntRc.bottom = yref + pTxIPRows[RowCnt + 1];

	str.Format(_T("%S"), S3GetNodeName(Rx, Tx, IP));
	SelectObject(m_HDC, m_hFontS);
	DrawText(m_HDC, str, -1, &fntRc, DT_CENTER);

	// ------------------------------------------------------------------------
	RowCnt++;

	int xrefcol = xref;

	S3DrawGDITxGain(Rx, Tx, IP, xrefcol, yref + pTxIPRows[RowCnt]);

	// ------------------------------------------------------------------------
	RowCnt++;

	HGDIOBJ fobj;

	if (m_TxPowerState >= S3_TX_SLEEP)
		fobj = SelectObject(m_HDC, m_hFontS);
	else
		fobj = SelectObject(m_HDC, m_hFontSB);

	fntRc.left = xrefcol;
	fntRc.right = fntRc.left + wIPCol - S3_TX_TABLE_R_MARG;

	fntRc.top = yref + pTxIPRows[RowCnt];
	fntRc.bottom = yref + pTxIPRows[RowCnt + 1];

	wchar_t units[S3_MAX_UNIT_STR_LEN];

	if (S3IPGetSigmaTau(Rx, Tx, IP) == TauNone)
	{
#ifdef S3_SHOW_P1DB_MODES
		fobj = SelectObject(m_HDC, m_hFontS);

		// Returns in V if g <= -16
		double P1dBIn;
		double P1dBOut;
		double Sens;

		S3GetLinkParas(Rx, Tx, IP,
				   &P1dBIn, &P1dBOut, &Sens);

		_tcscpy_s(units, S3_MAX_UNIT_STR_LEN, S3GetUnitString());

		if (S3GetUnits() == S3_UNITS_WATTS)
		{
			if (S3GetScale() == S3_SCALE_LOG)
			{
				str.Format(_T("%.1f\n%.1f"), P1dBIn, P1dBOut);
			}
			else
			{
				double P1i = S3dBm2mW(P1dBIn);
				double P1o = S3dBm2mW(P1dBOut);

				CString unitsI(_T(""));
				
				if (P1i < 1000.0)
				{
					unitsI = _T("m");
					
				}
				if (P1i < 1.0)
				{
					unitsI = _T("\u03bc");
					P1i *= 1000.0;
				}
				else
				{
					P1i /= 1000.0;
				}

				CString unitsO(_T(""));
				
				if (P1o < 1000.0)
				{
					unitsO = _T("m");
				}
				else
				{
					P1o /= 1000.0;
				}

				str.Format(_T("%.1f%s\n%.1f%s"), P1i, unitsI, P1o, unitsO);
			}
		}
		else
		{
			if (S3GetScale() == S3_SCALE_LOG)
			{
				str.Format(_T("%.1f\n%.1f"),
					S3Vpk2dBmV(P1dBIn), S3mVpk2dBmV(P1dBOut));
			}
			else
			{
				// str.Format(_T("%.1fV\n%.1fmV"), P1dBIn, P1dBOut);

				CString unitsI(_T(""));

				if (P1dBIn < 1.0)
				{
					unitsI = _T("m");
					P1dBIn *= 1000.0;
				}

				CString unitsO(_T("m"));

				if (P1dBOut < 1.0)
				{
					unitsO = _T("\u03bc");
					P1dBOut *= 1000.0;
				}
				else if (P1dBIn >= 1000.0)
				{
					unitsO = _T("%.2f");
					P1dBOut /= 1000.0;
				}

				str.Format(_T("%.1f%s\n%.1f%s"), P1dBIn, unitsI, P1dBOut, unitsO);
			}
		}
		
		DrawText(m_HDC, str, -1, &fntRc, DT_RIGHT);
		RowCnt++;

		fntRc.top = yref + pTxIPRows[RowCnt];
		fntRc.bottom = yref + pTxIPRows[RowCnt + 1];

		if (S3GetUnits() == S3_UNITS_WATTS)
		{
			if (S3GetScale() == S3_SCALE_LOG)
			{
				str.Format(_T("%.1f"), Sens);
			}
			else
			{
				if (Sens < 1.0)
					str.Format(_T("%.1f%c"), 1000.0 * S3dBm2mW(Sens), S3_SYM_MU_LC);
				else if (Sens >= 1000.0)
					str.Format(_T("%.1f"), S3dBm2mW(Sens) / 1000.0);
				else
					str.Format(_T("%.1fm"), S3dBm2mW(Sens));
			}
		}
		else
		{
			if (S3GetScale() == S3_SCALE_LOG)
			{
				str.Format(_T("%.1f"), S3mVrms2dBVrms(Sens));
			}
			else
			{
				if (Sens < 1.0)
					str.Format(_T("%.1f%c"), Sens * 1000.0, S3_SYM_MU_LC);
				else if (Sens >= 1000.0)
					str.Format(_T("%.2f"), Sens / 1000.0);
				else
					str.Format(_T("%.1fm"), Sens);
			}
		}

		DrawText(m_HDC, str, -1, &fntRc, DT_RIGHT);
		RowCnt++;

		SelectObject(m_HDC, fobj);
	#else // S3_SHOW_P1DB_MODES

	#ifdef S3_SHOW_P1DB
		fobj = SelectObject(m_HDC, m_hFontS);

		// Returns in V if g <= -16
		double p1db = S3IPGetP1dB(Rx, Tx, IP);

		// wchar_t units[S3_MAX_UNIT_STR_LEN];
		_tcscpy_s(units, S3_MAX_UNIT_STR_LEN, S3GetUnitString());

		if (S3GetUnits() == S3_UNITS_MV || S3IPGetGain(Rx, Tx, IP) <= -16)
		{
			if (ABS(p1db) < 1.0)
			{
				p1db *= 1000.0;
				_tcscpy_s(units, S3_MAX_UNIT_STR_LEN, _T("mV"));
			}
			else _tcscpy_s(units, S3_MAX_UNIT_STR_LEN, _T("V"));

			str.Format(_T("%d%s"), (int)ROUND(p1db), units);
		}
		else if (S3GetUnits() == S3_UNITS_DBUV)
			str.Format(_T("%d"), (int)ROUND(p1db));
		else
			str.Format(_T("%+d"), (int)ROUND(p1db));
		
		DrawText(m_HDC, str, -1, &fntRc, DT_RIGHT);

		fntRc.top = (fntRc.top + fntRc.bottom) / 2;
		
		SelectObject(m_HDC, m_hFontS);
		if (S3IPGetGain(Rx, Tx, IP) > -16)
			str.Format(_T("%s"), _T("CW"));
		else
			str.Format(_T("%s"), _T("Pulse"));

		DrawText(m_HDC, str, -1, &fntRc, DT_RIGHT);
		RowCnt++;

		SelectObject(m_HDC, fobj);

		/*
		double p1db = S3IPGetP1dB(Rx, Tx, IP);

		str.Format(_T("%.2f"), maxip);
		DrawText(m_HDC, str, -1, &fntRc, DT_RIGHT);

		fntRc.top = (fntRc.top + fntRc.bottom) / 2;
		
		SelectObject(m_HDC, m_hFontS);
		str.Format(_T("%.2f"), p1db);
		DrawText(m_HDC, str, -1, &fntRc, DT_RIGHT);
		*/
	#endif

	#ifdef NO_S3_SHOW_P1DB
		str.Format(_T("%.2f"), maxip);
		DrawText(m_HDC, str, -1, &fntRc, DT_RIGHT | DT_VCENTER);
	#endif // S3_SHOW_P1DB

	#endif  // !S3_SHOW_P1DB_MODES

	} // S3IPGetSigmaTau(Rx, Tx, IP) == TauNone
	else
	{
		RowCnt++;
		RowCnt++;
	}

	SelectObject(m_HDC, fobj);

	// ------------------------------------------------------------------------

	int yCentre = yref + (pTxIPRows[RowCnt] + pTxIPRows[RowCnt + 1] - S3_DIA_LED)/2;

	if (m_TxPowerState >= S3_TX_SLEEP) // || !S3TxGetPeakHoldCap(Rx, Tx))
	{
		S3BLT(m_hbmpBlkLED, xref + (wIPCol - S3_DIA_LED) / 2,
								yCentre,
					S3_DIA_LED, S3_DIA_LED);
	}
	else if (S3IPGetAlarms(Rx, Tx, IP) & S3_IP_OVERDRIVE)
	{
		if (!m_Parent->m_AnimateState)
			S3BLT(m_hbmpRedLED, xref + (wIPCol - S3_DIA_LED) / 2, yCentre,
					S3_DIA_LED, S3_DIA_LED);
		else
			S3BLT(m_hbmpBlkLED, xref + (wIPCol - S3_DIA_LED) / 2, yCentre,
					S3_DIA_LED, S3_DIA_LED);
	}
	else
	{
		S3BLT(m_hbmpGrnLED, xref + (wIPCol - S3_DIA_LED) / 2, yCentre,
				S3_DIA_LED, S3_DIA_LED);
	}

	// ------------------------------------------------------------------------
	RowCnt++;

	TxSigTau[IP]->SetString(S3TxGetTauUnits(Rx, Tx, IP));

	// ------------------------------------------------------------------------

	InputZ z = S3IPGetImpedance(Rx, Tx, IP);

	switch(z)
	{
	case W50:		str = "50"; break;
	case W1M:		str = "1M"; break;
	case ZUnknown:	str = "Unknown"; break;
	case ZError:	str = "Error"; break;
	default:		// Temporary in case of old file misread 
					str = "50*";
					S3IPSetImpedance(Rx, Tx, IP, W50);
	}
	
	TxHiZ[IP]->SetString(str);

	// ------------------------------------------------------------------------
	
	if (S3IPGetTestTonePending(Rx, Tx, IP))
		str = "Wait";
	else if (S3IPGetTestToneEnable(Rx, Tx, IP))
		str = "On";
	else
		str = "Off";

	TxTestTone[IP]->SetString(str);

	// ------------------------------------------------------------------------

#ifdef S3_SHOW_RF_POWER
	short RFLevel = S3IPGetRFLevel(Rx, Tx, IP);

	if (RFLevel != SHRT_MIN)
		str.Format(_T("%.2f"), (double)RFLevel / 100.0);
	else
		str.Format(_T("-.-"));

	TxScratch[IP]->SetString(str);
#endif

	// ------------------------------------------------------------------------

#ifdef S3LOWNOISE
	if (S3IPGetLowNoiseMode(Rx, Tx, IP))
		str = "On";
	else
		str = "Off";
	
	TxLowNoise[IP]->SetString(str);
#endif

	// ------------------------------------------------------------------------

	SelectObject(m_HDC, fobj);
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDITxGain(char Rx, char Tx, char IP,
									   int xref, int yref)
{
	short	Gain = S3IPGetGain(Rx, Tx, IP);

	int xleft = xref + S3_GAIN_L_OFFSET;
	
	// Bar graph references
	int yl = yref + m_hIPGain - S3_GAIN_MARGIN_W,
		yh = yref + S3_GAIN_MARGIN_W;

	int GainBar =	(int)((S3GetMaxGain(Rx, Tx) - Gain) * m_dBperPix);
	if (Gain < 0) GainBar++;

	int Gain0 =		(int)((S3GetMaxGain(Rx, Tx) - 0) * m_dBperPix);

	SelectObject(m_HDC, m_hPenIPLive);
	SelectObject(m_HDC, m_hBrushLightGrey);

	Rectangle(m_HDC,
		xleft + S3_GAIN_BORDER_W,				yh - 1,
		xleft + m_wGainBar - S3_GAIN_BORDER_W,	yl + 1 + 1);

	SelectObject(m_HDC, m_hPenNone);

	if (m_TxPowerState >= S3_TX_SLEEP || S3IPGetAlarms(Rx, Tx, IP) & S3_IP_OVERDRIVE)
		SelectObject(m_HDC, m_hBrushMediumGrey);
	else
		SelectObject(m_HDC, m_hBrushMenuBGLight);

	if (Gain > 0)
		Rectangle(m_HDC,
			xleft + S3_GAIN_BORDER_W + 1,			yh + GainBar,
			xleft + m_wGainBar - S3_GAIN_BORDER_W,	yh + Gain0 + 1);
	else
		Rectangle(m_HDC,
			xleft + S3_GAIN_BORDER_W + 1,			yh + Gain0,
			xleft + m_wGainBar - S3_GAIN_BORDER_W,	yh + GainBar + 1);

	// Draw tick-marks
	SelectObject(m_HDC, m_hPenIPOff);
	for(short g = 0; g < NTick; g++)
	{
		MoveToEx(m_HDC, xleft + S3_GAIN_BORDER_W, yh + GainTickMarks[g], NULL);
		LineTo(m_HDC, xleft + m_wGainBar / 2, yh + GainTickMarks[g]);
	}

	MoveToEx(m_HDC, xleft + S3_GAIN_BORDER_W, yh + Gain0, NULL);
	LineTo(m_HDC, xleft + m_wGainBar - S3_GAIN_BORDER_W, yh + Gain0);

	RECT fntRc;
	fntRc.left = xref;
	fntRc.right = fntRc.left + wIPCol - S3_TX_TABLE_R_MARG;

	fntRc.top = yref + m_hIPGain / 4 - 10;
	fntRc.bottom = fntRc.top + 30;

	SelectObject(m_HDC, m_hFontS);
	CString str;
	if (Gain == 0)
		str = _T("-0");
	else str.Format(_T("%+d"), Gain);

	DrawText(m_HDC, str, -1, &fntRc, DT_RIGHT);

}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDITxIPTable(char Rx, char Tx)
{
	char IP;
	unsigned char	TxType = S3TxGetType(Rx, Tx);

	int xref = m_RectTxParaList.left;
	int yref = m_RectTxParaList.top;

	SelectObject(m_HDC, m_hBrushBG3);
	S3_RECT_N(m_HDC, m_RectTxTable);
	S3_RECT_N(m_HDC, m_RectTxParaList);

	CString str;

	char RowCnt = 0;
	SelectObject(m_HDC, m_hFontS);
	
	if (TxType == S3_Tx8)
		S3DrawGDITxIPRowName(xref, yref, RowCnt++, _T("Active Input"));
	else
		S3DrawGDITxIPRowName(xref, yref, RowCnt++, _T(""));

	S3DrawGDITxIPRowName(xref, yref, RowCnt++, _T(""));
	S3DrawGDITxIPRowName(xref, yref, RowCnt++, _T("Gain (dB)"));
	
#ifdef S3_SHOW_P1DB_MODES

	if (!S3Get3PCLinearity())
	{
		if (S3GetScale() == S3_SCALE_LOG)
		{
			if (S3GetUnits() == S3_UNITS_WATTS)
				str.Format(_T("P1dB      in:\n(dBm)   out:"));
			else
				str.Format(_T("P1dB      in:\n(dBmV)  out:"));
		}
		else
		{
			if (S3GetUnits() == S3_UNITS_WATTS)
				str.Format(_T("P1dB       in:\n(W)       out:"));
			else
				str.Format(_T("P1dB      in:\n(Vpk)    out:"));
		}
	}
	else
	{
		if (S3GetScale() == S3_SCALE_LOG)
		{
			if (S3GetUnits() == S3_UNITS_WATTS)
				str.Format(_T("3%c Lin    in:\n(dBm)   out:"), '%');
			else
				str.Format(_T("3%c Lin    in:\n(dBmV)  out:"), '%');
		}
		else
		{
			if (S3GetUnits() == S3_UNITS_WATTS)
				str.Format(_T("3%c Lin    in:\n(W)       out:"), '%');
			else
				str.Format(_T("3%c Lin    in:\n(Vpk)    out:"), '%');
		}
	}

	S3DrawGDITxIPRowName(xref, yref, RowCnt++, str, DT_RIGHT);

	if (S3GetScale() == S3_SCALE_LOG)
	{
		if (S3GetUnits() == S3_UNITS_WATTS)
			str.Format(_T("Sen. (dBm)"));
		else
			str.Format(_T("Sen. (dBmV)"));
	}
	else
	{
		if (S3GetUnits() == S3_UNITS_WATTS)
			str.Format(_T("Sen. (W)"));
		else
			str.Format(_T("Sen. (Vrms)"));
	}
		
	S3DrawGDITxIPRowName(xref, yref, RowCnt++, str, DT_RIGHT);

#else // S3_SHOW_P1DB_MODES

#ifdef S3_SHOW_P1DB

	if (S3TxGetType(Rx, Tx) == S3_Tx1 && S3IPGetGain(Rx, Tx, 0) <= -16)
		str.Format(_T("iP1dB\n(%s)"), _T("Vpk"));
	else
		str.Format(_T("iP1dB\n(%s)"), S3GetUnitString());
	
	S3DrawGDITxIPRowName(xref, yref, RowCnt++, str, DT_LEFT);
#else
	if (S3GetUnits() == S3_UNITS_MV)
	{
		str.Format(_T("Pmax(%s)"), _T("Vpk"));
		S3DrawGDITxIPRowName(xref, yref, RowCnt++, str, DT_LEFT | DT_VCENTER);
	}
	else
	{
		str.Format(_T("Pmax(%s)"), S3GetUnitString());
		S3DrawGDITxIPRowName(xref, yref, RowCnt++, str, DT_LEFT | DT_VCENTER);
	}
#endif // S3_SHOW_P1DB
#endif // !S3_SHOW_P1DB_MODES

	S3DrawGDITxIPRowName(xref, yref, RowCnt++, _T("Peak detect"));

	str.Format(_T("\u222b\u03a4 (S)")); // S3_SYM_MU_LC);
	S3DrawGDITxIPRowName(xref, yref, RowCnt++, str);

	str.Format(_T("IPz (%c)"), S3_SYM_OMEGA_UC);
	S3DrawGDITxIPRowName(xref, yref, RowCnt++, str);

	if (RowCnt % 2)
		SelectObject(m_HDC, m_hBrushBG3);
	else
		SelectObject(m_HDC, m_hBrushBG2);

	// Rect returned is the text box, so need to frig...
	if (S3TxGetType(Rx, Tx) != S3_Tx1)
	{
		S3DrawGDITxIPRowName(xref, yref, RowCnt++, _T("Dummyyyy"));

		CRect rect = TxTestToneAll->Rect();
		rect.right += S3_TX_TABLE_R_MARG;

		S3_RECT(m_HDC, rect);

		TxTestToneAll->Draw();
	}
	else
		S3DrawGDITxIPRowName(xref, yref, RowCnt++, _T("Test tone"));

#ifdef S3_SHOW_RF_POWER
	str.Format(_T("RF (dBm)"));
	S3DrawGDITxIPRowName(xref, yref, RowCnt++, str);
#else
	str.Format(_T(""));
	S3DrawGDITxIPRowName(xref, yref, RowCnt++, str);
#endif

#ifdef S3LOWNOISE
	S3DrawGDITxIPRowName(xref, yref, RowCnt++, _T("LN mode"));
#endif
	if (S3GetWinTrackOption()) // Window tracking allowed
	{
	}

	xref = m_RectTxTable.left;
	yref = m_RectTxTable.top;
	
	for (IP = 0; IP < S3TxGetNIP(Rx, Tx); IP++)
	{
		int xrefcol = xref + IP * wIPCol;
		int yrefrow = yref;

		SelectObject(m_HDC, m_hPenNone);

		if (IP % 2)
			SelectObject(m_HDC, m_hBrushBG3);
		else
			SelectObject(m_HDC, m_hBrushBG2);

		Rectangle(m_HDC, xrefcol, yref,
			xrefcol + wIPCol + 1, m_RectScreen.bottom);

		S3DrawGDITxIP(Rx, Tx, IP, xrefcol, yrefrow);
	}
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDITxIPRowName(	int xref, int yref,
												char Row, CString cstr,
												int Justification)
{
	SelectObject(m_HDC, m_hPenNone);

	if (Row % 2)
		SelectObject(m_HDC, m_hBrushBG3);
	else
		SelectObject(m_HDC, m_hBrushBG2);
	
	yref += pTxIPRows[Row];

	Rectangle(m_HDC, xref, yref,
				m_RectScreen.right, yref + hTxIPRows[Row + 1] + 1);

	CRect fntRc(xref, yref,
				xref + m_RectTxParaList.Width(), yref + hTxIPRows[Row + 1]);
	
	if (Justification == DT_RIGHT)
		fntRc.right -= S3_TX_TABLE_R_MARG;
	else
		fntRc.left += S3_TX_TABLE_R_MARG;

	DrawText(m_HDC, cstr, -1, &fntRc, Justification);
}

// ----------------------------------------------------------------------------

CRect CS3GDIScreenMain::S3RectGDITxIPRowName(	int xref, int yref,
												char Row, int Justification)
{
	yref += pTxIPRows[Row];

	CRect fntRc(xref, yref,
				xref + m_RectTxParaList.Width(), yref + hTxIPRows[Row + 1]);

	if (Justification == DT_RIGHT)
		fntRc.right -= S3_TX_TABLE_R_MARG;
	else
		fntRc.left += S3_TX_TABLE_R_MARG;
	
	return &fntRc;
}

// ----------------------------------------------------------------------------
// Highlight table item
void CS3GDIScreenMain::S3DrawGDITxIPParaSelect(		int xref, int yref,
													char SelectedPara)
{
	if (SelectedPara == -1)
		return;

	char ParaRow = S3GDI_ParaRowMap(SelectedPara);

	HGDIOBJ p = SelectObject(m_HDC, m_hBrushMenuBGLight);
	Rectangle(m_HDC,	xref,			yref + pTxIPRows[ParaRow],
						xref + wIPCol,	yref + pTxIPRows[ParaRow + 1]);
	SelectObject(m_HDC, p);
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3GDITxNewTx(void)
{
	m_TxInfoPopup->Clear();
	m_TxBattInfoPopup->Clear();

	char Rx, Tx, IP;
	S3GetSelected(&Rx, &Tx, &IP);

	const char		*SN, *PN, *HWV, *FWV;
	S3TxGetInfo(Rx, Tx, &SN, &PN, &HWV, &FWV);
	
	CString str;

	str.Format(_T("%S"), PN);
	m_TxInfoPopup->AddItem(_T("P/N:"), str);

	str.Format(_T("%S"), S3TxGetFWDate(Rx, Tx));
	m_TxInfoPopup->AddItem(_T("Date:"), str);

	str.Format(_T("%S"), FWV);
	m_TxInfoPopup->AddItem(_T("F/W:"), str);

	S3TxGetBattInfo(Rx, Tx, &SN, &PN, &HWV, &FWV);

	str.Format(_T("%S"), SN);
	m_TxBattInfoPopup->AddItem(_T("S/N:"), str);
	
	str.Format(_T("%S"), PN);
	m_TxBattInfoPopup->AddItem(_T("P/N:"), str);

	//str.Format(_T("%S"), HWV);
	//m_TxBattInfoPopup->AddItem(_T("H/W:"), str);

	//str.Format(_T("%S"), FWV);
	//m_TxBattInfoPopup->AddItem(_T("F/W:"), str);
}

// ----------------------------------------------------------------------------
// TODO: This really should re-use the battery charger drawing code (and 
// shouldn't be in this file.
// (see S3DrawGDIBattChargeSeg() in S3GDIScreenMain.cpp).

void CS3GDIScreenMain::S3DrawGDITxBattSeg(char Rx, char Tx, int xref, int yref)
{
	SelectObject(m_HDC, m_hBrushBG3);
	S3_RECT_N(m_HDC, m_RectTxBatt);

	SelectObject(m_HDC, m_hPenNone);
	
	CRect fntRc = m_RectTxBatt;
	fntRc.left += NV_LMARGIN;
	fntRc.bottom = fntRc.top + HEAD_ROW;

	SelectObject(m_HDC, m_hFontL);
	DrawText(m_HDC, _T("Battery\r\n"), -1, &fntRc, DT_LEFT);

	// Draw with primitives
	SelectObject(m_HDC, m_hPenIPOff);
	SelectObject(m_HDC, m_hBrushBG1);

	// Body & +ve button
	Rectangle(m_HDC, xref, yref, xref + m_lChBatt, yref + m_wChBatt);
	Rectangle(m_HDC, xref + m_lChBatt - 1, yref + (int)(0.25 * m_wChBatt),
		xref + m_lChBatt + m_lChBattBtn, yref + (int)(0.75 * m_wChBatt));

	SelectObject(m_HDC, m_hFontS);
	CString str;

	unsigned char SoC = S3TxGetBattSoC(Rx, Tx);

	SelectObject(m_HDC, m_hPenNone);
	SelectObject(m_HDC, m_hBrushLightGrey);

	CString status;

	if (!S3TxGetBattValidated(Rx, Tx))
	{
		S3BLT(m_hbmpBattExclam,
			xref + m_lChBatt / 2 - 48/2, yref + m_wChBatt / 2 - 48/2, 48, 48);

		status = "Not validated";
	}
	else if (SoC == UCHAR_MAX) // Failed
	{
		// If battery has failed completely, won't actually get this.
		S3BLT(m_hbmpBattFail,
			xref + m_lChBatt / 2 - 32/2, yref + m_wChBatt / 2 - 32/2, 14, 32);

		status = "Unknown";
	}
	else if (S3TxBattGetAlarms(Rx, Tx) & S3_TX_BATT_COLD)
	{
		S3BLT(m_hbmpTxTxBattCold,
			xref + m_lChBatt / 2 - 8/2, yref + m_wChBatt / 2 - 28/2, 8, 28);

		status.Format(_T("\u03F4: %d < %d%cC"),
			S3TxGetBattTemp(Rx, Tx) / 10, S3_BATT_DISCHG_MIN_T, 0x00b0);
	}
	else if (S3TxBattGetAlarms(Rx, Tx) & S3_TX_BATT_HOT)
	{
		S3BLT(m_hbmpTxTxBattHot,
			xref + m_lChBatt / 2 - 8/2, yref + m_wChBatt / 2 - 28/2, 8, 28);

		status.Format(_T("\u03F4: %d > %d%cC"),
			S3TxGetBattTemp(Rx, Tx) / 10, S3_BATT_DISCHG_MAX_T, 0x00b0);
	}
	else // Discharging normally
	{
		unsigned char MeterBar;

		MeterBar = (m_lChBatt - ((S3_N_BATT_SEGS + 1) * S3_CHARGER_SEG_GAP)) /
														S3_N_BATT_SEGS;

		// 5 bars
		for (char i = 0; i < S3_N_BATT_SEGS; i++)
		{
			if (SoC > i * 100 / S3_N_BATT_SEGS)
			{
				if (S3TxGetAlarms(Rx, Tx) & S3_TX_BATT_ALARM)
				{
					if (m_Parent->m_AnimateState)
						SelectObject(m_HDC, m_hBrushRed);
					else
						SelectObject(m_HDC, m_hBrushLightGrey);
				}
				else if (SoC <= S3_SOC_ALARM)
					SelectObject(m_HDC, m_hBrushRed);
				else if (SoC <= S3_SOC_WARN)
					SelectObject(m_HDC, m_hBrushAmber);
				else
					SelectObject(m_HDC, m_hBrushGreen);
			}
			else
				SelectObject(m_HDC, m_hBrushLightGrey);

			RoundRect(m_HDC,
				xref + S3_CHARGER_SEG_GAP + i * (MeterBar + S3_CHARGER_SEG_GAP),
				yref + S3_CHARGER_SEG_GAP,
				xref + S3_CHARGER_SEG_GAP + (i + 1) * (MeterBar + S3_CHARGER_SEG_GAP) - 1,
				yref + m_wChBatt - S3_CHARGER_SEG_GAP + 1,
				1, 1);
		}

		unsigned short ATTE = S3TxGetATTE(Rx, Tx);
		unsigned short h = ATTE / 60;
		unsigned short m = ATTE % 60;
		
		if (h < 10)
			status.Format(_T("%d%c (%dh:%02dm)"), SoC, '%', h, m);
		else
			status.Format(_T("%d%c (%d h)"), SoC, '%', h);
	}

	fntRc.left = xref + m_lChBatt + 10;
	fntRc.top = yref + 2;
	fntRc.right = m_RectTxBatt.right - RMARGIN;
	fntRc.bottom = yref + m_wChBatt;

	DrawText(m_HDC, status, -1, &fntRc, DT_RIGHT);

}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawTxNodeNameEdit(char Rx, char Tx, char IP)
{
	CString str;
	str.Format(_T("%S"), S3GetNodeName(Rx, Tx, IP));

	char ParaRow = S3GDI_ParaRowMap(S3_TXTX_NODENAME);

	// Add width to accommodate 8 characters
	CRect RectText(
			m_RectTxTable.left + IP * wIPCol,
			m_RectTxTable.top + pTxIPRows[ParaRow],
			m_RectTxTable.left + (IP + 1) * wIPCol + 50,
			m_RectTxTable.top + pTxIPRows[ParaRow + 1] + 10);

	KeepInRect(m_RectTxTable, RectText);

	m_GDINodeNameEdit->Edit(RectText, str);
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3InitGDITxGain(char Rx, char Tx, char IP, int yref)
{
	S3InitGDIIPGain(Rx, Tx, IP, m_RectTxTable.left + IP * wIPCol, yref);
}

// ----------------------------------------------------------------------------