// Only navigable when system locked
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

class CS3Button
{
	CString	t;
	CRect	r;
	HDC		b;
	CS3GDIScreenMain	*p;

public:
	CS3Button(CS3GDIScreenMain *parent, HDC bitmap, CString txt, CRect rect);
	void	Draw(HDC *bitmap, const wchar_t *str);
	int		Find(POINT p);
	CRect	*Rect(void) { return &r; };
};

int CS3Button::Find(POINT p)
{
	if (r.PtInRect(p))
		return 1;

	return 0;
}

CS3Button::CS3Button(CS3GDIScreenMain *parent, HDC bitmap, CString txt, CRect rect)
{
	p = parent;
	b = bitmap;
	t = txt;
	r = rect;
}

void CS3Button::Draw(HDC *bitmap = NULL, const wchar_t *str = NULL)
{
	if (bitmap)
		TransparentBlt(
			p->GetDrawable(), r.left, r.top, r.Width(), r.Height(),
			*bitmap, 0, 0, r.Width(), r.Height(), p->m_crWhite);
	else
		TransparentBlt(
			p->GetDrawable(), r.left, r.top, r.Width(), r.Height(),
			b, 0, 0, r.Width(), r.Height(), p->m_crWhite);

	COLORREF cref = SetTextColor(p->GetDrawable(), p->m_crWhite);

	if (str)
		DrawText(p->GetDrawable(), str, -1, &r, S3_BTN_CENTRE);
	else
		DrawText(p->GetDrawable(), t, -1, &r, S3_BTN_CENTRE);

	SetTextColor(p->GetDrawable(), cref);
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3InitGDIShutdownScreen(void)
{
	char NButtons = 4;

	m_RectShutdownScreen = m_RectScreen;
	m_RectShutdownScreen.top = m_RectHeader.bottom;
	
	CRect Rect;
	
	Rect.left =	(m_RectScreen.Width() / NButtons - 150) / 2;
	Rect.top = m_RectShutdownScreen.top + 50;
	Rect.right = Rect.left + 150;
	Rect.bottom = Rect.top + 50;
	m_ButtonTxSleepAll = new CS3Button(this, m_hbmpBlueButton, _T("Sleep All"), Rect);

	Rect.left = 1 * m_RectScreen.Width() / NButtons + (m_RectScreen.Width() / NButtons - 150) / 2;
	Rect.right = Rect.left + 150;
	m_ButtonTxWakeAll = new CS3Button(this, m_hbmpBlueButton, _T("Wake All"), Rect);

	Rect.left = 2 * m_RectScreen.Width() / NButtons + (m_RectScreen.Width() / NButtons - 150) / 2;
	Rect.right = Rect.left + 150;
	m_ButtonSysShutdown = new CS3Button(this, m_hbmpRedButton, _T("Shutdown"), Rect);

	Rect.left = 3 * m_RectScreen.Width() / NButtons + (m_RectScreen.Width() / NButtons - 150) / 2;
	Rect.right = Rect.left + 150;
	m_ButtonSysRestart = new CS3Button(this, m_hbmpRedButton, _T("Restart"), Rect);

	CRect rect(m_RectScreen.left, m_RectScreen.bottom - 30 - 20,
		m_RectScreen.left + 60, m_RectScreen.bottom - 20);
	
	m_GDIMaintKeyEdit = new CS3Edit(this);
	m_GDIMaintKeyEdit->Create(WS_CHILD | ES_LEFT, rect, this, S3GDI_MAINT_KEY_EDIT);
	m_GDIMaintKeyEdit->SetFont(&m_cFontS);
	m_GDIMaintKeyEdit->ShowWindow(false);
	m_GDIMaintKeyEdit->SetWindowText(_T(""));
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3CloseGDIShutdownScreen(void)
{
	delete m_ButtonTxSleepAll;
	delete m_ButtonTxWakeAll;
	delete m_ButtonSysShutdown;
	delete m_ButtonSysRestart;
	delete m_GDIMaintKeyEdit;
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDIShutdownScreen(void)
{
	S3DrawGDIBackButton();

	SelectObject(m_HDC, m_hPenNone);
	SelectObject(m_HDC, m_hBrushBG3);

	S3_RECT(m_HDC, m_RectShutdownScreen);

	SetTextColor(m_HDC, m_crWhite);
	SelectObject(m_HDC, m_hBrushSleep);
	SelectObject(m_HDC, m_hFontL);
	
	bool AllAsleep = S3AllAsleep();
	bool AllAwake = S3AllAwake();

	if (!(AllAsleep && AllAwake))
	{
		if (!AllAsleep)
		{
			if (!S3GetSleepAll())
				m_ButtonTxSleepAll->Draw();
			else
				m_ButtonTxSleepAll->Draw(&m_hbmpGreyButton, _T("Sleeping"));
		}
		else
			m_ButtonTxSleepAll->Draw(&m_hbmpGreyButton, _T("All Asleep"));

		if (!AllAwake)
		{
			if (!S3GetWakeAll())
				m_ButtonTxWakeAll->Draw();
			else
				m_ButtonTxWakeAll->Draw(&m_hbmpGreyButton, _T("Waking"));
		}
		else
			m_ButtonTxWakeAll->Draw(&m_hbmpGreyButton, _T("All Awake"));
	}

	CRect RectTxt = m_RectShutdownScreen;

	if (S3GetSoftShutdownOption())
	{
		m_ButtonSysShutdown->Draw();
		m_ButtonSysRestart->Draw();

		SetTextColor(m_HDC, m_crTextNorm);
		
		RectTxt.top = m_ButtonTxSleepAll->Rect()->bottom + 40;

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

		RectTxt.top = m_ButtonTxSleepAll->Rect()->bottom + 40;
		if (AllAsleep)
		{
			DrawText(m_HDC,
				_T("Use front-panel switch to shutdown Sentinel 3"),
				-1, &RectTxt, DT_CENTER);
		}
		else
		{
			if (S3GetWakeAll())
				DrawText(m_HDC,
					_T("Waking up all transmitters\nPlease wait"),
					-1, &RectTxt, DT_CENTER);
			else if (!S3GetSleepAll())
				DrawText(m_HDC,
					_T("Use Sleep All button to shut down transmitters\n")
					_T("then use front-panel switch to shutdown Sentinel 3"),
					-1, &RectTxt, DT_CENTER);
			else
				DrawText(m_HDC, _T("Shutting down all transmitters\nPlease wait"),
					-1, &RectTxt, DT_CENTER);
		}
	}

	m_GDIMaintKeyEdit->ShowWindow(true);
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

	if (!S3AllAsleep() && m_ButtonTxSleepAll->Find(p))
	{
		S3EventLogAdd("Sleep-all requested by user", 1, -1, -1, -1);
		S3SetSleepAll(true);

		return 1;
	}
	else if (!S3AllAwake() && m_ButtonTxWakeAll->Find(p))
	{
		S3EventLogAdd("Wake-all requested by user", 1, -1, -1, -1);
		S3SetWakeAll(true);

		return 1;
	}
	else if (S3GetSoftShutdownOption())
	{
		if (m_ButtonSysShutdown->Find(p))
		{
			S3EventLogAdd("System shutdown requested by user", 1, -1, -1, -1);

			S3SetPowerDownPending(true);
			S3SetSleepAll(true);

			return 1;
		}
		else if (m_ButtonSysRestart->Find(p))
		{
			S3EventLogAdd("System restart requested by user", 1, -1, -1, -1);
			if (S3OSRestartRequest())
			{
				S3OSRestart();
#ifndef S3_AGENT
				m_Parent->AppShutdown();
#endif
			}

			return 1;
		}
	}
	
	return 0;
}

// ----------------------------------------------------------------------------

int CS3GDIScreenMain::S3LeaveShutdownScreen()
{
	m_GDIMaintKeyEdit->ShowWindow(false);

	CString str;
	int len;

	len = m_GDIMaintKeyEdit->GetWindowTextLength();

	if (len != 6)
		return 0;

	m_GDIMaintKeyEdit->GetWindowText(str);

	const char *DateTime = S3SysGetAppDateTime();

	unsigned char DTCnt = 0;
	bool lock = false;

	for(unsigned char c = 0; c < 6; c++)
	{
		if (DateTime[DTCnt] >= '0' && DateTime[DTCnt] <= '9')
		{
			if (str[c] != DateTime[DTCnt++])
			{
				lock = true;
				break;
			}
		}
	}

	S3SetLocked(lock);
	S3SetLockFile();

	m_GDIMaintKeyEdit->SetWindowText(_T(""));

	S3SetSIPRegKey(1); // Disable SIP pop-up

	return 0;
}

// ----------------------------------------------------------------------------