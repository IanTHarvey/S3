// ----------------------------------------------------------------------------
//

#include "stdafx.h"

#include "S3DataModel.h"
#include "S3ControllerDlg.h"

extern pS3DataModel S3Data;

#ifdef TRIZEPS
#include "drvLib_app.h"
#else
#include <windows.h>
#endif

#include "S3I2C.h"
#include "S3Gain.h"

extern int S3I2CSetGain(char Rx);



#define S3_N_POLL_CYCLES	6
char S3PollCycle;

int	S3I2CPoll();
int	S3I2CPoll0();
int S3I2CPoll1();
int	S3I2CPoll2();
int	S3I2CPoll3();
extern int S3I2CGetRxStartUp(	char Rx);
extern int S3I2CGetTxStartUp(	char Rx, char Tx);
extern int S3I2CTxSetStatus(	char Rx, char Tx);

int S3I2CTxDoComp(		char Rx, char Tx);

int S3I2CRxMS(unsigned char Rx);

extern unsigned char	*pS3I2CRxReadBuf;
extern unsigned char	*pS3I2CTxReadBuf;

extern int	HexStr2Hex(BYTE *buf, char *str);
extern char	S3BattAuthKeyStr[];
extern BYTE	S3BattAuthKey[];

// ----------------------------------------------------------------------------
// BE (I2C 16-bit registers) to LE (Windows)
unsigned short S3RevByteUShort(unsigned char *b)
{
	unsigned short	s;
	unsigned char	c[2];
	
	*c = *(b + 1);
	*(c + 1) = *b;

	s = *((unsigned short *)c);

	return s;
}

// ----------------------------------------------------------------------------

short S3RevByteShort(unsigned char *b)
{
	short			s;
	unsigned char	c[2];
	
	*c = *(b + 1);
	*(c + 1) = *b;

	s = *((short *)c);

	return s;
}

// ----------------------------------------------------------------------------
// Convert trailing spaces to zero (required by some strings read from devices)
void TrailSpaceTo0(char *str, char n)
{
	n--;
	while(n > 0 && str[n] == ' ')
		str[n--] = '\0';
}

// ----------------------------------------------------------------------------
// Hit the normally exlusive MS3 and MS4 master-selects together to turn off
// main PSU output. No coming back.

void S3I2CSwitchOffImmediate()
{
#ifdef TRIZEPS
	S3EventLogAdd("Killing the system", 1, -1, -1, -1);
	I2C_WriteRandom(S3I2C_EXPANDER_ADDR, 0x04, MS_BAT_3 | MS_BAT_4);
#endif // TRIZEPS
}

#define S3_POWER_DOWN_TIMEOUT	20 // x Main poll interval
extern int S3PowerDownTimeout;

extern int S3CloseAppTimeout;

unsigned char S3Uptime = 0;

// ----------------------------------------------------------------------------

int S3I2CPoll(CS3ControllerDlg *parent)
{
	if (S3Uptime < UCHAR_MAX)
		S3Uptime++;

	if (S3Uptime == 1)
		S3GetLockFile();

	// Check for pending actions
	if (S3GetPowerDownFailed())
	{
		if (S3PowerDownTimeout > S3_POWER_DOWN_TIMEOUT)
		{
			S3EventLogAdd("System shutdown without shutting down all transmitters", 3, -1, -1, -1);
			S3I2CSwitchOffImmediate();
		}

		S3PowerDownTimeout++;

		return 0;
	}

	if (S3GetCloseAppFailed())
	{
		if (S3CloseAppTimeout > S3_POWER_DOWN_TIMEOUT)
		{
			S3EventLogAdd("App close without shutting down all transmitters", 3, -1, -1, -1);
			parent->AppShutdown();
			return -1;
		}

		S3CloseAppTimeout++;

		return 0;
	}

	if (S3GetCloseAppPending())
	{
		if (S3AllAsleep())
		{
			parent->AppShutdown();
			return -1;
		}

		S3CloseAppTimeout++;

		if (S3CloseAppTimeout > S3_POWER_DOWN_TIMEOUT)
		{
			S3EventLogAdd("App close: Tx sleep timed out", 3, -1, -1, -1);

			S3SetCloseAppFailed(true);
			S3CloseAppTimeout = 0;
		}
	}
	else if (S3GetPowerDownPending())
	{
		if (S3AllAsleep())
			S3I2CSwitchOffImmediate();

		S3PowerDownTimeout++;

		if (S3PowerDownTimeout > S3_POWER_DOWN_TIMEOUT)
		{
			S3EventLogAdd("System shutdown: Tx sleep timed out", 3, -1, -1, -1);

			S3SetPowerDownFailed(true);
			S3PowerDownTimeout = 0;
		}
	}
	else
	{
		// In case of mechanism to cancel power down
		S3PowerDownTimeout = 0;
	}

	// Do the polling
	if (!(S3PollCycle % 2))
	{
		// Priority 1 poll slot
		S3I2CPoll0();
	}
	else
	{
		// Priority 2 poll slots
		switch(S3PollCycle)
		{	
		case 1: S3I2CPoll1(); break;
		case 3: S3I2CPoll2(); break;
		case 5: S3I2CPoll3(); break;
		}
	}

	S3PollCycle++;

	if (S3PollCycle == S3_N_POLL_CYCLES)
		S3PollCycle = 0;

	return 0;
}

// ----------------------------------------------------------------------------
// Priority 1 poll
int	S3I2CPoll0()
{
	int Update = 0;
#ifdef TRIZEPS

	S3I2CGetPowerSwitch();
	
	if (!S3GetDemoMode())
	{
		for (char Rx = 0; Rx < S3_MAX_RXS; Rx++)
			Update += S3I2CRxGetStatus(Rx);
	}
	else
	{
		// Just get live data for Rx 1 in demo mode
		Update += S3I2CRxGetStatus(0);
	}

#endif
	return Update;
}

extern int S3I2CChGetFault();

// ----------------------------------------------------------------------------
// Priority 2 poll
int S3I2CPoll1()
{
	int Update = 0;
#ifdef TRIZEPS
	for(unsigned char Ch = 0; Ch < S3_N_CHARGERS; Ch++)
		Update += S3I2CChGetStatus(Ch);

	S3I2CChGetFault();
#endif
	return Update;
}

// ----------------------------------------------------------------------------

int	S3I2CPoll2()
{
	int Update = 0;
#ifdef TRIZEPS
	// Update = S3I2CUpdateSys();
#endif
	return Update;
}

// ----------------------------------------------------------------------------

int	S3I2CPoll3()
{
	int Update = 0;

	// Auto-save
	S3Save(NULL);

#ifdef TRIZEPS
	// for (unsigned char Ch = 0; Ch < S3_N_CHARGERS; Ch++)
	//	Update += S3I2CChGetStatus(Ch);
#endif
	return Update;
}

// ----------------------------------------------------------------------------

int S3I2CInit()
{
	int	err = 0;

	S3PollCycle = 0;

	pS3I2CTxReadBuf = S3I2CTxReadBuf;
	pS3I2CRxReadBuf = S3I2CRxReadBuf;

	HexStr2Hex(S3BattAuthKey, S3BattAuthKeyStr);

#ifdef TRIZEPS
	I2C_Init();

	S3I2CIOInit();

	// Enable by default, may be disabled later due to fault or failed validation
	for (unsigned char Ch = 0; Ch < S3_N_CHARGERS; Ch++)
		S3I2CChEn(Ch, true);

	// TODO: Do a start-up round of everything not polled (limits etc).
	// ...or just leave to polling
	if (!S3GetDemoMode())
	{
		/*
		for(char Rx = 0; Rx < S3_MAX_RXS; Rx++)
		{
			if (S3I2CGetRxStartUp(Rx))
				S3RxSetAlarm(Rx, S3_RX_INIT_FAIL);
		}
		*/
	}
#endif

	return err;
}

// ----------------------------------------------------------------------------

int S3I2CClose()
{
#ifdef TRIZEPS
	return !I2C_Free();
#else
	return 0;
#endif
}

// ----------------------------------------------------------------------------

int S3GetCtrlTemps(double Ta[])
{
#ifdef TRIZEPS
	int	t;

	// Only this one works on iPAN7
	t = Get_Temperature(TEMP_CPU);
	Ta[0] = (t != 0x7fffffff) ? (double)t / 1000.0 : -273.0;
	
	t = Get_Temperature(TEMP_PMIC);
	Ta[1] = (t != 0x7fffffff) ? (double)t / 1000.0 : -273.0;
	
	t = Get_Temperature(TEMP_TRIZEPS);
	Ta[2] = (t != 0x7fffffff) ? (double)t / 1000.0 : -273.0;

	// This uses the expander I2C and 0x9c to be written to bus
	// t = Get_Temperature(TEMP_BASEBOARD);
	// Ta[3] = (t != 0x7fffffff) ? (double)t / 1000.0 : -273.0;
#else
	Ta[0] = Ta[1] = Ta[2] = Ta[3] = -273.0;
#endif

	return 0;
}

// ----------------------------------------------------------------------------	

int S3I2CIOInit()
{

#ifdef TRIZEPS

	// Set Port 0 outputs to zero before assigning as outputs - otherwise outputs
	// default of 1's long enough to switch off rack.
	I2C_WriteRandom(S3I2C_EXPANDER_ADDR, 0x04, 0x00); // Zero MS_BAT_3 & MS_BAT_4
	I2C_WriteRandom(S3I2C_EXPANDER_ADDR, 0x06, CTRL_SHTDN); // Set CTRL_SHTDN hi 

	// Configure pins as inputs
	I2C_WriteRandom(S3I2C_EXPANDER_ADDR, 0x0C, 0xC0);	// Port 0
	I2C_WriteRandom(S3I2C_EXPANDER_ADDR, 0x0D, 0x07);	// Port 1
	I2C_WriteRandom(S3I2C_EXPANDER_ADDR, 0x0E, 0x80);	// Port 2

	S3EventLogAdd("Configured expander", 1, -1, -1, -1);

	unsigned char p1 = I2C_ReadRandom(S3I2C_EXPANDER_ADDR, 0x02);
	//unsigned char pins[3];
	//S3I2CIORead(pins);
	
#endif // TRIZEPS

	return 0;
}

// ----------------------------------------------------------------------------
// Read the input registers

int S3I2CIORead(unsigned char pins[])
{
#ifdef TRIZEPS
	pins[0] = I2C_ReadRandom(S3I2C_EXPANDER_ADDR, 0x00);
	pins[1] = I2C_ReadRandom(S3I2C_EXPANDER_ADDR, 0x01);
	pins[2] = I2C_ReadRandom(S3I2C_EXPANDER_ADDR, 0x02);

	// And do something
#endif // TRIZEPS
	return 0;
}

// ----------------------------------------------------------------------------
// Write to the output registers

int S3I2CIOWrite(unsigned char pins[])
{
#ifdef TRIZEPS
	I2C_WriteRandom(S3I2C_EXPANDER_ADDR, 0x04, pins[0]);
	I2C_WriteRandom(S3I2C_EXPANDER_ADDR, 0x05, pins[1]);
	I2C_WriteRandom(S3I2C_EXPANDER_ADDR, 0x06, pins[2]);
#endif // TRIZEPS
	return 0;
}

// ----------------------------------------------------------------------------
// Write Rx optical DSA attenuation 
int S3I2CRxSetCalibration(char Rx, char Tx, double val)
{
	if (S3RxGetType(Rx) == S3_RxEmpty)
		return -1;
	
	int err = S3I2CRxWriteCalValue(Rx, Tx, val);

	return err;
}

// ----------------------------------------------------------------------------
// Write individual calibration value (per RF path) for RF DSA
int S3I2CTxSetRFCalibration(char Rx, char Tx, char RFPath, double val)
{
	if (S3RxGetType(Rx) == S3_RxEmpty)
		return -1;

	if (S3TxGetType(Rx, Tx) == S3_TxUnconnected)
		return -2;

	int err = S3I2CTxWriteRFCalValue(Rx, Tx, RFPath, val);

	if (!err)
		S3TxSetCalRF(Rx, Tx, RFPath, (short)(val * 100.0));

	return err;
}

// ----------------------------------------------------------------------------
// Write Tx optical DSA attenuation 
int S3I2CTxSetOptCalibration(char Rx, char Tx, double val)
{
	if (S3RxGetType(Rx) == S3_RxEmpty)
		return -1;

	if (S3TxGetType(Rx, Tx) == S3_TxUnconnected)
		return -2;

	int err = S3I2CTxWriteOptCalValue(val);

	if (!err)
		S3TxSetCalOpt(Rx, Tx, (short)(val * 100.0));

	return err;
}

// ----------------------------------------------------------------------------
// Interrogate TCA6424A I2C expander I/O pin P2:7. Once latched, stays latched
// until I/F board power-cycled.

int Debounce = 0;

bool S3I2CGetPowerSwitch()
{
#ifdef TRIZEPS
	if (S3GetSoftShutdownOption())
	{
		if (S3GetPowerDownPending())
			return true;

		unsigned char Port2 = (unsigned char)I2C_ReadRandom(S3I2C_EXPANDER_ADDR, 0x02);

		if (!(Port2 & CTRL_SHTDN))
			Debounce++;
		else
			Debounce = 0;
	
		if (Debounce > 3)
		{
			S3SetPowerDownPending(true);
			S3SetSleepAll();

			return true;
		}
	}
#endif
	return false;
}

// ----------------------------------------------------------------------------

int S3I2CWriteLocalShort(	unsigned char	DevAddr,
							unsigned char	RegAddr,
							unsigned short	Data)
{
	BOOL ok = TRUE;

#ifdef TRIZEPS
	unsigned char wbuf[3];
	wbuf[0] = RegAddr;
	wbuf[1] = *((unsigned char *)&Data + 1);
	wbuf[2] = *((unsigned char *)&Data + 0);
		
	ok = I2C_WriteRead(DevAddr, wbuf, 3, NULL, 0);
#endif
	return ok != TRUE;
}

// ----------------------------------------------------------------------------