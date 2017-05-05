// ----------------------------------------------------------------------------
// GPIB Command Parser/interpreter

#pragma once

#define S3_MAX_GPIB_CMD_LEN	256
#define S3_MAX_GPIB_RET_LEN	32768

#define S3_MAX_ARGS 12

// TODO: Worry about locale-specific
#define STRNCMP	_strnicmp
#define STRCMP	_stricmp

#define WSA_ERR_STR_LEN	256
extern char	WSAErrString[];
const char *GetWSAErrString();
const char *GetErrString();


// Copy cmd to GPIBCmdBuf and process
int S3ProcessGPIBCommand(const char *cmd);

/*
The follwoing functions reference these defined in S3GPIB.cpp:

#define S3_MAX_GPIB_CMD_LEN	256
#define S3_MAX_GPIB_RET_LEN	1024	// TODO: What's sensible?

char		GPIBCmdBuf[S3_MAX_GPIB_CMD_LEN];
char		GPIBRetBuf[S3_MAX_GPIB_RET_LEN];
unsigned	GPIBBufLen;
char		GPIBCurrentRx = -1;
*/

// os = character offset to start of arguments in GPIBCmdBuf
// Non-zero return indicates that message is an error

char *S3GPIBGetRetBuf();
char *S3GPIBGetCmdBuf();

// USB or Ethernet remote commands
char S3GetRemoteSrc();
void S3SetRemoteSrc(char MsgSrc);

int GetAddress(char *all, char *Rx, char *Tx, char *IP,
					const char **lastarg,
					const char *args);

int GetAddress2(char *all, char *Rx, char *Tx, char *IP);

int GetAddressNoArg(char *all, char *Rx, char *Tx, char *IP,
					const char *args);

int GetAddress2NoArg(char *all, char *Rx, char *Tx, char *IP);

// Good
int CmdMAXIP();
int CmdGAIN();
int CmdGET();
int CmdITAU();
int CmdLOCAL();
int CmdREMOTE();
int CmdLOWNOISE();
int CmdTCOMP();
int CmdTCOMPMODE();
int CmdLOGF();
int CmdIDNQ();
int CmdLOAD();
int CmdSAVE();
int CmdREPORTF();
int CmdTESTNAME();
int CmdUNITS();
int CmdREPORT();
int CmdSYS();
int CmdIMP();
int CmdSELECTIP();
int CmdSELECTTX();
int CmdPPMCALRX();
int CmdPPMCALTXRF();
int CmdPPMCALTXOPT();
int CmdPPMBATTID();
int CmdPPMBATTAUTH();
int CmdPPMTXID();
int CmdPPMRXID();
int CmdPPMSYSID();
int CmdSYSRESET();
int CmdSYSSETTIME();
int CmdTXSLEEP();
int CmdTXSTARTSTATE();
int CmdTXSELFTEST();
int CmdTXCHARGE();
int CmdCAL();
int CmdAGC();
int CmdWINTRACK();
int CmdIPTESTSIG();
int CmdSTAT();
int CmdSETQ();
int CmdSET();
int CmdGPIB();

int CmdRX();
int CmdTX();
int CmdIP();

int CmdSHUTDOWN();
int CmdSLEEPALL();
int CmdWAKEALL();
int CmdRESTART();

int CmdNAME();
int CmdLOGFCOPY();
int CmdSWUPDATE();
// Need sorting (get rid of os, use GetAddress2() where applicable)
// int CmdINP(		int os);



// int CmdTXACTIVE(int os);



// ----------------------------------------------------------------------------
// Methods to insert test data - maybe useful for demo mode

int DbgRxInsert();
int DbgRxRemove();

int DbgTxInsert();
int DbgTxRemove();

int DbgTxBrownOut();
int DbgRxRLL(int os);
int DbgIPOverDrive();

int DbgSysPN(int os);
int DbgSysSN(int os);
int DbgRxPN(int os);
int DbgRxSN(int os);
int DbgTxPN(int os);
int DbgTxSN(int os);
int DbgTxBattery(int os);
int DbgCancelAlarms(int os);

// ----------------------------------------------------------------------------
// Draft S3Agent Specific commands
int S3AgentProcessCmd(void);
int CmdGetBatt(char *Inbuf);
int CmdGetInit(char *Inbuf);
int CmdSetInit(char *Inbuf);
int CmdGetConn(char *Inbuf);
int CmdSetConn(char *Inbuf);
int CmdGetSysI(char *Inbuf);
int CmdGetFile(char *Inbuf);
int CmdGetSlot(char *Inbuf);
int CmdGetRXMod(char *Inbuf, int Rx);
int CmdGetTXMod(char *Inbuf, int Rx, int Tx);
int CmdGetInput(char *Inbuf, int Rx, int Tx, int IP);

// ----------------------------------------------------------------------------

#define S3_GPIB_CMD_UNRECOGNISED	2000
#define S3_GPIB_INVALID_PARAMETER	2100
#define S3_GPIB_INVALID_NUMBER		2101
#define S3_GPIB_NO_RX_SEL			2200
#define S3_GPIB_NO_TX_SEL			2201
#define S3_GPIB_NO_IP_SEL			2202
#define S3_GPIB_INVALID_IP			2203
#define S3_GPIB_RX_DISABLED			2204
#define S3_GPIB_INVALID_ADDRESS		2205
#define S3_GPIB_MALFORMED_ADDRESS	2206
#define S3_GPIB_OUT_RANGE_ADDRESS	2207
#define S3_GPIB_FILE_SAVE_FAIL		2208
#define S3_GPIB_INVALID_TX			2209
#define S3_GPIB_INVALID_ALL			2210
#define S3_GPIB_IP_SELECT_FAIL		2211
#define S3_GPIB_INVALID_MODE		2212
#define S3_GPIB_INVALID_RX			2213
#define S3_GPIB_BATT_INVALID		2214
#define S3_GPIB_TOO_FEW_PARAS		2215
#define S3_GPIB_TOO_MANY_PARAS		2216
#define S3_GPIB_INVALID_RF_IP		2217
#define S3_GPIB_ERR_NUMBER_PARAS	2218
#define S3_GPIB_MISSING_PARAMETER	2219
#define S3_GPIB_LOG_INIT_FAILED		2220
#define S3_GPIB_CALIBRATION_FAILED	2221
#define S3_GPIB_TX_NOT_EXIST		2222
#define S3_GPIB_RX_NOT_EXIST		2223
#define S3_GPIB_IP_NOT_EXIST		2224
#define S3_GPIB_ID_WRITE_FAILED		2226
#define S3_GPIB_INVALID_SNPN		2227
#define S3_GPIB_INVALID_TYPE		2228
#define S3_GPIB_BATTERY_SEALED		2229

#define S3_GPIB_CH_VALIDATED		2250
#define S3_GPIB_CH_NOT_EXIST		2251
#define S3_GPIB_CH_AUTH_FAILED		2252

#define S3_GPIB_COMMAND_LOCKED		2240

#define S3_GPIB_GAIN_LIMITED		2400
#define S3_GPIB_GAIN_CHANGED		2401

#define S3_GPIB_TIME_CHANGE_FAILED	2501

#define S3_NOT_CONNECTED			2300
#define S3_GPIB_LOCAL_MODE			2301

#define S3_MAX_ID_LEN				64

// #define CmdRX_OUT_OF_RANGE			10000

// ----------------------------------------------------------------------------
// Event simulator structs


// Transmitter head and battery
struct DbgPollTxStruct {
	char	type;
	char	SN[S3_MAX_SN_LEN];
	char	OverDrive[S3_MAX_IPS];
	short	IPPower[S3_MAX_IPS];
	unsigned char	SoC;		// 0 bad - >100 fully charge
	char	Temp;
	short	BattTemp;
	char	BrownOut;		// 5, 4, 3, 2, 1... count down
	char	BattSN[S3_MAX_SN_LEN];
	char	BattPN[S3_MAX_PN_LEN];
	char	BattHW[S3_MAX_SW_VER_LEN];
	char	BattFW[S3_MAX_SW_VER_LEN];

	// No, not an external event
	// char	PowerMode;		// On, sleep
};

struct DbgPollRxStruct {
	char	OccupierType;

	short	RLL[S3_MAX_TXS];		// 10mdBm - 0: disconnected
	short	RFGain[S3_MAX_TXS];		// 10mdBm - 0: disconnected
	short	RFLevel[S3_MAX_TXS];	// 10mdBm - 0: disconnected
	char	LinkGain[S3_MAX_TXS];	// 0: disconnected
	char	Temp;					// 0: disconnected

	char	SN[S3_MAX_SN_LEN];
	char	PN[S3_MAX_PN_LEN];
	char	HW[S3_MAX_SW_VER_LEN];
	char	FW[S3_MAX_SW_VER_LEN];
	char	FWDate[S3_MAX_SW_VER_LEN];

	struct DbgPollTxStruct Txs[S3_MAX_TXS];
};

// Charger & batteries
struct DbgPollChStruct {
	bool	Occupied;
	char	SoC;
	short	BattTemp;
	char	SN[S3_MAX_SN_LEN];
	char	PN[S3_MAX_PN_LEN];
	char	HW[S3_MAX_SW_VER_LEN];
	char	FW[S3_MAX_SW_VER_LEN];
};

struct DbgPollSysStruct {
	DbgPollRxStruct	Rx[S3_MAX_RXS];
	DbgPollChStruct	Ch[S3_N_CHARGERS];
};


// ----------------------------------------------------------------------------