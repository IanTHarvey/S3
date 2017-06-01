// ----------------------------------------------------------------------------
// Rx drawing functions for main overview screen

#include "stdafx.h"

#include <stdio.h>
#include "S3DataModel.h"

#ifdef S3_AGENT
#include "S3Agent/S3AgentDlg.h"
#else
#include "S3ControllerDlg.h"
#endif

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3InitGDIRx(void)
{
	// GetBitmapDims(m_hbmpRxMainRx1, &m_wRx, &m_hRx);
	GetDCDims(m_hbmpRxMainRx1, &m_wRx, &m_hRx);

	m_RxNumXOffset = 45;
	m_RxNumYOffset = -30;
	m_RxNumRad = 15;

	// Rx system gain - right & bottom
	m_radGainVal = 15;
	m_xosGainVal = 1 * m_wRx / 2 - 18;
	m_yosGainVal = m_hRx / 4 - 4 - 10;

	m_radGainUnits = 11;
	m_xosGainUnits = m_wRx / 3 - 2 - 9;
	m_yosGainUnits = m_hRx / 2 - 2 - 12;

	// RxRLL
	// static const int		m_RxRLLBar_h[S3_MAIN_RX_RLL_BARS];
	// static const int		m_RxRLLBar_w[S3_MAIN_RX_RLL_BARS];
	m_RxRLLBar_ys = 2;		// Vertical inter-bar spacing
	m_RxRLLBar_xs = 4;		// Horizontal distance to centre-line
	m_RxRLLBar_yos = 20;	// Vertical distance from bottom bar to centre-line
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDIRx(char Rx)
{

	unsigned char RxType = S3RxGetType(Rx);

	int xref = Rx * m_RxSep + m_RxSep / 2;

	S3RxSetCoords(Rx, xref, m_RxYref);

	// Indicate live detected stuff vs stuff read from config file. Moot apart
	// from implmentation of demo mode
	if (0) // S3RxGetDetected(Rx))
	{
		HGDIOBJ pen = SelectObject(m_HDC, m_hPenAlarm);
		HGDIOBJ br = SelectObject(m_HDC, GetStockObject(HOLLOW_BRUSH));

		RoundRect(m_HDC,
			xref - m_wRx / 2 - 5, m_RxYref - m_hRx / 2 - 5,
			xref + m_wRx / 2 + 15, m_RxYref + m_hRx / 2 + 5,
			30, 30);

		SelectObject(m_HDC, pen);
		SelectObject(m_HDC, br);
	}

	if (RxType == S3_RxEmpty)
	{
		SelectObject(m_HDC, m_hPenIPOff);
		SelectObject(m_HDC, GetStockObject(HOLLOW_BRUSH));

		S3BLT(m_hbmpRxMainEmpty, xref - m_wRx / 2, m_RxYref - m_hRx / 2,
				m_wRx, m_hRx);

		S3BLT(m_hbmpRxEmptyBar, xref, m_RxYref + 10,	4, 300);
	}
	else
	{
		SelectObject(m_HDC, m_hPenNone);
		SelectObject(m_HDC, m_hBrushRx);

		// Insert bitmap
		if (RxType == S3_Rx2)
			S3BLT(m_hbmpRxMainRx2, xref - m_wRx / 2, m_RxYref - m_hRx / 2,
				m_wRx, m_hRx);
		else
			S3BLT(m_hbmpRxMainRx1, xref - m_wRx / 2, m_RxYref - m_hRx / 2,
				m_wRx, m_hRx);
		
		char SelectedTx = S3RxGetHighlightedTx(Rx);

		S3DrawGDIRxRLL2(Rx, xref, m_RxYref - 20);
		S3DrawGDIFOLSym(Rx, xref, m_RectRx.bottom + 20, SelectedTx < 4);

		if (RxType == S3_Rx6)
		{
			char ActiveTx = S3RxGetActiveTx(Rx);

			int	xrefTx = Rx * m_RxSep + m_RxSep / 2;
			int yrefTx = (m_RectTx.bottom + m_RectTx.top) / 2;

			S3DrawGDITxSel(Rx, ActiveTx, xrefTx, yrefTx);

			if (S3RxGetConnectedTxs(Rx))
				SelectObject(m_HDC, m_hPenFOL6);
			else
				SelectObject(m_HDC, m_hPenFOL6Dark);

			MoveToEx(m_HDC, xref, m_RectRx.bottom, NULL);
			LineTo(m_HDC, xref, m_TxTop);

			for (unsigned char i = 0; i < S3_MAX_RXS; i++)
			{
				char	Tx;
				int		yref;

				// Left
				for (int Tx = ActiveTx - 1; Tx >= 0; Tx--)
				{
					yref = m_RectTx.top - (ActiveTx - Tx - 1) * m_yUnselSep - m_radTxUnsel;

					S3DrawGDITxUnsel(Rx, Tx, xref - m_xUnselOffset, yref);
				}

				// Right
				for (Tx = ActiveTx + 1; Tx < S3_MAX_RXS; Tx++)
				{
					// In this case we don't have to put in the stagger extra offset
					// that would make Tx5 interfere with it's Rx.
					if (ActiveTx == 0)
						yref = m_RectTx.top - (Tx - ActiveTx - 1) * m_yUnselSep - m_radTxUnsel;
					else
						yref = m_RectTx.top - (Tx - ActiveTx - 1) * m_yUnselSep - 2 * m_radTxUnsel;

					S3DrawGDITxUnsel(Rx, Tx, xref + m_xUnselOffset, yref);
				}
			}
			
			S3DrawGDIRxGain(Rx, -ActiveTx, xref, m_RxYref);
		}
		else if (RxType == S3_Rx2)
		{
			// Draw FOL from both Txs to the Rx
			int			xrefTx, yrefTx;
			S3TxType	TxType1, TxType2;
			int			yCentreFOL = (m_RectFOL.top + m_RectFOL.bottom) / 2;
			
			// Tx 0 and uplink ---------------------
			TxType1 = S3TxGetType(Rx, 0);

			xrefTx = Rx * m_RxSep + m_RxSep / 2 - 3 * m_radTxSel / 4;
			yrefTx = (m_RectTx.bottom + m_RectTx.top) / 2 - 3 * m_radTxSel;

			S3DrawGDITxSel(Rx, 0, xrefTx, yrefTx);
			S3DrawGDIRxGain(Rx, 0, xref, m_RxYref);

			if (S3TxFOLLive(Rx, 0))
				SelectObject(m_HDC, m_hPenFOL1);
			else
				SelectObject(m_HDC, m_hPenFOL1Dark);

			// Extra MoveToEx() (and - 1) required to dovetail v & h lines.
			// TODO: Use polyline()?
			int TopDiv = 5;
			int b = m_RxYref + (TopDiv - 1) * m_hRx / (2 * TopDiv) - 2;	// Bottom of RLL bottom bar
			int	l = xref - m_RxRLLBar_w[0] / 2 - 1;
			// int r = xref;

			MoveToEx(m_HDC, l, yCentreFOL, NULL);
			LineTo(m_HDC, xrefTx, yCentreFOL); // H
			MoveToEx(m_HDC, xrefTx, yCentreFOL - 1, NULL);
			LineTo(m_HDC, xrefTx, yrefTx - m_radTxSel); // Vlower
			MoveToEx(m_HDC, l, yCentreFOL + 1, NULL);
			LineTo(m_HDC, l, b); // Vupper

			// Tx 1 and uplink ---------------------
			TxType2 = S3TxGetType(Rx, 1);

			xrefTx = Rx * m_RxSep + m_RxSep / 2 + 3 * m_radTxSel / 4;
			yrefTx = (m_RectTx.bottom + m_RectTx.top) / 2;

			S3DrawGDITxSel(Rx, 1, xrefTx, yrefTx);
			S3DrawGDIRxGain(Rx, 1, xref, m_RxYref);

			if (S3TxFOLLive(Rx, 1))
				SelectObject(m_HDC, m_hPenFOL1);
			else
				SelectObject(m_HDC, m_hPenFOL1Dark);

			l = xref + m_RxRLLBar_w[0] / 2 + 1;

			MoveToEx(m_HDC, l, yCentreFOL, NULL);
			LineTo(m_HDC, xrefTx, yCentreFOL); // H
			MoveToEx(m_HDC, xrefTx, yCentreFOL - 1, NULL);
			LineTo(m_HDC, xrefTx, yrefTx - m_radTxSel); // V
			MoveToEx(m_HDC, l, yCentreFOL + 1, NULL);
			LineTo(m_HDC, l, b); // Vupper

			// Spine uplink ---------------------

			/*
			if (TxType1 == S3_TxUnconnected && TxType2 == S3_TxUnconnected)
				SelectObject(m_HDC, m_hPenFOL2Dark);
			else
				SelectObject(m_HDC, m_hPenFOL2);

			MoveToEx(m_HDC, xref, m_RectRx.bottom, NULL);
			LineTo(m_HDC, xref, (m_RectFOL.top + m_RectFOL.bottom) / 2); // V
			*/
		}
		else if (RxType == S3_Rx1)
		{
			int	xrefTx = Rx * m_RxSep + m_RxSep / 2;
			int yrefTx = (m_RectTx.bottom + m_RectTx.top) / 2;

			S3DrawGDITxSel(Rx, SelectedTx, xrefTx, yrefTx);

			if (S3TxFOLLive(Rx, 0))
				SelectObject(m_HDC, m_hPenFOL1);
			else
				SelectObject(m_HDC, m_hPenFOL1Dark);
				
			MoveToEx(m_HDC, xref, m_RectRx.bottom, NULL);
			LineTo(m_HDC, xref, m_TxTop);

			S3DrawGDIRxGain(Rx, 0, xref, m_RxYref);
		}
	}

	// Draw number as rack slot even if empty
	S3DrawGDIRxNumber(Rx, xref + m_RxNumXOffset, m_RxYref + +m_RxNumYOffset);

	/*
	// Indicate selected
	if (S3IsSelected(Rx, -1, -1))
	{
		// SelectObject(m_HDC, m_hPenIPSelected);
		SelectObject(m_HDC, m_hPenSel);
		SelectObject(m_HDC, GetStockObject(HOLLOW_BRUSH));

		RoundRect(m_HDC,
			xref - 2 * m_wRx / 3, m_RxYref + 2 * m_hRx / 3,
			xref + 2 * m_wRx / 3, m_RxYref - 2 * m_hRx / 3,
			30, 30);
	}*/

}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDIRxNumber(char Rx, int xref, int yref)
{
	
	if (S3RxGetAlarms(Rx) && m_Parent->m_AnimateState)
	{
		S3BLT(m_hbmpTxTxBattHot, xref + 5, yref - 3, 8, 28);
	}	
	else
	{
		SelectObject(m_HDC, m_hPenNone);
		SelectObject(m_HDC, m_hBrushRxNum);

		RECT fntRc;
		fntRc.top = yref - 21 + 10;
		fntRc.bottom = yref + 10 + 10;

		fntRc.left = xref - 7 + 10;
		fntRc.right = xref + 6 + 10;

		SelectObject(m_HDC, m_hFontL);

		CString str;
		str.Format(_T("%d"), Rx + 1);
		DrawText(m_HDC, str, -1, &fntRc, DT_LEFT);
	}
}

// ----------------------------------------------------------------------------
// Draw 3 FOL circles

void CS3GDIScreenMain::S3DrawGDIFOLSym(char Rx, int xref, int yref, bool left)
{
	int				w = 28, h = 42; // TODO: Get from bmp
	unsigned char	RxType = S3RxGetType(Rx);

	if (RxType != S3_Rx2)
	{
		if (S3RxGetConnectedTxs(Rx) == 0)
		{
			if (left)
				S3BLT(m_hbmpFOLDark, xref - w + 1, yref, w, h);
			else
				S3BLT(m_hbmpFOLDark, xref - 1, yref, w, h);
		}
		else
		{
			if (left)
				S3BLT(m_hbmpFOL, xref - w + 1, yref, w, h);
			else
				S3BLT(m_hbmpFOL, xref - 1, yref, w, h);
		}
	}
	else // Rx2
	{
		// Tx0
		int	l = xref - m_RxRLLBar_w[0] / 2 - 1;
		if (S3TxFOLLive(Rx, 0))
			S3BLT(m_hbmpFOL, l - w + 1, yref, w, h);
		else
			S3BLT(m_hbmpFOLDark, l - w + 1, yref, w, h);

		// Tx1
		l = xref + m_RxRLLBar_w[0] / 2 + 1;
		if (S3TxFOLLive(Rx, 1))
			S3BLT(m_hbmpFOL, l - 1, yref, w, h);
		else
			S3BLT(m_hbmpFOLDark, l - 1, yref, w, h);
	}
}

// ----------------------------------------------------------------------------
