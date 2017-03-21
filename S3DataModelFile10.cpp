 // Version 1.0 of the S3DataModel file utilities

// DO NOT USE INDEXING ACCESSORS - THESE FNS ACTS ON A NEW MEMORY BLOCK THAT
// OVER-WRITES THE MAIN BLOCK (AND ANY CHANGES MADE USING THE STANDARD
// ACCESSORS)

#include "windows.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include "S3DataModel.h"

extern	pS3DataModel S3Shadow;

// Shouldn't have used double....
#define MATCH_VERSION(A, B) (fabs((A) - (B)) < 0.000001 ? 1 : 0)
#define LATER_VERSION(A, B) (((fabs((A) - (B)) < 0.000001) || ((A) > (B))) ? 1 : 0)

// ----------------------------------------------------------------------------

int S3Save(const char *Filename)
{
	// System save
	if (Filename == NULL)
	{
		char SysFilename[S3_MAX_FILENAME_LEN];

		sprintf_s(SysFilename, S3_MAX_FILENAME_LEN, "%s\\%s.s3c",
			S3Shadow->m_ConfigPath, S3Shadow->m_ConfigName);

		wchar_t AutoSaveFileName[S3_MAX_FILENAME_LEN];

		swprintf_s(AutoSaveFileName, S3_MAX_FILENAME_LEN, L"%S\\S3AutoSave.flg",
					S3Shadow->m_ConfigPath);

		BOOL ok = DeleteFile(AutoSaveFileName);

		if (!ok)
		{
			S3EventLogAdd("S3Save: Failed to delete AutoSave flag file", 1, -1, -1, -1);
			// return 1;
		}

		int err = S3Save2(SysFilename);
		
		if (err)
			return err;
		else
		{
			FILE	*fid;
			err = _wfopen_s(&fid, AutoSaveFileName, L"w");
			if (!err)
			{
				fclose(fid);
				return 0;
			}
			else
				S3EventLogAdd("S3Save: Failed to create AutoSave flag file", 1, -1, -1, -1);
		}
	}

	// User save
	int err = S3Save2(Filename);

	if (!err) 
	{
		S3UpdateConfigName(Filename);
		S3Shadow->m_Modified = false;
	}

	return err;
}

// ----------------------------------------------------------------------------

int S3Read(const char *Filename)
{
	// System read
	if (Filename == NULL)
	{
		char SysFilename[S3_MAX_FILENAME_LEN];

		sprintf_s(SysFilename, S3_MAX_FILENAME_LEN, "%s\\%s.s3c",
			S3Shadow->m_ConfigPath, S3Shadow->m_ConfigName);

		int err = S3Read2(SysFilename);

		return err;
	}

	// User load
	int err = S3Read2(Filename);
	 
	if (!err)
		S3UpdateConfigName(Filename);

	return err;
}

// ----------------------------------------------------------------------------
// Split Filename into path and file

int S3UpdateConfigName(const char *Filename)
{
	S3GetPathFile(S3Shadow->m_ConfigPath, S3Shadow->m_ConfigName, Filename);

	// Suffix is ignored
	S3EventLogInit(Filename);

	return 0;
}

// ----------------------------------------------------------------------------
/*
int S3SaveStruct(const char *Filename)
{
	FILE	*fid;
	errno_t	err;

	strcpy_s(S3Shadow->m_ConfigPath, S3_MAX_FILENAME_LEN, Filename);

	if (S3Shadow == NULL)
		return 1;

	err = fopen_s(&fid, Filename, "wb");

	if (err)
		return 2;

	fwrite(S3Shadow, sizeof(S3DataModel), 1, fid);

	fclose(fid);

	return 0;
}

// ----------------------------------------------------------------------------

int S3ReadStruct(const char *Filename)
{
	if (S3Shadow == NULL)
		return 1;

	FILE	*fid;
	errno_t	err;

	err = fopen_s(&fid, Filename, "rb");

	if (err)
		return 2;

	fread_s(S3Shadow, sizeof(S3DataModel), sizeof(S3DataModel), 1, fid);

	fclose(fid);

	return 0;
}
*/

// ----------------------------------------------------------------------------

int S3SaveConfig(FILE *fid, S3Config *src)
{
	fwrite(src, sizeof(S3Config), 1, fid);

	return 0;
}

// ----------------------------------------------------------------------------

int S3ReadConfig(FILE *fid, S3Config *src)
{
	fread((void *)src, sizeof(S3Config), 1, fid);

	if (src->m_Gain > S3_MAX_GAIN || src->m_Gain < S3_MIN_GAIN)
		src->m_Gain = 0;

	return 0;
}

// ----------------------------------------------------------------------------

int S3Save2(const char *Filename)
{
	FILE	*fid;
	errno_t	err;
	pS3DataModel pSys;

	if (S3Shadow == NULL)
		return 1;

	pSys = S3Shadow;

	err = fopen_s(&fid, Filename, "wb");

	if (err)
		return 2;

	double Vfile = S3_FILE_VERSION;

	// fwrite(&(pSys->m_FileVersion), sizeof(double), 1, fid);
	fwrite(&Vfile, sizeof(double), 1, fid);

	fwrite(pSys->m_NodeName, sizeof(char), S3_MAX_NODE_NAME_LEN, fid);
	S3SaveConfig(fid, &(pSys->m_Config));
	S3SysSave(fid, pSys);

	for (char Rx = 0; Rx < S3_MAX_RXS; Rx++)
	{
		S3RxData	*pRx = &(pSys->m_Rx[Rx]);

		fwrite(pRx->m_NodeName, sizeof(char), S3_MAX_NODE_NAME_LEN, fid);
		S3SaveConfig(fid, &(pRx->m_Config));
		S3RxSave(fid, pRx);

		for (char Tx = 0; Tx < S3_MAX_TXS; Tx++)
		{
			pS3TxData pTx = &(pRx->m_Tx[Tx]);

			fwrite(pTx->m_NodeName, sizeof(char), S3_MAX_NODE_NAME_LEN, fid);
			S3SaveConfig(fid, &(pTx->m_Config));
			S3TxSave(fid, pTx);

			for (char IP = 0; IP < S3_MAX_IPS; IP++)
			{
				// Inheritables (Actual input parameters)
				pS3IPData pIP = &pTx->m_Input[IP];
				fwrite(pIP->m_NodeName, sizeof(char), S3_MAX_NODE_NAME_LEN, fid);
				S3SaveConfig(fid, &(pIP->m_Config));
				S3IPSave(fid, pIP);
			}
		}
	}

	fclose(fid);

	return 0;
}

// ----------------------------------------------------------------------------
S3DataModel		readModel; // Too big for stack

int S3Read2(const char *Filename)
{
	FILE	*fid;
	errno_t	err;
	// S3DataModel		readModel;
	pS3DataModel	pSys;

	if (S3Shadow == NULL)
		return 1;

	memcpy(&readModel, S3Shadow, sizeof(S3DataModel));

	pSys = &readModel;

	err = fopen_s(&fid, Filename, "rb");

	if (err)
		return 2;

	// TODO: Move to version-independent read function
	fread(&(pSys->m_FileVersion), sizeof(double), 1, fid);

	//if (pSys->m_FileVersion != S3_FILE_VERSION)
	//{
	//	fclose(fid);
	//	return 3;
	//}

	fread(pSys->m_NodeName, sizeof(char), S3_MAX_NODE_NAME_LEN, fid);
	// TEMP: For bad data in config files
	if (!isascii(pSys->m_NodeName[0]))
		pSys->m_NodeName[0] = '\0';

	if (pSys->m_NodeName[S3_MAX_NODE_NAME_LEN - 1] != '\0')
		pSys->m_NodeName[S3_MAX_NODE_NAME_LEN - 1] = '\0';

	S3ReadConfig(fid, &(pSys->m_Config));
	S3SysRead(fid, pSys);

	for (char Rx = 0; Rx < S3_MAX_RXS; Rx++)
	{
		S3RxData	*pRx = &(pSys->m_Rx[Rx]);

		fread(pRx->m_NodeName, sizeof(char), S3_MAX_NODE_NAME_LEN, fid);

		// TEMP:
		if (!isascii(pRx->m_NodeName[0]))
			pRx->m_NodeName[0] = '\0';

		if (pRx->m_NodeName[S3_MAX_NODE_NAME_LEN - 1] != '\0')
			pRx->m_NodeName[S3_MAX_NODE_NAME_LEN - 1] = '\0';

		S3ReadConfig(fid, &(pRx->m_Config));
		S3RxRead(fid, pRx);

		for (char Tx = 0; Tx < S3_MAX_TXS; Tx++)
		{
			pS3TxData pTx = &(pRx->m_Tx[Tx]);

			fread(pTx->m_NodeName, sizeof(char), S3_MAX_NODE_NAME_LEN, fid);
				
			// TEMP:
			if (!isascii(pTx->m_NodeName[0]))
				pTx->m_NodeName[0] = '\0';

			if (pTx->m_NodeName[S3_MAX_NODE_NAME_LEN - 1] != '\0')
				pTx->m_NodeName[S3_MAX_NODE_NAME_LEN - 1] = '\0';

			S3ReadConfig(fid, &(pTx->m_Config));
			S3TxRead(fid, pTx);

			for (char IP = 0; IP < S3_MAX_IPS; IP++)
			{
				// Inheritables (Actual input parameters)
				pS3IPData pIP = &pTx->m_Input[IP];

				fread(pIP->m_NodeName, sizeof(char), S3_MAX_NODE_NAME_LEN, fid);
	
				S3ReadConfig(fid, &(pIP->m_Config));
				pIP->m_PrevZ = pIP->m_Config.m_InputZ;
				S3IPRead(fid, pIP);

				pIP->m_MaxInput = S3CalcMaxIP(pIP->m_Config.m_Gain);
				pIP->m_P1dB = S3CalcP1dB(pIP->m_Config.m_Gain);
				
				// TEMP: 
				if (!isascii(pIP->m_NodeName[0]))
					pIP->m_NodeName[0] = '\0';

				if (pIP->m_NodeName[S3_MAX_NODE_NAME_LEN - 1] != '\0')
					pIP->m_NodeName[S3_MAX_NODE_NAME_LEN - 1] = '\0';
				
				pIP->m_Config.m_LowNoiseMode = false;
			}
		}
	}

	fclose(fid);

	pSys->m_AGC = pSys->m_Rx[0].m_AGC[0];

	// TODO: Could be done by just swapping pointers
	// All good, so copy. 
	memcpy(S3Shadow, &readModel, sizeof(S3DataModel));

	// Make sure shadow updated
	S3DbgPollInit();

	// TODO: We may try and set Tx active with invalidated battery
	// for (char Rx = 0; Rx < S3_MAX_RXS; Rx++)
	// 	S3RxSetActiveTx(Rx, 0);

	return 0;
}

// ----------------------------------------------------------------------------

int S3SysSave(FILE *fid, pS3DataModel Sys)
{
	fwrite(&Sys->m_DisplayUnits, sizeof(unsigned char), 1, fid);
	fwrite(&Sys->m_ContTComp, sizeof(unsigned char), 1, fid);

	if (LATER_VERSION(S3_FILE_VERSION, 1.6))
	{
		fwrite(&(Sys->m_TxStartState), sizeof(unsigned char), 1, fid);
		fwrite(&(Sys->m_AGC), sizeof(unsigned char), 1, fid);
	}

	return 0;
}

// ----------------------------------------------------------------------------

int S3SysRead(FILE *fid, pS3DataModel pSys)
{
	fread(&pSys->m_DisplayUnits, sizeof(unsigned char), 1, fid);
	fread(&pSys->m_ContTComp, sizeof(unsigned char), 1, fid);

	if (LATER_VERSION(readModel.m_FileVersion, 1.6))
	{
		fread(&pSys->m_TxStartState, sizeof(unsigned char), 1, fid);
		fread(&pSys->m_AGC, sizeof(unsigned char), 1, fid);
	}
	
	return 0;
}

// ----------------------------------------------------------------------------

int S3RxSave(FILE *fid, pS3RxData Rx)
{
	fwrite(&(Rx->m_Type),	sizeof(unsigned char), 1, fid);
	
	if (LATER_VERSION(S3_FILE_VERSION, 1.8))
		fwrite(&(Rx->m_ActiveTx),	sizeof(char), 1, fid);
	else
		fwrite(&(Rx->m_AGC),	sizeof(unsigned char), 1, fid);

	fwrite(Rx->m_SN,		sizeof(char), S3_MAX_SN_LEN, fid);
	fwrite(Rx->m_PN,		sizeof(char), S3_MAX_PN_LEN, fid);
	fwrite(Rx->m_FW,		sizeof(char), S3_MAX_SW_VER_LEN, fid);

	return 0;
}

// ----------------------------------------------------------------------------

int S3RxRead(FILE *fid, pS3RxData pRx)
{
	unsigned char Type;

	fread(&Type, sizeof(unsigned char), 1, fid);
	S3RxSetType(pRx, Type);

	if (LATER_VERSION(readModel.m_FileVersion, 1.8))
	{
		fread(&pRx->m_ActiveTx, sizeof(char), 1, fid);

		// Sanity limiting
		if (pRx->m_ActiveTx >= 100)
			pRx->m_ActiveTx -= 100;

		if (pRx->m_ActiveTx < 0 || pRx->m_ActiveTx > S3_MAX_TXS)
			pRx->m_ActiveTx = 0;
	}
	else
		fread(&pRx->m_AGC, sizeof(unsigned char), 1, fid);

	fread(pRx->m_SN, sizeof(char), S3_MAX_SN_LEN, fid);	// PPM serial no.
	fread(pRx->m_PN, sizeof(char), S3_MAX_PN_LEN, fid);
	fread(pRx->m_FW, sizeof(char), S3_MAX_SW_VER_LEN, fid);

	return 0;
}

// ----------------------------------------------------------------------------

int S3TxSave(FILE *fid, pS3TxData Tx)
{
	fwrite(&(Tx->m_Type), sizeof(unsigned char), 1, fid);
	fwrite(&(Tx->m_ActiveInput), sizeof(char), 1, fid);
	fwrite(Tx->m_SN, sizeof(char), S3_MAX_SN_LEN, fid);	// PPM serial no.
	fwrite(Tx->m_PN, sizeof(char), S3_MAX_PN_LEN, fid);
	fwrite(Tx->m_FW, sizeof(char), S3_MAX_SW_VER_LEN, fid);

	fwrite(&Tx->m_PowerStat, sizeof(unsigned char), 1, fid);

	if (LATER_VERSION(S3_FILE_VERSION, 1.5))
	{
		fwrite(&(Tx->m_UserSleep), sizeof(bool), 1, fid);
	}

	return 0;
}

// ----------------------------------------------------------------------------

int S3TxRead(FILE *fid, pS3TxData pTx)
{
	unsigned char Type;

	fread(&Type, sizeof(unsigned char), 1, fid);

	fread(&pTx->m_ActiveInput, sizeof(char), 1, fid);

	fread(pTx->m_SN, sizeof(char), S3_MAX_SN_LEN, fid);
	fread(pTx->m_PN, sizeof(char), S3_MAX_PN_LEN, fid);
	fread(pTx->m_FW, sizeof(char), S3_MAX_SW_VER_LEN, fid);
	
	fread(&pTx->m_PowerStat, sizeof(unsigned char), 1, fid);

	if (LATER_VERSION(readModel.m_FileVersion, 1.5))
	{
		fread(&pTx->m_UserSleep, sizeof(bool), 1, fid);
	}

	// Clear any pending sleep state
	if (pTx->m_PowerStat >= 100)
		pTx->m_PowerStat -= 100;

	if (pTx->m_PowerStat < 1)
		pTx->m_PowerStat = S3_TX_ON;

	if (!S3GetDemoMode())
	{
		if (1) // pTx->m_PowerStat != S3_TX_SLEEP)
			S3TxSetTypeP(pTx, S3_TxUnconnected);
		else
			S3TxSetTypeP(pTx, Type);
	}
	else
	{
		// Allow Tx 0/0 to be live
		if (pTx->m_ParentId == 0 && pTx->m_Id == 0)
			S3TxSetTypeP(pTx, S3_TxUnconnected);
		else
			S3TxSetTypeP(pTx, Type);
	}

	return 0;
}

// ----------------------------------------------------------------------------

/*
int S3RxReadId(FILE *fid, char Rx)
{
	unsigned char Type;

	pS3RxData pRx = &readModel.m_Rx[Rx];

	fread(&Type, sizeof(unsigned char), 1, fid);
	S3RxSetType(Rx, Type);

	fread(pRx->m_SN, sizeof(char), S3_MAX_SN_LEN, fid);	// PPM serial no.
	fread(pRx->m_PN, sizeof(char), S3_MAX_PN_LEN, fid);
	fread(pRx->m_FW, sizeof(char), S3_MAX_SW_VER_LEN, fid);

	return 0;
}
*/

// ----------------------------------------------------------------------------

int S3IPSave(FILE *fid, pS3IPData IP)
{
	if (LATER_VERSION(S3_FILE_VERSION, 1.4))
	{
		fwrite(&(IP->m_TestToneEnable), sizeof(char), 1, fid);
	}

	return 0;
}

// ----------------------------------------------------------------------------

int S3IPRead(FILE *fid, pS3IPData IP)
{
	if (LATER_VERSION(readModel.m_FileVersion, 1.4))
	{
		fread(&(IP->m_TestToneEnable), sizeof(char), 1, fid);

		if (IP->m_TestToneEnable < 0 || IP->m_TestToneEnable > 1)
			IP->m_TestToneEnable = 0;
	}

	// TODO: += 100 to schedule for update?

	return 0;
}

// ----------------------------------------------------------------------------
// Split Filename ino Path and File parts. If no path part is found in
// Filename, any Path contents will be safe (otherwise they will be overwritten
// with a new path). 

int S3GetPathFile(char *Path, char *File, const char *Filename)
{
	char IntFile[S3_MAX_FILENAME_LEN];

	// Find last file delimiter ----
	const char *slash = Filename, *next;

	while ((next = strpbrk(slash + 1, "\\/")))
		slash = next;

	if (Filename != slash)
		strcpy_s(IntFile, S3_MAX_CFG_NAME_LEN, slash + 1);
	else
	{
		strcpy_s(IntFile, S3_MAX_CFG_NAME_LEN, Filename); // no path
		slash = NULL;
	}

	// Remove any suffix (after first '.' from end)
	unsigned char lb = strlen(IntFile); // length with suffix
	unsigned char l = lb - 1;
	while (l && IntFile[l] != '.') l--;
	if (l)
		IntFile[l] = '\0';

	if (File != NULL)
	{
		strcpy_s(File, S3_MAX_CFG_NAME_LEN, IntFile);
	}

	if (Path != NULL && slash != NULL)
	{
		// Get Path ---- 

		// Copy up to filename
		strncpy_s(Path, S3_MAX_FILENAME_LEN, Filename,
			strlen(Filename) - lb);

		int i = strlen(Path) - 1;

		// Remove any trailing slashes
		while (i && (Path[i] == '\\' || Path[i] == '/'))
			Path[i--] = '\0';
	}

	return 0;
}

// ----------------------------------------------------------------------------