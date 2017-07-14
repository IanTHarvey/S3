// Draws the Overview screen RLL bar-graph display

#include "stdafx.h"

#include "S3DataModel.h"

#ifdef S3_AGENT
#include "S3Agent/S3AgentDlg.h"
#else
#include "S3ControllerDlg.h"
#endif

const int CS3GDIScreenMain::m_RxRLLBar_h[S3_MAIN_RX_RLL_BARS] = { 6, 7, 8, 9, 10 };
const int CS3GDIScreenMain::m_RxRLLBar_w[S3_MAIN_RX_RLL_BARS] = { 7, 9, 14, 22, 32 };

// Draw the RLL display using 5 rounded bars of increasing width and depth.
// Rx2 display is split down the middle, Rx6 only shows the active channel.

void CS3GDIScreenMain::S3DrawGDIRxRLL2(char Rx, int xref, int yref)
{
	int		TopDiv = 5;
	int		b = yref + (TopDiv - 1) * m_hRx / (2 * TopDiv);	// Bottom of bottom bar

	int		l, r;
	short	RLL, RLL_10mdBm;
	unsigned char RxType, TxType;

	b -= 2; // Fiddle factor

	RxType = S3RxGetType(Rx);

	SelectObject(m_HDC, m_hPenIPOff);

	HDC bmp;

	if (RxType == S3_Rx1 || RxType == S3_Rx6)
	{
		int		bb = b;
		char	Tx = 0;
		
		if (RxType == S3_Rx6)
		{
			// Change pending?
			Tx = S3RxGetActiveTx(Rx);
			if (Tx >= S3_PENDING)
				Tx = Tx - S3_PENDING;
				// return;
		}

		TxType = S3TxGetType(Rx, Tx);
		RLL_10mdBm = S3RxGetRLL(Rx, Tx);
		RLL = (short)(100.0 * ((double)RLL_10mdBm - S3RxGetRLLLo(Rx)) /
									(S3RxGetRLLHi(Rx) - S3RxGetRLLLo(Rx)));

		bool FolLive = S3TxFOLLive(Rx, Tx);

		SelectObject(m_HDC, m_hPenNone);

		for (unsigned char i = 0; i < S3_MAIN_RX_RLL_BARS; i++)
		{
			if (TxType != S3_TxUnconnected)
			{
				bmp = 0;

				if (FolLive && i == 0 && RLL < 0)
					bmp = m_hbmpRx1RLL1Red;
				else if (FolLive && (i == S3_MAIN_RX_RLL_BARS - 1 && RLL > 100))
					bmp = m_hbmpRx1RLL5Red;
				else if (FolLive && RLL >= i * 100 / S3_MAIN_RX_RLL_BARS)
					bmp = m_hbmpRx1RLLOn[i];
				else
				{
					SelectObject(m_HDC, m_hBrushRxRLLOff);

					RoundRect(m_HDC,
						xref - m_RxRLLBar_w[i], bb - m_RxRLLBar_h[i],
						xref + m_RxRLLBar_w[i] + 1, bb,
						i + 2, i + 2);
				}

				if (bmp)
					S3BLT_BLK(bmp,	xref - m_RxRLLBar_w[i],	bb - m_RxRLLBar_h[i],
									2 * m_RxRLLBar_w[i],	m_RxRLLBar_h[i]);
			}
			
			bb -= m_RxRLLBar_h[i] + 1;
		}

		// Draw FOL entry to Rx
		if (S3RxGetConnectedTxs(Rx) != 0)
		{
			if (RxType == S3_Rx1)
				SelectObject(m_HDC, m_hPenFOL1);
			else
				SelectObject(m_HDC, m_hPenFOL6);

			MoveToEx(m_HDC, xref, m_RectRx.bottom, NULL);
			LineTo(m_HDC, xref, b - 1);
			SelectObject(m_HDC, m_hPenIPOff);
		}
		else
		{
			if (RxType == S3_Rx1)
				SelectObject(m_HDC, m_hPenFOL1Dark);
			else
				SelectObject(m_HDC, m_hPenFOL6Dark);
			
			MoveToEx(m_HDC, xref, m_RectRx.bottom, NULL);
			LineTo(m_HDC, xref, b - 1);
			SelectObject(m_HDC, m_hPenIPOff);
		}
	} // Rx1 & Rx6
	else if (RxType == S3_Rx2)
	{
		for (unsigned char j = 0; j < 2; j++)
		{
			int	bb = b;

			TxType = S3TxGetType(Rx, j);
			RLL_10mdBm = S3RxGetRLL(Rx, j);
			RLL = (short)(100.0 * ((double)RLL_10mdBm - S3RxGetRLLLo(Rx)) /
									(S3RxGetRLLHi(Rx) - S3RxGetRLLLo(Rx)));

			bool FolLive = S3TxFOLLive(Rx, j);

			for (unsigned char i = 0; i < S3_MAIN_RX_RLL_BARS; i++)
			{
				SelectObject(m_HDC, m_hPenNone);

				// TODO: Assume RLL in %age
				if (FolLive && (i == 0 && RLL < 0) || (RLL > i * 100 / S3_MAIN_RX_RLL_BARS) &&
					(TxType != S3_TxUnconnected))
				{
					if (!j)
					{
						l = xref - m_RxRLLBar_w[i] - 1;

						if (i == 0 && RLL < 0)
							bmp = m_hbmpRx2RLL1Red;
						else if (i == S3_MAIN_RX_RLL_BARS - 1 && RLL > 100)
							bmp = m_hbmpRx2RLL5Red;
						else if (RLL >= i * 100 / S3_MAIN_RX_RLL_BARS)
							bmp = m_hbmpRx2RLLOn[i];
						
						S3BLT_BLK(bmp, l, bb - m_RxRLLBar_h[i],
							m_RxRLLBar_w[i], m_RxRLLBar_h[i]);
					}
					else
					{
						l = xref + 1;

						if (i == 0 && RLL < 0)
							bmp = m_hbmpRx2RLL1Red;
						else if (i == S3_MAIN_RX_RLL_BARS - 1 && RLL > 100)
							bmp = m_hbmpRx2RLL5Red;
						else
							bmp = m_hbmpRx2RLLOn[i];

						S3BLT_BLK(bmp, l, bb - m_RxRLLBar_h[i],
							m_RxRLLBar_w[i], m_RxRLLBar_h[i]);
					}
				}
				else
				{
					SelectObject(m_HDC, m_hBrushRxRLLOff);

					if (!j)
					{
						l = xref - m_RxRLLBar_w[i];
						r = xref;
						RoundRect(m_HDC,
							l, bb - m_RxRLLBar_h[i],
							r, bb,
							i + 2, i + 2);
					}
					else
					{
						l = xref + 1;
						r = xref + m_RxRLLBar_w[i] + 1;
						RoundRect(m_HDC,
							l, bb - m_RxRLLBar_h[i],
							r, bb,
							i + 2, i + 2);
					}
				}

				bb -= m_RxRLLBar_h[i] + 1;
			}
			
			// Draw FOLs into bottom RLL segments
			if (S3TxFOLLive(Rx, j))
				SelectObject(m_HDC, m_hPenFOL1);
			else
				SelectObject(m_HDC, m_hPenFOL1Dark);			
			
			if (!j)
			{
				int	l = xref - m_RxRLLBar_w[0] / 2 - 1;

				MoveToEx(m_HDC, l, m_RectRx.bottom, NULL);
				LineTo(m_HDC, l, b - 1);
			}
			else
			{
				int	l = xref + m_RxRLLBar_w[0] / 2 + 1;

				MoveToEx(m_HDC, l, m_RectRx.bottom, NULL);
				LineTo(m_HDC, l, b - 1);
			}

			SelectObject(m_HDC, m_hPenIPOff);
		}
	} // Rx2
}

// ----------------------------------------------------------------------------
// xref yref of parent Rx

void CS3GDIScreenMain::S3DrawGDIRxGain(char Rx, char Tx, int xref, int yref)
{
	CRect	fntRc;
	CString str;

	SelectObject(m_HDC, m_hPenFOL2);
	SelectObject(m_HDC, m_hBrushLightGrey); //  GetStockObject(HOLLOW_BRUSH));

	COLORREF cr_txt = SetTextColor(m_HDC, m_crWhite);

	// Must be an Rx2, draw right lobe
	if (Tx == 1)
	{
		fntRc.left =	xref + m_xosGainVal - m_radGainVal;
		fntRc.top =		yref + m_yosGainVal - m_radGainVal;
		fntRc.right =	xref + m_xosGainVal + m_radGainVal;
		fntRc.bottom =	yref + m_yosGainVal + m_radGainVal;

		// char LG = S3RxGetLinkGain(Rx, Tx);
		char LG = S3IPGetGain(Rx, Tx, S3TxGetActiveIP(Rx, Tx));

		if (LG != SCHAR_MIN)
			str.Format(_T("%+d"), LG);
		else
			str.Format(_T(" --- "), LG);

		SelectObject(m_HDC, m_hFontS);
		SetTextColor(m_HDC, m_crBlack);
		DrawText(m_HDC, str, -1, &fntRc,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		fntRc.left =	xref + m_xosGainUnits - m_radGainUnits;
		fntRc.top =		yref + m_yosGainUnits - m_radGainUnits + 1;
		fntRc.right =	xref + m_xosGainUnits + m_radGainUnits;
		fntRc.bottom =	yref + m_yosGainUnits + m_radGainUnits + 1;

		str.Format(_T("dB"));

		SelectObject(m_HDC, m_hFontVS);
		// SetTextColor(m_HDC, m_crWhite);

		DrawText(m_HDC, str, -1, &fntRc,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}
	else if (Tx < 1)
	{
		// Must be Rx1 input 1, or Rx6 input Tx draw left lobe
		Tx = -Tx;

		CRect fntRc;

		fntRc.left =	xref - m_xosGainVal - m_radGainVal;
		fntRc.top =		yref + m_yosGainVal - m_radGainVal;
		fntRc.right =	xref - m_xosGainVal + m_radGainVal;
		fntRc.bottom =	yref + m_yosGainVal + m_radGainVal;

		// char LG = S3RxGetLinkGain(Rx, Tx);
		char LG = S3IPGetGain(Rx, Tx, S3TxGetActiveIP(Rx, Tx));

		if (LG != SCHAR_MIN)
			str.Format(_T("%+d"), LG);
		else
			str.Format(_T(" --- "), LG);

		SelectObject(m_HDC, m_hFontS);
		SetTextColor(m_HDC, m_crBlack);

		DrawText(m_HDC, str, -1, &fntRc,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		fntRc.left =	xref - m_xosGainUnits - m_radGainUnits;
		fntRc.top =		yref + m_yosGainUnits - m_radGainUnits + 1;
		fntRc.right =	xref - m_xosGainUnits + m_radGainUnits;
		fntRc.bottom =	yref + m_yosGainUnits + m_radGainUnits + 1;

		str.Format(_T("dB"));

		SelectObject(m_HDC, m_hFontVS);

		DrawText(m_HDC, str, -1, &fntRc,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}

	cr_txt = SetTextColor(m_HDC, cr_txt);
}

// ----------------------------------------------------------------------------