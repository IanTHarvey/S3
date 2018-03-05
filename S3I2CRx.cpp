// ----------------------------------------------------------------------------

#include "stdafx.h"

#include "S3DataModel.h"

#ifdef TRIZEPS
#include "drvLib_app.h"
#endif

#include "S3I2C.h"
#include "S3Gain.h"

unsigned char	S3I2CRxReadBuf[0x80];
unsigned char	*pS3I2CRxReadBuf;

// Rx master-selects
#define MS_RX_1		0x20
#define MS_RX_2		0x10
#define MS_RX_3		0x08
#define MS_RX_4		0x04
#define MS_RX_5		0x02
#define MS_RX_6		0x01

#define S3_RX_LOC_ID		0x00

int S3I2CGetRxStartUp(	char Rx);
bool I2CRxSetPassword(const char *pw);
int S3I2CRxCtrlSetPassword(	const char *rPW);
void I2CRxTestPassword();

int S3I2CRxMS(unsigned char Rx);

unsigned char S3I2CTxOptAddr[S3_MAX_RXS] = 
{
	S3I2C_RX_OPT0_ADDR,
	S3I2C_RX_OPT1_ADDR,
	S3I2C_RX_OPT2_ADDR,
	S3I2C_RX_OPT3_ADDR,
	S3I2C_RX_OPT4_ADDR,
	S3I2C_RX_OPT5_ADDR
};

unsigned char S3I2CRxOptAddr[S3_MAX_RXS] = {0xA4, 0xA6};

// Active transmitter I2C-Serial bridge address (array for Rx2 & Rx6 only)
unsigned char S3I2CCurTxOptAddr = S3I2CTxOptAddr[0];

// Rx controller address (array for Rx2 only)
unsigned char S3I2CCurRxOptAddr = S3I2CRxOptAddr[0];

// ----------------------------------------------------------------------------

unsigned char	S3I2CGetCurOptAddr(char Rx, char Tx)
{
	return 0;
}

// ----------------------------------------------------------------------------
// Rx poll function

int S3I2CRxGetStatus(char Rx)
{
	int update = 0;

#ifdef TRIZEPS

	S3I2CRxMS(Rx);

	unsigned char	i2cCmdBufRead[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int				ok = 0;

	unsigned char	wbuf;
	int				err;

	// Get 3x RX-CTRL alarms
	wbuf = S3I2C_RX_CTRL_ALARMS;
	ok = I2C_WriteRead(S3I2C_RX_CTRL_ADDR, &wbuf, 1, S3I2CRxReadBuf, 3);

	if (ok)
	{
		S3RxCtrlSetAlarm(Rx, S3I2CRxReadBuf);

		update = S3RxSetDetected(Rx, true);

		if (update)
		{
			if (S3I2CGetRxStartUp(Rx))
			{
				S3I2CRxMS(0xFF);
				update = false;
				S3RxSetDetected(Rx, false);

				return update;
			}

			// Depending on time to complete, could bug out
			// here and leave to next poll
		}
	}
	else
	{
		update = S3RxSetDetected(Rx, false);

		if (update)
		{
			S3RxRemoved(Rx);
		}

		S3I2CRxMS(0xFF);
		return 0;
	}

	unsigned char RxType = S3RxGetType(Rx);

	// Switch optical input BEFORE attempting to read RLL
	S3I2CRxSetActiveTx(Rx);
	S3I2CSetUpOptAddr(Rx, 0);

	// Get general optical board data from channel 1
	// Read Tx monitors including RLL
	wbuf = S3I2C_RX_OPT_MON;
	ok = I2C_WriteRead(S3I2CCurRxOptAddr, &wbuf, 1, S3I2CRxReadBuf, 16);

	if (ok)
	{
		S3RxCancelAlarm(Rx, -1, S3_RX_INT_FAIL);

		// Vcc (mV)
		S3RxSetVcc(Rx, S3RevByteShort(S3I2CRxReadBuf + 2));

		// Module temperature (DegC / 256)
		short ModTemp = S3RevByteShort(S3I2CRxReadBuf + 14);
		S3RxSetTemp(Rx, (char)(ModTemp / 256));
	}
	else
	{
		S3RxSetTemp(Rx, SCHAR_MIN);
		S3RxSetVcc(Rx, 0);

		S3RxSetAlarm(Rx, -1, S3_RX_INT_FAIL);
	}

	if (RxType == S3_Rx1)
	{
		S3I2CSetUpOptAddr(Rx, 0);
		err = S3I2CRxProcessTx(Rx, 0);
	}
	else if (RxType == S3_Rx2)
	{
		// Show both
		S3I2CSetUpOptAddr(Rx, 0);
		err = S3I2CRxProcessTx(Rx, 0);

		S3I2CSetUpOptAddr(Rx, 1);
		ok = I2C_WriteRead(S3I2CCurRxOptAddr, &wbuf, 1, S3I2CRxReadBuf, 16);
		err = S3I2CRxProcessTx(Rx, 1);
	}
	else if (RxType == S3_Rx6)
	{
		// S3I2CRxSetActiveTx(Rx);

		for(char Tx = 0; Tx < S3_MAX_TXS; Tx++)
		{
			S3I2CSetUpOptAddr(Rx, Tx);
			err = S3I2CRxProcessTx(Rx, Tx);
		}
	}

	S3I2CRxMS(0xFF);

#endif

	return update;
}

// ----------------------------------------------------------------------------

int S3I2CSetUpOptAddr(char Rx, char Tx)
{
	if (S3RxGetType(Rx) == S3_Rx1)
	{
		S3I2CCurRxOptAddr = S3I2CRxOptAddr[0];
		S3I2CCurTxOptAddr = S3I2CTxOptAddr[0];
	}
	else if (S3RxGetType(Rx) == S3_Rx2)
	{
		if (Tx == -1)
		{
			S3I2CCurRxOptAddr = S3I2CRxOptAddr[0];
			S3I2CCurTxOptAddr = S3I2CTxOptAddr[0];
		}
		else if (Tx == 0)
		{
			S3I2CCurRxOptAddr = S3I2CRxOptAddr[0];
			S3I2CCurTxOptAddr = S3I2CTxOptAddr[0];
		}
		else
		{
			S3I2CCurRxOptAddr = S3I2CRxOptAddr[1];
			S3I2CCurTxOptAddr = S3I2CTxOptAddr[1];
		}
	}
	else if (S3RxGetType(Rx) == S3_Rx6)
	{
		S3I2CCurRxOptAddr = S3I2CRxOptAddr[0];
		S3I2CCurTxOptAddr = S3I2CTxOptAddr[Tx];
	}

	return 0;
}

// ----------------------------------------------------------------------------
// Contains all get & set comms with an individual Tx and per-Tx Rx comms

int S3I2CRxProcessTx(char Rx, char Tx)
{
	int				update = 0;
#ifdef TRIZEPS
	S3TimerStart(2);
	
	int err = 0;

	update += S3I2CTxGetStatus(Rx, Tx);

	// RLL (10mdBm)
	S3RxSetRLL(Rx, Tx, S3RevByteShort(S3I2CRxReadBuf + 4));

	if (S3TxGetType(Rx, Tx) != S3_TxUnconnected)
	{
		// Only update RF parameters if FOL active
		if (S3RxIsActiveTx(Rx, Tx))
		{
			S3RxSetRFLevel(Rx, Tx, S3RevByteShort(S3I2CRxReadBuf + 6));
			S3RxSetRFGain(Rx, Tx, S3RevByteShort(S3I2CRxReadBuf + 8));
		}

		err = S3I2CTxSetStatus(Rx, Tx);
		
		if (err)
		{
			if (err == 2)
			{
				// Gain change fail
			}

			S3TxSetAlarm(Rx, Tx, S3_TX_COMM_FAIL);
		}
		else
			S3TxCancelAlarm(Rx, Tx, S3_TX_COMM_FAIL);

		S3I2CRxSetAGC(Rx, Tx);
	}

	S3TimerStop(2);

#endif

	// TODO: Is this a useful return?
	return update;
}

// ----------------------------------------------------------------------------

int S3I2CRxSetAGC(char Rx, char Tx)
{
	int				ok = 0;

#ifdef TRIZEPS

	// Read and compare Rx setting with user value.
	unsigned char	wbuf;
	wbuf = S3I2C_RX_OPT_CFG_FLAGS;
	ok = I2C_WriteRead(S3I2CCurRxOptAddr, &wbuf, 1, S3I2CRxReadBuf, 2);

	unsigned char AGC =		S3I2CRxReadBuf[1] & 0x02;
	unsigned char UserAGC =	S3RxGetAGC(Rx, Tx);

	// Has AGC setting changed?
	if (UserAGC >= S3_PENDING)
	{
		// Force gain update in next poll
		for(char IP = 0; IP < S3TxGetNIP(Rx, Tx); IP++)
			S3IPSetGainSent(Rx, Tx, IP, SCHAR_MIN);

		UserAGC -= S3_PENDING;

		S3RxSetAGC(Rx, Tx, UserAGC);
	}

	if (AGC > 0 && (UserAGC != S3_AGC_CONT))
	{
		unsigned char wwbuf[2];

		// b:2 enables software gain setting
		wwbuf[0] = S3I2C_RX_OPT_CFG_FLAGS + 1;				
		wwbuf[1] = 0x04; // S3I2CRxReadBuf[1] & 0x04;	// Turn off

		ok = I2C_WriteRead(S3I2CCurRxOptAddr, wwbuf, 2, NULL, 0);
	}
	else if (AGC == 0 && UserAGC == S3_AGC_CONT)
	{
		unsigned char wwbuf[2];

		wwbuf[0] = S3I2C_RX_OPT_CFG_FLAGS + 1;
		wwbuf[1] = 0x06; // S3I2CRxReadBuf[1] & 0x06;	// Turn on

		ok = I2C_WriteRead(S3I2CCurRxOptAddr, wwbuf, 2, NULL, 0);
	}

#endif

	return 0;
}

// ----------------------------------------------------------------------------
// Get start-up parameters from Rxs & Txs - generally not stuff polled

int S3I2CGetRxStartUp(char Rx)
{
#ifdef TRIZEPS
	unsigned char	wbuf;
	BOOL	ok;

	// ------------------------------------------------------------------------
	// Get RX-CTRL identifiers
	wbuf = 0x00;
	ok = I2C_WriteRead(S3I2C_RX_CTRL_ADDR, &wbuf, 1, S3I2CRxReadBuf, 0x44);

	// If fails, assume empty slot.
	if (!ok)
	{
		return 0;
	}

	// Get Rx type
	unsigned char RxType = *(S3I2CRxReadBuf + S3I2C_RX_CTRL_RX_TYPE);

	switch(RxType)
	{
		case 0x01: S3RxInserted(Rx, S3_Rx1); break;
		case 0x02: S3RxInserted(Rx, S3_Rx2); break;
		case 0x06: S3RxInserted(Rx, S3_Rx6); break;
		default:
			return 2; // Error
	}

	char FW[S3_MAX_SW_VER_LEN];
	sprintf_s(FW, S3_MAX_SW_VER_LEN, "%c.%c.%c",
		S3I2CRxReadBuf[0x0f], S3I2CRxReadBuf[0x10], S3I2CRxReadBuf[0x11]);
	S3RxSetFW(Rx, FW);

	char FWDate[S3_MAX_FW_DATE_LEN];
	strncpy_s(FWDate, S3_MAX_FW_DATE_LEN, (char *)S3I2CRxReadBuf + 0x12, S3I2C_FWDATE_LEN);
	FWDate[S3I2C_FWDATE_LEN] = '\0';
	S3RxSetFWDate(Rx, FWDate);

	char PN[S3_MAX_PN_LEN];
	strncpy_s(PN, S3_MAX_PN_LEN, (char *)S3I2CRxReadBuf + 0x1D, S3I2C_PN_LEN);
	TrailSpaceTo0(PN, S3I2C_PN_LEN);
	S3RxSetPN(Rx, PN);

	char SN[S3_MAX_SN_LEN];
	strncpy_s(SN, S3_MAX_SN_LEN, (char *)S3I2CRxReadBuf + 0x32, S3I2C_SN_LEN);
	TrailSpaceTo0(SN, S3I2C_SN_LEN);
	S3RxSetSN(Rx, SN);

#ifdef S3_RX_DIAGS
	int err;

	err = S3I2CRxDumpCtrlConfig(Rx);
	
	S3I2CSetUpOptAddr(Rx, 0);
	err = S3I2CRxDumpOptConfig(Rx);
	
	if (RxType == S3_Rx2)
	{
		S3I2CSetUpOptAddr(Rx, 1);
		err = S3I2CRxDumpOptConfig(Rx);
	}
	
#endif
	
	// ------------------------------------------------------------------------
	// Get RX-OPT identifiers
	// wbuf = S3_RX_LOC_ID;
	// ok = I2C_WriteRead(S3I2C_RX_CTRL_ADDR, &wbuf, 1, S3I2CRxReadBuf, 0x44);

	S3I2CSetUpOptAddr(Rx, 0);

	wbuf = S3I2C_RX_BOARD_NO;
	ok = I2C_WriteRead(S3I2CCurRxOptAddr, &wbuf, 1, S3I2CRxReadBuf, 5);
	S3I2CRxReadBuf[5] = '\0';

	S3RxSetExtraGainCap(Rx, (char *)S3I2CRxReadBuf);

	// ---------------------------------------------------------------------------
	// Get RX-OPT thresholds from RxOpt[0]
	wbuf = S3I2C_RX_OPT_THRESH;
	ok = I2C_WriteRead(S3I2CCurRxOptAddr, &wbuf, 1, S3I2CRxReadBuf, 0x20);

	if (!ok)
	{
		return 1;
	}

	// Module temperature limits - 
	short hi = S3RevByteShort(S3I2CRxReadBuf + 0x1C) / 256;
	short lo = S3RevByteShort(S3I2CRxReadBuf + 0x1E) / 256;
	S3RxSetTempLimits(Rx, (char)hi, (char)lo);

	hi = S3RevByteShort(S3I2CRxReadBuf + 0x04);
	lo = S3RevByteShort(S3I2CRxReadBuf + 0x06);
	S3RxSetRLLLimits(Rx, hi, lo);

	// Read calibrated gain
	wbuf = S3I2C_RX_OPT_CAL_GAIN;
	ok = I2C_WriteRead(S3I2CCurRxOptAddr, &wbuf, 1, S3I2CRxReadBuf, 2);

	if (!ok)
		return 1;

	short cal;
	*((char *)&cal + 0) = S3I2CRxReadBuf[1];
	*((char *)&cal + 1) = S3I2CRxReadBuf[0];

	S3RxSetCalGain(Rx, 0, cal);

	if (S3RxGetType(Rx) == S3_Rx2)
	{
		S3I2CSetUpOptAddr(Rx, 1);

		wbuf = S3I2C_RX_OPT_CAL_GAIN;
		ok = I2C_WriteRead(S3I2CCurRxOptAddr, &wbuf, 1, S3I2CRxReadBuf, 2);

		if (!ok)
			return 1;

		short cal;
		*((char *)&cal + 0) = S3I2CRxReadBuf[1];
		*((char *)&cal + 1) = S3I2CRxReadBuf[0];

		S3RxSetCalGain(Rx, 1, cal);
	}

#endif	
	return 0;
}



// ----------------------------------------------------------------------------
// TODO: Untested & unused

int S3I2CRxSetActiveTx(char Rx)
{
#ifdef TRIZEPS
	char Tx = S3RxGetActiveTx(Rx);

	if (Tx == -1)
		return 0;

	if (Tx < S3_PENDING)
		return 0; // No change pending

	Tx -= 100; // Clear pending bit

	// TEST: RX6
	// Sleep(1000);
	
	if (S3I2CRxSwitchTx(Rx, Tx))
		return 1;

	// S3RxSetActiveTx(Rx, Tx); // Ack

	// TEST: RX6
	// Sleep(1000);

#endif

	return 0;
}

// ----------------------------------------------------------------------------
// TODO: Untested

int S3I2CRxSwitchTx(char Rx, char Tx)
{
#ifdef TRIZEPS

	if (S3RxGetType(Rx) != S3_Rx6)
		return 0;

	// TEST: Rx6
	unsigned char val = I2C_ReadRandom(S3I2C_RX_CTRL_ADDR, S3I2C_RX_CTRL_OPT_SW_POS);

	unsigned char	wbuf[2] = {S3I2C_RX_CTRL_OPT_SW_POS, 0};

	wbuf[1] = Tx + 1; // 1-based index

	Sleep(1000);

	BOOL ok = I2C_WriteRead(S3I2C_RX_CTRL_ADDR, wbuf, 2, NULL, 0);

	if (!ok)
	{
		return 1;
	}

	Sleep(1000);

	// TEST: Rx6
	val = I2C_ReadRandom(S3I2C_RX_CTRL_ADDR, S3I2C_RX_CTRL_OPT_SW_POS);

	S3RxSetActiveTx(Rx, val - 1);

	// TEST: Rx6
	if (val != wbuf[1])
	{	
		return 1;
	}
		
#endif
	return 0;
}

// ----------------------------------------------------------------------------

