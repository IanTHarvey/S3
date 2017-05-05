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

extern int set_RF_inp_board(unsigned char  path, unsigned char attenuation,
							char Rx, char Tx);

extern int select_RF_input(unsigned char IP, bool TestTone);

// ----------------------------------------------------------------------------
// Read back from Tx (local) and Rx (full fibre channel)
int S3I2CTxSelfTest(short *v1, short *v2, char Rx, char Tx)
{
	int		err;

	// May have been changed by user
	if (0) // !S3GetTxSelfTest())
		S3Data->m_Rx[Rx].m_Tx[Tx].m_SelfTestPending = false;
		
	if (0) // (!S3TxSelfTestPending(Rx, Tx))
		return 0;

#ifdef TRIZEPS

	char	IP = 0;
	short	RFLevel1 = 0, RFLevel3_0 = 0,
			RFLevel3_16 = 0, RFLevel3_32 = 0, RFLevel7 = 0;
	short	RxRFLevel1 = 0, RxRFLevel3_0 = 0,
			RxRFLevel3_16 = 0, RxRFLevel3_32 = 0, RxRFLevel7 = 0;

	// Max out Rx & Tx DSAs
	if (1)
	{
		short RxCal = S3RxGetCalGain(Rx, Tx);
		short TxCal = S3TxGetCalOpt(Rx, Tx);

		err = S3I2CWriteSerialShort(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_DSA, TxCal);
		if (err)
			{ err = 1;  goto ENDTEST; }

		err = S3I2CWriteLocalShort(S3I2CCurRxOptAddr, S3I2C_RX_OPT_DSA, RxCal);
		if (err)
			{ err = 1;  goto ENDTEST; }
	}
	
	// ------------------------------------------------------------------------
	//  Path 1

	if (S3TxGetType(Rx, Tx) == S3_Tx8)
		if (select_RF_input(1, false))
			{ err = 13;  goto ENDTEST; }

	if (S3I2CSwitchTestTone(false))
		{ err = 15;  goto ENDTEST; }

	if (set_RF_inp_board(1, 0, Rx, Tx))
		{ err = 11;  goto ENDTEST; }

	Sleep(300);

	short RFLevelBG = 0;
	if (!S3I2CReadSerialShort(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_RF_MON, &RFLevelBG))
	{
		*v1 = RFLevelBG;
	}
	else
		{ err = 14;  goto ENDTEST; }

	if (RFLevelBG > 0)
		{ err = 50; goto ENDTEST; }

	if (S3TxGetType(Rx, Tx) == S3_Tx8)
		if (select_RF_input(1, true))
			{ err = 13;  goto ENDTEST; }

	if (S3I2CSwitchTestTone(true))
		{ err = 15;  goto ENDTEST; }

	Sleep(400);
	
	short Dummy;
	S3I2CReadSerialShort(S3I2C_TX_CTRL_ADDR, 0xD2, &Dummy); // Dummy

	if (!S3I2CReadSerialShort(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_RF_MON, &RFLevel1))
	{
		*v2 = RFLevel1;

		if (RFLevel1 - RFLevelBG < 3000)
		{ err = 200; goto ENDTEST; }
	}
	else
		{ err = 24;  goto ENDTEST; }

	// ------------------------------------------------------------------------
	// Path 3 
	if (set_RF_inp_board(3, 0, Rx, Tx))
		{ err = 11;  goto ENDTEST; }

	Sleep(300);

	if (!S3I2CReadSerialShort(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_RF_MON, &RFLevel3_0))
	{
		*v2 = RFLevel3_0;
	}
	else
		{ err = 34;  goto ENDTEST; }

	if (S3TxGetType(Rx, Tx) == S3_Tx8)
		if (err = select_RF_input((unsigned char)S3Tx8IPMap[0] + 1, true))
			{ err = 13;  goto ENDTEST; }

	 if (S3I2CSwitchTestTone(false))
		{ err = 15;  goto ENDTEST; }

	Sleep(400);

	if (!S3I2CReadSerialShort(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_RF_MON, &RFLevelBG))
	{
		*v1 = RFLevelBG;
		if (RFLevel3_0 - RFLevelBG < 300)
			{ err = 210; goto ENDTEST; }
	}
	else
		{ err = 44;  goto ENDTEST; }

	// ------------------------------------------------------------------------
	// Path 3 with PAD16

	if (set_RF_inp_board(3, 16, Rx, Tx))
		{ err = 11;  goto ENDTEST; }

	Sleep(300);

	if (!S3I2CReadSerialShort(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_RF_MON, &RFLevelBG))
	{
		*v1 = RFLevelBG;
	}
	else
		{ err = 54;  goto ENDTEST; }

	if (S3TxGetType(Rx, Tx) == S3_Tx8)
		if (err = select_RF_input((unsigned char)S3Tx8IPMap[0] + 1, true))
			{ err = 13;  goto ENDTEST; }
	
	if (S3I2CSwitchTestTone(true))
		{ err = 15;  goto ENDTEST; }

	Sleep(400);

	if (!S3I2CReadSerialShort(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_RF_MON, &RFLevel3_16))
	{
		*v2 = RFLevel3_16;
		if (ABS(RFLevel3_16 - RFLevelBG) > 100)
			{ err = 220; goto ENDTEST; }
	}
	else
		{ err = 64;  goto ENDTEST; }

	// ------------------------------------------------------------------------
	// Path 3 with PAD16 + PAD16

	if (set_RF_inp_board(3, 32, Rx, Tx))
		{ err = 11;  goto ENDTEST; }

	Sleep(300);

	if (!S3I2CReadSerialShort(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_RF_MON, &RFLevel3_32))
	{
		*v2 = RFLevel3_32;
		// RxRFLevel1 = S3I2CRxGetRFLevel();

	}
	else
		{ err = 74;  goto ENDTEST; }

	if (S3TxGetType(Rx, Tx) == S3_Tx8)
		if (err = select_RF_input((unsigned char)S3Tx8IPMap[0] + 1, false))
			{ err = 13;  goto ENDTEST; }

	if (S3I2CSwitchTestTone(false))
		{ err = 15;  goto ENDTEST; }

	Sleep(400);

	S3I2CReadSerialShort(S3I2C_TX_CTRL_ADDR, 0xD2, &Dummy); // Dummy


	// TxCtrl CRASHES HERE
	if (!S3I2CReadSerialShort(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_RF_MON, &RFLevelBG))
	{
		*v1 = RFLevelBG;
		if (ABS(RFLevel3_16 - RFLevelBG) > 100)
			{ err = 220; goto ENDTEST; }
	}
	else
		{ err = 84;  goto ENDTEST; }

	// ------------------------------------------------------------------------
	// Path 7

	if (set_RF_inp_board(7, 0, Rx, Tx))
		{ err = 11;  goto ENDTEST; }

	Sleep(400);

	if (!S3I2CReadSerialShort(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_RF_MON, &RFLevelBG))
	{
		*v1 = RFLevelBG;
		// RxRFLevel1 = S3I2CRxGetRFLevel();
	}
	else
		{ err = 94;  goto ENDTEST; }

	if (S3TxGetType(Rx, Tx) == S3_Tx8)
		if (err = select_RF_input((unsigned char)S3Tx8IPMap[0] + 1, true))
			{ err = 13;  goto ENDTEST; }

	if (S3I2CSwitchTestTone(true))
		{ err = 15;  goto ENDTEST; }

	Sleep(400);

	if (!S3I2CReadSerialShort(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_RF_MON, &RFLevel7))
	{
		*v2 = RFLevel7;
		if (RFLevel7 - RFLevelBG < 3000)
			{ err = 230; goto ENDTEST; }
	}
	else
		{ err = 95;  goto ENDTEST; }

ENDTEST:
	if (S3TxGetType(Rx, Tx) == S3_Tx8)
		if (select_RF_input(1, false))
			err = 13;

	if (S3I2CSwitchTestTone(false))
		err = 60;

	Sleep(400);

	S3I2CReadSerialShort(S3I2C_TX_CTRL_ADDR, 0xD2, &Dummy); // Dummy

#endif // TRIZEPS

	if (!err)
	{
		S3IPSetGainSent(Rx, Tx, IP, SCHAR_MIN);
		S3Data->m_Rx[Rx].m_Tx[Tx].m_SelfTestPending = false;

		S3TxCancelAlarm(Rx, Tx,
				S3_TX_SELF_TEST_NOT_RUN | S3_TX_SELF_TEST_FAIL);

		if (S3TxGetType(Rx, Tx) == S3_Tx8)
			err = S3I2CTx8SelfTest(v1, v2, Rx, Tx);
	}
	else
	{
		if (err == 50)
		{
			// TODO: Separate "signal connected" alarm?
			// Live input connected? Abandon 
			S3TxCancelAlarm(Rx, Tx, S3_TX_SELF_TEST_FAIL);
			S3TxSetAlarm(Rx, Tx, S3_TX_SELF_TEST_NOT_RUN);

			S3IPSetGainSent(Rx, Tx, IP, SCHAR_MIN);
			S3Data->m_Rx[Rx].m_Tx[Tx].m_SelfTestPending = false;
		}
		else if (S3Data->m_Rx[Rx].m_Tx[Tx].m_SelfTestRetries < 3)
		{
			S3Data->m_Rx[Rx].m_Tx[Tx].m_SelfTestRetries++;

			if (err < 100)
				S3TxSetAlarm(Rx, Tx, S3_TX_SELF_TEST_NOT_RUN);
			else
				S3TxSetAlarm(Rx, Tx, S3_TX_SELF_TEST_FAIL);
		}
		else
		{
			S3TxCancelAlarm(Rx, Tx, S3_TX_SELF_TEST_NOT_RUN | S3_TX_SELF_TEST_FAIL);
			S3TxSetAlarm(Rx, Tx, S3_TX_SELF_TEST_RETRY);

			S3IPSetGainSent(Rx, Tx, IP, SCHAR_MIN);
			S3Data->m_Rx[Rx].m_Tx[Tx].m_SelfTestPending = false;
		}
	}

	return err; // All good
}

// ----------------------------------------------------------------------------

int S3I2CTx8SelfTest(short *v1, short *v2, char Rx, char Tx)
{
	int		err;

	// May have been changed by user
	if (!S3GetTxSelfTest())
		S3Data->m_Rx[Rx].m_Tx[Tx].m_SelfTestPending = false;
		
	if (!S3TxSelfTestPending(Rx, Tx))
		return 0;

#ifdef TRIZEPS

	char	IP = 0;
	short	RFLevel1 = 0;
	short	RFLevelBG = 0;

	// Max out Rx & Tx DSAs
	if (0)
	{
		short RxCal = S3RxGetCalGain(Rx, Tx);
		short TxCal = S3TxGetCalOpt(Rx, Tx);

		err = S3I2CWriteSerialShort(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_DSA, TxCal);
		if (err)
			{ err = 1;  goto ENDTEST; }

		err = S3I2CWriteLocalShort(S3I2CCurRxOptAddr, S3I2C_RX_OPT_DSA, RxCal);
		if (err)
			{ err = 1;  goto ENDTEST; }
	}
	
	// ------------------------------------------------------------------------
	//  Path 1

	if (err = select_RF_input((unsigned char)S3Tx8IPMap[0] + 1, false))
		{ err = 13;  goto ENDTEST; }

	if (set_RF_inp_board(1, 0, Rx, Tx))
		{ err = 11;  goto ENDTEST; }

	Sleep(300);

	if (!S3I2CReadSerialShort(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_RF_MON, &RFLevelBG))
	{
		*v1 = RFLevelBG;
	}
	else
		{ err = 14;  goto ENDTEST; }

	if (RFLevelBG > 0)
		{ err = 50; goto ENDTEST;	}

	if (S3I2CSwitchTestTone(true))
		{ err = 15;  goto ENDTEST; }

	Sleep(400);

	for(char IP = 0; IP < S3TxGetNIP(Rx, Tx); IP++)
	{
		// Test all Tx8 input select relays
		if (IP)
		if (err = select_RF_input((unsigned char)S3Tx8IPMap[IP] + 1, true))
				{ err = 13;  goto ENDTEST; }

		if (!S3I2CReadSerialShort(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_RF_MON, &RFLevel1))
		{
			*v2 = RFLevel1;

			if (RFLevel1 - RFLevelBG < 3000)
			{ err = 200; goto ENDTEST; }
			
			Sleep(400);
		}
		else
			{ err = 14;  goto ENDTEST; }

	}

ENDTEST:

	if (S3I2CSwitchTestTone(false))
		err = 60;
#endif // TRIZEPS

	if (!err)
	{
		S3IPSetGainSent(Rx, Tx, IP, SCHAR_MIN);
		S3Data->m_Rx[Rx].m_Tx[Tx].m_SelfTestPending = false;

		S3TxCancelAlarm(Rx, Tx,
				S3_TX_SELF_TEST_NOT_RUN | S3_TX_SELF_TEST_FAIL);
	}
	else
	{
		if (err == 50)
		{
			// TODO: Separate "signal connected" alarm?
			// Live input connected? Abandon 
			S3TxCancelAlarm(Rx, Tx, S3_TX_SELF_TEST_FAIL);
			S3TxSetAlarm(Rx, Tx, S3_TX_SELF_TEST_NOT_RUN);

			S3IPSetGainSent(Rx, Tx, IP, SCHAR_MIN);
			S3Data->m_Rx[Rx].m_Tx[Tx].m_SelfTestPending = false;
		}
		else if (S3Data->m_Rx[Rx].m_Tx[Tx].m_SelfTestRetries < 3)
		{
			S3Data->m_Rx[Rx].m_Tx[Tx].m_SelfTestRetries++;

			if (err < 100)
				S3TxSetAlarm(Rx, Tx, S3_TX_SELF_TEST_NOT_RUN);
			else
				S3TxSetAlarm(Rx, Tx, S3_TX_SELF_TEST_FAIL);
		}
		else
		{
			S3TxCancelAlarm(Rx, Tx, S3_TX_SELF_TEST_NOT_RUN | S3_TX_SELF_TEST_FAIL);
			S3TxSetAlarm(Rx, Tx, S3_TX_SELF_TEST_RETRY);

			S3IPSetGainSent(Rx, Tx, IP, SCHAR_MIN);
			S3Data->m_Rx[Rx].m_Tx[Tx].m_SelfTestPending = false;
		}
	}

	return err; // All good
}
// ----------------------------------------------------------------------------
// Reads back from Rx only

int S3I2CTxSelfTest2(char Rx, char Tx)
{
	if (!S3GetTxSelfTest())
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
	wbuf[0] = S3I2C_RX_OPT_DSA;
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