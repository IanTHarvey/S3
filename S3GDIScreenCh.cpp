// ----------------------------------------------------------------------------
// Charger status displays
//
// m_Screen == S3_CH_SCREEN
// ----------------------------------------------------------------------------

#include "stdafx.h"

#include "S3DataModel.h"

#ifdef S3_AGENT
#include "S3Agent/S3AgentDlg.h"
#else
#include "S3ControllerDlg.h"
#endif

extern pS3DataModel S3Data;

// Control status bits
#define BQ_RES7		0x80
#define BQ_FAS		0x40
#define BQ_SS		0x20	// Sealed
#define BQ_CALEN	0x10
#define BQ_CCA		0x08
#define BQ_BCA		0x04
#define BQ_CSV		0x02
#define BQ_RES0		0x01

// ----------------------------------------------------------------------------

// TODO: Put in class
CS3NameValue	*m_ChTimeRemain[S3_N_CHARGERS];
CS3NameValue	*m_ChBattT[S3_N_CHARGERS];
CS3NameValue	*m_ChBattV[S3_N_CHARGERS];
CS3NameValue	*m_ChBattI[S3_N_CHARGERS];
CS3NameValue	*m_ChBattType[S3_N_CHARGERS];
CS3NameValue	*m_ChSN[S3_N_CHARGERS];
CS3NameValue	*m_ChPN[S3_N_CHARGERS];
CS3NameValue	*m_ChHW[S3_N_CHARGERS];
CS3NameValue	*m_ChFW[S3_N_CHARGERS];

CRect			m_RectCh[S3_N_CHARGERS];

void CS3GDIScreenMain::S3InitGDIChScreen(void)
{
	int		w = m_RectScreen.Width() / S3_N_CHARGERS;

	for (char Ch = 0; Ch < S3_N_CHARGERS; Ch++)
	{
		char row = 2;

		m_RectCh[Ch].left =		m_RectScreen.left + Ch * w,
		m_RectCh[Ch].top =		m_RectHeader.bottom;
		m_RectCh[Ch].right =	m_RectScreen.left + (Ch + 1) * w,
		m_RectCh[Ch].bottom =	m_RectScreen.bottom;

		CString str;

		m_ChTimeRemain[Ch] = new CS3NameValue(	m_RectCh[Ch].left, 
						m_RectCh[Ch].top + HEAD_ROW + row++ * PARA_ROW, m_RectCh[Ch].Width(),
						_T("Time remaining"), _T("00h:00m"), false);
		m_ChTimeRemain[Ch]->RectEdit(m_HDC, m_hFontS);

		row++; // Separator

		str.Format(_T("\u03F4 (%+d - %+d%cC)"),
					S3_BATT_CHARGE_MIN_T / 10, S3_BATT_CHARGE_MAX_T / 10, 0x00b0);

		m_ChBattT[Ch] = new CS3NameValue(	m_RectCh[Ch].left, 
						m_RectCh[Ch].top + HEAD_ROW + row++ * PARA_ROW, m_RectCh[Ch].Width(),
						str, _T("+100"), false);
		m_ChBattT[Ch]->RectEdit(m_HDC, m_hFontS);

		m_ChBattV[Ch] = new CS3NameValue(	m_RectCh[Ch].left, 
						m_RectCh[Ch].top + HEAD_ROW + row++ * PARA_ROW, m_RectCh[Ch].Width(),
						_T("Voltage (V)"), _T("12.0"), false);
		m_ChBattV[Ch]->RectEdit(m_HDC, m_hFontS);

		m_ChBattI[Ch] = new CS3NameValue(	m_RectCh[Ch].left, 
						m_RectCh[Ch].top + HEAD_ROW + row++ * PARA_ROW, m_RectCh[Ch].Width(),
						_T("Current (mA)"), _T("2000"), false);
		m_ChBattI[Ch]->RectEdit(m_HDC, m_hFontS);

		row++; // Separator

		m_ChBattType[Ch] = new CS3NameValue(	m_RectCh[Ch].left, 
				m_RectCh[Ch].top + HEAD_ROW + row++ * PARA_ROW, m_RectCh[Ch].Width(),
				_T("Type"), _T("Unknown"), false);
		m_ChBattType[Ch]->RectEdit(m_HDC, m_hFontS);

		m_ChSN[Ch] = new CS3NameValue(	m_RectCh[Ch].left, 
				m_RectCh[Ch].top + HEAD_ROW + row++ * PARA_ROW, m_RectCh[Ch].Width(),
				_T("S/N"), _T("SN1234567890"), false);
		m_ChSN[Ch]->RectEdit(m_HDC, m_hFontS);

		m_ChPN[Ch] = new CS3NameValue(	m_RectCh[Ch].left, 
				m_RectCh[Ch].top + HEAD_ROW + row++ * PARA_ROW, m_RectCh[Ch].Width(),
				_T("P/N"), _T("PN-012345678901"), false);
		m_ChPN[Ch]->RectEdit(m_HDC, m_hFontS);

		m_ChHW[Ch] = new CS3NameValue(	m_RectCh[Ch].left, 
				m_RectCh[Ch].top + HEAD_ROW + row++ * PARA_ROW, m_RectCh[Ch].Width(),
				_T("H/W"), _T("Unknown"), false);
		m_ChHW[Ch]->RectEdit(m_HDC, m_hFontS);

		m_ChFW[Ch] = new CS3NameValue(	m_RectCh[Ch].left, 
				m_RectCh[Ch].top + HEAD_ROW + row++ * PARA_ROW, m_RectCh[Ch].Width(),
				_T("F/W"), _T("Unknown"), false);
		m_ChFW[Ch]->RectEdit(m_HDC, m_hFontS);
	}
}

// ----------------------------------------------------------------------------
void CS3GDIScreenMain::S3CloseGDIChScreen(void)
{
	for (char Ch = 0; Ch < S3_N_CHARGERS; Ch++)
	{
		delete m_ChTimeRemain[Ch];
		delete m_ChBattT[Ch];
		delete m_ChBattV[Ch];
		delete m_ChBattI[Ch];
		delete m_ChBattType[Ch];
		delete m_ChSN[Ch];
		delete m_ChPN[Ch];
		delete m_ChHW[Ch];
		delete m_ChFW[Ch];
	}
}

// ----------------------------------------------------------------------------
void CS3GDIScreenMain::S3DrawGDIChScreen(void)
{
	
	int		sep = (m_RectScreen.right - m_RectScreen.left) / S3_N_CHARGERS;
	RECT	fntRc;
	CString str;

	SetTextColor(m_HDC, m_crTextNorm);

	for (char Ch = 0; Ch < S3_N_CHARGERS; Ch++)
	{
		SelectObject(m_HDC, m_hPenNone);

		if (Ch % 2)
			SelectObject(m_HDC, m_hBrushBG1);
		else
			SelectObject(m_HDC, m_hBrushBG4);

		S3_RECT(m_HDC, m_RectCh[Ch]);

		S3DrawGDIBattCharge(Ch, m_RectCh[Ch].left + 50, m_RectCh[Ch].top + 15);

		// Charger number
		fntRc.left = m_RectCh[Ch].left + 10;
		fntRc.top = m_RectCh[Ch].top + 10;
		fntRc.right = m_RectCh[Ch].left + 30;
		fntRc.bottom = m_RectCh[Ch].top + 40;

		SelectObject(m_HDC, m_hFontL);
		str.Format(_T("%d"), Ch + 1);
		DrawText(m_HDC, str, -1, &fntRc, DT_LEFT);

		// Battery info
		fntRc.left = m_RectCh[Ch].left + 10;
		fntRc.top = m_RectCh[Ch].top + 50;
		fntRc.right = m_RectCh[Ch].right;
		fntRc.bottom = m_RectCh[Ch].top + 90;

		if (!S3ChOccupied(Ch))
		{
			str.Format(_T("No battery"));
		}
		else if (GetLockoutTime(Ch))
		{
			int tremain = (int)(GetLockoutTime(Ch) -
											S3Data->m_GUI->GetPosixTime());
			if (tremain < 0)
				tremain = 0;
			
			str.Format(_T("Locked: %d\n"), tremain);
		}
		else if (!S3ChBattValidated(Ch))
		{
			str.Format(_T("Not validated"));
		}
		else if (!(S3ChGetBattStatus(Ch) & BQ_FAS))
		{
			str.Format(_T("Full access"));
		}
		else if (!(S3ChGetBattStatus(Ch) & BQ_SS))
		{
			str.Format(_T("Not sealed"));
		}
		else if (S3ChGetAlarms(Ch) & S3_CH_BATT_COLD)
		{
			str.Format(_T("Under-Temp"));
		}
		else if (S3ChGetAlarms(Ch) & S3_CH_BATT_HOT)
		{
			str.Format(_T("Over-Temp"));
		}
		else if (S3ChGetAlarms(Ch) & S3_CH_CHARGE_FAULT)
		{
			str.Format(_T("Charging fault"));
		}
		else
		{
			char ChLevel = S3ChGetSoC(Ch);

			if (ChLevel < 0)
				str.Format(_T("Charging failed"));
			else if (!S3ChFullyCharged(Ch))
				str.Format(_T("Charging: %d%c"), ChLevel, '%');
			else
				str.Format(_T("Fully charged"));
		}

		DrawText(m_HDC, str, -1, &fntRc, DT_LEFT);

		SelectObject(m_HDC, m_hFontS);

		if (S3ChOccupied(Ch)) //  && S3ChBattValidated(Ch))
		{
			str.Format(_T("%S"), S3ChGetTimeToFullStr(Ch));
			m_ChTimeRemain[Ch]->SetValue(str);
			m_ChTimeRemain[Ch]->Draw(m_HDC, m_hFontS, m_hFontSB);
			
			if (S3ChGetBattTemp(Ch) == -2740)
				str = "-";
			else
				str.Format(_T("%+d"), S3ChGetBattTemp(Ch) / 10);
			
			m_ChBattT[Ch]->SetValue(str);
			m_ChBattT[Ch]->Draw(m_HDC, m_hFontS, m_hFontSB);

			str.Format(_T("%.1f"), S3ChGetBattV(Ch));
			m_ChBattV[Ch]->SetValue(str);
			m_ChBattV[Ch]->Draw(m_HDC, m_hFontS, m_hFontSB);

			str.Format(_T("%d"), S3ChGetBattI(Ch));
			m_ChBattI[Ch]->SetValue(str);
			m_ChBattI[Ch]->Draw(m_HDC, m_hFontS, m_hFontSB);

			// Battery info
			str.Format(_T("%s"), S3ChGetBattTypeStr(Ch));
			m_ChBattType[Ch]->SetValue(str);
			m_ChBattType[Ch]->Draw(m_HDC, m_hFontS, m_hFontSB);

			str.Format(_T("%S"), S3ChGetBattSN(Ch));
			m_ChSN[Ch]->SetValue(str);
			m_ChSN[Ch]->Draw(m_HDC, m_hFontS, m_hFontSB);

			str.Format(_T("%S"), S3ChGetBattPN(Ch));
			m_ChPN[Ch]->SetValue(str);
			m_ChPN[Ch]->Draw(m_HDC, m_hFontS, m_hFontSB);

			str.Format(_T("%S"), S3ChGetBattHW(Ch));
			m_ChHW[Ch]->SetValue(str);
			m_ChHW[Ch]->Draw(m_HDC, m_hFontS, m_hFontSB);

			str.Format(_T("%S"), S3ChGetBattFW(Ch));
			m_ChFW[Ch]->SetValue(str);
			m_ChFW[Ch]->Draw(m_HDC, m_hFontS, m_hFontSB);
		}
	}

	S3DrawGDIBackButton();
}

// ----------------------------------------------------------------------------

int CS3GDIScreenMain::S3FindChScreen(POINT p)
{
	if (m_RectBackButton.PtInRect(p))
	{
		m_Screen = S3_OVERVIEW_SCREEN;
		return 1;
	}

	return 0;
}


// -----------------------------------------------------------------------------
