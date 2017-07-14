#include "stdafx.h"

#include "S3DataModel.h"
#include "S3GPIB.h"
#include "S3I2C.h"
#ifdef TRIZEPS
#include "drvLib_app.h"
#endif

extern struct DbgPollSysStruct	DbgPollSysData;
extern pS3DataModel S3Data;

int S3DbgPoll();

// ---------------------------------------------------------------------------

int S3DbgPollInit()
{
	if (!S3GetDemoMode())
		return 0;

	for (char Ch = 0; Ch < S3_N_CHARGERS; Ch++)
	{
		DbgPollSysData.Ch[Ch].SoC = 0;
		DbgPollSysData.Ch[Ch].Occupied = 0;
		DbgPollSysData.Ch[Ch].SN[0] = '\0';
		DbgPollSysData.Ch[Ch].PN[0] = '\0';
		DbgPollSysData.Ch[Ch].HW[0] = '\0';
		DbgPollSysData.Ch[Ch].FW[0] = '\0';
		DbgPollSysData.Ch[Ch].BattTemp = 0;
	}

	for (char Rx = 0; Rx < S3_MAX_RXS; Rx++)
	{
		DbgPollSysData.Rx[Rx].OccupierType = S3Data->m_Rx[Rx].m_Type;

		for (char Tx = 0; Tx < S3_MAX_TXS; Tx++)
		{
			DbgPollSysData.Rx[Rx].RLL[Tx] = 0;

			DbgPollSysData.Rx[Rx].Txs[Tx].type =
				S3Data->m_Rx[Rx].m_Tx[Tx].m_Type;

			DbgPollSysData.Rx[Rx].Txs[Tx].SoC = 0;
			DbgPollSysData.Rx[Rx].Txs[Tx].BattTemp = 0;
			DbgPollSysData.Rx[Rx].Txs[Tx].BrownOut = 0;

			for (unsigned char IP = 0; IP < S3_MAX_IPS; IP++)
			{
				DbgPollSysData.Rx[Rx].Txs[Tx].IPPower[IP] = 0;
				DbgPollSysData.Rx[Rx].Txs[Tx].OverDrive[IP] = false;
			}
		}
	}

	if (S3GetDemoMode())
		S3PollSetDummyData();

	return 0;
}

// ---------------------------------------------------------------------------
// Allow GUI to set type in polled data - this is not for real

int S3PollRxSetType(pS3RxData pRx)
{
	DbgPollSysData.Rx[pRx->m_Id].OccupierType = pRx->m_Type;

	return 0;
}

// ---------------------------------------------------------------------------
// Ditto

int S3PollTxSetType(pS3TxData pTx)
{
	DbgPollSysData.Rx[pTx->m_ParentId].Txs[pTx->m_Id].type = pTx->m_Type;

	return 0;
}

// ---------------------------------------------------------------------------
// TODO: Remove - an unnecessary layer
/*
int	S3PollOld()
{
	int		Update = 0;

	Update += S3PollSys();

	return Update;
}
*/

// ---------------------------------------------------------------------------
// Was S3PollSys()

int	S3Poll(CS3ControllerDlg *parent)
{
	int		Update = 0;

	if (S3GetFactoryMode())
		return Update;

	Update = S3I2CPoll(parent);

	if (Update == -1)
		return 0;
	
	Update += S3DbgPoll();

	return Update;
}

// ----------------------------------------------------------------------------
// Do nothing unless in demo mode

int S3DbgPoll()
{
	int		Update = 0;

	if (!S3GetDemoMode())
		return Update;

	// Leave Rx[0] which may be 'live'
	for (char Rx = 1; Rx < S3_MAX_RXS; Rx++)
	{
		Update += S3DbgPollRx(Rx);
	}

	for (unsigned char Ch = 0; Ch < S3_N_CHARGERS; Ch++)
	{
		// Don't overwrite actual battery on charge
		if (S3Data->m_Chargers[Ch].m_Detected)
			continue;

		if (DbgPollSysData.Ch[Ch].Occupied !=
			S3Data->m_Chargers[Ch].m_Occupied)
		{
			Update++;

			if (DbgPollSysData.Ch[Ch].Occupied)
			{
				// EVENT
				S3ChInsert(Ch, DbgPollSysData.Ch[Ch].SN,
					DbgPollSysData.Ch[Ch].PN);
			}
			else
			{
				// EVENT
				S3ChRemove(Ch);

				DbgPollSysData.Ch[Ch].SoC = 0;
				DbgPollSysData.Ch[Ch].BattTemp = 0;
				DbgPollSysData.Ch[Ch].SN[0] = '\0';
				DbgPollSysData.Ch[Ch].PN[0] = '\0';
				DbgPollSysData.Ch[Ch].HW[0] = '\0';
				DbgPollSysData.Ch[Ch].FW[0] = '\0';
			}
		}

		if (DbgPollSysData.Ch[Ch].SoC !=
			S3Data->m_Chargers[Ch].m_SoC)
		{
			S3ChSetSoC(Ch, DbgPollSysData.Ch[Ch].SoC);

			Update++;
		}

		if (S3Data->m_Chargers[Ch].m_Occupied)
		{
			S3Data->m_Chargers[Ch].m_BattValidated = true;

			Update += S3ChSetBattTemp(Ch, DbgPollSysData.Ch[Ch].BattTemp);

			// This should only be interrogated on insertion
			S3ChSetBattFW(Ch, DbgPollSysData.Ch[Ch].FW);
			S3ChSetBattHW(Ch, DbgPollSysData.Ch[Ch].HW);
			
			S3ChSetBattSN(Ch, DbgPollSysData.Ch[Ch].SN);
			S3ChSetBattPN(Ch, DbgPollSysData.Ch[Ch].PN);

			if (DbgPollSysData.Ch[Ch].SoC !=
				S3Data->m_Chargers[Ch].m_SoC)
			{
				S3ChSetSoC(Ch, DbgPollSysData.Ch[Ch].SoC);

				Update++;
			}
		}
	}

	return Update;
}


// ---------------------------------------------------------------------------

int	S3DbgPollRx(char Rx)
{
	int		Update = 0;

	pS3RxData pRx = &S3Data->m_Rx[Rx];

	// Poll receiver
	if (DbgPollSysData.Rx[Rx].OccupierType != S3_RxEmpty)
	{
		if (pRx->m_Type != S3_RxEmpty)
		{
			// Trying to insert Rx into non-empty slot
			// Just ignore, don't error - can't physically happen
		}
		else
		{
			S3RxInserted(Rx, DbgPollSysData.Rx[Rx].OccupierType);
			Update = 1;
		}
		
		S3RxSetSN(Rx, DbgPollSysData.Rx[Rx].SN);
		S3RxSetPN(Rx, DbgPollSysData.Rx[Rx].PN);
		S3RxSetFW(Rx, DbgPollSysData.Rx[Rx].FW);
		S3RxSetFWDate(Rx, DbgPollSysData.Rx[Rx].FWDate);
		S3RxSetHW(Rx, DbgPollSysData.Rx[Rx].HW);
		S3RxSetTemp(Rx, DbgPollSysData.Rx[Rx].Temp);
		
		for(char Tx = 0; Tx < S3RxGetTxN(Rx); Tx++)
		{
			S3RxSetRLL(Rx, Tx, DbgPollSysData.Rx[Rx].RLL[Tx]);
			S3RxSetRFGain(Rx, Tx, DbgPollSysData.Rx[Rx].RFGain[Tx]);
			S3RxSetRFLevel(Rx, Tx, DbgPollSysData.Rx[Rx].RFLevel[Tx]);
		}

		// Mark as 'done'
		if (!pRx->m_Detected)
		{
			if (pRx->m_ActiveTx >= S3_PENDING)
				pRx->m_ActiveTx -= S3_PENDING;
		}
	}
	else
	{
		if (pRx->m_Type != S3_RxEmpty)
		{
			S3RxRemoved(Rx);
			Update = 1;
		}
	}

	// Poll each transmitter
	for (char Tx = 0; Tx < S3_MAX_TXS; Tx++)
	{
		if (S3TxValidQ(Rx, Tx))
		{
			// Get data held by Rx
			if (pRx->m_RLL[Tx] != DbgPollSysData.Rx[Rx].RLL[Tx])
			{
				pRx->m_RLL[Tx] = DbgPollSysData.Rx[Rx].RLL[Tx];
				Update++;
			}

			if (pRx->m_LinkGain[Tx] != DbgPollSysData.Rx[Rx].LinkGain[Tx])
			{
				pRx->m_LinkGain[Tx] = DbgPollSysData.Rx[Rx].LinkGain[Tx];
				Update++;
			}

			// Get data held by Tx
			pS3TxData pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

			// Check for 'pending' events and unpend them for pseudo Txs
			if (!pTx->m_Detected)
			{
				if (pTx->m_PowerStat > S3_PENDING)
					pTx->m_PowerStat -= S3_PENDING;

				if (pTx->m_ActiveInput >= S3_PENDING)
					pTx->m_ActiveInput -= S3_PENDING;

				pTx->m_BattValidated = true;
			}

			if (DbgPollSysData.Rx[Rx].Txs[Tx].type != S3_TxUnconnected)
			{
				if (pTx->m_Type != S3_TxUnconnected)
				{
					Update += S3TxSetBattSoC(Rx, Tx,
						DbgPollSysData.Rx[Rx].Txs[Tx].SoC);
					Update += S3TxSetBattTemp(Rx, Tx,
						DbgPollSysData.Rx[Rx].Txs[Tx].BattTemp);
					Update += S3TxSetTemp(Rx, Tx,
						DbgPollSysData.Rx[Rx].Txs[Tx].Temp);

					Update += S3TxSetSN(Rx, Tx, DbgPollSysData.Rx[Rx].Txs[Tx].SN);

					Update += S3TxSetBattInfo(Rx, Tx, 
						DbgPollSysData.Rx[Rx].Txs[Tx].BattSN, NULL, NULL, NULL);
						// DbgPollSysData.Rx[Rx].Txs[Tx].BattPN,
						// DbgPollSysData.Rx[Rx].Txs[Tx].BattHW,
						// DbgPollSysData.Rx[Rx].Txs[Tx].BattFW);
				}
				else
				{
					S3TxInserted(Rx, Tx, DbgPollSysData.Rx[Rx].Txs[Tx].type);
					Update = 1;
				}
			}
			else
			{
				if (pTx->m_Type != S3_TxUnconnected)
				{
					S3TxRemoved(Rx, Tx);
					Update = 1;
				}
			}

			char	IP;

			for (IP = 0; IP < S3_MAX_IPS; IP++)
			{
				if (DbgPollSysData.Rx[Rx].Txs[Tx].OverDrive[IP])
				{
					Update += S3IPSetAlarm(Rx, Tx, IP, S3_IP_OVERDRIVE);
				}
			}
		}
	}

	return Update;
}

// ---------------------------------------------------------------------------
// For Demo Mode only

int S3PollSetDummyData()
{
	SYSTEMTIME SystemTime;
	GetSystemTime(&SystemTime);
	srand(SystemTime.wSecond);

	// Charger batteries
	for (char Ch = 0; Ch < S3_N_CHARGERS; Ch++)
	{
		DbgPollSysData.Ch[Ch].SoC = (char)(100 * ((double)rand() / RAND_MAX));
		if (Ch < 1)
			DbgPollSysData.Ch[Ch].Occupied = 0;
			// DbgPollSysData.Ch[Ch].SoC = 100;
		else
			DbgPollSysData.Ch[Ch].Occupied = 1;
		
		// DbgPollSysData.Ch[Ch].Occupied = 1;
		sprintf_s(DbgPollSysData.Ch[Ch].SN, S3_MAX_SN_LEN, "N-B28-1-%d-123", Ch);
		DbgPollSysData.Ch[Ch].PN[0] = '\0';
		strcpy_s(DbgPollSysData.Ch[Ch].HW, S3_MAX_SW_VER_LEN, "1.0.2");
		strcpy_s(DbgPollSysData.Ch[Ch].FW, S3_MAX_SW_VER_LEN, "1.1.2");
		DbgPollSysData.Ch[Ch].BattTemp = 26 + (char)(10.0 * ((double)rand() / RAND_MAX - 0.5));;
	}

	// Initialise
	for (char cRx = 0; cRx < S3_MAX_RXS; cRx++)
	{
		sprintf_s(DbgPollSysData.Rx[cRx].SN, S3_MAX_SN_LEN,
				"107%d416%d", S3RxGetType(cRx), cRx);

		sprintf_s(DbgPollSysData.Rx[cRx].PN, S3_MAX_PN_LEN,
				"S3R-0%d-01-00", S3RxGetType(cRx), cRx);

		strcpy_s(DbgPollSysData.Rx[cRx].HW, S3_MAX_SW_VER_LEN, "1.0.2");
		strcpy_s(DbgPollSysData.Rx[cRx].FW, S3_MAX_SW_VER_LEN,	"1.11");
		strcpy_s(DbgPollSysData.Rx[cRx].FWDate, S3_MAX_SW_VER_LEN,	"19Jan17");
		DbgPollSysData.Rx[cRx].Temp = 20 + (char)(3.0 * ((double)rand() / RAND_MAX - 0.5));

		char cTx;
		for(cTx = 0; cTx < S3_MAX_TXS; cTx++)
		{
			if (S3TxGetPowerStat(cRx, cTx) != S3_TX_SLEEP)
				DbgPollSysData.Rx[cRx].RLL[cTx] = (short)(1000 + 200 * ((double)rand() / RAND_MAX - 0.5));
			else
				DbgPollSysData.Rx[cRx].RLL[cTx] = SHRT_MIN;

			DbgPollSysData.Rx[cRx].RFGain[cTx] = (short)(600 + 100 * ((double)rand() / RAND_MAX - 0.5));
			DbgPollSysData.Rx[cRx].RFLevel[cTx] = (short)(-2000 + 2000 * ((double)rand() / RAND_MAX - 0.5));
		}
		
		for (cTx = 0; cTx < S3_MAX_TXS; cTx++)
		{
			sprintf_s(DbgPollSysData.Rx[cRx].Txs[cTx].SN, S3_MAX_SN_LEN,
				"12%d4%d6%d", S3TxGetType(cRx, cTx), cRx, cTx);

			// Tx batteries
			DbgPollSysData.Rx[cRx].Txs[cTx].SoC = (char)(100 * ((double)rand() / RAND_MAX));
			DbgPollSysData.Rx[cRx].Txs[cTx].Temp = 20 + (char)(3.0 * ((double)rand() / RAND_MAX - 0.5));;

			// Make 1/N unvalidated
			if (rand() > RAND_MAX / 20)
			{
				sprintf_s(DbgPollSysData.Rx[cRx].Txs[cTx].BattSN, S3_MAX_SN_LEN,
					"12%d%d%d123", S3TxGetType(cRx, cTx), cRx, cTx);
			}
			else
			{
				sprintf_s(DbgPollSysData.Rx[cRx].Txs[cTx].BattSN, S3_MAX_SN_LEN,
					"12%d%d%d124", S3TxGetType(cRx, cTx), cRx, cTx);
			}
		}
	}

	return 0;
}


// ---------------------------------------------------------------------------

