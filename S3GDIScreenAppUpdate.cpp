// OS image update screens

#include "stdafx.h"

#include "S3DataModel.h"

#ifdef S3_AGENT
#include "S3Agent/S3AgentDlg.h"
#else
#include "S3ControllerDlg.h"
#endif

// bool m_AppUpdateScheduled = false;

// Layout 0
const wchar_t AppUpdateInstr[] = {
	_T("Only attempt update if you have a Sentinel 3 application update file provided by PPM and located in the root folder of a USB Mass Storage class device.")};

// Layout 1
const wchar_t AppUpdateNoImage[] = {
	_T("Sentinel 3 Application update failed")};

// Layout 1 Errors
const wchar_t AppUpdateNoImageHDD[] = {
	_T("No USB Mass Storage device found.")};

const wchar_t AppUpdateNoImageFile[] = {
	_T("No image file found on root of USB storage device.")};

const wchar_t AppUpdateProcessFail[] = {
	_T("An error occurred during the update process.")};

// Layout 2
wchar_t AppUpdateDoUpdate[256] = {
	_T("Sentinel 3 will shutdown, configuration data will be retained. Ensure that power is not interrupted when restarting.")};

// Layout 3
wchar_t AppUpdateUpdating[256] = {
	_T("Application update will be installed on restart.\nDo not switch off mains power until Sentinel 3 is shutdown.\n")};

unsigned char m_AppLayout;
unsigned char m_AppUpdateMsg;

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3InitGDIAppUpdateScreen(void)
{
	m_RectAppUpdateScreen = m_RectScreen;
	m_RectAppUpdateScreen.top = m_RectHeader.bottom;

	m_RectAppUpdateInstr = m_RectAppUpdateScreen;
	m_RectAppUpdateInstr.left += 40;
	m_RectAppUpdateInstr.top = m_RectAppUpdateScreen.top;
	m_RectAppUpdateInstr.right -= 40;
	m_RectAppUpdateInstr.bottom = m_RectAppUpdateInstr.top + 120;

	m_RectYes.left = m_RectAppUpdateScreen.left;
	m_RectYes.top = m_RectAppUpdateInstr.bottom;
	m_RectYes.right = m_RectYes.left + 150;
	m_RectYes.bottom = m_RectYes.top + 50;

	m_RectNo = m_RectYes;
	m_RectNo.left = m_RectYes.right;
	m_RectNo.right = m_RectNo.left + 150;

	// Distribute horizontally
	m_RectYes.MoveToXY(
		(m_RectScreen.Width() / 2 - m_RectYes.Width()) / 2,
		m_RectYes.top + 50);

	m_RectNo.MoveToXY(
		1 * m_RectScreen.Width() / 2 + (m_RectScreen.Width() / 2 - m_RectNo.Width()) / 2,
		m_RectNo.top + 50);

	if (!S3GetSoftShutdownOption())
	{
		// Layout 2
		_tcscpy_s(AppUpdateDoUpdate, 256,
			_T("After update Sentinel 3 must be shutdown, configuration data will be retained. Ensure that power is not interrupted when restarting."));

		// Layout 3
		_tcscpy_s(AppUpdateUpdating, 256,
			_T("Application update will be installed on restart.\nDo not switch off mains power until prompted.\n"));
	}
}

// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDIAppUpdateScreen()
{
	S3DrawGDIBackButton();
	
	SelectObject(m_HDC, m_hPenNone);
	SelectObject(m_HDC, m_hBrushBG3);
	
	S3_RECT(m_HDC, m_RectAppUpdateScreen);

	if (S3OSGetAppUpdateFail())
	{
		m_AppLayout = 1;
		m_AppUpdateMsg = 3;

		// Told the user, so let it go
		S3OSSetAppUpdateFail(false);
	}

	if (m_AppLayout == 0)
	{
		SetTextColor(m_HDC, m_crBlack);
		SelectObject(m_HDC, m_hBrushBG4);
		SelectObject(m_HDC, m_hFontL);
		DrawText(m_HDC, AppUpdateInstr, -1, &m_RectAppUpdateInstr, DT_WORDBREAK);

		SetTextColor(m_HDC, m_crWhite);
		SelectObject(m_HDC, m_hBrushSleep);
		// SelectObject(m_HDC, m_hFontL);
	
		S3BLT(m_hbmpBlueButton, m_RectYes.left, m_RectYes.top, 150, 50);
		
		DrawText(m_HDC, _T("Update"), -1, &m_RectYes,
			S3_BTN_CENTRE);

		S3BLT(m_hbmpBlueButton, m_RectNo.left, m_RectNo.top, 150, 50);
		
		DrawText(m_HDC, _T("Cancel"), -1, &m_RectNo,
			S3_BTN_CENTRE);
	}
	else if (m_AppLayout == 1)
	{
		SetTextColor(m_HDC, m_crBlack);
		SelectObject(m_HDC, m_hBrushBG4);
		SelectObject(m_HDC, m_hFontL);

		wchar_t tmp[S3GDI_MAX_SCREEN_MSG];

		// Give a reason
		if (m_AppUpdateMsg == 1)
			swprintf_s(tmp, S3GDI_MAX_SCREEN_MSG, _T("%s: %s"),
				AppUpdateNoImage, AppUpdateNoImageHDD);
		else  if (m_AppUpdateMsg == 2)
			swprintf_s(tmp, S3GDI_MAX_SCREEN_MSG, _T("%s: %s"),
				AppUpdateNoImage, AppUpdateNoImageFile);
		else  if (m_AppUpdateMsg == 3)
			swprintf_s(tmp, S3GDI_MAX_SCREEN_MSG, _T("%s: %s"),
				AppUpdateNoImage, AppUpdateProcessFail);
		else
			swprintf_s(tmp, S3GDI_MAX_SCREEN_MSG,
				_T("%s: Unknown error (%d)"), AppUpdateNoImage, m_AppUpdateMsg);
			
		DrawText(m_HDC, tmp, -1, &m_RectAppUpdateInstr, DT_WORDBREAK);

		SetTextColor(m_HDC, m_crWhite);
		SelectObject(m_HDC, m_hBrushSleep);
		// SelectObject(m_HDC, m_hFontL);
		
		S3BLT(m_hbmpBlueButton, m_RectYes.left, m_RectYes.top, 150, 50);
		DrawText(m_HDC, _T("Try again"), -1, &m_RectYes,
			S3_BTN_CENTRE);

		S3BLT(m_hbmpBlueButton, m_RectNo.left, m_RectNo.top, 150, 50);
		DrawText(m_HDC, _T("Cancel"), -1, &m_RectNo,
			S3_BTN_CENTRE);
	}
	else if (m_AppLayout == 2)
	{
		SetTextColor(m_HDC, m_crBlack);
		SelectObject(m_HDC, m_hBrushBG4);
		SelectObject(m_HDC, m_hFontL);

		DrawText(m_HDC, AppUpdateDoUpdate, -1, &m_RectAppUpdateInstr, DT_WORDBREAK);

		SetTextColor(m_HDC, m_crWhite);
		SelectObject(m_HDC, m_hBrushSleep);
		// SelectObject(m_HDC, m_hFontL);
		
		S3BLT(m_hbmpBlueButton, m_RectYes.left, m_RectYes.top, 150, 50);
		DrawText(m_HDC, _T("Update"), -1, &m_RectYes,
			S3_BTN_CENTRE);

		S3BLT(m_hbmpBlueButton, m_RectNo.left, m_RectNo.top, 150, 50);
		DrawText(m_HDC, _T("Cancel"), -1, &m_RectNo,
			S3_BTN_CENTRE);
	}
	else if (m_AppLayout == 3)
	{
		if (m_Parent->m_AppUpdateScheduled == false)
		{
			m_AppLayout = 1;
			m_AppUpdateMsg = 3;
		}

		SetTextColor(m_HDC, m_crRed);
		SelectObject(m_HDC, m_hBrushBG4);
		SelectObject(m_HDC, m_hFontL);

		DrawText(m_HDC, AppUpdateUpdating, -1, &m_RectAppUpdateInstr, DT_WORDBREAK);
	}
}

// ----------------------------------------------------------------------------

int CS3GDIScreenMain::S3FindAppUpdateScreen(POINT p)
{
	// m_AppUpdateMsg = 0;

	if (m_AppLayout == 3)
		return 0;

	if (m_RectBackButton.PtInRect(p))
	{
		m_AppLayout = 0;

		S3GDIChangeScreen(S3_OVERVIEW_SCREEN);
		
		return 1;
	}

	if (m_RectYes.PtInRect(p))
	{
		S3EventLogAdd("App update requested by user", 1, -1, -1, -1);

		if (m_AppLayout == 0)
		{
			if (m_AppUpdateMsg = S3OSAppUpdateRequest())
			{
				m_AppLayout = 1;
			}
			else
			{
				m_AppLayout = 2;
			}
		}
		else if (m_AppLayout == 1)
		{
			m_AppLayout = 0;
		}
		else if (m_AppLayout == 2)
		{
			m_AppLayout = 3;
			m_Parent->m_AppUpdateScheduled = true;
		}

		// Go to confirm screen
		return 1;
	}
	else if (m_RectNo.PtInRect(p))
	{
		S3EventLogAdd("App update cancelled by user", 1, -1, -1, -1); 
		if (m_AppLayout == 0)
		{
			S3GDIChangeScreen(S3_SETTINGS_SCREEN);
		}
		else if (m_AppLayout == 1)
		{
			m_AppLayout = 0;
			S3GDIChangeScreen(S3_SETTINGS_SCREEN);
		}
		else if (m_AppLayout == 2)
		{
			m_AppLayout = 0;
			S3GDIChangeScreen(S3_SETTINGS_SCREEN);
		}

		return 1;
	}
	else
	{
		return 1;
	}

	return 0;
}

// ----------------------------------------------------------------------------
