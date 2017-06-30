// ----------------------------------------------------------------------------
// Gain setting functions controlling: RxOptDSA, TxPath, TxRFDSA, TxOptDSA

#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "S3DataModel.h"


#ifdef TRIZEPS
#include "drvLib_app.h"
#endif

#include "S3I2C.h"
#include "S3Gain.h"

extern char S3TxGetRFPath(char Rx, char Tx, char IP);

extern int set_RF_inp_board(unsigned char  path, unsigned char attenuation,
							char Rx, char Tx);

extern pS3DataModel S3Data;
char	S3I2CCurPath; // 1-indexed

#ifdef TRIZEPS
extern unsigned char	S3I2CTxReadBuf[S3_SERIAL_FIFO_LEN]; // Read from optical serial link
#endif


// Debug vars

unsigned char	Dbg_RF1_DSA = 0;
short			Dbg_TxOpt = 0;	// Gain soft set 0xA8-9
short			Dbg_RxOpt = 0;	// Gain soft set 0xA4-5
short			Dbg_RLL = 0;
char			Dbg_Path = 0;

// ----------------------------------------------------------------------------

int	S3I2CSetPath(char path)
{
#ifdef TRIZEPS
	int err = S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR, S3I2C_TX_RF_PATH, path);
	
	if (err)
		return err;

	// err = S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_RF_PATH, 1);
	// char p = *S3I2CTxReadBuf;
#ifdef S3_READ_BACK
	err = S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_RF_PATH, 1);

	if (err)
		return 2;

	if (S3I2CTxReadBuf[0] != path)
		return 3;
#endif

#endif // TRIZEPS

	return 0;
}

// ----------------------------------------------------------------------------
// Tx Ctrl (RF). How atten arg used depends on selected RF path.
unsigned char S3I2CCalcRFAtten(char atten)
{
	unsigned char val = 0;

	if (S3I2CCurPath == 1)
	{
		val = (unsigned char)atten + 6;
		// val -= S3I2CTxRFFact[S3I2CCurPath - 1];
	}
	else if (S3I2CCurPath == 2)
	{
		// Nothing settable on this RF path
		val = 0xFF;
	}
	else
	{
		// Controls the 16 & 32dB PADs
		val = (unsigned char)atten;
	}

	return val;
}

// ----------------------------------------------------------------------------
// Tx Ctrl (RF). Applied value is attenuation.
// How atten arg used depends on selected RF path.

int	S3I2CSetRFAtten(char atten, char Rx, char Tx)
{
#ifdef TRIZEPS
	unsigned char val;

	if (S3I2CCurPath == 1)
	{
		val = (unsigned char)atten + 6;
		// val -= S3I2CTxRFFact[S3I2CCurPath - 1];
	}
	else if (S3I2CCurPath == 2)
	{
		// Nothing settable on this RF path
		val = 0xFF;
	}
	else
	{
		// Controls the 16 & 32dB PADs
		val = (unsigned char)atten;
	}

	int err = set_RF_inp_board(S3I2CCurPath, val, Rx, Tx);

	Dbg_RF1_DSA = val;
	Dbg_Path = S3I2CCurPath;

	if (err)
		return err;

#ifdef S3_READ_BACK

	Sleep(100);
	
	err = S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_RF_DSA, 1);

	Sleep(100);

	if (err)
		return 2;

	if (S3I2CTxReadBuf[0] != val)
		return 3;
#endif

#endif // TRIZEPS

	return 0;
}

// ----------------------------------------------------------------------------
// Tx Opt. Combine Tx RF and optical factory calibration
short	S3I2CCalcTxOptDSA(char Rx, char Tx, char IP, char dsa)
{
	short val = 0;

	short cal = S3TxGetCalOpt(Rx, Tx);

	// Frig as table assumes 16dB PAD can be used
	if (S3I2CCurPath == 7)
	{
		int gain = S3IPGetGain(Rx, Tx, IP);
		if (gain < -15 && gain >= -30)
			dsa = 15 + 1;
	}

	val = cal - ((short)dsa * 100 + 600);	// 10mdBm
	val -= S3TxGetCalRF(Rx, Tx, S3I2CCurPath - 1);

	return val;
}

// ----------------------------------------------------------------------------
// Tx Opt. Applied value is gain.

int	S3I2CSetTxOptDSA(char Rx, char Tx, char IP, char dsa)
{
#ifdef TRIZEPS

	short val;

	short cal = S3TxGetCalOpt(Rx, Tx);

	// Frig as table assumes 16dB PAD can be used
	int gain = S3IPGetGain(Rx, Tx, IP);
	if (S3I2CCurPath == 7)
	{
		// int gain = S3IPGetGain(Rx, Tx, IP);
		if (	(gain - S3TxGetAttenGainOffset(Rx, Tx)) < -15 &&
				(gain - S3TxGetAttenGainOffset(Rx, Tx)) >= -30)
				dsa = 15 + 1;
	}

	val = cal - ((short)dsa * 100 + 600);	// 10mdBm

	// if (S3I2CCurPath > 3 && S3I2CCurPath <= 7 && gain <= 0)
	//	val -= 600;

	val -= S3TxGetCalRF(Rx, Tx, S3I2CCurPath - 1);

	int err = S3I2CWriteSerialShort(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_DSA, val);

	Dbg_TxOpt = val;

	if (err)
		return err;

#ifdef S3_READ_BACK
	err = S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_DSA, 2);

	if (err)
		return 2;

	short rval;
	*((char *)&rval + 0) = S3I2CTxReadBuf[1];
	*((char *)&rval + 1) = S3I2CTxReadBuf[0];

	if (rval != val)
		return 3;
#endif
	
#endif // TRIZEPS

	return 0;
}

// ----------------------------------------------------------------------------
// Rx Opt
short S3I2CCalcRxOptDSA(char Rx, char Tx, char dsa)
{
	short val = 0;
	short cal = S3RxGetCalGain(Rx, Tx);

	val = cal - ((short)dsa * 100 + 450);
	val -= S3I2CRxOptFact[S3I2CCurPath - 1];

	// Target RLL = 1200 
	if (S3GetAGC() == S3_AGC_GAIN)
		val += 2 * (1200 - S3RxGetRLL(Rx, Tx));

	return val;
}

// ----------------------------------------------------------------------------
// Rx Opt. Applied value is gain

int	S3I2CSetRxOptDSA(char Rx, char Tx, char dsa)
{
#ifdef TRIZEPS
	unsigned char wbuf[3];
	short val;

	short cal = S3RxGetCalGain(Rx, Tx);

	val = cal - ((short)dsa * 100 + 450);
	val -= S3I2CRxOptFact[S3I2CCurPath - 1];

	// Target RLL = 1200 
	if (S3GetAGC() == S3_AGC_GAIN)
	{
		short RLL = S3RxGetRLL(Rx, Tx);

		Dbg_RLL = RLL;
		
		val += 2 * (1200 - RLL);
	}

	wbuf[0] = S3I2C_RX_OPT_DSA;
	wbuf[1] = *((unsigned char *)&val + 1);
	wbuf[2] = *((unsigned char *)&val + 0);
	
	BOOL ok = I2C_WriteRead(S3I2CCurRxOptAddr, wbuf, 3, NULL, 0);

	Dbg_RxOpt = val;

	if (!ok)
		return 1;

#ifdef S3_READ_BACK

	Sleep(1000);
	unsigned char rbuf[2];
	ok = I2C_WriteRead(S3I2CRxOptAddr, wbuf, 1, rbuf, 2);

	if (!ok)
		return 2;

	short rval;
	*((char *)&rval + 0) = rbuf[1];
	*((char *)&rval + 1) = rbuf[0];

	if (rval != val)
		return 3;
#endif


#endif
	return 0;
}

// ----------------------------------------------------------------------------
// Any fail should be strongly flagged - and handled

int S3I2CSetIPGain(char Rx, char Tx, char IP)
{
	if (IP >= 100)
		return 0;

	S3TimerStart(1);

	char	GainSent = S3IPGetGainSent(Rx, Tx, IP);
	char	PathSent = S3IPGetPathSent(Rx, Tx, IP);

	if (GainSent == -128) // Pending
	{
		// Careful: this is true if tx is asleep
		if (!S3TxRLLStable(Rx, Tx))
			return 0;

		int		Gain = S3IPGetGain(Rx, Tx, IP);

		S3I2CCurPath = S3TxGetRFPath(Rx, Tx, IP);

		const char *Paras;
		if (S3I2CCurPath > 3 && S3I2CCurPath <= 7)
			Paras = S3GetGainParas_dB(Gain - 0); // S3TxGetAttenGainOffset(Rx, Tx));
		else
			Paras = S3GetGainParas_dB(Gain);

		char Atten = 0;

		if (S3I2CCurPath >= 3 && S3I2CCurPath <= 6)
		{
			Atten = Paras[TX_RF1_PAD1] + Paras[TX_RF1_PAD2];
		}
		else if (S3I2CCurPath == 1)
		{
			Atten = Paras[TX_RF1_DSA];
		}
		else if (S3I2CCurPath == 7 || S3I2CCurPath == 2)
			Atten = 0;	// Not used
		else
			return 1;	// Trouble

		/*
		// TODO: Revisit
		short RF1Atten = 100 * (short)S3I2CCalcRFAtten(Atten);
		short RxAtten = S3I2CCalcRxOptDSA(Rx, Tx, Paras[ORX_OPT]);
		short TxAtten = S3I2CCalcTxOptDSA(Rx, Tx, IP, Paras[OTX_OPT]);

		short Srt[3];
		Srt[0] = RF1Atten;
		Srt[1] = RxAtten;
		Srt[2] = TxAtten;

		unsigned char i, min_i;
		short min = SHRT_MAX; 
		for(i = 0; i < 3; i++)
		{
			if (Srt[i] < min)
			{
				min = Srt[i];
				min_i = i;
			}
		}
		
		unsigned char max_i;
		short max = SHRT_MIN; 
		for(i = 0; i < 3; i++)
		{
			if (Srt[i] > max)
			{
				max = Srt[i];
				max_i = i;
			}
		}
		*/

		if (S3I2CSetRxOptDSA(Rx, Tx, Paras[RX_OPT_DSA]))
			return 4;

		if (S3I2CSetTxOptDSA(Rx, Tx, IP, Paras[TX_OPT_DSA]))
			return 5;

		if (S3I2CSetRFAtten(Atten, Rx, Tx))
			return 3;

		// *Presumed* successful as no read-back
		S3IPSetGainSent(Rx, Tx, IP, Gain);
		S3IPSetPathSent(Rx, Tx, IP, S3I2CCurPath);

		if (S3I2CCurPath != PathSent)
		{
			if (S3I2CTxUpdateTempPath(Rx, Tx))
				return 7;
				
			if (S3TxGetPeakHoldCap(Rx, Tx))
				if (S3I2CTxSetPeakThresh(Rx, Tx, S3I2CCurPath))
					return 6;

			// When changing path, may trigger peak detectiion
			// TODO: Required for all path changes or just -> path 1
			S3TxClearPeakHold(Rx, Tx, 0); // Force latch reset
			S3I2CTxPeakHoldLatchClear(Rx, Tx);
		}
	}

	S3TimerStop(1);

	return 0;
}

// ----------------------------------------------------------------------------