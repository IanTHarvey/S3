// ----------------------------------------------------------------------------
// Draws the receiver subscreen consisting of receiver information and a
// column for each transmitter.
//
// Keep code aligned with S3GDIScreenTx.cpp.
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

#define S3_RX_TABLE_R_MARG	8
#define S3_N_RLL_TICKS		15

char nTxParameters;
int hTxRow;
int wTxPara;
int wTxCol;

// TODO: Not used. Will be?
extern char S3GDI_RxRowParaMap[];
char S3GDI_RxParaRowMap(char para);

extern TRIVERTEX vertex_g[2];
extern TRIVERTEX vertex_r[2];
extern TRIVERTEX vertex_b[2];
extern TRIVERTEX vertex_grey[2];
extern TRIVERTEX vertex_dark_grey[2];
extern GRADIENT_RECT gRect; // = {0, 1};

#define S3_N_RXTX_PARAS	8

int hRxTxRows[S3_N_RXTX_PARAS] = {0, 35, 30, 160, 40, 40, 80, 40};
int pRxTxRows[S3_N_RXTX_PARAS];

CClickText	*RxAlarm;

// ----------------------------------------------------------------------------

int RxTxRowsCumSum()
{
	int	CumSum = 0;

	for(char i = 0; i < S3_N_RXTX_PARAS; i++)
	{
		CumSum += hRxTxRows[i];
		pRxTxRows[i] = CumSum;
	}

	return 0;
}

// ----------------------------------------------------------------------------
// Find row relates to the parameter ID
char S3GDI_RxParaRowMap(char para)
{
	for(char i = 0; i < S3GDI_MAX_TX_PARAS; i++)
		if (S3GDI_RxRowParaMap[i] != -1 && S3GDI_RxRowParaMap[i] == para)
			return i;

	return -1;
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3InitGDIRxScreen(void)
{
	m_wRxRx = m_RectScreen.Width() / 4;
	m_hTx = (m_RectScreen.bottom - m_RectHeader.bottom) / S3_MAX_RXS;
	m_wRLL = 30;
	m_hRLL = 100; // m_hTx - 5 - 5;
	m_wRxParaList = 100; // TODO: May not be required
	
	RxTxRowsCumSum();
	
	m_RectRxRx = m_RectScreen;
	m_RectRxRx.top = m_RectHeader.bottom;
	m_RectRxRx.right = m_RectRxRx.left + m_wRxRx;
	m_RectRxRx.bottom = m_RectRxRx.bottom - S3_HEIGHT_MSG_BAR;

	m_RectRxParaList = m_RectRxRx;
	m_RectRxParaList.left = m_RectRxRx.right;
	m_RectRxParaList.right = m_RectRxParaList.left + m_wRxParaList;

	m_RectRxTable = m_RectRxParaList;
	m_RectRxTable.left = m_RectRxParaList.right;
	m_RectRxTable.right = m_RectScreen.right;

	m_RectRxTx = m_RectRxRx;
	m_RectRxTx.left = m_RectRxTable.left;
	m_RectRxTx.right = m_RectRxTx.left + m_RectRxTable.Width() / S3_MAX_RXS;

	int	yTop = m_RectRxRx.top + HEAD_ROW;
	char RowCnt = 0;

	m_RxNodeName = new CS3NameValue(
				m_RectRxRx.left, yTop + RowCnt++ * PARA_ROW, m_wRxRx,
				_T("Name"), _T("12345678"), true, S3_RXRX_NODENAME);
	m_RectRxNodeName = m_RxNodeName->RectEdit(m_HDC, m_hFontL);

	m_RxType = new CS3NameValue(
				m_RectRxRx.left,	yTop + RowCnt++ * PARA_ROW, m_wRxRx,
				_T("Type"), _T("RxN+"), false);
	m_RxType->RectEdit(m_HDC, m_hFontS);

	CString str;

	str.Format(_T("\u03F4%cC"), 0x00b0);

	m_RxTemp = new CS3NameValue(m_Parent, 
				m_RectRxRx.left,	yTop + RowCnt++ * PARA_ROW, m_wRxRx,
				str, _T("-1288"), false);
	m_RxTemp->RectEdit(m_HDC, m_hFontS);

	m_RxVcc = new CS3NameValue(	m_Parent, 
				m_RectRxRx.left,	yTop + RowCnt++ * PARA_ROW, m_wRxRx,
				_T("Vcc (V)"), _T("222.8"), false);
	m_RxVcc->RectEdit(m_HDC, m_hFontS);

	RowCnt += 1;

	m_RxSN = new CS3NameValue(
				m_RectRxRx.left,	yTop + RowCnt++ * PARA_ROW, m_wRxRx,
				_T("S/N"), _T("SN1234567890"), false); // 9 char
	m_RxSN->RectEdit(m_HDC, m_hFontS);

	m_RxPN = new CS3NameValue(
				m_RectRxRx.left,	yTop + RowCnt++ * PARA_ROW, m_wRxRx,
				_T("P/N"), _T("PN1234567890123"), false); // 19 char
	m_RxPN->RectEdit(m_HDC, m_hFontS);

	/*
	m_RxHW = new CS3NameValue(
				m_RectRxRx.left,	yTop + RowCnt++ * PARA_ROW, m_wRxRx,
				_T("HW"), _T("Unknown"), false);
	m_RxHW->RectEdit(m_HDC, m_hFontS);
	*/
	
	m_RxFWDate = new CS3NameValue(
				m_RectRxRx.left,	yTop + RowCnt++ * PARA_ROW, m_wRxRx,
				_T("Date"), _T("Unknown"), false);
	m_RxFWDate->RectEdit(m_HDC, m_hFontS);

	m_RxFW = new CS3NameValue(
				m_RectRxRx.left, yTop + RowCnt++ * PARA_ROW, m_wRxRx,
				_T("F/W"), _T("Unknown"), false);
	m_RxFW->RectEdit(m_HDC, m_hFontS);

	m_RectRxMsg = m_RectScreen;
	m_RectRxMsg.top = m_RectRxMsg.bottom - S3_HEIGHT_MSG_BAR;

	m_RectRxMsg.left += 50;

	RxAlarm = new CClickText(this, m_RectRxMsg, m_HDC, DT_LEFT, 1);
	
	m_RectRxMsg.left -= 50;
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3CloseGDIRxScreen(void)
{
	delete m_RxNodeName;
	delete m_RxType;
	delete m_RxTemp;
	delete m_RxVcc;
	delete m_RxSN;
	delete m_RxPN;
	// delete m_RxHW;
	delete m_RxFWDate;
	delete m_RxFW;
	delete RxAlarm;
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDIRxScreen(void)
{
	// Make sure we're not left on this screen if Rx has been
	// disconnected
	char Rx = S3GetCurrentRx();

	if (Rx == -1 || S3GetType(Rx, -1) == S3_RxEmpty)
	{
		S3GDIChangeScreen(S3_OVERVIEW_SCREEN);
		return;
	}
	
	int xref = m_RectHeader.left, yref = m_RectHeader.bottom;

	S3DrawGDIBackButton();

	xref = 0;
	yref = m_RectHeader.bottom;

	S3DrawGDIRxRx(Rx);
	S3DrawGDIRxTxTable(Rx);
	S3DrawGDIRxMessage(Rx);
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDIRxRx(char Rx)
{
	RECT fntRc = m_RectRxRx;

	fntRc.left += NV_LMARGIN;

	SelectObject(m_HDC, m_hPenNone);
	SelectObject(m_HDC, m_hBrushBG2);

	S3_RECT_N(m_HDC, m_RectRxRx);

	SelectObject(m_HDC, m_hFontL);
	CString str;
	str.Format(_T("Receiver: %d"), Rx + 1);

	DrawText(m_HDC, str, -1, &fntRc, DT_LEFT);

	str.Format(_T("%S"), S3GetNodeName(Rx, -1, -1));
	m_RxNodeName->SetValue(str);
	m_RxNodeName->Draw(m_HDC, m_hFontS, m_hFontSB);

	str.Format(_T("%s"), S3GetModelName(Rx, -1));
	m_RxType->SetValue(str);
	m_RxType->Draw(m_HDC, m_hFontS, m_hFontSB);

	str.Format(_T("%d"), S3RxGetTemp(Rx));
	m_RxTemp->SetAlarm((S3RxGetAlarms(Rx) &
		(S3_RX_OVER_TEMP | S3_RX_UNDER_TEMP)) != 0);
	m_RxTemp->SetValue(str);
	m_RxTemp->Draw(m_HDC, m_hFontS, m_hFontSB);

	str.Format(_T("%.1f"), S3RxGetVcc(Rx) / 1000.0);
	m_RxVcc->SetAlarm((S3RxGetAlarms(Rx) &
		(S3_RX_OVER_VOLT | S3_RX_UNDER_VOLT)) != 0);
	m_RxVcc->SetValue(str);
	m_RxVcc->Draw(m_HDC, m_hFontS, m_hFontSB);

	const char		*SN, *PN, *HWV, *FWV;
	S3RxGetInfo(Rx, &SN, &PN, &HWV, &FWV);

	str.Format(_T("%S"), SN);
	m_RxSN->SetValue(str);
	m_RxSN->Draw(m_HDC, m_hFontS, m_hFontSB);

	str.Format(_T("%S"), PN);
	m_RxPN->SetValue(str);
	m_RxPN->Draw(m_HDC, m_hFontS, m_hFontSB);

	// str.Format(_T("%S"), HWV);
	// m_RxHW->SetValue(str);
	// m_RxHW->Draw(m_HDC, m_hFontS, m_hFontSB);

	str.Format(_T("%S"), S3RxGetFWDate(Rx));
	m_RxFWDate->SetValue(str);
	m_RxFWDate->Draw(m_HDC, m_hFontS, m_hFontSB);

	str.Format(_T("%S"), FWV);
	m_RxFW->SetValue(str);
	m_RxFW->Draw(m_HDC, m_hFontS, m_hFontSB);
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDIRxTx(char Rx, char Tx)
{
	SelectObject(m_HDC, m_hPenNone);

	CRect RectCol = m_RectRxTx;
	RectCol.OffsetRect(Tx * m_RectRxTx.Width(), 0);

	if (Tx % 2)
		SelectObject(m_HDC, m_hBrushBG3);
	else
		SelectObject(m_HDC, m_hBrushBG2);

	S3_RECT_N(m_HDC, RectCol);

	CRect RectItem = RectCol;

	RectItem.bottom = RectItem.top + hRxTxRows[1];

	S3DrawGDIColHeader(Rx, Tx, RectItem);

	S3TxType TxType = S3TxGetType(Rx, Tx);
	
	if (TxType == S3_TxUnconnected)
	{
		return;
	}

	if (!S3RxIsActiveTx(Rx, Tx) || S3TxGetPowerStat(Rx, Tx) == S3_TX_SLEEP)
		SetTextColor(m_HDC, m_crMenuTxtGreyed);
	else
		SetTextColor(m_HDC, m_crTextNorm);

	RectItem.top = RectItem.bottom;
	RectItem.bottom += hRxTxRows[2];

	CString str;
	
	// Name...
	str.Format(_T("%S"), S3GetNodeName(Rx, Tx, -1));
	SelectObject(m_HDC, m_hFontS);
	DrawText(m_HDC, str, -1, &RectItem, DT_CENTER);

	RectItem.top = RectItem.bottom;
	RectItem.bottom += hRxTxRows[3];

	S3DrawGDIRxRLL(Rx, Tx, RectItem.left, RectItem.top);

	SelectObject(m_HDC, m_hFontS);

	if (!S3GetLocked())
	{
		RectItem.top = RectItem.bottom;
		RectItem.bottom += hRxTxRows[4];

		str.Format(_T("%+.1f"), S3RxGetRFGain(Rx, Tx) / 100.0);
		DrawText(m_HDC, str, -1, &RectItem, DT_CENTER);

		RectItem.top = RectItem.bottom;
		RectItem.bottom += hRxTxRows[5];

		str.Format(_T("%+.1f"), S3RxGetRFLevel(Rx, Tx) / 100.0);
		DrawText(m_HDC, str, -1, &RectItem, DT_CENTER);
	}
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDIRxRLL(char Rx, char Tx, int xref, int yref)
{
	short	RLLdBm = S3RxGetRLL(Rx, Tx);
	double	RLLDisplay; // + S3_RLL_MIN_DBM;
	RLLDisplay = RLLdBm / 100.0;

	if (RLLdBm > S3_RLL_MAX_10MDBM)
		RLLdBm = S3_RLL_MAX_10MDBM;
	else if (RLLdBm < S3_RLL_MIN_10MDBM)
		RLLdBm = S3_RLL_MIN_10MDBM;

	double	RLLbar = 100.0 * (RLLdBm - S3_RLL_MIN_10MDBM) /
									(S3_RLL_MAX_10MDBM - S3_RLL_MIN_10MDBM);

	int		RLLbarPx = (int)ROUND(RLLbar);

	HGDIOBJ o_pen = SelectObject(m_HDC, m_hPenNone);
	HGDIOBJ o_brush = SelectObject(m_HDC, m_hBrushLightGrey);

	int bar_x = xref + 5;
	int	bar_y = yref + 5;

	// Draw frame
	Rectangle(m_HDC, bar_x,			bar_y,
						bar_x + m_wRLL,	bar_y + m_hRLL);

	// Draw bar
	if (0) // RLLdBm == S3_RLL_MIN_10MDBM || RLLdBm == S3_RLL_MAX_10MDBM)
	{
		SelectObject(m_HDC, m_hBrushRed);	

		Rectangle(m_HDC, bar_x,			bar_y + m_hRLL - RLLbarPx,
				bar_x + m_wRLL,	bar_y + m_hRLL);
	}
	else if (RLLdBm < S3RxGetRLLLo(Rx) || RLLdBm > S3RxGetRLLHi(Rx))
	{
		// SelectObject(m_HDC, m_hBrushAmber);
		SelectObject(m_HDC, m_hBrushRed);

		Rectangle(m_HDC, bar_x,			bar_y + m_hRLL - RLLbarPx,
				bar_x + m_wRLL,	bar_y + m_hRLL);
	}
	else // if (RLLdBm <= S3_RLL_GOOD_DBM)
	{
		if (S3RxIsActiveTx(Rx, Tx))
			SelectObject(m_HDC, m_hBrushRxRLLOn);
		else
			SelectObject(m_HDC, m_hBrushRxRLLOnUnsel);

		Rectangle(m_HDC, bar_x,			bar_y + m_hRLL - RLLbarPx,
				bar_x + m_wRLL,	bar_y + m_hRLL);
	}

	// Draw tick marks
	SelectObject(m_HDC, m_hPenIPLive);
	SelectObject(m_HDC, GetStockObject(HOLLOW_BRUSH));
	
	for(char i = 1; i < S3_N_RLL_TICKS; i++)
	{
		MoveToEx(m_HDC, bar_x,		
				bar_y + i * m_hRLL / S3_N_RLL_TICKS, NULL);

		short TickdBm = S3_RLL_MIN_10MDBM + 
			i * (S3_RLL_MAX_10MDBM - S3_RLL_MIN_10MDBM) / S3_N_RLL_TICKS;

		// Longer ticks for S3RxGetRLLLo & S3RxGetRLLHi
		if (ABS(TickdBm - S3RxGetRLLLo(Rx)) < 20)
			LineTo(m_HDC, bar_x + m_wRLL / 2 + m_wRLL / 4,
				bar_y + i * m_hRLL / S3_N_RLL_TICKS);
		else if (ABS(TickdBm - S3RxGetRLLHi(Rx)) < 20)
			LineTo(m_HDC, bar_x + m_wRLL / 2 + m_wRLL / 4,
				bar_y + i * m_hRLL / S3_N_RLL_TICKS);
		else
			LineTo(m_HDC, bar_x + m_wRLL / 2,
				bar_y + i * m_hRLL / S3_N_RLL_TICKS);
	}

	// Frame
	Rectangle(m_HDC, bar_x,			bar_y,
						bar_x + m_wRLL,	bar_y + m_hRLL);

	SelectObject(m_HDC, m_hFontS);

	RECT fntRc;

	fntRc.left = xref + m_wRLL + 5 + 5;
	fntRc.top = yref;
	fntRc.right = fntRc.left + 60;
	fntRc.bottom = fntRc.top + 30;

	CString str;

	str.Format(_T("%.1f"), S3_RLL_MAX_DBM);
	DrawText(m_HDC, str, -1, &fntRc, DT_LEFT);

	fntRc.top = yref + m_hRLL - 10;
	fntRc.bottom = fntRc.top + 30;

	str.Format(_T("%.1f"), S3_RLL_MIN_DBM);
	DrawText(m_HDC, str, -1, &fntRc, DT_LEFT);

	fntRc.left = xref + 5;
	fntRc.top = yref + m_hRLL + 5;
	fntRc.right = fntRc.left + m_RectRxTx.Width();
	fntRc.bottom = fntRc.top + 30;

	SelectObject(m_HDC, m_hFontL);

	if (S3RxGetRLL(Rx, Tx) == SHRT_MIN)
		str = "---";
	else
		str.Format(_T("%+0.1f"), RLLDisplay);
	DrawText(m_HDC, str, -1, &fntRc, DT_LEFT);

	SelectObject(m_HDC, m_hFontS);
	fntRc.top += 30;
	fntRc.bottom += 60;

	str.Format(_T("dBm"));
	DrawText(m_HDC, str, -1, &fntRc, DT_LEFT);

	SelectObject(m_HDC, o_pen);
	SelectObject(m_HDC, o_brush);
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDIRxTxTable(char Rx)
{
	SelectObject(m_HDC, m_hBrushBG2);
	S3_RECT_N(m_HDC, m_RectRxTable);

	SelectObject(m_HDC, m_hBrushBG3);
	S3_RECT_N(m_HDC, m_RectRxParaList);
	
	char Tx;
	unsigned char	RxType = S3RxGetType(Rx);

	nTxParameters = 3; // TODO: Just label, name and RLL for now
	// Row labels
	
	if (RxType == S3_Rx6)
		S3DrawGDIRxTxRowName(0, _T("Active Tx"));
	else
		S3DrawGDIRxTxRowName(0, _T(""));
	
	S3DrawGDIRxTxRowName(1, _T(""));
	S3DrawGDIRxTxRowName(2, _T("RLL (dBm)"));

	if (!S3GetLocked())
	{
		S3DrawGDIRxTxRowName(3, _T("RF Gain\n(dB)"));
		S3DrawGDIRxTxRowName(4, _T("RF Level\n(dBm)"));
		S3DrawGDIRxTxRowName(5, _T(""));
	}

	// Columns
	for (Tx = 0; Tx < S3RxGetNTx(Rx); Tx++)
	{
		S3DrawGDIRxTx(Rx, Tx);
	}
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDIRxTxRowName(char Row, wchar_t *cstr)
{
	if (Row % 2)
		SelectObject(m_HDC, m_hBrushBG3);
	else
		SelectObject(m_HDC, m_hBrushBG2);
	
	int y = m_RectRxParaList.top + pRxTxRows[Row];

	CRect RectItem(m_RectRxParaList.left, y,
		m_RectScreen.right, y + hRxTxRows[Row + 1]);

	S3_RECT_N(m_HDC, RectItem);

	SelectObject(m_HDC, m_hFontS);
		
	RectItem.right = m_RectRxParaList.right;
	RectItem.right -= S3_RX_TABLE_R_MARG;

	DrawText(m_HDC, cstr, -1, &RectItem, DT_RIGHT);
}
// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDIRxMessage(char Rx)
{
	SelectObject(m_HDC, m_hPenNone);
	SelectObject(m_HDC, m_hBrushBG4);
	COLORREF cr = SetTextColor(m_HDC, m_crTextNorm);

	S3_RECT(m_HDC, m_RectRxMsg);

	char S3AlarmString[S3_EVENTS_LINE_LEN];

	int NMsg = S3RxAlarmGetString(Rx, S3AlarmString, S3_EVENTS_LINE_LEN);

	if (NMsg)
	{
		if (S3AlarmString[0] == 'E')
		{
			S3BLT(m_hbmpSysError, m_RectRxMsg.left + 10, m_RectRxMsg.top + 5,
					32, 32);
		}
		else if (S3AlarmString[0] == 'W')
		{
			S3BLT(m_hbmpSysWarn, m_RectRxMsg.left + 10, m_RectRxMsg.top + 5,
					32, 32);
		}
		else if (S3AlarmString[0] == 'I')
		{
			S3BLT(m_hbmpSysInfo, m_RectRxMsg.left + 10, m_RectRxMsg.top + 5,
					32, 32);
		}
		else return;
		
		CString str(S3AlarmString + 2);

		RECT fntRc = m_RectRxMsg;
		// fntRc.left += 50;
		fntRc.top += 5;

		RxAlarm->Enable(true);
		RxAlarm->SetString(str);
		
		// SelectObject(m_HDC, m_hFontL);
		// DrawText(m_HDC, str, -1, &fntRc, DT_LEFT);
	}
	else
	{
		RxAlarm->Enable(false);
		RxAlarm->SetString(_T(""));
	}
}

// ----------------------------------------------------------------------------

int CS3GDIScreenMain::S3FindRxScreen(POINT p)
{
	if (m_RectBackButton.PtInRect(p))
	{
		S3GDIChangeScreen(S3_OVERVIEW_SCREEN);

		return 1;
	}

	char menu_item = m_ParaMenu->FindSelect(p);
	m_ParaMenu->Clear();

	m_GDINodeNameEdit->ShowWindow(SW_HIDE);

	char Rx, Tx, IP, Para;
	S3GetSelected(&Rx, &Tx, &IP);

#ifndef S3_AGENT
	if (S3GetRemote())
		return 0;
#endif

	if (menu_item != -1)
	{		
		Para = S3GetSelectedPara(Rx, -1, -1);
		
		S3SetParaValue(Rx, -1, -1, Para, menu_item);
		S3SetSelectedPara(Rx, -1, -1, -1);

		return 0;
	}
	else if (p.y > m_RectHeader.bottom)
	{
		if (RxAlarm->Find(p))
		{
			S3SetSelected(Rx, -1, -1);
			S3SetSelectedPara(Rx, -1, -1, S3_RX_CANCEL_ALARM);
			S3DrawGDIParaPopUp(p.x, p.y);

			return 1;
		}

		if (p.x < m_RectRxTable.left)
		{
			S3SetSelected(Rx, -1, -1);

			if (m_RectRxNodeName.PtInRect(p))
			{
				
				S3SetSelectedPara(Rx, -1, -1, S3_RXRX_NODENAME);
				S3DrawGDIParaPopUp(p.x, p.y);

				return 1;
			}

			return 0;
		}
		else
		{
			// Which transmitter?
			for (char Tx = 0; Tx < S3RxGetNTx(Rx); Tx++)
			{
				if (p.x < m_RectRxTable.left + (Tx + 1) * m_RectRxTx.Width())
				{
					m_GDINodeNameEdit->ShowWindow(SW_HIDE);

					char Rx, OldTx, OldIP;
					
					S3GetSelected(&Rx, &OldTx, &OldIP);

					// TEST: RX6
					if (0) // S3TxGetType(Rx, Tx) == S3_TxUnconnected)
					{
						S3SetSelected(Rx, -1, -1);

						return 0;
					}

					S3SetSelected(Rx, Tx, -1);

					//S3RxSetHighlightedTx(Rx, Tx);
					int found = S3FindRxScreenPara(p);

					if (found)
					{
						S3DrawGDIParaPopUp(p.x, p.y);
					}

					return found;
				}
			}
		}
	}

	S3SetSelected(Rx, -1, -1);

	return 0;
}

// ----------------------------------------------------------------------------

int CS3GDIScreenMain::S3FindRxScreenPara(POINT p)
{
	char Rx, Tx, IP;
					
	S3GetSelected(&Rx, &Tx, &IP);

	for(unsigned char i = 1; i < S3_N_RXTX_PARAS; i++)
	{
		if (p.y < m_RectRxTable.top + pRxTxRows[i])
		{
			return S3SetSelectedPara(Rx, Tx, IP, S3GDI_RxRowParaMap[i - 1]);
		}
	}

	return 0;
}

// ----------------------------------------------------------------------------

int CS3GDIScreenMain::S3GFill(TRIVERTEX *vertex, CRect rect)
{
	vertex[0].x     = rect.left + 1;
	vertex[0].y     = rect.top + 1;

	vertex[1].x     = rect.right;
	vertex[1].y     = rect.bottom;

	GradientFill(m_HDC, vertex, 2, &gRect, 1, GRADIENT_FILL_RECT_H);

	return 0;
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDIColHeaderOld(char Rx, char Tx, CRect &RectItem)
{
	char Alarms = 0; // S3IPGetAlarms(Rx, Tx, IP);

	COLORREF cr = SetTextColor(m_HDC, m_crWhite);

	// Contrast
	if (S3RxIsActiveTx(Rx, Tx))
	{	// Alternate green/red
		if (Alarms && m_Parent->m_AnimateState)
		{	// Red
			S3GFill(vertex_r, RectItem);
		}
		else
		{	// Green
			S3GFill(vertex_g, RectItem);
		}
	}
	else
	{	// Alternate white/red
		if (Alarms && m_Parent->m_AnimateState)
		{	// Red
			S3GFill(vertex_r, RectItem);
		}
		else if (S3TxIsDetected(Rx, Tx))
		{
			// Green unfilled rrectangle
			SetTextColor(m_HDC, cr);
		
			if (Tx % 2)
				SelectObject(m_HDC, m_hBrushBG2);
			else
				SelectObject(m_HDC, m_hBrushBG3);
			
			SelectObject(m_HDC, m_hPenDetected);
			S3_RECT(m_HDC, RectItem);
			SelectObject(m_HDC, m_hPenNone);
		}
		else
		{
			// Flat greys
			SetTextColor(m_HDC, cr);
		
			if (Tx % 2)
				SelectObject(m_HDC, m_hBrushDarkGrey);
			else
				SelectObject(m_HDC, m_hBrushMediumGrey);

			S3_RECT(m_HDC, RectItem);
		}
	}

	RectItem.top -= 4;
	SelectObject(m_HDC, m_hFontL);
	CString str;
	str.Format(_T("%d"), Tx + 1);
	DrawText(m_HDC, str, -1, &RectItem, DT_CENTER);

	S3TxType TxType = S3TxGetType(Rx, Tx);
	
	if (TxType == S3_TxUnconnected)
	{
		return;
	}

	if (!S3RxIsActiveTx(Rx, Tx) || S3TxGetPowerStat(Rx, Tx) == S3_TX_SLEEP)
		SetTextColor(m_HDC, m_crMenuTxtGreyed);
	else
		SetTextColor(m_HDC, cr);

}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDIColHeader(char Rx, char Tx, CRect &RectItem)
{
	char Alarms = 0; // S3IPGetAlarms(Rx, Tx, IP);
	
	bool sleep = (S3TxGetPowerStat(Rx, Tx) == S3_TX_SLEEP);

	COLORREF cr = SetTextColor(m_HDC, m_crWhite);
			
	if (Tx % 2)
		SelectObject(m_HDC, m_hBrushLightGrey);
	else
		SelectObject(m_HDC, m_hBrushMediumGrey);

	if (S3RxIsActiveTx(Rx, Tx))
	{	
		if (S3TxIsDetected(Rx, Tx))
		{
			SetTextColor(m_HDC, m_crWhite);
			// Alternate green/red
			if (Alarms && m_Parent->m_AnimateState)
			{
				S3GFill(vertex_r, RectItem); // Red
			}
			else
			{
				if (!sleep)
					S3GFill(vertex_g, RectItem); // Green
				else
					S3GFill(vertex_b, RectItem); // Blue
			}
		}
		else
		{
			// TODO: Argue whether non-detected tx can be active Tx
			// S3GFill(vertex_dark_grey, RectItem); // Grey
			S3GFill(vertex_g, RectItem); // Green
		}
	}
	else
	{	
		if (S3TxIsDetected(Rx, Tx))
		{
			SetTextColor(m_HDC, m_crTextNorm);

			if (Alarms && m_Parent->m_AnimateState)
			{
				SelectObject(m_HDC, m_hPenAlarm);
				S3_RECT(m_HDC, RectItem);
				SelectObject(m_HDC, m_hPenNone);
			}
			else
			{
				if (!sleep)
				{
					SelectObject(m_HDC, m_hPenDetected);	// Green
					// S3_RECT(m_HDC, RectItem);
					Rectangle(m_HDC, RectItem.left + 2, RectItem.top + 2,
						RectItem.right - 2, RectItem.bottom - 2);
					SelectObject(m_HDC, m_hPenNone);
				}
				else
				{
					SelectObject(m_HDC, m_hPenSleep);		// Blue
					// S3_RECT(m_HDC, RectItem);
					Rectangle(m_HDC, RectItem.left + 2, RectItem.top + 2,
						RectItem.right - 2, RectItem.bottom - 2);
					SelectObject(m_HDC, m_hPenNone);
				}
			}
		}
		else
		{
			// Flat greys
			SetTextColor(m_HDC, m_crTextNorm);

			S3_RECT(m_HDC, RectItem);
		}
	}

	RectItem.top -= 4;
	SelectObject(m_HDC, m_hFontL);
	CString str;
	str.Format(_T("%d"), Tx + 1);
	DrawText(m_HDC, str, -1, &RectItem, DT_CENTER);
}

// ----------------------------------------------------------------------------
