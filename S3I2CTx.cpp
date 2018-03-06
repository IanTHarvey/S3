// See S3I2CTxCtrlComms.cpp for rival implementation

#ifndef S3_TX_CTRL_COMMS

// ----------------------------------------------------------------------------
// Tx Comms over SC16IS740 I2C serial bridge to TxOpt and TxCtrl boards and
// battery.

#include "stdafx.h"
#include "S3DataModel.h"

extern pS3DataModel S3Data;

#ifdef TRIZEPS
#include "drvLib_app.h"
#endif

#include "S3I2C.h"
#include "S3Gain.h"

extern unsigned short	S3RevByteUShort(unsigned char *b);
extern short			S3RevByteShort(	unsigned char *b);

extern unsigned char	S3I2CCurTxOptAddr;

extern unsigned char	S3I2CTxReadBuf[S3_SERIAL_FIFO_LEN]; // Read from optical serial link
extern unsigned char	S3I2CRxReadBuf[];

extern int S3I2CTxSelfTest(		short *v1, short *v2, char Rx, char Tx);
extern int S3I2CTx8SelfTest(	short *v1, short *v2, char Rx, char Tx);
extern int S3I2CTxSelfTest2(	char Rx, char Tx);

int S3I2CTxGetRFCalGain(		char Rx, char Tx);
int S3I2CTxWriteFactoryCal(		char Rx, char Tx); // short *CalPath);

// int S3I2CTxDisablePeakDet(		char Rx, char Tx);
int S3I2CTxDisableRFLevelAlarm(	char Rx, char Tx);
extern int S3I2CTxOptAlarmMask();

int	S3I2CSwitchTestTone(		bool on);
int S3I2CTxSwitchInput(			char Rx, char Tx, char IP);

extern int S3I2CTxOptSetPassword(const char *rPW);

int S3I2CTxFixTau(char Rx, char Tx);

int S3I2CTxSwitchLaser();

// int S3I2CReadSerialStatus();

// Temporary...
int S3I2CTx8Initialise();

#define bit_clear(A, B)	((A) = ((A) & ~(0x01 << (B))))
#define bit_set(A, B)	((A) = ((A) |  (0x01 << (B))))
#define bit_test(A, B)	((A) &  (0x01 << (B)))

short TempCompTable[S3_TX_N_RF_PATH] =
	{292,	213,	133,	133,	133,	133,	133};

extern short PeakThTable[S3_TX_N_RF_PATH];

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


// Where did these come from? 
// TODO: Experiment with removal.
short S3I2CRxOptFact[7] =		{	400, 0, 500, 0, 0, 0, 700}; // FIXED

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

// DIAG:
int S3I2CTxSetBias(char Rx, char Tx);

extern int S3I2CTxWriteFactoryType(char Rx, char Tx, S3TxType type);

// Should only be used from factory files
extern int	S3I2CTxCtrlSetPassword(	const char *rPW);

// JB RF1 & 8 direct GPIO routines
int RF_i2c(BYTE I2C_address, unsigned char *buf);
int select_RF_input(unsigned char input, bool test_tone_on);

int S3I2CTxInit();
int S3I2CGetTxWakeUp(char Rx, char Tx);

// ----------------------------------------------------------------------------
// Use on start-up and insert events (or just insert and use polling to take care)
int S3I2CGetTxStartUp(char Rx, char Tx)
{
#ifdef TRIZEPS
	int err;

	// ---------------------------------------------------------------------------
	// Control board
	// ---------------------------------------------------------------------------

	// If Tx power-cycled

	// Get Tx identification info (first half)
	unsigned char	i;
	char tmp[S3I2C_PN_LEN + 1];
	
	if (!S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR,
							S3I2C_TX_CTRL_FW_V, 3 + S3I2C_FWDATE_LEN))
	{
		// Extract data from buffer
		sprintf_s(tmp, S3I2C_PN_LEN + 1, "%c.%c.%c",
										(char)S3I2CTxReadBuf[0],
										(char)S3I2CTxReadBuf[1],
										(char)S3I2CTxReadBuf[2]);
		S3TxSetFW(Rx, Tx, tmp);

		strncpy_s(tmp, S3I2C_PN_LEN + 1, (char *)(S3I2CTxReadBuf + 3), S3I2C_FWDATE_LEN);
		tmp[S3I2C_FWDATE_LEN] = '\0';

		S3TxSetFWDate(Rx, Tx, tmp);
	}
	else return 3;

	// Part number
	if (!S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_CTRL_PN, S3I2C_PN_LEN))
	{
		// Extract data from buffer
		for(i = 0; i < S3I2C_PN_LEN; i++) tmp[i] = S3I2CTxReadBuf[i];
		tmp[S3I2C_PN_LEN] = '\0';
		S3TxSetPN(Rx, Tx, tmp);

		// TODO: This should be redundant due to JB F/W fix?
		if (tmp[0] != 'S')
			S3EventLogAdd("PN corrupted", 3, Rx, Tx, -1);
	}
	else return 3;

	// Second half of identification data
	if (!S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_CTRL_SN, 24))
	{
		for(i = 0; i < S3I2C_SN_LEN; i++) tmp[i] = S3I2CTxReadBuf[i];
		tmp[S3I2C_SN_LEN] = '\0';
		S3TxSetSN(Rx, Tx, tmp);

		// Passive integrator labels
		S3TxSetTauUnits(Rx, Tx, S3I2CTxReadBuf + 0x12);
	}
	else return 4;
	
	err = S3I2CTxGetRFCalGain(Rx, Tx);
	if (err)
		return 5;

#ifdef S3_TX_DIAGS
	// S3I2CWriteLaserBias();

	// err = S3I2CTxSetBias(Rx, Tx);
	// err = S3I2CTxWriteFactoryType(Rx, Tx, S3_Tx1);
	

	// err = S3I2CTxGetRFCalGain(Rx, Tx);

	// err = S3I2CTxDumpCtrlConfig(Rx, Tx);
#endif

	
	// ------------------------------------------------------------------------
	// Battery
	// ------------------------------------------------------------------------
	S3I2CTxGetBattSN(Rx, Tx);

	unsigned char StartState = S3GetTxStartState();

	if (!S3TxGetBattValidated(Rx, Tx))
		StartState = S3_TXSTART_SLEEP;

	if (StartState == S3_TXSTART_SLEEP)
	{
		// No need to do anything
		S3TxSetPowerStat(Rx, Tx, S3_TX_SLEEP);
	}
	else if (StartState == S3_TXSTART_USER)
	{
		if (S3TxGetPowerStat(Rx, Tx) == S3_TX_SLEEP)
		{
			// If shutdown by system, not user
			if (!S3TxGetUserSleep(Rx, Tx))
				S3TxSetPowerStat(Rx, Tx, S3_TX_ON);
		}
		else // Not shut down by system (ie S3TxGetPowerStat = S3_TX_ON)
		{
			// Force restart
			// TODO: This short-cuts S3TxSetPowerStat
			S3Data->m_Rx[Rx].m_Tx[Tx].m_PowerStat = S3_TX_ON_PENDING;
		}		
	}
	else if (StartState == S3_TXSTART_ON)
	{
		// Force restart
		// TODO: This short-cuts S3TxSetPowerStat
		S3Data->m_Rx[Rx].m_Tx[Tx].m_PowerStat = S3_TX_ON_PENDING;
	}

	// ------------------------------------------------------------------------
	// TxOpt
	// ------------------------------------------------------------------------
	
	// NO. CANNOT READ ANYTHING FROM TxOpt HERE, AS MAYBE NOT AWAKE


	
	// err = S3I2CTxInit();

#endif // TRIZEPS

	return 0;
} // S3I2CGetTxStartUp

// ----------------------------------------------------------------------------

int S3I2CGetTxWakeUp(char Rx, char Tx)
{
#ifdef TRIZEPS
	int err;

	// ------------------------------------------------------------------------
	// Optical board
	// ------------------------------------------------------------------------

	err = S3I2CTxGetWavelength(Rx, Tx);
	err = S3I2CTxGetOptCalGain(Rx, Tx);

	if (!S3I2CReadSerialData(S3I2C_TX_OPT_ADDR,	S3I2C_TX_OPT_FW_V, 3))
	{
		S3TxOptSetFW(Rx, Tx, S3I2CTxReadBuf);
	}

	err = S3I2CTxGetPeakThresh(Rx, Tx);

	if (err)
		return 1;

	// This has moved to gain setting
	// S3TxClearPeakHold(Rx, Tx, 0); // Force clear
	// S3I2CTxPeakHoldLatchClear(Rx, Tx);

	// ------------------------------------------------------------------------
	// RF board(s)
	// ------------------------------------------------------------------------

	char IP = S3TxGetActiveIP(Rx, Tx);
	S3IPSetGainSent(Rx, Tx, IP, SCHAR_MIN); // Force update

	char ToneEnabled = S3IPGetTestToneEnable(Rx, Tx, IP);
	
	if (ToneEnabled)
		S3IPSetTestToneEnable(Rx, Tx, IP, ToneEnabled + 100);

	S3TxSetTCompMode(Rx, Tx, S3TxGetTCompMode(Rx, Tx) + 100);

	if (0)
	{
		short v1, v2;
		err = S3I2CTxSelfTest(&v1, &v2, Rx, Tx);
		if (err)
		{
			if (err < 100)
				S3TxSetAlarm(Rx, Tx, S3_TX_SELF_TEST_NOT_RUN);
			else
				S3TxSetAlarm(Rx, Tx, S3_TX_SELF_TEST_FAIL);
		}
		else
		{
			S3TxCancelAlarm(Rx, Tx,
			S3_TX_SELF_TEST_NOT_RUN | S3_TX_SELF_TEST_FAIL);
		}
	}

#ifdef S3_TX_DIAGS
	// S3I2CTxOptAlarmMask();
	err = S3I2CTxDumpCtrlConfig(Rx, Tx);
	err = S3I2CTxDumpOptConfig(Rx, Tx);
#endif

#endif // TRIZEPS

	return 0;
} // S3I2CGetTWakeUp

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
// Main polling loop Tx temp and compensation function

int S3I2CTxUpdateTemp(char Rx, char Tx)
{
	unsigned char mode = S3TxGetTCompMode(Rx, Tx);

	if (mode < S3_PENDING) // Mode change pending
	{
		char Tnow = S3TxGetTemp(Rx, Tx); // S3I2CTxCtrlGetTemp();

		if (S3Data->m_Rx[Rx].m_Tx[Tx].m_TempChange)
		{
			if (mode == S3_TCOMP_CONT)
			{
				int err;
			
				// Send temp to optical board
				err = S3I2CWriteSerialShort(S3I2C_TX_OPT_ADDR,
					S3I2C_TX_OPT_TCOMP_T, (short)Tnow * 256);
			}
		}
		
		if (mode == S3_TCOMP_GAIN)
		{
			S3I2CTxDoComp(Rx, Tx);
		}
	}
	else
	{
		return S3I2CTxSetCompMode(Rx, Tx);
	}

	return 0;
}

// ----------------------------------------------------------------------------
// Called only on change to RF path or compensation mode or to force update...

int S3I2CTxUpdateTempPath(char Rx, char Tx)
{
	unsigned char mode = S3TxGetTCompMode(Rx, Tx);
	char path = S3IPGetPathSent(Rx, Tx, S3TxGetActiveIP(Rx, Tx));

	if (mode == S3_TCOMP_OFF)
		return 0;

	int err;

	// Send slope to TxOpt
	err = S3I2CWriteSerialShort(S3I2C_TX_OPT_ADDR,
				S3I2C_TX_OPT_TCOMP_M, TempCompTable[path - 1]);

	if (mode == S3_TCOMP_CONT) // It'll get done anyway by S3I2CTxUpdateTemp
		return 0; 

	char Told = S3TxGetTemp(Rx, Tx);
	
	// Send temp to TxOpt
	err = S3I2CWriteSerialShort(S3I2C_TX_OPT_ADDR,
				S3I2C_TX_OPT_TCOMP_T, (short)Told * 256);
	
	S3TxSetTempComp(Rx, Tx, Told);

	return 0;
}

// ----------------------------------------------------------------------------
// Set peak threshold according to path, on path change.
// Password required.

/*
int S3I2CTxSetPeakThreshOld(char Rx, char Tx, char path)
{
	int err = S3I2CWriteSerialShort(S3I2C_TX_OPT_ADDR,
				S3I2C_TX_OPT_PEAK_H, PeakThTable[path - 1]);

	return err;
}
*/

// ----------------------------------------------------------------------------

int S3I2CTxSetPeakThresh(char Rx, char Tx, char path)
{
	int err = 0;
	short thresh = 13000; // PeakThTable[path - 1]
	
	err = S3I2CWriteSerialShort(S3I2C_TX_OPT_ADDR,
				S3I2C_TX_OPT_PEAK_THR, thresh);

	Sleep(100);

	// Read back
	if (!err)
	{
		short	rval;
		err = S3I2CReadSerialShort(S3I2C_TX_OPT_ADDR,
				S3I2C_TX_OPT_PEAK_THR, &rval);
		if (!err)
		{
			if (rval != thresh)
				err = 10;
		}
	}

	return err;
}

// ----------------------------------------------------------------------------
/*
int S3I2CTxFixBias(char Rx, char Tx)
{
	int	err;
	
	err = S3I2CTxOptSetPassword("D200");

	//int err = S3I2CWriteSerialShort(S3I2C_TX_OPT_ADDR,
	//			S3I2C_TX_OPT_BIAS_H, -10536);

	//Sleep(500);

	//err = S3I2CWriteSerialShort(S3I2C_TX_OPT_ADDR,
	//			S3I2C_TX_OPT_BIAS_H + 2, 5500);

	//Sleep(500);

	err = S3I2CWriteSerialShort(S3I2C_TX_OPT_ADDR,
				0xA0, 15000);

	Sleep(500);

	//short h, l;
	//err = S3I2CReadSerialShort(S3I2C_TX_OPT_ADDR,
	//			S3I2C_TX_OPT_BIAS_H, &h);

	//err = S3I2CReadSerialShort(S3I2C_TX_OPT_ADDR,
	//			S3I2C_TX_OPT_BIAS_H + 2, &l);

	short b;

	err = S3I2CReadSerialShort(S3I2C_TX_OPT_ADDR,
				0xA0, &b);

	return err;
}
*/

int S3I2CTxFixTau(char Rx, char Tx)
{
	int	err;
	
	err = S3I2CTxCtrlSetPassword("D200");

	err = S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR,
				0x45, 2);

	Sleep(500);

	unsigned char tau1, tau2;

	err = S3I2CReadSerialByte(S3I2C_TX_CTRL_ADDR, 0x44, &tau1);

	err = S3I2CReadSerialByte(S3I2C_TX_CTRL_ADDR, 0x45, &tau2);

	return err;
}

// ----------------------------------------------------------------------------
// TODO: Assumes compensation mode has changed

int S3I2CTxSetCompMode(char Rx, char Tx)
{
	unsigned char mode = S3TxGetTCompMode(Rx, Tx);

	if (mode >= S3_PENDING) // Pending
	{
		mode -= S3_PENDING;
		
		int err;

		unsigned char CtrlReg;
		err = S3I2CReadSerialByte(S3I2C_TX_OPT_ADDR,
					S3I2C_TX_OPT_TCOMP_ON, &CtrlReg);

		if (err)
			return 1;

		switch(mode)
		{
		case S3_TCOMP_OFF:
			err = S3I2CWriteSerialByte(S3I2C_TX_OPT_ADDR,
					S3I2C_TX_OPT_TCOMP_ON, CtrlReg & ~0x04);
			break; // None
		case S3_TCOMP_CONT:
			err = S3I2CWriteSerialByte(S3I2C_TX_OPT_ADDR,
					S3I2C_TX_OPT_TCOMP_ON, CtrlReg | 0x04);

			S3I2CTxUpdateTempPath(Rx, Tx);
			break; // Continuous
		case S3_TCOMP_GAIN:
			err = S3I2CWriteSerialByte(S3I2C_TX_OPT_ADDR,
					S3I2C_TX_OPT_TCOMP_ON, CtrlReg | 0x04);

			S3I2CTxUpdateTempPath(Rx, Tx);
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
// Update temperature compensation parameters

int S3I2CTxDoComp(char Rx, char Tx)
{
	// Update compensation temp if successful
	if (S3Data->m_Rx[Rx].m_Tx[Tx].m_TempComp != SCHAR_MIN)
		return 0;

	int err = S3I2CTxUpdateTempPath(Rx, Tx);

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
	err = S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_CFG, 2);

	// Mask the 5 MSBs which seem to get over-written randomly, but should
	// be harmless - otherwise get loads of log entries. b3 switches with
	// the temperature compensation mode.
	unsigned char cfg = *S3I2CTxReadBuf;
	if ((cfg & 0x07) != 0x04 && (cfg & 0x07) != 0x00 && (cfg & 0x07) != 0x05) 
	{
		char msg[S3_EVENTS_LINE_LEN];

		sprintf_s(msg, S3_EVENTS_LINE_LEN, "Tx optical config 0xCC OoB: 0x%02x",
							*S3I2CTxReadBuf);

		S3EventLogAdd(msg, 1, Rx, Tx, -1);

		if (1)
		{
			S3EventLogAdd("Correcting TEC control bits (clear b0 & b1)", 1, Rx, Tx, -1);
			err = S3I2CWriteSerialByte(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_CFG, cfg & ~0x03);
		}
	}

	if (0) // ITH: 06-06-17 *(S3I2CTxReadBuf + 1) != 0x08) // SGC
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
										S3_TX_CTRL_SLEEP, S3_TX_SLEEP_PART);
		if (!err)
			S3TxSetPowerStat(Rx, Tx, S3_TX_SLEEP);
	}
	else if (stat == S3_TX_ON_PENDING)
	{
		err = S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR,
										S3_TX_CTRL_SLEEP, S3_TX_SLEEP_NONE);
		if (!err)
		{
			// 500ms OK except on system start up where 1000ms required otherwise
			// 16dB PAD is in circuit for attenuation paths.
			Sleep(1000);

			S3TxSetPowerStat(Rx, Tx, S3_TX_ON);
			err = S3I2CGetTxWakeUp(Rx, Tx);
			
			if (err)
				return err;
			
			return -1;
		}
		else
			return err;
	}

	return err;
}

// ----------------------------------------------------------------------------

int S3I2CTxSetStatus(char Rx, char Tx)
{
	int err = 0;

	err = S3I2CTxSetPowerStat(Rx, Tx);

	if (err > 0)
		S3EventLogAdd("S3I2CTxSetPowerStat: Failed to set power mode", 3, Rx, Tx, -1);

	// If Tx just woken up, give it time to get its shit together
	if (err == -1)
		return 0;

	// Don't wake sleeping Tx
	if (S3TxGetPowerStat(Rx, Tx) == S3_TX_SLEEP)
		return 0;

	char IP = S3TxGetActiveIP(Rx, Tx);

	if (IP >= S3_PENDING)
	{
		IP -= S3_PENDING;
		err = S3I2CTxSwitchInput(Rx, Tx, IP);

		if (err)
			S3EventLogAdd("S3I2CTxSwitchInput: RF input switch failed", 3, Rx, Tx, -1);
	}

	// Ensure AGC correction is performed on stable RLL
	// if (S3Data->m_Rx[Rx].m_Tx[Tx].m_Uptime == S3_RLL_WARMUP_POLLS && S3GetAGC() == S3_AGC_GAIN)
	//	S3IPSetGainSent(Rx, Tx, IP, SCHAR_MIN);

	err = S3I2CSetIPGain(Rx, Tx, IP);
	
	if (err)
	{
		switch(err)
		{
			case 1:	S3EventLogAdd("S3I2CSetIPGain: Generic fail", 3, Rx, Tx, -1); break;
			case 2:	S3EventLogAdd("S3I2CSetIPGain: Path fail", 3, Rx, Tx, -1); break;
			case 3:	S3EventLogAdd("S3I2CSetIPGain: RF DSA fail", 3, Rx, Tx, -1); break;
			case 4:	S3EventLogAdd("S3I2CSetIPGain: Rx DSA fail", 3, Rx, Tx, -1); break;
			case 5:	S3EventLogAdd("S3I2CSetIPGain: Tx DSA fail", 3, Rx, Tx, -1); break;
			default:S3EventLogAdd("S3I2CSetIPGain: Unknown error", 3, Rx, Tx, -1); break;
		}
	}

	S3I2CTxSetCompMode(Rx, Tx); // IFF changed
	S3I2CTxUpdateTemp(Rx, Tx);
	S3I2CTxDoComp(Rx, Tx);

	if (1)
	{
		short v1, v2;
		err = S3I2CTxSelfTest(&v1, &v2, Rx, Tx);

		// Alarms handled internally
		if (0)
		{
			if (err)
			{
				if (err < 100)
					S3TxSetAlarm(Rx, Tx, S3_TX_SELF_TEST_NOT_RUN);
				else
					S3TxSetAlarm(Rx, Tx, S3_TX_SELF_TEST_FAIL);
			}
			else
			{
				S3TxCancelAlarm(Rx, Tx,
				S3_TX_SELF_TEST_NOT_RUN | S3_TX_SELF_TEST_FAIL);
			}
		}
	}

	S3I2CTxSetTestTone(Rx, Tx, IP);

	if (S3Data->m_Rx[Rx].m_Tx[Tx].m_Uptime > 0)
	{
		// short v1, v2;
		// err = S3I2CTxSelfTest(&v1, &v2, Rx, Tx);

	}
	
	return 0;
}

// ----------------------------------------------------------------------------
// Top-level Tx poll function

int S3I2CTxGetStatus(char Rx, char Tx)
{
	if (S3Data->m_Rx[Rx].m_Tx[Tx].m_Uptime < UCHAR_MAX)
		S3Data->m_Rx[Rx].m_Tx[Tx].m_Uptime++;
	
	S3TimerStart(0);

	int	err = 0;

	// Now look for Txs on the on the serial link(s)
	S3TxType Old = S3TxGetType(Rx, Tx);

	unsigned char Type;
	err = S3I2CReadSerialByte(S3I2C_TX_CTRL_ADDR, S3I2C_TX_CTRL_TX_TYPE, &Type);

	if (!err)
	{
		// If it wasn't here before?
		if (Old == S3_TxUnconnected)
		{
			switch(Type)
			{
			case S3_Tx1:
				S3TxInserted(Rx, Tx, S3_Tx1);
				break;
			case S3_Tx8:
				S3TxInserted(Rx, Tx, S3_Tx8);
				break;
			default:
				S3EventLogAdd("Mis-read of Tx type", 3, Rx, Tx, -1);
				S3TxSetAlarm(Rx, Tx, S3_TX_INIT_FAIL);
				S3TxRemoved(Rx, Tx);
				Sleep(1000);

				err = 5;
				goto RETERR;
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

				goto RETERR;
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

		goto RETERR;
	}

	// TODO: Establish when implemented
	err = S3I2CTxGetCtrlAlarms(Rx, Tx);
	if (err)
		goto RETERR;

	// --------------------------------------------------------------------
	// TxCtrl 
	
	// Board temperature
	err = S3I2CGetTxTemp(Rx, Tx);
	if (err) // TODO: S3_TX_TEMP_COMM_FAIL
		S3TxSetAlarm(Rx, Tx, S3_TX_TEMP_COMM_FAIL);
	else
		S3TxCancelAlarm(Rx, Tx, S3_TX_TEMP_COMM_FAIL);

	err = S3I2CGetTxBatt(Rx, Tx);
	if (err)
		S3TxBattSetAlarm(Rx, Tx, S3_TX_BATT_COMM_FAIL);
	else
		S3TxBattCancelAlarm(Rx, Tx, S3_TX_BATT_COMM_FAIL);

		// TODO: Move to after battery if JB can enable I2C battery comms in
	// partial sleep mode
	if (S3TxGetPowerStat(Rx, Tx) != S3_TX_ON)
		goto RETERR;

	// --------------------------------------------------------------------
	// TxOpt

	// S3I2CTxPeakHoldClearSet(Rx, Tx);

	// Get alarms as priority
	err = S3I2CTxGetOptAlarms(Rx, Tx);
	if (err)
		goto RETERR;
	
	err = S3I2CTxGetLaserPow(Rx, Tx);
	if (err) // TODO: S3_TX_OPT_COMM_FAIL
		S3TxSetAlarm(Rx, Tx, S3_TX_COMM_FAIL);
	else
		S3TxCancelAlarm(Rx, Tx, S3_TX_COMM_FAIL);

	// TODO: Is this redundant? Yes
	// S3I2CTxSetOptCtrlBits(Rx, Tx);

	// Catch-all for Kludge above
	S3TxCancelAlarm(Rx, Tx, S3_TX_INIT_FAIL);


#ifdef S3_TEMP_LOG
	//DIAG:
	S3I2CTempLog(Rx, Tx);
#endif

RETERR:
	S3TimerStop(0);
	return err;
} // S3I2CTxGetStatus

// ----------------------------------------------------------------------------
// This is the temperature against which compensation is applied
int S3I2CGetTxTemp(char Rx, char Tx)
{
#ifdef TRIZEPS

	// ------------------------------------------------------------------------
	// Read TxCtrl board temperature from main I2C map 
	// if (!S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_CTRL_TEMP, 1))
	
	// Read TxCtrl board temperature direct from temperature chip 
	char T = S3I2CTxCtrlGetTemp();

	if (T != SCHAR_MIN)
	{
		S3TxSetTemp(Rx, Tx, T);
	}

	// TEC temperature - not used for temperature compensation
	if (!S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_TEMP, 2))
	{
		S3TxSetTempTEC(Rx, Tx, S3RevByteShort(S3I2CTxReadBuf));
	}

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

		short Ts = *(short *)(S3I2CTxReadBuf + 10) - 2730;

		S3TxSetBattTemp(Rx, Tx, Ts);
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
	int AuthFail = S3I2CTxAuthenticate(Rx, Tx);

	S3TxSetBattValidated(Rx, Tx, AuthFail == 0);

	int err;

	// Get status registers
	unsigned char cmd[] = {0x00, 0x00};
	err = S3I2CWriteSerialData(S3I2C_TX_BATT_ADDR, 0x00, cmd, 2);
	err = S3I2CReadSerialData(S3I2C_TX_BATT_ADDR, 0x00, 0x02);

	unsigned char stat_h = *(S3I2CTxReadBuf + 1);

	if (!(stat_h & BQ_SS)) // Unsealed
	{
		// BlockDataControl	
		S3I2CWriteSerialByte(S3I2C_TX_BATT_ADDR, 0x61, 0x01);
		// DataFlashClass 58: Manufacturer info
		S3I2CWriteSerialByte(S3I2C_TX_BATT_ADDR, 0x3E, 58);
	}

	// DataFlashBlock 0: Offset
	S3I2CWriteSerialByte(S3I2C_TX_BATT_ADDR, 0x3F, 0x01);


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
	if (!S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_MON + 6, 8))
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

int S3I2CTxGetPeakThresh(char Rx, char Tx)
{
	if (!S3TxGetPeakHoldCap(Rx, Tx))
		return 0;

	if (!S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_PEAK_THR, 2))
	{
		S3TxSetPeakThresh(Rx, Tx, S3RevByteShort(S3I2CTxReadBuf));
	}

	return 0;
}

// ----------------------------------------------------------------------------

int S3I2CTxPeakHoldLatchClear(char Rx, char Tx)
{
	if (S3TxGetClearPeakHold(Rx, Tx))
	{
		unsigned char cfg;
		int err  = S3I2CReadSerialByte(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_CFG, &cfg);
		cfg &= ~0x01;
		
		err  = S3I2CWriteSerialByte(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_CFG, cfg);

		// If don't do this, nothing detected
		if (1)
		{
			Sleep(100);

			cfg |= 0x01;
			err  = S3I2CWriteSerialByte(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_CFG, cfg);

			if (!err)
				S3TxClearPeakHold(Rx, Tx, 1); // Ack

			return err;
		}
	}

	return 0;
}

// ----------------------------------------------------------------------------
// Set in max attenuation
int S3I2CTxSetSafeMode(char Rx, char Tx)
{	
	return 0;
	
	char IP = S3TxGetActiveIP(Rx, Tx);

	// Set gain to minimum and force immediate update
	S3SetGain(Rx, Tx, IP, S3_MIN_GAIN);
	S3IPSetGainSent(Rx, Tx, IP, SCHAR_MIN);
	int err = S3I2CSetIPGain(Rx, Tx, IP);

	return err;
}

// ----------------------------------------------------------------------------

int S3I2CTxGetOptAlarms(char Rx, char Tx)
{
#ifdef TRIZEPS
	if (!S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_ALARMS, S3_TX_OPT_ALARM_BYTES + 2))
	{
		unsigned char PeakAlarmLatchStatus;

		// Handle alarms...
		S3TxOptSetAlarm(Rx, Tx, S3I2CTxReadBuf);

		if (S3TxGetPeakHoldCap(Rx, Tx))
		{
			PeakAlarmLatchStatus = *(S3I2CTxReadBuf + S3_TX_OPT_ALARM_BYTES + 1);
		
			if (PeakAlarmLatchStatus & 0x02)
			{
				S3TxClearPeakHold(Rx, Tx, 0); // Force clear
			}

			unsigned char alarm = *(S3I2CTxReadBuf + 1);
			if (alarm & 0x10)
			{
				int err = S3I2CTxSetSafeMode(Rx, Tx);

				char IP = S3TxGetActiveIP(Rx, Tx);

				S3IPSetAlarm(Rx, Tx, IP, S3_IP_OVERDRIVE);

				// We've seen the latched alarm, so reset it
				S3TxClearPeakHold(Rx, Tx, 0); // Force latch reset
				S3I2CTxPeakHoldLatchClear(Rx, Tx);
			}
		}
		else
		{

		}
	}
	else return 1;

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

char S3I2CTxCtrlGetTemp()
{
	int err = S3I2CReadSerialData(S3I2C_TX_CTRL_TEMP_ADDR, 0, 1);

	if (!err)
		return (char)S3I2CTxReadBuf[0];

	return SCHAR_MIN;
}

// ----------------------------------------------------------------------------

int S3I2CTxSetTestTone(char Rx, char Tx, char IP)
{
	if (!S3TxRLLStable(Rx, Tx))
		return 0;

	int err = 0;
#ifdef TRIZEPS
	char ToneEnabled = S3IPGetTestToneEnable(Rx, Tx, IP);

	if (ToneEnabled >= S3_PENDING) // Update pending?
		ToneEnabled -= S3_PENDING;
	else
		return 0;
		
	// Switching of routing to RF1 input or RF8 board done by hardware mod
	// to RF1 board in Tx8 application.

	if (S3TxGetType(Rx, Tx) != S3_Tx8)
	{
	}
	else
	{
		err = select_RF_input(IP, ToneEnabled == 1);
	}

	err = S3I2CSwitchTestTone(ToneEnabled == 1);

	if (!err)
		S3IPSetTestToneEnable(Rx, Tx, IP, ToneEnabled); // Reset to ack

#endif			
	return err;
}

// ----------------------------------------------------------------------------
// For Tx1, mostly dummy except when called in Startup for all Tx types, which
// may switch test tone on/off depending on saved state.
int S3I2CTxSwitchInput(char Rx, char Tx, char NewIP)
{
	int err = 0;
#ifdef TRIZEPS

	char ToneEnabled = S3IPGetTestToneEnable(Rx, Tx, NewIP);
	
	// If not already pending
	if (ToneEnabled < S3_PENDING)
		S3IPSetTestToneEnable(Rx, Tx, NewIP, ToneEnabled + S3_PENDING);

	if (S3TxGetType(Rx, Tx) != S3_Tx8)
	{
	}
	else
	{
		err = select_RF_input((unsigned char)S3Tx8IPMap[NewIP],
			ToneEnabled == 1);

		if (err)
			return err;
	}

	S3TxSetActiveIP(Rx, Tx, NewIP); // Ack
	S3IPSetGainSent(Rx, Tx, NewIP, SCHAR_MIN);

#endif // TRIZEPS

	return err;
}

// ----------------------------------------------------------------------------
// Write to RF8 board I2C expander GPIO
// Obsolete as RF1 and RF8 expanders are initialised by TxCtrl firmware.

int S3I2CTx8Initialise()
{
	return 0;

#ifdef TRIZEPS
	// Set all ports to 0
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, START);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, S3_RF8_EXPDR_ADDR);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 4);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 0x84);
	
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 0x00);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 0x00);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 0x00);

	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, STOP);

	// Set all ports to outputs
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, START);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, S3_RF8_EXPDR_ADDR);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 4);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 0x8C);
	
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 0x00);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 0x00);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 0x00);

	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, STOP);

#endif // TRIZEPS

	return 0;
}

// ----------------------------------------------------------------------------

short S3I2CRxGetRFLevel()
{
#ifdef TRIZEPS
	unsigned char wbuf = S3I2C_RX_OPT_MON + 6;
	BOOL ok = I2C_WriteRead(S3I2CCurRxOptAddr, &wbuf, 1, S3I2CRxReadBuf, 2);
	
	return S3RevByteShort(S3I2CRxReadBuf);
#else
	return SHRT_MIN;
#endif
}

// ----------------------------------------------------------------------------
// For Tx1 & Tx8. For Tx8 routing of test-tone to input must also be set up
// via select_RF_input().

int S3I2CSwitchTestTone(bool on)
{
	unsigned char regval = 0x00;
	if (on)
		regval = 0x80;

	// Turn on RF1 tone oscillator via TxCtrl map
	int err = S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR,
			S3I2C_TX_CTRL_TEST_TONE, regval);

	return 0;
}

// ----------------------------------------------------------------------------
// From JB F/W

// Select RF input on Tx8 switch board. Test tone is routed if required, but
// tone is not turned on here.

int select_RF_input(unsigned char input, bool test_tone_on)
{
	unsigned char buf[3];

	switch (input)
	{
		case 0:
			buf[0] = 0x20;
			buf[1] = 0x90;
			buf[2] = 0x0D;
			break;
		case 1:
			buf[0] = 0x88;
			buf[1] = 0x08;
			buf[2] = 0x05;
			break;
		case 2:
			buf[0] = 0x21;
			buf[1] = 0x10;
			buf[2] = 0x07;
			break;
		case 3:
			buf[0] = 0x04;
			buf[1] = 0x14;
			buf[2] = 0x09;
			break;
		case 4:
			buf[0] = 0x88;
			buf[1] = 0x40;
			buf[2] = 0x03;
			break;
		case 5:
			buf[0] = 0x92;
			buf[1] = 0x00;
			buf[2] = 0x0F;
			break;
		case 6:
			buf[0] = 0x04;
			buf[1] = 0x30;
			buf[2] = 0x0B;
			break;
		case 7:
			buf[0] = 0xD0;
			buf[1] = 0x00;
			buf[2] = 0x01;
			break;
	}
	
	// Pulse start
	int err = RF_i2c(S3_RF8_EXPDR_ADDR, buf);

	// Pulse stop
	if (!err)
	{
		// delay_ms(30);
		Sleep(30);

		buf[0] = 0;
		buf[1] = 0;

		if (test_tone_on == true)
			bit_clear(buf[2], 0);

	 	if (!RF_i2c(S3_RF8_EXPDR_ADDR, buf))
			return 0;
		else
			return 1;
	}
	else
		return 1;	
}

// ----------------------------------------------------------------------------
// Function sends three bytes from the *buf to the I2C @RF_BOARD_ADD
// ----------------------------------------------------------------------------

int RF_i2c(unsigned char I2C_address, unsigned char *buf)
{
#ifdef TRIZEPS
	//unsigned char i = 0xFF;

	//while((i2c_lock) && (i))
	//{
	//	i--;
	//	delay_us(100);
	//}

	if (1)
	{
		// disable_interrupts(INT_SSP);
		/*
		SSPEN = 0;
		i2c_start(AUX);
		i2c_write(AUX, I2C_address | WRITE_FLAG);
		i2c_write(AUX, EXP_OUT_PORT0);
		i2c_write(AUX, buf[0]);
		i2c_write(AUX, buf[1]);
		i2c_write(AUX, buf[2]);
		i2c_stop(AUX);
		SSPEN=1;*/

		// enable_interrupts(INT_SSP);

		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, START);
		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, I2C_address);
		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 4);
		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 0x84);
		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, buf[0]);
		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, buf[1]);
		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, buf[2]);

		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, STOP);
	}

#endif // TRIZEPS

	return 0;
}

// ----------------------------------------------------------------------------
// functions sets the input board switching relays and DSAs
//		path - selects correct path (options 1 to 7)
//		attenuation - attenuation value
//		*buf - pointer to the buffer with command bytes
// ----------------------------------------------------------------------------

void RF_path_set(	unsigned char	path,
					unsigned char	attenuation,
					unsigned char	*buf,
					char Rx, char Tx)
{
	switch (path)
	{
		case 0:
			break;
		case 1:	
			buf[0] = 0x2A;
			buf[1] = 0x50;
			buf[2] = 0x04;
			if (bit_test(attenuation,4)) bit_set(buf[0],2);
			if (bit_test(attenuation,3)) bit_set(buf[1],0);
			if (bit_test(attenuation,2)) bit_set(buf[1],2);
			if (bit_test(attenuation,1)) bit_set(buf[0],4);
			if (bit_test(attenuation,0)) bit_set(buf[0],0);
			break;
		case 2:	
			buf[0] = 0x02;
			buf[1] = 0x40;
			buf[2] = 0x06;
			break;
		case 3:	
			buf[0] = 0x40;
			buf[1] = 0x00;
			buf[2] = 0x71;
			attenuation &= 0x30;
			if (attenuation == 16) buf[2] = 0xB1;
			if (attenuation == 32) buf[2] = 0x59;
			if (attenuation == 48) buf[2] = 0x99;
			break;
		case 4:	
			if (S3Data->m_Rx[Rx].m_Tx[Tx].m_OldTauOrder)
			{
				buf[0] = 0xC8;
				buf[1] = 0x28;
				buf[2] = 0x71;
			}
			else
			{
				buf[0] = 0x88;
				buf[1] = 0x2A;
				buf[2] = 0x71;
			}
			attenuation &= 0x30;
			if (attenuation == 16) buf[2] = 0xB1;
			if (attenuation == 32) buf[2] = 0x59;
			if (attenuation == 48) buf[2] = 0x99;
			break;
		case 5:	
			if (S3Data->m_Rx[Rx].m_Tx[Tx].m_OldTauOrder)
			{
				buf[0] = 0x88;
				buf[1] = 0x2A;
				buf[2] = 0x71;
			}
			else
			{
				buf[0] = 0xC8;
				buf[1] = 0x28;
				buf[2] = 0x71;
			}

			attenuation &= 0x30;
			if (attenuation == 16) buf[2] = 0xB1;
			if (attenuation == 32) buf[2] = 0x59;
			if (attenuation == 48) buf[2] = 0x99;
			break;
		case 6:
			buf[0] = 0x08;
			buf[1] = 0x22;
			buf[2] = 0x71;
			attenuation &= 0x30;
			if (attenuation == 16) buf[2] = 0xB1;
			if (attenuation == 32) buf[2] = 0x59;
			if (attenuation == 48) buf[2] = 0x99;
			break;
		case 7:
			buf[0] = 0x08;
			buf[1] = 0xA0;
			buf[2] = 0x10;
			break;
	}
}

// ----------------------------------------------------------------------------
//functions resets the input board switching relays and DSAs
//		path - selects correct path (options 1to 7)
//		attenuation - attenuation value
//		*buf - pointer to the buffer with command bytes
// ----------------------------------------------------------------------------

void RF_path_reset(	unsigned char path,
					unsigned char attenuation,
					unsigned char *buf,
					char Rx, char Tx)
{
	switch (path)
	{
		case 0:	;
			break;
		case 1:	
			buf[0] = 0x0A;
			buf[1] = 0x50;
			buf[2] = 0x00;
			if (bit_test(attenuation,4)) bit_set(buf[0],2);
			if (bit_test(attenuation,3)) bit_set(buf[1],0);
			if (bit_test(attenuation,2)) bit_set(buf[1],2);
			if (bit_test(attenuation,1)) bit_set(buf[0],4);
			if (bit_test(attenuation,0)) bit_set(buf[0],0);
			break;
		case 2:	
			buf[0] = 0x02;
			buf[1] = 0x40;
			buf[2] = 0x02;
			break;
		case 3:	
			buf[0] = 0x40;
			buf[1] = 0x00;
			buf[2] = 0x00;
			break;
		case 4:	
			if (S3Data->m_Rx[Rx].m_Tx[Tx].m_OldTauOrder)
			{
				buf[0] = 0xC8;
				buf[1] = 0x28;
				buf[2] = 0x00;
			}
			else
			{
				buf[0] = 0x88;
				buf[1] = 0x2A;
				buf[2] = 0x00;
			}
			break;
		case 5:
			if (S3Data->m_Rx[Rx].m_Tx[Tx].m_OldTauOrder)
			{
				buf[0] = 0x88;
				buf[1] = 0x2A;
				buf[2] = 0x00;
			}
			else
			{
				buf[0] = 0xC8;
				buf[1] = 0x28;
				buf[2] = 0x00;
			}
			break;
		case 6:	;
			buf[0] = 0x08;
			buf[1] = 0x22;
			buf[2] = 0x00;
			break;
		case 7:	;
			buf[0] = 0x08;
			buf[1] = 0x20;
			buf[2] = 0x00;
			break;
	}
}

// ----------------------------------------------------------------------------
//function setting up the RF input board
//		path - selects correct path (options 1to 7)
//		attenuation - attenuation value
// ----------------------------------------------------------------------------

int set_RF_inp_board(unsigned char  path, unsigned char attenuation, char Rx, char Tx)
{
	unsigned char buffer[3];
	int err;

	RF_path_set(path, attenuation, buffer, Rx, Tx);
 	err = RF_i2c(S3_RF1_EXPDR_ADDR, buffer);
	
	if (err)
		return err;

	// if (set_peak_threshold(path, attenuation) == NOK)
	//	return 0;

	//att_cal_apply(path);
	// delay_ms(30);

	Sleep(30);

	RF_path_reset(path, attenuation, buffer, Rx, Tx);
	err = RF_i2c(S3_RF1_EXPDR_ADDR, buffer);

	return err;
}



// ----------------------------------------------------------------------------

int S3I2CTxInit()
{
	// Set IO to output
/*
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, START);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, S3_RF1_EXPDR_ADDR);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 4);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 0x8C);
	
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 0x00);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 0x00);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 0x00);

	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, STOP);

	// Set all ports to 0
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, START);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, S3_RF1_EXPDR_ADDR);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 4);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 0x84);
	
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 0x00);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 0x00);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 0x00);

	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, STOP);
*/
	
	return 0;
}

// ----------------------------------------------------------------------------

int S3I2CTxSwitchLaser(bool on)
{
	int err;
	unsigned char rcfg, wcfg;

	err = S3I2CReadSerialByte(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_CFG + 1, &rcfg);

	if (on)
		wcfg = rcfg & ~0x01;
	else
		wcfg = rcfg | 0x01;
	
	err = S3I2CWriteSerialByte(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_CFG + 1, wcfg);

	return err;
}

// ----------------------------------------------------------------------------


#endif // NOT S3_TX_CTRL_COMMS
