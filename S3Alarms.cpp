// ---------------------------------------------------------------------------
// Implementation of alarms for all devices

#include "windows.h"
// #include "stdafx.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "S3DataModel.h"
#include "S3GPIB.h"

extern pS3DataModel S3Data;



// ---------------------------------------------------------------------------

int S3CancelAlarms()
{
	char	Ch;

	for (Ch = 0; Ch < S3_N_CHARGERS; Ch++)
	{
		S3ChCancelAlarm(Ch, 0xFF);
	}

	for (char Rx = 0; Rx < S3_MAX_RXS; Rx++)
	{
		if (S3RxExistQ(Rx))
		{
			S3RxData	*pRx = &(S3Data->m_Rx[Rx]);

			for (char Tx = 0; Tx < S3_MAX_TXS; Tx++)
			{
				if (S3TxExistQ(Rx, Tx))
				{
					S3TxData	*pTx = &(pRx->m_Tx[Tx]);

					S3TxCancelAlarm(Rx, Tx, S3_TX_ALARMS_ALL);
					// S3_TX_BATT_WARN | S3_TX_BATT_ALARM);

					for (char IP = 0; IP < S3_MAX_IPS; IP++)
					{
						// TODO: Need valid test here?
						{
							S3IPCancelAlarm(Rx, Tx, IP, S3_ALARMS_ALL);
						}
					}
				}
			}
		}
	}

	return 0;
}

// ---------------------------------------------------------------------------
// S3TxOptSetAlarm & S3TxCtrlSetAlarm just flag alarms from the transmitter
// I2C memory maps. S3 doesn't 'handle' them.
//
// TODO: Handle all firmware alarms properly and where possible use them
// rather than the S3 controller detecting and raising it's own.
//

int S3TxOptSetAlarm(char Rx, char Tx, const unsigned char *alarms)
{
	int StateChange = 0;

	pS3TxData pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

	// iff peak alarm then ignore completely
	if (!S3TxGetPeakHoldCap(Rx, Tx))
	{
		if (alarms[1] == S3_TX_OPT_PEAK && alarms[2] == 0)
			return 0;
	}

	if (alarms[0] & S3_TX_OPT_MAJOR)
	{
		// Alarm already raised?
		if (!(pTx->m_OptAlarms[0] & S3_TX_OPT_MAJOR))
		{
			S3EventLogAdd("TxOpt: Major alarm raised", 3, Rx, Tx, -1);
			StateChange = 1;
		}
	}
	else
	{
		if (pTx->m_OptAlarms[0] & S3_TX_OPT_MAJOR)
		{
			S3EventLogAdd("TxOpt: Major alarm cleared", 1, Rx, Tx, -1);
			StateChange = 1;
		}
	}

	for(unsigned char i = 0; i < S3_TX_OPT_ALARM_BYTES; i++)
	{
		if (alarms[i] != pTx->m_OptAlarms[i])
		{
			char Msg[S3_EVENTS_LINE_LEN];

			sprintf_s(Msg, S3_EVENTS_LINE_LEN,
				"TxOpt[%d]: Alarm status change: 0x%02x to 0x%02x",
					i, pTx->m_OptAlarms[i], alarms[i]);

			S3EventLogAdd(Msg, 3, Rx, Tx, -1);

			pTx->m_OptAlarms[i] = alarms[i];
		}
	}

	return StateChange;
}

// ---------------------------------------------------------------------------

int S3TxCtrlSetAlarm(char Rx, char Tx, const unsigned char *alarms)
{
	int StateChange = 0;

	pS3TxData pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

	if (alarms[0] & S3_TX_CTRL_MAJOR)
	{
		// Alarm already raised?
		if (!(pTx->m_CtrlAlarms[0] & S3_TX_CTRL_MAJOR))
		{
			S3EventLogAdd("TxCtrl: Major alarm raised", 1, Rx, Tx, -1);
			// pTx->m_CtrlAlarms[0] |= S3_TX_OPT_MAJOR;
			StateChange = 1;
		}
	}
	else
	{
		if (pTx->m_CtrlAlarms[0] & S3_TX_CTRL_MAJOR)
		{
			S3EventLogAdd("TxCtrl: Major alarm cleared", 1, Rx, Tx, -1);
			// pTx->m_OptAlarms[0] |= S3_TX_OPT_MAJOR;
			StateChange = 1;
		}
	}

	for(unsigned char i = 0; i < S3_TX_CTRL_ALARM_BYTES; i++)
	{
		if (alarms[i] != pTx->m_CtrlAlarms[i])
		{
			char Msg[S3_EVENTS_LINE_LEN];

			sprintf_s(Msg, S3_EVENTS_LINE_LEN,
				"TxCtrl: Alarm status change: %x to %x",
					pTx->m_OptAlarms[i], alarms[i]);

			S3EventLogAdd(Msg, 3, Rx, Tx, -1);

			pTx->m_CtrlAlarms[i] = alarms[i];
		}
	}

	return StateChange;
}
// ----------------------------------------------------------------------------

int S3TxSetAlarm(char Rx, char Tx, unsigned short alarms)
{
	int StateChange = 0;

	pS3TxData pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];
	
	// Alarm requested
	if (alarms & S3_TX_RLL_UNSTABLE)
	{
		// Alarm already raised?
		if (!(pTx->m_Alarms & S3_TX_RLL_UNSTABLE))
		{
			pTx->m_Alarms |= S3_TX_RLL_UNSTABLE;
			StateChange = 1;
		}
	}

	if (alarms & S3_TX_BATT_INVALID)
	{
		if (!(pTx->m_Alarms & S3_TX_BATT_INVALID))
		{
			S3EventLogAdd("Transmitter battery unvalidated", 1, Rx, Tx, -1);
			pTx->m_Alarms |= S3_TX_BATT_INVALID;
			StateChange = 1;
		}
	}

	if (alarms & S3_TX_COMM_FAIL)
	{
		if (!(pTx->m_Alarms & S3_TX_COMM_FAIL))
		{
			S3EventLogAdd("Transmitter communication failed", 1, Rx, Tx, -1);
			pTx->m_Alarms |= S3_TX_COMM_FAIL;
			StateChange = 1;
		}
	}

	if (alarms & S3_TX_TEMP_COMM_FAIL)
	{
		if (!(pTx->m_Alarms & S3_TX_TEMP_COMM_FAIL))
		{
			S3EventLogAdd("Transmitter temperature communication failed", 1, Rx, Tx, -1);
			pTx->m_Alarms |= S3_TX_TEMP_COMM_FAIL;
			StateChange = 1;
		}
	}

	if (alarms & S3_TX_BATT_WARN)
	{
		if (!(pTx->m_Alarms & S3_TX_BATT_WARN))
		{
			S3EventLogAdd("Tx battery low warning raised", 1, Rx, Tx, -1);
			pTx->m_Alarms |= S3_TX_BATT_WARN;
			StateChange = 1;
		}
	}

	if (alarms & S3_TX_BATT_ALARM)
	{
		if (!(pTx->m_Alarms & S3_TX_BATT_ALARM))
		{
			S3EventLogAdd("Tx battery low alarm raised", 3, Rx, Tx, -1);
			pTx->m_Alarms |= S3_TX_BATT_ALARM;
			StateChange = 1;
		}
	}

	if (alarms & S3_TX_RECOMP_REQ)
	{
		if (!(pTx->m_Alarms & S3_TX_RECOMP_REQ))
		{
			S3EventLogAdd("Tx temperature compensation required", 1, Rx, Tx, -1);
			pTx->m_Alarms |= S3_TX_RECOMP_REQ;
			StateChange = 1;
		}
	}

	if (alarms & S3_TX_SELF_TEST_FAIL)
	{
		if (!(pTx->m_Alarms & S3_TX_SELF_TEST_FAIL))
		{
			S3EventLogAdd("Tx self-test failed", 3, Rx, Tx, -1);
			pTx->m_Alarms |= S3_TX_SELF_TEST_FAIL;
			StateChange = 1;
		}
	}

	if (alarms & S3_TX_SELF_TEST_NOT_RUN)
	{
		if (!(pTx->m_Alarms & S3_TX_SELF_TEST_NOT_RUN))
		{
			S3EventLogAdd("Tx self-test did not complete", 3, Rx, Tx, -1);
			pTx->m_Alarms |= S3_TX_SELF_TEST_NOT_RUN;
			StateChange = 1;
		}
	}

	if (alarms & S3_TX_SELF_TEST_RETRY)
	{
		if (!(pTx->m_Alarms & S3_TX_SELF_TEST_RETRY))
		{
			S3EventLogAdd("Tx self-test abandoned after retries", 3, Rx, Tx, -1);
			pTx->m_Alarms |= S3_TX_SELF_TEST_RETRY;
			StateChange = 1;
		}
	}
	
	if (alarms & S3_TX_INIT_FAIL)
	{
		if (!(pTx->m_Alarms & S3_TX_INIT_FAIL))
		{
			S3EventLogAdd("Tx initialization failed", 3, Rx, Tx, -1);
			pTx->m_Alarms |= S3_TX_INIT_FAIL;
			StateChange = 1;
		}
	}

	if (alarms & S3_TX_OVER_TEMP)
	{
		if (!(pTx->m_Alarms & S3_TX_OVER_TEMP))
		{
			S3EventLogAdd("Tx over temperature limit", 3, Rx, Tx, -1);
			pTx->m_Alarms |= S3_TX_OVER_TEMP;
			StateChange = 1;
		}
	}

	if (alarms & S3_TX_UNDER_TEMP)
	{
		if (!(pTx->m_Alarms & S3_TX_UNDER_TEMP))
		{
			S3EventLogAdd("Tx under temperature limit", 3, Rx, Tx, -1);
			pTx->m_Alarms |= S3_TX_UNDER_TEMP;
			StateChange = 1;
		}
	}

	return StateChange;
}

// ---------------------------------------------------------------------------
// Return highest priority alarm
// TODO: This is ugly and difficult to maintain

int S3TxAlarmGetString(char Rx, char Tx, char *S3AlarmString, int len)
{
	pS3TxData pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];
	pTx->m_CurAlarmSrc = -1;

	if (pTx->m_Alarms & S3_TX_RLL_UNSTABLE)
	{
		strcpy_s(S3AlarmString, len, "I:RLL stabilising. Please wait");
		pTx->m_CurAlarmSrc = 0;
		pTx->m_CurAlarm = pTx->m_Alarms;
		return 1;
	}

	if (pTx->m_Alarms & S3_TX_COMM_FAIL)
	{
		strcpy_s(S3AlarmString, len, "E:Transmitter communications failed");
		pTx->m_CurAlarmSrc = 0;
		pTx->m_CurAlarm = pTx->m_Alarms;
		return 1;
	}

	if (pTx->m_Alarms & S3_TX_TEMP_COMM_FAIL)
	{
		strcpy_s(S3AlarmString, len, "E:Transmitter temperature communications failed");
		pTx->m_CurAlarmSrc = 0;
		pTx->m_CurAlarm = pTx->m_Alarms;
		return 1;
	}
	
	if (pTx->m_Alarms & S3_TX_INIT_FAIL)
	{
		strcpy_s(S3AlarmString, len, "E:Tx initialisation failed. Please retry.");
		pTx->m_CurAlarmSrc = 0;
		pTx->m_CurAlarm = pTx->m_Alarms;
		return 1;
	}

	if (pTx->m_Alarms & S3_TX_OVER_TEMP)
	{
		strcpy_s(S3AlarmString, len, "E:Tx over-temperature. Entering sleep mode");
		pTx->m_CurAlarmSrc = 0;
		pTx->m_CurAlarm = pTx->m_Alarms;
		return 1;
	}

	if (pTx->m_Alarms & S3_TX_BATT_ALARM)
	{
		strcpy_s(S3AlarmString, len, "E:Battery exhausted");
		pTx->m_CurAlarmSrc = 0;
		pTx->m_CurAlarm = pTx->m_Alarms;
		return 1;
	}

	if (pTx->m_Alarms & S3_TX_BATT_WARN)
	{
		strcpy_s(S3AlarmString, len, "W:Battery low");
		return 1;
	}

	//if (pTx->m_OptAlarms[0] & S3_TX_OPT_MAJOR)
	//{
	//	strcpy_s(S3AlarmString, len, "E:Unspecified major alarm");
	//	pTx->m_CurAlarmSrc = 1;
	//	pTx->m_CurAlarm = S3_TX_OPT_MAJOR;
	//	return 1;
	//}

	if (pTx->m_Alarms & S3_TX_SELF_TEST_FAIL)
	{
		strcpy_s(S3AlarmString, len, "E:Tx self-test failed");
		pTx->m_CurAlarmSrc = 0;
		pTx->m_CurAlarm = S3_TX_SELF_TEST_FAIL;
		return 1;
	}

	if (pTx->m_Alarms & S3_TX_SELF_TEST_NOT_RUN)
	{
		strcpy_s(S3AlarmString, len, "I:Tx self-test failed to run");
		pTx->m_CurAlarmSrc = 0;
		pTx->m_CurAlarm = S3_TX_SELF_TEST_NOT_RUN;
		return 1;
	}

	if (pTx->m_Alarms & S3_TX_SELF_TEST_RETRY)
	{
		strcpy_s(S3AlarmString, len, "I:Tx self-test abandoned after retries");
		pTx->m_CurAlarmSrc = 0;
		pTx->m_CurAlarm = S3_TX_SELF_TEST_RETRY;
		return 1;
	}

	if (pTx->m_Alarms & S3_TX_RECOMP_REQ)
	{
		strcpy_s(S3AlarmString, len, "W:Temperature re-compensation required");
		pTx->m_CurAlarmSrc = 0;
		pTx->m_CurAlarm = S3_TX_RECOMP_REQ;
		return 1;
	}

	if (pTx->m_Alarms & S3_TX_LASER_HI)
	{
		strcpy_s(S3AlarmString, len, "E:Laser power high");
		pTx->m_CurAlarmSrc = 0;
		pTx->m_CurAlarm = S3_TX_LASER_HI;
		return 1;
	}

	if (pTx->m_Alarms & S3_TX_LASER_LO)
	{
		strcpy_s(S3AlarmString, len, "E:Laser power low");
		pTx->m_CurAlarmSrc = 0;
		pTx->m_CurAlarm = S3_TX_LASER_LO;
		return 1;
	}

	if (pTx->m_Alarms) // Catch-all
	{
		sprintf_s(S3AlarmString, len, "I:TxAlarm:0x%04x", pTx->m_Alarms);
		pTx->m_CurAlarmSrc = 0;
		pTx->m_CurAlarm = pTx->m_Alarms;
		return 1;
	}

	if (pTx->m_BattAlarms)
	{
		sprintf_s(S3AlarmString, len, "I:TxBattAlarm: 0x%02x", pTx->m_BattAlarms);
		pTx->m_CurAlarmSrc = 6;
		pTx->m_CurAlarm = pTx->m_BattAlarms;
		return 1;
	}

	if (pTx->m_OptAlarms[0] & S3_TX_OPT_MAJOR)
	{
		// Is alarm in primary byte?
		if (pTx->m_OptAlarms[0] & ~(S3_TX_OPT_MINOR | S3_TX_OPT_MAJOR))
		{
			pTx->m_CurAlarmSrc = 1;

			if (pTx->m_OptAlarms[0] & S3_TX_OPT_LASER)
			{
				sprintf_s(S3AlarmString, len, "E:Laser not started. Try sleep-cycle", pTx->m_OptAlarms[0]);
				pTx->m_CurAlarm = S3_TX_OPT_LASER;
				return 1;
			}

			if (pTx->m_OptAlarms[0] & S3_TX_OPT_AGC)
			{
				sprintf_s(S3AlarmString, len, "E:AGC alarm 0x%02x", pTx->m_OptAlarms[0]);
				pTx->m_CurAlarm = S3_TX_OPT_AGC;
				return 1;
			}

			if (pTx->m_OptAlarms[0] & S3_TX_OPT_TEC)
			{
				sprintf_s(S3AlarmString, len, "E:TEC alarm 0x%02x", pTx->m_OptAlarms[0]);
				pTx->m_CurAlarm = S3_TX_OPT_TEC;
				return 1;
			}
		}

		// Look for alarms in secondary bytes
		if (pTx->m_OptAlarms[1] & S3_TX_OPT_PEAK)
		{
			// Ignore if not in Tx H/W & F/W
			if (S3TxGetPeakHoldCap(Rx, Tx))
			{
				sprintf_s(S3AlarmString, len, "W:Peak detection alarm");
				pTx->m_CurAlarmSrc = 2;
				pTx->m_CurAlarm = S3_TX_OPT_PEAK;
			}
			else
			{
				sprintf_s(S3AlarmString, len, "I:Bogus peak detection alarm");
				pTx->m_CurAlarmSrc = 2;
				pTx->m_CurAlarm = S3_TX_OPT_PEAK;
			}

			return 1;
		}
		
		for(char i = 1; i < 3; i++)
		{
			if (pTx->m_OptAlarms[i])
			{
				sprintf_s(S3AlarmString, len, "E:TxOptAlarm[%d]: 0x%02x",
					i, pTx->m_OptAlarms[i]);

				pTx->m_CurAlarmSrc = i + 1;
				pTx->m_CurAlarm = pTx->m_OptAlarms[i];

				return 1;
			}
		}
	}

	if (pTx->m_OptAlarms[0] & S3_TX_OPT_MINOR)
	{
		if (pTx->m_OptAlarms[0] & ~(S3_TX_OPT_MINOR | S3_TX_OPT_MAJOR))
		{
			sprintf_s(S3AlarmString, len, "I:Minor Alarm: 0x%02x",
			pTx->m_OptAlarms[0]);

			pTx->m_CurAlarmSrc = 1;
			pTx->m_CurAlarm = S3_TX_OPT_MINOR;
			return 1;
		}

		for(char i = 1; i < 3; i++)
		{
			if (pTx->m_OptAlarms[i])
			{
				sprintf_s(S3AlarmString, len, "I:TxOptAlarm[%d]: 0x%02x",
					i, pTx->m_OptAlarms[i]);

				pTx->m_CurAlarmSrc = i + 1;
				pTx->m_CurAlarm = pTx->m_OptAlarms[i];

				return 1;
			}
		}
	}

	if (pTx->m_CtrlAlarms[0] & S3_TX_CTRL_MAJOR)
	{
		for(char i = 0; i < 2; i++)
		{
			if (pTx->m_CtrlAlarms[i])
			{
				sprintf_s(S3AlarmString, len, "I:TxCtrlAlarm[%d]: 0x%02x",
					i, pTx->m_CtrlAlarms[i]);

				pTx->m_CurAlarmSrc = i + 4;
				pTx->m_CurAlarm = pTx->m_CtrlAlarms[i];
				return 1;
			}
		}
	}

	return 0;
}

// ---------------------------------------------------------------------------
// Cancel local alarms, if remote alarms, alarm will just re-appear.

int S3TxCancelCurAlarm(char Rx, char Tx)
{
	pS3TxData pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

	if (pTx->m_CurAlarmSrc == -1)
		return 0;

	switch(pTx->m_CurAlarmSrc)
	{
	case 0: S3TxCancelAlarm(Rx, Tx, pTx->m_CurAlarm); break;
	case 1: 
	case 2: 
	case 3: 
		{
			unsigned char clr[3] = {0, 0, 0};
			S3TxOptSetAlarm(Rx, Tx, clr); break;
		}
	case 4: 
	case 5: 
		{
			unsigned char clr[3] = {0, 0, 0};
			S3TxCtrlSetAlarm(Rx, Tx, clr); break;
		}
	case 6: S3TxBattCancelAlarm(Rx, Tx, (unsigned char)pTx->m_CurAlarm); break;
	}

	pTx->m_CurAlarmSrc = -1;

	return 0;
}

// ---------------------------------------------------------------------------

int S3TxCancelAlarm(char Rx, char Tx, unsigned short alarms)
{
	int StateChange = 0;

	pS3TxData pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

	if (alarms & S3_TX_RLL_UNSTABLE)
	{
		if (pTx->m_Alarms & S3_TX_RLL_UNSTABLE)
		{
			pTx->m_Alarms &= ~S3_TX_RLL_UNSTABLE;
			StateChange = 1;
		}
	}

	if (alarms & S3_TX_BATT_INVALID)
	{
		if (pTx->m_Alarms & S3_TX_BATT_INVALID)
		{
			S3EventLogAdd("Tx battery invalid cancelled", 1, Rx, Tx, -1);
			pTx->m_Alarms &= ~S3_TX_BATT_INVALID;
			StateChange = 1;
		}
	}

	if (alarms & S3_TX_COMM_FAIL)
	{
		if (pTx->m_Alarms & S3_TX_COMM_FAIL)
		{
			S3EventLogAdd("Transmitter re-acquired", 1, Rx, Tx, -1);
			pTx->m_Alarms &= ~S3_TX_COMM_FAIL;
			StateChange = 1;
		}
	}

	if (alarms & S3_TX_TEMP_COMM_FAIL)
	{
		if (pTx->m_Alarms & S3_TX_TEMP_COMM_FAIL)
		{
			S3EventLogAdd("Transmitter temperature re-acquired", 1, Rx, Tx, -1);
			pTx->m_Alarms &= ~S3_TX_TEMP_COMM_FAIL;
			StateChange = 1;
		}
	}

	if (alarms & S3_TX_BATT_WARN)
	{
		if (pTx->m_Alarms & S3_TX_BATT_WARN)
		{
			S3EventLogAdd("Tx battery low warning cancelled", 1, Rx, Tx, -1);
			pTx->m_Alarms &= ~S3_TX_BATT_WARN;
			StateChange = 1;
		}
	}

	if (alarms & S3_TX_BATT_ALARM)
	{
		if (pTx->m_Alarms & S3_TX_BATT_ALARM)
		{
			S3EventLogAdd("Tx battery low alarm cancelled", 1, Rx, Tx, -1);
			pTx->m_Alarms &= ~S3_TX_BATT_ALARM;
			StateChange = 1;
		}
	}

	if (alarms & S3_TX_RECOMP_REQ)
	{
		if (pTx->m_Alarms & S3_TX_RECOMP_REQ)
		{
			S3EventLogAdd("Tx temperature compensation no longer required", 1, Rx, Tx, -1);
			pTx->m_Alarms &= ~S3_TX_RECOMP_REQ;
			StateChange = 1;
		}
	}

	if (alarms & S3_TX_SELF_TEST_FAIL)
	{
		if (pTx->m_Alarms & S3_TX_SELF_TEST_FAIL)
		{
			S3EventLogAdd("Tx self-test cancelled", 1, Rx, Tx, -1);
			pTx->m_Alarms &= ~S3_TX_SELF_TEST_FAIL;
			StateChange = 1;
		}
	}

	if (alarms & S3_TX_SELF_TEST_NOT_RUN)
	{
		if (pTx->m_Alarms & S3_TX_SELF_TEST_NOT_RUN)
		{
			S3EventLogAdd("Tx self-test run failure cancelled", 1, Rx, Tx, -1);
			pTx->m_Alarms &= ~S3_TX_SELF_TEST_NOT_RUN;
			StateChange = 1;
		}
	}

	if (alarms & S3_TX_SELF_TEST_RETRY)
	{
		if (pTx->m_Alarms & S3_TX_SELF_TEST_RETRY)
		{
			S3EventLogAdd("Tx self-test abandoned cancelled", 1, Rx, Tx, -1);
			pTx->m_Alarms &= ~S3_TX_SELF_TEST_RETRY;
			StateChange = 1;
		}
	}

	if (alarms & S3_TX_INIT_FAIL)
	{
		if (pTx->m_Alarms & S3_TX_INIT_FAIL)
		{
			S3EventLogAdd("Tx reinitialisation succeeded", 1, Rx, Tx, -1);
			pTx->m_Alarms &= ~S3_TX_INIT_FAIL;
			StateChange = 1;
		}
	}

	if (alarms & S3_TX_OVER_TEMP)
	{
		if (pTx->m_Alarms & S3_TX_OVER_TEMP)
		{
			S3EventLogAdd("Tx over-temperature cancelled", 1, Rx, Tx, -1);
			pTx->m_Alarms &= ~S3_TX_OVER_TEMP;
			StateChange = 1;
		}
	}

	if (alarms & S3_TX_UNDER_TEMP)
	{
		if (pTx->m_Alarms & S3_TX_UNDER_TEMP)
		{
			S3EventLogAdd("Tx under-temperature cancelled", 1, Rx, Tx, -1);
			pTx->m_Alarms &= ~S3_TX_UNDER_TEMP;
			StateChange = 1;
		}
	}

	return StateChange;
}

// ---------------------------------------------------------------------------

int S3TxBattSetAlarm(char Rx, char Tx, unsigned char alarms)
{
	int StateChange = 0;

	pS3TxData pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];
	
	// Alarm requested
	if (alarms & S3_TX_BATT_COMM_FAIL)
	{
		// Alarm already raised?
		if (!(pTx->m_BattAlarms & S3_TX_BATT_COMM_FAIL))
		{
			S3EventLogAdd("Tx battery comms failed", 3, Rx, Tx, -1);
			pTx->m_BattAlarms |= S3_TX_BATT_COMM_FAIL;
			StateChange = 1;
		}
	}

	if (alarms & S3_TX_BATT_COLD)
	{
		if (!(pTx->m_BattAlarms & S3_TX_BATT_COLD))
		{
			S3EventLogAdd("Tx battery cold alarm raised", 3, Rx, Tx, -1);
			pTx->m_BattAlarms |= S3_TX_BATT_COLD;
			StateChange = 1;
		}
	}
	
	if (alarms & S3_TX_BATT_HOT)
	{
		if (!(pTx->m_BattAlarms & S3_TX_BATT_HOT))
		{
			S3EventLogAdd("Tx battery hot alarm raised", 3, Rx, Tx, -1);
			pTx->m_BattAlarms |= S3_TX_BATT_HOT;
			StateChange = 1;
		}
	}

	return StateChange;
}

// ---------------------------------------------------------------------------

int S3TxBattCancelAlarm(char Rx, char Tx, unsigned char alarms)
{
	int StateChange = 0;

	pS3TxData pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

	if (alarms & S3_TX_BATT_COMM_FAIL)
	{
		// Alarm actually raised?
		if (pTx->m_BattAlarms & S3_TX_BATT_COMM_FAIL)
		{
			S3EventLogAdd("Tx battery comms fail cancelled", 3, Rx, Tx, -1);
			pTx->m_BattAlarms &= ~S3_TX_BATT_COMM_FAIL;
			StateChange = 1;
		}
	}

	if (alarms & S3_TX_BATT_HOT)
	{
		if (pTx->m_BattAlarms & S3_TX_BATT_HOT)
		{
			S3EventLogAdd("Tx battery hot cancelled", 1, Rx, Tx, -1);
			pTx->m_BattAlarms &= ~S3_TX_BATT_HOT;
			StateChange = 1;
		}
	}

	if (alarms & S3_TX_BATT_COLD)
	{
		if (pTx->m_BattAlarms & S3_TX_BATT_COLD)
		{
			S3EventLogAdd("Tx battery cold cancelled", 1, Rx, Tx, -1);
			pTx->m_BattAlarms &= ~S3_TX_BATT_COLD;
			StateChange = 1;
		}
	}

	return StateChange;
}
// ---------------------------------------------------------------------------

int S3IPSetAlarm(char Rx, char Tx, char IP, unsigned char alarm)
{
	int StateChange = 0;

	pS3IPData pIp = &S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP];

	if (alarm & S3_IP_TEST_FAIL)
	{
		if (!(pIp->m_Alarms & S3_IP_TEST_FAIL))
		{
			S3EventLogAdd("Input selector test fail", 1, Rx, Tx, IP);
			pIp->m_Alarms |= S3_IP_TEST_FAIL;
			StateChange = 1;
		}
	}

	if (alarm & S3_IP_OVERDRIVE)
	{
		if (!(pIp->m_Alarms & S3_IP_OVERDRIVE))
		{
			S3EventLogAdd("Input overdrive alarm", 1, Rx, Tx, IP);
			pIp->m_Alarms |= S3_IP_OVERDRIVE;
			StateChange = 1;
		}
	}

	return StateChange;
}

// ---------------------------------------------------------------------------

int S3IPCancelAlarm(char Rx, char Tx, char IP, unsigned char alarms)
{
	int StateChange = 0;

	pS3IPData pIp = &S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP];

	// Handle one at a time (if there's more than one)
	if (alarms & S3_IP_TEST_FAIL)
	{
		if (pIp->m_Alarms & S3_IP_TEST_FAIL)
		{
			S3EventLogAdd("Input selector test fail cancelled", 1, Rx, Tx, IP);
			pIp->m_Alarms &= ~S3_IP_TEST_FAIL;
			StateChange = 1;
		}
	}

	if (alarms & S3_IP_OVERDRIVE)
	{
		if (pIp->m_Alarms & S3_IP_OVERDRIVE)
		{
			S3EventLogAdd("Input overdrive alarm cancelled", 1, Rx, Tx, IP);
			pIp->m_Alarms &= ~S3_IP_OVERDRIVE;
			StateChange = 1;
		}
	}


	return StateChange;
}

// ---------------------------------------------------------------------------

int S3ChSetAlarm(char Ch, unsigned char alarms)
{
	int StateChange = 0;

	pS3Charger pCh = &S3Data->m_Chargers[Ch];

	// These should be exclusive, but generated by charger, so just report
	if (alarms & S3_CH_BATT_COLD)
	{
		if (!(pCh->m_Alarms & S3_CH_BATT_COLD))
		{
			S3EventLogAdd("Charger battery cold alarm raised", 3, -1, -1, Ch);
			pCh->m_Alarms |= S3_CH_BATT_COLD;
			StateChange = 1;
		}
	}
	
	if (alarms & S3_CH_BATT_HOT)
	{
		if (!(pCh->m_Alarms & S3_CH_BATT_HOT))
		{
			S3EventLogAdd("Charger battery hot alarm raised", 3, -1, -1, Ch);
			pCh->m_Alarms |= S3_CH_BATT_HOT;
			StateChange = 1;
		}
	}
	
	if (alarms & S3_CH_BATT_INVALID)
	{
		if (!(pCh->m_Alarms & S3_CH_BATT_INVALID))
		{
			S3EventLogAdd("Attempt to charge non-validated battery",
				3, -1, -1, Ch);
			pCh->m_Alarms |= S3_CH_BATT_INVALID;
			StateChange = 1;
		}
	}

	if (alarms & S3_CH_NO_CHARGE_VOLTAGE)
	{
		if (!(pCh->m_Alarms & S3_CH_NO_CHARGE_VOLTAGE))
		{
			S3EventLogAdd("No battery charge supply",
				3, -1, -1, Ch);
			pCh->m_Alarms |= S3_CH_NO_CHARGE_VOLTAGE;
			StateChange = 1;
		}
	}

	if (alarms & S3_CH_CHARGE_FAULT)
	{
		if (!(pCh->m_Alarms & S3_CH_CHARGE_FAULT))
		{
			S3EventLogAdd("Battery charge fault",
				3, -1, -1, Ch);
			pCh->m_Alarms |= S3_CH_CHARGE_FAULT;
			StateChange = 1;
		}
	}

	return StateChange;
}

// ---------------------------------------------------------------------------

int S3ChCancelAlarm(char Ch, unsigned char alarms)
{
	int StateChange = 0;

	pS3Charger pCh = &S3Data->m_Chargers[Ch];

	if (alarms & S3_CH_BATT_HOT)
	{
		if (pCh->m_Alarms & S3_CH_BATT_HOT)
		{
			S3EventLogAdd("Charger battery hot cancelled", 1, -1, -1, Ch);
			pCh->m_Alarms &= ~S3_CH_BATT_HOT;
			StateChange = 1;
		}
	}

	if (alarms & S3_CH_BATT_COLD)
	{
		if (pCh->m_Alarms & S3_CH_BATT_COLD)
		{
			S3EventLogAdd("Charger battery cold cancelled", 1, -1, -1, Ch);
			pCh->m_Alarms &= ~S3_CH_BATT_COLD;
			StateChange = 1;
		}
	}

	if (alarms & S3_CH_BATT_INVALID)
	{
		if (pCh->m_Alarms & S3_CH_BATT_INVALID)
		{
			S3EventLogAdd("Non-validated battery removed", 1, -1, -1, Ch);
			pCh->m_Alarms &= ~S3_CH_BATT_INVALID;
			StateChange = 1;
		}
	}

	if (alarms & S3_CH_NO_CHARGE_VOLTAGE)
	{
		if (pCh->m_Alarms & S3_CH_NO_CHARGE_VOLTAGE)
		{
			S3EventLogAdd("Battery charge voltage re-established", 1, -1, -1, Ch);
			pCh->m_Alarms &= ~S3_CH_NO_CHARGE_VOLTAGE;
			StateChange = 1;
		}
	}

	if (alarms & S3_CH_CHARGE_FAULT)
	{
		if (pCh->m_Alarms & S3_CH_CHARGE_FAULT)
		{
			S3EventLogAdd("Battery charge fault reset", 1, -1, -1, Ch);
			pCh->m_Alarms &= ~S3_CH_CHARGE_FAULT;
			StateChange = 1;
		}
	}

	return StateChange;
}

// ---------------------------------------------------------------------------

unsigned char S3RxGetAlarms(char Rx)
{
	return S3Data->m_Rx[Rx].m_Alarms;
}

// ---------------------------------------------------------------------------

int S3RxSetAlarm(char Rx, char Tx, unsigned char alarms)
{
	int StateChange = 0;

	pS3RxData pRx = &S3Data->m_Rx[Rx];

	if (Tx != -1)
	{
		// Alarm requested
		if (alarms & S3_RX_RLL_HIGH)
		{
			// Alarm already raised?
			if (!(pRx->m_TxAlarms[Tx] & S3_RX_RLL_HIGH))
			{
				S3EventLogAdd("RLL too high", 3, Rx, Tx, -1);
				pRx->m_TxAlarms[Tx] |= S3_RX_RLL_HIGH;
				StateChange = 1;
			}
		}

		if (alarms & S3_RX_RLL_LOW)
		{
			if (!(pRx->m_TxAlarms[Tx] & S3_RX_RLL_LOW))
			{
				S3EventLogAdd("RLL too low", 3, Rx, Tx, -1);
				pRx->m_TxAlarms[Tx] |= S3_RX_RLL_LOW;
				StateChange = 1;
			}
		}

		return 0;
	}
	
	// Alarm requested
	if (alarms & S3_RX_INT_FAIL)
	{
		// Alarm already raised?
		if (!(pRx->m_Alarms & S3_RX_INT_FAIL))
		{
			S3EventLogAdd("Receiver internal comms failure raised", 3, Rx, -1, -1);
			pRx->m_Alarms |= S3_RX_INT_FAIL;
			StateChange = 1;
		}
	}

	if (alarms & S3_RX_OVER_TEMP)
	{
		if (!(pRx->m_Alarms & S3_RX_OVER_TEMP))
		{
			S3EventLogAdd("Receiver module over-temperature alarm raised", 3, Rx, -1, -1);
			pRx->m_Alarms |= S3_RX_OVER_TEMP;
			StateChange = 1;
		}
	}
	
	if (alarms & S3_RX_UNDER_TEMP)
	{
		if (!(pRx->m_Alarms & S3_RX_UNDER_TEMP))
		{
			S3EventLogAdd("Receiver module under-temperature alarm raised", 3, Rx, -1, -1);
			pRx->m_Alarms |= S3_RX_UNDER_TEMP;
			StateChange = 1;
		}
	}

	if (alarms & S3_RX_OVER_VOLT)
	{
		if (!(pRx->m_Alarms & S3_RX_OVER_VOLT))
		{
			S3EventLogAdd("Receiver module high supply voltage alarm raised", 3, Rx, -1, -1);
			pRx->m_Alarms |= S3_RX_OVER_VOLT;
			StateChange = 1;
		}
	}
	
	if (alarms & S3_RX_UNDER_VOLT)
	{
		if (!(pRx->m_Alarms & S3_RX_UNDER_VOLT))
		{
			S3EventLogAdd("Receiver module low supply voltage alarm raised", 3, Rx, -1, -1);
			pRx->m_Alarms |= S3_RX_UNDER_VOLT;
			StateChange = 1;
		}
	}

	if (alarms & S3_RX_INIT_FAIL)
	{
		if (!(pRx->m_Alarms & S3_RX_INIT_FAIL))
		{
			S3EventLogAdd("Receiver module initialisation failed", 3, Rx, -1, -1);
			pRx->m_Alarms |= S3_RX_INIT_FAIL;
			StateChange = 1;
		}
	}

	if (alarms & S3_RX_RLL_LOW)
	{
		if (!(pRx->m_Alarms & S3_RX_RLL_LOW))
		{
			S3EventLogAdd("RLL low", 3, Rx, -1, -1);
			pRx->m_Alarms |= S3_RX_RLL_LOW;
			StateChange = 1;
		}
	}
	

	return StateChange;
}

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Return highest priority alarm

// S3_RX_INT_FAIL
// S3_RX_OVER_TEMP
// S3_RX_UNDER_TEMP
// S3_RX_OVER_VOLT
// S3_RX_UNDER_VOLT
// S3_RX_INIT_FAIL
// S3_RX_RLL_LOW
// S3_RX_COMM_FAIL

int S3RxAlarmGetString(char Rx, char *S3AlarmString, int len)
{
	pS3RxData pRx = &S3Data->m_Rx[Rx];
	pRx->m_CurAlarmSrc = -1;

	if (pRx->m_Alarms & S3_RX_INT_FAIL)
	{
		strcpy_s(S3AlarmString, len, "E:Internal comms fail");
		pRx->m_CurAlarmSrc = 0;
		pRx->m_CurAlarm = S3_RX_INT_FAIL;
		return 1;
	}

	if (pRx->m_Alarms & S3_RX_OVER_TEMP)
	{
		strcpy_s(S3AlarmString, len, "E:Over temperature");
		pRx->m_CurAlarmSrc = 0;
		pRx->m_CurAlarm = S3_RX_OVER_TEMP;
		return 1;
	}

	if (pRx->m_Alarms & S3_RX_UNDER_VOLT)
	{
		strcpy_s(S3AlarmString, len, "E:Power supply under-voltage");
		pRx->m_CurAlarmSrc = 0;
		pRx->m_CurAlarm = S3_RX_UNDER_VOLT;
		return 1;
	}

	if (pRx->m_Alarms & S3_RX_OVER_VOLT)
	{
		strcpy_s(S3AlarmString, len, "E:Power supply under-voltage");
		pRx->m_CurAlarmSrc = 0;
		pRx->m_CurAlarm = S3_RX_OVER_VOLT;
		return 1;
	}

	if (pRx->m_Alarms & S3_RX_INIT_FAIL)
	{
		strcpy_s(S3AlarmString, len, "E:Start-up failed");
		pRx->m_CurAlarmSrc = 0;
		pRx->m_CurAlarm = S3_RX_INIT_FAIL;
		return 1;
	}

	if (pRx->m_Alarms)
	{
		sprintf_s(S3AlarmString, len, "E:Unspecified alarm condition: %d",
			pRx->m_Alarms);

		return 1;
	}

	for(char Tx = 0; Tx < S3RxGetTxN(Rx); Tx++)
	{
		if (pRx->m_TxAlarms[Tx] & S3_RX_RLL_LOW)
		{
			strcpy_s(S3AlarmString, len, "W:Received light level low");
			pRx->m_CurAlarmSrc = 100 + Tx;
			pRx->m_CurAlarm = S3_RX_RLL_LOW;
			return 1;
		}

		if (pRx->m_TxAlarms[Tx] & S3_RX_RLL_HIGH)
		{
			strcpy_s(S3AlarmString, len, "W:Received light level high");
			pRx->m_CurAlarmSrc = 100 + Tx;
			pRx->m_CurAlarm = S3_RX_RLL_HIGH;
			return 1;
		}
	}

	return 0;
}

// ----------------------------------------------------------------------------

int S3RxCancelAlarm(char Rx, char Tx, unsigned char alarms)
{
	int StateChange = 0;

	pS3RxData pRx = &S3Data->m_Rx[Rx];

	if (Tx != -1)
	{
		if (alarms & S3_RX_RLL_HIGH)
		{
			// Only on disconnection?
			if (pRx->m_TxAlarms[Tx] & S3_RX_RLL_HIGH)
			{
				S3EventLogAdd("RLL high cancelled", 1, Rx, Tx, -1);
				pRx->m_TxAlarms[Tx] &= ~S3_RX_RLL_HIGH;
				StateChange = 1;
			}
		}

		if (alarms & S3_RX_RLL_LOW)
		{
			// Only on disconnection?
			if (pRx->m_TxAlarms[Tx] & S3_RX_RLL_LOW)
			{
				S3EventLogAdd("RLL low cancelled", 1, Rx, Tx, -1);
				pRx->m_TxAlarms[Tx] &= ~S3_RX_RLL_LOW;
				StateChange = 1;
			}
		}

		return 0;
	}

	if (alarms & S3_RX_INT_FAIL)
	{
		// Only on disconnection?
		if (pRx->m_Alarms & S3_RX_INT_FAIL)
		{
			S3EventLogAdd("Receiver internal comms failure cancelled", 1, Rx, -1, -1);
			pRx->m_Alarms &= ~S3_RX_INT_FAIL;
			StateChange = 1;
		}
	}

	if (alarms & S3_RX_OVER_TEMP)
	{
		if (pRx->m_Alarms & S3_RX_OVER_TEMP)
		{
			S3EventLogAdd("Receiver module over-temperature alarm cancelled", 1, Rx, -1, -1);
			pRx->m_Alarms &= ~S3_RX_OVER_TEMP;
			StateChange = 1;
		}
	}
	
	if (alarms & S3_RX_UNDER_TEMP)
	{
		if (pRx->m_Alarms & S3_RX_UNDER_TEMP)
		{
			S3EventLogAdd("Receiver module under-temperature alarm cancelled", 1, Rx, -1, -1);
			pRx->m_Alarms &= ~S3_RX_UNDER_TEMP;
			StateChange = 1;
		}
	}

	if (alarms & S3_RX_OVER_VOLT)
	{
		if (pRx->m_Alarms & S3_RX_OVER_VOLT)
		{
			S3EventLogAdd("Receiver module high supply voltage alarm cancelled", 1, Rx, -1, -1);
			pRx->m_Alarms &= ~S3_RX_OVER_VOLT;
			StateChange = 1;
		}
	}
	
	if (alarms & S3_RX_UNDER_VOLT)
	{
		if (pRx->m_Alarms & S3_RX_UNDER_VOLT)
		{
			S3EventLogAdd("Receiver module low suppy voltage alarm cancelled", 1, Rx, -1, -1);
			pRx->m_Alarms &= ~S3_RX_UNDER_VOLT;
			StateChange = 1;
		}
	}

	if (alarms & S3_RX_INIT_FAIL)
	{
		if (pRx->m_Alarms & S3_RX_INIT_FAIL)
		{
			S3EventLogAdd("Receiver module initialisation fail cancelled", 1, Rx, -1, -1);
			pRx->m_Alarms &= ~S3_RX_INIT_FAIL;
			StateChange = 1;
		}
	}

	if (alarms & S3_RX_RLL_LOW)
	{
		if (pRx->m_Alarms & S3_RX_RLL_LOW)
		{
			S3EventLogAdd("RLL low cancelled", 1, Rx, -1, -1);
			pRx->m_Alarms &= ~S3_RX_RLL_LOW;
			StateChange = 1;
		}
	}

	return StateChange;
}

// ---------------------------------------------------------------------------
// Cancel local alarms, if remote alarms, alarm will just re-appear.

int S3RxCancelCurAlarm(char Rx)
{
	pS3RxData pRx = &S3Data->m_Rx[Rx];

	if (pRx->m_CurAlarmSrc == -1)
		return 0;

	if (pRx->m_CurAlarmSrc >= 100)
	{
		S3RxCancelAlarm(Rx, pRx->m_CurAlarmSrc - 100, pRx->m_CurAlarm);
	}
	else
	{
		switch(pRx->m_CurAlarmSrc)
		{
			case 0: S3RxCancelAlarm(Rx, -1, pRx->m_CurAlarm); break;
		}
	}

	pRx->m_CurAlarmSrc = -1;

	return 0;
}

// ---------------------------------------------------------------------------