// ----------------------------------------------------------------------------
// Tx Comms over SC16IS740 I2C serial bridge to TxOpt and TxCtrl boards and
// battery.
#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "S3DataModel.h"

extern pS3DataModel S3Data;

#ifdef TRIZEPS
#include "drvLib_app.h"
#endif

#include "S3I2C.h"
#include "S3Gain.h"


// ----------------------------------------------------------------------------
// Read back from Tx (local) and Rx (full fibre channel)
int S3I2CTxSelfTest(char Rx, char Tx)
{
#ifdef TRIZEPS
	short	RFLevel1 = 0, RFLevel3_0 = 0,
			RFLevel3_16 = 0, RFLevel3_32 = 0, RFLevel7 = 0;
	short	RxRFLevel1 = 0, RxRFLevel3_0 = 0,
			RxRFLevel3_16 = 0, RxRFLevel3_32 = 0, RxRFLevel7 = 0;

	short RxCal = S3RxGetCalGain(Rx, Tx);
	short TxCal = S3TxGetCalOpt(Rx, Tx);

	// Max out Rx & Tx DSAs
	int err = S3I2CWriteSerialShort(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_DSA, TxCal);

	if (err)
		return 1;

	unsigned char wbuf[3];
	wbuf[0] = S3I2C_RX_OP_DSA;
	wbuf[1] = *((unsigned char *)&RxCal + 1);
	wbuf[2] = *((unsigned char *)&RxCal + 0);
	
	BOOL ok = I2C_WriteRead(S3I2CCurRxOptAddr, wbuf, 3, NULL, 0);

	if (!ok)
		return 2;

	if (S3I2CSwitchTestTone(true))
		return 60;
	
	// Path 1 ------------------------------------------------

	if (S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR, S3I2C_TX_RF_PATH, 1))
		return 11;
	
	// err = S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_CTRL_TEST_TONE, 1);

	if (S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR, S3I2C_TX_RF_DSA, 0))
		return 12;

	Sleep(600);

	for(char IP = 0; IP < S3TxGetNIP(Rx, Tx); IP++)
	{
		// Test all Tx8 input select relays
		
		// if (S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR, S3I2C_TX_RF_IP,
		// 	(unsigned char)S3Tx8IPMap[IP] + 1))

		if (S3I2CTxSwitchInput(Rx, Tx, IP))
			return 13;

		if (!S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_RF_MON, 2))
		{
			RFLevel1 = S3RevByteShort(S3I2CTxReadBuf);
			RxRFLevel1 = S3I2CRxGetRFLevel();
		}
		else
			return 14;

		// TODO: May need to do more than set alarm?
		if (0) // RFLevel1 < 1000)
		{
			S3TxSetAlarm(Rx, Tx, S3_TX_SELF_TEST_FAIL);
			return 100;
		}
		else
			S3TxCancelAlarm(Rx, Tx, S3_TX_SELF_TEST_FAIL);
	}

	// Path 3 ------------------------------------------------
	Sleep(1000);

	if (S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR, S3I2C_TX_RF_PATH, 3))
	{ return 21;}

	Sleep(600);

	// Sleep(2000);
	if (!S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_RF_MON, 2))
	{
		RFLevel3_0 = S3RevByteShort(S3I2CTxReadBuf);
		RxRFLevel3_0 = S3I2CRxGetRFLevel();

		if (ABS(RxRFLevel3_0 - RxRFLevel1) < 2000)
			return 200;
	}
	else
		return 22;

	Sleep(600);

	// ------------------------------------------------------------------------
	// Switch in PAD16
	if (S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR, S3I2C_TX_RF_DSA, 16))
		return 31;

	Sleep(600);
	if (!S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_RF_MON, 2))
	{
		RFLevel3_16 = S3RevByteShort(S3I2CTxReadBuf);
		RxRFLevel3_16 = S3I2CRxGetRFLevel();

		if (ABS(RxRFLevel3_16 - RxRFLevel3_0) < 800)
		{
			S3EventLogAdd("PAD16 in test fail", 3, Rx, Tx, -1);
			return 200;
		}
	}
	else	return 32;

	// Switch out PAD16
	if (S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR, S3I2C_TX_RF_DSA, 0))
		return 33;

	Sleep(600);
	if (!S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_RF_MON, 2))
	{
		RFLevel3_16 = S3RevByteShort(S3I2CTxReadBuf);
		RxRFLevel3_16 = S3I2CRxGetRFLevel();

		if (ABS(RxRFLevel3_16 - RxRFLevel3_0) > 300)
		{
			S3EventLogAdd("PAD16 out test fail", 3, Rx, Tx, -1);
			return 200;
		}
	}
	else	return 34;

	// ------------------------------------------------------------------------
	// Switch in PAD32
	if (S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR, S3I2C_TX_RF_DSA, 32))
		return 41;

	Sleep(2000);
	if (!S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_RF_MON, 2))
	{
		RFLevel3_32 = S3RevByteShort(S3I2CTxReadBuf);
		RxRFLevel3_32 = S3I2CRxGetRFLevel();

		if (ABS(RxRFLevel3_32 - RxRFLevel3_0) < 800)
		{
			S3EventLogAdd("PAD32 in test fail", 3, Rx, Tx, -1);
			return 200;
		}
	}
	else
		return 42;

	// Switch out PAD32
	if (S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR, S3I2C_TX_RF_DSA, 0))
		return 43;

	Sleep(600);
	if (!S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_RF_MON, 2))
	{
		RFLevel3_32 = S3RevByteShort(S3I2CTxReadBuf);
		RxRFLevel3_32 = S3I2CRxGetRFLevel();

		if (ABS(RxRFLevel3_32 - RxRFLevel3_0) > 300)
		{
			S3EventLogAdd("PAD32 out test fail", 3, Rx, Tx, -1);
			return 200;
		}
	}
	else
		return 44;

	// Path 7 ------------------------------------------------

	if (S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR, S3I2C_TX_RF_PATH, 7))
		return 51;

	Sleep(1000);
	if (!S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_RF_MON, 2))
	{
		RFLevel7 = S3RevByteShort(S3I2CTxReadBuf);
		RxRFLevel7 = S3I2CRxGetRFLevel();

		if (ABS(RxRFLevel7 - RxRFLevel3_0) < 10)
		{
			S3EventLogAdd("HIZ test fail", 3, Rx, Tx, -1);
			return 200;
		}
	}
	else	return 52;

	if (S3I2CSwitchTestTone(false))
		return 60;
	
	//if (S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR,
	//		S3I2C_TX_CTRL_TEST_TONE, 0x00))
	//	return 19;

	Sleep(200);

	// TODO: Compare relative RFLevels

	double RatLevel1 = 0, RatLevel3_0 = 0, RatLevel3_16 = 0, RatLevel3_32 = 0, RatLevel7 = 0;
	RatLevel1 =		(double)(RxRFLevel1 - RxRFLevel3_0) / RxRFLevel1; //	(double)(RFLevel1 - RxRFLevel1); // / RxRFLevel1;
	RatLevel3_0 =	(double)(RxRFLevel3_0 - RxRFLevel7) / RxRFLevel3_0; // (double)(RFLevel3_0 - RxRFLevel3_0); //  / RxRFLevel3_0;
	// RatLevel3_16 =	(double)(RFLevel3_16 - RxRFLevel3_16); //  / RxRFLevel3_16;
	// RatLevel3_32 =	(double)(RFLevel3_32 - RxRFLevel3_32); //  / RxRFLevel3_32;
	// RatLevel7 =		(double)(RFLevel7 - RxRFLevel7); //  / RxRFLevel7;

#endif // TRIZEPS

	return 0; // All good
}

// ----------------------------------------------------------------------------
// Reads back from Rx only

int S3I2CTxSelfTest2(char Rx, char Tx)
{
	if (!S3SelfTestEnabled())
		return 0;

#ifdef TRIZEPS
	short	RxRFLevel1 = 0, RxRFLevel3_0 = 0,
			RxRFLevel3_16 = 0, RxRFLevel3_32 = 0, RxRFLevel7 = 0;

	short RxCal = S3RxGetCalGain(Rx, Tx);
	short TxCal = S3TxGetCalOpt(Rx, Tx);

	// Max out Rx & Tx DSAs
	int err = S3I2CWriteSerialShort(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_DSA, TxCal);

	if (err)
		return 1;

	unsigned char wbuf[3];
	wbuf[0] = S3I2C_RX_OP_DSA;
	wbuf[1] = *((unsigned char *)&RxCal + 1);
	wbuf[2] = *((unsigned char *)&RxCal + 0);
	
	BOOL ok = I2C_WriteRead(S3I2CCurRxOptAddr, wbuf, 3, NULL, 0);

	if (!ok)
		return 2;

	if (S3I2CSwitchTestTone(true))
		return 60;

	Sleep(1000);

	char IP = 0;
	
	// Path 1 ------------------------------------------------

	S3SetGain(Rx, Tx, IP, 20);
	S3IPSetGainSent(Rx, Tx, IP, SCHAR_MIN);
	S3I2CSetIPGain(Rx, Tx, IP);
	
	if (err) // S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR, S3I2C_TX_RF_PATH, 1))
		return 11;
	
	// err = S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_CTRL_TEST_TONE, 1);

	if (S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR, S3I2C_TX_RF_DSA, 0))
		return 12;

	Sleep(600);

	for(char IP = 0; IP < S3TxGetNIP(Rx, Tx); IP++)
	{
		// Test all Tx8 input select relays
		if (S3I2CTxSwitchInput(Rx, Tx, IP))
			return 13;

		RxRFLevel1 = S3I2CRxGetRFLevel();

		// TODO: May need to do more than set alarm?
		if (0) // RFLevel1 < 1000)
		{
			S3TxSetAlarm(Rx, Tx, S3_TX_SELF_TEST_FAIL);
			return 100;
		}
		else
			S3TxCancelAlarm(Rx, Tx, S3_TX_SELF_TEST_FAIL);
	}

	// Path 3 ------------------------------------------------
	Sleep(1000);

	if (S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR, S3I2C_TX_RF_PATH, 3))
		return 21;

	Sleep(600);

	RxRFLevel3_0 = S3I2CRxGetRFLevel();

	if (ABS(RxRFLevel3_0 - RxRFLevel1) < 2000)
	{
		S3EventLogAdd("Path 3 switch fail", 3, Rx, Tx, -1);
		return 200;
	}

	Sleep(600);

	// ------------------------------------------------------------------------
	// Switch in PAD16
	if (S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR, S3I2C_TX_RF_DSA, 16))
		return 31;

	Sleep(1600);

	RxRFLevel3_16 = S3I2CRxGetRFLevel();

	if (ABS(RxRFLevel3_16 - RxRFLevel3_0) < 800)
	{
		S3EventLogAdd("PAD16 switch in fail", 3, Rx, Tx, -1);
		return 200;
	}
	
	// Switch out PAD16
	if (S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR, S3I2C_TX_RF_DSA, 0))
		return 33;

	Sleep(1000);

	RxRFLevel3_16 = S3I2CRxGetRFLevel();

	if (ABS(RxRFLevel3_16 - RxRFLevel3_0) > 300)
	{
		S3EventLogAdd("PAD16 switch out fail", 3, Rx, Tx, -1);
		return 200;
	}

	// ------------------------------------------------------------------------
	// Switch in PAD32
	if (S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR, S3I2C_TX_RF_DSA, 32))
		return 41;

	Sleep(1600);

	RxRFLevel3_32 = S3I2CRxGetRFLevel();

	if (ABS(RxRFLevel3_32 - RxRFLevel3_0) < 800)
	{
		S3EventLogAdd("PAD32 switch in fail", 3, Rx, Tx, -1);
		return 200;
	}

	// Switch out PAD32
	if (S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR, S3I2C_TX_RF_DSA, 0))
		return 43;

	Sleep(600);

	RxRFLevel3_32 = S3I2CRxGetRFLevel();

	if (ABS(RxRFLevel3_32 - RxRFLevel3_0) > 300)
	{
		S3EventLogAdd("PAD32 switch out fail", 3, Rx, Tx, -1);
		return 200;
	}
	
	// Path 7 ------------------------------------------------

	if (S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR, S3I2C_TX_RF_PATH, 7))
		return 51;

	Sleep(1000);

	RxRFLevel7 = S3I2CRxGetRFLevel();

	if (ABS(RxRFLevel7 - RxRFLevel3_0) < 10)
	{
		S3EventLogAdd("HiZ test fail", 3, Rx, Tx, -1);
		return 200;
	}

	if (S3I2CSwitchTestTone(false))
		return 60;
	
	//if (S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR,
	//		S3I2C_TX_CTRL_TEST_TONE, 0x00))
	//	return 19;

	Sleep(200);

#endif // TRIZEPS

	return 0; // All good
}

// ----------------------------------------------------------------------------
// Dev only