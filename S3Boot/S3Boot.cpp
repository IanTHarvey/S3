// S3Boot.cpp : Defines the entry point for the console application.
//
// Sentinel 3 installation and update loader.
//

#include "windows.h"
#include "Winbase.h"
#include "stdafx.h"
#include "stdlib.h"

#include "SHA256.h"

#define S3_MAX_FILENAME_LEN		128

#define	S3_ROOT_DIR			"\\FlashDisk\\S3"
#define S3_RUN_PATH			"\\Flashdisk\\S3Controller"
#define S3_RUN_DIR			"S3Controller"
#define S3_HOME_DIR			"\\Flashdisk\\S3"
#define S3_IMAGE_FILE		"nk.nb0"
#define S3_REG_FILE			"Registry.dat"

#define S3_PANIC_FILE		_T(S3_HOME_DIR) _T("\\S3Panic.s3p")
#define S3_VERSION_FILE		_T(S3_HOME_DIR) _T("\\S3Boot.s3v")
#define S3_UPDATE_FILENAME	"S3CUpdate.upd"
#define S3_EXE_NAME			"S3Controller.exe"
#define S3_EXE_BAK_NAME		"S3Controller.bak"

// #define S3_BOOT_VERSION		"1.1"		// Initial version
// #define S3_BOOT_VERSION		"1.2"		// Detects OS update file. If found, copies
											// S3Controller.exe from \Windows
// #define S3_BOOT_VERSION		"1.3"		// Also copies support files from \Windows
// #define S3_BOOT_VERSION		"1.4"		// Deletes saved registry if present
// #define S3_BOOT_VERSION		"1.5"		// No change
#define S3_BOOT_VERSION			"1.06.000"	// Detect and mitigate missing S3Controller.exe

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

	char s[] = {"Sentinel 3"};
	char buf[2 * SHA256::DIGEST_SIZE + 1];
	FILE	*Fver;
	
	sha256(buf, s, 10);
	
	// May be useful to report by S3Controller in future
	err = _wfopen_s(&Fver, S3_VERSION_FILE, _T("w"));
	if (!err)
	{
		fprintf(Fver, "%s", S3_BOOT_VERSION);
		fclose(Fver);
	}

	err = fopen_s(&logfid, "\\Flashdisk\\S3Boot.log", "w");
	fprintf(logfid, "Started S3Boot (v%s).\n", S3_BOOT_VERSION);

	// ---------------------------------------------------------------------------
	// FIRST USE INSTALLATION

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
			fprintf(logfid, "Failed to create S3Controller directory: %d\n",
				GetLastError());
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
			fprintf(logfid, "Failed to create S3 directory: %d\n",
				GetLastError());
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
			fprintf(logfid, "OS update file not found.\n");
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

	// Final test; if no /Flashdisk/S3Controller.exe, S3 is effectively bricked
	// as no Ethernet services started if already locked. So look to re-instate
	// from back-up file or executable from \Windows image.

	swprintf_s(src, S3_MAX_FILENAME_LEN, _T("%S\\%S"), S3_RUN_PATH, S3_EXE_NAME);
	
	fileAtt = GetFileAttributes(src);
	if (fileAtt == INVALID_FILE_ATTRIBUTES)
	{
		fprintf(logfid, "PANIC: No application file found, finding alternate\n");

		// Create a panic notification file so that S3Controller can do some
		// notification if required.
		FILE *FPanic;
		err = _wfopen_s(&FPanic, S3_PANIC_FILE, _T("w"));

		if (err)
		{
			fprintf(logfid, "Failed to create panic notification file: %d\n",
															GetLastError());
		}
		else
		{
			fprintf(logfid, "Created panic notification file\n");
			fclose(FPanic);
		}

		// Look for BAK file
		swprintf_s(src, S3_MAX_FILENAME_LEN,	_T("%S\\%S"), S3_RUN_PATH, S3_EXE_BAK_NAME);
		swprintf_s(dest, S3_MAX_FILENAME_LEN,	_T("%S\\%S"), S3_RUN_PATH, S3_EXE_NAME);

		if (CopyFile(src, dest, false) == false)
		{
			DWORD gle = GetLastError();

			if (GetLastError() == ERROR_FILE_NOT_FOUND)
				fprintf(logfid, "No application back-up file found\n");
			else
				fprintf(logfid, "Copy of application back-up file failed: %d\n", gle);

			// Look for file in Windows image
			if (S3BootCopy2Flash(_T(S3_EXE_NAME), _T(S3_RUN_DIR)))
			{
				fprintf(logfid, "Failed to copy S3Controller from Windows image to run-time directory\n");
			}
			else
			{
				fprintf(logfid, "Copied S3Controller from Windows image to run-time directory\n");
			}
		}
		else
			fprintf(logfid, "Application back-up file copied\n");

		fprintf(logfid, "PANIC: Measures finished\n");
	}
	else
	{
		fprintf(logfid, "Application file present, no panic\n");

		if (DeleteFile(S3_PANIC_FILE) == false)
		{
			if (GetLastError() != ERROR_FILE_NOT_FOUND)
				fprintf(logfid, "Failed to delete panic file\n");
		}
		else
			fprintf(logfid, "Panic file deleted\n");
	}

	// Signal completed for dependants (S3Controller.exe)
	// See /HKLM/init

#ifdef _WIN32_WCE
	DWORD LaunchNo = _wtoi(argv[1]);
	SignalStarted(LaunchNo);

	fprintf(logfid, "SignalStarted (%d)\nFinishing S3Boot.\n", LaunchNo);
#endif

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

// -----------------------------------------------------------------------------

#define HEADER_SIZE		4096

int CheckExtractBinary()
{
	FILE *fid;
	
	errno_t err = fopen_s(&fid, S3_UPDATE_FILENAME, "rb");

	if (err)
	{
		fprintf(logfid, "Failed to open update file: %s\n", S3_UPDATE_FILENAME);
		return 1;
	}
	
	char Header[HEADER_SIZE];

	fread(Header, sizeof(char), HEADER_SIZE, fid);	

	int FileVersion[3];
	int HeaderSize;
	int PayloadSize;
	int	Hash[32];
	
	for(int i = 0; i < 3; i++)
	{
		FileVersion[i] = *((int *)Header + i * sizeof(int));
	}

	if (1)
	{
		HeaderSize = *((int *)Header + 3 * sizeof(int));
		PayloadSize = *((int *)Header + 4 * sizeof(int));

		memcpy(Hash, ((int *)Header + 5 * sizeof(int)), 32 * sizeof(int));
	}
	
	fclose(fid);

	return 0;
}

// -----------------------------------------------------------------------------