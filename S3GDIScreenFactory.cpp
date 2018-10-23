#include "stdafx.h"

#include "S3DataModel.h"

#ifdef S3_AGENT
#include "../S3AgentDlg.h"
#else
#include "S3ControllerDlg.h"
#endif

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3InitGDIFactoryScreen(void)
{
	m_RectFactoryScreen = m_RectScreen;
	m_RectFactoryScreen.top = m_RectHeader.bottom;
	
	m_RectFactoryLock.left = m_RectFactoryScreen.left;
	m_RectFactoryLock.top = m_RectFactoryScreen.top;
	m_RectFactoryLock.right = m_RectFactoryScreen.left + 150;
	m_RectFactoryLock.bottom = m_RectFactoryLock.top + 50;

	m_RectFactoryShutdown = m_RectFactoryLock;
	m_RectFactoryShutdown.left = m_RectFactoryLock.right;
	m_RectFactoryShutdown.right = m_RectFactoryShutdown.left + 150;

	m_RectFactoryRestart = m_RectFactoryLock;
	m_RectFactoryRestart.left = m_RectFactoryShutdown.right;
	m_RectFactoryRestart.right = m_RectFactoryRestart.left + 150;

	m_RectFactoryClose = m_RectFactoryLock;
	m_RectFactoryClose.left = m_RectFactoryRestart.right;
	m_RectFactoryClose.right = m_RectFactoryClose.left + 150;

	// Distribute horizontally
	char NButtons = 4;

	m_RectFactoryLock.MoveToXY(0 * m_RectScreen.Width() / NButtons +
		(m_RectScreen.Width() / NButtons -	m_RectFactoryLock.Width()) / 2,
		m_RectFactoryLock.top + 50);

	m_RectFactoryShutdown.MoveToXY(1 * m_RectScreen.Width() / NButtons +
		(m_RectScreen.Width() / NButtons -	m_RectFactoryShutdown.Width()) / 2,
		m_RectFactoryShutdown.top + 50);

	m_RectFactoryRestart.MoveToXY(2 * m_RectScreen.Width() / NButtons +
		(m_RectScreen.Width() / NButtons -	m_RectFactoryRestart.Width()) / 2,
		m_RectFactoryRestart.top + 50);

	m_RectFactoryClose.MoveToXY(3 * m_RectScreen.Width() / NButtons +
		(m_RectScreen.Width() / NButtons -	m_RectFactoryClose.Width()) / 2,
		m_RectFactoryClose.top + 50);

	m_RectCalibrate = m_RectFactoryLock;
	m_RectCalibrate.top = m_RectCalibrate.top + 50 + 50;
	m_RectCalibrate.bottom = m_RectCalibrate.bottom + 50 + 50;

	m_RectSystem = m_RectCalibrate;
	m_RectSystem.MoveToXY(1 * m_RectScreen.Width() / NButtons +
		(m_RectScreen.Width() / NButtons -	m_RectSystem.Width()) / 2,
		m_RectSystem.top);

	m_RectDemo = m_RectSystem;
	m_RectDemo.MoveToXY(2 * m_RectScreen.Width() / NButtons +
		(m_RectScreen.Width() / NButtons -	m_RectDemo.Width()) / 2,
		m_RectDemo.top);

	m_RectFactoryMsg = m_RectScreen;
	m_RectFactoryMsg.left += LHMARGIN;
	m_RectFactoryMsg.top = m_RectDemo.bottom + 50;
	m_RectFactoryMsg.bottom = m_RectFactoryMsg.top + 50;

	m_StrFactoryMsg = _T("");
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDIFactoryScreen(void)
{
	// CShutdownDlg dlg;
	// dlg.DoModal();
	S3DrawGDIBackButton();

	SelectObject(m_HDC, m_hPenNone);
	SelectObject(m_HDC, m_hBrushBG3);

	// S3_RECT(m_HDC, m_RectShutdownScreen);
	// S3BLTR(m_hbmpPPMBG, m_RectShutdownScreen);

	//RECT fntRc;
	//fntRc.left = xref + m_lChBatt + 10;
	//fntRc.top = yref + 2;
	//fntRc.right = fntRc.left + 110;
	//fntRc.bottom = yref + m_wChBatt;

	SetTextColor(m_HDC, m_crWhite);
	SelectObject(m_HDC, m_hBrushSleep);
	SelectObject(m_HDC, m_hFontL);
	
	S3BLTR(m_hbmpBlueButton, m_RectFactoryLock);
	DrawText(m_HDC, _T("Lock"), -1, &m_RectFactoryLock, S3_BTN_CENTRE);

	if (S3GetSoftShutdownOption())
	{
		S3BLTR(m_hbmpRedButton, m_RectFactoryShutdown);
		S3BLTR(m_hbmpRedButton, m_RectFactoryRestart);
	}
	else
	{
		S3BLTR(m_hbmpGreyButton, m_RectFactoryShutdown);
		S3BLTR(m_hbmpGreyButton, m_RectFactoryRestart);
	}

	DrawText(m_HDC, _T("Shutdown"), -1, &m_RectFactoryShutdown, S3_BTN_CENTRE);
	DrawText(m_HDC, _T("Restart"), -1, &m_RectFactoryRestart, S3_BTN_CENTRE);

	S3BLTR(m_hbmpBlueButton, m_RectFactoryClose);
	DrawText(m_HDC, _T("Close"), -1, &m_RectFactoryClose, S3_BTN_CENTRE);

	S3BLTR(m_hbmpBlueButton, m_RectDemo);
	if (S3GetDemoMode())
		DrawText(m_HDC, _T("Live"), -1, &m_RectDemo,	S3_BTN_CENTRE);
	else
		DrawText(m_HDC, _T("Demo"), -1, &m_RectDemo,	S3_BTN_CENTRE);

	S3BLTR(m_hbmpBlueButton, m_RectSystem);
	DrawText(m_HDC, _T("System"), -1, &m_RectSystem,	S3_BTN_CENTRE);

	S3BLTR(m_hbmpBlueButton, m_RectCalibrate);
	DrawText(m_HDC, _T("Calibrate"), -1, &m_RectCalibrate,	S3_BTN_CENTRE);

	SetTextColor(m_HDC, m_crTextNorm);
	DrawText(m_HDC, m_StrFactoryMsg, -1, &m_RectFactoryMsg,	DT_LEFT);
}

// ----------------------------------------------------------------------------
// TODO: Garbage?

extern int S3I2CTest();

int CS3GDIScreenMain::S3FindFactoryScreen(POINT p)
{
	if (m_RectBackButton.PtInRect(p))
	{
		S3GDIChangeScreen(S3_OVERVIEW_SCREEN);
		return 1;
	}

	if (S3GetRemote())
		return 0;

	if (S3GetSoftShutdownOption())
	{
		if (m_RectFactoryShutdown.PtInRect(p))
		{
			S3EventLogAdd("System shutdown requested by user", 1, -1, -1, -1);
			S3SetPowerDownPending(true);
			S3SetSleepAll();
			// m_Parent->SysShutdown();

			return 1;
		}
		else if (m_RectFactoryRestart.PtInRect(p))
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
	
	if (m_RectFactoryClose.PtInRect(p))
	{
		S3EventLogAdd("Application close requested by user", 1, -1, -1, -1);
		S3SetCloseAppPending(true);
		S3SetSleepAll();
		
		return 1;
	}
	else if (m_RectCalibrate.PtInRect(p))
	{
		S3GDIChangeScreen(S3_CALIBRATE_SCREEN);
		m_Parent->ShowFactory(S3_CALIBRATE_SCREEN);
		return 1;
	}
	else if (m_RectSystem.PtInRect(p))
	{
		S3GDIChangeScreen(S3_FACTORY_SYS_SCREEN);
		m_Parent->ShowFactory(S3_FACTORY_SYS_SCREEN);
		return 1;
	}
	else if (m_RectFactoryLock.PtInRect(p))
	{
		if (!S3SetLocked(true) && !S3SetLockFile())
			m_StrFactoryMsg = _T("System locked");
		else
			m_StrFactoryMsg = _T("System lock failed");

		return 1;
	}
	else if (m_RectDemo.PtInRect(p))
	{
		S3SetDemoMode(!S3GetDemoMode());
		S3GDIChangeScreen(S3_OVERVIEW_SCREEN);
		return 1;
	}
	
	return 0;
}

// ----------------------------------------------------------------------------

int CS3GDIScreenMain::S3LeaveFactoryScreen()
{
	m_StrFactoryMsg = _T("");

	return 0;
}

// ----------------------------------------------------------------------------