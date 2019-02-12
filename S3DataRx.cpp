#include "stdafx.h"
#include "S3DataModel.h"
#include "S3I2C.h"
#include "S3GPIB.h"

#ifdef S3_AGENT
#include "S3Agent\S3Comms.h"
#endif

extern pS3DataModel S3Data;
// extern pS3DataModel S3Shadow;

// ----------------------------------------------------------------------------

int S3RxInit(pS3RxData node)
{
	S3ConfigInit(&(node->m_Config));

	node->m_Id = -1;
	node->m_Type = S3_RxEmpty;
	node->m_Detected = false;
	node->m_SelectedTx = 0;	// Nothing selected
	node->m_ActiveTx = 0;	// Will always have valid value - whatever's
							// connected (or not)

	// No need to force this, as S3Agent should just reflect S3 on start-up
#ifndef S3_AGENT
	node->m_ActiveTxPending = true;
#else
	node->m_ActiveTxPending = false;
#endif

	node->m_Fmax = S3_1GHZ;
	// node->m_ExtraGainCap = 15;
	node->m_ExtraGainCap = 0;

	strcpy_s(node->m_SN, S3_MAX_SN_LEN, "Unknown");
	strcpy_s(node->m_PN, S3_MAX_PN_LEN, "Unknown");
	strcpy_s(node->m_HW, S3_MAX_SW_VER_LEN, "Unknown");
	strcpy_s(node->m_FW, S3_MAX_SW_VER_LEN, "Unknown");
	strcpy_s(node->m_ModelName, S3_MAX_MODEL_ID_LEN, RxTypeStrings[S3_RxEmpty]);

	// Enable all as don't know type here
	for (unsigned char i = 0; i < S3_MAX_TXS; i++)
	{
		node->m_RLL[i] = SHRT_MIN;
		node->m_RFGain[i] = SHRT_MIN;
		node->m_RFLevel[i] = SHRT_MIN;
		node->m_LinkGain[i] = SCHAR_MIN;

		node->m_TxAlarms[i] = 0;
	}

	node->m_AGC[0] = S3_AGC_GAIN;
	node->m_AGC[1] = S3_AGC_GAIN;

	node->m_Vcc = 0;

	// Read from Rx on detection
	node->m_TempLo = S3_RX_UNDER_TEMP_LIM;
	node->m_TempHi = S3_RX_OVER_TEMP_LIM;

	// Read from Rx on detection
	node->m_RLLLo = S3_RLL_GOOD_LO_10MDBM;
	node->m_RLLHi = S3_RLL_GOOD_HI_10MDBM;

	// Read from Rx on detection
	node->m_CalGain[0] = SHRT_MIN;
	node->m_CalGain[1] = SHRT_MIN;
	
	node->m_Alarms = 0x00;
	node->m_Temp = -128; // Unknown

	for(unsigned char i = 0; i < S3_RX_CTRL_ALARM_BYTES;  i++)
		node->m_RxAlarms[i] = 0;

	return 0;
}

// ----------------------------------------------------------------------------

int S3RxSetType(pS3RxData Rx, S3RxType type)
{
	if (type == S3_RxEmpty)
	{
		if (Rx->m_Type != S3_RxEmpty)
			S3EventLogAdd("Receiver disconnected", 1, Rx->m_Id, -1, -1);

		strcpy_s(Rx->m_ModelName, S3_MAX_MODEL_ID_LEN, RxTypeStrings[S3_RxEmpty]);
	}
	else
	{
		if (Rx->m_Type == S3_RxEmpty)
			S3EventLogAdd("Receiver inserted", 1, Rx->m_Id, -1, -1);

		if (type == S3_Rx1)
		{
			strcpy_s(Rx->m_ModelName, S3_MAX_MODEL_ID_LEN, RxTypeStrings[S3_Rx1]);
		}
		else if (type == S3_Rx2)
		{
			strcpy_s(Rx->m_ModelName, S3_MAX_MODEL_ID_LEN, RxTypeStrings[S3_Rx2]);
		}
		else if (type == S3_Rx6)
		{
			strcpy_s(Rx->m_ModelName, S3_MAX_MODEL_ID_LEN, RxTypeStrings[S3_Rx6]);
		}
		else
			return 1;
	}

	Rx->m_Type = type;
	Rx->m_Alarms = 0;

#ifndef S3_AGENT
	// DEBUG ONLY: Required for GUI setting of type, otherwise poll mechanism
	// overwrites it
	S3PollRxSetType(Rx);
#endif

	return 0;
}

// ----------------------------------------------------------------------------
// New Rx module discovered.

int S3RxInserted(char Rx, S3RxType type)
{
#ifndef S3_AGENT
	pS3RxData pRx = &S3Data->m_Rx[Rx];
	// pS3RxData pRxS = &S3Shadow->m_Rx[Rx];

	/*
	if (type == pRxS->m_Type)
	{
		// Copy from 'shadow' data
		// strcpy_s(pRx->m_NodeName, S3_MAX_NODE_NAME_LEN, pRxS->m_NodeName);
	}
	*/

	// Initialise 'live' data
	for (char Tx = 0; Tx < S3_MAX_TXS; Tx++)
	{
		pRx->m_RLL[Tx] = SHRT_MIN;
		pRx->m_RFGain[Tx] = SHRT_MIN;
		pRx->m_RFLevel[Tx] = SHRT_MIN;
		pRx->m_LinkGain[Tx] = SCHAR_MIN;

		// pRx->m_Tx[Tx].m_Type = S3_TxUnconnected;
	}

	pRx->m_Detected = true;
	S3RxSetType(pRx, type);

	// Force switch to active transmitter
	pRx->m_ActiveTxPending = true;
#endif

	return 0;
}

// ---------------------------------------------------------------------------
// Rx module physically removed.

int S3RxRemoved(char Rx)
{
#ifndef S3_AGENT
	char SRx, STx, SIP;
	
	S3GetSelected(&SRx, &STx, &SIP);

	if (SRx == Rx)
		S3SetSelected(-1, -1, -1);

	pS3RxData pRx = &S3Data->m_Rx[Rx];

	S3RxCancelAlarm(Rx, -1, S3_ALARMS_ALL);
	S3RxSetType(pRx, S3_RxEmpty);

	pRx->m_Detected = false;

	for(char Tx = 0; Tx < S3_MAX_TXS; Tx++)
		S3TxRemoved(Rx, Tx);
#endif

	return 0;
}

// ---------------------------------------------------------------------------

int S3RxSetActiveTxPending(char Rx, bool pending)
{
	S3Data->m_Rx[Rx].m_ActiveTxPending = pending;

	return 0;
}

// -----------------------------------------------------------------------------

bool S3RxGetActiveTxPending(char Rx)
{
	return S3Data->m_Rx[Rx].m_ActiveTxPending;
}

// -----------------------------------------------------------------------------
int S3RxSetActiveTx(char Rx, char Tx)
{
#ifdef S3_AGENT
    CString Command, Args, Response, Taustr;
    Command = L"SELECTTX";


    Args.Format(_T(" %d %d"), (Rx + 1), (Tx + 1));

    Command.Append(Args);
    Response = SendSentinel3Message(Command);

	return 0;
#else
	pS3RxData	pRx;

	if (Rx < 0 || Rx >= S3_MAX_RXS)
		return 1;

	if (Tx < 0 || Tx >= S3_MAX_TXS)
		return 1;

	pRx = &S3Data->m_Rx[Rx];

	if (pRx->m_Type == S3_Rx6)
	{
		// Nothing to do
		if (pRx->m_ActiveTx == Tx)
			return 0;

		if (pRx->m_ActiveTxPending)
		{
			// pRx->m_ActiveTxPending = false;	// Ack

			// Rx6 Tx is aleady connected, but RLL stability has not been
			// established until it becomes active for the first time.
			if (pRx->m_Tx[Tx].m_RLLStableCnt == S3_RLL_STAB_UNKNOWN)
				pRx->m_Tx[Tx].m_RLLStableCnt = 0;
		}
		// else
		{
			// Ensure Tx updates selected input and gain
			pRx->m_Tx[Tx].m_ActiveInputPending = true;			

			pRx->m_ActiveTx = Tx;
			pRx->m_ActiveTxPending = true;
		}
	}
	else
		pRx->m_ActiveTx = 0;

	return 0;
#endif
}

// ---------------------------------------------------------------------------
// Only applicable to Rx6 - what would be sensible to return for Rx1 & 2s?
// See below...

char S3RxGetActiveTx(char Rx)
{
	if (Rx < 0 || Rx >= S3_MAX_RXS)
		return -1;

	pS3RxData pRx = &S3Data->m_Rx[Rx];

	if (pRx->m_Type == S3_Rx6)
	{
		return pRx->m_ActiveTx;
	}
	else
		return -1;
}

// ---------------------------------------------------------------------------

bool S3RxIsActiveTx(char Rx, char Tx)
{
	if (Rx < 0 || Rx >= S3_MAX_RXS)
		return false;

	pS3RxData pRx = &S3Data->m_Rx[Rx];

	if (pRx->m_Type == S3_Rx6)
	{
		return pRx->m_ActiveTx == Tx;
	}
	else
		return true;
}

// ----------------------------------------------------------------------------
// Does it exist?
int S3RxExistQ(char Rx)
{
	if (!S3RxValidQ(Rx) || S3Data->m_Rx[Rx].m_Type == S3_RxEmpty)
		return 0;

	return 1;
}

// ----------------------------------------------------------------------------
// Could it exist?
int S3RxValidQ(char Rx)
{
	if (Rx < 0 || Rx >= S3_MAX_RXS)
		return 0;

	return 1;
}
// ----------------------------------------------------------------------------

char S3RxGetTxN(char Rx)
{
	char ret = 0;

	switch (S3Data->m_Rx[Rx].m_Type)
	{
	case S3_Rx1:
		ret = 1;
		break;
	case S3_Rx2:
		ret = 2;
		break;
	case S3_Rx6:
		ret = 6;
		break;
	}

	return ret;
}

// ----------------------------------------------------------------------------

int S3RxSetNodeName(const char *CurrentNode, char *NodeName)
{
	pS3RxData Rx = &(S3Data->m_Rx[CurrentNode[0] - 1]);
	strcpy_s(Rx->m_NodeName, S3_MAX_NODE_NAME_LEN, NodeName);

	return 0;
}

// ----------------------------------------------------------------------------
// This value may just be a calculation and not require explicit setting.
int S3RxSetLinkGain(char Rx, char Tx, char Gain)
{
	S3Data->m_Rx[Rx].m_LinkGain[Tx] = Gain;

	return 0;
}

// ----------------------------------------------------------------------------
// This may just be a re-calc to, say, update the display.
char S3RxGetLinkGain(char Rx, char Tx)
{
	return S3Data->m_Rx[Rx].m_LinkGain[Tx];
}

// ----------------------------------------------------------------------------

unsigned char S3RxGetType(char Rx)
{
	pS3RxData	pRx;

	if (Rx == -1)
		return S3_RxEmpty;

	pRx = &S3Data->m_Rx[Rx];

	/*if (S3Data->m_DemoMode && pRx->m_Type == S3_TxUnconnected)
		return S3Shadow->m_Rx[Rx].m_Type;*/

	return pRx->m_Type;
}

// ---------------------------------------------------------------------------

short S3RxGetRLL(char Rx, char Tx)
{
	pS3RxData	pRx;

	pRx = &S3Data->m_Rx[Rx];

	return pRx->m_RLL[Tx];
}

// ---------------------------------------------------------------------------

int S3RxSetRLLLimits(char Rx, short hi, short lo)
{
	S3Data->m_Rx[Rx].m_RLLHi = hi;
	S3Data->m_Rx[Rx].m_RLLLo = lo;

	return 0;
}

// ---------------------------------------------------------------------------

short S3RxGetRLLLo(char Rx)
{
	return S3Data->m_Rx[Rx].m_RLLLo;
}

// ---------------------------------------------------------------------------

short S3RxGetRLLHi(char Rx)
{
	return S3Data->m_Rx[Rx].m_RLLHi;
}

// ---------------------------------------------------------------------------
// SHRT_MIN means undefined/unread. If about to sleep invalidate RLL,
// otherwise it will be seen as a Tx re-awakening.

int S3RxSetRLL(char Rx, char Tx, short RLL)
{
	pS3TxData pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

	// If not the active Rx6 Tx, then hold onto old value and suppress alarm
	if (!S3RxIsActiveTx(Rx, Tx))
	{
		if (pTx->m_RLLStableCnt != S3_RLL_STAB_OK)
			pTx->m_RLLStableCnt = S3_RLL_STAB_UNKNOWN;
		return 0;
	}

	if (!S3TxExistQ(Rx, Tx))
	{
		S3Data->m_Rx[Rx].m_RLL[Tx] = SHRT_MIN;
		return 0;
	}
 
	if (S3TxGetPowerStat(Rx, Tx) == S3_TX_SLEEP_PENDING ||
		S3TxGetPowerStat(Rx, Tx) == S3_TX_SLEEP)
	{
		S3Data->m_Rx[Rx].m_RLL[Tx] = SHRT_MIN;

		return 0;
	}

	short OldRLL = S3Data->m_Rx[Rx].m_RLL[Tx];
	S3Data->m_Rx[Rx].m_RLL[Tx] = RLL;

	// Wait for stable RLL on start-up (inc. wake-up)
	if (pTx->m_RLLStableCnt != S3_RLL_STAB_OK)
	{
		if (RLL < S3RxGetRLLLo(Rx)) // S3_RLL_GOOD_LO_10MDBM)
		{
			pTx->m_RLLStableCnt = 0;

			if (pTx->m_RLLLowCnt < S3_RLL_LOW_CNT)
				pTx->m_RLLLowCnt++;
			else
				pTx->m_RLLStableCnt = S3_RLL_STAB_LOW;
		}
		else
		{
			// Reset stability count if RLL was low and now OK
			if (pTx->m_RLLLowCnt)
			{
				pTx->m_RLLStableCnt = 0;
			}

			pTx->m_RLLLowCnt = 0;

			if (RLL > S3_RLL_GOOD_LO_10MDBM && ABS(RLL - OldRLL) < S3_RLL_STABLE_THRESH)
			{
				pTx->m_RLLStableCnt++;
			}
			else
				pTx->m_RLLStableCnt = 0;

			if (pTx->m_RLLStableCnt > S3_RLL_STABLE_CNT)
			{
				// Mark Tx as 'stable'. Permanent until TxOpt reset (wake)
				pTx->m_RLLStableCnt = S3_RLL_STAB_OK;
			}
			else
			{
				// TODO: Count these to implement time-out?
				return 0;
			}
		}
	}

	// Once S3_RLL_STAB_OK, treat low RLLs as read
	if ((pTx->m_RLLStableCnt == S3_RLL_STAB_OK ||
								pTx->m_RLLStableCnt == S3_RLL_STAB_LOW) &&
		(RLL < S3RxGetRLLLo(Rx) && RLL != SHRT_MIN))
	{
		S3RxSetAlarm(Rx, Tx, S3_RX_RLL_LOW);
		return 0;
	}
	else
	{
		int update = S3RxCancelAlarm(Rx, Tx, S3_RX_RLL_LOW);

		// If low RLL now good, need to force a gain change
		if (update)
		{
			unsigned char UserAGC =	S3RxGetAGC(Rx, Tx);

			if (UserAGC >= S3_PENDING)
				UserAGC -= S3_PENDING;
			
			// If AGC gain-change mode, need to re-apply RLL factor
			if (UserAGC == S3_AGC_GAIN)
				S3IPSetGainSent(Rx, Tx, S3TxGetActiveIP(Rx, Tx), SCHAR_MIN); 
		}
	}

	if (RLL > S3RxGetRLLHi(Rx) && RLL != SHRT_MIN)
	{
		S3RxSetAlarm(Rx, Tx, S3_RX_RLL_HIGH);
		return 0;
	}
	else
		S3RxCancelAlarm(Rx, Tx, S3_RX_RLL_HIGH);

	return 0;
}

// ---------------------------------------------------------------------------
// Tx (1 or 2) is only applicable to Rx2

unsigned char S3RxGetAGC(char Rx, char Tx)
{
	// TODO: Handle this?
	if (Rx == -1)
		return 0;

	if (S3RxGetType(Rx) == S3_Rx2)
	{
		ASSERT(Tx < 2);
	}
	else
	{
		Tx = 0;
	}

	return S3Data->m_Rx[Rx].m_AGC[Tx];
}

// ---------------------------------------------------------------------------
// Although global setting, need to track whether changes have been applied
// to individual receivers.

int S3RxSetAGC(char Rx, char Tx, unsigned char AGC)
{
#ifdef S3_AGENT
	CString Command, Args, Response, Taustr;
    Command = L"AGC";

    switch(AGC)
    {
        case (S3_AGC_OFF + S3_PENDING):
            Args.Format(_T(" %d OFF"), (Rx + 1));
            break;
        case (S3_AGC_CONT + S3_PENDING):
            Args.Format(_T(" %d CONT"), (Rx + 1));
            break;
        case (S3_AGC_GAIN + S3_PENDING):
            Args.Format(_T(" %d GAIN"), (Rx + 1));
            break;
    }

    Command.Append(Args);
    Response = SendSentinel3Message(Command);
	return 0;
#else
	if (S3RxGetType(Rx) == S3_Rx2)
		ASSERT(Tx < 2);
	else
		Tx = 0;

	S3Data->m_Rx[Rx].m_AGC[Tx] = AGC;

	return 0;
#endif
}

// ---------------------------------------------------------------------------

short S3RxGetRFGain(char Rx, char Tx)
{
	pS3RxData	pRx;

	pRx = &S3Data->m_Rx[Rx];

	return pRx->m_RFGain[Tx];
}

// ---------------------------------------------------------------------------

int S3RxSetRFGain(char Rx, char Tx, short Gain)
{
	S3Data->m_Rx[Rx].m_RFGain[Tx] = Gain;

	return 0;
}
// ---------------------------------------------------------------------------

int S3RxSetPN(char Rx, const char *s)
{
	pS3RxData pRx = &S3Data->m_Rx[Rx];

	strcpy_s(pRx->m_PN, S3_MAX_PN_LEN, s);

	// TODO: Set Fmax from part number (Also Tx)
	S3RxSetFmax(Rx, s);

	return 0;
}

const char *S3RxGetPN(char Rx)
{
	pS3RxData pRx = &S3Data->m_Rx[Rx];

	return pRx->m_PN;
}

// ---------------------------------------------------------------------------

int S3RxSetFWDate(char Rx, const char *s)
{
	pS3RxData pRx = &S3Data->m_Rx[Rx];

	strcpy_s(pRx->m_FWDate, S3_MAX_FW_DATE_LEN, s);

	return 0;
}

const char *S3RxGetFWDate(char Rx)
{
	pS3RxData pRx = &S3Data->m_Rx[Rx];

	return pRx->m_FWDate;
}

// ---------------------------------------------------------------------------
// STUB:
int S3RxSetFmax(char Rx, const char *s)
{
	return 0;
}

S3Fmax S3RxGetFmax(char Rx)
{
	return S3Data->m_Rx[Rx].m_Fmax;
}

// ---------------------------------------------------------------------------

int S3RxSetSN(char Rx, const char *s)
{
	pS3RxData pRx = &S3Data->m_Rx[Rx];

	strcpy_s(pRx->m_SN, S3_MAX_SN_LEN, s);

	return 0;
}

char *S3RxGetSN(char Rx, const char *s)
{
	pS3RxData pRx = &S3Data->m_Rx[Rx];

	return pRx->m_SN;
}

// ---------------------------------------------------------------------------

int S3RxSetHW(char Rx, const char *s)
{
	return strcpy_s(S3Data->m_Rx[Rx].m_HW, S3_MAX_SW_VER_LEN, s);
}

const char *S3RxGetHW(char Rx)
{
	return S3Data->m_Rx[Rx].m_HW;
}

// ---------------------------------------------------------------------------

int S3RxSetFW(char Rx, const char *s)
{
	return strcpy_s(S3Data->m_Rx[Rx].m_FW, S3_MAX_SW_VER_LEN, s);
}

const char *S3RxGetFW(char Rx)
{
	return S3Data->m_Rx[Rx].m_FW;
}

// ---------------------------------------------------------------------------

void S3RxSetCoords(char Rx, int x, int y)
{
	pS3RxData pRx = &S3Data->m_Rx[Rx];

	pRx->m_Xref = x;
	pRx->m_Yref = y;
}

// ---------------------------------------------------------------------------

void S3RxGetCoords(char Rx, int *x, int *y)
{
	pS3RxData pRx = &S3Data->m_Rx[Rx];

	*x = pRx->m_Xref;
	*y = pRx->m_Yref;
}

// ---------------------------------------------------------------------------
// Only use for drawing stuff

unsigned char S3RxGetConnectedTxs(char Rx)
{
	pS3RxData pRx = &S3Data->m_Rx[Rx];
	unsigned char cnt = 0;

	for (char Tx = 0; Tx < S3RxGetTxN(Rx); Tx++)
		if (S3TxFOLLive(Rx, Tx))
			cnt++; // pRx->m_Input[i].m_Type != S3_TxUnconnected;
		
	return cnt;
}

// ---------------------------------------------------------------------------

unsigned char S3RxGetConnectedTx(char Rx, char Tx)
{
	pS3RxData pRx = &S3Data->m_Rx[Rx];

	return pRx->m_Tx[Tx].m_Type != S3_TxUnconnected;
}

// ---------------------------------------------------------------------------
// TODO: Trigger insert/remove event (or start-up only, as no hot-swap of Rxs)

int S3RxSetDetected(char Rx, bool detected)
{
	int	update = 0;

	if (S3Data->m_Rx[Rx].m_Detected != detected)
	{	
		S3Data->m_Rx[Rx].m_Detected = detected;
		update++;
	}

	if (S3Data->m_Rx[Rx].m_Type != S3_RxEmpty && !detected)
		update++;

	return update; 
}

// ---------------------------------------------------------------------------

bool S3RxGetDetected(char Rx)
{
	return S3Data->m_Rx[Rx].m_Detected;
}

// ---------------------------------------------------------------------------

char S3RxGetTemp(char Rx)
{
	return S3Data->m_Rx[Rx].m_Temp;
}

// ---------------------------------------------------------------------------

int	S3RxSetTempLimits(char Rx, char hi, char lo)
{
	S3Data->m_Rx[Rx].m_TempHi = hi;
	S3Data->m_Rx[Rx].m_TempLo = lo;

	return 0;
}

// ---------------------------------------------------------------------------
int S3RxSetTemp(char Rx, char t)
{
	S3Data->m_Rx[Rx].m_Temp = t;

	if (t < S3Data->m_Rx[Rx].m_TempLo)
		S3RxSetAlarm(Rx, -1, S3_RX_UNDER_TEMP);
	else if (t > S3Data->m_Rx[Rx].m_TempHi)
		S3RxSetAlarm(Rx, -1, S3_RX_OVER_TEMP);
	else
		S3RxCancelAlarm(Rx, -1, S3_RX_UNDER_TEMP | S3_RX_OVER_TEMP);

	return 0;
}

// ---------------------------------------------------------------------------

unsigned short S3RxGetVcc(char Rx)
{
	return S3Data->m_Rx[Rx].m_Vcc;
}

// ---------------------------------------------------------------------------

int S3RxSetVcc(char Rx, unsigned short v)
{
	S3Data->m_Rx[Rx].m_Vcc = v;

	if (v < S3_RX_UNDER_VOLT_LIM)
		S3RxSetAlarm(Rx, -1, S3_RX_UNDER_VOLT);
	else if (v > S3_RX_OVER_VOLT_LIM)
		S3RxSetAlarm(Rx, -1, S3_RX_OVER_VOLT);
	else
		S3RxCancelAlarm(Rx, -1, S3_RX_UNDER_VOLT | S3_RX_OVER_VOLT);

	return 0;
}

// ---------------------------------------------------------------------------
// Issue 80: Tx only applicable to Rx2

int S3RxSetCalGain(char Rx, char Tx, short cal)
{
	if (S3RxGetType(Rx) != S3_Rx2 || Tx == -1)
		Tx = 0;

	S3Data->m_Rx[Rx].m_CalGain[Tx] = cal;

	return 0;
}

// ---------------------------------------------------------------------------

short S3RxGetCalGain(char Rx, char Tx)
{
	if (Tx == -1)
		Tx = 0;

	if (S3RxGetType(Rx) != S3_Rx2)
		Tx = 0;

	return S3Data->m_Rx[Rx].m_CalGain[Tx];
}

// ---------------------------------------------------------------------------

int S3RxSetRFLevel(char Rx, char Tx, short level)
{
	S3Data->m_Rx[Rx].m_RFLevel[Tx] = level;

	return 0;
}

// ---------------------------------------------------------------------------

short S3RxGetRFLevel(char Rx, char Tx)
{
	return S3Data->m_Rx[Rx].m_RFLevel[Tx];
}

// ---------------------------------------------------------------------------
// Sort out Rx from Rx+ series

int S3RxSetExtraGainCap(char Rx, const char *FWPartNum)
{
	S3RxData *pRx = &S3Data->m_Rx[Rx];

	if (!strcmp(FWPartNum, "82044"))
	{
		pRx->m_ExtraGainCap = 0;
	}
	else // 82060 onwards
	{
		S3Data->m_Rx[Rx].m_ExtraGainCap = 15;
		strcat_s(S3Data->m_Rx[Rx].m_ModelName, S3_MAX_MODEL_ID_LEN, "+");
	}

	// Limit the stored gains in case Rx of different type inserted
	for(char Tx = 0; Tx < S3_MAX_TXS; Tx++)
	{
		S3TxData *pTx = &pRx->m_Tx[Tx];
		for(char IP = 0; IP < S3_MAX_IPS; IP++)
		{
			if (pTx->m_Input[IP].m_Config.m_Gain < S3_MIN_GAIN + pRx->m_ExtraGainCap)
				pTx->m_Input[IP].m_Config.m_Gain = S3_MIN_GAIN + pRx->m_ExtraGainCap;

			if (pTx->m_Input[IP].m_Config.m_Gain > S3_MAX_GAIN + pRx->m_ExtraGainCap)
				pTx->m_Input[IP].m_Config.m_Gain = S3_MAX_GAIN + pRx->m_ExtraGainCap;
		}
	}

	return 0;
}

// ----------------------------------------------------------------------------

char S3RxGetExtraGainCap(char Rx)
{
	return S3Data->m_Rx[Rx].m_ExtraGainCap;
}

// ---------------------------------------------------------------------------