// S3Boot.cpp : Defines the entry point for the console application.
//
// Sentinel 3 installation and update loader.
//
// TODO:
// Use better update exe naming convention (add obfuscation).
// Check update later than current (incorporate date/version in above).
// 

#include "windows.h"
#include "Winbase.h"
#include "stdafx.h"
#include "stdlib.h"

#define S3_MAX_FILENAME_LEN		128

#define	S3_ROOT_DIR		"\\FlashDisk\\S3"
#define S3_RUN_PATH		"\\Flashdisk\\S3Controller"
#define S3_RUN_DIR		"S3Controller"
#define S3_HOME_DIR		"\\Flashdisk\\S3"
#define S3_IMAGE_FILE	"nk.nb0"
#define S3_REG_FILE		"Registry.dat"

#define S3_UPDATE_FILENAME	"S3CUpdate.upd"
#define S3_EXE_NAME			"S3Controller.exe"
#define S3_EXE_BAK_NAME		"S3Controller.bak"

// #define S3_BOOT_VERSION		"1.1"
// #define S3_BOOT_VERSION		"1.2"	// Detects OS update file. If found, copies
										// S3Controller.exe from \Windows
// #define S3_BOOT_VERSION		"1.3"	// Also copies support files from \Windows
// #define S3_BOOT_VERSION		"1.4"	// Deletes saved registry if present
#define S3_BOOT_VERSION			"1.5"	// No change

#define S3_OSDATE_FILENAME	"S3OSDate"

int S3BootCopy2Flash(const wchar_t *src, const wchar_t *dest);
FILE	*logfid;

// ---------------------------------------------------------------------------

// int WinMain(int argc, _TCHAR* argv[])
int _tmain(int argc, _TCHAR* argv[])
{	
	wchar_t src[S3_MAX_FILENAME_LEN];
	wchar_t dest[S3_MAX_FILENAME_LEN];
	errno_t		err, ret_err = 0;
	BOOL		berr;
	DWORD	fileAtt;

	// ---------------------------------------------------------------------------
	// FIRST USE INSTALLATION

	err = fopen_s(&logfid, "\\Flashdisk\\S3Boot.log", "w");

	fprintf(logfid, "Started S3Boot (v%s).\n", S3_BOOT_VERSION);

	fileAtt = GetFileAttributes(_T(S3_RUN_PATH));

	if (fileAtt == INVALID_FILE_ATTRIBUTES ||
								(fileAtt & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		fprintf(logfid, "S3Controller directory not found. New install.\n");
		
		fprintf(logfid, "Deleting image file %s (if exists).\n", S3_IMAGE_FILE);
		swprintf_s(dest, S3_MAX_FILENAME_LEN, _T("Flashdisk\\%S"), S3_IMAGE_FILE);
		DeleteFile(dest);

		fprintf(logfid, "Deleting saved registry file %s (if exists).\n", S3_REG_FILE);
		swprintf_s(dest, S3_MAX_FILENAME_LEN, _T("Flashdisk\\%S"), S3_REG_FILE);
		DeleteFile(dest);
		
		berr = CreateDirectory(_T(S3_RUN_PATH), NULL);

		if (berr == 0)
		{
			fprintf(logfid, "Failed to create S3Controller directory\n");
			ret_err = 1;
		}

		fprintf(logfid, "Created S3Controller directory\n");

		if (S3BootCopy2Flash(_T(S3_EXE_NAME), _T(S3_RUN_DIR)))
		{
			ret_err = 2;
		}
		else
		{
			fprintf(logfid, "Copied S3Controller to run-time directory\n");
		}
	}
	else
		fprintf(logfid, "S3Controller run-time directory already installed.\n");

	fileAtt = GetFileAttributes(_T(S3_HOME_DIR));

	if (fileAtt == INVALID_FILE_ATTRIBUTES ||
								(fileAtt & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		fprintf(logfid, "S3 directory not found. New install.\n");

		berr = CreateDirectory(_T(S3_HOME_DIR), NULL);

		if (berr == 0)
		{
			fprintf(logfid, "Failed to create S3 directory\n");
			ret_err = 3;
		}
		else
		{
			fprintf(logfid, "Created S3 directory\n");
			// BOOT_PPM_Test_18bpp.png

			S3BootCopy2Flash(_T("BOOT_PPM_Test_18bpp.png"), _T("boot.png"));
			S3BootCopy2Flash(_T("S3Demo.s3c"), _T("S3"));
			S3BootCopy2Flash(_T("S3Default.s3c"), _T("S3"));
		}

		STARTUPINFO si;
		ZeroMemory( &si, sizeof(si) );
		si.cb = 0; // sizeof(si);

		PROCESS_INFORMATION pi;

		wchar_t cmd[] = _T("\\Windows\\touchc.exe");

		// See https://msdn.microsoft.com/en-us/library/ee488927.aspx for unsupported
		// args in WINCE
		if (CreateProcess(cmd, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
		{
			fprintf(logfid, "Running touch screen calibration\n");
			WaitForSingleObject(pi.hProcess, INFINITE);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
		else
			fprintf(logfid, "Failed to create touch screen calibration process: %d\n",
								GetLastError());
	}
	else
	{
		fprintf(logfid, "S3 data directory already installed.\n");

		// Has OS update been attempted?
		swprintf_s(src, S3_MAX_FILENAME_LEN, _T("%S\\%S.s3o"),
			S3_ROOT_DIR, S3_OSDATE_FILENAME);

		FILE *f;

		err = _wfopen_s(&f, src, _T("rb"));

		if (!err)
		{
			fclose(f);

			fprintf(logfid, "Found OS update file. Copying S3 exe from Windows to Flashdisk\n");
			
			if (S3BootCopy2Flash(_T(S3_EXE_NAME), _T(S3_RUN_DIR)))
			{
				fprintf(logfid, "Copy S3Controller to run-time directory failed\n");
				ret_err = 2;
			}
			else
			{
				fprintf(logfid, "Copied S3Controller to run-time directory\n");
			}
		}
		else
			fprintf(logfid, "OS update file not found.");
	}

	if (ret_err)
	{
		fprintf(logfid, "Closing with installation error: %d\n", ret_err);
		fclose(logfid);
		return ret_err;
	}

	// copy \Flashdisk\S3Controller\S3Controller.exe \Flashdisk\S3Controller\S3Controller.bak 
	// copy \Flashdisk\S3ControllerNew.exe \Flashdisk\S3Controller\S3Controller.exe

	// ---------------------------------------------------------------------------
	// CHECK FOR S3CONTROLLER APPLICATION UPDATE
	
	swprintf_s(src, S3_MAX_FILENAME_LEN, _T("\\Flashdisk\\%S"), S3_UPDATE_FILENAME);

	FILE *f;

	err = _wfopen_s(&f, src, _T("rb"));

	if (!err)
	{
		fclose(f);

		fprintf(logfid, "Found application update file\n");

		// --------------------------------------------------------------------
		swprintf_s(src, S3_MAX_FILENAME_LEN,	_T("%S\\%S"), S3_RUN_PATH, S3_EXE_NAME);
		swprintf_s(dest, S3_MAX_FILENAME_LEN,	_T("%S\\%S"), S3_RUN_PATH, S3_EXE_BAK_NAME);

		DWORD gle = 0;

		if (!DeleteFile(dest))
		{
			gle = GetLastError();
		}
		
		if (!gle || gle == ERROR_FILE_NOT_FOUND)
		{
			fprintf(logfid, "Delete old back-up OK\n");
		
			gle = 0;
			
			if (!MoveFile(src, dest))
			{
				fprintf(logfid, "Back-up failed\n");
			}
			else
			{
				fprintf(logfid, "Back-up OK\n");
			}
		}
		else
		{
			if (gle == ERROR_ACCESS_DENIED)
				fprintf(logfid, "Delete old back-up: Access denied\n");
			else
				fprintf(logfid, "Delete old back-up: Unknown error\n");
		}

		// --------------------------------------------------------------------
		swprintf_s(src, S3_MAX_FILENAME_LEN,	_T("\\Flashdisk\\%S"), S3_UPDATE_FILENAME);
		swprintf_s(dest, S3_MAX_FILENAME_LEN,	_T("%S\\%S"), S3_RUN_PATH, S3_EXE_NAME);

		gle = 0;

		if (!DeleteFile(dest))
		{
			gle = GetLastError();
		}

		if (!gle || gle == ERROR_FILE_NOT_FOUND)
		{
			fprintf(logfid, "Delete old exe OK\n");

			if (!MoveFile(src, dest))
			{
				fprintf(logfid, "Update failed\n");
			}
			else
				fprintf(logfid, "Update OK\n");
		}
		else
		{
			if (gle == ERROR_ACCESS_DENIED)
				fprintf(logfid, "Delete old exe: Access denied\n");
			else
				fprintf(logfid, "Delete old exe: Unknown error\n");
		}
	}
	else
		fprintf(logfid, "No application update file found\n");

	// Signal completed for dependents (S3Controller.exe)
	// See /HKLM/init
	DWORD LaunchNo = _wtoi(argv[1]);
	SignalStarted(LaunchNo);

	fprintf(logfid, "SignalStarted (%d)\nFinishing S3Boot.\n", LaunchNo);
	fclose(logfid);

	return ret_err;
}

// ----------------------------------------------------------------------------
// Copy image \Windows files to \Flashdisk\destdir
// destdir may be a path\file relative to \Flashdisk.

int S3BootCopy2Flash(const wchar_t *file, const wchar_t *destdir)
{
	wchar_t isrc[S3_MAX_FILENAME_LEN];
	wchar_t idest[S3_MAX_FILENAME_LEN];

	DWORD	fileAtt;

	swprintf_s(isrc, S3_MAX_FILENAME_LEN,	_T("\\Windows\\%s"), file);

	swprintf_s(idest, S3_MAX_FILENAME_LEN,	_T("\\Flashdisk\\%s"), destdir);
	fileAtt = GetFileAttributes(idest);

	if (fileAtt == INVALID_FILE_ATTRIBUTES ||
								(fileAtt & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		// destdir is a file
		swprintf_s(idest, S3_MAX_FILENAME_LEN,	_T("\\Flashdisk\\%s"), destdir);
	}
	else
	{
		// destdir is a directory, so copy the source file name to destdir
		swprintf_s(idest, S3_MAX_FILENAME_LEN,	_T("\\Flashdisk\\%s\\%s"), destdir, file);
		
		if (GetFileAttributes(idest) != INVALID_FILE_ATTRIBUTES)
		{
			// File exists and just in case read-only
			if (!SetFileAttributes(idest, FILE_ATTRIBUTE_NORMAL))
				fprintf(logfid, "Copy destination: Failed to remove read-only permission\n");
		}
	}
 
	fwprintf(logfid, _T("Copying: %s -> %s: "), isrc, idest);

	if (!CopyFile(isrc, idest, false))
	{
		fwprintf(logfid, _T("Failed\n"));
		return 1;
	}
	else 
		fwprintf(logfid, _T("OK\n"));

	// Files copied from \Windows, will be read-only
	if (!SetFileAttributes(idest, FILE_ATTRIBUTE_NORMAL))
		fprintf(logfid, "Copied file: Failed to remove read-only permission\n");

	return 0;
}

// ----------------------------------------------------------------------------
