// ----------------------------------------------------------------------------
// S3 controller, rx and tx functions grouped only because they're only
// applicable to factory set-up.
//
// None of this should be used in normal 'run-time'.
//
// All functions assume comms to slot1, tx1 is already set up.
//

#include "stdafx.h"
#include <math.h>

#include "S3DataModel.h"

extern pS3DataModel S3Data;

#ifdef TRIZEPS
#include "drvLib_app.h"
#endif

#include "S3I2C.h"
#include "S3Gain.h"

extern unsigned short	S3RevByteUShort(unsigned char *b);
extern short			S3RevByteShort(	unsigned char *b);

int	S3I2CTxSetPassword(		const char *rPW);
int	S3I2CTxOptSetPassword(	const char *rPW);
int	S3I2CTxCtrlSetPassword(	const char *rPW);

int S3I2CRxCtrlSetPassword(	const char *rPW);
int S3I2CRxOptSetPassword(	const char *rPW);

// ----------------------------------------------------------------------------
// Set I2C password for Tx optical and control boards

int S3I2CTxSetPassword(const char *rPW)
{
	int err = 0;

	if (S3I2CTxOptSetPassword(rPW))
		err = 1;
	if (!err)
		err = S3I2CTxCtrlSetPassword(rPW);

	return err;
}

// ----------------------------------------------------------------------------
// Set I2C password for Tx optical board

int S3I2CTxOptSetPassword(const char *rPW)
{
	S3EventLogAdd("S3I2CTxOptSetPassword", 3, -1, -1, -1);
	int err = 0;

#ifdef TRIZEPS
	err = S3I2CWriteSerialData(S3I2C_TX_OPT_ADDR, S3I2C_PW_ADDR,
											(const unsigned char *)rPW, 4);
#endif

	return err;
}

// ----------------------------------------------------------------------------
// Set I2C password for Tx control board

int S3I2CTxCtrlSetPassword(const char *rPW)
{
	S3EventLogAdd("S3I2CTxCtrlSetPassword", 3, -1, -1, -1);

	int err = 0;

#ifdef TRIZEPS

	if (1)
	{
		// err = S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, 0x1D, 1);
		// unsigned char prev = *S3I2CTxReadBuf;

		err = S3I2CWriteSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_PW_ADDR,
					(const unsigned char *)rPW, 4);

		Sleep(500);

		// err = S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, 0x1D, 1);
		// if (*S3I2CTxReadBuf != 'S' && *S3I2CTxReadBuf != prev)
		//	S3EventLogAdd("S3I2CTxCtrlSetPassword: PN corrupted this call", 3, -1, -1, -1);
	}
	else
	{
		err = S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, 0x1D, 1);
		unsigned char prev = *S3I2CTxReadBuf;

		err += S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR, S3I2C_PW_ADDR + 0, *(rPW + 0));
		Sleep(50);
		err += S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR, S3I2C_PW_ADDR + 1, *(rPW + 1));
		Sleep(50);
		err += S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR, S3I2C_PW_ADDR + 2, *(rPW + 2));
		Sleep(50);
		err += S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR, S3I2C_PW_ADDR + 3, *(rPW + 3));
		Sleep(50);

		err = S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, 0x1D, 1);
		if (*S3I2CTxReadBuf != 'S' && *S3I2CTxReadBuf != prev)
			S3EventLogAdd("S3I2CTxCtrlSetPassword: PN corrupted this call", 3, -1, -1, -1);
	}

#endif

	return err;
}

// ----------------------------------------------------------------------------
// One-off fix

int S3I2CTxWriteLaserBias()
{

	FILE *fid;
	int err = fopen_s(&fid, "\\Flashdisk\\S3\\WriteBias.txt", "w");

	Sleep(1000);
	
	err = S3I2CTxCtrlSetPassword("D200");
	Sleep(2000);

	unsigned char addr = 0xA1;
	unsigned char val = 27;

	err = S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR, addr, val);

	Sleep(1000);

	if (err)
		fprintf(fid, "Write failed: %d", addr);
	else
		fprintf(fid, "Write ok: %d: %d\n", addr, val);

	Sleep(1000);

	if (err)
	{
		fclose(fid);
	}

	err = S3I2CTxCtrlSetPassword("----");
	fprintf(fid, "I2CTxSetPassword: Cleared: %d\n", err);
	Sleep(2000);

	err = S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, addr, 1);

	Sleep(100);

	if (err)
	{
		fprintf(fid, "Read failed: %d\n", addr);
	}
	else
	{
		unsigned char cal = 0;
		cal = S3I2CTxReadBuf[0];

		if (cal != val)
		{
			fprintf(fid, "Bias failed: w: %d; r: %d\n", 
				val, cal);
		}
		else
		{
			fprintf(fid, "Calibration values ok: w: %d; r: %d\n", 
				val, cal);
		}
	}

	fclose(fid);

	return 0;
}

// ----------------------------------------------------------------------------
// Write individual calibration values for factory set-up

int S3I2CTxWriteRFCalValue(char Rx, char Tx, unsigned char Path, double dcal)
{
	// TODO: Test dcal in range
	
	short cal;

	cal = (short)(ROUND(dcal * 100.0));

	// THIS IS A CALIBRATION-ONLY PROCEDURE
	
	int err = S3I2CTxCtrlSetPassword("D200");

	if (err)
		return 1;

	if (0)
	{
		err = S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR,
			S3I2C_TX_CTRL_CAL_DATA + Path * 2 + 1, *((char *)&cal + 0));

		if (err)
			return 3;

		Sleep(500);

		err = S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR,
			S3I2C_TX_CTRL_CAL_DATA + Path * 2, *((char *)&cal + 1));

		if (err)
			return 2;

		Sleep(500);
	}
	else
	{
		err = S3I2CWriteSerialShort(S3I2C_TX_CTRL_ADDR,
			S3I2C_TX_CTRL_CAL_DATA + Path * 2, cal);

		if (err)
			return 3;

		// Sleep(500); // Required for pre 15Mar17 TxCtrl F/W
	}

	// Read back
	err = S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR,
		S3I2C_TX_CTRL_CAL_DATA + Path * 2, 2);

	if (err)
	{
		return 5;
	}
			
	short rcal;
	*((char *)&rcal + 0) = S3I2CTxReadBuf[1];
	*((char *)&rcal + 1) = S3I2CTxReadBuf[0];

	if (rcal != cal)
	{
		err = 6;
	}

	// Force re-read and re-set of gain for immediate feedback
	char IP = 0;
	
	IP = S3TxGetActiveIP(Rx, Tx);

	S3TxSetCalRF(Rx, Tx, Path, cal);
	S3IPSetGainSent(Rx, Tx, IP, SCHAR_MIN);
	S3I2CSetIPGain(Rx, Tx, IP);

	return err;
}

// ----------------------------------------------------------------------------

int S3I2CTxWriteOptCalValue(double dcal)
{
	short cal;

	cal = (short)(ROUND(dcal * 100.0));

	// THIS IS A CALIBRATION-ONLY PROCEDURE
	
	int err;

	err = S3I2CTxOptSetPassword("D200");
	if (err)
		return 1;

	err = S3I2CWriteSerialShort(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_CAL_GAIN, cal);

	Sleep(500);

	if (err)
		return 2;

	// Read back
	short rcal;
	err = S3I2CReadSerialShort(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_CAL_GAIN, &rcal);

	if (err)
		return 5;
			
	if (rcal != cal)
		return 6;

	// THIS IS A CALIBRATION-ONLY PROCEDURE
	// ------------------------------------------------------------------------

	return 0;
}

// ----------------------------------------------------------------------------
// Password required

int S3I2CTxDisablePeakDet(char Rx, char Tx)
{
	// Set to 327dBm
	int err  = S3I2CWriteSerialShort(S3I2C_TX_OPT_ADDR,
				S3I2C_TX_OPT_PEAK_H, 32767);

	return err;
}

// ----------------------------------------------------------------------------
// Password not required

int S3I2CTxDisableRFLevelAlarm(char Rx, char Tx)
{
	// Set to 20dBm
	int err  = S3I2CWriteSerialShort(S3I2C_TX_OPT_ADDR,
				S3I2C_TX_OPT_RF_H, 2000);

	return err;
}

// ----------------------------------------------------------------------------
// One-off CEA/prototype serial numbers

int S3I2CRxWriteID(const char *Type, const char *PN, const char *SN)
{
	BOOL	ok = TRUE;

	// ATiS
	// char PN[] = "S3R-01-01-00";
	// char SN[] = "1248574";

	// CEA
	// char PN[] = "S3R-01-01-00";
	// char SN[] = "1247106";

	// Eng demo
	// char PN[] = "S3R-01-01-00";
	// char SN[] = "Eng0001";

	// Eng cardboard
	// char PN[] = "S3R-01-01-00";
	// char SN[] = "Eng0002";

	// char PN[] = "S3R-02-01-00";
	// char SN[] = "Eng0003";

#ifdef TRIZEPS
	
	S3TxType S3Type;

	if (!_stricmp(Type, "RX1"))
		S3Type = S3_Rx1;
	else if (!_stricmp(Type, "RX2"))
		S3Type = S3_Rx2;
	else if (!_stricmp(Type, "RX6"))
		S3Type = S3_Rx6;
	else if (!_stricmp(Type, "-"))
		S3Type = 0;
	else
		return 5;

	int err = S3I2CRxCtrlSetPassword("D200");

	if (err)
		return 8;

	char	Rx = 0;

	char	wbuf[24];
	char	rbuf[24];

	if (S3Type)
	{
		I2C_WriteRandom(S3I2C_RX_CTRL_ADDR, S3I2C_RX_CTRL_RX_TYPE, (unsigned char)S3Type);

		unsigned char rtype = I2C_ReadRandom(S3I2C_RX_CTRL_ADDR, S3I2C_RX_CTRL_RX_TYPE);
	
		if (rtype != S3Type)
			return 20;
	}

	if (ok && *PN != '-')
	{
		for(unsigned char i = 0; i < 24; i++) wbuf[i] = '\0';

		wbuf[0] = S3I2C_RX_CTRL_PN;
		strcpy_s(wbuf + 1, 24 - 1, PN);
		ok = I2C_WriteRead(S3I2C_RX_CTRL_ADDR, (unsigned char *)(wbuf),
					S3I2C_PN_LEN + 1, NULL, 0);

		Sleep(100);

		wbuf[0] = S3I2C_RX_CTRL_PN;
		ok = I2C_WriteRead(S3I2C_RX_CTRL_ADDR, (unsigned char *)wbuf,
					1, (unsigned char *)rbuf, S3I2C_PN_LEN);

		if (strcmp(PN, rbuf))
			return 10;

		S3RxSetPN(Rx, PN);
	}

	// -------------------------------------------------------------

	if (ok && *SN != '-')
	{
		for(unsigned char i = 0; i < 24; i++) wbuf[i] = '\0';

		wbuf[0] = S3I2C_RX_CTRL_SN;
		strcpy_s(wbuf + 1, 24 - 1, SN);
		ok = I2C_WriteRead(S3I2C_RX_CTRL_ADDR, (unsigned char *)wbuf,
			S3I2C_SN_LEN + 1, NULL, 0);

		Sleep(100);

		ok = I2C_WriteRead(S3I2C_RX_CTRL_ADDR, (unsigned char *)wbuf,
					1, (unsigned char *)rbuf, S3I2C_SN_LEN);

		if (strcmp(SN, rbuf))
			return 20;

		S3RxSetSN(Rx, SN);
	}

	// -------------------------------------------------------------

#endif

	return ok == 0;
}

// ----------------------------------------------------------------------------
// FACTORY-ONLY

int S3I2CTxWriteID(const char *Type, const char *PN, const char *SN)
{
	// ATiS
	// char PN[] = "S3T-08-01-00";
	// char SN[] = "1248574";

	// CEA
	// char PN[] = "S3T-01-01-00";
	// char SN[] = "1247107";

	// Eng
	// char PN[] = "S3T-08-01-00";
	// char SN[] = "EngT801";

	int err = 0;

	// TODO: Check lengths return 2 for OoR

#ifdef TRIZEPS

	S3TxType S3Type;

	if (!_stricmp(Type, "TX1"))
		S3Type = S3_Tx1;
	else if (!_stricmp(Type, "TX8"))
		S3Type = S3_Tx8;
	else if (!_stricmp(Type, "-"))
		S3Type = 0;
	else
		return 5;

	char Rx = 0, Tx = 0;

	err = S3I2CTxCtrlSetPassword("D200");

	if (err)
		return 8;

	if (S3Type)
	{
		err = S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR, S3I2C_TX_CTRL_TX_TYPE, S3Type);
			
		if (err)
			return 1;

		Sleep(500);

		// Read back
		err = S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_CTRL_TX_TYPE, 1);

		if (err)
			return 1;

		if (S3I2CTxReadBuf[0] != S3Type)
			return 20;
	}

	char buf[20];

	if (!err  && *PN != '-')
	{
		for(unsigned char i = 0; i < 20; i++) buf[i] = '\0';
		strcpy_s(buf, 20, PN);

		if (0)
		{
			for(unsigned char i = 0; i < S3I2C_PN_LEN; i++)
			{
				err = S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR, S3I2C_TX_CTRL_PN + i,
					(unsigned char)buf[i]);
			
				if (err)
					break;

				Sleep(1000);
			}
		}
		else
		{
			err = S3I2CWriteSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_CTRL_PN,
				(unsigned char *)buf, 5);

			Sleep(500);

			err = S3I2CWriteSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_CTRL_PN + 5,
				(unsigned char *)buf + 5, 5);

			Sleep(500);

			err = S3I2CWriteSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_CTRL_PN + 10,
				(unsigned char *)buf + 10, 5);

			Sleep(500);

			err = S3I2CWriteSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_CTRL_PN + 15,
				(unsigned char *)buf + 15, 4);

			Sleep(500);
		}

		if (!err)
		{
			err = S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR,
									S3I2C_TX_CTRL_PN, S3I2C_PN_LEN);

			if (strcmp(PN, (char *)S3I2CTxReadBuf))
				err = 30;
			else
				S3TxSetPN(Rx, Tx, PN);
		}
	}

	if (!err && *SN != '-')
	{
		for(unsigned char i = 0; i < 20; i++) buf[i] = '\0';
		strcpy_s(buf, 20, SN); 

		if (1)
		{
			err = S3I2CWriteSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_CTRL_SN,
				(unsigned char *)buf, S3I2C_SN_LEN);

			Sleep(500);
		}
		else
		{
			for(unsigned char i = 0; i < S3I2C_SN_LEN; i++)
			{
				err = S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR, S3I2C_TX_CTRL_SN + i,
					(unsigned char)buf[i]);

				if (err)
					break;
				
				Sleep(1000);
			}
		}

		if (!err)
		{
			err = S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR,
									S3I2C_TX_CTRL_SN, S3I2C_SN_LEN);

			if (strcmp(SN, (char *)S3I2CTxReadBuf))
				err = 10;
			else
				S3TxSetSN(Rx, Tx, SN);
		}
	}

#endif

	return err;
}

// ----------------------------------------------------------------------------
// One-off write Tx type as Tx1 or Tx8. TODO: Need GUI interface. 

int S3I2CTxWriteFactoryType(char Rx, char Tx, S3TxType type)
{
	if (S3I2CTxCtrlSetPassword("D200"))
		return 1;

	// unsigned char RegAddr = 0xA1;	// This works
	unsigned char RegAddr = S3I2C_TX_CTRL_TX_TYPE;		// This doesn't

	// Read before
	if (S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, RegAddr, 1))
		return 2;

	Sleep(1000);

	// Fecking debugger... 
	unsigned char t = S3I2CTxReadBuf[0];

	if( S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR, RegAddr, type))
		return 3;

	Sleep(2000);

	// Read after
	/*
	if (S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, RegAddr, 1))
		return 2;

	Sleep(1000);

	t = S3I2CTxReadBuf[0];
	*/

	return 0;
}

// ----------------------------------------------------------------------------
// Not used.

int S3I2CTxWriteFactoryCal(char Rx, char Tx) // short *CalPath)
{
	FILE *fid;
	int err = fopen_s(&fid, "\\Flashdisk\\S3\\WriteCal2.txt", "w");

	Sleep(1000);
	
	err = S3I2CTxCtrlSetPassword("D200");

	fprintf(fid, "I2CTxSetPassword: %d\n", err);

	for(char i = 0; i < 7; i++)
	{
		short cal = S3TxGetCalRF(Rx, Tx, i);

		err = S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR, 0xA8 + i * 2,
			*((char *)&cal + 1));

		Sleep(1000);

		err = S3I2CWriteSerialByte(S3I2C_TX_CTRL_ADDR, 0xA8 + i * 2 + 1,
			*((char *)&cal + 0));

		if (err)
		{
			fprintf(fid, "Write failed[%d]: %d", i, 0xA8 + i * 2);
			break;
		}

		fprintf(fid, "Write[%d]: %d: %d\n", i, 0xA8 + i * 2,
			S3TxGetCalRF(Rx, Tx, i));

		Sleep(1000);
	}

	if (err)
	{
		fclose(fid);
		return 1;
	}

	err = S3I2CTxCtrlSetPassword("----");
	fprintf(fid, "I2CTxSetPassword: Cleared: %d\n", err);

	for(char i = 0; i < 7; i++)
	{
		err = S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, 0xA8 + i * 2, 2);

		Sleep(100);

		if (err)
		{
			fprintf(fid, "Read failed[%d]: %d\n", i, 0xA8 + i * 2);
		}
		else
		{
			short cal;
			*((char *)&cal + 0) = S3I2CTxReadBuf[1];
			*((char *)&cal + 1) = S3I2CTxReadBuf[0];

			if (cal != S3TxGetCalRF(Rx, Tx, i))
			{
				fprintf(fid, "Calibration values failed[%d]: w: %d; r: %d\n", i, 
					S3TxGetCalRF(Rx, Tx, i), cal);
			}
			else
			{
				fprintf(fid, "Calibration values ok[%d]: w: %d; r: %d\n", i,
					S3TxGetCalRF(Rx, Tx, i), cal);
			}
		}
	}

	fclose(fid);

	return 0;
}


// ----------------------------------------------------------------------------
// Set I2C passwords on RX control and optical boards
int S3I2CRxCtrlSetPassword(const char *rPW)
{
	S3EventLogAdd("S3I2CRxCtrlSetPassword", 3, -1, -1, -1);
	
	int				ok = 1;

#ifdef TRIZEPS
	unsigned char	pw[6];

	pw[0] = S3I2C_PW_ADDR;
	pw[1] = rPW[0];
	pw[2] = rPW[1];
	pw[3] = rPW[2];
	pw[4] = rPW[3];
	
	ok = I2C_WriteRead(S3I2C_RX_CTRL_ADDR, pw, 5, NULL, 0x00);

	Sleep(200);

/*
	unsigned char rd = I2C_ReadRandom(S3I2C_RX_CTRL_ADDR, S3I2C_RX_CTRL_PN);

	if (rd == '0')
	{
		S3EventLogAdd("PN corrupted (by password?)", 3, -1, -1, -1);
	}
*/

#endif
	return ok ? 0 : 1;
}

// ----------------------------------------------------------------------------
// Set I2C passwords on RX control and optical boards
int S3I2CRxOptSetPassword(const char *rPW)
{
	S3EventLogAdd("S3I2CRxOptSetPassword", 3, -1, -1, -1);
	int				ok = 1;

#ifdef TRIZEPS
	unsigned char	pw[6];

	pw[0] = S3I2C_PW_ADDR;
	pw[1] = rPW[0];
	pw[2] = rPW[1];
	pw[3] = rPW[2];
	pw[4] = rPW[3];

	ok = I2C_WriteRead(S3I2CCurRxOptAddr, pw, 5, NULL, 0x00);
#endif
	return ok ? 0 : 1;
}

// ----------------------------------------------------------------------------
// Test only

void I2CRxOptTestPassword()
{
#ifdef TRIZEPS
	unsigned char	rpw[4];
	unsigned char	msg[6];
	int				ok;

	// Attempt increment - 0x7c protected, so should fail
	msg[0] = 0x7c;
	ok = I2C_WriteRead(S3I2CCurRxOptAddr, msg, 1, rpw, 1);
	msg[1] = rpw[0] + 1;
	ok = I2C_WriteRead(S3I2CCurRxOptAddr, msg, 2, NULL, 0x00);
	ok = I2C_WriteRead(S3I2CCurRxOptAddr, msg, 1, rpw, 1);

	S3I2CRxCtrlSetPassword("D200");

	// Attempt increment - should succeed
	msg[0] = 0x7c;
	ok = I2C_WriteRead(S3I2CCurRxOptAddr, msg, 1, rpw, 1);
	msg[1] = rpw[0] + 1;
	ok = I2C_WriteRead(S3I2CCurRxOptAddr, msg, 2, NULL, 0x00);
	ok = I2C_WriteRead(S3I2CCurRxOptAddr, msg, 1, rpw, 1);

	S3I2CRxCtrlSetPassword("----");

	// Attempt increment - should fail
	msg[0] = 0x7c;
	ok = I2C_WriteRead(S3I2CCurRxOptAddr, msg, 1, rpw, 1);
	msg[1] = rpw[0] + 1;
	ok = I2C_WriteRead(S3I2CCurRxOptAddr, msg, 2, NULL, 0x00);
	ok = I2C_WriteRead(S3I2CCurRxOptAddr, msg, 1, rpw, 1);

	return;

#endif
}

// ----------------------------------------------------------------------------

int S3I2CRxWriteCalValue(char Rx, char Tx, double dcal)
{
#ifdef TRIZEPS
	short cal;

	cal = (short)(ROUND(dcal * 100.0));

	int err = S3I2CRxOptSetPassword("D200");

	if (err)
		return 1;

	Sleep(50);

	unsigned char wbuf[3];
	wbuf[0] = S3I2C_RX_OPT_CAL_GAIN;
	wbuf[1] = *((char *)&cal + 1);
	wbuf[2] = *((char *)&cal + 0);

	BOOL ok = I2C_WriteRead(S3I2CCurRxOptAddr, wbuf, 3, NULL, 0);

	if (!ok)
		return 2;

	Sleep(500);

	// Read back
	ok = I2C_WriteRead(S3I2CCurRxOptAddr, wbuf, 1, S3I2CRxReadBuf, 2);

	if (!ok)
		return 4;

	short rcal;
	*((char *)&rcal + 0) = S3I2CRxReadBuf[1];
	*((char *)&rcal + 1) = S3I2CRxReadBuf[0];

	if (rcal != cal)
		return 5;

	S3RxSetCalGain(Rx, Tx, cal);
#endif
	return 0;
}

// ----------------------------------------------------------------------------

void S3I2CRxWriteSN(char *SN)
{
#ifdef TRIZEPS
	unsigned char buf[S3_MAX_SN_LEN];
	int len = strlen(SN);

	BOOL			ok;

	buf[0] = S3I2C_RX_CTRL_SN;

	for(unsigned char i = 0; i < len; i++) 
		buf[i + 1] = SN[i];

	ok = I2C_WriteRead(S3I2C_RX_CTRL_ADDR, buf, len + 2, NULL, 0);

#endif
}

// ----------------------------------------------------------------------------

void S3I2CRxWritePN(char *PN)
{
#ifdef TRIZEPS

	unsigned char buf[S3_MAX_PN_LEN];
	int len = strlen(PN);

	BOOL			ok;

	buf[0] = S3I2C_RX_CTRL_PN;

	for(unsigned char i = 0; i < len; i++) 
		buf[i + 1] = PN[i];

	ok = I2C_WriteRead(S3I2C_RX_CTRL_ADDR, buf, len + 2, NULL, 0);

#endif
}

// ----------------------------------------------------------------------------

int S3I2CTxOptAlarmMask()
{
	int err;
	
	//err = S3I2CTxOptSetPassword("D200");

	//if (err)
	//	return 1;

	if (1)
	{	
		err = S3I2CWriteSerialByte(S3I2C_TX_OPT_ADDR, 0xCA, 0xD8);
	
		if (err)
			return 2;
	}
	else
	{
		err = S3I2CWriteSerialByte(S3I2C_TX_OPT_ADDR, 0xCB, 0x08);

		if (err)
			return 3;
	}

	return 0;
}

// ----------------------------------------------------------------------------
