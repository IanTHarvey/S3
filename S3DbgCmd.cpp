// ----------------------------------------------------------------------------
//
// All these commands are to simulate the data available via the various I2C
// paths. S3 functions will only poll the data, not using it's own functions
// to access anything directly (unless stated otherwise).
// 
// ----------------------------------------------------------------------------

#include "windows.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "S3DataModel.h"
#include "S3GPIB.h"
// #include "S3I2C.h"

struct DbgPollSysStruct	DbgPollSysData;

extern char		GPIBCmdBuf[];
extern char		GPIBRetBuf[];
extern unsigned	GPIBBufLen;
extern char		GPIBCurrentRx;
extern char		GPIBCurrentTx; // For settings etc
extern char		GPIBCurrentIP;

int DbgRxInsert(int os);
int DbgRxRemove(int os);

int DbgTxInsert(int os);
int DbgTxRemove(int os);
int DbgTxSelect(int os);
int DbgTxBrownOut(int os);

int DbgTxFW(int os);
int DbgTxHW(int os);
int DbgRxFW(int os);
int DbgRxHW(int os);

int DbgRxRLL(int os);

int DbgIPOverdrive(int os);
int DbgCancelAlarms(int os);

int	DbgRxLinkGain(int os);

// Charger battery
int DbgChBattInsert(int os);
int DbgChBattRemove(int os);

int DbgChBattHWV(int os);
int DbgChBattFWV(int os);
int DbgChBattSN(int os);
int DbgChBattPN(int os);

int DbgChSoC(int os);
int DbgChBattTemp(int os);

// Tx battery
int DbgTxBattAll(int os);
int DbgTxBattHWV(int os);
int DbgTxBattFWV(int os);
int DbgTxBattSN(int os);
int DbgTxBattPN(int os);

int DbgTxSoC(int os);
int DbgTxBattTemp(int os);

// ----------------------------------------------------------------------------
// Event simulators

int DbgProcessCmd()
{
	int err;

	if (!STRNCMP(GPIBCmdBuf, "DBGRXINSERT ", 12))
	{
		err = DbgRxInsert(12);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGRXREMOVE ", 12))
	{
		err = DbgRxRemove(12);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGTXINSERT ", 12))
	{
		err = DbgTxInsert(12);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGTXREMOVE ", 12))
	{
		err = DbgTxRemove(12);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGTXSELECT ", 12))
	{
		err = DbgTxSelect(12);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGTXBROWNOUT ", 14))
	{
		err = DbgTxBrownOut(14);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGIPOVERDRIVE ", 15))
	{
		err = DbgIPOverdrive(15);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGRXRLL ", 9))
	{
		err = DbgRxRLL(9);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGSYSPARTNO ", 13))
	{
		err = DbgSysPN(13);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGSYSSERIALNO ", 14))
	{
		err = DbgSysSN(14);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGRXPARTNO ", 12))
	{
		err = DbgRxPN(12);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGRXSERIALNO ", 14))
	{
		err = DbgRxSN(14);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGRXLINKGAIN ", 14))
	{
		err = DbgRxLinkGain(14);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGTXPARTNO ", 12))
	{
		err = DbgTxPN(12);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGTXSERIALNO ", 14))
	{
		err = DbgTxSN(14);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGTXBATTLEVEL ", 15))
	{
		err = DbgTxSoC(15);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGTXBATTHW ", 12))
	{
		err = DbgTxBattHWV(12);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGTXBATTFW ", 12))
	{
		err = DbgTxBattFWV(12);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGTXBATTSN ", 12))
	{
		err = DbgTxBattSN(12);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGTXBATTPN ", 12))
	{
		err = DbgTxBattPN(12);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGTXBATTTEMP ", 14))
	{
		err = DbgTxBattTemp(14);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGTXBATTALL ", 13))
	{
		err = DbgTxBattAll(13);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGCANCELALL", 12))
	{
		err = DbgCancelAlarms(12);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGCHBATTLEVEL ", 15))
	{
		err = DbgChSoC(15);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGCHBATTINSERT ", 16))
	{
		err = DbgChBattInsert(16);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGCHBATTREMOVE ", 16))
	{
		err = DbgChBattRemove(16);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGCHBATTHW ", 12))
	{
		err = DbgChBattHWV(12);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGCHBATTFW ", 12))
	{
		err = DbgChBattFWV(12);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGCHBATTPN ", 12))
	{
		err = DbgChBattPN(12);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGCHBATTSN ", 12))
	{
		err = DbgChBattSN(12);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGCHBATTTEMP ", 14))
	{
		err = DbgChBattTemp(14);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGTXHW ", 8))
	{
		err = DbgTxHW(8);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGTXFW ", 8))
	{
		err = DbgTxFW(8);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGRXHW ", 8))
	{
		err = DbgRxHW(8);
	}
	else if (!STRNCMP(GPIBCmdBuf, "DBGRXFW ", 8))
	{
		err = DbgRxFW(8);
	}
	else	err = S3_GPIB_CMD_UNRECOGNISED;

	return err;
}

// ----------------------------------------------------------------------------
// Rxs not hot-swappable but may happen anyway.

int DbgRxInsert(int os)
{
	char	*eptr;
	char	Rx = (char)strtol(GPIBCmdBuf + os, &eptr, 10) - 1;

	if (!S3RxValidQ(Rx))
		return S3_GPIB_INVALID_ADDRESS;

	char type = (char)strtol(eptr, &eptr, 10);

	if (type != S3_Rx1	&& type != S3_Rx2 && type != S3_Rx6)
		return 2;

	// WiP
	DbgPollSysData.Rx[Rx].OccupierType = type;

	return 0;
}

// ----------------------------------------------------------------------------
// Ditto.

int DbgRxRemove(int os)
{
	char	*eptr;
	char	Rx = (char)strtol(GPIBCmdBuf + os, &eptr, 10) - 1;

	if (!S3RxValidQ(Rx))
		return S3_GPIB_INVALID_ADDRESS;

	if (!S3RxExistQ(Rx))
		return S3_GPIB_INVALID_RX;

	DbgPollSysData.Rx[Rx].OccupierType = S3_RxEmpty;

	return 0;
}

// ----------------------------------------------------------------------------
// FOL connection

int DbgTxInsert(int os)
{
	char	*eptr;
	char	Rx = (char)strtol(GPIBCmdBuf + os, &eptr, 10) - 1;
	char	Tx = (char)strtol(eptr, &eptr, 10) - 1;

	if (!S3TxValidQ(Rx, Tx))
		return S3_GPIB_INVALID_ADDRESS;

	if (S3TxExistQ(Rx, Tx))
		return S3_GPIB_INVALID_TX;

	char type = (char)strtol(eptr, &eptr, 10);

	if (type != S3_Tx1	&& type != S3_Tx8)
		return 2;

	DbgPollSysData.Rx[Rx].Txs[Tx].type = type;
	DbgPollSysData.Rx[Rx].RLL[Tx] = 0;

	return 0;
}

// ----------------------------------------------------------------------------
// FOL disconnection

int DbgTxRemove(int os)
{
	char	*eptr;
	char	Rx = (char)strtol(GPIBCmdBuf + os, &eptr, 10) - 1;
	char	Tx = (char)strtol(eptr, &eptr, 10) - 1;

	if (!S3TxValidQ(Rx, Tx))
		return S3_GPIB_INVALID_ADDRESS;

	if (!S3TxExistQ(Rx, Tx))
		return S3_GPIB_INVALID_TX;

	DbgPollSysData.Rx[Rx].Txs[Tx].type = S3_TxUnconnected;
	DbgPollSysData.Rx[Rx].RLL[Tx] = 0;

	return 0;
}

// ----------------------------------------------------------------------------

int DbgTxSelect(int os)
{
	char	*eptr;
	char	Rx = (char)strtol(GPIBCmdBuf + os, &eptr, 10) - 1;
	char	Tx = (char)strtol(eptr, &eptr, 10) - 1;

	if (!S3TxValidQ(Rx, Tx))
		return S3_GPIB_INVALID_ADDRESS;

	if (!S3TxExistQ(Rx, Tx))
		return S3_GPIB_INVALID_TX;

	S3RxSetHighlightedTx(Rx, Tx);

	return 0;
}

// ----------------------------------------------------------------------------

int DbgTxBrownOut(int os)
{
	char	*eptr;
	char	Rx = (char)strtol(GPIBCmdBuf + os, &eptr, 10) - 1;
	char	Tx = (char)strtol(eptr, &eptr, 10) - 1;

	if (!S3TxValidQ(Rx, Tx))
		return S3_GPIB_INVALID_ADDRESS;

	if (!S3TxExistQ(Rx, Tx))
		return S3_GPIB_INVALID_TX;

	char val = (char)strtol(eptr, &eptr, 10);

	DbgPollSysData.Rx[Rx].Txs[Tx].BrownOut = val;

	return 0;
}

// ----------------------------------------------------------------------------

int DbgRxRLL(int os)
{
	char	*eptr;
	char	Rx = (char)strtol(GPIBCmdBuf + os, &eptr, 10) - 1;
	char	Tx = (char)strtol(eptr, &eptr, 10) - 1;

	if (!S3TxValidQ(Rx, Tx))
		return S3_GPIB_INVALID_ADDRESS;

	if (!S3TxExistQ(Rx, Tx))
		return S3_GPIB_INVALID_TX;

	short val = (short)strtol(eptr, &eptr, 10);

	DbgPollSysData.Rx[Rx].RLL[Tx] = val;

	return 0;
}

// ----------------------------------------------------------------------------

int DbgRxLinkGain(int os)
{
	char	*eptr;
	char	Rx = (char)strtol(GPIBCmdBuf + os, &eptr, 10) - 1;
	char	Tx = (char)strtol(eptr, &eptr, 10) - 1;

	if (!S3TxValidQ(Rx, Tx))
		return S3_GPIB_INVALID_ADDRESS;

	if (!S3TxExistQ(Rx, Tx))
		return S3_GPIB_INVALID_TX;

	char val = (char)strtol(eptr, &eptr, 10);

	DbgPollSysData.Rx[Rx].LinkGain[Tx] = val;

	return 0;
}

// ----------------------------------------------------------------------------

int DbgIPOverdrive(int os)
{

	char		all, Rx, Tx, IP;
	const char	*lastarg;
	int res = GetAddress(&all, &Rx, &Tx, &IP, &lastarg, GPIBCmdBuf + os);
	// int res = GetAddress2(&all, &Rx, &Tx, &IP);

	if (res != 3)
		return S3_GPIB_INVALID_ADDRESS;

	char val = (char)strtol(lastarg, NULL, 10);

	if (val)
		S3IPSetAlarm(Rx, Tx, IP, S3_IP_OVERDRIVE);
	else
		S3IPCancelAlarm(Rx, Tx, IP, S3_IP_OVERDRIVE);

	return 0;
}

// ----------------------------------------------------------------------------

int DbgSysPN(int os)
{
	return S3SysSetPN(GPIBCmdBuf + os);
}

// ----------------------------------------------------------------------------

int DbgSysSN(int os)
{
	return S3SysSetSN(GPIBCmdBuf + os);
}

// ----------------------------------------------------------------------------

int DbgRxPN(int os)
{
	char		all, Rx, Tx, IP;
	const char	*lastarg;

	int res = GetAddress(&all, &Rx, &Tx, &IP, &lastarg, GPIBCmdBuf + os);

	if (res != 1)
		return S3_GPIB_INVALID_ADDRESS;

	return S3RxSetPN(Rx, lastarg);
}

// ----------------------------------------------------------------------------

int DbgRxSN(int os)
{
	char		all, Rx, Tx, IP;
	const char	*lastarg;

	int res = GetAddress(&all, &Rx, &Tx, &IP, &lastarg, GPIBCmdBuf + os);

	if (res != 1)
		return S3_GPIB_INVALID_ADDRESS;

	return S3RxSetSN(Rx, lastarg);
}

// ----------------------------------------------------------------------------

int DbgTxPN(int os)
{
	char		all, Rx, Tx, IP;
	const char	*lastarg;

	int res = GetAddress(&all, &Rx, &Tx, &IP, &lastarg, GPIBCmdBuf + os);

	if (res != 2)
		return S3_GPIB_INVALID_ADDRESS;

	return S3TxSetPN(Rx, Tx, lastarg);
}

// ----------------------------------------------------------------------------

int DbgTxSN(int os)
{
	char		all, Rx, Tx, IP;
	const char	*lastarg;

	int res = GetAddress(&all, &Rx, &Tx, &IP, &lastarg, GPIBCmdBuf + os);

	if (res != 2)
		return S3_GPIB_INVALID_ADDRESS;

	return S3TxSetSN(Rx, Tx, lastarg);
}

// -------------------------------------------------------------------------
// Tx battery

// ----------------------------------------------------------------------------
// Read by S3PollRx

int DbgTxSoC(int os)
{
	char			all, Rx, Tx, IP;
	const char		*lastarg;
	char			*eptr;

	int res = GetAddress(&all, &Rx, &Tx, &IP, &lastarg, GPIBCmdBuf + os);

	if (res != 2)
		return S3_GPIB_INVALID_ADDRESS;

	unsigned char BattSoC = (unsigned char)strtol(lastarg, &eptr, 10);

	DbgPollSysData.Rx[Rx].Txs[Tx].SoC = BattSoC;

	return 0;
}

// -------------------------------------------------------------------------
// Read by S3PollRx

int DbgTxBattTemp(int os)
{
	char			all, Rx, Tx, IP;
	const char		*lastarg;
	char			*eptr;
	int res = GetAddress(&all, &Rx, &Tx, &IP, &lastarg, GPIBCmdBuf + os);

	if (res != 2)
		return S3_GPIB_INVALID_ADDRESS;

	long l = strtol(lastarg, &eptr, 10);
	l = (l > SCHAR_MAX) ? SCHAR_MAX : l;
	l = (l < SCHAR_MIN) ? SCHAR_MIN : l;

	char BattT = (char)l;

	DbgPollSysData.Rx[Rx].Txs[Tx].BattTemp = BattT;

	return 0;
}

// ----------------------------------------------------------------------------
// Test data only

int DbgTxBattAll(int os)
{
	char			*eptr;
	unsigned char	BattCh = (unsigned char)strtol(GPIBCmdBuf + os, &eptr, 10);

	for (char Rx = 0; Rx < S3_MAX_RXS; Rx++)
		for (char Tx = 0; Tx < S3_MAX_TXS; Tx++)
			DbgPollSysData.Rx[Rx].Txs[Tx].SoC = BattCh;

	return 0;
}

// ----------------------------------------------------------------------------

int DbgTxBattFWV(int os)
{
	char			all, Rx, Tx, IP;
	const char		*lastarg;
	int res = GetAddress(&all, &Rx, &Tx, &IP, &lastarg, GPIBCmdBuf + os);

	if (res != 2)
		return S3_GPIB_INVALID_ADDRESS;

	if (lastarg)
		strcpy_s(DbgPollSysData.Rx[Rx].Txs[Tx].BattFW, S3_MAX_SW_VER_LEN, lastarg);
	else
		return 1;

	return 0;
}

// ----------------------------------------------------------------------------

int DbgTxBattHWV(int os)
{
	char			all, Rx, Tx, IP;
	const char		*lastarg;
	int res = GetAddress(&all, &Rx, &Tx, &IP, &lastarg, GPIBCmdBuf + os);

	if (res != 2)
		return S3_GPIB_INVALID_ADDRESS;

	if (lastarg)
		strcpy_s(DbgPollSysData.Rx[Rx].Txs[Tx].BattHW, S3_MAX_SW_VER_LEN, lastarg);
	else
		return 1;

	return 0;
}

// ----------------------------------------------------------------------------

int DbgTxBattSN(int os)
{
	char			all, Rx, Tx, IP;
	const char		*lastarg;
	int res = GetAddress(&all, &Rx, &Tx, &IP, &lastarg, GPIBCmdBuf + os);

	if (res != 2)
		return S3_GPIB_INVALID_ADDRESS;

	if (lastarg)
		strcpy_s(DbgPollSysData.Rx[Rx].Txs[Tx].BattSN, S3_MAX_SN_LEN, lastarg);
	else
		return 1;

	return 0;
}

// ----------------------------------------------------------------------------

int DbgTxBattPN(int os)
{
	char			all, Rx, Tx, IP;
	const char		*lastarg;
	int res = GetAddress(&all, &Rx, &Tx, &IP, &lastarg, GPIBCmdBuf + os);

	if (res != 2)
		return S3_GPIB_INVALID_ADDRESS;

	if (lastarg)
		strcpy_s(DbgPollSysData.Rx[Rx].Txs[Tx].BattPN, S3_MAX_PN_LEN, lastarg);
	else
		return 1;

	return 0;
}
// ----------------------------------------------------------------------------

int DbgCancelAlarms(int os)
{
	return S3CancelAlarms();
}

// ----------------------------------------------------------------------------

int DbgChSoC(int os)
{
	char	*eptr;

	char Ch = (char)strtol(GPIBCmdBuf + os, &eptr, 10) - 1;
	char SoC = (char)strtol(eptr, &eptr, 10);

	DbgPollSysData.Ch[Ch].SoC = SoC;

	return 0;
}

// ----------------------------------------------------------------------------

int DbgChBattInsert(int os)
{
	char	*eptr;

	char Ch = (char)strtol(GPIBCmdBuf + os, &eptr, 10) - 1;

	if (eptr)
		strcpy_s(DbgPollSysData.Ch[Ch].SN, S3_MAX_ID_LEN, eptr);
	else // Serial number is mandatory
		return 1;

	DbgPollSysData.Ch[Ch].Occupied = 1;

	return 0;
}

// ----------------------------------------------------------------------------

int DbgChBattRemove(int os)
{
	char	*eptr;

	char Ch = (char)strtol(GPIBCmdBuf + os, &eptr, 10) - 1;

	DbgPollSysData.Ch[Ch].Occupied = 0;

	return 0;
}

// ----------------------------------------------------------------------------

int DbgChBattHWV(int os)
{
	char	*eptr;

	char Ch = (char)strtol(GPIBCmdBuf + os, &eptr, 10) - 1;

	if (eptr)
		strcpy_s(DbgPollSysData.Ch[Ch].HW, S3_MAX_SW_VER_LEN, eptr);
	else
		return 1;

	return 0;
}

// ----------------------------------------------------------------------------

int DbgChBattFWV(int os)
{
	char	*eptr;
	char	Ch = (char)strtol(GPIBCmdBuf + os, &eptr, 10) - 1;

	if (eptr)
		strcpy_s(DbgPollSysData.Ch[Ch].FW, S3_MAX_SW_VER_LEN, eptr);
	else
		return 1;

	return 0;
}

// ----------------------------------------------------------------------------

int DbgChBattPN(int os)
{
	char	*eptr;
	char	Ch = (char)strtol(GPIBCmdBuf + os, &eptr, 10) - 1;

	if (eptr)
		strcpy_s(DbgPollSysData.Ch[Ch].PN, S3_MAX_SW_VER_LEN, eptr);
	else
		return 1;

	return 0;
}

// ----------------------------------------------------------------------------

int DbgChBattSN(int os)
{
	char	*eptr;
	char	Ch = (char)strtol(GPIBCmdBuf + os, &eptr, 10) - 1;

	if (eptr)
		strcpy_s(DbgPollSysData.Ch[Ch].SN, S3_MAX_SW_VER_LEN, eptr);
	else
		return 1;

	return 0;
}
// -------------------------------------------------------------------------
// Read by S3Poll()

int DbgChBattTemp(int os)
{
	// To be got from I2C eventually

	char	*eptr;
	char	Ch = (char)strtol(GPIBCmdBuf + os, &eptr, 10) - 1;

	if (eptr)
	{
		long	l = strtol(eptr, &eptr, 10);

		l = (l > SCHAR_MAX) ? SCHAR_MAX : l;
		l = (l < SCHAR_MIN) ? SCHAR_MIN : l;

		char BattT = (char)l;

		DbgPollSysData.Ch[Ch].BattTemp = BattT;
	}	
	else
		return 1;

	return 0;
}

// ----------------------------------------------------------------------------
// Not bothering with polling (or checking) - this is just to populate for
// testing

int DbgTxHW(int os)
{
	char			all, Rx, Tx, IP;
	const char		*lastarg;
	int res = GetAddress(&all, &Rx, &Tx, &IP, &lastarg, GPIBCmdBuf + os);

	if (res != 2)
		return S3_GPIB_INVALID_ADDRESS;

	if (lastarg)
		S3TxSetHW(Rx, Tx, lastarg);
	else
		return 1;

	return 0;
}

int DbgTxFW(int os)
{
	char			all, Rx, Tx, IP;
	const char		*lastarg;
	int res = GetAddress(&all, &Rx, &Tx, &IP, &lastarg, GPIBCmdBuf + os);

	if (res != 2)
		return S3_GPIB_INVALID_ADDRESS;

	if (lastarg)
		S3TxSetFW(Rx, Tx, lastarg);
	else
		return 1;

	return 0;
}

int DbgRxHW(int os)
{
	char			all, Rx, Tx, IP;
	const char		*lastarg;
	int res = GetAddress(&all, &Rx, &Tx, &IP, &lastarg, GPIBCmdBuf + os);

	if (res != 1)
		return S3_GPIB_INVALID_ADDRESS;

	if (lastarg)
		S3RxSetHW(Rx, lastarg);
	else
		return 1;

	return 0;
}

int DbgRxFW(int os)
{
	char			all, Rx, Tx, IP;
	const char		*lastarg;
	int res = GetAddress(&all, &Rx, &Tx, &IP, &lastarg, GPIBCmdBuf + os);

	if (res != 1)
		return S3_GPIB_INVALID_ADDRESS;

	if (lastarg)
		S3RxSetFW(Rx, lastarg);
	else
		return 1;

	return 0;
}


// -------------------------------------------------------------------------
