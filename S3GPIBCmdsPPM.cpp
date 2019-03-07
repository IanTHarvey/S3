// ----------------------------------------------------------------------------
// GPIB remote commands for PPM set-up only
//
// ----------------------------------------------------------------------------

#include "stdafx.h"
#include <float.h>

#include "S3DataModel.h"
#include "S3GPIB.h"
#include "S3I2C.h"

extern unsigned char	GPIBNArgs;
extern char				GPIBCmd[S3_MAX_GPIB_CMD_LEN];
extern char				*GPIBCmdArgs[S3_MAX_ARGS];

extern char		GPIBCmdBuf[S3_MAX_GPIB_CMD_LEN];
extern char		GPIBRetBuf[S3_MAX_GPIB_RET_LEN];
extern char		GPIBRetBufTmp[S3_MAX_GPIB_RET_LEN];
extern unsigned	GPIBBufLen;

extern char		GPIBRx, GPIBTx, GPIBIP;

extern pS3DataModel S3Data;

extern SigmaT	S3Str2SigmaT(const char *str);
extern InputZ	S3Str2InputZ(const char *str);

extern char		GetAddArg(		const char *carg);
extern short	GetShortArg(	const char *carg);
extern double	GetDoubleArg(	const char *carg);

// ----------------------------------------------------------------------------
// Must specify channel although only required for Rx2

int CmdPPMCALRX()
{
	if (S3GetLocked())
		return S3_GPIB_COMMAND_LOCKED;
	
	if (GPIBNArgs != 4)
		return S3_GPIB_ERR_NUMBER_PARAS;

	char			all, Rx, Tx;

	int res = GetAddress2(&all, &Rx, &Tx, NULL);

	char *pEnd;
	double val = (double)strtod(GPIBCmdArgs[3], &pEnd);
	
	if (*pEnd != '\0')
		return S3_GPIB_INVALID_NUMBER;

	if (S3RxGetType(Rx) == S3_Rx2)
	{
		if (Tx < 0 || Tx > 1)
			return S3_GPIB_INVALID_ADDRESS;
	}
	else
	{
		if (Tx != 0)
			return S3_GPIB_INVALID_ADDRESS;

		Tx = -1;
	}

	int err = S3SetFactoryMode(Rx, Tx, true);

	if (!err)
	{
		if (S3I2CRxSetCalibration(Rx, Tx, val))
			return S3_GPIB_CALIBRATION_FAILED;
	}
	else
	{
		return S3_GPIB_CALIBRATION_FAILED;
	}

	S3SetFactoryMode(Rx, -1, false);

	return 0;
}

// ----------------------------------------------------------------------------
// Must specify channel although only required for Rx2

int CmdPPMGETCALRX()
{
	if (S3GetLocked())
		return S3_GPIB_COMMAND_LOCKED;
	
	char	all, Rx, Tx, IP;

	if (GPIBNArgs == 3)
	{
		int		res = GetAddress2NoArg(&all, &Rx, &Tx, &IP, false);
			
		if (res < 0)
		{
			if (res == -1 || res == -2)
				return S3_GPIB_MALFORMED_ADDRESS;
			if (res == -3)
				return S3_GPIB_INVALID_ADDRESS;
			if (res == -4)
				return S3_GPIB_OUT_RANGE_ADDRESS;
		}
		else
		{
			if (res > 2000)
				return res;

			// We need the full Tx (RLL source) address
			if (res != 2)
				return S3_GPIB_INVALID_ADDRESS;
		}
	}
	else return S3_GPIB_ERR_NUMBER_PARAS;

	if (S3RxGetType(Rx) == S3_Rx2)
	{
		if (Tx < 0 || Tx > 1)
			return S3_GPIB_INVALID_ADDRESS;
	}
	else
	{
		if (Tx != 0)
			return S3_GPIB_INVALID_ADDRESS;
	}

	double Cal = (double)S3RxGetCalGain(Rx, Tx) / 100.0;

	sprintf_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "I: %.2f", Cal); 

	return 0;
}

// ----------------------------------------------------------------------------

int CmdPPMCALTXRF()
{
	if (S3GetLocked())
		return S3_GPIB_COMMAND_LOCKED;

	if (GPIBNArgs != 5)
		return S3_GPIB_ERR_NUMBER_PARAS;

	char			all, Rx, Tx;

	int res = GetAddress2(&all, &Rx, &Tx, NULL);

	if (res > 2000)
		return res;

	char *pEnd;
	char path = (char)strtol(GPIBCmdArgs[3], &pEnd, 10);
	
	if (*pEnd != '\0')
		return S3_GPIB_INVALID_NUMBER;

	if (path == 2 || path < 1 || path > 7)
		return S3_GPIB_INVALID_PARAMETER;

	path--; // 0-indexed

	double val = strtod(GPIBCmdArgs[4], &pEnd);
	
	if (*pEnd != '\0')
		return S3_GPIB_INVALID_NUMBER;

	if (S3SetFactoryMode(Rx, Tx, true))
		return S3_GPIB_CALIBRATION_FAILED;

	if (S3I2CTxSetRFCalibration(Rx, Tx, path, val))
		return S3_GPIB_CALIBRATION_FAILED;

	S3SetFactoryMode(Rx, Tx, false);

	return 0;
}

// ----------------------------------------------------------------------------

int CmdPPMCALTXOPT()
{
	if (S3GetLocked())
		return S3_GPIB_COMMAND_LOCKED;

	if (GPIBNArgs != 4)
		return S3_GPIB_ERR_NUMBER_PARAS;

	char			all, Rx, Tx;

	int res = GetAddress2(&all, &Rx, &Tx, NULL);

	if (res > 2000)
		return res;

	char *pEnd;
	double val = strtod(GPIBCmdArgs[3], &pEnd);
	
	if (*pEnd != '\0')
		return S3_GPIB_INVALID_NUMBER;

	if (S3SetFactoryMode(Rx, Tx, true))
		return S3_GPIB_CALIBRATION_FAILED;

	if (S3I2CTxSetOptCalibration(Rx, Tx, val))
		return S3_GPIB_CALIBRATION_FAILED;

	S3SetFactoryMode(Rx, Tx, false);

	return 0;
}

// ----------------------------------------------------------------------------

int CmdPPMBATTID()
{
	if (S3GetLocked())
		return S3_GPIB_COMMAND_LOCKED;
	
	if (GPIBNArgs != 3)
		return S3_GPIB_ERR_NUMBER_PARAS;

	// TODO: Get Ch from command?
	char Ch = 0;
	if (!S3ChOccupied(Ch))
		return S3_GPIB_CH_NOT_EXIST;

	char SN[S3_MAX_SN_LEN], PN[S3_MAX_PN_LEN];

	strcpy_s(SN, S3_MAX_SN_LEN, GPIBCmdArgs[1]);
	strcpy_s(PN, S3_MAX_PN_LEN, GPIBCmdArgs[2]);

	S3SetFactoryMode(-1, -1, true);

	S3I2CChMS(Ch);

	int err = S3I2CChWriteSNPN(Ch, SN, PN);

	S3SetFactoryMode(-1, -1, false);

	if (err == 1)
	{
		return S3_GPIB_ID_WRITE_FAILED;
	}
	else if (err == 2)
	{
		return S3_GPIB_INVALID_SNPN;
	}
	else if (err == 3)
	{
		return S3_GPIB_BATTERY_SEALED;
	}

	return err;
}

// ----------------------------------------------------------------------------

int CmdPPMBATTAUTH()
{
	int err = 0;

	if (S3GetLocked())
		return S3_GPIB_COMMAND_LOCKED;
	
	if (GPIBNArgs != 2)
		return S3_GPIB_ERR_NUMBER_PARAS;

	// TODO: Get Ch from command?
	char Ch = (char)GetShortArg(GPIBCmdArgs[1]) - 1;

	if (Ch < 0 || Ch >= S3_N_CHARGERS)
		return S3_GPIB_CH_NOT_EXIST;

	if (!S3ChOccupied(Ch))
		return S3_GPIB_CH_NOT_EXIST;

	if (S3ChBattValidated(Ch))
		return S3_GPIB_CH_VALIDATED;

	S3SetFactoryMode(-1, -1, true);

	S3I2CChMS(Ch);

	err = S3I2CChWriteSecKeys();

	S3SetFactoryMode(-1, -1, false);

	if (err)
	{
		return S3_GPIB_CH_AUTH_FAILED;
	}

	return err;
}

// ----------------------------------------------------------------------------
// Type, PN, SN

int CmdPPMTXID()
{
	if (S3GetLocked())
		return S3_GPIB_COMMAND_LOCKED;
	
	if (GPIBNArgs != 4)
		return S3_GPIB_ERR_NUMBER_PARAS;

	// TODO: Get Tx from command?
	char Rx = 0, Tx = 0;
	if (S3TxGetType(Rx, Tx) == S3_TxUnconnected)
		return S3_GPIB_TX_NOT_EXIST;

	char SN[S3_MAX_SN_LEN], PN[S3_MAX_PN_LEN];

	strcpy_s(PN, S3_MAX_PN_LEN, GPIBCmdArgs[2]);
	strcpy_s(SN, S3_MAX_SN_LEN, GPIBCmdArgs[3]);

	S3SetFactoryMode(Rx, Tx, true);

	int err = S3I2CTxWriteID(GPIBCmdArgs[1], PN, SN);

	S3SetFactoryMode(Rx, Tx, false);

	if (err == 10 || err == 20 || err == 30)
		return S3_GPIB_ID_WRITE_FAILED; // Read-back fails
	else if (err == 5)
		return S3_GPIB_INVALID_TYPE;
	else if (err == 2)
		return S3_GPIB_INVALID_SNPN;
	else if (err)
		return S3_GPIB_ID_WRITE_FAILED;

	return err;
}

// ----------------------------------------------------------------------------
// Type, PN, SN

int CmdPPMRXID()
{
	if (S3GetLocked())
		return S3_GPIB_COMMAND_LOCKED;
	
	if (GPIBNArgs != 4)
		return S3_GPIB_ERR_NUMBER_PARAS;

	// TODO: Get Rx from command? YES!
	char Rx = 0;
	if (S3RxGetType(Rx) == S3_RxEmpty)
		return S3_GPIB_RX_NOT_EXIST;

	char SN[S3_MAX_SN_LEN], PN[S3_MAX_PN_LEN];

	strcpy_s(PN, S3_MAX_PN_LEN, GPIBCmdArgs[2]);
	strcpy_s(SN, S3_MAX_SN_LEN, GPIBCmdArgs[3]);

	int err = S3SetFactoryMode(Rx, -1, true);

	if (err)
		return S3_GPIB_ID_WRITE_FAILED;

	err = S3I2CRxWriteID(GPIBCmdArgs[1], PN, SN);

	S3SetFactoryMode(Rx, -1, false);

	if (err == 10 || err == 20)
		return S3_GPIB_ID_WRITE_FAILED; // Read-back fails
	else if (err == 2)
		return S3_GPIB_INVALID_SNPN;
	else if (err)
		return S3_GPIB_ID_WRITE_FAILED;

	return err;
}

// ----------------------------------------------------------------------------

int CmdPPMSYSID()
{
	if (S3GetLocked())
		return S3_GPIB_COMMAND_LOCKED;
	
	if (GPIBNArgs != 3)
		return S3_GPIB_ERR_NUMBER_PARAS;

	char SN[S3_MAX_SN_LEN], PN[S3_MAX_PN_LEN];

	strcpy_s(PN, S3_MAX_PN_LEN, GPIBCmdArgs[1]);
	strcpy_s(SN, S3_MAX_SN_LEN, GPIBCmdArgs[2]);

	int err = S3SetFactoryMode(-1, -1, true);

	if (err)
		return S3_GPIB_ID_WRITE_FAILED;

	S3SysSetPN(PN);
	err = S3SysWritePN();
	
	if (!err)
	{
		S3SysSetSN(SN);
		err = S3SysWriteSN();
	}

	S3SetFactoryMode(-1, -1, false);

	if (err)
		return S3_GPIB_ID_WRITE_FAILED;

	return err;
}

// ----------------------------------------------------------------------------

int CmdPPMTERMINATOR()
{
	if (GPIBNArgs != 2)
		return S3_GPIB_ERR_NUMBER_PARAS;

	if (!STRNCMP(GPIBCmdArgs[1], "NL", 2))
	{
		S3SetTerminator(0);
	}
	else if (!STRNCMP(GPIBCmdArgs[1], "ZERO", 4))
	{
		S3SetTerminator(1);
	}
	else if (!STRNCMP(GPIBCmdArgs[1], "NONE", 4))
	{
		S3SetTerminator(2);
	}
	else
		return S3_GPIB_INVALID_PARAMETER;

	return 0;
}

// ----------------------------------------------------------------------------

