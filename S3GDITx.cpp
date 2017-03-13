// ----------------------------------------------------------------------------
// Tx drawing functions for main overview screen

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
#include "S3ControllerDlg.h"

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDITxSel(char Rx, char Tx,
	int xref, int yref)
{
	S3TxType	TxType;
	char		ActiveIP, TestIP, ActiveTx;

	ActiveIP = S3TxGetActiveIP(Rx, Tx);
	TestIP = S3TxGetTestToneIP(Rx, Tx);
	TxType = S3TxGetType(Rx, Tx);
	S3TxPwrMode PowerState = S3TxGetPowerStat(Rx, Tx);
	ActiveTx = S3RxGetActiveTx(Rx);
	char BattValid = S3TxGetBattValid(Rx, Tx);

	S3TxSetCoords(Rx, Tx, xref, yref);

	SelectObject(m_HDC, m_hBrushIP);

	// Draw Tx1/Tx8 input(s)
	if (PowerState < S3_TX_SLEEP) // && !S3TxGetTest(Rx, Tx))
	{
		if (TxType == S3_Tx8)
		{
			for (int i = 0; i < S3_MAX_IPS; i++)
			{
				double xrad, yrad;

				if (i == ActiveIP)
				{
					xrad = m_posActiveIP * cos(m_th_l[i]);
					yrad = m_posActiveIP * sin(m_th_l[i]);

					SelectObject(m_HDC, m_hPenIPLive);
					SelectObject(m_HDC, m_hLiveIPBrush);

					if (S3IPGetAlarms(Rx, Tx, i) & S3_IP_OVERDRIVE)
					{
						if (m_Parent->m_AnimateState)
							SelectObject(m_HDC, m_hPenAlarm);
					}

					if (i != TestIP)
					{	
						S3DrawGDIIP(m_hbmpTxSelIPAct, m_hbmpTxSelIPActAlrm, Rx, Tx, i,
							xref - (int)xrad, yref + (int)yrad, m_radTxSelIPAct);
					}
					else
					{
						S3DrawGDITxTestIP(xref, yref, m_radTxSel, -m_th_l[i], 1.6);
					}
				}
				else
				{
					xrad = m_posIP * cos(m_th_l[i]);
					yrad = m_posIP * sin(m_th_l[i]);

					SelectObject(m_HDC, m_hPenIPOff);
					SelectObject(m_HDC, m_hBrushIP);

					if (S3IPGetAlarms(Rx, Tx, i) & S3_IP_OVERDRIVE)
					{
						if (m_Parent->m_AnimateState)
							SelectObject(m_HDC, m_hPenAlarm);
					}

					if (i != TestIP)
					{
						S3DrawGDIIP(m_hbmpTxSelIPInact, m_hbmpTxSelIPInactAlrm, Rx, Tx, i,
							xref - (int)xrad, yref + (int)yrad, m_radTxSelIP);
					}
					else
					{
						S3DrawGDITxTestIP(xref, yref, m_radTxSel, -m_th_l[i], 1.6);
					}
				}
			}
		}
		else if (TxType == S3_Tx1)
		{
			SelectObject(m_HDC, m_hPenIPLive);
			SelectObject(m_HDC, m_hLiveIPBrush);

			if (S3IPGetAlarms(Rx, Tx, 0) & S3_IP_OVERDRIVE)
			{
				if (m_Parent->m_AnimateState)
					SelectObject(m_HDC, m_hPenAlarm);
			}
			

			if (0 != TestIP)
			{
				S3DrawGDIIP(m_hbmpTxSelIPAct, m_hbmpTxSelIPActAlrm, Rx, Tx, 0,
					xref, yref + (int)(m_radRatioPosActIP * m_radTxSel), m_radTxSelIPAct);
			}
			else
			{
				S3DrawGDITxTestIP(xref, yref, m_radTxSel, -M_PI / 2.0, 1.6);
			}
		}
	} // !Sleep

	// Draw Tx body
	if ((TxType == S3_Tx1 || TxType == S3_Tx8)) // && !S3TxGetTest(Rx, Tx))
	{
		SelectObject(m_HDC, m_hPenIPOff);
		// SelectObject(m_HDC, m_hPenNone);
		
		// Insert bitmap
		if (PowerState >= S3_TX_SLEEP)
		{	
			S3BLT(m_hbmpTxSelSleep,	xref - m_radTxSel,	yref - m_radTxSel,
										2 * m_radTxSel,		2 * m_radTxSel);

			if (m_Parent->m_AnimateState && S3TxGetEmergency(Rx, Tx))
				S3BLT(m_hbmpTxTxBattHot,
							xref + m_radTxSel + m_radTxSel / 8,	yref - m_radTxSel - + m_radTxSel / 4,
							8, 28);

		}
		else if (ActiveTx != -1 && ActiveTx != Tx)
		{
			if (S3TxGetAnyAlarm(Rx, Tx) && m_Parent->m_AnimateState)
			{
				S3BLT(m_hbmpTxLrgAlarm,	xref - m_radTxSel,	yref - m_radTxSel,
											2 * m_radTxSel,		2 * m_radTxSel);
			}
			else if (!S3TxRLLStable(Rx, Tx) && m_Parent->m_AnimateState)
			{
				// S3BLT(m_hbmpTxUnselAct, xref - m_radTxUnsel, yref - m_radTxUnsel,
				//	2 * m_radTxUnsel, 2 * m_radTxUnsel);
			}
			else
			{
				// Grey large
				S3BLT(m_hbmpTxSelInact,	xref - m_radTxSel,	yref - m_radTxSel,
											2 * m_radTxSel,		2 * m_radTxSel);
			}
		}
		else
		{
			if (S3TxGetAnyAlarm(Rx, Tx) && m_Parent->m_AnimateState)
			{
				S3BLT(m_hbmpTxLrgAlarm,	xref - m_radTxSel,	yref - m_radTxSel,
											2 * m_radTxSel,		2 * m_radTxSel);
			}
			else if (!S3TxRLLStable(Rx, Tx) && m_Parent->m_AnimateState)
			{
				// S3BLT(m_hbmpTxUnselAct, xref - m_radTxUnsel, yref - m_radTxUnsel,
				//	2 * m_radTxUnsel, 2 * m_radTxUnsel);
			}
			else
			{	// Orange large
				S3BLT(m_hbmpTxSelAct,	xref - m_radTxSel, yref - m_radTxSel,
											2 * m_radTxSel, 2 * m_radTxSel);
			}
		}

		// Removed as interfered with having Tx Id central a la original drawing
		if (0 && TxType == S3_Tx8 && PowerState < S3_TX_SLEEP)
		{
			// Indicate selected input
			SelectObject(m_HDC, m_hPenIPLive);
			SelectObject(m_HDC, m_hLiveIPBrush);

			S3DrawCircle(xref - 7 - 5, yref + 20 - 7, 10);

			CRect fntRc(xref - 7 - 9,	yref - 20 + 22, 
						xref + 13 - 10,	yref + 10 + 22);

			SelectObject(m_HDC, m_hFontS);
			CString str;
			str.Format(_T("%d"), ActiveIP + 1);

			DrawText(m_HDC, str, -1, &fntRc, DT_LEFT);
		}

		S3DrawGDITxId(Rx, Tx, xref, yref);

		if (PowerState < S3_TX_SLEEP)
			S3DrawGDIBatt(Rx, Tx, xref, yref);
	}
	else
	{
		if (1) // !S3TxGetTest(Rx, Tx))
		{
			S3BLT(m_hbmpTxLrgEmpty,	xref - m_radTxSel,	yref - m_radTxSel,
									2 * m_radTxSel,		2 * m_radTxSel);
		}
		else
		{	// In test signal mode
			SelectObject(m_HDC, m_hPenIPOff);
			SelectObject(m_HDC, m_hBrushTxTest);

			S3DrawCircle(xref, yref, m_radTxSel);
			S3DrawGDITxId(Rx, Tx, xref, yref);
		}
	}

}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDITxId(char Rx, char Tx, int xref, int yref)
{
	int offset = 0; // 10

	CRect fntRc(	xref - 7 + offset,	yref - 20 - offset,
					xref + 13 + offset,	yref + 10 - offset);

	if (S3TxGetPowerStat(Rx, Tx) >= S3_TX_SLEEP)
		SetTextColor(m_HDC, m_crWhite);
	else
		SetTextColor(m_HDC, m_crTextNorm);

	SelectObject(m_HDC, m_hFontL);
	CString str;
	str.Format(_T("%d"), Tx + 1);
	DrawText(m_HDC, str, -1, &fntRc, DT_LEFT);

	SetTextColor(m_HDC, m_crTextNorm);
}

// ----------------------------------------------------------------------------
// Unselected (small) transmitters.
// TODO: Needs splitting up

void CS3GDIScreenMain::S3DrawGDITxUnsel(char Rx, char Tx,
	int xref, int yref)
{
	char		IsLeft = 0;

	char		ActiveIP = S3TxGetActiveIP(Rx, Tx);
	char		TestIP = S3TxGetTestToneIP(Rx, Tx);
	S3TxType	TxType = S3TxGetType(Rx, Tx);
	S3TxPwrMode	PowerState = S3TxGetPowerStat(Rx, Tx);
	char		ActiveTx = S3RxGetActiveTx(Rx);

	S3TxSetCoords(Rx, Tx, xref, yref);

	if (xref < Rx * m_RxSep + m_RxSep / 2)
		IsLeft = 1;

	SelectObject(m_HDC, m_hBrushIP);
	if (PowerState < S3_TX_SLEEP) // && !S3TxGetTest(Rx, Tx))
	{
		if (TxType == S3_Tx8)
		{
			for (char IP = 0; IP < S3_MAX_IPS; IP++)
			{
				double xrad, yrad;

				if (IP == ActiveIP)
				{
					if (IsLeft)
					{
						xrad = m_posActiveIPUnsel * cos(m_th_l[IP]);
						yrad = m_posActiveIPUnsel * sin(m_th_l[IP]);
					}
					else
					{
						xrad = m_posActiveIPUnsel * cos(m_th_r[IP]);
						yrad = m_posActiveIPUnsel * sin(m_th_r[IP]);
					}

					SelectObject(m_HDC, m_hPenIPLive);
					SelectObject(m_HDC, m_hLiveIPBrush);

					if (S3IPGetAlarms(Rx, Tx, IP) & S3_IP_OVERDRIVE)
					{
						if (m_Parent->m_AnimateState)
							SelectObject(m_HDC, m_hPenAlarm);
					}

					// Use primitives to draw alarmed inputs for now. All bitmapped
					// eventually
					
					if (IP != TestIP)
					{
						S3DrawGDIIP(m_hbmpTxUnselIPAct, m_hbmpTxUnselIPActAlrm, Rx, Tx, IP,
							xref - (int)xrad, yref + (int)yrad, m_radTxUnselIPAct);
					}
					else
					{
						if (IsLeft)
							S3DrawGDITxTestIP(xref, yref, m_radTxUnsel, -m_th_l[IP], 1.4);
						else
							S3DrawGDITxTestIP(xref, yref, m_radTxUnsel, -m_th_r[IP], 1.4);
					}
				}
				else
				{
					if (IsLeft)
					{
						xrad = m_posActiveIPUnsel * cos(m_th_l[IP]);
						yrad = m_posActiveIPUnsel * sin(m_th_l[IP]);
					}
					else
					{
						xrad = 1.1 * m_posIPUnsel * cos(m_th_r[IP]);
						yrad = 1.1 * m_posIPUnsel * sin(m_th_r[IP]);
					}

					SelectObject(m_HDC, m_hPenIPOff);
					SelectObject(m_HDC, m_hBrushIP);

					if (S3IPGetAlarms(Rx, Tx, IP) & S3_IP_OVERDRIVE)
					{
						if (m_Parent->m_AnimateState)
							SelectObject(m_HDC, m_hPenAlarm);
					}

					if (IP != TestIP)
					{
						S3DrawGDIIP(m_hbmpTxUnselIPInact, m_hbmpTxUnselIPInactAlrm, Rx, Tx, IP,
							xref - (int)xrad, yref + (int)yrad, m_radTxUnselIP);
					}
					else
					{
						if (IsLeft)
							S3DrawGDITxTestIP(xref, yref, m_radTxUnsel, -m_th_l[IP], 1.4);
						else
							S3DrawGDITxTestIP(xref, yref, m_radTxUnsel, -m_th_r[IP], 1.4);
					}
				}
			}
		}
		else if (TxType == S3_Tx1)
		{
			SelectObject(m_HDC, m_hPenIPLive);
			SelectObject(m_HDC, m_hLiveIPBrush);

			if (S3IPGetAlarms(Rx, Tx, 0) & S3_IP_OVERDRIVE)
			{
				if (m_Parent->m_AnimateState)
					SelectObject(m_HDC, m_hPenAlarm);
			}

			if (0 != TestIP)
			{
				S3DrawGDIIP(m_hbmpTxUnselIPAct, m_hbmpTxUnselIPActAlrm, Rx, Tx, 0, 
					xref, yref + (int)(m_radRatioPosActIP * m_radTxUnsel), m_radTxUnselIPAct);
			}
			else
			{
				S3DrawGDITxTestIP(xref, yref, m_radTxUnsel, -M_PI / 2.0, 1.4);
			}
		}
	}
	
	if ((TxType == S3_Tx1 || TxType == S3_Tx8)) // && !S3TxGetTest(Rx, Tx))
	{
		SelectObject(m_HDC, m_hPenIPOff);

		// Insert bitmap
		if (PowerState >= S3_TX_SLEEP)
		{	
			S3BLT(m_hbmpTxUnselSleep, xref - m_radTxUnsel, yref - m_radTxUnsel,
				2 * m_radTxUnsel, 2 * m_radTxUnsel);
		}
		else if (ActiveTx != -1 && ActiveTx != Tx)
		{
			if (S3TxGetAnyAlarm(Rx, Tx) && m_Parent->m_AnimateState)
			{
				S3BLT(m_hbmpTxSmlAlarm, xref - m_radTxUnsel, yref - m_radTxUnsel,
					2 * m_radTxUnsel, 2 * m_radTxUnsel);
			}
			else if (!S3TxRLLStable(Rx, Tx) && m_Parent->m_AnimateState)
			{
				// S3BLT(m_hbmpTxUnselAct, xref - m_radTxUnsel, yref - m_radTxUnsel,
				//	2 * m_radTxUnsel, 2 * m_radTxUnsel);
			}
			else
			{	// Small orange
				S3BLT(m_hbmpTxUnselInact, xref - m_radTxUnsel, yref - m_radTxUnsel,
					2 * m_radTxUnsel, 2 * m_radTxUnsel);
			}
		}
		else
		{
			if (S3TxGetAnyAlarm(Rx, Tx) && m_Parent->m_AnimateState)
			{
				S3BLT(m_hbmpTxSmlAlarm, xref - m_radTxUnsel, yref - m_radTxUnsel,
					2 * m_radTxUnsel, 2 * m_radTxUnsel);
			}
			else if (!S3TxRLLStable(Rx, Tx) && m_Parent->m_AnimateState)
			{
				// S3BLT(m_hbmpTxUnselAct, xref - m_radTxUnsel, yref - m_radTxUnsel,
				//	2 * m_radTxUnsel, 2 * m_radTxUnsel);
			}
			else
			{
				S3BLT(m_hbmpTxUnselAct, xref - m_radTxUnsel, yref - m_radTxUnsel,
					2 * m_radTxUnsel, 2 * m_radTxUnsel);
			}
		}

		S3DrawGDITxIdUnsel(Rx, Tx, xref, yref, IsLeft);

		if (PowerState < S3_TX_SLEEP)
			S3DrawGDIBattUnsel(Rx, Tx, xref, yref, IsLeft);
	}
	else
	{
		if (1) // !S3TxGetTest(Rx, Tx))
		{
			// Draw outline
			//SelectObject(m_HDC, m_hPenFOL1Dark);
			//SelectObject(m_HDC, GetStockObject(HOLLOW_BRUSH));
			//S3DrawCircle(xref, yref, m_radTxUnsel);

			if (S3RxIsActiveTx(Rx, Tx))
			{
				S3BLT(m_hbmpTxSmlEmpty, xref - m_radTxUnsel, yref - m_radTxUnsel,
						2 * m_radTxUnsel, 2 * m_radTxUnsel);

				SelectObject(m_HDC, m_hPenFOL1Dark);
				S3DrawCircle(xref, yref, m_radTxUnsel + 2);
			}
			else
				S3BLT(m_hbmpTxSmlEmpty, xref - m_radTxUnsel, yref - m_radTxUnsel,
						2 * m_radTxUnsel, 2 * m_radTxUnsel);
		}
		else
		{	
			// In test signal mode
			SelectObject(m_HDC, m_hPenIPOff);
			SelectObject(m_HDC, m_hBrushTxTest);

			S3DrawCircle(xref, yref, m_radTxUnsel);
			S3DrawGDITxIdUnsel(Rx, Tx, xref, yref, IsLeft);
		}
	}
	
	// Draw horizontal line to 'uplink' FOL
	if ((ActiveTx == Tx || ActiveTx == -1) && S3TxFOLLive(Rx, Tx))
		SelectObject(m_HDC, m_hPenFOL1);
	else
		SelectObject(m_HDC, m_hPenFOL1Dark);

	// Draw horizontal FOL connection to right or left of 'spine'
	if (IsLeft)
	{	// +/- width/2 of Rx6 FOL 'uplink' as drawn after
		MoveToEx(m_HDC, Rx * m_RxSep + m_RxSep / 2 - 2, yref, NULL);
		LineTo(m_HDC, xref + m_radTxUnsel, yref);
	}
	else
	{
		MoveToEx(m_HDC, Rx * m_RxSep + m_RxSep / 2 + 2, yref, NULL);
		LineTo(m_HDC, xref - m_radTxUnsel, yref);
	}

	// Overwrite vertical 'uplink' with 'dark' FOL below this Tx
	if (ActiveTx == Tx)
	{
		SelectObject(m_HDC, m_hPenFOL6Dark);

		MoveToEx(m_HDC, Rx * m_RxSep + m_RxSep / 2, yref + 1, NULL);
		LineTo(m_HDC, Rx * m_RxSep + m_RxSep / 2, m_TxTop);
	}

	// 'Selected' no longer a thing - should remove all references
	// Indicate selected
	/*
	if (S3IsSelected(Rx, Tx, -1))
	{
		// SelectObject(m_HDC, m_hPenIPSelected);
		SelectObject(m_HDC, m_hPenSel);
		SelectObject(m_HDC, GetStockObject(HOLLOW_BRUSH));

		S3DrawCircle(xref, yref, m_radTxUnsel + 10);
	}
	*/
}

// ----------------------------------------------------------------------------
// Draw 1:5 icoceles triangle with point at rad from xref, yref, pointing to
// xref, yref.
// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDITxTestIP(int xref, int yref, int rad,
	double th, double size)
{
	double	x[3], y[3];
	POINT	p[3];

	// Isoceles triangle 1:5 base:height
	x[0] = (double)-rad;		y[0] = 0.0;
	x[1] = -size * (double)rad;	y[1] = size * (double)rad / 10.0;
	x[2] = -size * (double)rad;	y[2] = -size * (double)rad / 10.0;

	// Rotate and translate
	for (char i = 0; i < 3; i++)
	{
		p[i].x = (int)(x[i] * cos(th) - y[i] * sin(th)) + xref;
		p[i].y = (int)(x[i] * sin(th) + y[i] * cos(th)) + yref;
	}

	Polygon(m_HDC, p, 3);
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDITxIdUnsel(char Rx, char Tx, int xref, int yref,
																char IsLeft)
{
	int offset = 0; // 4
	RECT fntRc;
	fntRc.top = yref - 11 - offset;
	fntRc.bottom = yref + 5 - offset;

	if (IsLeft)
	{
		fntRc.left = xref - 5 + offset;
		fntRc.right = xref + 10 + offset;
	}
	else
	{
		fntRc.left = xref - 5 - offset;
		fntRc.right = xref + 10 - offset;
	}

	if (S3TxGetPowerStat(Rx, Tx) >= S3_TX_SLEEP)
		SetTextColor(m_HDC, m_crWhite);
	else
		SetTextColor(m_HDC, m_crTextNorm);

	SelectObject(m_HDC, m_hFontS);
	CString str;
	str.Format(_T("%d"), Tx + 1);
	DrawText(m_HDC, str, -1, &fntRc, DT_LEFT);

	SetTextColor(m_HDC, m_crTextNorm);
}

// ----------------------------------------------------------------------------
// Referenced to Tx reference location

void CS3GDIScreenMain::S3DrawGDIBatt(char Rx, char Tx, int xref, int yref)
{
	unsigned char SoC = S3TxGetBattSoC(Rx, Tx);

	if (SoC == UCHAR_MAX) // Comms failed
		return;

	unsigned short a = S3TxGetAlarms(Rx, Tx);

	// Don't flash if invalid
	if (S3TxGetBattValid(Rx, Tx) &&	(a & S3_TX_BATT_ALARM) &&
												m_Parent->m_AnimateState)
		return;

	int	batt_xref = xref + 25;
	int batt_yref = yref - 30;

	if (!S3TxGetBattValid(Rx, Tx))
	{
		// Draw dead battery
		S3BLT(m_hbmpTxBatt[S3_N_BATT_SEGS + 1], batt_xref, batt_yref,
			14, 31);
		S3BLT(m_hbmpTxSelBattInvalid, batt_xref - 18, batt_yref - 8,
			48, 48);
	}
	else if (S3TxBattGetAlarms(Rx, Tx) & S3_TX_BATT_COLD)
	{
		S3BLT(m_hbmpTxBatt[S3_N_BATT_SEGS + 1], batt_xref, batt_yref, 14, 31);
		S3BLT(m_hbmpTxTxBattCold, batt_xref + 8 / 2, batt_yref, 8, 28);
	}
	else if (S3TxBattGetAlarms(Rx, Tx) & S3_TX_BATT_HOT)
	{
		S3BLT(m_hbmpTxBatt[S3_N_BATT_SEGS + 1], batt_xref, batt_yref, 14, 31);
		S3BLT(m_hbmpTxTxBattHot, batt_xref + 8 / 2, batt_yref, 8, 28);
	}
	else // Discharging normally
	{
		HDC	h;

		if (SoC > 80)
			h = m_hbmpTxBatt[0];
		else if (SoC > 60)
			h = m_hbmpTxBatt[1];
		else if (SoC > 40)
			h = m_hbmpTxBatt[2];
		else if (SoC > 20)
			h = m_hbmpTxBatt[3];
		else if (SoC > S3_SOC_WARN)
			h = m_hbmpTxBatt[4];
		else if (SoC > S3_SOC_ALARM)
			h = m_hbmpTxBatt[5];
		else
			h = m_hbmpTxBatt[6];

		S3BLT(h, batt_xref, batt_yref, 14, 31);
	}
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDIBattUnsel(char Rx, char Tx,
	int xref, int yref, char IsLeft)
{
	char SoC = S3TxGetBattSoC(Rx, Tx);

	if (SoC == UCHAR_MAX)
		return;

	unsigned short a = S3TxGetAlarms(Rx, Tx);

	// Don't flash if invalid
	if (S3TxGetBattValid(Rx, Tx) && (a & S3_TX_BATT_ALARM) && m_Parent->m_AnimateState)
		return;

	int	batt_xref, batt_yref;

	if (IsLeft)
		batt_xref = xref + m_radTxUnsel - 9 + 6; // - m_radTxUnsel / 2 - 14 + 5;
	else
		batt_xref = xref - m_radTxUnsel - 6; // + m_radTxUnsel / 2 - 5;

	batt_yref = yref - m_radTxUnsel - m_radTxUnsel / 2 + 3;

	if (!S3TxGetBattValid(Rx, Tx))
	{
		S3BLT(m_hbmpTxUBatt[S3_N_BATT_SEGS + 1], batt_xref, batt_yref, 9, 20);
		S3BLT(m_hbmpTxUnselBattInvalid, batt_xref - 9, batt_yref - 2, 24, 24);
	}
	else if (S3TxBattGetAlarms(Rx, Tx) & S3_TX_BATT_COLD)
	{
		S3BLT(m_hbmpTxUBatt[S3_N_BATT_SEGS + 1], batt_xref, batt_yref, 9, 20);
		S3BLT(m_hbmpTxTxBattCold, batt_xref, batt_yref - 5, 8, 28);
	}
	else if (S3TxBattGetAlarms(Rx, Tx) & S3_TX_BATT_HOT)
	{
		S3BLT(m_hbmpTxUBatt[S3_N_BATT_SEGS + 1], batt_xref, batt_yref, 9, 20);
		S3BLT(m_hbmpTxTxBattHot, batt_xref, batt_yref - 5, 8, 28);
	}
	else // Discharging normally
	{
		HDC	h;

		if (SoC > 80)
			h = m_hbmpTxUBatt[0];
		else if (SoC > 60)
			h = m_hbmpTxUBatt[1];
		else if (SoC > 40)
			h = m_hbmpTxUBatt[2];
		else if (SoC > 20)
			h = m_hbmpTxUBatt[3];
		else if (SoC > S3_SOC_WARN)
			h = m_hbmpTxUBatt[4];
		else if (SoC > S3_SOC_ALARM)
			h = m_hbmpTxUBatt[5];
		else
			h = m_hbmpTxUBatt[6];

		S3BLT(h, batt_xref, batt_yref, 9, 20);
	}
}

// ----------------------------------------------------------------------------
// Draw RF input bitmap (h1) and outer alarm bitmap (h2) if indicated

void CS3GDIScreenMain::S3DrawGDIIP(HDC h1, HDC h2, char Rx, char Tx, char IP,
	int xref, int yref, int r1)
{
	int	r2 = r1 + 1; // Assume alarm bitmap is one pixel bigger than normal.


	if (S3IPGetAlarms(Rx, Tx, IP) & S3_IP_OVERDRIVE)
		S3BLT(h2, xref - r2, yref - r2, 2 * r2, 2 * r2);
	else
		S3BLT(h1, xref - r1, yref  - r1, 2 * r1, 2 * r1);

}

// ----------------------------------------------------------------------------
