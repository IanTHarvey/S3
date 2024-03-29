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
#include "stdafx.h"
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
// In-place de-space

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

	while(str[i] == ' ' || str[i] == '\r' || str[i] == '\n')
	{
		str[i] = '\0';

		i--;
		len--;
	}

	// Multiple repeated spaces
	for(i = 0; i < len; i++)
	{
		if (str[i] == ' ')
		{
			while (str[i + 1] == ' ')
			{
				for(unsigned j = i + 1; j < len + 1; j++)
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
		S3LookUpError(GPIBRetBuf, err);
		return err;
	}

    if(!STRNCMP(GPIBCmdBuf, "S3", 2))
    {
        err = S3AgentProcessCmd();
        return err;
    }

	// Log only user commands
	S3EventLogAdd(GPIBCmdBuf, 3, -1, -1, -1);

	char *Cmd = GPIBCmdArgs[0];
	char Initial = toupper(*Cmd);

	switch(Initial)
	{
	case '*':
		if		(!STRCMP(Cmd,	"*IDN?"))			err = CmdIDNQ();
		else	err = S3_GPIB_CMD_UNRECOGNISED;
		break;
	case 'A':
		if		(!STRCMP(Cmd,	"AGC"))				err = CmdAGC();
		else	err = S3_GPIB_CMD_UNRECOGNISED;
		break;
	case 'G':
		if		(!STRCMP(Cmd,	"GAIN"))			err = CmdGAIN();
		else if	(!STRCMP(Cmd,	"GET"))				err = CmdGET();
		else if	(!STRCMP(Cmd,	"GETRLL"))			err = CmdGETRLL();
		else if	(!STRCMP(Cmd,	"GETTXSN"))			err = CmdGETTXSN(); 
		else if	(!STRCMP(Cmd,	"GETTXSTABLE"))		err = CmdGETTXSTABLE();
		else if	(!STRCMP(Cmd,	"GETTXBATTLIFE"))	err = CmdGETTXBATTLIFE();
		else if	(!STRCMP(Cmd,	"GETTXPOWER"))		err = CmdGETTXPOWER();
		else if	(!STRCMP(Cmd,	"GETTXSETTINGS"))	err = CmdGETTXSETTINGS();
		else if	(!STRCMP(Cmd,	"GETTXCHARGE"))		err = CmdTXCHARGE();
		else	err = S3_GPIB_CMD_UNRECOGNISED;
		break;
	case 'I':
		if		(!STRCMP(Cmd,	"IP"))				err = CmdIP();
		else if (!STRCMP(Cmd,	"IPZ"))				err = CmdIMP();
		else if (!STRCMP(Cmd,	"IPTESTSIG"))		err = CmdIPTESTSIG();
		else if (!STRCMP(Cmd,	"ITAU"))			err = CmdITAU();
		else	err = S3_GPIB_CMD_UNRECOGNISED;
		break;
	case 'L':
		if		(!STRCMP(Cmd,	"LOAD"))			err = CmdLOAD();
		else if (!STRCMP(Cmd,	"LOWNOISE"))		err = CmdLOWNOISE();
		else if (!STRCMP(Cmd,	"LOCAL"))			err = CmdLOCAL();
		else if (!STRCMP(Cmd,	"LOGF"))			err = CmdLOGF();
        else if (!STRCMP(Cmd,	"LOGFCOPY"))		err = CmdLOGFCOPY();
		else	err = S3_GPIB_CMD_UNRECOGNISED;
		break;
	case 'M':
		if		(!STRCMP(Cmd,	"MAXIP"))			err = CmdMAXIP(); 
		else	err = S3_GPIB_CMD_UNRECOGNISED;
		break;
	case 'N':
    	if		(!STRCMP(Cmd,	"NAME"))			err = CmdNAME();
		else	err = S3_GPIB_CMD_UNRECOGNISED;
		break;
	case 'P':
		if		(!STRCMP(Cmd,	"PPMCALRX"))		err = CmdPPMCALRX();
		else if	(!STRCMP(Cmd,	"PPMGETCALRX"))		err = CmdPPMGETCALRX(); 
		else if (!STRCMP(Cmd,	"PPMCALTXRF"))		err = CmdPPMCALTXRF();
		else if (!STRCMP(Cmd,	"PPMCALTXOPT"))		err = CmdPPMCALTXOPT();
		else if (!STRCMP(Cmd,	"PPMBATTAUTH"))		err = CmdPPMBATTAUTH();
		else if (!STRCMP(Cmd,	"PPMBATTID"))		err = CmdPPMBATTID();
		else if (!STRCMP(Cmd,	"PPMTXID"))			err = CmdPPMTXID();
		else if (!STRCMP(Cmd,	"PPMRXID"))			err = CmdPPMRXID();
		else if (!STRCMP(Cmd,	"PPMRXOPTFW"))		err = CmdPPMRXOPTFW();
		else if (!STRCMP(Cmd,	"PPMSYSID"))		err = CmdPPMSYSID();
		else if	(!STRCMP(Cmd,	"PPMTERMINATOR"))	err = CmdPPMTERMINATOR();
		else	err = S3_GPIB_CMD_UNRECOGNISED;
		break;
	case 'R':
		if		(!STRCMP(Cmd,	"REMOTE"))			err = CmdREMOTE();
		else if (!STRCMP(Cmd,	"RX"))				err = CmdRX();
		else if (!STRCMP(Cmd,	"REPORTF"))			err = CmdREPORTF();
		else if (!STRCMP(Cmd,	"REPORT"))			err = CmdREPORT();
        else if (!STRCMP(Cmd,	"RESTART"))			err = CmdRESTART();
		else	err = S3_GPIB_CMD_UNRECOGNISED;
		break;
	case 'S':
		if		(!STRCMP(Cmd,	"SAVE"))			err = CmdSAVE();
		else if (!STRCMP(Cmd,	"SELECTIP"))		err = CmdSELECTIP();
		else if (!STRCMP(Cmd,	"SET"))				err = CmdSET();
		else if (!STRCMP(Cmd,	"SELECTTX"))		err = CmdSELECTTX();
		else if (!STRCMP(Cmd,	"SYSRESET"))		err = CmdSYSRESET();
		else if (!STRCMP(Cmd,	"SYSSETTIME"))		err = CmdSYSSETTIME();
        else if (!STRCMP(Cmd,   "SHUTDOWN"))		err = CmdSHUTDOWN();
        else if (!STRCMP(Cmd,   "SLEEPALL"))		err = CmdSLEEPALL();
        else if (!STRCMP(Cmd,   "SWUPDATE"))		err = CmdSWUPDATE();
		else if (!STRCMP(Cmd,   "SCALE"))			err = CmdSCALE();
		else if (!STRCMP(Cmd,   "SIGTYPE"))			err = CmdSIGTYPE();
		else if (!STRCMP(Cmd,   "SHOW3PC"))			err = CmdSHOW3PC();
		else	err = S3_GPIB_CMD_UNRECOGNISED;
		break;
	case 'T':
		if		(!STRCMP(Cmd,	"TXCHARGE"))		err = CmdTXCHARGE();
		else if (!STRCMP(Cmd,	"TXPOWER"))			err = CmdTXSLEEP();
        else if (!STRCMP(Cmd,	"TXSTARTSTATE"))	err = CmdTXSTARTSTATE();
		else if (!STRCMP(Cmd,	"TXSELFTEST"))		err = CmdTXSELFTEST();
		else if (!STRCMP(Cmd,	"TX"))				err = CmdTX();
		else if (!STRCMP(Cmd,	"TESTNAME"))		err = CmdTESTNAME();
		else if (!STRCMP(Cmd,	"TCOMPMODE"))		err = CmdTCOMPMODE();
		else if (!STRCMP(Cmd,	"TCOMP"))			err = CmdTCOMP();
		else if	(!STRCMP(Cmd,	"TONE"))			err = CmdCAL();
		else	err = S3_GPIB_CMD_UNRECOGNISED;
		break;
	case 'U':
		if		(!STRCMP(Cmd,	"UNITS"))			err = CmdUNITS();
		else	err = S3_GPIB_CMD_UNRECOGNISED;
		break;
	case 'V':
		if		(!STRCMP(Cmd,	"VERSIONSW"))		err = CmdVERSIONSW();
		else	err = S3_GPIB_CMD_UNRECOGNISED;
		break;
	case 'W':
		if		(!STRCMP(Cmd,	"WINTRACK"))		err = CmdWINTRACK();
        else if (!STRCMP(Cmd,	"WAKEALL"))			err = CmdWAKEALL();
		else	err = S3_GPIB_CMD_UNRECOGNISED;
		break;
	default:
		err = S3_GPIB_CMD_UNRECOGNISED;
		break;
	}

	// If nothing command-specific added to return buffer, add generic error
	if (strlen(GPIBRetBuf) == 0)
		S3LookUpError(GPIBRetBuf, err);
	else if (err == 2)
		return S3_GPIB_INVALID_SNPN;

	if (err)
		S3EventLogAdd(GPIBRetBuf, 3, -1, -1, -1);

	return err;
}

// ----------------------------------------------------------------------------

int S3LookUpError(char *buf, int err)
{
	if (err == 0)
	{
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "OK:");
		return 0;
	}

	switch(err)
	{
	case S3_GPIB_CMD_UNRECOGNISED:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: Unrecognised command");							break;
	case S3_GPIB_INVALID_PARAMETER:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: Invalid parameter");								break;
	case S3_GPIB_FILE_SAVE_FAIL:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: Failed to save configuration file");				break;
	case S3_GPIB_IP_SELECT_FAIL:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: Failed to select requested input");				break;
	case S3_GPIB_TOO_FEW_PARAS:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: Too few parameters");							break;
	case S3_GPIB_TOO_MANY_PARAS:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: Too many parameters");							break;
	case S3_GPIB_INVALID_RF_IP:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: Invalid RF input number");						break;
	case S3_GPIB_NO_RX_SEL:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: No receiver selected");							break;
	case S3_GPIB_NO_TX_SEL:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: No transmitter selected");						break;
	case S3_GPIB_MALFORMED_ADDRESS:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: Malformed address");								break;
	case S3_GPIB_INVALID_ADDRESS:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: Invalid address");								break;
	case S3_GPIB_OUT_RANGE_ADDRESS:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: Address out of range");							break;
	case S3_GPIB_INVALID_IP:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: RF input address not set");						break;
	case S3_GPIB_ERR_NUMBER_PARAS:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: Incorrect number of parameters");				break;
	case S3_GPIB_MISSING_PARAMETER:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: No parameters supplied");						break;
	case S3_GPIB_LOG_INIT_FAILED:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: Log file initialisation failed");				break;
	case S3_GPIB_INVALID_NUMBER:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: Invalid numeric parameter");						break;
	case S3_GPIB_INVALID_MODE:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: Invalid transmitter power mode");				break;
	case S3_GPIB_INVALID_TX:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: No current transmitter set");					break;
	case S3_GPIB_INVALID_RX:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: No current receiver set");						break;
	case S3_GPIB_TX_NOT_EXIST:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: Specified transmitter not available");			break;
	case S3_GPIB_RX_NOT_EXIST:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: Specified receiver not available");				break;
	case S3_GPIB_IP_NOT_EXIST:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: Specified input not available");					break;
	case S3_GPIB_GAIN_LIMITED:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "W: Requested gain constrained by settings");		break;
	case S3_GPIB_TX_PROT_MODE:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "W: Tx in protection mode. Gain not changed");		break;
	case S3_GPIB_GAIN_CHANGED:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "W: Command required gain setting to be adjusted");	break;
	case S3_GPIB_TIME_CHANGE_FAILED:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: System time was not set");						break;
	case S3_GPIB_CALIBRATION_FAILED:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: Calibration failed");							break;
	case S3_GPIB_CH_NOT_EXIST:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: No battery on charger port");					break;
	case S3_GPIB_CH_VALIDATED:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "W: Battery already validated");						break;
	case S3_GPIB_CH_AUTH_FAILED:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: Battery validation failed");						break;
	case S3_GPIB_ID_WRITE_FAILED:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: Device ID write failed");						break;
	case S3_GPIB_INVALID_SNPN:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: Invalid SN/PN");									break;
	case S3_GPIB_INVALID_TYPE:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: Invalid type");									break;
	case S3_GPIB_COMMAND_LOCKED:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: Factory-only command");							break;
	case S3_GPIB_BATTERY_SEALED:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: Battery sealed");								break;
	case S3_GPIB_MAX_IP_IGNORED:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "W: Maximum input parameter deprecated");			break;
	case S3_GPIB_COMMAND_FAILED:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: Command failed");								break;
	default:
		strcpy_s(buf, S3_MAX_GPIB_RET_LEN, "E: Unknown error");
		return 1; // May want to handle
	}

	return 0;
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
// Wrapper for commands that only provide an address

int GetAddress2NoArg(char *all, char *Rx, char *Tx, char *IP, bool mustExist)
{
	if (GPIBNArgs < S3_MAX_ARGS)
	{
		strcat_s(GPIBCmdBuf, S3_MAX_GPIB_CMD_LEN, " DUMMY");
		GPIBCmdArgs[GPIBNArgs++] = GPIBCmdBuf + strlen(GPIBCmdBuf) - 5;

		return GetAddress2(all, Rx, Tx, IP, mustExist);
	}

	return -1; // Too many args
}

// ----------------------------------------------------------------------------
// Attempts to extract Rx/Tx/IP address from the args string. Lastarg should
// be a final argument after any address args. A small +ve integer return
// indicates the likely length of the address.

int GetAddress2(char *all, char *Rx, char *Tx, char *IP, bool mustExist)
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
		if (eptr == GPIBCmdArgs[3])
			return S3_GPIB_MALFORMED_ADDRESS;

		txl = strtol(GPIBCmdArgs[2], &eptr, 10);
		if (eptr == GPIBCmdArgs[2])
			return S3_GPIB_MALFORMED_ADDRESS;
		
		rxl = strtol(GPIBCmdArgs[1], &eptr, 10);
		if (eptr == GPIBCmdArgs[1])
			return S3_GPIB_MALFORMED_ADDRESS;

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
		
		if (mustExist && !S3RxExistQ(*Rx))
			return S3_GPIB_RX_NOT_EXIST;

		if (mustExist && !S3TxExistQ(*Rx, *Tx))
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

		if (mustExist && !S3RxExistQ(*Rx))
			return S3_GPIB_RX_NOT_EXIST;

		return 1;
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
	else if (!STRCMP(str, "-"))
		Tau = TauUnknown;
	else
		Tau = TauError;

	return Tau;
}

// ----------------------------------------------------------------------------

InputZ S3Str2InputZ(const char *str)
{
	InputZ Z;

	if (!STRCMP(str, "50"))	
		Z = W50;
	else if (!STRCMP(str, "HIZ") || !STRCMP(str, "1M"))
		Z = W1M;
	else if (!STRCMP(str, "-"))
		Z = ZUnknown;
	else
		Z = ZError;

	return Z;
}

// ----------------------------------------------------------------------------
