// Utilities not attached to particular data

#ifndef TRIZEPS
#include "S3ControllerX86/targetver.h"
#else
#define WINVER _WIN32_WCE
#include <ceconfig.h>
#endif

#include <afxpriv.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <float.h>
#include <math.h>

#include <windows.h>

#include "S3SystemDetails.h"
#include "S3DataModel.h"
#include "S3GPIB.h"
#include "S3I2C.h"
#include "S3Gain.h"

extern pS3DataModel S3Data;

// ----------------------------------------------------------------------------

#define S3_MAX_PATH_STRING_LEN	32
char S3PathString[S3_MAX_PATH_STRING_LEN];

int S3GetPathStr(char Rx, char Tx, char IP, char **str)
{
	if (Rx == -1)
		strcpy_s(S3PathString, S3_MAX_PATH_STRING_LEN, "System");
	else if (Tx == -1)
		sprintf_s(S3PathString, S3_MAX_PATH_STRING_LEN, "Rx%d", Rx + 1);
	else if (IP == -1)
		sprintf_s(S3PathString, S3_MAX_PATH_STRING_LEN, "Rx%d/Tx%d",
						Rx + 1, Tx + 1);
	else
		sprintf_s(S3PathString, S3_MAX_PATH_STRING_LEN, "Rx%d/Tx%d/RF%d",
						Rx + 1, Tx + 1, IP + 1);

	*str = S3PathString;

	return 0;
}

// ----------------------------------------------------------------------------

int S3GetSelPathStr(char **str)
{
	return S3GetPathStr(S3Data->m_SelectedRx, S3Data->m_SelectedTx, -1, str);
}

// ----------------------------------------------------------------------------

char *S3GetPathStrStr(unsigned char *cData)
{
	if (cData[0] == 0)
		strcpy_s(S3PathString, S3_MAX_PATH_STRING_LEN, "System");
	else if (cData[1] == 0)
		sprintf_s(S3PathString, S3_MAX_PATH_STRING_LEN, "Rx%d", cData[0]);
	else if (cData[2] == 0)
		sprintf_s(S3PathString, S3_MAX_PATH_STRING_LEN, "Rx%d/Tx%d",
						cData[0], cData[1]);
	else
		sprintf_s(S3PathString, S3_MAX_PATH_STRING_LEN, "Rx%d/Tx%d/RF%d",
						cData[0], cData[1], cData[2]);

	return S3PathString;
}


// ---------------------------------------------------------------------------

// Extract integer from any leading/trailing non-numeric characters

int S3ExtractSN(int *iSN, const char *SN)
{
	int	len = strlen(SN);
	int	idx = 0;
	int start, end;

	char extract[S3_MAX_SN_LEN];

	*extract = '\0';
	*iSN = INT_MIN;		// No match

	if (!len)
		return 0;

	while(!isdigit(SN[idx++]) && idx < len);

	start = idx - 1;

	while(isdigit(SN[idx++]) && idx < len);

	end = idx;

	strncpy_s(extract, S3_MAX_SN_LEN, SN + start, end - start);

	*(extract + end - start) = '\0';

	if (end - start > 0)
	{
		*iSN = atoi(extract);
	}

	return 0;
}

// ---------------------------------------------------------------------------

bool S3FileExist(const wchar_t *FilePath)
{
	FILE *f;
	errno_t	err;

	err = _wfopen_s(&f, FilePath, _T("rb"));

	if (err)
	{
		return false;
	}

	fclose(f);

	return true;
}

// ----------------------------------------------------------------------------

bool S3DirExist(const wchar_t *dir)
{
	DWORD fileAtt = GetFileAttributes(dir);

	if (	fileAtt == INVALID_FILE_ATTRIBUTES ||
			(fileAtt & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------------

