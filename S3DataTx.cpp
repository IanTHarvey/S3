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
#include <ctype.h>
#include <math.h>

#include "S3DataModel.h"
#include "S3GPIB.h"
#include "S3I2C.h"

extern pS3DataModel S3Data;
extern pS3DataModel S3Shadow;

// ----------------------------------------------------------------------------

int S3TxInit(pS3TxData node)
{
	S3ConfigInit(&(node->m_Config));

	node->m_Id = node->m_ParentId = -1;
	node->m_Type = S3_TxUnconnected;
	node->m_Detected = false;
	node->m_Uptime = 0;
	node->m_PowerStat = S3_TX_ON;	// Unknown status
	node->m_UserSleep = false;
	node->m_EmergencySleep = false;
	node->m_ActiveInput = 0;
	node->m_TestSigInput = -1; // Off

	node->m_SelfTestPending = false;
	node->m_SelfTestRetries = 0;

	node->m_OldTauOrder = false;
	node->m_PeakHoldCap = false;
	node->m_AttenGainCap = false;

	node->m_AttenGainOffset = 0;

	node->m_CompMode = 1; // Continuous

	node->m_Xref = node->m_Yref = -1;

	strcpy_s(node->m_SN, S3_MAX_SN_LEN, "Unknown");
	strcpy_s(node->m_PN, S3_MAX_PN_LEN, "Unknown");
	strcpy_s(node->m_FW, S3_MAX_SW_VER_LEN, "Unknown");
	strcpy_s(node->m_HW, S3_MAX_SW_VER_LEN, "Unknown");
	strcpy_s(node->m_ModelName, S3_MAX_MODEL_ID_LEN, "Unknown");

	strcpy_s(node->m_BattSN, S3_MAX_SN_LEN, "Unknown");
	strcpy_s(node->m_BattPN, S3_MAX_SN_LEN, "Unknown");
	strcpy_s(node->m_BattHW, S3_MAX_SW_VER_LEN, "Unknown");
	strcpy_s(node->m_BattFW, S3_MAX_SW_VER_LEN, "Unknown");

	node->m_SoC = 0;
	node->m_BattTemp = 0;
	node->m_BattValidated = false;
	node->m_ATTE = 0;

	node->m_TempTx = SCHAR_MIN;
	node->m_TempComp = SCHAR_MIN;
	
	// Experimental
	node->m_TempStableCnt = 0;
	node->m_TempChange = 1;
	node->m_TempReport = SCHAR_MIN;

	node->m_RLLStableCnt = 0;

	node->m_LaserPow = SHRT_MIN;
	node->m_LaserLo = SHRT_MIN;	// Disabled
	node->m_LaserHi = SHRT_MAX;	// Disabled

	node->m_PeakThresh = SHRT_MIN;
	node->m_PeakHold = SHRT_MIN;
	node->m_ClearPeakHold = 0;

	node->m_CalOpt = SHRT_MIN;
	for(char i = 0; i < 7; i++)
		node->m_CalRF[i] = SHRT_MIN;

	node->m_Wavelength = S3_1310nm;
	node->m_Fmax = S3_1GHZ;

	node->m_Alarms = 0x00;
	node->m_BattAlarms = 0x00;
	node->m_OptAlarms[0] = node->m_OptAlarms[1] = node->m_OptAlarms[2] = 0x00;
	node->m_CtrlAlarms[0] = node->m_CtrlAlarms[1] = 0x00;

	// Only require initialisation for testing
	node->m_Tau_ns[0] = 0.0;
	node->m_Tau_ns[1] = 100.0;		// 0.1us
	node->m_Tau_ns[2] = 1000.0;		// 1.0us
	node->m_Tau_ns[3] = 10000.0;	// 10.0us

	wcscpy_s(node->m_TauUnits[0], S3_MAX_TAU_UNITS_LEN, _T("None"));
	wcscpy_s(node->m_TauUnits[1], S3_MAX_TAU_UNITS_LEN, _T("0.1\u03BCs"));
	wcscpy_s(node->m_TauUnits[2], S3_MAX_TAU_UNITS_LEN, _T("1.0\u03BCs"));
	wcscpy_s(node->m_TauUnits[3], S3_MAX_TAU_UNITS_LEN, _T("10.0\u03BCs"));

	return 0;
}

// ----------------------------------------------------------------------------

int S3TxSetType(char Rx, char Tx, S3TxType type)
{
	pS3TxData pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

	if (type == S3_TxUnconnected)
	{
		if (pTx->m_Type != S3_TxUnconnected)
			S3TxSetUnconnected(pTx);

		strcpy_s(pTx->m_ModelName, S3_MAX_MODEL_ID_LEN, "Unconnected");
	}
	else if (type == S3_Tx1)
	{
		if (pTx->m_Type == S3_TxUnconnected)
			S3TxSetConnected(pTx);

		strcpy_s(pTx->m_ModelName, S3_MAX_MODEL_ID_LEN, "Tx1");
	}
	else if (type == S3_Tx8)
	{
		if (pTx->m_Type == S3_TxUnconnected)
			S3TxSetConnected(pTx);

		strcpy_s(pTx->m_ModelName, S3_MAX_MODEL_ID_LEN, "Tx8");
	}
	else
		return 1;

	pTx->m_Type = type;

	// DEBUG ONLY: Required for GUI setting of type, otherwise poll mech
	// overwrites it
	S3PollTxSetType(pTx);

	return 0;
}

// ----------------------------------------------------------------------------

int S3TxSetTypeP(pS3TxData pTx, S3TxType type)
{
	if (type == S3_TxUnconnected)
	{
		if (pTx->m_Type != S3_TxUnconnected)
			S3TxSetUnconnected(pTx);

		strcpy_s(pTx->m_ModelName, S3_MAX_MODEL_ID_LEN, "Unconnected");
	}
	else if (type == S3_Tx1)
	{
		if (pTx->m_Type == S3_TxUnconnected)
			S3TxSetConnected(pTx);

		strcpy_s(pTx->m_ModelName, S3_MAX_MODEL_ID_LEN, "Tx1");
	}
	else if (type == S3_Tx8)
	{
		if (pTx->m_Type == S3_TxUnconnected)
			S3TxSetConnected(pTx);

		strcpy_s(pTx->m_ModelName, S3_MAX_MODEL_ID_LEN, "Tx8");
	}
	else
		return 1;

	pTx->m_Type = type;

	// DEBUG ONLY: Required for GUI setting of type, otherwise poll mech
	// overwrites it
	S3PollTxSetType(pTx);

	return 0;
}

// ---------------------------------------------------------------------------

int S3TxInserted(char Rx, char Tx, S3TxType type)
{
	pS3TxData pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

	S3TxSetType(Rx, Tx, type);

	pTx->m_Detected = true;
	pTx->m_LaserPow = SHRT_MIN;

	pTx->m_Uptime = 0;

	pTx->m_OldTauOrder = false;
	pTx->m_AttenGainCap = false;
	pTx->m_PeakHoldCap = false;
	pTx->m_AttenGainOffset = 0;
	pTx->m_PeakThresh = SHRT_MIN;
	pTx->m_PeakHold = SHRT_MIN;
	pTx->m_ClearPeakHold = 0;

	pTx->m_EmergencySleep = false;
	
	pTx->m_SelfTestPending = S3Data->m_TxSelfTest;
	pTx->m_SelfTestRetries = 0;

	pTx->m_TempStableCnt = 0;
	pTx->m_TempReport = SCHAR_MIN;
	pTx->m_TempChange = 0;

	pTx->m_RLLStableCnt = 0;

	S3Data->m_Rx[Rx].m_RLL[Tx] = SHRT_MIN;

	// In case 'sleeped' when live
	// pTx->m_PowerStat = S3_TX_ON;

	return 0;
}

// ---------------------------------------------------------------------------

int S3TxRemoved(char Rx, char Tx)
{
	pS3TxData pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

	S3RxCancelAlarm(Rx, Tx, S3_ALARMS_ALL);

	S3TxCancelAlarm(Rx, Tx, S3_TX_ALARMS_ALL);
	S3TxSetType(Rx, Tx, S3_TxUnconnected);

	// S3TxSetActiveIP(Rx, Tx, -1);

	pTx->m_Detected = false;
	pTx->m_TestSigInput = -1;
	pTx->m_Uptime = 0;
	pTx->m_PeakHoldCap = false;

	S3Data->m_Rx[Rx].m_RLL[Tx] = SHRT_MIN;

	return 0;
}

// ---------------------------------------------------------------------------

bool S3TxIsDetected(char Rx, char Tx)
{
	pS3TxData pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

	return pTx->m_Detected;
}

// ---------------------------------------------------------------------------

bool S3TxGetPeakHoldCap(char Rx, char Tx)
{
	pS3TxData pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

	return pTx->m_PeakHoldCap;
}

// ---------------------------------------------------------------------------

int S3TxSetPeakHoldCap(char Rx, char Tx, bool peakholdcap)
{
	S3Data->m_Rx[Rx].m_Tx[Tx].m_PeakHoldCap = peakholdcap;
	
	return 0;
}

// ---------------------------------------------------------------------------

S3TxPwrMode S3TxGetPowerStat(char Rx, char Tx)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_PowerStat;
}

// ----------------------------------------------------------------------------

int S3TxSetPowerStatP(pS3TxData Tx, S3TxPwrMode mode)
{
	// Up date data model
	Tx->m_PowerStat = mode;

	// TODO: I2C

	return 0;
}

// ----------------------------------------------------------------------------
// Does it exist?
int S3TxExistQ(char Rx, char Tx)
{
	if (Tx == -1 || !S3TxValidQ(Rx, Tx) ||
			S3Data->m_Rx[Rx].m_Tx[Tx].m_Type == S3_TxUnconnected)
		return 0;

	return 1;
}

// ----------------------------------------------------------------------------
// Could it exist? Or, is the address safe?
int S3TxValidQ(char Rx, char Tx)
{
	if (Rx < 0 || Tx < 0)
		return 0;

	switch (S3Data->m_Rx[Rx].m_Type)
	{
	case S3_Rx1:
		if (Tx > 0)
			return 0;
		break;
	case S3_Rx2:
		if (Tx > 1)
			return 0;
		break;
	case S3_Rx6:
		if (Tx > 5)
			return 0;
		break;
	case S3_RxEmpty:
		return 0;
		break;
	default:
		return -1;
	}

	return 1;
}

// ----------------------------------------------------------------------------

int S3IPExistQ(char Rx, char Tx, char IP)
{
	if (!S3TxValidQ(Rx, Tx))
		return 0;

	if (S3Data->m_Rx[Rx].m_Tx[Tx].m_Type == S3_Tx8 &&
											IP >= 0 && IP < S3_MAX_IPS)
		return 1;

	if (S3Data->m_Rx[Rx].m_Tx[Tx].m_Type == S3_Tx1 && IP == 0)
		return 1;

	return 0;
}

// ----------------------------------------------------------------------------
// Only to be event-driven

int S3TxSetUnconnected(pS3TxData Tx)
{
	S3EventLogAdd("Transmitter disconnected",
		1, Tx->m_ParentId, Tx->m_Id, -1);

	S3TxCancelAlarm(Tx->m_ParentId, Tx->m_Id, S3_TX_ALARMS_ALL);

	for (char IP = 0; IP < S3TxGetNIP(Tx->m_ParentId, Tx->m_Id); IP++)
	{
		S3IPCancelAlarm(Tx->m_ParentId, Tx->m_Id, IP, S3_ALARMS_ALL);
	}

	// Tx->m_PowerStat = S3_TX_NOT_CONNECTED;
	Tx->m_SoC = 0;
	// Tx->m_Alarms = 0x00;
	Tx->m_BattAlarms = 0x00;
	Tx->m_OptAlarms[0] = Tx->m_OptAlarms[1] = Tx->m_OptAlarms[2] = 0x00;

	// S3RxSetActiveTx(Tx->m_ParentId, Tx->m_Id);

	return 0;
}

// ----------------------------------------------------------------------------
// Only to be event-driven

int S3TxSetConnected(pS3TxData pTx)
{
	S3EventLogAdd("Transmitter connected",
		1, pTx->m_ParentId, pTx->m_Id, -1);

	// i2c Set parameters for slot
	//pTx->m_BattValidated = S3BattValidate(pTx->m_BattSN) == true;

	//if (pTx->m_BattValidated)
	//	S3TxSetAlarm(pTx->m_ParentId, pTx->m_Id, S3_CH_BATT_INVALID);
	//else
	//	S3TxCancelAlarm(pTx->m_ParentId, pTx->m_Id, S3_CH_BATT_INVALID);

	// TODO: I2C
	// pTx->m_PowerStat = S3_TX_ON;
	pTx->m_SoC = 0;
	pTx->m_Alarms = 0x00;
	pTx->m_BattAlarms = 0x00;
	pTx->m_OptAlarms[0] = pTx->m_OptAlarms[1] = pTx->m_OptAlarms[2] = 0x00;

	// S3RxSetActiveTx(Tx->m_ParentId, Tx->m_Id);

	return 0;
}
// ----------------------------------------------------------------------------

int S3TxSetNodeName(const char *	Node, char *NodeName)
{
	pS3TxData TxHd = &(S3Data->m_Rx[Node[0] - 1].m_Tx[Node[1] - 1]);
	strcpy_s(TxHd->m_NodeName, S3_MAX_NODE_NAME_LEN, NodeName);

	return 0;
}

// ----------------------------------------------------------------------------

int	S3TxPushConfig(char CurrentRx, char CurrentTx)
{
	S3TxData	*md = &(S3Data->m_Rx[CurrentRx].m_Tx[CurrentTx]);

	for (char j = 0; j < S3_MAX_IPS; j++)
	{
		pS3IPData IP = &(md->m_Input[j]);

		S3CopyConfig(&(IP->m_Config), &(md->m_Config));
	}

	return 0;
}

// ----------------------------------------------------------------------------
// DONE: Use instead of Rx->m_Exist[]

bool S3TxConnected(char Rx, char Tx)
{
	pS3TxData	pTx = &S3Data->m_Rx[Tx].m_Tx[Tx];

	if (pTx->m_Type != S3_TxUnconnected)
		return true;

	return false;
}

// ----------------------------------------------------------------------------

int S3TxCopy(pS3TxData node, pS3TxData src)
{
	// S3ConfigInit(&(node->m_Config));

	node->m_Id = src->m_Id;
	node->m_ParentId = src->m_ParentId;

	node->m_Type = src->m_Type;
	// node->m_Status = src->m_Status;
	node->m_PowerStat = src->m_PowerStat; // Unknown status
	node->m_ActiveInput = src->m_ActiveInput;

	strcpy_s(node->m_SN, S3_MAX_SN_LEN, src->m_SN);
	strcpy_s(node->m_ModelName, S3_MAX_MODEL_ID_LEN, src->m_ModelName);

	S3CopyConfig(&node->m_Config, &src->m_Config);

	return 0;
}

// ----------------------------------------------------------------------------
// Not used

int S3TxPowerAll(unsigned char On)
{
	// Initialise
	for (char cRx = 0; cRx < S3_MAX_RXS; cRx++)
	{
		S3RxData	*pRx = &(S3Data->m_Rx[cRx]);

		if (S3RxExistQ(cRx))
		{
			for (char cTx = 0; cTx < S3_MAX_TXS; cTx++)
			{
				if (S3TxExistQ(cRx, cTx))
				{
					if (On)
						S3TxSetPowerStat(cRx, cTx, S3_TX_ON);
					else
						S3TxSetPowerStat(cRx, cTx, S3_TX_SLEEP);
				}
			}
		}
	}

	return 0;
}

// ----------------------------------------------------------------------------

S3TxType S3TxGetType(char Rx, char Tx)
{
	pS3TxData	pTx;

	pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

	/*if (S3Data->m_DemoMode && pTx->m_Type == S3_TxUnconnected)
		return S3Shadow->m_Rx[Rx].m_Tx[Tx].m_Type;*/

	return pTx->m_Type;
}

// ----------------------------------------------------------------------------

int S3TxSetTestToneIP(char Rx, char Tx, char IP) // , unsigned char SigOn)
{
	
	if (S3Data->m_Rx[Rx].m_Tx[Tx].m_TestSigInput >= 99)
	{
		S3Data->m_Rx[Rx].m_Tx[Tx].m_TestSigInput = IP;
	}
	else
	{
		if (S3Data->m_Rx[Rx].m_Tx[Tx].m_TestSigInput != IP)
		{
			S3Data->m_Rx[Rx].m_Tx[Tx].m_TestSigInput = IP + 100;
		}
	}
	
	// else
	//	S3Data->m_Rx[Rx].m_Tx[Tx].m_TestSigInput = -1;

	/*
	// No. Must specifiy individual input....
	if (Tx == -1)
	{
	if (Rx == -1)
	{
	for (char i = 0; i < S3_MAX_RXS; i++)
	{
	for (char j = 0; j < S3_MAX_TXS; j++)
	{
	if (S3Data->m_Rx[i].m_Input[j].m_Type == S3_Tx1 ||
	S3Data->m_Rx[i].m_Input[j].m_Type == S3_Tx8)
	{
	S3Data->m_Rx[i].m_Input[j].m_TestSig = (SigOn != 0);
	}
	}
	}
	}
	else
	{
	for (char j = 0; j < S3_MAX_TXS; j++)
	{
	if (S3Data->m_Rx[Rx].m_Input[j].m_Type == S3_Tx1 ||
	S3Data->m_Rx[Rx].m_Input[j].m_Type == S3_Tx8)
	{
	S3Data->m_Rx[Rx].m_Input[j].m_TestSig = (SigOn != 0);
	}
	}
	}
	}
	else
	{
	S3Data->m_Rx[Rx].m_Tx[Tx].m_TestSig = (SigOn != 0);

	// I2C Set
	}
	*/

	return 0;
}

// ---------------------------------------------------------------------------
// Obsolete - although test tone is mutually exclusive, not to _user_ 
char S3TxGetTestToneIP(char Rx, char Tx)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_TestSigInput;
}

// ---------------------------------------------------------------------------

int S3TxSetActiveIP(char Rx, char Tx, char IP)
{
	pS3TxData	pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

	char curIP = pTx->m_ActiveInput;

	if (curIP >= 100)
		curIP -= 100;

	if (S3IPGetAlarms(Rx, Tx, curIP) & S3_IP_OVERDRIVE)
		return 1;

	if (pTx->m_Type == S3_Tx8)
	{
		// Nothing to do
		if (pTx->m_ActiveInput == IP)
			return 0;

		if (pTx->m_ActiveInput == IP + 100)
			pTx->m_ActiveInput -= 100;	// Ack
		else
			pTx->m_ActiveInput = IP + 100; // Updated request
	}
	else
		pTx->m_ActiveInput = 0;

	return 0;
}

// ---------------------------------------------------------------------------

char S3TxGetActiveIP(char Rx, char Tx)
{
	pS3TxData	pTx;
	char IP;

	if (Rx == -1 || Tx == -1)
		return -1;
	
	pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

	if (pTx->m_Type == S3_Tx8)
		IP = pTx->m_ActiveInput;
	else IP = 0;

	return IP;
}

// ----------------------------------------------------------------------------

int S3TxSetPowerStat(char Rx, char Tx, S3TxPwrMode mode)
{
	// Update data model
	if (mode != S3_TX_ON && mode != S3_TX_SLEEP)
		return 1;

	if (!S3TxGetBattValidated(Rx, Tx))
		mode = S3_TX_SLEEP;
	
	if (Rx == -1 && Tx == -1)
	{
		char Rxs;
		
		if (S3GetDemoMode())
			Rxs = 1;
		else
			Rxs = S3_MAX_RXS;
		
		for (Rx = 0; Rx < Rxs; Rx++)
		{
			if (S3RxExistQ(Rx))
			{
				for (char Tx = 0; Tx < S3_MAX_TXS; Tx++)
				{
					if (S3TxExistQ(Rx, Tx))
					{
						pS3TxData pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

						if (pTx->m_PowerStat == S3_TX_ON &&
														mode == S3_TX_SLEEP)
						{
							pTx->m_PowerStat = S3_TX_SLEEP_PENDING;
							pTx->m_UserSleep = false;
						}
						else if (pTx->m_PowerStat == S3_TX_SLEEP &&
														mode == S3_TX_ON)
						{
							pTx->m_PowerStat = S3_TX_ON_PENDING;
						}
					}
				}
			}
		}
	}
	else
	{
		pS3TxData pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

		if ((pTx->m_PowerStat == S3_TX_ON || 
			pTx->m_PowerStat == S3_TX_ON_PENDING) && mode == S3_TX_SLEEP)
		{
			pTx->m_PowerStat = S3_TX_SLEEP_PENDING;
		}
		else if (pTx->m_PowerStat == S3_TX_SLEEP_PENDING && mode == S3_TX_SLEEP)
		{
			pTx->m_PowerStat = mode;

			pTx->m_LaserPow = SHRT_MIN;
			pTx->m_Uptime = 0;
			pTx->m_Alarms = 0;
			pTx->m_RLLStableCnt = 0;
		}
		else if (pTx->m_PowerStat == S3_TX_SLEEP && mode == S3_TX_ON)
		{
			pTx->m_PowerStat = S3_TX_ON_PENDING;

			pTx->m_LaserPow = SHRT_MIN;
			pTx->m_Uptime = 0;
			pTx->m_Alarms = 0;
			pTx->m_RLLStableCnt = 0;
		}
		else if (pTx->m_PowerStat == S3_TX_ON_PENDING && mode == S3_TX_ON)
			pTx->m_PowerStat = mode;
	}

	return 0;
}

// ----------------------------------------------------------------------------

int S3TxSetTCompMode(char Rx, char Tx, unsigned char mode)
{
	// In case multiply scheduled
	if (mode >= 200)
		mode -= 100;

	// Update data model
	if (Rx == -1 && Tx == -1)
	{
		// Apply global setting to all Txs
		for (Rx = 0; Rx < S3_MAX_RXS; Rx++)
		{
			if (S3RxExistQ(Rx))
			{
				for (char Tx = 0; Tx < S3_MAX_TXS; Tx++)
				{
					// Apply anyway
					if (1) // S3TxExistQ(Rx, Tx))
					{
						S3Data->m_Rx[Rx].m_Tx[Tx].m_CompMode = mode;
					}
				}
			}
		}
	}
	else
	{
		S3Data->m_Rx[Rx].m_Tx[Tx].m_CompMode = mode;
	}

	return 0;
}

// ----------------------------------------------------------------------------

unsigned char S3TxGetTCompMode(char Rx, char Tx)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_CompMode;
}

// ---------------------------------------------------------------------------

int S3TxFindSN(char *cRx, char *cTx, char *SN)
{
	char	MatchCnt = 0; // Sanity check

	// Initialise
	for (char Rx = 0; Rx < S3_MAX_RXS; Rx++)
	{
		S3RxData	*pRx = &(S3Data->m_Rx[Rx]);

		if (S3RxExistQ(Rx))
		{
			for (char Tx = 0; Tx < S3_MAX_TXS; Tx++)
			{
				if (S3TxExistQ(Rx, Tx))
				{
					S3TxData *pTx = &(pRx->m_Tx[Tx]);

					if (!strcmp(SN, pTx->m_SN))
					{
						// ASSERT(MatchCnt == 0);

						*cTx = Tx;
						*cRx = Rx;

						MatchCnt++;
					}
				}
			}
		}
	}

	return MatchCnt;
}

// ---------------------------------------------------------------------------
// For testing only

int S3SetDummyTxSN()
{
	char	MatchCnt = 0; // Sanity check

	// Initialise
	for (char Rx = 0; Rx < S3_MAX_RXS; Rx++)
	{
		S3RxData	*pRx = &(S3Data->m_Rx[Rx]);

		for (char Tx = 0; Tx < S3_MAX_TXS; Tx++)
		{
			S3TxData *pTx = &(pRx->m_Tx[Tx]);

			sprintf_s(pTx->m_SN, S3_MAX_SN_LEN,
				"SN-%s-%03d-%03d", pTx->m_ModelName, Rx, Tx);

			// Make 1/10 naughty
			if (rand() > RAND_MAX / 40)
			{
				sprintf_s(pTx->m_BattSN, S3_MAX_SN_LEN,
					"SN-%s-%03d-%03d123", pTx->m_ModelName, Rx, Tx);
			}
			else
			{
				sprintf_s(pTx->m_BattSN, S3_MAX_SN_LEN,
					"SN-%s-%03d-%03d124", pTx->m_ModelName, Rx, Tx);
			}
		}
	}

	return MatchCnt;
}

// ---------------------------------------------------------------------------

int S3TxSetPN(char Rx, char Tx, const char *s)
{
	pS3TxData pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

	strcpy_s(pTx->m_PN, S3_MAX_PN_LEN, s);

	// TODO: Set Fmax from part number (Also Rx)
	S3TxSetFmax(Rx, Tx, s);

	return 0;
}

const char *S3TxGetPN(char Rx, char Tx)
{
	pS3TxData pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

	return pTx->m_PN;
}

// ---------------------------------------------------------------------------
// STUB:
int S3TxSetFmax(char Rx, char Tx, const char *s)
{
	// TODO: Extract from s
	return 0;
}

S3Fmax S3TxGetFmax(char Rx, char Tx)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_Fmax;
}

// ---------------------------------------------------------------------------
// ATiS
// char PN[] = "S3T-08-01-00";
// char SN[] = "1248574";

// CEA
// char PN[] = "S3T-01-01-00";
// char SN[] = "1247107";

// Eng
// char PN[] = "S3T-08-01-00";
// char SN[] = "EngT801";

// Wichita
// char PN[] = "S3T-08-01-00";
// char SN[] = "1248718";

int S3TxSetSN(char Rx, char Tx, const char *s)
{
	pS3TxData pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

	strcpy_s(pTx->m_SN, S3_MAX_SN_LEN, s);

	// Wichita Tx8 - SN1248718 - this is truly shite
	int iSN;
	
	S3ExtractSN(&iSN, s);

	if (iSN > 1248718)
	{
		pTx->m_OldTauOrder = false;

		pTx->m_AttenGainCap = true;
		pTx->m_AttenGainOffset = S3_ATTEN_GAIN_OFFSET;
	}
	else
	{
		pTx->m_OldTauOrder = true;
				
		pTx->m_AttenGainCap = false;
		pTx->m_AttenGainOffset = 0;
	}

	return 0;
}

const char *S3TxGetSN(char Rx, char Tx)
{
	pS3TxData pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

	return pTx->m_SN;
}

// ---------------------------------------------------------------------------

int S3TxSetHW(char Rx, char Tx, const char *s)
{
	return strcpy_s(S3Data->m_Rx[Rx].m_Tx[Tx].m_HW, S3_MAX_SW_VER_LEN, s);
}

const char *S3TxGetHW(char Rx, char Tx)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_HW;
}

// ---------------------------------------------------------------------------

int S3TxSetFW(char Rx, char Tx, const char *s)
{
	return strcpy_s(S3Data->m_Rx[Rx].m_Tx[Tx].m_FW, S3_MAX_SW_VER_LEN, s);
}

const char *S3TxGetFW(char Rx, char Tx)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_FW;
}

// ---------------------------------------------------------------------------

int S3TxSetFWDate(char Rx, char Tx, const char *s)
{
	return strcpy_s(S3Data->m_Rx[Rx].m_Tx[Tx].m_FWDate, S3_MAX_FW_DATE_LEN, s);
}

const char *S3TxGetFWDate(char Rx, char Tx)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_FWDate;
}

// ---------------------------------------------------------------------------
// Don't store, just deduce Tx capabilities

int S3TxOptSetFW(char Rx, char Tx, const unsigned char *s)
{
	char v0, v1, v2;
	v0 = *s - '0'; v1 = *(s + 1) - '0'; v2 = *(s + 2) - '0';

	// > 1.2.0 have true peak hold capability

	// TODO: Temporarily disable for 1.2.0 - too flaky
	if (v0 == 1 && v1 == 2)
		S3TxSetPeakHoldCap(Rx, Tx, true); // false);
	else if (v0 > 1) 
		S3TxSetPeakHoldCap(Rx, Tx, true);
	else if (v0 == 1 && v1 >= 2)
		S3TxSetPeakHoldCap(Rx, Tx, true);
	else
		S3TxSetPeakHoldCap(Rx, Tx, false);

	return 0;
}

// ---------------------------------------------------------------------------

void S3TxSetCoords(char Rx, char Tx, int x, int y)
{
	pS3TxData pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

	pTx->m_Xref = x;
	pTx->m_Yref = y;
}

// ---------------------------------------------------------------------------

void S3TxGetCoords(char Rx, char Tx, int *x, int *y)
{
	pS3TxData pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

	*x = pTx->m_Xref;
	*y = pTx->m_Yref;
}

// ---------------------------------------------------------------------------
// TODO: Check these may be empty but never NULL.

int S3TxGetInfo(char Rx, char Tx,
	const char **SN, const char **PN,
	const char **HWV, const char **FWV)
{
	pS3TxData	pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

	if (SN != NULL)
	{
		*SN = pTx->m_SN;
	}

	if (PN != NULL)
	{
		*PN = pTx->m_PN;
	}

	if (HWV != NULL)
	{
		*HWV = pTx->m_HW;
	}

	if (FWV != NULL)
	{
		*FWV = pTx->m_FW;
	}

	return 0;
}

// ---------------------------------------------------------------------------

int S3RxGetInfo(char Rx,
	const char **SN, const char **PN,
	const char **HWV, const char **FWV)
{
	pS3RxData	pRx = &S3Data->m_Rx[Rx];

	if (SN != NULL)
	{
		*SN = pRx->m_SN;
	}

	if (SN != NULL)
	{
		*PN = pRx->m_PN;
	}

	if (HWV != NULL)
	{
		*HWV = pRx->m_HW;
	}

	if (FWV != NULL)
	{
		*FWV = pRx->m_FW;
	}

	return 0;
}

// ---------------------------------------------------------------------------

bool S3TxFOLLive(char Rx, char Tx)
{
	// If Rx6, FOL only live if switched Tx is 'live'
	if (S3RxGetType(Rx) == S3_Rx6)
	{
		Tx = S3RxGetActiveTx(Rx);
		if (Tx >= 100)
			return false;
	}

	return	S3TxGetPowerStat(Rx, Tx) < S3_TX_SLEEP &&
			S3TxGetType(Rx, Tx) != S3_TxUnconnected &&
			S3TxGetBattValidated(Rx, Tx);
}

// ----------------------------------------------------------------------------

int S3TxSetTempTEC(char Rx, char Tx, short t)
{
	pS3TxData	pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];
	pTx->m_TempTEC = t;

	return 0;
}

// ----------------------------------------------------------------------------

short S3TxGetTempTEC(char Rx, char Tx)
{
	pS3TxData	pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];
	
	return pTx->m_TempTEC;
}

// ----------------------------------------------------------------------------

char S3TxGetTemp(char Rx, char Tx)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_TempTx;
}

// ----------------------------------------------------------------------------

int S3TxSetTemp(char Rx, char Tx, char t)
{
	pS3TxData	pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];
	pTx->m_TempChange = 0;

	// Assumes fairly regular update interval for m_TempStableCnt to be
	// meaningful time-wise
	if (t == pTx->m_TempTx)
	{
		pTx->m_TempStableCnt++;

		if (pTx->m_TempStableCnt > S3_TEMP_STABLE_INTERVAL)
		{
			if (pTx->m_TempReport != t)
			{
				pTx->m_TempReport = t;
				pTx->m_TempChange = 1;
			}

			pTx->m_TempStableCnt = 0;
		}
	}
	else if (ABS(t - pTx->m_TempTx) > 1)
	{
		pTx->m_TempReport = t;
		pTx->m_TempChange = 1;
		pTx->m_TempStableCnt = 0;
	}
	else
	{
		pTx->m_TempStableCnt = 0;
	}

	pTx->m_TempTx = t;

	if (t > S3_TX_OVER_TEMP_LIM)
	{
		S3TxSetAlarm(Rx, Tx, S3_TX_OVER_TEMP);
		S3TxSetPowerStat(Rx, Tx, S3_TX_SLEEP);
		S3TxSetEmergency(Rx, Tx, true);
	}
	else
	{
		S3TxCancelAlarm(Rx, Tx, S3_TX_OVER_TEMP);

	}

	if (S3GetTCompMode() != S3_TCOMP_GAIN)
	{
		pTx->m_TempComp = t;
		return 0;
	}

	if (abs(pTx->m_TempComp - t) > S3_COMP_TEMP_DIFF)
	{
		S3TxSetAlarm(Rx, Tx, S3_TX_RECOMP_REQ);
	}
	else
	{
		S3TxCancelAlarm(Rx, Tx, S3_TX_RECOMP_REQ);
	}

	return 0;
}

// ----------------------------------------------------------------------------

/*
char S3TxGetTemp(char Rx, char Tx)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_TempReport;
}

char S3TxGetTempPrev(char Rx, char Tx)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_TempReportPrev;
}

char S3TxResetTemp(char Rx, char Tx)
{
	S3Data->m_Rx[Rx].m_Tx[Tx].m_TempReportPrev =
		S3Data->m_Rx[Rx].m_Tx[Tx].m_TempReport;
}

unsigned char S3TxGetTempStable(char Rx, char Tx)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_TempStableCnt;
}
*/

/*
int S3TxSetTemp(char Rx, char Tx, char t)
{
	pS3TxData	pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

	// Assumes fairly regular update interval for m_TempStableCnt to be
	// meaningful time-wise
	if (t == pTx->m_TempTx)
	{
		pTx->m_TempStableCnt++;

		if (m_TempStableCnt > 5)
		{
			pTx->m_TempReportPrev = pTx->m_TempReport;
			pTx->m_TempReport = t;
			pTx->m_TempStableCnt = 0;
		}
	}
	else
		pTx->m_TempStableCnt = 0;

	if (ABS(t - pTx->m_TempTx) > 1)
	{
		pTx->m_TempReportPrev = pTx->m_TempReport;
		pTx->m_TempReport = t;
		pTx->m_TempStableCnt = 0;
	}

	pTx->m_TempTx = t;

	if (t > S3_TX_OVER_TEMP_LIM)
	{
		S3TxSetAlarm(Rx, Tx, S3_TX_OVER_TEMP);
		S3TxSetPowerStat(Rx, Tx, S3_TX_SLEEP);
		S3TxSetEmergency(Rx, Tx, true);
	}
	else
	{
		S3TxCancelAlarm(Rx, Tx, S3_TX_OVER_TEMP);

	}

	if (S3GetTCompMode() != S3_TCOMP_GAIN)
	{
		pTx->m_TempComp = t;
		return 0;
	}

	if (abs(pTx->m_TempComp - t) > S3_COMP_TEMP_DIFF)
	{
		S3TxSetAlarm(Rx, Tx, S3_TX_RECOMP_REQ);
	}
	else
	{
		S3TxCancelAlarm(Rx, Tx, S3_TX_RECOMP_REQ);
	}

	return 0;
}
*/

// ----------------------------------------------------------------------------

char S3TxGetTempComp(char Rx, char Tx)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_TempComp;
}

// ----------------------------------------------------------------------------

int S3TxSetTempComp(char Rx, char Tx, char t)
{
	S3Data->m_Rx[Rx].m_Tx[Tx].m_TempComp = t;
	return 0;
}

// ----------------------------------------------------------------------------

int S3TxDoComp(char Rx, char Tx)
{
	if (S3RxGetType(Rx) == S3_RxEmpty)
		return 0;

	if (S3GetTCompMode() != S3_TCOMP_GAIN)
	{
		S3Data->m_Rx[Rx].m_Tx[Tx].m_TempComp = S3Data->m_Rx[Rx].m_Tx[Tx].m_TempTx;
		return 0;
	}
	
	if (S3Data->m_Rx[Rx].m_Tx[Tx].m_TempComp == S3Data->m_Rx[Rx].m_Tx[Tx].m_TempTx)
		return 0;

	// Schedule compensation update for next poll
	S3Data->m_Rx[Rx].m_Tx[Tx].m_TempComp = SCHAR_MIN;

	return 0;
}

// ----------------------------------------------------------------------------

short S3TxGetLaserPow(char Rx, char Tx)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_LaserPow;
}

// ----------------------------------------------------------------------------

int S3TxSetLaserPow(char Rx, char Tx, short p)
{
	S3Data->m_Rx[Rx].m_Tx[Tx].m_LaserPow = p;

	if (p > S3Data->m_Rx[Rx].m_Tx[Tx].m_LaserHi)
		S3TxSetAlarm(Rx, Tx, S3_TX_LASER_HI);
	else if (p < S3Data->m_Rx[Rx].m_Tx[Tx].m_LaserLo)
		S3TxSetAlarm(Rx, Tx, S3_TX_LASER_LO);
	else
		S3TxCancelAlarm(Rx, Tx, S3_TX_LASER_HI | S3_TX_LASER_LO);

	return 0;
}

// ----------------------------------------------------------------------------

int S3TxSetLaserLim(char Rx, char Tx, short lo, short hi)
{
	S3Data->m_Rx[Rx].m_Tx[Tx].m_LaserLo = lo;
	S3Data->m_Rx[Rx].m_Tx[Tx].m_LaserHi  = hi;
	
	return 0;
}

// ----------------------------------------------------------------------------

int S3TxSetTauUnits(char Rx, char Tx, const unsigned char *units)
{
	pS3TxData	pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

	// TODO: Put some proper limits on this to protect against garbage

	if (*units > 10 || *(units + 1) > 10)
		return 1;

	for(unsigned char i = 0; i < 3; i++)
	{
		int nChar;
		
		pTx->m_Tau_ns[i + 1] = *(units) * pow(10.0, *(units + i * 2 + 1));

		if (*(units + i * 2 + 1) < 3)
			nChar = swprintf_s(pTx->m_TauUnits[i + 1], 16, _T("%.1fns"), pTx->m_Tau_ns[i + 1]);
		else if (*(units + i * 2 + 1) < 6)
			nChar = swprintf_s(pTx->m_TauUnits[i + 1], 16, _T("%.1f\u03bcs"), pTx->m_Tau_ns[i + 1] / 1000.0); // mu = 03BC	
		else if (*(units + i * 2 + 1) < 9)
			nChar = swprintf_s(pTx->m_TauUnits[i + 1], 16, _T("%.1fms"), pTx->m_Tau_ns[i + 1] / 1000000.0);
	}

	return 0;
}

// ----------------------------------------------------------------------------

wchar_t *S3TxGetTauUnits(char Rx, char Tx, char IP)
{
	SigmaT T = S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_Tau;

	if (T > 3 || T < 0)
	{
		S3SetSigmaTau(Rx, Tx, IP, TauNone);
		T = TauNone;
	}

	return S3Data->m_Rx[Rx].m_Tx[Tx].m_TauUnits[T];
}

// ----------------------------------------------------------------------------

wchar_t *S3TxGetTauLabel(char Rx, char Tx, SigmaT T)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_TauUnits[T];
}

// ----------------------------------------------------------------------------
// TODO: Messy - move to wchar_t for everything


int S3TxGetTauUnitsA(char *S3TxGetTauUnitsStr, char Rx, char Tx, char IP)
{
	SigmaT T = S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_Tau;
	
	wchar_t *str;
	str = S3Data->m_Rx[Rx].m_Tx[Tx].m_TauUnits[T];
	
	for(unsigned char i = 0; i <= wcslen(str) && i < S3_MAX_TAU_UNITS_LEN; i++)
	{
		if (str[i] == 0x3bc)
			S3TxGetTauUnitsStr[i] = 'u';
		else
			S3TxGetTauUnitsStr[i] = (char)str[i];
	}

	return 0;
}

// ----------------------------------------------------------------------------

int S3TxSetCalOpt(char Rx, char Tx, short cal)
{
	S3Data->m_Rx[Rx].m_Tx[Tx].m_CalOpt = cal;

	return 0;
}

// ---------------------------------------------------------------------------

short S3TxGetCalRF(char Rx, char Tx, unsigned char Path)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_CalRF[Path];
}

// ---------------------------------------------------------------------------

int S3TxSetCalRF(char Rx, char Tx, unsigned char Path, short cal)
{
	S3Data->m_Rx[Rx].m_Tx[Tx].m_CalRF[Path] = cal;

	return 0;
}

// ---------------------------------------------------------------------------

short S3TxGetCalOpt(char Rx, char Tx)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_CalOpt;
}

// ---------------------------------------------------------------------------

/*
int S3TxSetCalGain2(char Rx, char Tx, char IP, short cal)
{
	S3Data->m_Rx[Rx].m_Tx[Tx].m_CalOpt = cal;

	return 0;
}

// ---------------------------------------------------------------------------

short S3TxGetCalGain2(char Rx, char Tx, char IP)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_CalOpt;
}
*/

// ---------------------------------------------------------------------------

int S3TxSetWavelength(char Rx, char Tx, unsigned char w)
{
	S3Data->m_Rx[Rx].m_Tx[Tx].m_Wavelength = w;

	return 0;
}

unsigned char S3TxGetWavelength(char Rx, char Tx)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_Wavelength;
}

// ---------------------------------------------------------------------------
// TODO: Assign valid serial numbers to test batteries

char S3TxGetBattValidated(char Rx, char Tx)
{
	if (Rx == -1 || Tx == -1)
		return -1;

#ifdef S3_TX_VALIDBATTDISABLED
	return 1;
#else
	pS3TxData pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

	return pTx->m_BattValidated ? 1 : 0;
	
#endif
}

int S3TxSetBattValidated(char Rx, char Tx, bool valid)
{
	if (Rx == -1 || Tx == -1)
		return -1;

	pS3TxData pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

#ifdef S3_TX_VALIDBATTDISABLED
	pTx->m_BattValidated = true;
#else
	pTx->m_BattValidated = valid;
#endif

	return 0;
}

// ---------------------------------------------------------------------------
// Has Tx been explicitly sleeped by user or remote command? Used to determine
// start up sate in S3_TXSTART_USER mode.

int S3TxSetUserSleep(char Rx, char Tx, bool user)
{
	S3Data->m_Rx[Rx].m_Tx[Tx].m_UserSleep = user;

	return 0;
}

// ---------------------------------------------------------------------------

bool S3TxGetUserSleep(char Rx, char Tx)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_UserSleep;
}

// ---------------------------------------------------------------------------

int S3TxSetEmergency(	char Rx, char Tx, bool on)
{
	S3Data->m_Rx[Rx].m_Tx[Tx].m_EmergencySleep = on;

	return 0;
}

// ---------------------------------------------------------------------------
// Differentiate protective shutdown from normal

bool S3TxGetEmergency(	char Rx, char Tx)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_EmergencySleep;
}

// ---------------------------------------------------------------------------

int S3TxSetPeakThresh(char Rx, char Tx, short thresh)
{
	S3Data->m_Rx[Rx].m_Tx[Tx].m_PeakThresh = thresh;
	return 0;
}

// ---------------------------------------------------------------------------

short S3TxGetPeakThresh(char Rx, char Tx)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_PeakThresh;	
}

// ---------------------------------------------------------------------------

extern short PeakThTable[];

// TODO: Obsolete
int S3TxSetPeakHold(char Rx, char Tx, short hold)
{
	S3Data->m_Rx[Rx].m_Tx[Tx].m_PeakHold = hold;

	char path = S3IPGetPathSent(Rx, Tx, S3TxGetActiveIP(Rx, Tx));
	short limit = PeakThTable[path - 1];

	if (hold > limit)
	{
		return 1; // S3IPSetAlarm(Rx, Tx, S3TxGetActiveIP(Rx, Tx), S3_IP_OVERDRIVE); 
	}

	return 0;
}

// ---------------------------------------------------------------------------

// TODO: Obsolete
short S3TxGetPeakHold(char Rx, char Tx)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_PeakHold;	
}

// ---------------------------------------------------------------------------

int S3TxSetStableCnt(char Rx, char Tx, unsigned char StableCnt)
{
	S3Data->m_Rx[Rx].m_Tx[Tx].m_RLLStableCnt = StableCnt;
	
	return 0;
}

// ---------------------------------------------------------------------------

unsigned char S3TxGetStableCnt(char Rx, char Tx)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_RLLStableCnt;
}

// ---------------------------------------------------------------------------

bool S3TxRLLStable(char Rx, char Tx)
{
	if (S3TxGetPowerStat(Rx, Tx) == S3_TX_SLEEP)
		return true;

	return S3Data->m_Rx[Rx].m_Tx[Tx].m_RLLStableCnt == 0xFF;
}

// ----------------------------------------------------------------------------

unsigned char S3TxGetClearPeakHold(char Rx, char Tx)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_ClearPeakHold;
}

// ----------------------------------------------------------------------------

int S3TxClearPeakHold(char Rx, char Tx, unsigned char ack)
{
	if (!ack)
	{
		S3Data->m_Rx[Rx].m_Tx[Tx].m_ClearPeakHold = 1;
	}
	else
	{
		// S3Data->m_Rx[Rx].m_Tx[Tx].m_GainLock = 0;
		S3Data->m_Rx[Rx].m_Tx[Tx].m_ClearPeakHold = 0;
	}

	return 0;
}

// ----------------------------------------------------------------------------

int S3TxGetNIP(char Rx, char Tx)
{
	switch (S3Data->m_Rx[Rx].m_Tx[Tx].m_Type)
	{
	case S3_Tx1:
		return 1;
		break;
	case S3_Tx8:
		return 8;
		break;
	}

	return 0;
}

// ----------------------------------------------------------------------------

int S3TxSetTestToneEnableAll(char Rx, char Tx, char Enable)
{
	char IP;

	for(IP = 0; IP < S3TxGetNIP(Rx, Tx); IP++)
	{
		S3IPSetTestToneEnable(Rx, Tx, IP, Enable);
	}

	return 0;
}

// ---------------------------------------------------------------------------

char S3TxGetAnyAlarm(char Rx, char Tx)
{
	for (char IP = 0; IP < S3TxGetNIP(Rx, Tx); IP++)
		if (S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Alarms)
			return 1;

	if (S3Data->m_Rx[Rx].m_Tx[Tx].m_Alarms)
		return 1;

	if (S3Data->m_Rx[Rx].m_Tx[Tx].m_BattAlarms)
		return 1;

	// Just use Major/Minor for now
	if (S3Data->m_Rx[Rx].m_Tx[Tx].m_OptAlarms[0] & S3_TX_OPT_MAJOR)
		return 1;

	return 0;
}

// ---------------------------------------------------------------------------

int S3TxSysSetSN(char Rx, char Tx, const char *s)
{
	strcpy_s( S3Data->m_Rx[Rx].m_Tx[Tx].m_SN, S3_MAX_SN_LEN, s);

	return 0;
}

const char *S3TxSysGetSN(char Rx, char Tx)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_SN;
}

// ---------------------------------------------------------------------------

int S3TxGetInfoStr(char *info, char Rx, char Tx)
{
	// assert(info != NULL);

	char tmp[S3_MAX_INFO_STR_LEN];

	*tmp = '\0';

	S3ConfigGetInfoStr(tmp, Rx, Tx, -1);

	if (strlen(S3Data->m_Rx[Rx].m_Tx[Tx].m_NodeName))
		sprintf_s(info, S3_MAX_INFO_STR_LEN, "%s:\n%s",
			S3Data->m_Rx[Rx].m_Tx[Tx].m_NodeName, tmp);
	else
		sprintf_s(info, S3_MAX_INFO_STR_LEN, "%s:\n%s",
			"Unnamed", tmp);

	return 0;
}

// ---------------------------------------------------------------------------

int S3TxSetPeakPulse(short pulse)
{
	S3Data->m_PeakPulse = pulse;
	return 0;
}

short S3TxGetPeakPulse()
{
	return S3Data->m_PeakPulse;	
}

// ----------------------------------------------------------------------------

char S3TxGetAttenGainOffset(char Rx, char Tx)
{
	return 0; // S3Data->m_Rx[Rx].m_Tx[Tx].m_AttenGainOffset;
}

// ----------------------------------------------------------------------------
// Obsolete

pS3TxData S3TxGetPtr(char Rx, char Tx)
{
	return &(S3Data->m_Rx[Rx].m_Tx[Tx]);
}

// ---------------------------------------------------------------------------

bool S3TxSelfTestPending(char Rx, char Tx)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_SelfTestPending;
}

// ---------------------------------------------------------------------------
