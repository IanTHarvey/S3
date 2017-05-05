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

unsigned char	GPIBNArgs;
char			GPIBCmd[S3_MAX_GPIB_CMD_LEN];
char			*GPIBCmdArgs[S3_MAX_ARGS];

char		GPIBCmdBuf[S3_MAX_GPIB_CMD_LEN];
char		GPIBRetBuf[S3_MAX_GPIB_RET_LEN];
char		GPIBRetBufTmp[S3_MAX_GPIB_RET_LEN];
unsigned	GPIBBufLen;

char		GPIBRx = -1;
char		GPIBTx = -1; // For settings etc
char		GPIBIP = -1;

extern pS3DataModel S3Data;

int DbgProcessCmd();
unsigned char S3GetArgs();

SigmaT S3Str2SigmaT(const char *str);
InputZ S3Str2InputZ(const char *str);

// TODO: This should be used for single-digit addresses rather than strtol
char	GetAddArg(		const char *carg);
short	GetShortArg(	const char *carg);
double	GetDoubleArg(	const char *carg);

// ----------------------------------------------------------------------------
// In-place de-spacer
int S3DeSpace(char *str)
{
	unsigned	len = strlen(str);
	unsigned	i = 0;

	for(i = 0; i < len; i++)
		 if (str[i] == '\t')
			 str[i] = ' ';

	// Leading
	while (str[0] == ' ')
	{
		for (unsigned j = 0; j < len + 1; j++)
			str[j] = str[j + 1];

		len--;
	}

	// Trailing
	i = len - 1;

	while (str[i] == ' ' || str[i] == '\r' || str[i] == '\n')
	{
		str[i] = '\0';

		i--;
		len--;
	}

	// Multiple repeated
	for (i = 0; i < len; i++)
	{
		if (str[i] == ' ')
		{
			while (str[i + 1] == ' ')
			{
				for (unsigned j = i + 1; j < len + 1; j++)
					str[j] = str[j + 1];

				len--;
			}
		}
	}

	return len;
}

// ----------------------------------------------------------------------------
// Addresses are single digit, input 1-indexed, output 0-indexed

char GetAddArg(const char *carg)
{
	char 	arg;

	if (*(carg + 1) != '\0')
		return -1;

	arg = *carg - '0' - 1;

	if (arg < 0 || arg > S3_MAX_IPS) return -1;

	return arg;
}

// ----------------------------------------------------------------------------
// Error indicated by SHRT_MIN value

short GetShortArg(const char *carg)
{	
	char	*pEnd;

	// TODO: Check short limits
	short arg = (short)strtol(carg, &pEnd, 10);

	if (*pEnd != '\0')
		return SHRT_MIN;

	return arg;
}

// ----------------------------------------------------------------------------
// Error indicated by DBL_MIN value

double GetDoubleArg(const char *carg)
{	
	char	*pEnd;

	// TODO: Check short limits
	double arg = strtod(carg, &pEnd);

	if (*pEnd != '\0')
		return DBL_MIN;

	return arg;
}

// ----------------------------------------------------------------------------
// Split GPIBCmd into command  and args

unsigned char S3GetArgs()
{
	unsigned char	last = 0, nargs = 0;

	strcpy_s(GPIBCmd, S3_MAX_GPIB_CMD_LEN, GPIBCmdBuf);
	
	// Get number of arguments and replace delimiter with '\0'
	unsigned char i;
	for (i = 0; i < GPIBBufLen + 1 && nargs < S3_MAX_ARGS; i++)
	{
		if (GPIBCmd[i] == ' ' || GPIBCmd[i] == '\0')
		{
			GPIBCmdArgs[nargs] = GPIBCmd + last;
			GPIBCmd[i] = '\0';
			nargs++;
			last = i + 1;
		}
	}

	// Still more to come...
	if (i != GPIBBufLen + 1)
		return 0xFF;

	return nargs;
}

// ----------------------------------------------------------------------------
// S3-specific, non-SCPI commands
int S3ProcessGPIBCommand(const char *cmd)
{
	int		err = 0;

	*GPIBRetBuf = '\0';

	strcpy_s(GPIBCmdBuf, S3_MAX_GPIB_CMD_LEN, cmd);
	GPIBBufLen = strlen(GPIBCmdBuf);

	GPIBBufLen = S3DeSpace(GPIBCmdBuf);

	GPIBNArgs = S3GetArgs();

	if (GPIBNArgs == 0xFF)
	{
		strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Too many parameters");
		return S3_GPIB_TOO_MANY_PARAS;
	}

	if (!STRNCMP(GPIBCmdBuf, "DBG", 3))
	{
		err = DbgProcessCmd();

		if (err == S3_GPIB_INVALID_ADDRESS)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN,
							"S3ProcessGPIBCommand: Invalid address");
		else if (err == S3_GPIB_INVALID_TX)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN,
							"S3ProcessGPIBCommand: Invalid transmitter");
		else if (err == S3_GPIB_CMD_UNRECOGNISED)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN,
							"S3ProcessGPIBCommand: Unrecognized command");
		else if (err)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN,
							"TODO: S3ProcessGPIBCommand: Fix error message");
		else
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "OK:");

		return err;
	}

    if(!STRNCMP(GPIBCmdBuf, "S3", 2))
    {
        err = S3AgentProcessCmd();

        return err;
    }

	// TEST: ONLY - log all remote commands
	S3EventLogAdd(GPIBCmdBuf, 3, -1, -1, -1);

	// In Local mode controller still responds to remote commands, in Remote
	// mode, GUI is read-only.
	if (0) // !S3GetRemote())
	{
		strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "LOCAL MODE: Not responding");
		return S3_GPIB_LOCAL_MODE;
	}

	// TODO: Remove initial filtering, saves bugger-all
	char *Cmd = GPIBCmdArgs[0];
	char Initial = toupper(*Cmd);

	if (Initial == '*')
	{
		if		(!STRCMP(Cmd,	"*IDN?"))		err = CmdIDNQ();
		else
			err = S3_GPIB_CMD_UNRECOGNISED;
	}
	else if (Initial == 'A')
	{
		if		(!STRCMP(Cmd,	"AGC"))			err = CmdAGC();
		else
			err = S3_GPIB_CMD_UNRECOGNISED;
	}
	else if (Initial == 'B')
	{
		err = S3_GPIB_CMD_UNRECOGNISED;
	}
	else if (Initial == 'C')
	{
		err = S3_GPIB_CMD_UNRECOGNISED;
	}
	else if (Initial == 'G')
	{
		if		(!STRCMP(Cmd,	"GAIN"))		err = CmdGAIN();
		else if	(!STRCMP(Cmd,	"GET"))			err = CmdGET(); 
		else
			err = S3_GPIB_CMD_UNRECOGNISED;
	}
	else if (Initial == 'I')
	{
		if		(!STRCMP(Cmd,	"IP"))			err = CmdIP();
		else if (!STRCMP(Cmd,	"IPZ"))			err = CmdIMP();
		else if (!STRCMP(Cmd,	"IPTESTSIG"))	err = CmdIPTESTSIG();
		else if (!STRCMP(Cmd,	"ITAU"))		err = CmdITAU();
		else
			err = S3_GPIB_CMD_UNRECOGNISED;
	}
	else if (Initial == 'L')
	{
		if		(!STRCMP(Cmd,	"LOAD"))		err = CmdLOAD();
		else if (!STRCMP(Cmd,	"LOWNOISE"))	err = CmdLOWNOISE();
		else if (!STRCMP(Cmd,	"LOCAL"))		err = CmdLOCAL();
		else if (!STRCMP(Cmd,	"LOGF"))		err = CmdLOGF();
        else if (!STRCMP(Cmd,	"LOGFCOPY"))    err = CmdLOGFCOPY();
		else
			err = S3_GPIB_CMD_UNRECOGNISED;
	}
	else if (Initial == 'M')
	{
		if		(!STRCMP(Cmd,	"MAXIP"))		err = CmdMAXIP(); 
		else
			err = S3_GPIB_CMD_UNRECOGNISED;
	}
    else if (Initial == 'N')
    {
    	if		(!STRCMP(Cmd,	"NAME"))	err = CmdNAME();
		else
			err = S3_GPIB_CMD_UNRECOGNISED;
    }
	else if (Initial == 'P')
	{
		if		(!STRCMP(Cmd,	"PPMCALRX"))	err = CmdPPMCALRX(); 
		else if (!STRCMP(Cmd,	"PPMCALTXRF"))	err = CmdPPMCALTXRF();
		else if (!STRCMP(Cmd,	"PPMCALTXOPT"))	err = CmdPPMCALTXOPT();
		else if (!STRCMP(Cmd,	"PPMBATTAUTH"))	err = CmdPPMBATTAUTH();
		else if (!STRCMP(Cmd,	"PPMBATTID"))	err = CmdPPMBATTID();
		else if (!STRCMP(Cmd,	"PPMTXID"))		err = CmdPPMTXID();
		else if (!STRCMP(Cmd,	"PPMRXID"))		err = CmdPPMRXID();
		else if (!STRCMP(Cmd,	"PPMSYSID"))	err = CmdPPMSYSID();
		// else if (!STRCMP(Cmd,	"PPMTXTYPE"))	err = CmdPPMTXTYPE(); 
		else
			err = S3_GPIB_CMD_UNRECOGNISED;
	}
	else if (Initial == 'R')
	{
		if		(!STRCMP(Cmd,	"REMOTE"))		err = CmdREMOTE();
		else if (!STRCMP(Cmd,	"RX"))			err = CmdRX();
		else if (!STRCMP(Cmd,	"REPORTF"))		err = CmdREPORTF();
		else if (!STRCMP(Cmd,	"REPORT"))		err = CmdREPORT();
        else if (!STRCMP(Cmd,	"RESTART"))		err = CmdRESTART();
		else
			err = S3_GPIB_CMD_UNRECOGNISED;
	}
	else if (Initial == 'S')
	{
		if		(!STRCMP(Cmd,	"SAVE"))		err = CmdSAVE();
		else if (!STRCMP(Cmd,	"SELECTIP"))	err = CmdSELECTIP();
		else if (!STRCMP(Cmd,	"SET"))			err = CmdSET();
		else if (!STRCMP(Cmd,	"SELECTTX"))	err = CmdSELECTTX();
		else if (!STRCMP(Cmd,	"SYS"))			err = CmdSYS();
		else if (!STRCMP(Cmd,	"SYSRESET"))	err = CmdSYSRESET();
		else if (!STRCMP(Cmd,	"SYSSETTIME"))	err = CmdSYSSETTIME();
        else if (!STRCMP(Cmd,   "SHUTDOWN"))    err = CmdSHUTDOWN();
        else if (!STRCMP(Cmd,   "SLEEPALL"))    err = CmdSLEEPALL();
        else if (!STRCMP(Cmd,   "SWUPDATE"))    err = CmdSWUPDATE();
		else
			err = S3_GPIB_CMD_UNRECOGNISED;
	}
	else if (Initial == 'T')
	{
		if		(!STRCMP(Cmd,	"TXCHARGE"))	err = CmdTXCHARGE();
		else if (!STRCMP(Cmd,	"TXPOWER"))		err = CmdTXSLEEP();
        else if (!STRCMP(Cmd,	"TXSTARTSTATE"))err = CmdTXSTARTSTATE();
		else if (!STRCMP(Cmd,	"TXSELFTEST"))	err = CmdTXSELFTEST();
		else if (!STRCMP(Cmd,	"TX"))			err = CmdTX();
		else if (!STRCMP(Cmd,	"TESTNAME"))	err = CmdTESTNAME();
		else if (!STRCMP(Cmd,	"TCOMPMODE"))	err = CmdTCOMPMODE();
		else if (!STRCMP(Cmd,	"TCOMP"))		err = CmdTCOMP();
		else if	(!STRCMP(Cmd,	"TONE"))		err = CmdCAL();
		else
			err = S3_GPIB_CMD_UNRECOGNISED;
	}
	else if (Initial == 'U')
	{
		if		(!STRCMP(Cmd,	"UNITS"))		err = CmdUNITS();
		else
			err = S3_GPIB_CMD_UNRECOGNISED;
	}
	else if (Initial == 'W')
	{
		if		(!STRCMP(Cmd,	"WINTRACK"))	err = CmdWINTRACK();
        else if (!STRCMP(Cmd,	"WAKEALL"))     err = CmdWAKEALL();
		else
			err = S3_GPIB_CMD_UNRECOGNISED;
	}
	else
	{
		err = S3_GPIB_CMD_UNRECOGNISED;
	}

	// If nothing specific added to return buffer by command, add generic error
	if (strlen(GPIBRetBuf) == 0)
	{
		// TODO: switch(err)...
		if (err == S3_GPIB_CMD_UNRECOGNISED)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Unrecognised command");
		else if (err == S3_GPIB_INVALID_PARAMETER)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Invalid parameter");
		else if (err == S3_GPIB_FILE_SAVE_FAIL)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Failed to save configuration file");
		else if (err == S3_GPIB_IP_SELECT_FAIL)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Failed to select requested input");
		else if (err == S3_GPIB_TOO_FEW_PARAS)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Too few parameters");
		else if (err == S3_GPIB_TOO_MANY_PARAS)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Too many parameters");
		else if (err == S3_GPIB_INVALID_RF_IP)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Invalid RF input number");
		else if (err == S3_GPIB_NO_RX_SEL)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: No receiver selected");
		else if (err == S3_GPIB_NO_TX_SEL)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: No transmitter selected");
		else if (err == S3_GPIB_MALFORMED_ADDRESS)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Malformed address");
		else if (err == S3_GPIB_INVALID_ADDRESS)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Invalid address");
		else if (err == S3_GPIB_OUT_RANGE_ADDRESS)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Address out of range");
		else if (err == S3_GPIB_INVALID_IP)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: RF input address not set");
		else if (err == S3_GPIB_ERR_NUMBER_PARAS)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Incorrect number of parameters");
		else if (err == S3_GPIB_MISSING_PARAMETER)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: No parameters supplied");
		else if (err == S3_GPIB_LOG_INIT_FAILED)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Log file initialisation failed");
		else if (err == S3_GPIB_INVALID_NUMBER)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Invalid numeric parameter");
		else if (err == S3_GPIB_INVALID_MODE)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Invalid transmitter power mode");
		else if (err == S3_GPIB_INVALID_TX)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: No current transmitter set");
		else if (err == S3_GPIB_TX_NOT_EXIST)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Specified transmitter not available");
		else if (err == S3_GPIB_RX_NOT_EXIST)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Specified receiver not available");
		else if (err == S3_GPIB_IP_NOT_EXIST)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Specified input not available");
		else if (err == S3_GPIB_GAIN_LIMITED)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "W: Requested gain constrained by settings");
		else if (err == S3_GPIB_GAIN_CHANGED)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "W: Command required gain setting to be adjusted");
		else if (err == S3_GPIB_TIME_CHANGE_FAILED)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: System time was not set");
		else if (err == S3_GPIB_CALIBRATION_FAILED)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Calibration failed");
		else if (err == S3_GPIB_CH_NOT_EXIST)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: No battery on charger port");
		else if (err == S3_GPIB_CH_VALIDATED)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "W: Battery already validated");
		else if (err == S3_GPIB_CH_AUTH_FAILED)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Battery validation failed");
		else if (err == S3_GPIB_ID_WRITE_FAILED)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Tx ID write failed");
		else if (err == S3_GPIB_INVALID_SNPN)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Invalid SN/PN");
		else if (err == S3_GPIB_INVALID_TYPE)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Invalid type");
		else if (err == S3_GPIB_COMMAND_LOCKED)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Factory-only command");
		else if (err == S3_GPIB_BATTERY_SEALED)
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Battery sealed");	
		else
			strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "OK:");
	}
	else if (err == 2)
	{
		return S3_GPIB_INVALID_SNPN;
	}

	if (err)
		S3EventLogAdd(GPIBRetBuf, 3, -1, -1, -1);

	return err;
}


// ----------------------------------------------------------------------------

char S3GetRemoteSrc()
{
	return S3Data->m_MsgSrc;
}

// ----------------------------------------------------------------------------

void S3SetRemoteSrc(char MsgSrc)
{
	S3Data->m_MsgSrc = MsgSrc;
}

// ----------------------------------------------------------------------------

char *S3GPIBGetRetBuf()
{
	return GPIBRetBuf;
}

// ----------------------------------------------------------------------------

char *S3GPIBGetCmdBuf()
{
	return GPIBCmdBuf;
}

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
			S3IPSetTestToneEnable(GPIBRx, GPIBTx, GPIBIP, 1);
		else if (!STRNCMP(GPIBCmdArgs[1], "OFF", 3))
			S3IPSetTestToneEnable(GPIBRx, GPIBTx, GPIBIP, 0);
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

		val = GetShortArg(GPIBCmdArgs[1]);

		if (val == SHRT_MIN)
			return S3_GPIB_INVALID_PARAMETER;

		if (S3SetGain(GPIBRx, GPIBTx, GPIBIP, (char)val) == 1)
			return S3_GPIB_GAIN_LIMITED;
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

		val = GetShortArg(GPIBCmdArgs[4]);
		
		if (val == SHRT_MIN)
			return S3_GPIB_INVALID_PARAMETER;

		if (S3SetGain(Rx, Tx, IP, (char)val) == 1)
			return S3_GPIB_GAIN_LIMITED;
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
		
		if (S3SetImpedance(GPIBRx, GPIBTx, GPIBIP, IPz))
				return S3_GPIB_GAIN_CHANGED;
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
		
		if (S3SetImpedance(Rx, Tx, IP, IPz))
			return S3_GPIB_GAIN_CHANGED;
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

		if (!STRNCMP(GPIBCmdArgs[1], "ON", 2))
			// S3IPCal(GPIBRx, GPIBTx, GPIBIP, (unsigned char)1);
			S3TxSetTestToneIP(GPIBRx, GPIBTx, GPIBIP);
		else if (!STRNCMP(GPIBCmdArgs[1], "OFF", 3))
			S3TxSetTestToneIP(GPIBRx, GPIBTx, -1);
			// S3IPCal(GPIBRx, GPIBTx, GPIBIP, (unsigned char)0);
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
		//	S3TxSetTestToneIP(Rx, Tx, IP);
		// 	S3IPCal(Rx, Tx, IP, (unsigned char)1);
		else if (!STRCMP(GPIBCmdArgs[4], "OFF"))
			S3IPSetTestToneEnable(Rx, Tx, IP, 0);
		//	S3TxSetTestToneIP(Rx, Tx, -1);
		//	S3IPCal(Rx, Tx, IP, (unsigned char)0);
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
	
	// strcpy_s(S3Data->m_EventLogName, S3_MAX_FILENAME_LEN,
	//	GPIBCmdBuf + os);

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
// Wrapper for commands that only provide an address

int GetAddress2NoArg(char *all, char *Rx, char *Tx, char *IP)
{
	if (GPIBNArgs < S3_MAX_ARGS)
	{
		strcat_s(GPIBCmdBuf, S3_MAX_GPIB_CMD_LEN, " DUMMY");
		GPIBCmdArgs[GPIBNArgs++] = GPIBCmdBuf + strlen(GPIBCmdBuf) - 5;

		return GetAddress2(all, Rx, Tx, IP);
	}

	return -1; // Too many args
}

// ----------------------------------------------------------------------------
// Attempts to extract Rx/Tx/IP address from the args string. Lastarg should
// be a final argument after any address args. A small +ve integer return
// indicates the likely length of the address.

int GetAddress2(char *all, char *Rx, char *Tx, char *IP)
{
	char			*eptr;

	if (all) *all = -1;
	
	*Rx = -1;
	
	if (Tx) *Tx = -1;
	if (IP) *IP = -1;

	// Just the value, no address
	if (GPIBNArgs < 3)
		return 0;

	if (GPIBNArgs == 3)
	{
		if (!STRCMP(GPIBCmdArgs[1], "ALL"))
		{
			*all = 1;

			return 100;
		}
	}

	long	rxl, txl, ipl;

	if (GPIBNArgs >= 5 && IP)
	{
		ipl = strtol(GPIBCmdArgs[3], &eptr, 10);
		txl = strtol(GPIBCmdArgs[2], &eptr, 10);
		rxl = strtol(GPIBCmdArgs[1], &eptr, 10);

		// TODO: Check valid range
		*IP = (char)ipl - 1;
		*Tx = (char)txl - 1;
		*Rx = (char)rxl - 1;

		int err = S3IPInvalidQ(*Rx, *Tx, *IP);

		if (err)
			return err;

		return 3;
	}
	else if ((GPIBNArgs >= 4 || IP == NULL) && Tx)
	{
		txl = strtol(GPIBCmdArgs[2], &eptr, 10);
		rxl = strtol(GPIBCmdArgs[1], &eptr, 10);

		if (!txl || !rxl)
			return -2;

		*Tx = (char)txl - 1;
		*Rx = (char)rxl - 1;
		
		if (!S3RxExistQ(*Rx))
			return S3_GPIB_RX_NOT_EXIST;

		if (!S3TxExistQ(*Rx, *Tx))
			return S3_GPIB_TX_NOT_EXIST;

		return 2;
	}
	else if (GPIBNArgs >= 3 || Tx == NULL)
	{
		if (*GPIBCmdArgs[1] == '0') // Rx0 == Controller
		{
			*Rx = -2;
			return 1;
		}

		rxl = strtol(GPIBCmdArgs[1], &eptr, 10);

		if (rxl < 0)
			return -2;

		*Rx = (char)rxl - 1;

		if (!S3RxExistQ(*Rx))
			return S3_GPIB_RX_NOT_EXIST;

		return 1;
	}

	return 0;
}

// ----------------------------------------------------------------------------
// Deprecate in favour of GetAddress2()

int GetAddress(char *all, char *Rx, char *Tx, char *IP,
			   const char **lastarg, const char *args)
{
	char			*eptr;
	
	const char		*starts[4];
	char			argscpy[S3_MAX_GPIB_CMD_LEN];

	*all = *Rx = *Tx = *IP = -1;
	*lastarg = NULL;

	unsigned		l = strlen(args);

	if (l > S3_MAX_GPIB_CMD_LEN)
		return -1;

	strcpy_s(argscpy, S3_MAX_GPIB_CMD_LEN, args);

	unsigned char last = 0, nargs = 0;
	// Get number of arguments and replace delimiter with '\0'
	for (unsigned char i = 0; i < l + 1 && nargs < 4; i++)
	{
		if (argscpy[i] == ' ' || argscpy[i] == '\0')
		{
			*lastarg = args + last;

			starts[nargs] = argscpy + last;
			argscpy[i] = '\0';
			nargs++;
			last = i + 1;
		}
	}

	// Just the value, no address
	if (nargs < 2)
		return 0;

	// Just plain wrong
	if (nargs > 4)
		return -1;

	if (nargs == 2)
	{
		if (!STRCMP(starts[0], "ALL"))
		{
			*all = 1;

			return 100;
		}
	}

	long			rxl, txl, ipl;

	if (nargs == 4)
	{
		ipl = strtol(starts[2], &eptr, 10);
		txl = strtol(starts[1], &eptr, 10);
		rxl = strtol(starts[0], &eptr, 10);

		// TODO: Check valid range
		*IP = (char)ipl - 1;
		*Tx = (char)txl - 1;
		*Rx = (char)rxl - 1;

		if (!ipl || !txl || !rxl)
			return -2;

		if (!S3TxExistQ(*Rx, *Tx))
			return -3;

		if (!S3IPValidQ(*Rx, *Tx, *IP))
			return -4;

		return 3;
	}
	else if (nargs == 3)
	{
		txl = strtol(starts[1], &eptr, 10);
		rxl = strtol(starts[0], &eptr, 10);

		if (!txl || !rxl)
			return -2;

		*Tx = (char)txl - 1;
		*Rx = (char)rxl - 1;

		if (!S3TxExistQ(*Rx, *Tx))
			return -3;

		return 2;
	}
	else if (nargs == 2)
	{
		if (*starts[0] == '0') // Rx0 == Controller
		{
			*Rx = -2;
			return 1;
		}

		rxl = strtol(starts[0], &eptr, 10);

		if (rxl < 0)
			return -2;

		*Rx = (char)rxl - 1;

		if (!S3RxExistQ(*Rx))
			return -3;

		return 1;
	}

	return 0;
}

// ----------------------------------------------------------------------------

int GetAddressNoArg(char *all, char *Rx, char *Tx, char *IP, const char *args)
{
	char			*eptr;
	
	const char		*starts[4];
	char			argscpy[S3_MAX_GPIB_CMD_LEN];

	*all = *Rx = *Tx = *IP = -1;

	unsigned		l = strlen(args);

	if (l > S3_MAX_GPIB_CMD_LEN)
		return -1;

	strcpy_s(argscpy, S3_MAX_GPIB_CMD_LEN, args);

	unsigned char last = 0, nargs = 0;
	
	if (l)
	{
		// Get number of arguments and replace delimiter with '\0'
		for (unsigned char i = 0; i < l + 1 && nargs < 4; i++)
		{
			if (argscpy[i] == ' ' || argscpy[i] == '\0')
			{
				starts[nargs] = argscpy + last;
				argscpy[i] = '\0';
				nargs++;
				last = i + 1;
			}
		}
	}

	// Just the cmd, no address
	if (nargs < 1)
		return 0;

	// Just plain wrong
	if (nargs > 3)
		return -1;

	if (nargs == 1)
	{
		if (!STRCMP(starts[0], "ALL"))
		{
			*all = 1;

			return 100;
		}
	}

	long			rxl, txl, ipl;

	if (nargs == 3)
	{
		ipl = strtol(starts[2], &eptr, 10);
		txl = strtol(starts[1], &eptr, 10);
		rxl = strtol(starts[0], &eptr, 10);

		// TODO: Check valid range
		*IP = (char)ipl - 1;
		*Tx = (char)txl - 1;
		*Rx = (char)rxl - 1;

		if (!ipl || !txl || !rxl)
			return -2;

		if (!S3TxExistQ(*Rx, *Tx))
			return -3;

		if (!S3IPValidQ(*Rx, *Tx, *IP))
			return -4;

		return 3;
	}
	else if (nargs == 2)
	{
		txl = strtol(starts[1], &eptr, 10);
		rxl = strtol(starts[0], &eptr, 10);

		if (!txl || !rxl)
			return -2;

		*Tx = (char)txl - 1;
		*Rx = (char)rxl - 1;

		if (!S3TxExistQ(*Rx, *Tx))
			return -3;

		return 2;
	}
	else if (nargs == 1)
	{
		if (*starts[0] == '0') // Rx0 == Controller
		{
			*Rx = -2;
			return 1;
		}

		rxl = strtol(starts[0], &eptr, 10);

		if (rxl < 0)
			return -2;

		*Rx = (char)rxl - 1;

		if (!S3RxExistQ(*Rx))
			return -3;

		return 1;
	}

	return 0;
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
int CmdRESTART()
{
    S3OSRestart();

	// TODO: Re-instate MB change
    // AfxGetMainWnd()->PostMessage(REM_SHUTDOWNREQ, 0, 0);
    return 0;
}
int CmdSLEEPALL()
{
    S3SetSleepAll(true);
    return 0;
}

int CmdWAKEALL()
{
    S3SetSleepAll(false);
    return 0;
}


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

SigmaT S3Str2SigmaT(const char *str)
{
	SigmaT	Tau; 
	if (!STRCMP(str, "OFF"))
		Tau = TauNone;
	else if (!STRCMP(str, "LO"))
		Tau = TauLo;
	else if (!STRCMP(str, "MED"))
		Tau = TauMd;
	else if (!STRCMP(str, "HI"))
		Tau = TauHi;
	else 
		Tau = TauUnknown;

	return Tau;
}

// ----------------------------------------------------------------------------

InputZ S3Str2InputZ(const char *str)
{
	if (!STRCMP(str, "50"))	
	{
		return W50;
	}
	else if (!STRCMP(str, "HIZ"))
	{
		return W1M;
	}
	else
		return ZUnknown;
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