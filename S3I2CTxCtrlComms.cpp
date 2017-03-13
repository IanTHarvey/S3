// -----------------------------------------------------------------------------
//
// Copied from S3I2CTx.cpp
//
// Test code for moving internal Tx comms to controller - this should
// make comms more stable with the Tx MCU being tied up less talking to
// the optical board. The downside is there maybe a hit on overall comms
// time.

#ifdef S3_TX_CTRL_COMMS

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

extern unsigned short	S3RevByteUShort(unsigned char *b);
extern short			S3RevByteShort(	unsigned char *b);

extern unsigned char	S3I2CCurOptAddr;

extern unsigned char	S3I2CTxReadBuf[]; // Read from optical serial link
extern unsigned char	S3I2CRxReadBuf[];

extern int S3I2CTxSelfTest(		char Rx, char Tx);
extern int S3I2CTxSelfTest2(	char Rx, char Tx);

int S3I2CTxDumpOptConfig(	char Rx, char Tx);
int S3I2CTxDumpCtrlConfig(	char Rx, char Tx);

int S3I2CTxGetRFCalGain(	char Rx, char Tx);
int S3I2CTxWriteFactoryCal(	char Rx, char Tx); // short *CalPath);

int S3I2CTxDisablePeakDet(		char Rx, char Tx);
int S3I2CTxDisableRFLevelAlarm(	char Rx, char Tx);
extern int S3I2CTxOptAlarmMask();

int	S3I2CSwitchTestTone(bool on);
int S3I2CTxSwitchInput(			char Rx, char Tx, char IP);

// int S3I2CReadSerialStatus();

// Temporary...
int S3I2CTx8Initialise();

// Tx Eng
// const char S3I2CTxRFFact[7] =		{	3,	0,	0,	0,	0,	0,	0	};
// const char S3I2CTxOptFact[7] =		{	7,	0,	5,	-4, -4, -8,	9	};
// const char S3I2CRxOptFact[7] =		{	4,	0,	5,	0,	0,	0,	7	};

// Attenuations
// Tx Eng
//const char S3I2CTxRFFact[7] =		{	0,	0,	0,	0,	0,	0,	0	};
//short S3I2CRxOptFact[7] =			{	400,	0,	500,	0,	0,	0,	700	};

// Tx CEA
// const char S3I2CTxRFFact[7] =	{	0, 0, 0, 0, 0, 0, 0};
const char S3I2CTxRFFact[7] =	{	0, 0, 0, 0, 0, 0, 0}; // ATiS

short S3I2CRxOptFact[7] =		{	400, 0, 500, 0, 0, 0, 700}; // FIXED
// short S3I2CRxOptFact[7] =		{	400, 0, 500, 0, 0, 0, 700}; // FIXED // ATiS

#if !defined(S3_USE_FACT_CAL) || defined(S3_WRITE_FACT_CAL)
// short S3I2CTxOptFact[7] =	{	200, 0, 330, -100, -300, -550, 670}; // CEA
// short S3I2CTxOptFact[7] =	{	200, 0, 330, -100, -300, -550, 670}; // CEA was 'Eng'
// short S3I2CTxOptFact[7] =	{	300, 0, 430,  100, -100, -350, 870}; // Eng

// Path 6 gain is maxed out (at ~-800) and still ~0.7dB low.
short S3I2CTxOptFact[7] =		{	340, 0, 470,  -400, -430, -820, 930}; // ATiS
#else
// short S3I2CTxOptFact[7] =	{	200, 0, 400, -400, -400, -800, 800}; // CEA
//short S3I2CTxOptFact[7] =
//	{	SHRT_MIN,	SHRT_MIN,	SHRT_MIN,	SHRT_MIN,
//		SHRT_MIN,	SHRT_MIN,	SHRT_MIN	};
#endif

extern int S3I2CTxWriteFactoryType(char Rx, char Tx, S3TxType type);
extern int S3I2CTxWriteSNs();

// Should only be used from factory files
extern int	S3I2CTxCtrlSetPassword(	const char *rPW);


// ----------------------------------------------------------------------------
// Use on start-up and insert events (or just insert and use polling to take care)
int S3I2CGetTxStartUp(char Rx, char Tx)
{
#ifdef TRIZEPS
	int err;

#ifdef S3_WRITE_FACT_CAL
	err = S3I2CTxWriteFactoryCal(S3I2CTxOptFact);
#endif

	// err = S3I2CTxCtrlSetPassword("D200");

	// WTF was this here? See below
	// S3TxSetType(Rx, Tx, S3_TxUnconnected);

	// TODO: Look to re-instate
	//err = S3I2CGetTxBatt(Rx, Tx);
	//if (err)
	//	return 1;
	//
	//if (S3TxGetBattSoC(Rx, Tx) < S3_SOC_MIN) // %
	//{
	//	return 2;
	//}

	// ---------------------------------------------------------------------------
	// Control board
	// ---------------------------------------------------------------------------

	// Get Tx identification info (first half)
	unsigned char	i;
	char tmp[20];
	
	if (!S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, 0x00, 18))
	{
		// Extract data from buffer

		/*
		for(i = 0; i < 15; i++) tmp[i] = S3I2CTxReadBuf[i];
		tmp[15] = '\0';
		S3TxSetPN(Rx, Tx, tmp);
		*/
		sprintf_s(tmp, 16, "%c.%c.%c",	(char)S3I2CTxReadBuf[15],
										(char)S3I2CTxReadBuf[16],
										(char)S3I2CTxReadBuf[17]);
		S3TxSetFW(Rx, Tx, tmp);
	}
	else return 3;

	// Part number
	if (!S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, 0x1D, S3I2C_PN_LEN))
	{
		// Extract data from buffer
		for(i = 0; i < S3I2C_PN_LEN; i++) tmp[i] = S3I2CTxReadBuf[i];
		tmp[S3I2C_PN_LEN] = '\0';
		S3TxSetPN(Rx, Tx, tmp);
	}
	else return 3;

	// Second half of identification data
	if (!S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, 0x32, 24))
	{
		for(i = 0; i < S3I2C_SN_LEN; i++) tmp[i] = S3I2CTxReadBuf[i];
		tmp[S3I2C_SN_LEN] = '\0';
		S3TxSetSN(Rx, Tx, tmp);

		// Passive integrator labels
		S3TxSetTauUnits(Rx, Tx, S3I2CTxReadBuf + 0x12);
	}
	else return 4;
	
	// Set compensation mode (from global)
	S3I2CTxSetCompMode(Rx, Tx);

	// S3I2CTxDoComp(Rx, Tx);

	err = S3I2CTxGetRFCalGain(Rx, Tx);
	if (err)
		return 5;

	// ---------------------------------------------------------------------------
	// Optical board
	// ---------------------------------------------------------------------------

	// Frig I2C map
	err = S3I2CTxDisablePeakDet(Rx, Tx);
	err = S3I2CTxDisableRFLevelAlarm(Rx, Tx);

	err = S3I2CTxGetWavelength(Rx, Tx);
	err = S3I2CTxGetOptCalGain(Rx, Tx);

	if (err)
		return 6;

	// ---------------------------------------------------------------------------
	// Battery
	S3I2CTxGetBattSN(Rx, Tx);

	char IP = S3TxGetActiveIP(Rx, Tx);
	S3IPSetGainSent(Rx, Tx, IP, -128); // Force update

	// Force switch to active input
	S3I2CTxSwitchInput(Rx, Tx, IP);

	err = S3I2CTxSelfTest2(Rx, Tx);
	if (err)
	{
		if (err < 100)
			S3TxSetAlarm(Rx, Tx, S3_TX_SELF_TEST_NOT_RUN);
		else
			S3TxSetAlarm(Rx, Tx, S3_TX_SELF_TEST_FAIL);

		if (S3I2CSwitchTestTone(false))
			return 1;
	}
	else
		S3TxCancelAlarm(Rx, Tx,
			S3_TX_SELF_TEST_NOT_RUN | S3_TX_SELF_TEST_FAIL);
#endif

#ifdef S3_TX_DIAGS
	// One-off writes
	// S3I2CWriteLaserBias();
	// 	err = S3I2CTxWriteFactoryType(Rx, Tx, S3_Tx1);
	
	// S3I2CTxOptAlarmMask();

	// err = S3I2CTxWriteSNs();
	// Sleep(1000);
	// err = S3I2CTxGetRFCalGain(Rx, Tx);
	// err = S3I2CTxWriteFactoryCal(0, 0);

	err = S3I2CTxDumpOptConfig(Rx, Tx);
	err = S3I2CTxDumpCtrlConfig(Rx, Tx);
#endif

	return 0;
} // S3I2CGetTxStartUp

// ----------------------------------------------------------------------------
// Get optical board calibration value
int S3I2CTxGetOptCalGain(char Rx, char Tx)
{
	short cal;
	int err = S3I2CReadSerialShort(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_CAL_GAIN, &cal);

	if (err)
		return 1;

	S3TxSetCalOpt(Rx, Tx, cal);

	return 0;
}

// -----------------------------------------------------------------------------

int S3I2CTxGetRFCalGain(char Rx, char Tx)
{
	for(char i = 0; i < 7; i++)
	{
#ifdef S3_USE_FACT_CAL
		// Get form Tx
		int err = S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, 0xA8 + i * 2, 2);

		if (err)
			return 1;

		short cal;
		*((char *)&cal + 0) = S3I2CTxReadBuf[1];
		*((char *)&cal + 1) = S3I2CTxReadBuf[0];

		// S3I2CTxOptFact[i] = cal;
		S3TxSetCalRF(Rx, Tx, i, cal);
#else
		// Get from code
		S3TxSetCalRF(Rx, Tx, i, S3I2CTxOptFact[i]);
#endif

	}
	
	return 0;
}

// ----------------------------------------------------------------------------
// Path must be set on Tx before calling this

// TODO: Not apparently implemented on Tx control board. This is a
// placeholder only.

int S3I2CTxGetCalGain2(char Rx, char Tx, char IP)
{
	int		err;
	short	cal;
	
	if (1)
	{
		// TODO: Cannot write to this region, with/without password, cf
		// SN field which is writable without password

		// err = S3I2CWriteSerialStr(S3I2C_TX_CTRL_ADDR, S3I2C_PW_ADDR, "D200");
		// Sleep(500);
		// char Data[7] = {10, 20, 30, 40, 50, 60, 70};
		// err = S3I2CWriteSerialStr(S3I2C_TX_CTRL_ADDR, 0xA8, Data, 7);

		//Sleep(500);

		err = S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, 0xA8, 9);

		if (err)
			return 1;
		
		// These are static and do not reflect Tx opt calibration
		*((char *)&cal + 0) = S3I2CTxReadBuf[8];
		*((char *)&cal + 1) = S3I2CTxReadBuf[7];

		S3TxSetCalOpt(Rx, Tx, cal);
	}
	else if (0)
	{
		err = S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_OPT_CAL_GAIN, 2);

		if (err)
			return 1;

		*((char *)&cal + 0) = S3I2CTxReadBuf[1];
		*((char *)&cal + 1) = S3I2CTxReadBuf[0];

		S3TxSetCalOpt(Rx, Tx, cal);
	}

	return 0;
}

// ----------------------------------------------------------------------------
// TODO: Check for write errors
int S3I2CTxSetCompMode(char Rx, char Tx)
{
	unsigned char mode = S3TxGetTCompMode(Rx, Tx);

	if (mode >= 100) // Pending
	{
		mode -= 100;
		
		int err;

		// TEST:
		switch(mode)
		{
		case S3_TCOMP_OFF:
			err = S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR,
					S3I2C_TX_CTRL_TCOMP, 0x00);
			break; // None
		case S3_TCOMP_CONT:
			err = S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR,
					S3I2C_TX_CTRL_TCOMP, 0x02);
			break; // Continuous
		case S3_TCOMP_GAIN:
			err = S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR,
					S3I2C_TX_CTRL_TCOMP, 0x01);
			break; // Path change
		default: return 1;
		}

		if (!err)
			S3TxSetTCompMode(Rx, Tx, mode); // Signal completion
		else
			return 1;
	}

	return 0;
}

// ----------------------------------------------------------------------------
// Perform temperature compensation (bit reset by Tx on completion) 

int S3I2CTxDoComp(char Rx, char Tx)
{
	// Update compensation temp if successful
	if (S3Data->m_Rx[Rx].m_Tx[Tx].m_TempComp != SCHAR_MIN)
		return 0;

	int err = S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_CTRL_TCOMP, 1);

	if (!err)
		S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR,
			S3I2C_TX_CTRL_TCOMP, S3I2CTxReadBuf[0] | 0x80);

	if (!err)
	{
		S3TxSetTempComp(Rx, Tx, S3Data->m_Rx[Rx].m_Tx[Tx].m_TempTx);
		S3TxCancelAlarm(Rx, Tx, S3_TX_RECOMP_REQ);
	}

	Sleep(100);

	if (err)
		S3EventLogAdd("Forced T comp failed", 3, Rx, Tx, -1);

	return err;
}

// ----------------------------------------------------------------------------
// TEST: Overwrite corrupted control bits
// 
// Always check these if gain controls goes wonky - some form of compensation
// is applied and gain seen to wander.

int S3I2CTxSetOptCtrlBits(char Rx, char Tx)
{
	int err = 0;

	// Check for corrupted config
	err = S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, 0xCC, 2);

	// Mask the 5 MSBs which seem to get over-written randomly, but should
	// be harmless - otherwise get loads of log entries. b3 switches with
	// the temperature compensation mode.
	if ((*S3I2CTxReadBuf & 0x7) != 0x04 && (*S3I2CTxReadBuf & 0x7) != 0) 
	{
		char msg[S3_EVENTS_LINE_LEN];

		sprintf_s(msg, S3_EVENTS_LINE_LEN, "Tx optical config 0xCC OoB: 0x%02x",
							*S3I2CTxReadBuf);

		S3EventLogAdd(msg, 1, Rx, Tx, -1);

		if (1)
		{
			S3EventLogAdd("Correcting TEC control bits (clear b0 & b1)", 1, Rx, Tx, -1);
			err = S3I2CWriteSerialByte(S3I2C_TX_OPT_ADDR, 0xCC, *S3I2CTxReadBuf & ~0x03);
		}
	}

	if (*(S3I2CTxReadBuf + 1) != 0x08) // SGC
	{
		char msg[S3_EVENTS_LINE_LEN];

		sprintf_s(msg, S3_EVENTS_LINE_LEN, "Tx optical config 0xCD OoB: 0x%02x",
						*(S3I2CTxReadBuf + 1));

		S3EventLogAdd(msg, 1, Rx, Tx, -1);
		
		if (1)
		{
			S3EventLogAdd("Correcting control bits (set SGC)", 1, Rx, Tx, -1);
			err = S3I2CWriteSerialByte(S3I2C_TX_OPT_ADDR, 0xCD, 0x08);
		}
	}

	return err;
}


// ----------------------------------------------------------------------------

int S3I2CTxSetPowerStat(char Rx, char Tx)
{
	unsigned char stat = S3TxGetPowerStat(Rx, Tx);

	int err = 0;
		
	if (stat == S3_TX_SLEEP_PENDING)
	{
		err = S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR,
										S3_TX_CTRL_SLEEP, S3_TX_SLEEP_FULL);
		if (!err)
			S3TxSetPowerStat(Rx, Tx, S3_TX_SLEEP);
	}
	else if (stat == S3_TX_ON_PENDING)
	{
		err = S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR,
										S3_TX_CTRL_SLEEP, S3_TX_SLEEP_NONE);
		if (!err)
			S3TxSetPowerStat(Rx, Tx, S3_TX_ON);
	}

	return err;
}

// ----------------------------------------------------------------------------

int S3I2CTxSetStatus(char Rx, char Tx)
{
	int err = 0;

	err = S3I2CTxSetPowerStat(Rx, Tx);

	if (err)
		S3EventLogAdd("S3I2CTxSetPowerStat: Failed to set power mode", 3, Rx, Tx, -1);

	// Don't wake sleeping Tx
	if (S3TxGetPowerStat(Rx, Tx) == S3_TX_SLEEP)
		return 0;

	char IP = S3TxGetActiveIP(Rx, Tx);

	if (IP >= 100)
	{
		IP -= 100;
		err = S3I2CTxSwitchInput(Rx, Tx, IP);

		if (err)
			S3EventLogAdd("S3I2CTxSwitchInput: RF input switch failed", 3, Rx, Tx, -1);
	}

	err = S3I2CSetIPGain(Rx, Tx, IP);
	
	if (err)
	{
		switch(err)
		{
			case 1:		S3EventLogAdd("S3I2CSetIPGain: Generic fail", 3, Rx, Tx, -1); break;
			case 2:		S3EventLogAdd("S3I2CSetIPGain: Path fail", 3, Rx, Tx, -1); break;
			case 3:		S3EventLogAdd("S3I2CSetIPGain: RF DSA fail", 3, Rx, Tx, -1); break;
			case 4:		S3EventLogAdd("S3I2CSetIPGain: Rx DSA fail", 3, Rx, Tx, -1); break;
			case 5:		S3EventLogAdd("S3I2CSetIPGain: Tx DSA fail", 3, Rx, Tx, -1); break;
			default:	S3EventLogAdd("S3I2CSetIPGain: Unknown error", 3, Rx, Tx, -1); break;
		}
	}

	S3I2CTxSetCompMode(Rx, Tx);
	S3I2CTxSetTestTone(Rx, Tx, IP);

	S3I2CTxDoComp(Rx, Tx);

	return 0;
}

// ----------------------------------------------------------------------------

int S3I2CTxGetStatus(char Rx, char Tx)
{
	int	err = 0;

	if (S3TxGetPowerStat(Rx, Tx) == S3_TX_SLEEP_PENDING)
		return 0;

	short RLL = S3RxGetRLL(Rx, Tx);

	if (S3TxGetPowerStat(Rx, Tx) == S3_TX_SLEEP)
	{
		// Has Tx been restarted?
		if (RLL < S3_RLL_TX_ALIVE)
			return 0; // No

		// Make connection
		S3TxSetPowerStat(Rx, Tx, S3_TX_ON);
	}

	// Now look for Txs on the on the serial link(s)
	S3TxType Old = S3TxGetType(Rx, Tx);

	// Use this as an indicator that a Tx1 is attached to the Rx
	// if (S3RxGetRLL(Rx, Tx) > S3_RLL_TX_ALIVE)
	err = S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_CTRL_TX_TYPE, 1);

	if (!err)
	{
		// TODO: Kludge? Change Tx F/W behaviour.
		// This is for behaviour introduced in TxCtrl F/W 17Nov16. When the FOL
		// goes dark, the Tx restarts but without RF/optical output (RLL = bugger
		// all). S3_TX_CTRL_SLEEP register does not reflect this, so
		// S3_TX_SLEEP_PART must be sent first so S3_TX_SLEEP_NONE triggers a
		// change.
		if (RLL < S3_RLL_TX_ALIVE)
		{
			err = S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR,
										S3_TX_CTRL_SLEEP, S3_TX_SLEEP_PART);

			Sleep(1000);

			err = S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR,
										S3_TX_CTRL_SLEEP, S3_TX_SLEEP_NONE);
			// Allow time to wake up?
			Sleep(1000);

			// Catch on next poll
			return 0;
		}

		// If it wasn't here before?
		if (Old == S3_TxUnconnected)
		{
			switch(S3I2CTxReadBuf[0])
			{
			case S3_Tx1:
				S3TxInserted(Rx, Tx, S3_Tx1);
				break;
			case S3_Tx8:
				S3TxInserted(Rx, Tx, S3_Tx8);
				S3I2CTx8Initialise();
				break;
			default:
				S3EventLogAdd("Mis-read of Tx type", 3, Rx, Tx, -1);
				S3TxSetAlarm(Rx, Tx, S3_TX_INIT_FAIL);
				S3TxRemoved(Rx, Tx);
				Sleep(1000);

				return 5;
			}

			// Not really necessary?
			S3TxCancelAlarm(Rx, Tx, S3_TX_INIT_FAIL);
			
			err = S3I2CGetTxStartUp(Rx, Tx);
		
			if (err)
			{
				char msg[S3_EVENTS_LINE_LEN];
				sprintf_s(msg, S3_EVENTS_LINE_LEN, 
					"S3I2CGetTxStartUp: Failed with error %d", err);
				S3EventLogAdd(msg, 3, Rx, Tx, -1);
				S3TxSetAlarm(Rx, Tx, S3_TX_INIT_FAIL);

				return err;
			}
			else S3TxCancelAlarm(Rx, Tx, S3_TX_INIT_FAIL);

			S3EventLogAdd("Tx init success", 1, Rx, Tx, -1);
		}
	}
	else
	{
		// Was it attached before?
		if (Old != S3_TxUnconnected)
			S3TxRemoved(Rx, Tx);

		return 0;
	}

	// Get alarms as priority
	err = S3I2CTxGetOptAlarms(Rx, Tx);
	if (err)
		return err;

	// TODO: Establish when implemented
	err = S3I2CTxGetCtrlAlarms(Rx, Tx);
	if (err)
		return err;

	// --------------------------------------------------------------------
	// TxCtrl board temperature

	err = S3I2CGetTxTemp(Rx, Tx);
	if (err) // TODO: S3_TX_TEMP_COMM_FAIL
		S3TxBattSetAlarm(Rx, Tx, S3_TX_COMM_FAIL);
	else
		S3TxBattCancelAlarm(Rx, Tx, S3_TX_COMM_FAIL);

	err = S3I2CGetTxBatt(Rx, Tx);
	if (err)
		S3TxBattSetAlarm(Rx, Tx, S3_TX_BATT_COMM_FAIL);
	else
		S3TxBattCancelAlarm(Rx, Tx, S3_TX_BATT_COMM_FAIL);
	
	// TODO: Re-test with JB's updated H/W & F/W
	// May not talk reliably, so consider disconnected
	/*
	if (S3TxGetBattSoC(Rx, Tx) < S3_SOC_MIN) // %
	{
		S3EventLogAdd("Tx battery SoC below reliable threshold", 3, Rx, Tx, -1);
		S3TxRemoved(Rx, Tx);
		return 2;
	}*/

	err = S3I2CTxGetLaserPow(Rx, Tx);
	if (err) // TODO: S3_TX_OPT_COMM_FAIL
		S3TxSetAlarm(Rx, Tx, S3_TX_COMM_FAIL);
	else
		S3TxCancelAlarm(Rx, Tx, S3_TX_COMM_FAIL);

	S3I2CTxSetOptCtrlBits(Rx, Tx);

	// Catch-all for Kludge above
	S3TxCancelAlarm(Rx, Tx, S3_TX_INIT_FAIL);

	// DIAG
	// S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_RF_IP, 1);
	// char t = (char)S3I2CTxReadBuf[0];

	return err;
} // S3I2CTxGetStatus

// ----------------------------------------------------------------------------
// This is the temperature against which compensation is applied
int S3I2CGetTxTemp(char Rx, char Tx)
{
#ifdef TRIZEPS

	// ------------------------------------------------------------------------
	// Read TxCtrl board temperature
	if (!S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_CTRL_TEMP, 1))
	{
		// char T = (char)ROUND(((double)S3I2CTxReadBuf[0] * 256 + (double)S3I2CTxReadBuf[1]) / 256.0);
		S3TxSetTemp(Rx, Tx, S3I2CTxReadBuf[0]);
	}

	/*
	// TEC temperature
	if (!S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_TEMP, 2))
	{
		char T = (char)ROUND(((double)S3I2CTxReadBuf[0] * 256 + (double)S3I2CTxReadBuf[1]) / 256.0);
		S3TxSetTemp(Rx, Tx, T);
	}
	*/
#endif

	return 0;
}

// ----------------------------------------------------------------------------

int S3I2CGetTxBatt(char Rx, char Tx)
{
#ifdef TRIZEPS

	// ------------------------------------------------------------------------
	// Read Battery. SoC = 0x02
	if (!S3I2CReadSerialData(S3I2C_TX_BATT_ADDR, 0x02, 16))
	{
		S3TxSetBattSoC(Rx, Tx, S3I2CTxReadBuf[0]);

		short Ts = *(short *)(S3I2CTxReadBuf + 10);
		char T = (char)((double)Ts / 10.0 - 273);

		S3TxSetBattTemp(Rx, Tx, T);
		S3TxSetBattI(Rx, Tx, *(short *)(S3I2CTxReadBuf + 14));
	}
	else return 1;

	// Average time to empty (0x18)
	if (!S3I2CReadSerialData(S3I2C_TX_BATT_ADDR, 0x18, 0x02))
	{
		S3TxSetATTE(Rx, Tx, *((unsigned short *)S3I2CTxReadBuf));
	}
	else return 1;

#endif

	return 0;
}

// ----------------------------------------------------------------------------
// Read from 'Manufacturer Info' block (A 0-31)

int S3I2CTxGetBattSN(char Rx, char Tx)
{
	// BlockDataControl	
	S3I2CWriteSerialByte(S3I2C_TX_BATT_ADDR, 0x61, 0x00);
	// DataFlashClass 58: Manufacturer info
	S3I2CWriteSerialByte(S3I2C_TX_BATT_ADDR, 0x3E, 58);
	// DataFlashBlock 0: Offset
	S3I2CWriteSerialByte(S3I2C_TX_BATT_ADDR, 0x3F, 0x00);

	if (!S3I2CReadSerialData(S3I2C_TX_BATT_ADDR, 0x40, 24))
	{
		S3TxSetBattInfo(Rx, Tx, (char *)(S3I2CTxReadBuf + 0),
								(char *)(S3I2CTxReadBuf + 10), NULL, NULL);
			
		return 0;
	}

	return 1;
}

// ----------------------------------------------------------------------------
// ...and RFLevel & RFGain
int S3I2CTxGetLaserPow(char Rx, char Tx)
{
#ifdef TRIZEPS
	if (!S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, 0xD6, 8))
	{
		S3TxSetLaserPow(Rx, Tx, S3RevByteShort(S3I2CTxReadBuf));

		char IP = S3TxGetActiveIP(Rx, Tx);
		S3IPSetRFLevel(Rx, Tx, IP, S3RevByteShort(S3I2CTxReadBuf + 4));
		S3IPSetRFGain(Rx, Tx, IP, S3RevByteShort(S3I2CTxReadBuf + 6));
	}
#endif

	// TEST:	0xCC-D:		Tx-Opt config bits
	//			0xD0->:		Monitor and alarms
	//			0xDA-B->:	RF level
	//			0xDC-D->:	RF gain
	//			0xBE-C1:	Gain compensation temp and slope
	
	//int err;
	//err = S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, 0xD4, 2); // Bias
	//err = S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, 0xE2, 2); // Laser temp
	//err = S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, 0xB2, 2); // Laser temp
	//err = S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, 0xA0, 2); // Bias set

	//err = S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, 0xDA, 4);
	// err = S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, 0xBE, 4);
	// err = S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, 0xD0, 48);

	return 0;
}

// ----------------------------------------------------------------------------

int S3I2CTxGetOptAlarms(char Rx, char Tx)
{
#ifdef TRIZEPS
	if (!S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_ALARMS, 3))
	{
		// TODO: Temp disable peak alarm - this should be masked by
		// 
		S3I2CTxReadBuf[1] &= ~S3_TX_OPT_PEAK;

		// Handle alarms...
		S3TxOptSetAlarm(Rx, Tx, S3I2CTxReadBuf);
	}
	else return 1;

	// ----------- ONE-OFF ONLY -----------
	if (0) // Rx == 0)
	{
		S3I2CWriteSerialShort(S3I2C_TX_OPT_ADDR, 0x7C, 500);
		Sleep(100);
		S3I2CWriteSerialShort(S3I2C_TX_OPT_ADDR, 0x7E, -5800);

		int err = S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, 0x7C, 4);
		short v = S3RevByteShort(S3I2CTxReadBuf + 0);
		v = S3RevByteShort(S3I2CTxReadBuf + 2);

		err = S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, 0xE6, 2);
		v = S3RevByteShort(S3I2CTxReadBuf + 0);
	}

	if (0)
	{
		short v1, v2;

		int err = S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, 0xBE, 4);
		
		v1 = S3RevByteShort(S3I2CTxReadBuf + 0);
		v2 = S3RevByteShort(S3I2CTxReadBuf + 2);
	}


	/*
	int err = S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, 0xDA, 2);

	short w;
	*((char *)&w + 0) = S3I2CTxReadBuf[1];
	*((char *)&w + 1) = S3I2CTxReadBuf[0];

	double p = (double)w / 100.0;
	*/

#endif

	return 0;
}


// ----------------------------------------------------------------------------
// INFO: This does not work - may be some latency in Tx ctrl board ISR?

int	S3I2CSetPathA(char path, char atten)
{
	unsigned char buf[2];

	buf[0] = path;
	buf[1] = atten;

	return S3I2CWriteSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_RF_PATH, buf, 2);
}

// ----------------------------------------------------------------------------

int S3I2CTxGetCtrlAlarms(char Rx, char Tx)
{
#ifdef TRIZEPS
	if (!S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_CTRL_ALARMS, 2))
	{
		// Handle alarms...
		S3TxCtrlSetAlarm(Rx, Tx, S3I2CTxReadBuf);

		// TEST: 0-6 path calibration values, 7-8 combined RF + Tx gain
		// factor.
		// int err  = S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, 0xA8, 9);
	
		// return err;
	}
	else return 1;
#endif

	return 0;
}

// ----------------------------------------------------------------------------
// TODO: Remove

int S3I2CTxGetBattStatus()
{
	return 0;
}

// ----------------------------------------------------------------------------

int S3I2CSetTxCalibration(double val)
{
	// Send calibration value to Tx map
	return 0;
}

// ----------------------------------------------------------------------------

int S3I2CTxGetWavelength(char Rx, char Tx)
{
	int err = S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_WLENGTH, 2);

	if (err)
		return 1;

	short w;
	*((char *)&w + 0) = S3I2CTxReadBuf[1];
	*((char *)&w + 1) = S3I2CTxReadBuf[0];

	switch(w)
	{
	case 1310:	S3TxSetWavelength(Rx, Tx, S3_1310nm); break;
	case 1550:	S3TxSetWavelength(Rx, Tx, S3_1550nm); break;
	default:	S3TxSetWavelength(Rx, Tx, S3_L_Unknown); break;
	}

	return 0;
}

// ----------------------------------------------------------------------------

int S3I2CTxSetTestTone(char Rx, char Tx, char IP)
{
	int err = 0;
#ifdef TRIZEPS
	char TestIP = S3TxGetTestToneIP(Rx, Tx);

	if (TestIP >= 99) // Update pending?
		TestIP -= 100;
	else
		return 0;
	
	if (TestIP == IP)
	{
		err = S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR,
			S3I2C_TX_CTRL_TEST_TONE, 0x80);

		// Write directly to SC16
		//I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 0x4f);
		//I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 0x07);	
		//I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, STOP);
	}
	else
	{
		err = S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR,
			S3I2C_TX_CTRL_TEST_TONE, 0x00);

		//I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 0x4f);
		//I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 0x03);	
		//I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, STOP);
	}

	if (!err)
		S3TxSetTestToneIP(Rx, Tx, TestIP); // Reset to ack
#endif			
	return err;
}

// ----------------------------------------------------------------------------

int S3I2CTxSwitchInput(char Rx, char Tx, char NewIP)
{
#ifdef TRIZEPS
	
	if (1)
	{
		// S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_RF_IP, 1);

		// char t1 = (char)S3I2CTxReadBuf[0];

		// if (t1 > 8)
		//	return 4;
		
		if (S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR, S3I2C_TX_RF_IP,
											(unsigned char)S3Tx8IPMap[NewIP] + 1))
			return 1;

		// Sleep(500);

		//if (S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_RF_IP, 1))
		//	return 2;

		//char t = (char)S3I2CTxReadBuf[0];

		//if (t != S3Tx8IPMap[NewIP] + 1)
		//	return 3;

		S3TxSetActiveIP(Rx, Tx, NewIP); // Ack
		S3IPSetGainSent(Rx, Tx, NewIP, SCHAR_MIN);
	}
	else
	{
	// TEST TEST TEST
	// Set IO to output
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, START);
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, S3_RF8_EXPDR_ADDR);
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 4);
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 0x8C);
	
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 0x00);
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 0x00);
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 0x00);

	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, STOP);


	// Set all ports to 0
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, START);
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, S3_RF8_EXPDR_ADDR);
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 4);
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 0x84);
	
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 0x00);
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 0x00);
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 0x00);

	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, STOP);

	Sleep(50);

	// Set RFn as input
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, START);
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, S3_RF8_EXPDR_ADDR);
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 4);
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 0x84);
	
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 0x22);
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 0x10);
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 0x01);

	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, STOP);

	Sleep(50);

	// Pulse off
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, START);
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, S3_RF8_EXPDR_ADDR);
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 4);
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 0x84);
	
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 0x00);
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 0x00);
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 0x00);

	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, STOP);

	Sleep(200);
	}

	// TEST TEST TEST

#endif // TRIZEPS

	return 0;
}

// ----------------------------------------------------------------------------
// Temporary - should be done by Tx8 firmware?

int S3I2CTx8Initialise()
{
	return 0;

#ifdef TRIZEPS
	// Set all ports to 0
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, START);
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, S3_RF8_EXPDR_ADDR);
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 4);
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 0x84);
	
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 0x00);
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 0x00);
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 0x00);

	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, STOP);

	// Set all ports to outputs
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, START);
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, S3_RF8_EXPDR_ADDR);
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 4);
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 0x8C);
	
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 0x00);
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 0x00);
	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, 0x00);

	I2C_WriteRandom(S3I2CCurOptAddr, 0x00 << 3, STOP);

#endif // TRIZEPS

	return 0;
}

// ----------------------------------------------------------------------------

short S3I2CRxGetRFLevel()
{
#ifdef TRIZEPS
	unsigned char wbuf = S3I2C_RX_OPT_MON + 6;
	BOOL ok = I2C_WriteRead(S3I2CRxOptAddr, &wbuf, 1, S3I2CRxReadBuf, 2);
	
	return S3RevByteShort(S3I2CRxReadBuf);
#else
	return SHRT_MIN;
#endif
}

// ----------------------------------------------------------------------------

int S3I2CSwitchTestTone(bool on)
{
	return 0;
	
	unsigned char regval = 0x00;
	if (on)
		regval = 0x80;

	// Turn on tone
	char retry = 0;
	
	do
	{
		S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR,
				S3I2C_TX_CTRL_TEST_TONE, regval);

		Sleep(150);

		S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_CTRL_TEST_TONE, 1);
	}
	while(*S3I2CTxReadBuf != regval && retry++ < 5);

	if (retry == 5)
		return 1;

	return 0;
}

// ----------------------------------------------------------------------------

#endif // S3_TX_CTRL_COMMS