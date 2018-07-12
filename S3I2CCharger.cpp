// ----------------------------------------------------------------------------
// I2C master select
//
// Rx & Ch used shared I2C which shouldn't be a problem as on different
// addresses. However making battery and and Rx MSs exclusive seems to make it
// more robust.
//
// TTL IO
// 0 MS_BAT_2
// 1 MS_BAT_1
// 2 MS_1
// 3 MS_2
// 4 MS_3
// 5 MS_4
// 6 MS_5
// 7 MS_6
// 
// Expander Port 0
// 0 MS_BAT_3
// 1 MS_BAT_4
// 2 BAT_EN_4
// 3 BAT_EN_3
// 4 BAT_EN_2
// 5 BAT_EN_1
// 6 BAT_FAULT_4
// 7 BAT_FAULT_3
// 
// -----------------------------------------------------------------------------

#include "stdafx.h"

#include "S3DataModel.h"

extern pS3DataModel S3Data;

#ifdef TRIZEPS
#include "drvLib_app.h"
#endif

#include "S3I2C.h"
#include "S3Gain.h"

#define S3_FLASH_BLOCK_SIZE	32
#define S3_FLASH_SN_SIZE	10
#define S3_FLASH_PN_SIZE	21

int S3I2CChGetFault();

// Back-plane alarm lines
#define S3_ALARM_RX_FAN1	0x80
#define S3_ALARM_RX_FAN2	0x40
#define S3_ALARM_RX0		0x20
#define S3_ALARM_RX1		0x10
#define S3_ALARM_RX2		0x08
#define S3_ALARM_RX3		0x04
#define S3_ALARM_RX4		0x02
#define S3_ALARM_RX5		0x01

// ----------------------------------------------------------------------------
// TODO: Use GPIO for chargers 0 & 1

unsigned char CountPins(unsigned char pins)
{
	unsigned char  count = 0;
    for(int i = 0; i < 8; i++)
        count += (pins >> i) & 0x01;

	return count;
}

// ----------------------------------------------------------------------------
// Charger master-select

int S3I2CChMS(unsigned char Ch)
{
#ifdef TRIZEPS
	int pins;

	// TODO: Not currently used
	unsigned short Alarms = Get_TTLPort(0xFFFF, 0); // & 0x003f;

	if (Ch == 0xFF)
	{
		// Deselect all chargers
		// return 0;

		// Zero bits 0 & 1 of expander port 0
		pins = I2C_ReadRandom(S3I2C_EXPANDER_ADDR, 0x04);
		pins &= ~0x03;
		I2C_WriteRandom(S3I2C_EXPANDER_ADDR, 0x04, pins);

		// Zero pins 0 & 1 of TTL port
		pins = Get_TTLOutPort(0xFFFF, 0);
		pins &= ~0x03;
		Set_TTLPort(pins, 0);

		return 0;
	}

	if (Ch > 1)
	{
		// MS on TTL port pins 0 & 1

		// Clear pins 0 & 1 on expander port
		pins = I2C_ReadRandom(S3I2C_EXPANDER_ADDR, 0x04);
		pins &= ~MS_BAT_3;
		pins &= ~MS_BAT_4;

		I2C_WriteRandom(S3I2C_EXPANDER_ADDR, 0x04, pins);

		// Set pin 0/1 on TTL port and clear Rx MS pins
		pins = 0;
		
		if (Ch == 3)
		{
			pins |= MS_BAT_1;
		}
		else if (Ch == 2)
		{
			pins |= MS_BAT_2;
		}

		Set_TTLPort(pins, 0);
	}
	else
	{
		// MS on expander port 0: pins 0 & 1

		// Clear all Rx MS pins on TTL port
		pins = 0;
		Set_TTLPort(pins, 0);

		// Set pins 0/1 on expander port
		pins = I2C_ReadRandom(S3I2C_EXPANDER_ADDR, 0x04);

		if (Ch == 1)
		{
			pins |= MS_BAT_3;
			pins &= ~MS_BAT_4;
		}
		else if (Ch == 0)
		{
			pins &= ~MS_BAT_3;
			pins |= MS_BAT_4;
		}

		I2C_WriteRandom(S3I2C_EXPANDER_ADDR, 0x04, pins);
		// Sleep(1);
	}

	// ASSERT?
	// if (CountPins(pins) > 1)
	//	return 0;


#endif // TRIZEPS
	return 0;
}

// ----------------------------------------------------------------------------
// MS lines are now exclusive

int S3I2CRxMS(unsigned char Rx)
{
#ifdef TRIZEPS

	unsigned char pins;

	// Clear battery MS pins 0 & 1 on expander port
	pins = I2C_ReadRandom(S3I2C_EXPANDER_ADDR, 0x04);
	pins &= ~MS_BAT_3;
	pins &= ~MS_BAT_4;

	I2C_WriteRandom(S3I2C_EXPANDER_ADDR, 0x04, pins);

	// Set Rx MS pin on TTL port and clear battery MSs
	unsigned short pin;
	if (Rx == 0xFF)
		pin = 0;
	else
		pin = 1 << (Rx + 2); 
		
	// First arg appears to make no difference
	Clr_TTLPortBit(0xFFFF, 0);
	Set_TTLPortBit(pin, 0);

#endif

	// ASSERT?
	// if (CountPins((unsigned char)pin) > 1)
	// 	return 0;

	return 0;
}

// ----------------------------------------------------------------------------

int S3I2CChGetFault()
{
#ifdef TRIZEPS
	unsigned char pins0 = I2C_ReadRandom(S3I2C_EXPANDER_ADDR, 0x00);
	unsigned char pins1 = I2C_ReadRandom(S3I2C_EXPANDER_ADDR, 0x01);

	unsigned char fault[4] = {0, 0, 0, 0};
	if (!(pins0 & 0x40))
	{
		S3ChSetAlarm(3, S3_CH_CHARGE_FAULT);
		fault[3] = 1;
	}
	else
		S3ChCancelAlarm(3, S3_CH_CHARGE_FAULT);

	if (!(pins0 & 0x80))
	{
		S3ChSetAlarm(2, S3_CH_CHARGE_FAULT);
		fault[2] = 1;
	}
	else
		S3ChCancelAlarm(2, S3_CH_CHARGE_FAULT);

	if (!(pins1 & 0x01))
	{
		S3ChSetAlarm(1, S3_CH_CHARGE_FAULT);
		fault[1] = 1;
	}
	else
		S3ChCancelAlarm(1, S3_CH_CHARGE_FAULT);

	if (!(pins1 & 0x02))
	{
		S3ChSetAlarm(0, S3_CH_CHARGE_FAULT);
		fault[0] = 1;
	}
	else
		S3ChCancelAlarm(0, S3_CH_CHARGE_FAULT);

	// char Msg[S3_EVENTS_LINE_LEN];

	// sprintf("Battery charging fault (1-4): %d %d %d %d",
	// 	fault[0], fault[1], fault[2], fault[3]);

	// S3EventLogAdd(Msg, 3, -1, -1, 0);
#endif // TRIZEPS

	return 0;
}

// ----------------------------------------------------------------------------

int S3I2CChEn(unsigned char Ch, bool enable)
{
#ifdef TRIZEPS
	int pins = I2C_ReadRandom(S3I2C_EXPANDER_ADDR, 0x04);

	// enable = false;
	// Zero pin to enable
	if (enable)
	{
		if (Ch == 0)
		{
			pins &= ~EN_BAT_1;
		}
		else if (Ch == 1)
		{
			pins &= ~EN_BAT_2;
		}
		if (Ch == 2)
		{
			pins &= ~EN_BAT_3;
		}
		else if (Ch == 3)
		{
			pins &= ~EN_BAT_4;
		}
	}
	else
	{
		// One to disable
		if (Ch == 0)
		{
			pins |= EN_BAT_1;
		}
		else if (Ch == 1)
		{
			pins |= EN_BAT_2;
		}
		if (Ch == 2)
		{
			pins |= EN_BAT_3;
		}
		else if (Ch == 3)
		{
			pins |= EN_BAT_4;
		}
	}

	I2C_WriteRandom(S3I2C_EXPANDER_ADDR, 0x04, pins);

	/*if (CountPins(pins) > 1)
		return 0;*/

	//int pins0 = I2C_ReadRandom(S3I2C_EXPANDER_ADDR, 0x00);
	//int pins1 = I2C_ReadRandom(S3I2C_EXPANDER_ADDR, 0x01);

#endif // TRIZEPS
	return 0;
}

// ----------------------------------------------------------------------------

int S3I2CChargerInit(unsigned char Ch)
{
	return 0;
}

// ----------------------------------------------------------------------------
// FLAGS
// -----
// OTC:		Overtemperature in Charge condition is detected. True when set.
// OTD:		Overtemperature in Discharge condition is detected. True when set.
// BATHI:	Battery High bit that indicates a high battery voltage condition. Refer to the data flash Cell BH
// parameters for threshold settings. True when set.
// BATLOW:	Battery Low bit that indicates a low battery voltage condition. Refer to the data flash Cell BL
// parameters for threshold settings. True when set.
// CHG_INH:	Charge Inhibit: unable to begin charging. Refer to the data flash [Charge Inhibit Temp Low,
// Charge Inhibit Temp High] parameters for threshold settings. True when set.
// XCHG:	Charging not allowed.
// FC:		Full charge is detected. FC is set when charge termination is reached and FC Set% = –1
// (see Charging and Charge Termination Indication for details) or StateOfCharge() is larger
// than FC Set% and FC Set% is not –1. True when set.
// CHG:		(Fast) charging allowed. True when set.
// OCVTAKEN: Cleared on entry to RELAX mode and set to 1 when OCV measurement is performed in
// RELAX mode.
// CF:		Condition Flag indicates that the gauge needs to run through an update cycle to optimize
// accuracy.
// SOC1:	State-of-Charge Threshold 1 reached. True when set.
// SOCF:	State-of-Charge Threshold Final reached. True when set.
// DSG:		Discharging detected. True when set.
//
// FLAGSB
// ------
// SOH: StateOfHealth() calculation is active.
// LIFE: Indicates that LiFePO4 RELAX is enabled.
// FIRSTDOD: Set when RELAX mode is entered and then cleared upon valid DOD measurement for QMAX
// update or RELAX exit.
// DODEOC: DOD at End-of-Charge is updated.
// DTRC: Indicates RemainingCapacity() has been changed due to change in temperature.
//

// For some sanity, see:
// http://e2e.ti.com/support/power_management/battery_management/f/180/p/213060/820268#820268
//

int S2I2CChFactorySN();
int S2I2CChFactoryPN();

int S3I2CChGetStatus(unsigned char Ch)
{
#ifdef TRIZEPS
	S3TimerStart(1);

	S3I2CChMS(Ch);
	S3I2CChEn(Ch, true);
	
	unsigned char i2cStartAddr = 0x00;
	unsigned char i2cStdBufRead[32];
	unsigned char i2cCmdBufRead[2];

	// Try talking...
	BOOL ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, &i2cStartAddr, 1, i2cStdBufRead, 20);

	if (!ok)
	{
		S3I2CChMS(0xFF);

		// Still nobody at home
		if (S3Data->m_Chargers[Ch].m_Detected == false)
			return 0;

		// Somebody's just left the building
		S3Data->m_Chargers[Ch].m_Occupied = false;
		S3Data->m_Chargers[Ch].m_Detected = false;
		S3ChInit(Ch);
			
		return 1; // Indicate change
	}

	bool new_insert = (S3Data->m_Chargers[Ch].m_Detected == false);

	S3Data->m_Chargers[Ch].m_Occupied = true;
	S3Data->m_Chargers[Ch].m_Detected = true;

	S3ChSetSoC(Ch, i2cStdBufRead[0x02]);
	
	// x 0.1 deg K
	unsigned short temp;
	temp = *((short *)(i2cStdBufRead + 0x0c)) - 2730;
	S3ChSetBattTemp(Ch, temp);

	unsigned short t;
	t  = *((unsigned short *)(i2cStdBufRead + 0x08));
	S3ChSetBattV(Ch, (double)t / 1000.0);

	t  = *((unsigned short *)(i2cStdBufRead + 0x10));
	S3ChSetBattI(Ch, t);

	// TODO: This is a specific H/W fault condition
	//if (t == 0 && S3ChGetSoC(Ch) != 100)
	//	S3ChSetAlarm(Ch, S3_CH_NO_CHARGE_VOLTAGE);
	//else
	//	S3ChCancelAlarm(Ch, S3_CH_NO_CHARGE_VOLTAGE);

	i2cStartAddr = 0x1A; // 'Command' Average time to full
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, &i2cStartAddr, 1, i2cCmdBufRead, 2);

	unsigned short tmin = *((unsigned short *)i2cCmdBufRead);
	S3ChSetTimeToFull(Ch, tmin);

	// ---------------------------------------------------------------------------
	// TODO: Is there any danger in not polling every time?

	if (!new_insert)
	{
		// Attempt to clear tripped charger
		if ((S3ChGetAlarms(Ch) & S3_CH_CHARGE_FAULT) && S3ChBattValidated(Ch))
		{
			S3I2CChEn(Ch, false);
			Sleep(10);
			S3I2CChEn(Ch, true);
		}

		// Enable 12V supply if good
		if (!(S3ChGetAlarms(Ch) & S3_CH_BATT_HOT) && S3ChBattValidated(Ch))
			S3I2CChEn(Ch, true);
		// else
			// S3I2CChEn(Ch, false);

		// If we disable the charge current, we also lose comms (why?)
		// so invalidated battery comes and goes as supply is enabled
		// disabled - which is not ideal.
		// if (!S3ChBattValidated(Ch) && tmin < 60)
		//	S3I2CChEn(Ch, false);

 		S3I2CChMS(0xFF);
		return 0;
	}

	S3I2CChEn(Ch, true);

	i2cStartAddr = 0x00;

	unsigned char cmd[3] = {0, 0, 0};

	cmd[1] = 0x00; // Control status (2B)
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 3, NULL, 0);
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, &i2cStartAddr, 1, i2cCmdBufRead, 2);
	S3ChSetBattStatus(Ch, i2cCmdBufRead);

	// New discovery only
	if (1)
	{
		// S3I2CChSetBattSealed(Ch);
		// S3I2CChSetBattUnseal();
		// Sleep(100);
		// S3I2CChSetBattFullAccess();
		// Sleep(100);
		// S3I2CChReadSecKeys();
		// S3I2CChWriteSecKeys();
		int ChAuth = S3I2CChAuthenticate(Ch);
		S3ChSetBattValidated(Ch, ChAuth == 0);
	}

	char ver[S3_MAX_SW_VER_LEN];

	cmd[1] = 0x02; // FW
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 3, NULL, 0);
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, &i2cStartAddr, 1, i2cCmdBufRead, 2);
	sprintf_s(ver, S3_MAX_SW_VER_LEN, "%04x", *((unsigned short *)i2cCmdBufRead));
	S3ChSetBattFW(Ch, ver);

	cmd[1] = 0x03; // HW
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 3, NULL, 0);
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, &i2cStartAddr, 1, i2cCmdBufRead, 2);
	sprintf_s(ver, S3_MAX_SW_VER_LEN, "%04x", *((unsigned short *)i2cCmdBufRead));
	S3ChSetBattHW(Ch, ver);

	char SN[S3_MAX_SN_LEN], PN[S3_MAX_PN_LEN];
	S3I2CChReadSNPN(Ch, SN, PN);

	// Keep aligned with Tx battery functions (S3TxSetBattInfo())
	S3ChSetBattSN(Ch, SN);
	S3ChSetBattPN(Ch, PN);

	// Enable 12V supply if good
	if (S3ChBattValidated(Ch))
		S3I2CChEn(Ch, true);
	// else
	//	S3I2CChEn(Ch, false);

	S3I2CChMS(0xFF);

	S3TimerStop(1);
#endif // TRIZEPS

	return 0;
}

// ----------------------------------------------------------------------------

int S3I2CChReadSNPN(char Ch, char *SN, char *PN)
{
#ifdef TRIZEPS
	unsigned char cmd[3] = {0x00, 0x00, 0x00};
	BOOL ok;
	unsigned char i2cStartAddr = 0x00;
	unsigned char i2cStdBufRead[S3_FLASH_BLOCK_SIZE];

	// Get 'Manufacturer Info' block data

	if (!(S3ChGetBattStatus(Ch) & BQ_SS))
	{
		// Unsealed
		cmd[0] = 0x61;	// BlockDataControl
		cmd[1] = 0x01;	// Enable SEALED mode operation of DataFlashBlock

		ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

		if (!ok)
			return 1;

		cmd[0] = 0x3e;	// DataFlashClass
		cmd[1] = 58;	// Subclass 'Manufacturer Info'
		ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

		if (!ok)
			return 1;
	}

	cmd[0] = 0x3f;	// DataFlashBlock
	if (1)
	{	// Unsealed
		// if BlockDataControl (0x61) == 0x00
		//		0x00 transfer authentication data
		//		0x01 transfer manufacturer data
		// else if BlockDataControl (0x61) == 0x01
		//		WTF?
		cmd[1] = 0x01;	// Offset bank: 0: '0-31', 1: 32-63
	}
	else
	{
		// Sealed
		// if BlockDataControl (0x61) == 0x01
		//		Allows SEALED mode operation??
		//
		// 0x00 transfer authentication data
		// 0x01 transfer manufacturer data
		cmd[1] = 0x01;	// Offset bank: 0: '0-31', 1: 32-63
	}

	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

	if (!ok)
		return 1;

	i2cStartAddr = 0x40;
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR,
				&i2cStartAddr,	1,
				i2cStdBufRead,	S3_FLASH_BLOCK_SIZE);

	if (!ok)
		return 1;

	strcpy_s(SN, S3_MAX_SN_LEN, (char *)(i2cStdBufRead + 0));
	strcpy_s(PN, S3_MAX_PN_LEN, (char *)(i2cStdBufRead + S3_FLASH_SN_SIZE));

#endif

	return 0;
}


// ----------------------------------------------------------------------------

/*
int S3I2CChGetStatus(char Ch)
{

	return 0;
}
*/

// ----------------------------------------------------------------------------
// Gash...
extern int S3GPIOtest();
extern unsigned char i2cBuf[];

int S3I2CTest()
{

#ifdef TRIZEPS

	S3GPIOtest();
	return 0;

	int err;

	// err = S3I2CInit();
	// I2C_SetFastMode(0);
	// S3I2CChMS(3);
	I2C_Init();

	S3I2CChGetStatus(3);
	
	return 0;
	
	// Set up input pins (configuration) registers
	if (0)
	{
		// Set input pins
		I2C_WriteRandom(S3I2C_EXPANDER_ADDR, 0x0C, 0xC0);
		I2C_WriteRandom(S3I2C_EXPANDER_ADDR, 0x0D, 0x07);
		I2C_WriteRandom(S3I2C_EXPANDER_ADDR, 0x0E, 0x80);

		// Set outputs
		I2C_WriteRandom(S3I2C_EXPANDER_ADDR, 0x04, 0xFF);
		I2C_WriteRandom(S3I2C_EXPANDER_ADDR, 0x05, 0xFF & ~0x08 & ~0x10);
		I2C_WriteRandom(S3I2C_EXPANDER_ADDR, 0x06, 0xFF);
	}
	else
	{
		I2C_WriteRandom(S3I2C_EXPANDER_ADDR, 0x0C, 0xC0);
		I2C_WriteRandom(S3I2C_EXPANDER_ADDR, 0x0D, 0x07);
		I2C_WriteRandom(S3I2C_EXPANDER_ADDR, 0x0E, 0x80);

		// I2C_WriteBurst(S3I2C_EXPANDER_ADDR, i2cBufW, 0x00, 4);
		
		// I2C_WriteRead(S3I2C_EXPANDER_ADDR, i2cBufW, 4, NULL, 0);

		unsigned char i2cOPWrite[] = {0x84, 0xFF, 0xFF & ~0x08 & ~0x10, 0xFF};
		// unsigned char i2cOPWrite[] = {0x04, 0x10, 0x20, 0x30};

		I2C_WriteRead(S3I2C_EXPANDER_ADDR, i2cOPWrite, 4, NULL, 0);
	}

	Sleep(80);
	// I2C_Init();
	// err = I2C_ReadBurst(S3I2C_READ_ADDR, i2cBuf, 0, 255);
	// int val = I2C_ReadRandom(S3I2C_EXPANDER_ADDR, 0x04);
	
	// err = I2C_ReadBurst(S3I2C_EXPANDER_ADDRESS, i2cBuf, 0, 3);

	int pins1 = I2C_ReadRandom(S3I2C_EXPANDER_ADDR, 0x04);
	int pins2 = I2C_ReadRandom(S3I2C_EXPANDER_ADDR, 0x05);
	int pins3 = I2C_ReadRandom(S3I2C_EXPANDER_ADDR, 0x06);
	unsigned char i2cOPBufRead[3];
	// I2C_WriteRead(S3I2C_CH_BATT_ADDR, NULL, 0, i2cOPWrite, 1);
	I2C_WriteRead(S3I2C_EXPANDER_ADDR, NULL, 0, i2cOPBufRead, 3);

	int val1 = I2C_ReadRandom(S3I2C_EXPANDER_ADDR, 0x0C);
	int val2 = I2C_ReadRandom(S3I2C_EXPANDER_ADDR, 0x0D);
	int val3 = I2C_ReadRandom(S3I2C_EXPANDER_ADDR, 0x0E);

	// err = I2C_ReadBurst(S3I2C_EXPANDER_ADDR, i2cBuf, 0x0C, 3);

	int ErrCnt = 0;

	S3I2CChMS(3);
	S3I2CChEn(3, true);

	unsigned char i2cBufWrite[] = {0x10, 0x11};
	unsigned char i2cBufRead[2] = {0xFF, 0xFF};

	I2C_WriteRead(0xAA, i2cBufWrite, 2, NULL, 0);
	I2C_WriteRead(0xAA, NULL, 0, i2cBufRead, 2);

	unsigned char i2cVBufWrite[] = {0x08, 0x00};
	unsigned char i2cVBufRead[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

	I2C_WriteRead(S3I2C_CH_BATT_ADDR, i2cVBufWrite, 6, NULL, 0);
	I2C_WriteRead(S3I2C_CH_BATT_ADDR, NULL, 0, i2cVBufRead, 6);

	unsigned char i2cTBufWrite[] = {0x0c, 0x0d};
	unsigned char i2cTBufRead[2] = {0xFF, 0xFF};

	I2C_WriteRead(S3I2C_CH_BATT_ADDR, i2cTBufWrite, 2, NULL, 0);
	I2C_WriteRead(S3I2C_CH_BATT_ADDR, NULL, 0, i2cTBufRead, 2);

	// This doesn't work
	// I2C_WriteRead(S3I2C_BATTERY_ADDRESS, i2cTBufWrite, 2, i2cTBufRead, 2);

	unsigned char i2cSNBufWrite[] = {0x28, 0x29};
	unsigned char i2cSNBufRead[2] = {0xFF, 0xFF};

	I2C_WriteRead(S3I2C_CH_BATT_ADDR, i2cSNBufWrite, 2, NULL, 0);
	I2C_WriteRead(S3I2C_CH_BATT_ADDR, NULL, 0, i2cSNBufRead, 2);

	unsigned char i2cIntTBufWrite[] = {0x2a, 0x2b};
	unsigned char i2cIntTBufRead[2] = {0xFF, 0xFF};

	I2C_WriteRead(S3I2C_CH_BATT_ADDR, i2cIntTBufWrite, 2, NULL, 0);
	I2C_WriteRead(S3I2C_CH_BATT_ADDR, NULL, 0, i2cIntTBufRead, 2);

	unsigned char i2cCyclesBufWrite[] = {0x2c, 0x2d};
	unsigned char i2cCyclesBufRead[2] = {0xFF, 0xFF};

	I2C_WriteRead(S3I2C_CH_BATT_ADDR, i2cCyclesBufWrite, 2, NULL, 0);
	I2C_WriteRead(S3I2C_CH_BATT_ADDR, NULL, 0, i2cCyclesBufRead, 2);

	unsigned char i2cCapBufWrite[] = {0x3c, 0x3d};
	unsigned char i2cCapBufRead[2] = {0xFF, 0xFF};

	I2C_WriteRead(S3I2C_CH_BATT_ADDR, i2cCapBufWrite, 2, NULL, 0);
	I2C_WriteRead(S3I2C_CH_BATT_ADDR, NULL, 0, i2cCapBufRead, 2);

	unsigned char i2cFWBufWrite[] = {0x00, 0x01, 0x02, 0x00};
	unsigned char i2cFWBufRead[2] = {0xFF, 0xFF};

	I2C_WriteRead(S3I2C_CH_BATT_ADDR, i2cFWBufWrite, 4, NULL, 0);
	// I2C_WriteRead(S3I2C_CH_BATT_ADDR, NULL, 0, i2cFWBufRead, 2);

	i2cFWBufRead[0] = I2C_ReadRandom(S3I2C_CH_BATT_ADDR, 0x00);
	i2cFWBufRead[1] = I2C_ReadRandom(S3I2C_CH_BATT_ADDR, 0x01);

	unsigned char i2cHWBufWrite[] = {0x00, 0x01, 0x00, 0x00};
	unsigned char i2cHWBufRead[2] = {0xFF, 0xFF};

	// I2C_WriteRandom(S3I2C_CH_BATT_ADDR, 0x00, 0x00);
	// I2C_WriteRandom(S3I2C_CH_BATT_ADDR, 0x01, 0x02);

	I2C_WriteRead(S3I2C_CH_BATT_ADDR, i2cHWBufWrite, 4, NULL, 0);
	// I2C_WriteRead(S3I2C_CH_BATT_ADDR, i2cHWBufWrite + 2, 1, NULL, 0);
	Sleep(100);
	// I2C_WriteRead(S3I2C_CH_BATT_ADDR, NULL, 0, i2cHWBufRead, 2);

	i2cHWBufRead[0] = I2C_ReadRandom(S3I2C_CH_BATT_ADDR, 0x00);
	i2cHWBufRead[1] = I2C_ReadRandom(S3I2C_CH_BATT_ADDR, 0x01);

	int SoC = I2C_ReadRandom(S3I2C_CH_BATT_ADDR, 0x02);

	unsigned char i2cFlagsBufWrite[] = {0x0E, 0x0F};
	unsigned char i2cFlagsBufRead[2] = {0xFF, 0xFF};

	err = I2C_WriteRead(S3I2C_CH_BATT_ADDR, i2cFlagsBufWrite, 2, NULL, 0);
	I2C_WriteRead(S3I2C_CH_BATT_ADDR, NULL, 0, i2cFlagsBufRead, 2);

#endif

	return 0;
} // test


// ----------------------------------------------------------------------------
// Factory routine
//

int S3I2CChWriteSNPN(char Ch, const char *SN, const char *PN)
{
#ifdef TRIZEPS
	// char SNc[] =	{"Demo001"};
	// char PNc[] =	{"S3-BAT-2P-00"};

	if (strlen(SN) >= S3_FLASH_SN_SIZE)
		return 2;

	if (strlen(PN) >= S3_FLASH_PN_SIZE)
		return 2;

	if ((S3ChGetBattStatus(Ch) & BQ_SS))
		return 3;

	BOOL	ok;
	unsigned char cmd[32];
	unsigned char cmdSN[S3_FLASH_BLOCK_SIZE + 1];
	unsigned char i2cStartAddr = 0x00;

	// Unsealed
	cmd[0] = 0x61;	// BlockDataControl
	cmd[1] = 0x00;	// Access general data flash (not authentication data????)
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

	if (!ok)
		return 4;

	// Get 'Manufacturer Info' block data
	cmd[0] = 0x3e;	// DataFlashClass
	cmd[1] = 58;	// Subclass 'Manufacturer Info'
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

	cmd[0] = 0x3f;	// DataFlashBlock
	cmd[1] = 0x00;	// Offset bank: 0: '0-31', 1: 32-63
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

	unsigned char i;
	for(i = 0; i < S3_FLASH_BLOCK_SIZE + 1; i++)
		cmdSN[i] = 0;
	
	// SEALED access? Manufacturer data because unsealed and... fuck knows,,,
	// if set to 0x01 it shits on the authentication data? It doesn't work
	// and seals battery anyway.
	cmdSN[0] = 0x40; // Write data to offset 0x40 + offset
	cmdSN[1] = 0x00;

	for(i = 0; i < strlen(SN); i++)
		cmdSN[i + 1] = SN[i];

	for(i = 0; i < strlen(PN); i++)
		cmdSN[i + 1 + S3_FLASH_SN_SIZE] = PN[i];

	unsigned char len = 1 + strlen(PN) + S3_FLASH_SN_SIZE + 1;

	// Copy data then write to flash

	// Could write part of block but would need to read previous
	// block to calculate (or adjust) checksum.

	// Calculate checksum for the WHOLE block
	unsigned char CheckSum = 0;
	for(unsigned char i = 1; i < S3_FLASH_BLOCK_SIZE + 1; i++)
		CheckSum += cmdSN[i];

	CheckSum = 0xFF - CheckSum;

	// Write the WHOLE data block
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmdSN, S3_FLASH_BLOCK_SIZE + 1, NULL, 0);

	// Set checksum to force transfer to data flash
	cmd[0] = 0x60;
	cmd[1] = CheckSum;
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

	// Do reset
	cmd[0] = 0x00;
	cmd[1] = 0x41;
	cmd[2] = 0x00;
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 3, NULL, 0);

	Sleep(100);

	// Read back and check
	char SNrd[S3_MAX_SN_LEN], PNrd[S3_MAX_PN_LEN];
	if (!S3I2CChReadSNPN(Ch, SNrd, PNrd))
	{
		if (strcmp(SNrd, SN))
			return 1;

		if (strcmp(PNrd, PN))
			return 1;
	}
	else return 1;

#endif // TRIZEPS

	S3ChSetBattSN(Ch, SN);
	S3ChSetBattPN(Ch, PN);

	return 0;	
}

// ----------------------------------------------------------------------------
// This doesn't work - PN combined with serial number and flashed as one block.

int S2I2CChFactoryPN()
{
#ifdef TRIZEPS
	char PartNum[] =	{"S3-BAT-2P-00"};

	unsigned char i2cCmdBufRead[2];

	BOOL	ok;
	unsigned char cmd[32];
	unsigned char cmdSN[32];
	unsigned char i2cStartAddr = 0x00;

	// Get 'Manufacturer Info' block data
	cmd[0] = 0x3e;	// DataFlashClass
	cmd[1] = 58;	// Subclass 'Manufacturer Info'
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

	cmd[0] = 0x3f;	// DataFlashBlock
	cmd[1] = 0x00;	// Offset bank: 0: '0-31', 1: 32-63
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

	cmdSN[0] = 0x40 + S3_FLASH_SN_SIZE;
	cmdSN[1] = 0x00;
	strcat_s((char *)cmdSN, 32, PartNum);
	//ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, strlen((char *)cmd) + 1, NULL, 0);


	// WRITE_TO_FLASH TEST CODE

	// Write 0x52 to subclass 56 (manufacturer data), address 0x04 (+ 0x40)
	
	// Calculate checksum
	unsigned char CheckSum = 0;
	for(unsigned char i = 1; i < strlen((char *)cmdSN); i++)
		CheckSum += cmdSN[i];

	CheckSum = 0xFF - CheckSum;

	// Get checksum
	cmd[0] = 0x60;
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 1, i2cCmdBufRead, 1);

	// Write to data block
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmdSN, strlen((char *)cmdSN), NULL, 0);

	// Set checksum to force transfer to data flash
	cmd[0] = 0x60;
	cmd[1] = CheckSum;
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

	// Do reset
	cmd[0] = 0x00;
	cmd[1] = 0x41;
	cmd[2] = 0x00;
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 3, NULL, 0);

	// Keep aligned with Tx battery functions (S3TxSetBattInfo())
	// S3ChSetBattSN(Ch, (char *)(i2cStdBufRead + 0));
	// S3ChSetBattPN(Ch, (char *)(i2cStdBufRead + S3_FLASH_SN_SIZE));

#endif // TRIZEPS

	return 0;	
}

// ----------------------------------------------------------------------------

