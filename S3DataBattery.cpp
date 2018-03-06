// ----------------------------------------------------------------------------
// Tx and charger battery functions

#include "stdafx.h"
#include "S3DataModel.h"
#include "S3GPIB.h"

extern pS3DataModel S3Data;
extern const wchar_t *BattTypeStrings[];

// ----------------------------------------------------------------------------

unsigned char S3TxGetBattSoC(char Rx, char Tx)
{
	if (Rx == -1 || Tx == -1)
		return 1;

	pS3TxData	pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

	/*
	// Shouldn't this have been ruled out before charge enquiry?
	if (pTx->m_Type == S3_TxUnconnected)
	{
		*charge = -1;
		return 1;
	}

	*charge = -1;

	// I2C Get (update?) battery status

	// TODO: Any error/calc to handle here?
	*charge = pTx->m_SoC;
	*/

	return pTx->m_SoC;
}

// ---------------------------------------------------------------------------
// Currently called by polling functions with data off the dbg structure

int S3TxSetBattSoC(char Rx, char Tx, unsigned char SoC)
{
	int update = 0;

	pS3TxData	pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

	// I2C Get (update?) battery status

	// TODO: Any error/calc to handle here?
	if (pTx->m_SoC != SoC)
	{
		update = 1;
	}

	// Although charge may not have changed, alarm may still be raised
	// at start-up
	pTx->m_SoC = SoC;

	if (SoC <= S3_SOC_ALARM)
		update += S3TxSetAlarm(Rx, Tx, S3_TX_BATT_ALARM);
	else
		update += S3TxCancelAlarm(Rx, Tx, S3_TX_BATT_ALARM);
	
	if (SoC <= S3_SOC_WARN)
		update += S3TxSetAlarm(Rx, Tx, S3_TX_BATT_WARN);
	else
		update += S3TxCancelAlarm(Rx, Tx, S3_TX_BATT_WARN);

	return update;
}

// ---------------------------------------------------------------------------
// Average time to empty (minutes)

int S3TxSetATTE(char Rx, char Tx, unsigned short atte)
{
	S3Data->m_Rx[Rx].m_Tx[Tx].m_ATTE = atte;

	return 0;
}

unsigned short S3TxGetATTE(char Rx, char Tx)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_ATTE;
}

// ---------------------------------------------------------------------------

int S3TxSetBattTemp(char Rx, char Tx, short t)
{
	int update = 0;

	pS3TxData	pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

	// I2C Get (update?) battery status

	// TODO: Any error/calc to handle here?
	if (pTx->m_BattTemp != t)
	{
		update = 1;
	}

	pTx->m_BattTemp = t;

	if (t < S3_BATT_DISCHG_MIN_T)
	{
		// Unlikely, but ensure exclusive 
		update += S3TxBattSetAlarm(Rx, Tx, S3_TX_BATT_COLD);
		update += S3TxBattCancelAlarm(Rx, Tx, S3_TX_BATT_HOT);

	}
	else if (t > S3_BATT_DISCHG_MAX_T)
	{
		update += S3TxBattSetAlarm(Rx, Tx, S3_TX_BATT_HOT);
		update += S3TxBattCancelAlarm(Rx, Tx, S3_TX_BATT_COLD);
	}
	else
		update += S3TxBattCancelAlarm(Rx, Tx, S3_TX_BATT_COLD | S3_TX_BATT_HOT);

	return update;
}

// ----------------------------------------------------------------------------

short S3TxGetBattTemp(char Rx, char Tx)
{
	if (S3Data->m_Rx[Rx].m_Tx[Tx].m_Type == S3_TxUnconnected)
		return S3_INVALID_TEMP;

	return S3Data->m_Rx[Rx].m_Tx[Tx].m_BattTemp;
}

// ----------------------------------------------------------------------------

int S3TxSetBattI(char Rx, char Tx, short i)
{
	S3Data->m_Rx[Rx].m_Tx[Tx].m_I = i;

	return 0;
}

// ---------------------------------------------------------------------------

short S3TxGetBattI(char Rx, char Tx)
{
	if (S3Data->m_Rx[Rx].m_Tx[Tx].m_Type == S3_TxUnconnected)
		return S3_INVALID_TEMP;

	return S3Data->m_Rx[Rx].m_Tx[Tx].m_I;
}

// ---------------------------------------------------------------------------

int S3TxSetBattInfo(char Rx, char Tx,
	const char *BattSN, const char *BattPN,
	const char *BattHW, const char *BattFW)
{
	int update = 0;

	pS3TxData	pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

	if (BattSN != NULL)
	{
		if (*BattSN == '\0' || strcmp(pTx->m_BattSN, BattSN))
		{
			if (pTx->m_BattValidated) //  = S3BattValidate(BattSN))
				S3TxCancelAlarm(Rx, Tx, S3_TX_BATT_INVALID);
			else
				S3TxSetAlarm(Rx, Tx, S3_TX_BATT_INVALID);

			strcpy_s(pTx->m_BattSN, S3_MAX_SN_LEN, BattSN);
			update++;
		}
	}

	if (BattPN != NULL && strcmp(pTx->m_BattPN, BattPN))
	{
		strcpy_s(pTx->m_BattPN, S3_MAX_PN_LEN, BattPN);
		update++;
	}

	if (BattHW != NULL && strcmp(pTx->m_BattHW, BattHW))
	{
		strcpy_s(pTx->m_BattHW, S3_MAX_SW_VER_LEN, BattHW);
		update++;
	}

	if (BattFW != NULL && strcmp(pTx->m_BattFW, BattFW))
	{
		strcpy_s(pTx->m_BattFW, S3_MAX_SW_VER_LEN, BattFW);
		update++;
	}

	return update;
}

// ---------------------------------------------------------------------------

int S3TxGetBattInfo(char Rx, char Tx,
	const char **BattSN, const char **BattPN,
	const char **BattHW, const char **BattFW)
{
	pS3TxData	pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

	if (*BattSN != NULL)
	{
		*BattSN = pTx->m_BattSN;
	}

	if (BattPN != NULL)
	{
		*BattPN = pTx->m_BattPN;
	}

	if (BattHW != NULL)
	{
		*BattHW = pTx->m_BattHW;
	}

	if (BattFW != NULL)
	{
		*BattFW = pTx->m_BattFW;
	}

	return 0;
}


// ----------------------------------------------------------------------------
// Applicable to charger and Tx batteries

// TODO: Placeholder
bool S3BattValidate(const char *str)
{
#ifdef S3_VALIDBATTDISABLED
	return true;
#else
	if (!strcmp("bq34z100-g1 Creasefield", str))
		return true;

	if (!strncmp("12471", str, 5))
		return true;
	
	int l;

	// TODO: Still valid for Tx batteries
	l = strlen(str);
	if (!strcmp("123", str + l - 3))
		return true;

	return false;
#endif
}

// ----------------------------------------------------------------------------
