// ----------------------------------------------------------------------------
// GPIB remote command parser/interpreter
//
// Incoming GPIB messages use 1-based indexing, internally zero-based node
// addressing is used, with -1 indicating invalid nodes. See:
//
//		GPIBRx = -1;
//		GPIBTx = -1; // For settings etc
//		GPIBIP = -1;
//
// ----------------------------------------------------------------------------

// See AfxGetMainWnd()->PostMessage(REM_SHUTDOWNREQ, 0, 0);
// #include "afxwin.h"
#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <float.h>
#include "S3DataModel.h"
#include "S3GPIB.h"
#include "S3I2C.h"

extern unsigned char	GPIBNArgs;
extern char			GPIBCmd[S3_MAX_GPIB_CMD_LEN];
extern char			*GPIBCmdArgs[S3_MAX_ARGS];

extern char		GPIBCmdBuf[S3_MAX_GPIB_CMD_LEN];
extern char		GPIBRetBuf[S3_MAX_GPIB_RET_LEN];
extern char		GPIBRetBufTmp[S3_MAX_GPIB_RET_LEN];
extern unsigned	GPIBBufLen;

extern char		GPIBRx;
extern char		GPIBTx;
extern char		GPIBIP;

extern pS3DataModel S3Data;

extern unsigned char S3GetArgs();

extern SigmaT S3Str2SigmaT(const char *str);
extern InputZ S3Str2InputZ(const char *str);

// TODO: This should be used for single-digit addresses rather than strtol
extern char		GetAddArg(		const char *carg);
extern short	GetShortArg(	const char *carg);
extern double	GetDoubleArg(	const char *carg);


// ----------------------------------------------------------------------------
// Nod to GPIB-ness

int CmdIDNQ()
{
	strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "I: PPM,SCT-3,FOL,V1.0");

	return 0;
}

// ----------------------------------------------------------------------------
// TODO: Add global command

int CmdAGC()
{
	if (GPIBNArgs != 2)
		return S3_GPIB_ERR_NUMBER_PARAS;

		if (!STRNCMP(GPIBCmdArgs[1], "OFF", 2))
		{
			S3SetAGC(S3_AGC_OFF + 100);
		}
		else if (!STRNCMP(GPIBCmdArgs[1], "CONT", 4))
		{
			S3SetAGC(S3_AGC_CONT + 100);
		}
		else if (!STRNCMP(GPIBCmdArgs[1], "GAIN", 4))
		{
			S3SetAGC(S3_AGC_GAIN + 100);
		}
		else
			return S3_GPIB_INVALID_PARAMETER;

		return 0;
}
// ----------------------------------------------------------------------------

int CmdTESTNAME()
{
	// Allow no arg to reset
	// if (GPIBNArgs > 2)
	//	return S3_GPIB_TOO_MANY_PARAS;

	if (GPIBNArgs == 1)
		*S3Data->m_TestName = '\0';
	else
		strcpy_s(S3Data->m_TestName, S3_MAX_FILENAME_LEN, GPIBCmdBuf + 9);
		//	GPIBCmdArgs[1]);

	return 0;
}

// ----------------------------------------------------------------------------

int CmdIPTESTSIG()
{
	if (GPIBNArgs == 2)
	{
		// Try GPIBRx, GPIBTx, GPIBIP
		if (!S3IPValidQ(GPIBRx, GPIBTx, GPIBIP))
			return S3_GPIB_INVALID_ADDRESS;

		if (!STRNCMP(GPIBCmdArgs[1], "ON", 2))
		{
			if (S3IPSetTestToneEnable(GPIBRx, GPIBTx, GPIBIP, 1) == 2)
				return S3_GPIB_TX_PROT_MODE;
		}
		else if (!STRNCMP(GPIBCmdArgs[1], "OFF", 3))
		{
			if (S3IPSetTestToneEnable(GPIBRx, GPIBTx, GPIBIP, 0) == 2)
				return S3_GPIB_TX_PROT_MODE;
		}
		else
			return S3_GPIB_INVALID_PARAMETER;
	}
	else if (GPIBNArgs == 5)
	{
		// Try to get an address
		char	all, Rx, Tx, IP;
		int		res = GetAddress2(&all, &Rx, &Tx, &IP);
		
		if (res < 0)
		{
			if (res == -1 || res == -2)
				return S3_GPIB_MALFORMED_ADDRESS;
			if (res == -3)
				return S3_GPIB_INVALID_ADDRESS;
			if (res == -4)
				return S3_GPIB_OUT_RANGE_ADDRESS;
		}
		else if (res > 2000)
			return res;

		if (!STRCMP(GPIBCmdArgs[4], "ON"))
		{
			if (S3IPSetTestToneEnable(Rx, Tx, IP, 1) == 2)
				return S3_GPIB_TX_PROT_MODE;
		}
		else if (!STRCMP(GPIBCmdArgs[4], "OFF"))
		{
			if (S3IPSetTestToneEnable(Rx, Tx, IP, 0) == 2)
				return S3_GPIB_TX_PROT_MODE;
		}
		else
			return S3_GPIB_INVALID_PARAMETER;
	}
	else
		return S3_GPIB_ERR_NUMBER_PARAS;

	return 0;
}

// ----------------------------------------------------------------------------

int CmdRX()
{
	if (GPIBNArgs != 2)
		return S3_GPIB_ERR_NUMBER_PARAS;

	char	val = GetAddArg(GPIBCmdArgs[1]);

	if (val < 0 || val >= S3_MAX_RXS)
		return S3_GPIB_INVALID_ADDRESS;

	if (!S3RxExistQ((unsigned char)val))
		return S3_GPIB_RX_NOT_EXIST;

	GPIBRx = val;

	return 0;
}

// ----------------------------------------------------------------------------

int CmdTX()
{
	if (GPIBNArgs != 2)
		return S3_GPIB_ERR_NUMBER_PARAS;

	if (GPIBRx == -1)
	{
		GPIBTx = -1;

		return S3_GPIB_NO_RX_SEL;
	}

	// Just set the GPIB current Tx
	char	val = GetAddArg(GPIBCmdArgs[1]);

	if (val < 0 || val > S3_MAX_TXS)
		return S3_GPIB_INVALID_ADDRESS;

	if (!S3TxExistQ(GPIBRx, val))
		return S3_GPIB_TX_NOT_EXIST;

	GPIBTx = val;

	return 0;
}

// ----------------------------------------------------------------------------

int CmdIP()
{
	if (GPIBNArgs != 2)
		return S3_GPIB_ERR_NUMBER_PARAS;

	if (GPIBRx == -1)
	{
		GPIBTx = -1;

		return S3_GPIB_NO_RX_SEL;
	}

	if (GPIBTx == -1)
		return S3_GPIB_NO_TX_SEL;

	char	val = GetAddArg(GPIBCmdArgs[1]);

	if (!S3IPValidQ(GPIBRx, GPIBTx, val))
		return S3_GPIB_INVALID_RF_IP;

	GPIBIP = val;

	return 0;
}

// ----------------------------------------------------------------------------
// SET

int CmdGAIN()
{
	short	val;

	if (GPIBNArgs < 2)
		return S3_GPIB_ERR_NUMBER_PARAS;

	int	err;
	if (GPIBNArgs == 2)
	{
		// Try GPIBRx, GPIBTx, GPIBIP
		if (err = S3IPInvalidQ(GPIBRx, GPIBTx, GPIBIP))
			return err;

		if (!STRCMP(GPIBCmdArgs[4], "CLEAR"))
		{
			if (S3IPGetAlarms(GPIBRx, GPIBTx, GPIBIP) & S3_IP_OVERDRIVE)
			{
				S3IPCancelAlarm(GPIBRx, GPIBTx, GPIBIP, S3_IP_OVERDRIVE);
				S3TxClearPeakHold(GPIBRx, GPIBTx, 0);
			}
			else
				return S3_GPIB_INVALID_PARAMETER;
		}
		else
		{
			val = GetShortArg(GPIBCmdArgs[1]);

			if (val == SHRT_MIN)
				return S3_GPIB_INVALID_PARAMETER;

			int GainLimited = S3SetGain(GPIBRx, GPIBTx, GPIBIP, (char)val);

			if (GainLimited == 1)
				return S3_GPIB_GAIN_LIMITED;
			else if (GainLimited == 2)
				return S3_GPIB_TX_PROT_MODE;
		}
	}
    else if (GPIBNArgs == 3 && !STRCMP(GPIBCmdArgs[1],"DEF"))
    {
        val = GetShortArg(GPIBCmdArgs[2]);
        if (val == SHRT_MIN)
			return S3_GPIB_INVALID_PARAMETER;

		if (S3SetGain(-1, -1, -1, (char)val) == 1)
			return S3_GPIB_GAIN_LIMITED;
    }
	else if (GPIBNArgs == 5)
	{
		// Try to get an address
		char		all, Rx, Tx, IP;
		int res = GetAddress2(&all, &Rx, &Tx, &IP);
		
		if (res < 0)
		{
			if (res == -1 || res == -2)
				return S3_GPIB_MALFORMED_ADDRESS;
			if (res == -3)
				return S3_GPIB_INVALID_ADDRESS;
			if (res == -4)
				return S3_GPIB_OUT_RANGE_ADDRESS;
		}
		else if (res > 2000)
			return res;

		if (!STRCMP(GPIBCmdArgs[4], "CLEAR"))
		{
			if (S3IPGetAlarms(Rx, Tx, IP) & S3_IP_OVERDRIVE)
			{
				S3IPCancelAlarm(Rx, Tx, IP, S3_IP_OVERDRIVE);
				S3TxClearPeakHold(Rx, Tx, 0);
			}
			else
				return S3_GPIB_INVALID_PARAMETER;
		}
		else
		{
			val = GetShortArg(GPIBCmdArgs[4]);
		
			if (val == SHRT_MIN)
				return S3_GPIB_INVALID_PARAMETER;

			int GainLimited = S3SetGain(Rx, Tx, IP, (char)val);

			if (GainLimited == 1)
				return S3_GPIB_GAIN_LIMITED;
			else if (GainLimited == 2)
				return S3_GPIB_TX_PROT_MODE;
		}
	}
	else
		return S3_GPIB_ERR_NUMBER_PARAS;

	return 0;
}

// ----------------------------------------------------------------------------

int CmdMAXIP()
{
	long int	val;
	char		*pEnd;

	if (GPIBNArgs < 2)
		return S3_GPIB_ERR_NUMBER_PARAS;

	if (GPIBNArgs == 2)
	{
		// Try GPIBRx, GPIBTx, GPIBIP
		if (!S3IPValidQ(GPIBRx, GPIBTx, GPIBIP))
			return S3_GPIB_INVALID_ADDRESS;

		val = strtol(GPIBCmdArgs[1], &pEnd, 10);

		if (S3IPSetMaxInput(GPIBRx, GPIBTx, GPIBIP, (char)val))
			return S3_GPIB_INVALID_PARAMETER;
	}
	else if (GPIBNArgs == 5)
	{
		char		all, Rx, Tx, IP;
		int res = GetAddress2(&all, &Rx, &Tx, &IP);
		
		if (res < 0)
		{
			if (res == -1 || res == -2)
				return S3_GPIB_MALFORMED_ADDRESS;
			if (res == -3)
				return S3_GPIB_INVALID_ADDRESS;
			if (res == -4)
				return S3_GPIB_OUT_RANGE_ADDRESS;
		}
		else if (res > 2000)
			return res;

		val = strtol(GPIBCmdArgs[4], &pEnd, 10);
		if (S3IPSetMaxInput(Rx, Tx, IP, (char)val))
			return S3_GPIB_INVALID_PARAMETER;
	}
	else
		return S3_GPIB_ERR_NUMBER_PARAS;

	return 0;
}

// ----------------------------------------------------------------------------
// SET

int CmdIMP()
{
	if (GPIBNArgs < 2)
		return S3_GPIB_ERR_NUMBER_PARAS;

	if (GPIBNArgs == 2)
	{
		// Try GPIBRx, GPIBTx, GPIBIP
		if (!S3IPValidQ(GPIBRx, GPIBTx, GPIBIP))
			return S3_GPIB_INVALID_ADDRESS;

		InputZ IPz = S3Str2InputZ(GPIBCmdArgs[1]);
		if (IPz == ZUnknown)
			return S3_GPIB_INVALID_PARAMETER;
		
		int update = S3SetImpedance(GPIBRx, GPIBTx, GPIBIP, IPz);
		if (update == 1)
			return S3_GPIB_GAIN_CHANGED;
		else if (update == 2)
			return S3_GPIB_TX_PROT_MODE;
	}
	else if (GPIBNArgs == 5)
	{
		// Try to get an address
		char	all, Rx, Tx, IP;
		int		res = GetAddress2(&all, &Rx, &Tx, &IP);
		InputZ	IPz;

		if (res < 0)
		{
			if (res == -1 || res == -2)
				return S3_GPIB_MALFORMED_ADDRESS;
			if (res == -3)
				return S3_GPIB_INVALID_ADDRESS;
			if (res == -4)
				return S3_GPIB_OUT_RANGE_ADDRESS;
		}
		else if (res > 2000)
			return res;
		else if (res == 100) // TODO: "ALL" Disabled
			IPz = S3Str2InputZ(GPIBCmdArgs[2]);
		else
			IPz = S3Str2InputZ(GPIBCmdArgs[4]);
		
		if (IPz == ZUnknown)
			return S3_GPIB_INVALID_PARAMETER;
		
		int update = S3SetImpedance(Rx, Tx, IP, IPz);
		if (update == 1)
			return S3_GPIB_GAIN_CHANGED;
		else if (update == 2)
			return S3_GPIB_TX_PROT_MODE;
	}
	else
		return S3_GPIB_ERR_NUMBER_PARAS;

	return 0;
}

// ----------------------------------------------------------------------------
// SET

int CmdCAL()
{
	if (GPIBNArgs == 2)
	{
		// Try GPIBRx, GPIBTx, GPIBIP
		if (!S3IPValidQ(GPIBRx, GPIBTx, GPIBIP))
			return S3_GPIB_INVALID_ADDRESS;

		if (!STRCMP(GPIBCmdArgs[1], "ON"))
			S3TxSetTestToneIP(GPIBRx, GPIBTx, GPIBIP);
		else if (!STRCMP(GPIBCmdArgs[1], "OFF"))
			S3TxSetTestToneIP(GPIBRx, GPIBTx, -1);
		else
			return S3_GPIB_INVALID_PARAMETER;
	}
	else if (GPIBNArgs == 5)
	{
		// Try to get an address
		char	all, Rx, Tx, IP;
		int		res = GetAddress2(&all, &Rx, &Tx, &IP);
		
		if (res < 0)
		{
			if (res == -1 || res == -2)
				return S3_GPIB_MALFORMED_ADDRESS;
			if (res == -3)
				return S3_GPIB_INVALID_ADDRESS;
			if (res == -4)
				return S3_GPIB_OUT_RANGE_ADDRESS;
		}
		else if (res > 2000)
			return res;

		if (!STRCMP(GPIBCmdArgs[4], "ON"))
			S3IPSetTestToneEnable(Rx, Tx, IP, 1);
		else if (!STRCMP(GPIBCmdArgs[4], "OFF"))
			S3IPSetTestToneEnable(Rx, Tx, IP, 0);
		else
			return S3_GPIB_INVALID_PARAMETER;
	}
	else
		return S3_GPIB_ERR_NUMBER_PARAS;

	return 0;
}

// ----------------------------------------------------------------------------
// SET

int CmdUNITS()
{
	if (GPIBNArgs != 2)
		return S3_GPIB_ERR_NUMBER_PARAS;

	if (!STRCMP(GPIBCmdArgs[1], "DBM"))
		S3SetUnits(S3_UNITS_DBM);
	else if (!STRCMP(GPIBCmdArgs[1], "DBUV"))
		S3SetUnits(S3_UNITS_DBUV);
	else if (!STRCMP(GPIBCmdArgs[1], "MV"))
		S3SetUnits(S3_UNITS_MV);
	else
		return S3_GPIB_INVALID_PARAMETER;

	return 0;
}

// ----------------------------------------------------------------------------

int CmdSYS()
{
	S3RxReport(GPIBRetBuf, GPIBRx); 
	S3TxReport(GPIBRetBuf, GPIBRx, GPIBTx);

	// For each IP, 
	for (unsigned char i = 0; i < S3TxGetNIP(GPIBRx, GPIBTx); i++)
		S3IPReport(GPIBRetBuf, GPIBRx, GPIBTx, i);

	return 0;
}

// ----------------------------------------------------------------------------

int CmdREPORT()
{
	if (GPIBNArgs < 2 && GPIBNArgs > 5)
		return S3_GPIB_ERR_NUMBER_PARAS;

	int err = 0;

	char	all, Rx, Tx, IP;
	const char *cmd;

	cmd = GPIBCmdArgs[GPIBNArgs - 1];

	if (GPIBNArgs != 2)
	{
		int		res = GetAddress2(&all, &Rx, &Tx, &IP);
		
		if (res < 0)
		{
			if (res == -1 || res == -2)
				return S3_GPIB_MALFORMED_ADDRESS;
			if (res == -3)
				return S3_GPIB_INVALID_ADDRESS;
			if (res == -4)
				return S3_GPIB_OUT_RANGE_ADDRESS;
		}
		else if (res > 2000)
			return res;
	}
	else
	{
		Rx = GPIBRx;
		Tx = GPIBTx;
		IP = GPIBIP;

		// cmd = GPIBCmdArgs[1];
	}

	if (!STRCMP(cmd, "ALL"))
	{
		err = CmdSYS();
	}
	else if (!STRCMP(cmd, "TOPOLOGY"))
	{
		err = S3TopologyReport(GPIBRetBuf);
	}
	else if (!STRCMP(cmd, "SYS"))
	{
		err = S3SysReport(GPIBRetBuf);
	}
	else if (!STRCMP(cmd, "RX"))
	{
		int err = S3IPInvalidQ(Rx, Tx, IP);
		if (err == S3_GPIB_RX_NOT_EXIST)
			return err;

		err = S3RxReport(GPIBRetBuf, Rx);
	}
	else if (!STRCMP(cmd, "TX"))
	{
		int err = S3IPInvalidQ(Rx, Tx, IP);
		if (err == S3_GPIB_TX_NOT_EXIST)
			return err;

		err = S3TxReport(GPIBRetBuf, Rx, Tx);
	}
	else if (!STRCMP(cmd, "IP"))
	{
		int err = S3IPInvalidQ(Rx, Tx, IP);
		if (err)
			return err;

		err = S3IPReport(GPIBRetBuf, Rx, Tx, IP);
	}
	else
	{
		return S3_GPIB_INVALID_PARAMETER;
	}

	// if (err)
	// 	return S3_GPIB_INVALID_IP;

	// Dump to file if required
	S3ReportToFile(GPIBRetBuf);

	// Indicate success
	// strcat_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN - strlen(GPIBRetBuf), ": OK");

	strcpy_s(GPIBRetBufTmp, S3_MAX_GPIB_RET_LEN, GPIBRetBuf);
	sprintf_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "I:\n%s", GPIBRetBufTmp);

	return 0;
}

// ----------------------------------------------------------------------------

int CmdLOCAL()
{
	S3SetRemote(false);
	return 0;
}

// ----------------------------------------------------------------------------

int CmdREMOTE()
{
	S3SetRemote(true);
	return 0;
}

// ----------------------------------------------------------------------------

int CmdSTAT()
{
	return 0;
}

// ----------------------------------------------------------------------------

int CmdSETQ()
{
	return 0;
}

// ----------------------------------------------------------------------------

int CmdSET()
{
	char	startarg;
	char	all, Rx, Tx, IP;

	if (GPIBNArgs == 8)
	{
		int		res = GetAddress2(&all, &Rx, &Tx, &IP);
		
		if (res < 0)
		{
			if (res == -1 || res == -2)
				return S3_GPIB_MALFORMED_ADDRESS;
			if (res == -3)
				return S3_GPIB_INVALID_ADDRESS;
			if (res == -4)
				return S3_GPIB_OUT_RANGE_ADDRESS;
		}
		else if (res > 2000)
			return res;

		startarg = 4;
	}
	else if (GPIBNArgs == 5)
	{
		int err = S3IPInvalidQ(GPIBRx, GPIBTx, GPIBIP);
		if (err)
			return err;
		
		Rx = GPIBRx;
		Tx = GPIBTx;
		IP = GPIBIP;

		startarg = 1;
	}
	else return S3_GPIB_ERR_NUMBER_PARAS;

	if (S3IPGetAlarms(Rx, Tx, IP) & S3_IP_OVERDRIVE)
		return S3_GPIB_TX_PROT_MODE;

	// if (!S3IPValidQ(GPIBRx, GPIBTx, GPIBIP))
	//	return S3_GPIB_INVALID_ADDRESS;

	// int err = S3IPInvalidQ(Rx, Tx, IP);
	// if (err)
	//	return err;

	int err = 0;

	// TODO: Contradictory settings need to be resolved before applying

	// SET [rx tx ip] G M T Z
	if (STRCMP(GPIBCmdArgs[startarg], "-"))
	{
		short val = GetShortArg(GPIBCmdArgs[startarg]);
		if (val == SHRT_MIN)
			return S3_GPIB_INVALID_PARAMETER;

		if (S3SetGain(Rx, Tx, IP, (char)val) == 1)
			err = S3_GPIB_GAIN_LIMITED;
	}

	startarg++;

	if (STRCMP(GPIBCmdArgs[startarg], "-"))
	{
		double vald = GetDoubleArg(GPIBCmdArgs[startarg]);
		if (vald == DBL_MIN)
			return S3_GPIB_INVALID_PARAMETER;

		if (S3IPSetMaxInput(Rx, Tx, IP, vald))
			return S3_GPIB_INVALID_PARAMETER;
	}

	startarg++;

	if (STRCMP(GPIBCmdArgs[startarg], "-"))
	{
		SigmaT Tau = S3Str2SigmaT(GPIBCmdArgs[startarg]);
		if (Tau == TauUnknown)
			return S3_GPIB_INVALID_PARAMETER;

		if (S3SetSigmaTau(Rx, Tx, IP, Tau) == 1)
			err = S3_GPIB_GAIN_LIMITED;
	}

	startarg++;

	if (STRCMP(GPIBCmdArgs[startarg], "-"))
	{
		InputZ IPz = S3Str2InputZ(GPIBCmdArgs[startarg]);
		if (IPz == ZUnknown)
			return S3_GPIB_INVALID_PARAMETER;

		if (S3SetImpedance(Rx, Tx, IP, IPz) == 1)
			err = S3_GPIB_GAIN_CHANGED;
	}

	startarg++;

	return err;
}

// ----------------------------------------------------------------------------

int CmdGET()
{
	char	all, Rx, Tx, IP;

	if (GPIBNArgs == 4)
	{

		int		res = GetAddress2NoArg(&all, &Rx, &Tx, &IP);
			
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

			// We need the full address
			if (res != 3)
				return S3_GPIB_INVALID_ADDRESS;
		}
	}
	else if (GPIBNArgs == 1)
	{
		int err = S3IPInvalidQ(GPIBRx, GPIBTx, GPIBIP);
		if (err)
			return err;
		
		Rx = GPIBRx;
		Tx = GPIBTx;
		IP = GPIBIP;
	}
	else return S3_GPIB_ERR_NUMBER_PARAS;

	// GET [rx tx ip] G M T Z
	char gain =		S3IPGetGain(Rx, Tx, IP);
	double maxip =	S3IPGetMaxInput(Rx, Tx, IP);

	char tau[S3_PARA_NAME_LEN];
	S3IPGetSigmaTauS(tau, Rx, Tx, IP);

	char IPz[S3_PARA_NAME_LEN];
	S3IPGetInputZS(IPz, Rx, Tx, IP);

	sprintf_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "I: %d,%.3f,%s,%s",
		gain, maxip, tau, IPz); 

	return 0;
}

// ----------------------------------------------------------------------------

int CmdGPIB()
{
	return 0;
}

// ----------------------------------------------------------------------------

int CmdLOWNOISE()
{
	char		all, Rx, Tx, IP;

	if (GPIBNArgs < 2)
		return S3_GPIB_INVALID_PARAMETER;

	if (GPIBNArgs == 2)
	{
		// Try GPIBRx, GPIBTx, GPIBIP

		if (!S3IPValidQ(GPIBRx, GPIBTx, GPIBIP))
			return S3_GPIB_INVALID_ADDRESS;

		if (!STRCMP(GPIBCmdArgs[1], "ON"))
		{
			if (S3IPSetLowNoiseMode(GPIBRx, GPIBTx, GPIBIP, true))
				return S3_GPIB_GAIN_CHANGED;
		}
		else if (!STRCMP(GPIBCmdArgs[1], "OFF"))
		{
			if (S3IPSetLowNoiseMode(GPIBRx, GPIBTx, GPIBIP, false))
				return S3_GPIB_GAIN_CHANGED;
		}
		else
			return S3_GPIB_INVALID_PARAMETER;
	}
	else if (GPIBNArgs == 5)
	{
		// Try to get an address
		int res = GetAddress2(&all, &Rx, &Tx, &IP);
		
		if (res < 0)
		{
			if (res == -1 || res == -2)
				return S3_GPIB_MALFORMED_ADDRESS;
			if (res == -3)
				return S3_GPIB_INVALID_ADDRESS;
			if (res == -4)
				return S3_GPIB_OUT_RANGE_ADDRESS;
		}
		else if (res > 2000)
			return res;

		if (!STRCMP(GPIBCmdArgs[GPIBNArgs - 1], "ON"))
		{
			if (S3IPSetLowNoiseMode(Rx, Tx, IP, true))
				return S3_GPIB_GAIN_CHANGED;
		}
		else if (!STRCMP(GPIBCmdArgs[GPIBNArgs - 1], "OFF"))
		{
			if (S3IPSetLowNoiseMode(Rx, Tx, IP, false))
				return S3_GPIB_GAIN_CHANGED;
		}
		else
			return S3_GPIB_INVALID_PARAMETER;
	}
	else
		return S3_GPIB_ERR_NUMBER_PARAS;

	return 0;
}

// ----------------------------------------------------------------------------

int CmdITAU()
{
	if (GPIBNArgs < 2)
		return S3_GPIB_TOO_FEW_PARAS;

	if (GPIBNArgs == 2)
	{
		// Try GPIBRx, GPIBTx, GPIBIP
		if (!S3IPValidQ(GPIBRx, GPIBTx, GPIBIP))
			return S3_GPIB_INVALID_ADDRESS;

		if (S3IPGetAlarms(GPIBRx, GPIBTx, GPIBIP) & S3_IP_OVERDRIVE)
			return S3_GPIB_TX_PROT_MODE;

		SigmaT	Tau = S3Str2SigmaT(GPIBCmdArgs[1]);
		if (Tau == TauUnknown)
			return S3_GPIB_INVALID_PARAMETER;

		if (S3SetSigmaTau(GPIBRx, GPIBTx, GPIBIP, Tau))
			return S3_GPIB_GAIN_CHANGED;
	}
	else if (GPIBNArgs == 5)
	{
		// Try to get an address
		char	all, Rx, Tx, IP;
		int		res = GetAddress2(&all, &Rx, &Tx, &IP);

		if (S3IPGetAlarms(Rx, Tx, IP) & S3_IP_OVERDRIVE)
			return S3_GPIB_TX_PROT_MODE;
		
		if (res < 0)
		{
			if (res == -1 || res == -2)
				return S3_GPIB_MALFORMED_ADDRESS;
			if (res == -3)
				return S3_GPIB_INVALID_ADDRESS;
			if (res == -4)
				return S3_GPIB_OUT_RANGE_ADDRESS;
		}
		else if (res > 2000)
			return res;

		SigmaT	Tau = S3Str2SigmaT(GPIBCmdArgs[4]);
		if (Tau == TauUnknown)
			return S3_GPIB_INVALID_PARAMETER;

		if (S3SetSigmaTau(Rx, Tx, IP, Tau))
			return S3_GPIB_GAIN_CHANGED;
	}
	else
		return S3_GPIB_ERR_NUMBER_PARAS;
	
	return 0;
}

// ----------------------------------------------------------------------------

int CmdWINTRACK()
{
	if (GPIBNArgs == 2)
	{
		// Try GPIBRx, GPIBTx, GPIBIP
		if (!S3IPValidQ(GPIBRx, GPIBTx, GPIBIP))
			return S3_GPIB_INVALID_ADDRESS;

		if (!STRNCMP(GPIBCmdArgs[1], "ON", 2))
			S3IPWindowTrack(GPIBRx, GPIBTx, GPIBIP, (unsigned char)1);
		else if (!STRNCMP(GPIBCmdArgs[1], "OFF", 3))
			S3IPWindowTrack(GPIBRx, GPIBTx, GPIBIP, (unsigned char)0);
		else
			return S3_GPIB_INVALID_PARAMETER;
	}
	else if (GPIBNArgs == 5)
	{
		// Try to get an address
		char	all, Rx, Tx, IP;
		int		res = GetAddress2(&all, &Rx, &Tx, &IP);
		
		if (res < 0)
		{
			if (res == -1 || res == -2)
				return S3_GPIB_MALFORMED_ADDRESS;
			if (res == -3)
				return S3_GPIB_INVALID_ADDRESS;
			if (res == -4)
				return S3_GPIB_OUT_RANGE_ADDRESS;
		}
		else if (res > 2000)
			return res;

		if (!STRCMP(GPIBCmdArgs[4], "ON"))
			S3IPWindowTrack(Rx, Tx, IP, (unsigned char)1);
		else if (!STRCMP(GPIBCmdArgs[4], "OFF"))
			S3IPWindowTrack(Rx, Tx, IP, (unsigned char)0);
		else
			return S3_GPIB_INVALID_PARAMETER;
	}
	else
		return S3_GPIB_ERR_NUMBER_PARAS;

	return 0;
}

// ----------------------------------------------------------------------------

int CmdLOAD()
{
	if (GPIBNArgs < 2)
		return S3_GPIB_TOO_FEW_PARAS;
	else if (GPIBNArgs > 2)
		return S3_GPIB_TOO_MANY_PARAS;

	char Filename[S3_MAX_FILENAME_LEN];

	strcpy_s(Filename, S3_MAX_FILENAME_LEN, GPIBCmdArgs[1]);

	int err = S3Read(Filename);

	return err;
}

// ----------------------------------------------------------------------------

int CmdSAVE()
{
	if (GPIBNArgs < 2)
		return S3_GPIB_TOO_FEW_PARAS;
	else if (GPIBNArgs > 2)
		return S3_GPIB_TOO_MANY_PARAS;

	char Filename[S3_MAX_FILENAME_LEN];

	strcpy_s(Filename, S3_MAX_FILENAME_LEN, GPIBCmdArgs[1]);

	int	err = S3Save(Filename);

	return err;
}

// ----------------------------------------------------------------------------

int CmdREPORTF()
{
	if (GPIBNArgs < 2)
		return S3_GPIB_TOO_FEW_PARAS;
	else if (GPIBNArgs > 2)
		return S3_GPIB_TOO_MANY_PARAS;

	strcpy_s(S3Data->m_ReportFileName, S3_MAX_FILENAME_LEN,
		GPIBCmdArgs[1]);

	return 0;
}

// ----------------------------------------------------------------------------

int CmdLOGF()
{
	if (GPIBNArgs < 2)
		return S3_GPIB_MISSING_PARAMETER;
	
	// OK for file or path/file
	if (S3EventLogInit(GPIBCmdArgs[1]))
		return S3_GPIB_LOG_INIT_FAILED;

	return 0;
}

// ----------------------------------------------------------------------------

int CmdTCOMPMODE()
{
	if (GPIBNArgs < 2)
		return S3_GPIB_MISSING_PARAMETER;

	if (S3GetTCompGainOption())
	{
		if (!STRCMP(GPIBCmdArgs[1], "OFF"))
			S3SetTCompMode(0);
		else if (!STRCMP(GPIBCmdArgs[1], "CONT"))
			S3SetTCompMode(1);
		else if (!STRCMP(GPIBCmdArgs[1], "GAIN"))
			S3SetTCompMode(2);
		else
			return S3_GPIB_INVALID_PARAMETER;
	}
	else
	{
		if (GPIBNArgs < 2)
			return S3_GPIB_MISSING_PARAMETER;

		if (!STRCMP(GPIBCmdArgs[1], "OFF"))
			S3SetTCompMode(S3_TCOMP_OFF);
		else if (!STRCMP(GPIBCmdArgs[1], "ON"))
			S3SetTCompMode(S3_TCOMP_CONT);
		else
			return S3_GPIB_INVALID_PARAMETER;

	}

	return 0;
}

// ----------------------------------------------------------------------------

int CmdTCOMP()
{
	if (GPIBNArgs < 2)
		return S3_GPIB_MISSING_PARAMETER;

	if (GPIBNArgs == 2)
	{
		if (!STRCMP(GPIBCmdArgs[1], "ALL"))
		{
			S3DoComp(-1, -1);
		}
		else
			return S3_GPIB_INVALID_PARAMETER;
	}
	else if (GPIBNArgs == 3)
	{
		// Try to get an address
		char		all, Rx, Tx, IP;
		int res = GetAddress2NoArg(&all, &Rx, &Tx, &IP);
		
		if (res > 0 && res < 3)
		{
			S3DoComp(Rx, Tx);
		}
		else if (res < 0)
		{
			if (res == -1 || res == -2)
				return S3_GPIB_MALFORMED_ADDRESS;
			if (res == -3)
				return S3_GPIB_INVALID_ADDRESS;
			if (res == -4)
				return S3_GPIB_OUT_RANGE_ADDRESS;
		}
		else if (res > 2000)
			return res;
	}
	else
		return S3_GPIB_ERR_NUMBER_PARAS;

	return 0;
}

// ----------------------------------------------------------------------------
// Rx6 only

int CmdSELECTTX()
{
	char	Rx, Tx, IP;

	if (GPIBNArgs == 3) // Rx address
	{
		int res = GetAddress2NoArg(NULL, &Rx, &Tx, &IP);

		if (res != 2 || !S3TxValidQ(Rx, Tx))
			return S3_GPIB_INVALID_ADDRESS;

		if (res > 2000)
			return res;

		S3RxSetActiveTx(Rx, Tx);

	}
	else if (GPIBNArgs == 2) // No args found
	{
		Tx = GetAddArg(GPIBCmdArgs[1]);

		if (Tx < 0 || !S3TxValidQ(GPIBRx, Tx))
			return S3_GPIB_INVALID_IP;

		S3RxSetActiveTx(GPIBRx,Tx);
	}
	else
		return S3_GPIB_ERR_NUMBER_PARAS;

	return 0;
}

// ----------------------------------------------------------------------------
// Tx8 only

int CmdSELECTIP()
{
	char	Rx, Tx, IP;

	if (GPIBNArgs == 4) // Tx address
	{
		int res = GetAddress2NoArg(NULL, &Rx, &Tx, &IP);

		if (res != 3 || !S3IPValidQ(Rx, Tx, IP))
			return S3_GPIB_INVALID_ADDRESS;

		if (res > 2000)
			return res;

		S3TxSetActiveIP(Rx, Tx, IP);

	}
	else if (GPIBNArgs == 2) // No args found
	{
		IP = GetAddArg(GPIBCmdArgs[1]);

		if (!S3IPValidQ(GPIBRx, GPIBTx, IP))
			return S3_GPIB_INVALID_ADDRESS;

		S3TxSetActiveIP(GPIBRx, GPIBTx, IP);
	}
	else
		return S3_GPIB_ERR_NUMBER_PARAS;

	return 0;
}

// ----------------------------------------------------------------------------
// GET

int CmdTXCHARGE()
{
	if (GPIBNArgs != 1 && GPIBNArgs != 3)
		return S3_GPIB_ERR_NUMBER_PARAS;

	char			all, Rx, Tx, IP;
	unsigned char	SoC;

	int res = GetAddress2NoArg(&all, &Rx, &Tx, &IP);

	// No args found
	if (res == 0)
	{
		if (!S3TxValidQ(GPIBRx, GPIBTx))
			return S3_GPIB_INVALID_TX;

		SoC = S3TxGetBattSoC(GPIBRx, GPIBTx);
	}
	else if (res < 0)
	{
		if (res == -1 || res == -2)
			return S3_GPIB_MALFORMED_ADDRESS;
		if (res == -3)
			return S3_GPIB_INVALID_ADDRESS;
		if (res == -4)
			return S3_GPIB_OUT_RANGE_ADDRESS;
	}
	else if (res > 2000)
		return res;
		
	SoC = S3TxGetBattSoC(Rx, Tx);

	// TODO: Use this format for all returns, use "Err:" in front of all
	// error returns?
	sprintf_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "I: %d", SoC);
	
	return 0;
}

// ----------------------------------------------------------------------------
// SET

int CmdTXSLEEP()
{
	if (GPIBNArgs < 2 || GPIBNArgs > 4)
		return S3_GPIB_ERR_NUMBER_PARAS;

	char			all, Rx, Tx, IP;
	unsigned char	p;

	int res = GetAddress2(&all, &Rx, &Tx, &IP);

	if (res > 2000)
		return res;

	if (!STRCMP(GPIBCmdArgs[GPIBNArgs - 1], "ON"))
		p = S3_TX_ON;
	else if (!STRCMP(GPIBCmdArgs[GPIBNArgs - 1], "SLEEP"))
		p = S3_TX_SLEEP;
	else
		return S3_GPIB_INVALID_MODE;

	if (all != -1)
	{
		S3TxSetPowerStat(-1, -1, p);
		return 0;
	}

	// No args found
	if (res == 0)
	{
		if (!S3TxValidQ(GPIBRx, GPIBTx))
			return S3_GPIB_INVALID_TX;

		S3TxSetPowerStat(GPIBRx, GPIBTx, p);

		if (p == S3_TX_SLEEP)
			S3TxSetUserSleep(Rx, Tx, true);
		else
			S3TxSetUserSleep(Rx, Tx, false);
	}
	else if (res < 0)
	{
		if (res == -1 || res == -2)
			return S3_GPIB_MALFORMED_ADDRESS;
		if (res == -3)
			return S3_GPIB_INVALID_ADDRESS;
		if (res == -4)
			return S3_GPIB_OUT_RANGE_ADDRESS;
	}
	else if (res == 2)
		S3TxSetPowerStat(Rx, Tx, p);
	
	return 0;
}

// ----------------------------------------------------------------------------

int CmdSYSRESET()
{
	return S3Reset();
}

// ----------------------------------------------------------------------------

int CmdSYSSETTIME()
{
	if (GPIBNArgs != 7)
		return S3_GPIB_ERR_NUMBER_PARAS;

	short Y, M, D, h, m, s;

	if ((Y = GetShortArg(GPIBCmdArgs[1])) == SHRT_MIN)
		return S3_GPIB_INVALID_PARAMETER;
	if ((M = GetShortArg(GPIBCmdArgs[2])) == SHRT_MIN)
		return S3_GPIB_INVALID_PARAMETER;
	if ((D = GetShortArg(GPIBCmdArgs[3])) == SHRT_MIN)
		return S3_GPIB_INVALID_PARAMETER;
	if ((h = GetShortArg(GPIBCmdArgs[4])) == SHRT_MIN)
		return S3_GPIB_INVALID_PARAMETER;
	if ((m = GetShortArg(GPIBCmdArgs[5])) == SHRT_MIN)
		return S3_GPIB_INVALID_PARAMETER;
	if ((s = GetShortArg(GPIBCmdArgs[6])) == SHRT_MIN)
		return S3_GPIB_INVALID_PARAMETER;

	SYSTEMTIME SysT;
	SysT.wYear = Y;
	SysT.wMonth = M;
	SysT.wDayOfWeek = 0;
	SysT.wDay = D;
	SysT.wHour = h;
	SysT.wMinute = m;
	SysT.wSecond = s;
	SysT.wMilliseconds = 0;

	BOOL ok = SetSystemTime(&SysT);

	if (!ok)
		return S3_GPIB_TIME_CHANGE_FAILED;

	return 0;
}

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

	S3SetFactoryMode(Rx, Tx, true);

	if (S3I2CRxSetCalibration(Rx, Tx, val))
		return S3_GPIB_CALIBRATION_FAILED;

	S3SetFactoryMode(Rx, -1, false);

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

	// Sleep(1000);

	if (1)
	{
		if (S3I2CTxSetRFCalibration(path, val))
			return S3_GPIB_CALIBRATION_FAILED;
	}
	else
		Sleep(2000);

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

	if (1)
	{
		if (S3I2CTxSetOptCalibration(val))
			return S3_GPIB_CALIBRATION_FAILED;
	}
	else
		Sleep(2000);

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

	S3I2CChMS(Ch); // S3SetFactoryMode sets to 0

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

int CmdPPMRXID()
{
	if (S3GetLocked())
		return S3_GPIB_COMMAND_LOCKED;
	
	if (GPIBNArgs != 4)
		return S3_GPIB_ERR_NUMBER_PARAS;

	// TODO: Get Rx from command?
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

int CmdSHUTDOWN()
{
    S3EventLogAdd("System shutdown requested by user", 1, -1, -1, -1);
    CmdSLEEPALL();

	// TODO: Re-instate MB change
    // AfxGetMainWnd()->PostMessage(REM_SHUTDOWNREQ, 0, 0);
    return 0;
}

// ----------------------------------------------------------------------------

int CmdRESTART()
{
    S3OSRestart();

	// TODO: Re-instate MB change
    // AfxGetMainWnd()->PostMessage(REM_SHUTDOWNREQ, 0, 0);
    return 0;
}

// ----------------------------------------------------------------------------

int CmdSLEEPALL()
{
    S3SetSleepAll(true);
    return 0;
}

// ----------------------------------------------------------------------------

int CmdWAKEALL()
{
    S3SetSleepAll(false);
    return 0;
}

// ----------------------------------------------------------------------------

int CmdNAME()
{
	if (GPIBNArgs < 2)
		return S3_GPIB_ERR_NUMBER_PARAS;

    switch(GPIBNArgs)
    {
    case 2:
        {
            strcpy_s(S3Data->m_NodeName, S3_MAX_NODE_NAME_LEN,
                    GPIBCmdArgs[1]);
        }
        break;
    case 3:
        {
            char	val = GetAddArg(GPIBCmdArgs[1]);

            if (val < 0 || val > S3_MAX_RXS)
                return S3_GPIB_INVALID_ADDRESS;
            
            if (!S3RxExistQ((unsigned char)val))
                return S3_GPIB_RX_NOT_EXIST;

            strcpy_s(S3Data->m_Rx[val].m_NodeName, S3_MAX_NODE_NAME_LEN,
                    GPIBCmdArgs[2]);
        }
        break;
    case 4:
        {
            char	Rxval = GetAddArg(GPIBCmdArgs[1]);

            if (Rxval < 0 || Rxval > S3_MAX_RXS)
                return S3_GPIB_INVALID_ADDRESS;
            
            if (!S3RxExistQ((unsigned char)Rxval))
                return S3_GPIB_RX_NOT_EXIST;

            char Txval = GetAddArg(GPIBCmdArgs[2]); 

            if (Txval < 0 || Txval > S3_MAX_TXS)
                return S3_GPIB_INVALID_ADDRESS;

            if (!S3TxExistQ(Rxval, Txval))
                return S3_GPIB_TX_NOT_EXIST;

            strcpy_s(S3Data->m_Rx[Rxval].m_Tx[Txval].m_NodeName, S3_MAX_NODE_NAME_LEN,
                    GPIBCmdArgs[3]);
        }
        break;
    case 5:
        {
            char	Rxval = GetAddArg(GPIBCmdArgs[1]);

            if (Rxval < 0 || Rxval > S3_MAX_RXS)
                return S3_GPIB_INVALID_ADDRESS;
            
            if (!S3RxExistQ((unsigned char)Rxval))
                return S3_GPIB_RX_NOT_EXIST;

            char Txval = GetAddArg(GPIBCmdArgs[2]); 

            if (Txval < 0 || Txval > S3_MAX_TXS)
                return S3_GPIB_INVALID_ADDRESS;

            if (!S3TxExistQ(Rxval, Txval))
                return S3_GPIB_TX_NOT_EXIST;
            
            char Ipval = GetAddArg(GPIBCmdArgs[3]);

            if (!S3IPValidQ(Rxval, Txval, Ipval))
		        return S3_GPIB_INVALID_RF_IP;

            strcpy_s(S3Data->m_Rx[Rxval].m_Tx[Txval].m_Input[Ipval].m_NodeName, S3_MAX_NODE_NAME_LEN,
                    GPIBCmdArgs[4]);
        }
        break;
    }
    return 0;
}

// ----------------------------------------------------------------------------

int CmdLOGFCOPY()
{
    int result;
    result = S3LogFileToUSBRequest();

    switch(result)
    {
    case 0:
        sprintf_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "Log File Copied");
        break;
    case 1:
        sprintf_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: No USB Device Found");
        break;
    case 2:
        sprintf_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: No Log File to Transfer");
        break;
    case 3:
        sprintf_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Log File Already Exists");
        break;
    }
    return 0;
}

// ----------------------------------------------------------------------------

int CmdSWUPDATE()
{
    int result;
    result = S3OSSWUpdateRequest();

    switch(result)
    {
    case 0:
        sprintf_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "OS Update initialised");
        break;
    case 1:
        sprintf_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: No USB Device Found");
        break;
    case 2:
        sprintf_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: No OS Image Found");
        break;
    }
    return 0;
}

// ----------------------------------------------------------------------------

int CmdTXSTARTSTATE()
{
	if (GPIBNArgs != 2)
		return S3_GPIB_ERR_NUMBER_PARAS;

	if (!STRCMP(GPIBCmdArgs[1], "USER"))
		S3SetTxStartState(S3_TXSTART_USER);
	else if (!STRCMP(GPIBCmdArgs[1], "SLEEP"))
		S3SetTxStartState(S3_TXSTART_SLEEP);
    else if (!STRCMP(GPIBCmdArgs[1], "ON"))
		S3SetTxStartState(S3_TXSTART_ON);
	else
		return S3_GPIB_INVALID_MODE;
	return 0;
}

// ----------------------------------------------------------------------------

int CmdTXSELFTEST()
{
	if (GPIBNArgs != 2)
		return S3_GPIB_ERR_NUMBER_PARAS;

	if (!STRCMP(GPIBCmdArgs[1], "ON"))
		S3SetTxSelfTest(true);
    else if (!STRCMP(GPIBCmdArgs[1], "OFF"))
		S3SetTxSelfTest(false);
	else
		return S3_GPIB_INVALID_MODE;
	return 0;
}

// ----------------------------------------------------------------------------