// ----------------------------------------------------------------------------
// Battery charger functions

#include "windows.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "S3DataModel.h"
#include "S3GPIB.h"

extern pS3DataModel S3Data;
extern wchar_t *BattTypeStrings[];

// ---------------------------------------------------------------------------

int S3ChInitAll()
{
	unsigned char Ch;

	for (Ch = 0; Ch < S3_N_CHARGERS; Ch++)
		S3ChInit(Ch);
	
	return 0;
}

// ---------------------------------------------------------------------------

int S3ChInit(unsigned char Ch)
{
	// S3Data->m_Chargers[Ch].m_Type = S3_BattUnknown;
	S3Data->m_Chargers[Ch].m_Type = S3_Batt2S1P;

	S3Data->m_Chargers[Ch].m_MfrData[0] = '\0';

	S3Data->m_Chargers[Ch].m_Occupied = false;
	S3Data->m_Chargers[Ch].m_Detected = false;

	S3Data->m_Chargers[Ch].m_BattSN[0] = '\0';
	S3Data->m_Chargers[Ch].m_BattPN[0] = '\0';

	S3Data->m_Chargers[Ch].m_FW[0] = '\0';
	S3Data->m_Chargers[Ch].m_HW[0] = '\0';
	
	S3Data->m_Chargers[Ch].m_Alarms = 0;

	S3Data->m_Chargers[Ch].m_BattTemp = 0;
	S3Data->m_Chargers[Ch].m_SoC = 0;
	S3Data->m_Chargers[Ch].m_Charged = false;

	S3Data->m_Chargers[Ch].m_BattValidated = false;

	S3Data->m_Chargers[Ch].m_ATTF = 0;

	return 0;
}

// ----------------------------------------------------------------------------

bool S3ChOccupied(char Ch)
{
	return S3Data->m_Chargers[Ch].m_Occupied;
}

// ---------------------------------------------------------------------------
// TODO: Deprecate to S3SetChargeLevel?

bool S3ChFullyCharged(char Ch)
{
	if (S3Data->m_Chargers[Ch].m_Occupied)
		return S3Data->m_Chargers[Ch].m_Charged;

	return false;
}

// ---------------------------------------------------------------------------

char S3ChGetSoC(char Ch)
{
	if (S3Data->m_Chargers[Ch].m_Occupied)
		return S3Data->m_Chargers[Ch].m_SoC;

	return -1;
}

// ---------------------------------------------------------------------------

char S3ChSetSoC(char Ch, char ChLevel)
{
	if (!S3Data->m_Chargers[Ch].m_Occupied)
		return -1;

	if (ChLevel >= 100)
		S3Data->m_Chargers[Ch].m_Charged = true;
	else
		S3Data->m_Chargers[Ch].m_Charged = false;

	// -ve used to indicate failure
	// if (ChLevel < 0)
	// 	ChLevel = 0;
	// else 

	if (ChLevel > 100)
		ChLevel = 0;

	S3Data->m_Chargers[Ch].m_SoC = ChLevel;

	return 0;
}

// ---------------------------------------------------------------------------

const wchar_t *S3ChGetBattTypeStr(char Ch)
{
	return BattTypeStrings[S3Data->m_Chargers[Ch].m_Type];
}

// ---------------------------------------------------------------------------

int S3ChSetBattType(char Ch, unsigned char Type)
{
	S3Data->m_Chargers[Ch].m_Type = Type;

	return 0;
}

// ---------------------------------------------------------------------------

const char *S3ChGetBattSN(char Ch)
{
	return S3Data->m_Chargers[Ch].m_BattSN;
}

// ---------------------------------------------------------------------------

const char *S3ChGetBattPN(char Ch)
{
	return S3Data->m_Chargers[Ch].m_BattPN;
}

// ---------------------------------------------------------------------------

int	S3ChSetBattSN(char Ch, const char *SN)
{
	if (!S3Data->m_Chargers[Ch].m_Occupied)
		return 1;

	strcpy_s(S3Data->m_Chargers[Ch].m_BattSN, S3_MAX_SN_LEN, SN);

	// S3Data->m_Chargers[Ch].m_BattValidated = true;
	// S3ChBattValidate(Ch);

	return 0;
}

// ---------------------------------------------------------------------------

int S3ChSetBattPN(char Ch, const char *PN)
{
	if (!S3Data->m_Chargers[Ch].m_Occupied)
		return 1;

	strcpy_s(S3Data->m_Chargers[Ch].m_BattPN, S3_MAX_PN_LEN, PN);

	return 0;
}

// ---------------------------------------------------------------------------

const char *S3ChGetBattHW(char Ch)
{
	return S3Data->m_Chargers[Ch].m_HW;
}

// ---------------------------------------------------------------------------

int S3ChSetBattHW(char Ch, const char *Ver)
{
	if (S3Data->m_Chargers[Ch].m_Occupied)
	{
		strcpy_s(S3Data->m_Chargers[Ch].m_HW, S3_MAX_SW_VER_LEN, Ver);
		return 0;
	}

	return 1;
}

// ---------------------------------------------------------------------------


int S3ChSetBattStatus(char Ch, const unsigned char *stat)
{
	if (S3Data->m_Chargers[Ch].m_Occupied)
	{
		S3Data->m_Chargers[Ch].stat_l = *stat;
		S3Data->m_Chargers[Ch].stat_h = *(stat + 1);
	}

	return 1;
}

// ---------------------------------------------------------------------------

unsigned char S3ChGetBattStatus(char Ch)
{
	return	S3Data->m_Chargers[Ch].stat_h;
}

// ---------------------------------------------------------------------------

const char *S3ChGetBattFW(char Ch)
{
	return S3Data->m_Chargers[Ch].m_FW;
}

// ---------------------------------------------------------------------------

int S3ChSetBattFW(char Ch, const char *Ver)
{
	if (S3Data->m_Chargers[Ch].m_Occupied)
	{
		strcpy_s(S3Data->m_Chargers[Ch].m_FW, S3_MAX_SW_VER_LEN, Ver);
		return 0;
	}

	return 1;
}

// ---------------------------------------------------------------------------

const char *S3ChGetBattMfr(char Ch)
{
	return S3Data->m_Chargers[Ch].m_MfrData;
}

// ---------------------------------------------------------------------------

int S3ChSetBattMfr(char Ch, const char *MfrData)
{
	if (S3Data->m_Chargers[Ch].m_Occupied)
	{
		strcpy_s(S3Data->m_Chargers[Ch].m_MfrData, S3_MAX_SW_VER_LEN, MfrData);
		return 0;
	}

	return 1;
}

// ---------------------------------------------------------------------------

int	S3ChSetTimeToFull(	char Ch, unsigned short tmin)
{
	if (S3Data->m_Chargers[Ch].m_Occupied)
	{
		S3Data->m_Chargers[Ch].m_ATTF = tmin;
		return 0;
	}

	return 1;
}

// ---------------------------------------------------------------------------
#define S3_MAX_DURATION_LEN 16
char S3DurStr[S3_MAX_DURATION_LEN];

const char *S3ChGetTimeToFullStr(	char Ch)
{
	*S3DurStr = '\0';

	if (S3Data->m_Chargers[Ch].m_Occupied)
	{
		if (S3Data->m_Chargers[Ch].m_ATTF == 0xFFFF)
		{
			// TODO: This is probably a fault - the charger fuse
			// may have tripped resulting in ~0 charge current
			strcpy_s(S3DurStr, S3_MAX_DURATION_LEN, "0");
		}
		else
		{
			unsigned short h = S3Data->m_Chargers[Ch].m_ATTF / 60;
			unsigned short m = S3Data->m_Chargers[Ch].m_ATTF % 60;

			sprintf_s(S3DurStr, S3_MAX_DURATION_LEN, "%dh:%02dm", h, m);
		}
	}

	return S3DurStr;
}

// ---------------------------------------------------------------------------

unsigned short	S3ChGetTimeToFull(	char Ch)
{
	if (S3Data->m_Chargers[Ch].m_Occupied)
	{
		return S3Data->m_Chargers[Ch].m_ATTF;
	}

	return 0;
}

// ---------------------------------------------------------------------------
// Stuff for GUI
// TODO: Get rid
// ---------------------------------------------------------------------------

/*
void S3ChSetCoords(char Ch, int x, int y)
{
	pS3Charger pCh = &S3Data->m_Chargers[Ch];

	pCh->m_Xref = x;
	pCh->m_Yref = y;
}

// ---------------------------------------------------------------------------

void S3ChGetCoords(char Ch, int *x, int *y)
{
	pS3Charger pCh = &S3Data->m_Chargers[Ch];

	*x = pCh->m_Xref;
	*y = pCh->m_Yref;
}
*/

// ---------------------------------------------------------------------------
/*
void S3ChSetMainCoords(char Ch, int x, int y)
{
	pS3Charger pCh = &S3Data->m_Chargers[Ch];

	pCh->m_mainXref = x;
	pCh->m_mainYref = y;
}

// ---------------------------------------------------------------------------
// TODO: This should go

void S3ChGetMainCoords(char Ch, int *x, int *y)
{
	pS3Charger pCh = &S3Data->m_Chargers[Ch];

	*x = pCh->m_mainXref;
	*y = pCh->m_mainYref;
}

*/

// ---------------------------------------------------------------------------
// Eventually SN, PN etc will be interrogated from charger

int	S3ChInsert(char Ch, char *SN, char *PN)
{
	if (S3Data->m_Chargers[Ch].m_Detected)
		return 1;
	
	if (S3Data->m_Chargers[Ch].m_Occupied)
		return 1;

	S3ChInit(Ch);

	S3Data->m_Chargers[Ch].m_Occupied = true;

	if (SN)
		strcpy_s(S3Data->m_Chargers[Ch].m_BattSN, S3_MAX_SN_LEN, SN);

	if (!S3Data->m_Chargers[Ch].m_BattValidated) // = S3BattValidate(SN)))
		S3ChSetAlarm(Ch, S3_CH_BATT_INVALID);
	else
		S3ChCancelAlarm(Ch, S3_CH_BATT_INVALID);

	if (PN)
		strcpy_s(S3Data->m_Chargers[Ch].m_BattPN, S3_MAX_PN_LEN, PN);

	S3Data->m_Chargers[Ch].m_SoC = 0;
	S3Data->m_Chargers[Ch].m_Charged = false;

	// TODO: Interrogate fixed parameters, HW, FW (and SN and PN unlike here)
	// etc... Currently done by polling

	return 0;
}

// ----------------------------------------------------------------------------
// TODO: Placeholder
bool S3ChBattValidate(char Ch)
{
	if (!strcmp("bq34z100-G1 Creasefield", S3Data->m_Chargers[Ch].m_MfrData))
		return true;

	// if (!strcmp("bq34z100-G1 Texas Inst.", S3Data->m_Chargers[Ch].m_MfrData))
	//	return true;

	int l;

	// TODO: Still valid for Tx batteries
	l = strlen(S3Data->m_Chargers[Ch].m_BattSN);
	if (!strcmp("123", S3Data->m_Chargers[Ch].m_BattSN + l - 3))
		return true;

	return false; // TEST:
}

// ----------------------------------------------------------------------------

int	S3ChRemove(char Ch)
{
	if (!S3Data->m_Chargers[Ch].m_Occupied)
		return 1;

	S3ChCancelAlarm(Ch, S3_ALARMS_ALL);

	S3ChInit(Ch);

	return 0;
}

// ----------------------------------------------------------------------------

short S3ChGetBattTemp(char Ch)
{
	// Should be an error?
	if (!S3Data->m_Chargers[Ch].m_Occupied)
		return S3_INVALID_TEMP;

	return S3Data->m_Chargers[Ch].m_BattTemp;
}

// ----------------------------------------------------------------------------

int	S3ChSetBattTemp(char Ch, short t)
{
	// Should be an error?
	if (!S3Data->m_Chargers[Ch].m_Occupied)
		return 0;

	int update = 0;

	pS3Charger	pCh = &S3Data->m_Chargers[Ch];

	// I2C Get (update?) battery status

	// TODO: Any error/calc to handle here?
	if (pCh->m_BattTemp != t)
	{
		update = 1;
	}

	pCh->m_BattTemp = t;

	if (t < S3_BATT_CHARGE_MIN_T)
	{
		// Unlikely, but ensure exclusive 
		update += S3ChSetAlarm(Ch, S3_CH_BATT_COLD);
		update += S3ChCancelAlarm(Ch, S3_CH_BATT_HOT);
	}
	else if (t > S3_BATT_CHARGE_MAX_T)
	{
		update += S3ChSetAlarm(Ch, S3_CH_BATT_HOT);
		update += S3ChCancelAlarm(Ch, S3_CH_BATT_COLD);
	}
	else
	{
		update += S3ChCancelAlarm(Ch, S3_CH_BATT_COLD | S3_CH_BATT_HOT);
	}

	return update;
}

// ----------------------------------------------------------------------------

bool S3ChBattValidated(char Ch)
{
	if (!S3Data->m_Chargers[Ch].m_Occupied)
		return true;

#ifdef S3_CH_VALIDBATTDISABLED
	return true;
#else
	return S3Data->m_Chargers[Ch].m_BattValidated;
#endif
}

// ----------------------------------------------------------------------------

int S3ChSetBattValidated(char Ch, bool valid)
{
	if (!S3Data->m_Chargers[Ch].m_Occupied)
		return true;

#ifdef S3_CH_VALIDBATTDISABLED
	S3Data->m_Chargers[Ch].m_BattValidated = true;
#else
	S3Data->m_Chargers[Ch].m_BattValidated = valid;
#endif

	return 0;
}

// ----------------------------------------------------------------------------

unsigned char S3ChGetNOnCharge()
{
	unsigned char Ch, n = 0;

	for (Ch = 0; Ch < S3_N_CHARGERS; Ch++)
		if (S3Data->m_Chargers[Ch].m_Occupied)
			n++;

	return n;
}

// ---------------------------------------------------------------------------

double S3ChGetBattV(char Ch)
{
	return S3Data->m_Chargers[Ch].m_V;
}

// ---------------------------------------------------------------------------

int S3ChSetBattV(char Ch, double v)
{
	S3Data->m_Chargers[Ch].m_V = v;

	return 1;
}

// ---------------------------------------------------------------------------

short S3ChGetBattI(char Ch)
{
	return S3Data->m_Chargers[Ch].m_I;
}

// ---------------------------------------------------------------------------

int S3ChSetBattI(char Ch, short i)
{
	S3Data->m_Chargers[Ch].m_I = i;

	return 1;
}

// ---------------------------------------------------------------------------