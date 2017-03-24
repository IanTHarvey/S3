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

void CS3GDIScreenMain::S3InitGDIShutdownScreen(void)
{
	m_RectShutdownScreen = m_RectScreen;
	m_RectShutdownScreen.top = m_RectHeader.bottom;
	
	m_RectTxSleepAll.left = m_RectShutdownScreen.left;
	m_RectTxSleepAll.top = m_RectShutdownScreen.top;
	m_RectTxSleepAll.right = m_RectShutdownScreen.left + 150;
	m_RectTxSleepAll.bottom = m_RectTxSleepAll.top + 50;

	m_RectTxWakeAll = m_RectTxSleepAll;
	m_RectTxWakeAll.left = m_RectTxSleepAll.right;
	m_RectTxWakeAll.right = m_RectTxWakeAll.left + 150;

	m_RectSysShutdown = m_RectTxSleepAll;
	m_RectSysShutdown.left = m_RectTxWakeAll.right;
	m_RectSysShutdown.right = m_RectSysShutdown.left + 150;

	m_RectSysRestart = m_RectTxSleepAll;
	m_RectSysRestart.left = m_RectSysShutdown.right;
	m_RectSysRestart.right = m_RectSysRestart.left + 150;

	// Distribute horizontally
	char NButtons = 4;
	m_RectTxSleepAll.MoveToXY(	0 +
		(m_RectScreen.Width() / NButtons - m_RectTxSleepAll.Width()) / 2,
		m_RectTxSleepAll.top + 50);

	m_RectTxWakeAll.MoveToXY(	1 * m_RectScreen.Width() / NButtons +
		(m_RectScreen.Width() / NButtons -	m_RectTxWakeAll.Width()) / 2,
		m_RectTxWakeAll.top + 50);

	m_RectSysShutdown.MoveToXY(	2 * m_RectScreen.Width() / NButtons +
		(m_RectScreen.Width() / NButtons -	m_RectSysShutdown.Width()) / 2,
		m_RectSysShutdown.top + 50);

	m_RectSysRestart.MoveToXY(	3 * m_RectScreen.Width() / NButtons +
		(m_RectScreen.Width() / NButtons -	m_RectSysRestart.Width()) / 2,
		m_RectSysRestart.top + 50);

}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDIShutdownScreen(void)
{

	S3DrawGDIBackButton();

	SelectObject(m_HDC, m_hPenNone);
	SelectObject(m_HDC, m_hBrushBG3);

	S3_RECT(m_HDC, m_RectShutdownScreen);
	// S3BLTR(m_hbmpPPMBG, m_RectShutdownScreen);

	SetTextColor(m_HDC, m_crWhite);
	SelectObject(m_HDC, m_hBrushSleep);
	SelectObject(m_HDC, m_hFontL);
	
	bool AllAsleep = S3AllAsleep();

	if (!AllAsleep)
	{
		if (!S3GetSleepAll())
		{
			S3BLTR(m_hbmpBlueButton, m_RectTxSleepAll);
			DrawText(m_HDC, _T("Sleep All"), -1, &m_RectTxSleepAll, S3_BTN_CENTRE);
		}
		else
		{
			S3BLTR(m_hbmpGreyButton, m_RectTxSleepAll);
			DrawText(m_HDC, _T("Sleeping"), -1, &m_RectTxSleepAll, S3_BTN_CENTRE);
		}
	}
	else
	{
		S3BLTR(m_hbmpGreyButton, m_RectTxSleepAll);
		DrawText(m_HDC, _T("All Asleep"), -1, &m_RectTxSleepAll, S3_BTN_CENTRE);
	}

	bool AllAwake = S3AllAwake();

	if (!AllAwake)
	{
		if (!S3GetWakeAll())
		{
			S3BLTR(m_hbmpBlueButton, m_RectTxWakeAll);
			DrawText(m_HDC, _T("Wake All"), -1, &m_RectTxWakeAll, S3_BTN_CENTRE);
		}
		else
		{
			S3BLTR(m_hbmpGreyButton, m_RectTxWakeAll);
			DrawText(m_HDC, _T("Waking"), -1, &m_RectTxWakeAll, S3_BTN_CENTRE);
		}
	}
	else
	{
		S3BLTR(m_hbmpGreyButton, m_RectTxWakeAll);
		DrawText(m_HDC, _T("All Awake"), -1, &m_RectTxWakeAll, S3_BTN_CENTRE);
	}

	CRect RectTxt = m_RectShutdownScreen;

	if (S3GetSoftShutdownOption())
	{
		S3BLTR(m_hbmpRedButton, m_RectSysShutdown);
		DrawText(m_HDC, _T("Shutdown"), -1, &m_RectSysShutdown, S3_BTN_CENTRE);

		S3BLTR(m_hbmpRedButton, m_RectSysRestart);
		DrawText(m_HDC, _T("Restart"), -1, &m_RectSysRestart, S3_BTN_CENTRE);

		SetTextColor(m_HDC, m_crTextNorm);
		
		RectTxt.top = m_RectTxSleepAll.bottom + 40;

		if (S3GetWakeAll())
			DrawText(m_HDC, _T("Waking up all transmitters\nPlease wait"),
					-1, &RectTxt, DT_CENTER);

		else if (!AllAsleep && S3GetSleepAll())
			DrawText(m_HDC, _T("Shutting down transmitters\nPlease wait"),
					-1, &RectTxt, DT_CENTER);
	}
	else
	{
		SetTextColor(m_HDC, m_crTextNorm);

		RectTxt.top = m_RectTxSleepAll.bottom + 40;
		if (AllAsleep)
		{
			DrawText(m_HDC, _T("Use front-panel switch to shutdown Sentinel 3"),
				-1, &RectTxt, DT_CENTER);
		}
		else
		{
			if (S3GetWakeAll())
				DrawText(m_HDC, _T("Waking up all transmitters\nPlease wait"),
					-1, &RectTxt, DT_CENTER);
			else if (!S3GetSleepAll())
				DrawText(m_HDC, _T("Use Sleep All button to shut down transmitters\nthen use front-panel switch to shutdown Sentinel 3"),
					-1, &RectTxt, DT_CENTER);
			else
				DrawText(m_HDC, _T("Shutting down all transmitters\nPlease wait"),
					-1, &RectTxt, DT_CENTER);
		}
	}
}

// ----------------------------------------------------------------------------
// TODO: Garbage?

extern int S3I2CTest();

int CS3GDIScreenMain::S3FindShutdownScreen(POINT p)
{
	if (m_RectBackButton.PtInRect(p))
	{
		S3GDIChangeScreen(S3_OVERVIEW_SCREEN);
		return 1;
	}

	if (S3GetRemote())
		return 0;

	if (!S3AllAsleep() && m_RectTxSleepAll.PtInRect(p))
	{
		S3EventLogAdd("Sleep-all requested by user", 1, -1, -1, -1);
		S3SetSleepAll(true);

		return 1;
	}
	else if (!S3AllAwake() && m_RectTxWakeAll.PtInRect(p))
	{
		S3EventLogAdd("Wake-all requested by user", 1, -1, -1, -1);
		S3SetWakeAll(true);

		return 1;
	}
	else if (S3GetSoftShutdownOption())
	{
		if (m_RectSysShutdown.PtInRect(p))
		{
			S3EventLogAdd("System shutdown requested by user", 1, -1, -1, -1);
			// S3SetSleepAll(true);
			// m_Parent->SysShutdown();
			S3SetPowerDownPending(true);
			S3SetSleepAll(true);

			return 1;
		}
		else if (m_RectSysRestart.PtInRect(p))
		{
			S3EventLogAdd("System restart requested by user", 1, -1, -1, -1);
			if (S3OSRestartRequest())
			{
				S3OSRestart();
				m_Parent->AppShutdown();
			}

			return 1;
		}
	}
	
	return 0;
}

// ----------------------------------------------------------------------------
