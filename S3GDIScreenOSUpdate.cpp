// OS image update screens

#include "stdafx.h"

#include "S3SystemDetails.h"
#include "S3DataModel.h"
#include "S3Update.h"

extern S3DataModel *S3Data;

#ifdef S3_AGENT
#include "S3Agent/S3AgentDlg.h"
#else
#include "S3ControllerDlg.h"
#endif

wchar_t	spinner[5] = {_T("|/-\\")};
char	spincnt = 0;

// Layout 0
const wchar_t SWUpdateInstr[] = {
	_T("Only attempt update if you have a Sentinel 3 image file provided by PPM")
	_T("and located in the root folder of a USB Mass Storage class device.")};

// Layout 1
const wchar_t SWUpdateNoImage[] = {
	_T("Sentinel 3 Image update failed")};

// Layout 1 Errors
const wchar_t SWUpdateNoImageHDD[] = {
	_T("No USB Mass Storage device found.")};

const wchar_t SWUpdateNoImageFile[] = {
	_T("No image file found on root of USB storage device.")};

const wchar_t SWUpdateProcessFail[] = {
	_T("An error occurred during the update process.")};

// Layout 2
const wchar_t SWUpdateDoUpdate[] = {
	_T("Sentinel 3 will update and restart, configuration data will be retained. ")
	_T("Ensure that power is not interrupted during the update.")};

// Layout 3
const wchar_t SWUpdateUpdating[] = {
	_T("Updating will take 1-2 minutes.\nDo not switch off mains power.\n")
	_T("Please wait until Sentinel 3 restarts fully.")};

unsigned char m_Layout = 0;
unsigned char m_UpdateMsg = 0;

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3InitGDISWUpdateScreen(void)
{
	m_RectSWUpdateScreen = m_RectScreen;
	m_RectSWUpdateScreen.top = m_RectHeader.bottom;

	m_RectSWUpdateInstr = m_RectSWUpdateScreen;
	m_RectSWUpdateInstr.left += 40;
	m_RectSWUpdateInstr.top = m_RectSWUpdateScreen.top;
	m_RectSWUpdateInstr.right -= 40;
	m_RectSWUpdateInstr.bottom = m_RectSWUpdateInstr.top + 240;

	m_RectSWUpdateYes.left = m_RectSWUpdateScreen.left;
	m_RectSWUpdateYes.top = m_RectSWUpdateInstr.bottom;
	m_RectSWUpdateYes.right = m_RectSWUpdateYes.left + 150;
	m_RectSWUpdateYes.bottom = m_RectSWUpdateYes.top + 50;

	m_RectSWUpdateNo = m_RectSWUpdateYes;
	m_RectSWUpdateNo.left = m_RectSWUpdateYes.right;
	m_RectSWUpdateNo.right = m_RectSWUpdateNo.left + 150;

	// Distribute horizontally
	m_RectSWUpdateYes.MoveToXY(
		(m_RectScreen.Width() / 2 - m_RectSWUpdateYes.Width()) / 2,
		m_RectSWUpdateYes.top + 50);

	m_RectSWUpdateNo.MoveToXY(
		1 * m_RectScreen.Width() / 2 + (m_RectScreen.Width() / 2 - m_RectSWUpdateNo.Width()) / 2,
		m_RectSWUpdateNo.top + 50);
}

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3DrawGDISWUpdateScreen()
{
	S3DrawGDIBackButton();
	
	SelectObject(m_HDC, m_hPenNone);
	SelectObject(m_HDC, m_hBrushBG3);
	
	S3_RECT(m_HDC, m_RectSWUpdateScreen);

	if (S3OSUpdateFail())
	{
		m_Layout = 1;
		m_UpdateMsg = 3;

		// Told the user, so let it go
		S3OSSetUpdateFail(false);
	}

	if (m_Layout == 0)
	{
		SetTextColor(m_HDC, m_crBlack);
		SelectObject(m_HDC, m_hBrushBG4);
		SelectObject(m_HDC, m_hFontL);
		DrawText(m_HDC, SWUpdateInstr, -1, &m_RectSWUpdateInstr, DT_WORDBREAK);

		SetTextColor(m_HDC, m_crWhite);
		SelectObject(m_HDC, m_hBrushSleep);
	
		S3BLT(m_hbmpBlueButton, m_RectSWUpdateYes.left, m_RectSWUpdateYes.top, 150, 50);
		
		DrawText(m_HDC, _T("Update"), -1, &m_RectSWUpdateYes,
			S3_BTN_CENTRE);

		S3BLT(m_hbmpBlueButton, m_RectSWUpdateNo.left, m_RectSWUpdateNo.top, 150, 50);
		
		DrawText(m_HDC, _T("Cancel"), -1, &m_RectSWUpdateNo,
			S3_BTN_CENTRE);
	}
	else if (m_Layout == 1)
	{
		SetTextColor(m_HDC, m_crBlack);
		SelectObject(m_HDC, m_hBrushBG4);
		SelectObject(m_HDC, m_hFontL);

		wchar_t tmp[S3GDI_MAX_SCREEN_MSG];

		// Give a reason
		if (!S3Data->m_ImgUpdate->Unwrapping)
		{
			if (S3Data->m_ImgUpdate->GetError() == 2001)
				swprintf_s(tmp, S3GDI_MAX_SCREEN_MSG, _T("%s: %s"),
					SWUpdateNoImage, _T("Update file not found"));
			else  if (m_UpdateMsg == 3)
				swprintf_s(tmp, S3GDI_MAX_SCREEN_MSG, _T("%s: %s"),
					SWUpdateNoImage, SWUpdateProcessFail);
			else
			{
				m_Layout = 2;
				return;
			}

			DrawText(m_HDC, tmp, -1, &m_RectSWUpdateInstr, DT_WORDBREAK);

			SetTextColor(m_HDC, m_crWhite);
			SelectObject(m_HDC, m_hBrushSleep);
			
			S3BLT(m_hbmpBlueButton, m_RectSWUpdateYes.left, m_RectSWUpdateYes.top, 150, 50);
			DrawText(m_HDC, _T("Try again"), -1, &m_RectSWUpdateYes,
				S3_BTN_CENTRE);

			S3BLT(m_hbmpBlueButton, m_RectSWUpdateNo.left, m_RectSWUpdateNo.top, 150, 50);
			DrawText(m_HDC, _T("Cancel"), -1, &m_RectSWUpdateNo,
				S3_BTN_CENTRE);
		}
		else
		{
			CString SWUpdateMsg;
			SWUpdateMsg.Format(_T("Unpacking update file. Please wait %c\n"),
				spinner[spincnt++]);

			if (spincnt == 4)
				spincnt = 0;

			DrawText(m_HDC, SWUpdateMsg, -1, &m_RectSWUpdateInstr, DT_WORDBREAK);
		}
	}
	else if (m_Layout == 2)
	{
		SetTextColor(m_HDC, m_crBlack);
		SelectObject(m_HDC, m_hBrushBG4);
		SelectObject(m_HDC, m_hFontL);

		CString SWUpdateMsg = SWUpdateDoUpdate;

		if (!S3Data->m_ImgUpdate->GetError())
		{
			SWUpdateMsg.Format(_T("Valid OS image update file found: ")
				_T("v%s (%s), current is v%s. ")
				_T("Check new version is correct.\n\n"), 
				S3Data->m_ImgUpdate->GetVersion(),
				S3Data->m_ImgUpdate->GetDateTime(),
				_T(S3_SYS_SW));

			SWUpdateMsg += SWUpdateDoUpdate;
		}
		else
			SWUpdateMsg.Format(_T("Update file invalid: %s\n"),
									S3Data->m_ImgUpdate->GetErrorStr());

		DrawText(m_HDC, SWUpdateMsg, -1, &m_RectSWUpdateInstr, DT_WORDBREAK);

		SetTextColor(m_HDC, m_crWhite);
		SelectObject(m_HDC, m_hBrushSleep);
		
		if (!S3Data->m_ImgUpdate->Unwrapping)
		{
			if (!S3Data->m_ImgUpdate->GetError())
			{
				S3BLT(m_hbmpBlueButton, m_RectSWUpdateYes.left, m_RectSWUpdateYes.top, 150, 50);
				DrawText(m_HDC, _T("Update"), -1, &m_RectSWUpdateYes,
					S3_BTN_CENTRE);
			}

			S3BLT(m_hbmpBlueButton, m_RectSWUpdateNo.left, m_RectSWUpdateNo.top, 150, 50);
			DrawText(m_HDC, _T("Cancel"), -1, &m_RectSWUpdateNo,
				S3_BTN_CENTRE);
		}
	}
	else if (m_Layout == 3)
	{
		if (m_Parent->m_SWUpdateScheduled == false)
		{
			m_Layout = 1;
			m_UpdateMsg = 3;
		}

		SetTextColor(m_HDC, m_crRed);
		SelectObject(m_HDC, m_hBrushBG4);
		SelectObject(m_HDC, m_hFontL);

		DrawText(m_HDC, SWUpdateUpdating, -1, &m_RectSWUpdateInstr, DT_WORDBREAK);
	}
}

// ----------------------------------------------------------------------------

int CS3GDIScreenMain::S3FindSWUpdateScreen(POINT p)
{
	if (S3Data->m_ImgUpdate->Unwrapping)
		return 0;

	if (m_Layout == 3)
		return 0;

	if (m_RectBackButton.PtInRect(p))
	{
		m_Layout = 0;

		S3GDIChangeScreen(S3_OVERVIEW_SCREEN);
		
		return 1;
	}

	if (m_RectSWUpdateYes.PtInRect(p))
	{
		S3EventLogAdd("OS image update requested by user", 1, -1, -1, -1);

		if (m_Layout == 0)
		{
			S3OSSWUpdateRequest();
			m_Layout = 1;
		}
		else if (m_Layout == 1)
		{
			m_Layout = 0;
		}
		else if (m_Layout == 2)
		{
			if (S3Data->m_ImgUpdate->GetError())
				return 0;

			m_Layout = 3;
			m_Parent->m_SWUpdateScheduled = true;
		}

		// Go to confirm screen
		return 1;
	}
	else if (m_RectSWUpdateNo.PtInRect(p))
	{
		S3EventLogAdd("OS image update cancelled by user", 1, -1, -1, -1); 
		if (m_Layout == 0)
		{
			S3GDIChangeScreen(S3_SETTINGS_SCREEN);
		}
		else if (m_Layout == 1)
		{
			m_Layout = 0;
			S3GDIChangeScreen(S3_SETTINGS_SCREEN);
		}
		else if (m_Layout == 2)
		{
			m_Layout = 0;
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
