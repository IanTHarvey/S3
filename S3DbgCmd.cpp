// ----------------------------------------------------------------------------
//
// All these commands are to simulate the data available via the various I2C
// paths. S3 functions will only poll the data, not using it's own functions
// to access anything directly (unless stated otherwise).
// 
// ----------------------------------------------------------------------------

#include "stdafx.h"

#include "S3DataModel.h"
#include "S3GPIB.h"

struct DbgPollSysStruct	DbgPollSysData;

extern char		GPIBCmdBuf[];
extern char		GPIBRetBuf[];
extern unsigned	GPIBBufLen;
extern char		GPIBCurrentRx;
extern char		GPIBCurrentTx; // For settings etc
extern char		GPIBCurrentIP;

extern char		*GPIBCmdArgs[];

int DbgRxInsert();
int DbgRxRemove();

int DbgTxInsert();
int DbgTxRemove();
int DbgTxSelect();
int DbgTxBrownOut();

int DbgTxFW();
int DbgTxHW();
int DbgRxFW();
int DbgRxHW();

int DbgLock();

int DbgRxRLL();

int DbgIPOverdrive();
int DbgCancelAlarms();

int	DbgRxLinkGain();

// Charger battery
int DbgChBattInsert();
int DbgChBattRemove();

int DbgChBattHWV();
int DbgChBattFWV();
int DbgChBattSN();
int DbgChBattPN();

int DbgChSoC();
int DbgChBattTemp();

// Tx battery
int DbgTxBattAll();
int DbgTxBattHWV();
int DbgTxBattFWV();
int DbgTxBattSN();
int DbgTxBattPN();

int DbgTxSoC();
int DbgTxBattTemp();

// ----------------------------------------------------------------------------
// Event simulators

int DbgProcessCmd()
{
	int err;
	char *Cmd = GPIBCmdArgs[0];

	if (!STRCMP(Cmd, "DBGRXINSERT"))
	{
		err = DbgRxInsert();
	}
	else if (!STRCMP(Cmd, "DBGRXREMOVE"))
	{
		err = DbgRxRemove();
	}
	else if (!STRCMP(Cmd, "DBGTXINSERT"))
	{
		err = DbgTxInsert();
	}
	else if (!STRCMP(Cmd, "DBGTXREMOVE"))
	{
		err = DbgTxRemove();
	}
	else if (!STRCMP(Cmd, "DBGTXSELECT"))
	{
		err = DbgTxSelect();
	}
	else if (!STRCMP(Cmd, "DBGTXBROWNOUT"))
	{
		err = DbgTxBrownOut();
	}
	else if (!STRCMP(Cmd, "DBGIPOVERDRIVE"))
	{
		err = DbgIPOverdrive();
	}
	else if (!STRCMP(Cmd, "DBGRXRLL"))
	{
		err = DbgRxRLL();
	}
	else if (!STRCMP(Cmd, "DBGSYSPARTNO"))
	{
		err = DbgSysPN();
	}
	else if (!STRCMP(Cmd, "DBGSYSSERIALNO"))
	{
		err = DbgSysSN();
	}
	else if (!STRCMP(Cmd, "DBGRXPARTNO"))
	{
		err = DbgRxPN();
	}
	else if (!STRCMP(Cmd, "DBGRXSERIALNO"))
	{
		err = DbgRxSN();
	}
	else if (!STRCMP(Cmd, "DBGRXLINKGAIN"))
	{
		err = DbgRxLinkGain();
	}
	else if (!STRCMP(Cmd, "DBGTXPARTNO"))
	{
		err = DbgTxPN();
	}
	else if (!STRCMP(Cmd, "DBGTXSERIALNO"))
	{
		err = DbgTxSN();
	}
	else if (!STRCMP(Cmd, "DBGTXBATTLEVEL"))
	{
		err = DbgTxSoC();
	}
	else if (!STRCMP(Cmd, "DBGTXBATTHW"))
	{
		err = DbgTxBattHWV();
	}
	else if (!STRCMP(Cmd, "DBGTXBATTFW"))
	{
		err = DbgTxBattFWV();
	}
	else if (!STRCMP(Cmd, "DBGTXBATTSN"))
	{
		err = DbgTxBattSN();
	}
	else if (!STRCMP(Cmd, "DBGTXBATTPN"))
	{
		err = DbgTxBattPN();
	}
	else if (!STRCMP(Cmd, "DBGTXBATTTEMP"))
	{
		err = DbgTxBattTemp();
	}
	else if (!STRCMP(Cmd, "DBGTXBATTALL"))
	{
		err = DbgTxBattAll();
	}
	else if (!STRCMP(Cmd, "DBGCANCELALL"))
	{
		err = DbgCancelAlarms();
	}
	else if (!STRCMP(Cmd, "DBGCHBATTLEVEL"))
	{
		err = DbgChSoC();
	}
	else if (!STRCMP(Cmd, "DBGCHBATTINSERT"))
	{
		err = DbgChBattInsert();
	}
	else if (!STRCMP(Cmd, "DBGCHBATTREMOVE"))
	{
		err = DbgChBattRemove();
	}
	else if (!STRCMP(Cmd, "DBGCHBATTHW"))
	{
		err = DbgChBattHWV();
	}
	else if (!STRCMP(Cmd, "DBGCHBATTFW"))
	{
		err = DbgChBattFWV();
	}
	else if (!STRCMP(Cmd, "DBGCHBATTPN"))
	{
		err = DbgChBattPN();
	}
	else if (!STRCMP(Cmd, "DBGCHBATTSN"))
	{
		err = DbgChBattSN();
	}
	else if (!STRCMP(Cmd, "DBGCHBATTTEMP"))
	{
		err = DbgChBattTemp();
	}
	else if (!STRCMP(Cmd, "DBGTXHW"))
	{
		err = DbgTxHW();
	}
	else if (!STRCMP(Cmd, "DBGTXFW"))
	{
		err = DbgTxFW();
	}
	else if (!STRCMP(Cmd, "DBGRXHW"))
	{
		err = DbgRxHW();
	}
	else if (!STRCMP(Cmd, "DBGRXFW"))
	{
		err = DbgRxFW();
	}
	else if (!STRCMP(Cmd, "DBGLOCK"))
	{
		err = DbgLock();
	}
	else	err = S3_GPIB_CMD_UNRECOGNISED;

	return err;
}

// ----------------------------------------------------------------------------
// Rxs not hot-swappable but may happen anyway.

int DbgRxInsert()
{
	char	*eptr;
	char	Rx = (char)strtol(GPIBCmdArgs[1], &eptr, 10) - 1;

	if (!S3RxValidQ(Rx))
		return S3_GPIB_INVALID_ADDRESS;

	char type = (char)strtol(GPIBCmdArgs[2], &eptr, 10);

	if (type != S3_Rx1	&& type != S3_Rx2 && type != S3_Rx6)
		return 2;

	// WiP
	DbgPollSysData.Rx[Rx].OccupierType = type;

	return 0;
}

// ----------------------------------------------------------------------------
// Ditto.

int DbgRxRemove()
{
	char	*eptr;
	char	Rx = (char)strtol(GPIBCmdArgs[1], &eptr, 10) - 1;

	if (!S3RxValidQ(Rx))
		return S3_GPIB_INVALID_ADDRESS;

	if (!S3RxExistQ(Rx))
		return S3_GPIB_INVALID_RX;

	DbgPollSysData.Rx[Rx].OccupierType = S3_RxEmpty;

	return 0;
}

// ----------------------------------------------------------------------------
// FOL connection

int DbgTxInsert()
{
	char	*eptr;
	char	Rx = (char)strtol(GPIBCmdArgs[1], &eptr, 10) - 1;
	char	Tx = (char)strtol(GPIBCmdArgs[2], &eptr, 10) - 1;

	if (!S3TxValidQ(Rx, Tx))
		return S3_GPIB_INVALID_ADDRESS;

	if (S3TxExistQ(Rx, Tx))
		return S3_GPIB_INVALID_TX;

	char type = (char)strtol(GPIBCmdArgs[2], &eptr, 10);

	if (type != S3_Tx1	&& type != S3_Tx8)
		return 2;

	DbgPollSysData.Rx[Rx].Txs[Tx].type = type;
	DbgPollSysData.Rx[Rx].RLL[Tx] = 0;

	return 0;
}

// ----------------------------------------------------------------------------
// FOL disconnection

int DbgTxRemove()
{
	char	*eptr;
	char	Rx = (char)strtol(GPIBCmdArgs[1], &eptr, 10) - 1;
	char	Tx = (char)strtol(GPIBCmdArgs[2], &eptr, 10) - 1;

	if (!S3TxValidQ(Rx, Tx))
		return S3_GPIB_INVALID_ADDRESS;

	if (!S3TxExistQ(Rx, Tx))
		return S3_GPIB_INVALID_TX;

	DbgPollSysData.Rx[Rx].Txs[Tx].type = S3_TxUnconnected;
	DbgPollSysData.Rx[Rx].RLL[Tx] = 0;

	return 0;
}

// ----------------------------------------------------------------------------

int DbgTxSelect()
{
	char	*eptr;
	char	Rx = (char)strtol(GPIBCmdArgs[1], &eptr, 10) - 1;
	char	Tx = (char)strtol(GPIBCmdArgs[2], &eptr, 10) - 1;

	if (!S3TxValidQ(Rx, Tx))
		return S3_GPIB_INVALID_ADDRESS;

	if (!S3TxExistQ(Rx, Tx))
		return S3_GPIB_INVALID_TX;

	S3RxSetHighlightedTx(Rx, Tx);

	return 0;
}

// ----------------------------------------------------------------------------

int DbgTxBrownOut()
{
	char	*eptr;
	char	Rx = (char)strtol(GPIBCmdArgs[1], &eptr, 10) - 1;
	char	Tx = (char)strtol(GPIBCmdArgs[2], &eptr, 10) - 1;

	if (!S3TxValidQ(Rx, Tx))
		return S3_GPIB_INVALID_ADDRESS;

	if (!S3TxExistQ(Rx, Tx))
		return S3_GPIB_INVALID_TX;

	char val = (char)strtol(GPIBCmdArgs[3], &eptr, 10);

	DbgPollSysData.Rx[Rx].Txs[Tx].BrownOut = val;

	return 0;
}

// ----------------------------------------------------------------------------

int DbgRxRLL()
{
	char	*eptr;
	char	Rx = (char)strtol(GPIBCmdArgs[1], &eptr, 10) - 1;
	char	Tx = (char)strtol(GPIBCmdArgs[2], &eptr, 10) - 1;

	if (!S3TxValidQ(Rx, Tx))
		return S3_GPIB_INVALID_ADDRESS;

	if (!S3TxExistQ(Rx, Tx))
		return S3_GPIB_INVALID_TX;

	short val = (short)strtol(GPIBCmdArgs[3], &eptr, 10);

	DbgPollSysData.Rx[Rx].RLL[Tx] = val;

	return 0;
}

// ----------------------------------------------------------------------------

int DbgRxLinkGain()
{
	char	*eptr;
	char	Rx = (char)strtol(GPIBCmdArgs[1], &eptr, 10) - 1;
	char	Tx = (char)strtol(GPIBCmdArgs[2], &eptr, 10) - 1;

	if (!S3TxValidQ(Rx, Tx))
		return S3_GPIB_INVALID_ADDRESS;

	if (!S3TxExistQ(Rx, Tx))
		return S3_GPIB_INVALID_TX;

	char val = (char)strtol(GPIBCmdArgs[3], &eptr, 10);

	DbgPollSysData.Rx[Rx].LinkGain[Tx] = val;

	return 0;
}

// ----------------------------------------------------------------------------

int DbgIPOverdrive()
{

	char		all, Rx, Tx, IP;
	int res = GetAddress2(&all, &Rx, &Tx, &IP);

	if (res != 3)
		return S3_GPIB_INVALID_ADDRESS;

	char val = (char)strtol(GPIBCmdArgs[4], NULL, 10);

	if (val)
		S3IPSetAlarm(Rx, Tx, IP, S3_IP_OVERDRIVE);
	else
		S3IPCancelAlarm(Rx, Tx, IP, S3_IP_OVERDRIVE);

	return 0;
}

// ----------------------------------------------------------------------------

int DbgSysPN()
{
	return S3SysSetPN(GPIBCmdArgs[1]);
}

// ----------------------------------------------------------------------------

int DbgSysSN()
{
	return S3SysSetSN(GPIBCmdArgs[1]);
}

// ----------------------------------------------------------------------------

int DbgRxPN()
{
	char		all, Rx, Tx, IP;

	int res = GetAddress2(&all, &Rx, &Tx, &IP);

	if (res != 1)
		return S3_GPIB_INVALID_ADDRESS;

	return S3RxSetPN(Rx, GPIBCmdArgs[2]);
}

// ----------------------------------------------------------------------------

int DbgRxSN()
{
	char		all, Rx, Tx, IP;

	int res = GetAddress2(&all, &Rx, &Tx, &IP);

	if (res != 1)
		return S3_GPIB_INVALID_ADDRESS;

	return S3RxSetSN(Rx, GPIBCmdArgs[2]);
}

// ----------------------------------------------------------------------------

int DbgTxPN()
{
	char		all, Rx, Tx, IP;

	int res = GetAddress2(&all, &Rx, &Tx, &IP);

	if (res != 2)
		return S3_GPIB_INVALID_ADDRESS;

	return S3TxSetPN(Rx, Tx, GPIBCmdArgs[3]);
}

// ----------------------------------------------------------------------------

int DbgTxSN()
{
	char		all, Rx, Tx, IP;

	int res = GetAddress2(&all, &Rx, &Tx, &IP);
	if (res != 2)
		return S3_GPIB_INVALID_ADDRESS;

	return S3TxSetSN(Rx, Tx, GPIBCmdArgs[3]);
}

// -------------------------------------------------------------------------
// Tx battery

// ----------------------------------------------------------------------------
// Read by S3PollRx

int DbgTxSoC()
{
	char			all, Rx, Tx, IP;
	char			*eptr;

	int res = GetAddress2(&all, &Rx, &Tx, &IP);

	if (res != 2)
		return S3_GPIB_INVALID_ADDRESS;

	unsigned char BattSoC = (unsigned char)strtol(GPIBCmdArgs[3], &eptr, 10);

	DbgPollSysData.Rx[Rx].Txs[Tx].SoC = BattSoC;

	return 0;
}

// -------------------------------------------------------------------------
// Read by S3PollRx

int DbgTxBattTemp()
{
	char			all, Rx, Tx, IP;
	char			*eptr;
	int res = GetAddress2(&all, &Rx, &Tx, &IP);

	if (res != 2)
		return S3_GPIB_INVALID_ADDRESS;

	long l = strtol(GPIBCmdArgs[3], &eptr, 10);
	l = (l > SCHAR_MAX) ? SCHAR_MAX : l;
	l = (l < SCHAR_MIN) ? SCHAR_MIN : l;

	char BattT = (char)l;

	DbgPollSysData.Rx[Rx].Txs[Tx].BattTemp = BattT;

	return 0;
}

// ----------------------------------------------------------------------------
// Test data only

int DbgTxBattAll()
{
	char			*eptr;
	unsigned char	BattCh = (unsigned char)strtol(GPIBCmdArgs[1], &eptr, 10);

	for (char Rx = 0; Rx < S3_MAX_RXS; Rx++)
		for (char Tx = 0; Tx < S3_MAX_TXS; Tx++)
			DbgPollSysData.Rx[Rx].Txs[Tx].SoC = BattCh;

	return 0;
}

// ----------------------------------------------------------------------------

int DbgTxBattFWV()
{
	char			all, Rx, Tx, IP;
	int res = GetAddress2(&all, &Rx, &Tx, &IP);

	if (res != 2)
		return S3_GPIB_INVALID_ADDRESS;

	if (GPIBCmdArgs[3])
		strcpy_s(DbgPollSysData.Rx[Rx].Txs[Tx].BattFW, S3_MAX_SW_VER_LEN, GPIBCmdArgs[3]);
	else
		return 1;

	return 0;
}

// ----------------------------------------------------------------------------

int DbgTxBattHWV()
{
	char			all, Rx, Tx, IP;
	int res = GetAddress2(&all, &Rx, &Tx, &IP);

	if (res != 2)
		return S3_GPIB_INVALID_ADDRESS;

	if (GPIBCmdArgs[3])
		strcpy_s(DbgPollSysData.Rx[Rx].Txs[Tx].BattHW, S3_MAX_SW_VER_LEN, GPIBCmdArgs[3]);
	else
		return 1;

	return 0;
}

// ----------------------------------------------------------------------------

int DbgTxBattSN()
{
	char			all, Rx, Tx, IP;
	int res = GetAddress2(&all, &Rx, &Tx, &IP);

	if (res != 2)
		return S3_GPIB_INVALID_ADDRESS;

	if (GPIBCmdArgs[3])
		strcpy_s(DbgPollSysData.Rx[Rx].Txs[Tx].BattSN, S3_MAX_SN_LEN, GPIBCmdArgs[3]);
	else
		return 1;

	return 0;
}

// ----------------------------------------------------------------------------

int DbgTxBattPN()
{
	char			all, Rx, Tx, IP;
	int res = GetAddress2(&all, &Rx, &Tx, &IP);

	if (res != 2)
		return S3_GPIB_INVALID_ADDRESS;

	if (GPIBCmdArgs[3])
		strcpy_s(DbgPollSysData.Rx[Rx].Txs[Tx].BattPN, S3_MAX_PN_LEN, GPIBCmdArgs[3]);
	else
		return 1;

	return 0;
}
// ----------------------------------------------------------------------------

int DbgCancelAlarms()
{
	return S3CancelAlarms();
}

// ----------------------------------------------------------------------------

int DbgChSoC()
{
	char	*eptr;

	char Ch = (char)strtol(GPIBCmdArgs[1], &eptr, 10) - 1;
	char SoC = (char)strtol(GPIBCmdArgs[2], &eptr, 10);

	DbgPollSysData.Ch[Ch].SoC = SoC;

	return 0;
}

// ----------------------------------------------------------------------------

int DbgChBattInsert()
{
	char	*eptr;

	char Ch = (char)strtol(GPIBCmdArgs[1], &eptr, 10) - 1;

	if (eptr)
		strcpy_s(DbgPollSysData.Ch[Ch].SN, S3_MAX_ID_LEN, GPIBCmdArgs[2]);
	else // Serial number is mandatory
		return 1;

	DbgPollSysData.Ch[Ch].Occupied = 1;

	return 0;
}

// ----------------------------------------------------------------------------

int DbgChBattRemove()
{
	char	*eptr;

	char Ch = (char)strtol(GPIBCmdArgs[1], &eptr, 10) - 1;

	DbgPollSysData.Ch[Ch].Occupied = 0;

	return 0;
}

// ----------------------------------------------------------------------------

int DbgChBattHWV()
{
	char	*eptr;

	char Ch = (char)strtol(GPIBCmdArgs[1], &eptr, 10) - 1;

	if (eptr)
		strcpy_s(DbgPollSysData.Ch[Ch].HW, S3_MAX_SW_VER_LEN, GPIBCmdArgs[2]);
	else
		return 1;

	return 0;
}

// ----------------------------------------------------------------------------

int DbgChBattFWV()
{
	char	*eptr;
	char	Ch = (char)strtol(GPIBCmdArgs[1], &eptr, 10) - 1;

	if (eptr)
		strcpy_s(DbgPollSysData.Ch[Ch].FW, S3_MAX_SW_VER_LEN, GPIBCmdArgs[2]);
	else
		return 1;

	return 0;
}

// ----------------------------------------------------------------------------

int DbgChBattPN()
{
	char	*eptr;
	char	Ch = (char)strtol(GPIBCmdArgs[1], &eptr, 10) - 1;

	if (eptr)
		strcpy_s(DbgPollSysData.Ch[Ch].PN, S3_MAX_SW_VER_LEN, GPIBCmdArgs[2]);
	else
		return 1;

	return 0;
}

// ----------------------------------------------------------------------------

int DbgChBattSN()
{
	char	*eptr;
	char	Ch = (char)strtol(GPIBCmdArgs[1], &eptr, 10) - 1;

	if (eptr)
		strcpy_s(DbgPollSysData.Ch[Ch].SN, S3_MAX_SW_VER_LEN, GPIBCmdArgs[2]);
	else
		return 1;

	return 0;
}
// -------------------------------------------------------------------------
// Read by S3Poll()

int DbgChBattTemp()
{
	// To be got from I2C eventually

	char	*eptr;
	char	Ch = (char)strtol(GPIBCmdArgs[1], &eptr, 10) - 1;

	if (eptr)
	{
		long	l = strtol(GPIBCmdArgs[2], &eptr, 10);

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

int DbgTxHW()
{
	char			all, Rx, Tx, IP;
	int res = GetAddress2(&all, &Rx, &Tx, &IP);

	if (res != 2)
		return S3_GPIB_INVALID_ADDRESS;

	if (GPIBCmdArgs[3])
		S3TxSetHW(Rx, Tx, GPIBCmdArgs[3]);
	else
		return 1;

	return 0;
}

// ----------------------------------------------------------------------------

int DbgTxFW()
{
	char			all, Rx, Tx, IP;
	int res = GetAddress2(&all, &Rx, &Tx, &IP);

	if (res != 2)
		return S3_GPIB_INVALID_ADDRESS;

	if (GPIBCmdArgs[3])
		S3TxSetFW(Rx, Tx, GPIBCmdArgs[3]);
	else
		return 1;

	return 0;
}

// ----------------------------------------------------------------------------

int DbgRxHW()
{
	char			all, Rx, Tx, IP;
	int res = GetAddress2(&all, &Rx, &Tx, &IP);

	if (res != 1)
		return S3_GPIB_INVALID_ADDRESS;

	if (GPIBCmdArgs[2])
		S3RxSetHW(Rx, GPIBCmdArgs[2]);
	else
		return 1;

	return 0;
}

// -------------------------------------------------------------------------

int DbgRxFW()
{
	char			all, Rx, Tx, IP;
	int res = GetAddress2(&all, &Rx, &Tx, &IP);

	if (res != 1)
		return S3_GPIB_INVALID_ADDRESS;

	if (GPIBCmdArgs[2])
		S3RxSetFW(Rx, GPIBCmdArgs[2]);
	else
		return 1;

	return 0;
}

// -------------------------------------------------------------------------

int DbgLock()
{
	char	*eptr;
	long l = strtol(GPIBCmdArgs[1], &eptr, 10);

	if (l != 0 && l != 1)
		return S3_GPIB_INVALID_PARAMETER;

	bool locked = (l == 1);

	S3SetLocked(locked);

	return 0;
}

// -------------------------------------------------------------------------
