// ----------------------------------------------------------------------------
// OS-interface typ[e functions.
//
// The S3OS.obj file should be delated in post-build to ensure the application
// build date is updated with __DATE__ and __TIME__.

#include "windows.h"
#include "tchar.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "S3DataModel.h"

extern pS3DataModel S3Data;

#ifdef TRIZEPS
#include "drvLib_app.h"
#endif

// Func unsigned long GetImageId(char *ImageID);
// Func void          ResetAfter_us(unsigned long us,int  restartclean);
// Func void          SoftReset   ( int clean);
// Func int           WriteRegistry(void);
// Func int           ErasePersistantRegistry(void);


// Func BOOL GetRegistryBootloaderVersion(TCHAR* pEbootVersion);
// Func BOOL SaveBootloaderToFile(LPCTSTR filename);
// Func BOOL UpdateBootloader(LPCTSTR filename);
// Func BOOL VerifyBootloader(LPCTSTR filename);
// Func BOOL SaveImageToFile(LPCTSTR filename);
// Func BOOL UpdateImage(LPCTSTR filename);
// Func BOOL VerifyImage(LPCTSTR filename);

// ----------------------------------------------------------------------------

// #define RUNTIME_DIRECTORY

#define S3_RUN_DIR	"\\Flashdisk\\S3Controller"
#define S3_HOME_DIR	"\\Flashdisk\\S3"

#define S3_UPDATE_FILENAME	"S3CUpdate.upd"
#define S3_EXE_NAME			"S3Controller.exe"
#define S3_EXE_BAK_NAME		"S3Controller.bak"

// #ifdef TRIZEPS
wchar_t OSHDDDirectory[] =	{_T("\\Hard Disk")};
wchar_t OSFDDDirectory[] =	{_T("\\Flashdisk")};

wchar_t OSImageUpdateFilename[] =	{_T("S3TestOS.nb0")};
wchar_t OSAppUpdateFilename[] =		{_T(S3_UPDATE_FILENAME)};
wchar_t OSImageUpdateFilePath[MAX_PATH];
wchar_t OSAppUpdateFilePath[MAX_PATH];
wchar_t OSImageNB0FilePath[MAX_PATH];
// #endif

// ----------------------------------------------------------------------------


int S3OSInit()
{
	swprintf_s(OSImageUpdateFilePath, MAX_PATH, _T("%s\\%s"),
		OSHDDDirectory, OSImageUpdateFilename);

	swprintf_s(OSImageNB0FilePath, MAX_PATH, _T("%s\\%s"),
		OSFDDDirectory, OSImageUpdateFilename);

	swprintf_s(OSAppUpdateFilePath, MAX_PATH, _T("%s\\%s"),
		OSHDDDirectory, OSAppUpdateFilename);

	S3Data->m_OSUpdateFail = false;

	S3Data->m_PowerDownPending = false;
	S3Data->m_PowerDownFailed = false;

	// sprintf_s(S3Data->m_AppDateTime, S3_DATETIME_LEN, "%s %s",
	// 	__DATE__, __TIME__);

	S3BackupConfig();

#ifdef TRIZEPS
	BOOL ok = WriteRegistry();

	if (!ok)
	{
		S3EventLogAdd("S3OSInit: Failed to save registry on start-up", 1, -1, -1, -1);
		return 1;
	}

#endif

	// Read the name of the old image, to check if updated
	wchar_t FileName[S3_MAX_FILENAME_LEN];

	swprintf_s(FileName, S3_MAX_FILENAME_LEN, _T("%S\\%S.s3o"),
			S3_ROOT_DIR, S3_OSDATE_FILENAME);

	FILE *fid;
	errno_t err = _wfopen_s(&fid, FileName, _T("r"));

	if (!err)
	{
		// An update attempted
		fseek(fid, 0, SEEK_END);
		long	fsize = ftell(fid);
		fseek(fid, 0, SEEK_SET);  // Rewind

		// TODO: These should be handled as errors
		if (fsize >= S3_MAX_OS_IMAGE_ID_LEN)
		{
			S3EventLogAdd("S3OSInit: Image ID file size incorrect", 1, -1, -1, -1);
			return 1;
		}

		char	OldOSDate[S3_MAX_OS_IMAGE_ID_LEN];
		size_t nread = fread(OldOSDate, 1, fsize, fid);

		fclose(fid);

		if (nread != fsize)
		{
			S3EventLogAdd("S3OSInit: fread failed", 1, -1, -1, -1);
			return 1;
		}

		OldOSDate[fsize] = '\0';

		// If new image ID same as old, then assume an update failed
		if (!strcmp(OldOSDate, S3Data->m_ImageID))
		{
			S3EventLogAdd("S3OSInit: Failed as new image date same as old", 1, -1, -1, -1);
			S3Data->m_OSUpdateFail = true;
		}

		// Delete file whatever to prevent nagging
		// TODO: Test and handle
		BOOL ok = DeleteFile(FileName);

		if (!ok)
		{
			S3EventLogAdd("S3OSInit: Failed to delete image ID file", 1, -1, -1, -1);
			return 1;
		}
	}
	else
	{
		if (err != ENOENT)
		{
			S3EventLogAdd("S3OSInit: Failed to read saved image ID", 1, -1, -1, -1);
			return 1;
		}
	}

	return 0;
}

// ----------------------------------------------------------------------------

bool S3OSUpdateFail()
{
	return S3Data->m_OSUpdateFail;
}

void S3OSSetUpdateFail(bool fail)
{
	S3Data->m_OSUpdateFail = fail;
}

// ----------------------------------------------------------------------------

bool S3OSGetAppUpdateFail()
{
	return S3Data->m_OSAppUpdateFail;
}

void S3OSSetAppUpdateFail(bool fail)
{
	S3Data->m_OSAppUpdateFail = fail;
}

// ----------------------------------------------------------------------------

// Look for image file on USB
int S3OSImageUpdate()
{
	S3EventLogAdd("Image update", 1, -1, -1, -1);
#ifdef TRIZEPS

	FILE *f;
	errno_t	err;

	err = _wfopen_s(&f, OSImageUpdateFilePath, _T("rb"));

	if (err)
	{
		S3EventLogAdd("Image file not found", 1, -1, -1, -1);
		return 1;
	}

	fclose(f);

	BOOL verified = TRUE; // VerifyImage(OSImageUpdateFilePath);
	
	if (!verified)
	{
		S3EventLogAdd("Image file verification failed", 1, -1, -1, -1);
		return 2;
	}

	// TODO: Don't just copy here
	// Check and strip header, unencrypt binary...
	// TODO: Consider moving to S3Boot

	if (!CopyFile(OSImageUpdateFilePath, OSImageNB0FilePath, false))
	{
		S3EventLogAdd("Failed to copy image file from HDD to Flashdisk", 1, -1, -1, -1);
		return 5;
	}

	BOOL updated = UpdateImage(OSImageNB0FilePath);

	// TODO: Revisit...
	// int syserr = CreateProcess("dir");

	if (!updated)
	{
		S3EventLogAdd("Image update failed", 1, -1, -1, -1);
		return 3;
	}

	// Erase registry so all keys from new image not merged with old local store
	int DelReg = ErasePersistantRegistry();

	if (DelReg != 0x80000000)
	{
		S3EventLogAdd("Registry erase failed", 1, -1, -1, -1);
		return 4;
	}

	// Write the name of the old image, to check if updated after restart
	char FileName[S3_MAX_FILENAME_LEN];

	sprintf_s(FileName, S3_MAX_FILENAME_LEN, "%s\\%s.s3o",
			S3_ROOT_DIR, S3_OSDATE_FILENAME);

	FILE *fid;
	err = fopen_s(&fid, FileName, "w");

	if (!err)
	{
		fprintf(fid, "%s", S3Data->m_ImageID);
		fclose(fid);
	}
	else
		S3EventLogAdd("Failed to save old image ID: Ignored", 1, -1, -1, -1);

	S3OSRestart();

#endif

	return 0;
}

// ----------------------------------------------------------------------------
// Look for exe file on USB

int S3OSAppUpdate()
{
	S3EventLogAdd("App update", 1, -1, -1, -1);
#ifdef TRIZEPS

	if (!S3FileExist(OSAppUpdateFilePath))
	{
		S3EventLogAdd("App exe file not found", 1, -1, -1, -1);
		return 1;
	}

	// BOOL updated = UpdateImage(OSImageUpdateFilePath);
	// Copy image from "\Hard drive" to "Flashdisk"

	// wchar_t src[S3_MAX_FILENAME_LEN];
	wchar_t dest[S3_MAX_FILENAME_LEN];

	swprintf_s(dest, MAX_PATH, _T("%s\\%S"), OSFDDDirectory, S3_UPDATE_FILENAME);

	if (!CopyFile(OSAppUpdateFilePath, dest, false))
	{
		S3EventLogAdd("Copy of exe file failed", 3, -1, -1, -1);
		return 1;
	}

	S3EventLogAdd("Copy of exe file successful", 1, -1, -1, -1);

	// Restart a problem for S3Boot, which cannot copy/move exe files at
	// its point in the boot sequence. No error, just a zero-length file
	// of the correct name is written.

	S3OSRestart();
	// SystemShutdown();

	// S3SetPowerDownPending(true);
	// S3SetSleepAll(true);

#endif

	return 0;
}

// ----------------------------------------------------------------------------

int S3OSGetImageID()
{
#ifdef TRIZEPS
	char	ImageID[S3_MAX_OS_IMAGE_ID_LEN];

	unsigned long ok = GetImageId(ImageID);

	if (!ok)
	{
		strcpy_s(S3Data->m_ImageID, S3_MAX_OS_IMAGE_ID_LEN,
			"Failed to attain OS ID");

		return 1;
	}
	else
		strcpy_s(S3Data->m_ImageID, S3_MAX_OS_IMAGE_ID_LEN, ImageID);

	// Format "OS; dd/mm/yyyy; hh:mm:ss.cc"
	unsigned char i = 0, j;

	while(S3Data->m_ImageID[i] != ';')
	{
		S3Data->m_ImageOS[i] = S3Data->m_ImageID[i];
		i++;
	}

	i += 2;
	j = 0;
	char k = 0;

	while(S3Data->m_ImageID[i] != ';')
	{
		// Remove the century 20xy -> xy
		if (k != 6 && k != 7)
		{
			S3Data->m_ImageDate[j] = S3Data->m_ImageID[i];
			j++;
		}
		
		k++; i++;
	}

	i += 2;
	j = 0;

	// Check for single-digit hour and make "%02d"
	if (S3Data->m_ImageID[i] == ' ')
	{
		S3Data->m_ImageTime[j++] = '0';
		i++;
	}

	while(S3Data->m_ImageID[i] != '\0')
	{
		S3Data->m_ImageTime[j] = S3Data->m_ImageID[i];
		i++; j++;
	}

	// Remove seconds/hundredths
	while(--j && S3Data->m_ImageTime[j] != ':');

	S3Data->m_ImageTime[j] = '\0';

#endif
	return 0;
}

// ----------------------------------------------------------------------------

bool S3OSRestartRequest()
{
	// Are you sure?
	S3EventLogAdd("Restart requested", 1, -1, -1, -1);

	return true;
}

// ----------------------------------------------------------------------------
// Must only be called once to avoid resetting watchdog timer.

int S3OSRestart()
{
	S3SetFactoryMode(-1, -1, true);

	S3EventLogAdd("Restarting system", 1, -1, -1, -1);

#ifdef TRIZEPS

	BOOL ok = WriteRegistry();

	if (!ok)
	{
		S3EventLogAdd("Failed to write registry", 3, -1, -1, -1);
		return 1;
	}

	// Set watchdog timer.
	ResetAfter_us(3 * 1000000, TRUE); // Clean restart
#endif

	return 0;
}

// ----------------------------------------------------------------------------

int S3OSSWUpdateRequest()
{
	S3EventLogAdd("OS image update requested", 1, -1, -1, -1);

	DWORD fileAtt = GetFileAttributes(OSHDDDirectory);

	if (fileAtt == INVALID_FILE_ATTRIBUTES ||
								(fileAtt & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		S3EventLogAdd("No USB HDD found", 1, -1, -1, -1);
		return 1;
	}

	BOOL exist = S3FileExist(OSImageUpdateFilePath);
	
	if (!exist)
	{
		S3EventLogAdd("No OS image found", 1, -1, -1, -1);
		return 2;
	}

	S3EventLogAdd("OS image update request OK", 1, -1, -1, -1);

/*
	// TODO: What does this actually check... and why doesn't it work?
	BOOL verified = VerifyImage(OSImageUpdateFilename);
	
	if (!verified)
		return 2;
*/
	return 0;

}

// ----------------------------------------------------------------------------

int S3OSAppUpdateRequest()
{
	S3EventLogAdd("Exe update requested", 1, -1, -1, -1);

	DWORD fileAtt = GetFileAttributes(OSHDDDirectory);

	if (fileAtt == INVALID_FILE_ATTRIBUTES ||
								(fileAtt & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		S3EventLogAdd("No USB HDD found", 1, -1, -1, -1);
		return 1;
	}

	if (!S3FileExist(OSAppUpdateFilePath))
	{
		S3EventLogAdd("No exe found", 1, -1, -1, -1);
		return 2;
	}

	S3EventLogAdd("Exe update request OK", 1, -1, -1, -1);

	return 0;
}

// ----------------------------------------------------------------------------

int S3OSTest()
{
	return 0;
}

// ----------------------------------------------------------------------------

void SystemShutdown(void)
{
	S3EventLogAdd("Hard shutdown", 1, -1, -1, -1);

#ifdef TRIZEPS
	SetOperatingPoint(0); //   DWORD corefreq
#endif // TRIZEPS
}

// ----------------------------------------------------------------------------

int S3LogFileToUSBRequest()
{
	S3EventLogAdd("Log file copy requested", 1, -1, -1, -1);

	DWORD fileAtt = GetFileAttributes(OSHDDDirectory);

	if (fileAtt == INVALID_FILE_ATTRIBUTES ||
								(fileAtt & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		S3EventLogAdd("No USB HDD found", 1, -1, -1, -1);
		return 1;
	}

	char FullPathName[S3_MAX_FILENAME_LEN];

	sprintf_s(FullPathName, S3_MAX_FILENAME_LEN, "%s\\%s.s3l",
		S3Data->m_EventLogPath, S3Data->m_EventLogName);

	FILE *fid;
	errno_t err = fopen_s(&fid, FullPathName, "rb");

	if (err)
	{
		S3EventLogAdd("Failed to find log file to transfer", 1, -1, -1, -1);
		return 2;
	}

	fclose(fid);

	sprintf_s(FullPathName, S3_MAX_FILENAME_LEN, "%S\\%s.s3l",
		OSHDDDirectory, S3Data->m_EventLogName);

	err = fopen_s(&fid, FullPathName, "rb");

	if (!err)
	{
		S3EventLogAdd("Log file already exists on USB drive", 1, -1, -1, -1);
		fclose(fid);
		return 3;
	}

	S3LogFileToUSB();

	return 0;
}

// ----------------------------------------------------------------------------
// S3LogFileToUSBRequest has been called

int S3LogFileToUSB()
{
	wchar_t src[S3_MAX_FILENAME_LEN];
	wchar_t dest[S3_MAX_FILENAME_LEN];

	swprintf_s(src, S3_MAX_FILENAME_LEN, _T("%S\\%S.s3l"),
		S3Data->m_EventLogPath, S3Data->m_EventLogName);

	swprintf_s(dest, S3_MAX_FILENAME_LEN, _T("%s\\%S.s3l"),
		OSHDDDirectory, S3Data->m_EventLogName);

	if (!CopyFile(src, dest, false))
	{
		S3EventLogAdd("Copy of log file to USB drive failed", 3, -1, -1, -1);
		return 1;
	}

	S3EventLogAdd("Copy of log file to USB drive successful", 1, -1, -1, -1);

	return 0;
}

// ----------------------------------------------------------------------------

int S3SysReadSN()
{
	FILE *fid;
	errno_t err = fopen_s(&fid, S3Data->m_SNFileName, "r");

	if (!err)
	{
		char buf[S3_MAX_SN_LEN];

		char *str = fgets(buf, S3_MAX_SN_LEN, fid);
		fclose(fid);

		// TODO: Sanity check buf
		if (str == buf)
		{
			strcpy_s(S3Data->m_SN, S3_MAX_SN_LEN, buf);

			// TODO: No way to test interface board version,
			// so use system serial numbers - not very satisfactory.

			if (!strcmp(S3Data->m_SN, "1247108")) // CEA
			{
				S3Data->m_SoftShutdownOption = false;
			}
			else if (!strcmp(S3Data->m_SN, "1247843")) // ATiS
			{
				S3Data->m_SoftShutdownOption = false;
			}
			else if (!strcmp(S3Data->m_SN, "DEMO000")) // Demo1
			{
				S3Data->m_SoftShutdownOption = false;
			}
			else // Eng0002 and all future S3s
			{
				S3Data->m_SoftShutdownOption = true;
			}
		}
		else
		{
			strcpy_s(S3Data->m_SN, S3_MAX_SN_LEN, "Unknown");
			return 1;
		}
	}
	else
	{
		strcpy_s(S3Data->m_SN, S3_MAX_SN_LEN, "Unknown");
		return 1;
	}

	return 0;
}

// ----------------------------------------------------------------------------

int S3SysWriteSN()
{
	FILE *fid;
	errno_t err = fopen_s(&fid, S3Data->m_SNFileName, "w");

	if (!err)
	{
		fprintf(fid, "%s", S3Data->m_SN);
		fclose(fid);

		return 0;
	}

	return 1;
}

// ----------------------------------------------------------------------------

int S3SysReadPN()
{
	FILE *fid;
	errno_t err = fopen_s(&fid, S3Data->m_PNFileName, "r");

	if (!err)
	{
		char buf[S3_MAX_PN_LEN];

		char *str = fgets(buf, S3_MAX_PN_LEN, fid);
		fclose(fid);

		// TODO: Sanity check buf
		if (str == buf)
			strcpy_s(S3Data->m_PN, S3_MAX_PN_LEN, buf);
		else
		{
			strcpy_s(S3Data->m_PN, S3_MAX_PN_LEN, "Unknown");
			return 1;
		}
	}
	else
	{
		strcpy_s(S3Data->m_PN, S3_MAX_PN_LEN, "Unknown");
		return 1;
	}

	return 0;
}

// ----------------------------------------------------------------------------

int S3SysWritePN()
{
	FILE *fid;
	errno_t err = fopen_s(&fid, S3Data->m_PNFileName, "w");

	if (!err)
	{
		fprintf(fid, "%s", S3Data->m_PN);
		fclose(fid);

		return 0;
	}

	return 1;
}

// ----------------------------------------------------------------------------
// set/cancel

extern int S3PowerDownTimeout;

int S3SetPowerDownPending(bool set)
{
	if (set == false)
		S3PowerDownTimeout = 0;
	
	S3Data->m_PowerDownPending = set;
			
	return 0;
}

// ----------------------------------------------------------------------------

bool S3GetPowerDownPending()
{
	return S3Data->m_PowerDownPending;
}

// ----------------------------------------------------------------------------

int S3SetPowerDownFailed(bool set)
{
	S3Data->m_PowerDownFailed = set;
			
	return 0;
}

// ----------------------------------------------------------------------------

bool S3GetPowerDownFailed()
{
	return S3Data->m_PowerDownFailed;
}

// ----------------------------------------------------------------------------
// set/cancel

extern int S3CloseAppTimeout;

int S3SetCloseAppPending(bool set)
{
	if (set == false)
		S3CloseAppTimeout = 0;
	
	S3Data->m_CloseAppPending = set;
			
	return 0;
}

// ----------------------------------------------------------------------------

bool S3GetCloseAppPending()
{
	return S3Data->m_CloseAppPending;
}

// ----------------------------------------------------------------------------

int S3SetCloseAppFailed(bool set)
{
	S3Data->m_CloseAppFailed = set;
			
	return 0;
}

// ----------------------------------------------------------------------------

bool S3GetCloseAppFailed()
{
	return S3Data->m_CloseAppFailed;
}

// ----------------------------------------------------------------------------
// Format the app build date/time macros to be the same as the OS image
// date/time.
// Shouldn't really be done at run-time, but wtf...

int S3SetAppDateTime()
{
	char			AppDateTime[S3_DATETIME_LEN];

	int				day, year, imonth;
	char			month[20];

	strcpy_s(AppDateTime, S3_DATETIME_LEN, __DATE__);

	sscanf_s(AppDateTime, "%s %d %d", month, 20, &day, &year);

    if      (!strcmp(month, "Jan" )) imonth = 1;
    else if (!strcmp(month, "Feb" )) imonth = 2;
    else if (!strcmp(month, "Mar" )) imonth = 3;
    else if (!strcmp(month, "Apr" )) imonth = 4;
    else if (!strcmp(month, "May" )) imonth = 5;
    else if (!strcmp(month, "Jun" )) imonth = 6;
    else if (!strcmp(month, "Jul" )) imonth = 7;
    else if (!strcmp(month, "Aug" )) imonth = 8;
    else if (!strcmp(month, "Sep" )) imonth = 9;
    else if (!strcmp(month, "Oct" )) imonth = 10;
    else if (!strcmp(month, "Nov" )) imonth = 11;
    else if (!strcmp(month, "Dec" )) imonth = 12;
    else imonth = 0;

	strcpy_s(AppDateTime, S3_DATETIME_LEN, __TIME__);
	AppDateTime[5] = '\0'; // Trim s from hh:mm:ss

	sprintf_s(S3Data->m_AppDateTime, S3_DATETIME_LEN, "%02d/%02d/%02d %s",
		day, imonth, year - 2000, AppDateTime);

	return 0;
}

// ----------------------------------------------------------------------------

int S3BackupConfig()
{
	wchar_t src[S3_MAX_FILENAME_LEN];
	wchar_t dest[S3_MAX_FILENAME_LEN];

	swprintf_s(src, S3_MAX_FILENAME_LEN, _T("%S\\%S.s3c"),
		S3Data->m_ConfigPath, S3Data->m_ConfigName);

	swprintf_s(dest, S3_MAX_FILENAME_LEN, _T("%S\\%S.bak"),
		S3Data->m_ConfigPath, S3Data->m_ConfigName);

	if (!CopyFile(src, dest, false))
	{
		S3EventLogAdd("Backup of config file failed", 3, -1, -1, -1);
		return 1;
	}

	return 0;
}

// ----------------------------------------------------------------------------

int S3GetScreenOffsets(short *x, short *y)
{
	if (S3Data->m_ScrnOSx == SHRT_MIN || S3Data->m_ScrnOSy == SHRT_MIN)
	{
		*x = 0;
		*y = 0;

		return 1;
	}
	else
	{
		*x = S3Data->m_ScrnOSx;
		*y = S3Data->m_ScrnOSy;
	}

	return 0;
}

// ---------------------------------------------------------------------------

int S3WriteScreenOffsets(short x, short y)
{
	FILE *fid;
	errno_t err = fopen_s(&fid, S3Data->m_ScreenOffsetFileName, "wb");

	if (!err)
	{
		short buf[2];

		S3Data->m_ScrnOSx = buf[0] = x;
		S3Data->m_ScrnOSx = buf[1] = y;

		err = fwrite(buf, sizeof(short), 2, fid);

		fclose(fid);

		return 0;
	}

	return 0;
}

// ---------------------------------------------------------------------------

int S3ReadScreenOffsets()
{
	FILE *fid;
	errno_t err = fopen_s(&fid, S3Data->m_ScreenOffsetFileName, "rb");

	if (!err)
	{
		short buf[2];

		err = fread(buf, sizeof(short), 2, fid);
		fclose(fid);

		if ((buf[0] > -16 && buf[0] < 16) && (buf[1] > -16 && buf[1] < 16))
		{
			S3Data->m_ScrnOSx = buf[0];
			S3Data->m_ScrnOSy = buf[1];

			return 0;
		}
	}
	else
	{
		// CEA values
		S3Data->m_ScrnOSx = 0;
		S3Data->m_ScrnOSy = 16;
	}

	return 1;
}

// ---------------------------------------------------------------------------
// Unlock/lock 'factory' screen functions. Look for S3Unlock.s3k file on
// USB HDD, otherwise look for S3Lock.s3k on \Flashdisk.

int S3GetLockFile()
{
	FILE *fid;
	errno_t err = fopen_s(&fid, S3Data->m_UnlockFileName, "r");

	if (!err)
	{
		S3Data->m_Locked = false;
		fclose(fid);

		S3EventLogAdd("System unlocked via USB HDD unlock file",
														3, -1, -1, -1);

		err = fopen_s(&fid, S3Data->m_LockFileName, "r");

		if (!err)
		{
			fclose(fid);

			wchar_t FileName[S3_MAX_FILENAME_LEN];

			swprintf_s(FileName, S3_MAX_FILENAME_LEN, L"%S",
					S3Data->m_LockFileName);

			DeleteFile(FileName);
		}
	}
	else
	{	
		errno_t err = fopen_s(&fid, S3Data->m_LockFileName, "r");

		if (!err)
		{
			S3Data->m_Locked = true;
			fclose(fid);
		}
		else
		{
			S3Data->m_Locked = false;
		}
	}

	return 0;
}

// ---------------------------------------------------------------------------

int S3SetLockFile()
{
	FILE *fid;
	errno_t err = fopen_s(&fid, S3Data->m_LockFileName, "w");

	if (!err)
	{
		fclose(fid);
		S3EventLogAdd("System lock file created", 3, -1, -1, -1);
	}
	else
	{
		S3EventLogAdd("System lock file creation failed", 3, -1, -1, -1);
		return 1;
	}

	return 0;
}

// ---------------------------------------------------------------------------

bool S3GetLocked()
{
	return S3Data->m_Locked;
}

int S3SetTelnetAuthRegKey(DWORD data);

// ---------------------------------------------------------------------------

int S3SetLocked()
{
	int err = 0;
	
	S3Data->m_Locked = true;

	// Require authentication
	if (S3SetTelnetAuthRegKey(1))
		err = 1;

#ifdef TRIZEPS
	BOOL ok = WriteRegistry();

	if (!ok)
	{
		S3EventLogAdd("S3SetLocked: Failed to write registry", 3, -1, -1, -1);
		err = 2;
	}
#endif

	if (S3SetLockFile())
		err = 3;

	return err;
}

// ---------------------------------------------------------------------------

int S3SetTelnetAuthRegKey(DWORD data)
{
#ifdef TRIZEPS

	HKEY	hKey;
	int		err;

	err = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Comm\\TELNETD"), 0, 
											KEY_SET_VALUE, &hKey);

	if (err == ERROR_SUCCESS)
	{
		err = RegSetValueEx(hKey, _T("UseAuthentication"), 0,
			REG_DWORD, (BYTE *)(&data), sizeof(DWORD));

		if (err != ERROR_SUCCESS)
		{
			S3EventLogAdd("Failed to set telnetd registry key", 1, -1, -1, -1);
			return 1;
		}
	}
	else
	{
		S3EventLogAdd("Failed to open telnetd registry key", 1, -1, -1, -1);
		return 1;
	}

#endif

	return 0;
}

// ---------------------------------------------------------------------------


