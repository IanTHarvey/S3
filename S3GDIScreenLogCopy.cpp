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

extern int S3I2CTest();

// ----------------------------------------------------------------------------

void CS3GDIScreenMain::S3InitGDILogCopyScreen(void)
{
	m_RectLogCopyScreen = m_RectScreen;
	m_RectLogCopyScreen.top = m_RectHeader.bottom;

	m_RectLogCopyInstr = m_RectLogCopyScreen;
	m_RectLogCopyInstr.left += 40;
	m_RectLogCopyInstr.top = m_RectLogCopyScreen.top;
	m_RectLogCopyInstr.right -= 40;
	m_RectLogCopyInstr.bottom = m_RectLogCopyInstr.top + 120;

	m_RectYes.left = m_RectLogCopyScreen.left;
	m_RectYes.top = m_RectLogCopyInstr.bottom;
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

#define S3GDI_MAX_SCREEN_MSG	256

// Layout 0
const wchar_t LogCopyInstr[] = {
	_T("Log file will be copied to root folder of a USB Mass Storage class device.")};

// Layout 1
const wchar_t LogCopyNoFile[] = {
	_T("Log file not found.")};

// Layout 1 Errors
const wchar_t LogCopyNoUSBHDD[] = {
	_T("No USB Mass Storage device found.")};

const wchar_t LogCopyLogFileExists[] = {
	_T("The named log file already exists on the USB drive.")};

const wchar_t LogCopyFailed[] = {
	_T("Copy failed")};

// Layout 2
const wchar_t LogCopyComplete[] = {
	_T("Copy completed.")};


unsigned char m_LogCopyLayout;
unsigned char m_LogCopyMsg;

void CS3GDIScreenMain::S3DrawGDILogCopyScreen()
{
	S3DrawGDIBackButton();
	
	SelectObject(m_HDC, m_hPenNone);
	SelectObject(m_HDC, m_hBrushBG3);
	
	S3_RECT(m_HDC, m_RectLogCopyScreen);

	if (m_LogCopyLayout == 0)
	{
		SetTextColor(m_HDC, m_crBlack);
		SelectObject(m_HDC, m_hBrushBG4);
		SelectObject(m_HDC, m_hFontL);

		DrawText(m_HDC, LogCopyInstr, -1, &m_RectLogCopyInstr, DT_WORDBREAK);

		SetTextColor(m_HDC, m_crWhite);
		SelectObject(m_HDC, m_hBrushSleep);
		// SelectObject(m_HDC, m_hFontL);
		
		S3BLT(m_hbmpBlueButton, m_RectYes.left, m_RectYes.top, 150, 50);
		
		DrawText(m_HDC, _T("Copy"), -1, &m_RectYes,
			S3_BTN_CENTRE);

		S3BLT(m_hbmpBlueButton, m_RectNo.left, m_RectNo.top, 150, 50);
		
		DrawText(m_HDC, _T("Cancel"), -1, &m_RectNo,
			S3_BTN_CENTRE);
	}
	else if (m_LogCopyLayout == 1)
	{
		SetTextColor(m_HDC, m_crBlack);
		SelectObject(m_HDC, m_hBrushBG4);
		SelectObject(m_HDC, m_hFontL);

		wchar_t tmp[S3GDI_MAX_SCREEN_MSG];

		// Give a reason
		if (m_LogCopyMsg == 1)
			swprintf_s(tmp, S3GDI_MAX_SCREEN_MSG, _T("%s: %s"),
				LogCopyFailed, LogCopyNoUSBHDD);
		else  if (m_LogCopyMsg == 2)
			swprintf_s(tmp, S3GDI_MAX_SCREEN_MSG, _T("%s: %s"),
				LogCopyFailed, LogCopyNoFile);
		else  if (m_LogCopyMsg == 3)
			swprintf_s(tmp, S3GDI_MAX_SCREEN_MSG, _T("%s: %s"),
				LogCopyFailed, LogCopyLogFileExists);
			
		DrawText(m_HDC, tmp, -1, &m_RectLogCopyInstr, DT_WORDBREAK);

		SetTextColor(m_HDC, m_crWhite);
		SelectObject(m_HDC, m_hBrushSleep);
		// SelectObject(m_HDC, m_hFontL);
		
		S3BLT(m_hbmpBlueButton, m_RectYes.left, m_RectYes.top, 150, 50);
		DrawText(m_HDC, _T("Try again"), -1, &m_RectYes,
			S3_BTN_CENTRE);

		S3BLT(m_hbmpBlueButton, m_RectNo.left, m_RectNo.top, 150, 50);
		DrawText(m_HDC, _T("Cancel"), -1, &m_RectNo, S3_BTN_CENTRE);
	}
	else if (m_LogCopyLayout == 2)
	{
		SetTextColor(m_HDC, m_crBlack);
		SelectObject(m_HDC, m_hBrushBG4);
		SelectObject(m_HDC, m_hFontL);

		DrawText(m_HDC, LogCopyComplete, -1, &m_RectLogCopyInstr, DT_WORDBREAK);

		SetTextColor(m_HDC, m_crWhite);
		SelectObject(m_HDC, m_hBrushSleep);
		
		S3BLT(m_hbmpBlueButton, m_RectNo.left, m_RectNo.top, 150, 50);
		DrawText(m_HDC, _T("Back"), -1, &m_RectNo,
			S3_BTN_CENTRE);
	}
}

// ----------------------------------------------------------------------------
// TODO: Garbage?


int CS3GDIScreenMain::S3FindLogCopyScreen(POINT p)
{
	m_LogCopyMsg = 0;

	if (m_RectBackButton.PtInRect(p))
	{
		m_LogCopyLayout = 0;

		S3GDIChangeScreen(S3_OVERVIEW_SCREEN);
		
		return 1;
	}

	if (m_RectYes.PtInRect(p))
	{
		if (m_LogCopyLayout == 0)
		{
			if (m_LogCopyMsg = S3LogFileToUSBRequest())
			{
				m_LogCopyLayout = 1;
			}
			else
			{
				m_LogCopyLayout = 2;
			}
		}
		else if (m_LogCopyLayout == 1)
		{
			m_LogCopyLayout = 0;
		}

		// Go to confirm screen
		return 1;
	}
	else if (m_RectNo.PtInRect(p))
	{
		if (m_LogCopyLayout == 0)
		{
			S3GDIChangeScreen(S3_SETTINGS_SCREEN);
		}
		else if (m_LogCopyLayout == 1)
		{
			m_LogCopyLayout = 0;
			S3GDIChangeScreen(S3_SETTINGS_SCREEN);
		}
		else if (m_LogCopyLayout == 2)
		{
			m_LogCopyLayout = 0;
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
