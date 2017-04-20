// OS image update screens

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

// bool m_SWUpdateScheduled = false;

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3InitGDISWUpdateScreen(void)
{
	m_RectSWUpdateScreen = m_RectScreen;
	m_RectSWUpdateScreen.top = m_RectHeader.bottom;

	m_RectSWUpdateInstr = m_RectSWUpdateScreen;
	m_RectSWUpdateInstr.left += 40;
	m_RectSWUpdateInstr.top = m_RectSWUpdateScreen.top;
	m_RectSWUpdateInstr.right -= 40;
	m_RectSWUpdateInstr.bottom = m_RectSWUpdateInstr.top + 120;

	m_RectYes.left = m_RectSWUpdateScreen.left;
	m_RectYes.top = m_RectSWUpdateInstr.bottom;
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
}

// ----------------------------------------------------------------------------

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
	_T("Sentinel 3 will update and restart, configuration data will be retained.")
	_T("Ensure that power is not interrupted during the update.")};

// Layout 3
const wchar_t SWUpdateUpdating[] = {
	_T("Updating will take 1-2 minutes.\nDo not switch off mains power.\n")
	_T("Please wait until Sentinel 3 restarts fully.")};

unsigned char m_Layout = 0;
unsigned char m_UpdateMsg = 0;

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
		// SelectObject(m_HDC, m_hFontL);
	
		S3BLT(m_hbmpBlueButton, m_RectYes.left, m_RectYes.top, 150, 50);
		
		DrawText(m_HDC, _T("Update"), -1, &m_RectYes,
			S3_BTN_CENTRE);

		S3BLT(m_hbmpBlueButton, m_RectNo.left, m_RectNo.top, 150, 50);
		
		DrawText(m_HDC, _T("Cancel"), -1, &m_RectNo,
			S3_BTN_CENTRE);
	}
	else if (m_Layout == 1)
	{
		SetTextColor(m_HDC, m_crBlack);
		SelectObject(m_HDC, m_hBrushBG4);
		SelectObject(m_HDC, m_hFontL);

		wchar_t tmp[S3GDI_MAX_SCREEN_MSG];

		// Give a reason
		if (m_UpdateMsg == 1)
			swprintf_s(tmp, S3GDI_MAX_SCREEN_MSG, _T("%s: %s"),
				SWUpdateNoImage, SWUpdateNoImageHDD);
		else  if (m_UpdateMsg == 2)
			swprintf_s(tmp, S3GDI_MAX_SCREEN_MSG, _T("%s: %s"),
				SWUpdateNoImage, SWUpdateNoImageFile);
		else  if (m_UpdateMsg == 3)
			swprintf_s(tmp, S3GDI_MAX_SCREEN_MSG, _T("%s: %s"),
				SWUpdateNoImage, SWUpdateProcessFail);
		else
			swprintf_s(tmp, S3GDI_MAX_SCREEN_MSG,
				_T("%s: Unknown error (%d)"), SWUpdateNoImage, m_UpdateMsg);
			
		DrawText(m_HDC, tmp, -1, &m_RectSWUpdateInstr, DT_WORDBREAK);

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
	else if (m_Layout == 2)
	{
		SetTextColor(m_HDC, m_crBlack);
		SelectObject(m_HDC, m_hBrushBG4);
		SelectObject(m_HDC, m_hFontL);

		DrawText(m_HDC, SWUpdateDoUpdate, -1, &m_RectSWUpdateInstr, DT_WORDBREAK);

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
// TODO: Garbage?


int CS3GDIScreenMain::S3FindSWUpdateScreen(POINT p)
{
	// m_UpdateMsg = 0;

	if (m_Layout == 3)
		return 0;

	if (m_RectBackButton.PtInRect(p))
	{
		m_Layout = 0;

		S3GDIChangeScreen(S3_OVERVIEW_SCREEN);
		
		return 1;
	}

	if (m_RectYes.PtInRect(p))
	{
		// S3GDIChangeScreen(S3_OVERVIEW_SCREEN);
		S3EventLogAdd("OS image update requested by user", 1, -1, -1, -1);

		if (m_Layout == 0)
		{
			if (m_UpdateMsg = S3OSSWUpdateRequest())
			{
				m_Layout = 1;
			}
			else
			{
				m_Layout = 2;
			}
		}
		else if (m_Layout == 1)
		{
			m_Layout = 0;
		}
		else if (m_Layout == 2)
		{
			m_Layout = 3;
			m_Parent->m_SWUpdateScheduled = true;
		}

		// Go to confirm screen
		return 1;
	}
	else if (m_RectNo.PtInRect(p))
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
		// S3GDIChangeScreen(S3_OVERVIEW_SCREEN);

		return 1;
	}

	return 0;
}

// ----------------------------------------------------------------------------
