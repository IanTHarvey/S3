//
// TODO: Make Rx/Tx/IP indexing char and use -1 to indicate non-existence

#include "stdafx.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <float.h>
#include <math.h>

#include "S3SystemDetails.h"
#include "S3DataModel.h"
#include "S3GPIB.h"
#include "S3I2C.h"
#include "S3Gain.h"

#ifdef S3_AGENT
#include "S3Agent\S3Comms.h"
#endif

#ifdef TRIZEPS
#include "drvLib_app.h"
#endif

extern struct DbgPollSysStruct	DbgPollSysData;

wchar_t *UnitStrings[] = {
	{L"dBm"},
	{L"dB\u03bcV"},
	{L"Vpk"}
};

// Use type defines to index. TODO: Worth implementing as functions?
wchar_t *RxTypeStrings[] = {
	{ L"Empty" },
	{ L"Rx1" },
	{ L"Rx2" },
	{ L"" },
	{ L"" },
	{ L"" },
	{ L"Rx6" }
};

wchar_t *TxTypeStrings[] = {
	{ L"Unconnected" },
	{ L"Tx1" },
	{ L"" },
	{ L"" },
	{ L"" },
	{ L"" },
	{ L"" },
	{ L"" },
	{ L"Tx8" }
};

wchar_t *ParaValueStrings[] = {
	{ L"Unconnected" },
	{ L"Tx1" },
	{ L"" },
	{ L"" },
	{ L"" },
	{ L"" },
	{ L"" },
	{ L"" },
	{ L"Tx8" }
};

wchar_t *BattTypeStrings[] = {
	{ L"Unknown" },
	{ L"Unvalidated" },
	{ L"" },
	{ L"" },
	{ L"2S1P" },
	{ L"2S2P" }
};

// ----------------------------------------------------------------------------

S3DataModel sS3Data;
// S3DataModel sS3Shadow;

pS3DataModel S3Data = NULL;
pS3DataModel S3Shadow = NULL;

extern int S3GPIOtest();
extern int S3I2CSetIPGain(char Rx, char Tx, char IP);

// TODO: TBD... Map user input to internal Tx input
unsigned char S3Tx8IPMap[8] = {0, 1, 2, 3, 4, 5, 6, 7};

short PeakThTable[S3_TX_N_RF_PATH] =
	{200,	200,	-700,	-700,	-700,	-700,	-700};

// ----------------------------------------------------------------------------

FILE	*S3DbgLog = NULL;

#ifdef S3_TRACE_FILE
//FILE	*S3DbgLog;
#endif

pS3DataModel S3Init(bool DemoMode)
{
	// Live data model
	S3Data =	&sS3Data;
	S3DataModelInit(S3Data, DemoMode);

	// TODO: An alternative for later
	// Read from configuration file
	// S3Shadow =	&sS3Shadow;
	// S3DataModelInit(S3Shadow);

	S3Shadow =	&sS3Data;
	
	S3ChInitAll();
#ifndef S3_AGENT
	S3DbgPollInit();
	S3EventLogInit(NULL);

#ifndef	S3VIRGIN
	int err;
	if (err = S3Read(NULL))
	{
		char Msg[S3_EVENTS_LINE_LEN];

		strcpy_s(Msg, S3_EVENTS_LINE_LEN, "Failed to load default configuration: ");

		switch(err)
		{
		case 1: strcat_s(Msg, S3_EVENTS_LINE_LEN, "System not initialised"); 
			break;
		case 2: strcat_s(Msg, S3_EVENTS_LINE_LEN, "Failed to open file");
			break;
		case 3: strcat_s(Msg, S3_EVENTS_LINE_LEN, "Incompatible file format");
			break;
		default: strcat_s(Msg, S3_EVENTS_LINE_LEN, "Unspecified error");
			break;
		}

		S3EventLogAdd(Msg, 1, -1, -1, -1);
	}
#endif // S3_AGENT

	S3SetUnits(-1);

#endif // S3VIRGIN

	return S3Data;
}

// ----------------------------------------------------------------------------

int S3DataModelInit(pS3DataModel dm, bool DemoMode)
{
	memset(dm, 0, sizeof(S3DataModel));

	// TODO: Need this?
	sprintf_s(dm->m_SW, S3_MAX_SW_VER_LEN, "%f", S3_CONTROLLER_VERSION);
	
	dm->m_SWVersionD = S3_CONTROLLER_VERSION;
	dm->m_FileVersion = S3_FILE_VERSION;

	dm->m_ContTComp = S3_TCOMP_CONT; // Continuous by default
	dm->m_AGC = S3_AGC_CONT;
	dm->m_TxStartState = S3_TXSTART_SLEEP;

	dm->m_TCompGainOption = true;

	dm->m_LowNoiseOption = false;
	dm->m_WinTrackOption = S3_DEF_WIN_TRACK_OPTION;
	dm->m_SoftShutdownOption = true;
	
	dm->m_Modified = false;

	dm->m_Locked = false;
	dm->m_TxSelfTest = true;

	dm->m_FactoryMode = false;

	dm->m_ScrnOSx = dm->m_ScrnOSy = SHRT_MIN;

	dm->m_DemoMode = DemoMode;

    dm->m_SelectedIP = -1;
    dm->m_SelectedTx = -1;
    dm->m_SelectedRx = -1;

	// Too dumb to remember, every time...
#ifdef _NEVER_DEBUG
	dm->m_Remote = true;
#else
	dm->m_Remote = false;
#endif

	sprintf_s(dm->m_NodeName, S3_MAX_NODE_NAME_LEN, "Controller");

	*dm->m_ReportFileName = '\0';
	*dm->m_TestName = '\0';

#ifndef S3_AGENT
	// TODO: Should all files be tied to the same system (config) name (and fixed path)?
	if (dm->m_DemoMode)
		strcpy_s(dm->m_ConfigName, S3_MAX_CFG_NAME_LEN, S3_DEF_DEMO_CONFIG_FILENAME);	// Load/save filename
	else
		strcpy_s(dm->m_ConfigName, S3_MAX_CFG_NAME_LEN, S3_DEF_CONFIG_FILENAME);

	strcpy_s(dm->m_ConfigPath, S3_MAX_FILENAME_LEN, S3_ROOT_DIR);				// Load/save path

	strcpy_s(dm->m_EventLogName, S3_MAX_FILENAME_LEN, S3_DEF_EVENT_LOG_FILENAME);
	strcpy_s(dm->m_EventLogPath, S3_MAX_FILENAME_LEN, S3_ROOT_DIR);

	sprintf_s(dm->m_LockFileName, S3_MAX_FILENAME_LEN, "%s\\%s.s3k",
			S3_ROOT_DIR, S3_LOCK_FILENAME);

	sprintf_s(dm->m_UnlockFileName, S3_MAX_FILENAME_LEN, "%s\\%s.s3k",
			"Hard Disk", S3_UNLOCK_FILENAME);

	// S3GetLockFile();

	sprintf_s(dm->m_SNFileName, S3_MAX_FILENAME_LEN, "%s\\%s.s3n",
			S3_ROOT_DIR, S3_SN_FILENAME);

	S3SysReadSN();

	sprintf_s(dm->m_PNFileName, S3_MAX_FILENAME_LEN, "%s\\%s.s3n",
			S3_ROOT_DIR, S3_PN_FILENAME);

	S3SysReadPN();

	sprintf_s(dm->m_ScreenOffsetFileName, S3_MAX_FILENAME_LEN, "%s\\%s.s3o",
			S3_ROOT_DIR, S3_SCREEN_OFFSET_FILENAME);

	S3ReadScreenOffsets();
#endif

	// TODO: Override above from registry if available
	// ...

	dm->m_SelectedRx = dm->m_SelectedTx = -1;

	S3SysInit(dm);

	// Initialise
	for (char Rx = 0; Rx < S3_MAX_RXS; Rx++)
	{
		S3RxData	*pRx = &(dm->m_Rx[Rx]);
		sprintf_s(pRx->m_NodeName, S3_MAX_NODE_NAME_LEN, "Rx%d", Rx + 1);
		S3RxInit(pRx);
		pRx->m_Id = Rx;
		
		for (char Tx = 0; Tx < S3_MAX_TXS; Tx++)
		{
			pS3TxData	pTx = &(pRx->m_Tx[Tx]);

			sprintf_s(pTx->m_NodeName, S3_MAX_NODE_NAME_LEN, "Tx%d", Tx + 1);
			S3TxInit(pTx);
			pTx->m_ParentId = Rx;
			pTx->m_Id = Tx;

			for (char IP = 0; IP < S3_MAX_IPS; IP++)
			{
				// Inheritables (Actual input parameters)
				pS3IPData pIP = &pTx->m_Input[IP];
				S3IPInit(pIP);
				sprintf_s(pIP->m_NodeName, S3_MAX_NODE_NAME_LEN, "RF%d", IP + 1);

				S3Data->m_GainSent[Rx][Tx][IP] = -128;
				S3Data->m_PathSent[Rx][Tx][IP] = -128;
			}
		}
	}

	return 0;
}

// ----------------------------------------------------------------------------

int S3Reset()
{
	S3Init(false);

	return 0;
}

// ----------------------------------------------------------------------------

int S3End(void)
{
	// S3Save2(S3_DEFAULT_CONFIG_FILENAME);

	S3EventLogAdd("S3End invoked", 1, -1, -1, -1);

	S3EventLogClose();

	S3Data = NULL;

	return 0;
}

// ----------------------------------------------------------------------------

const char *S3GetTestName()
{
	return S3Data->m_TestName;
}

// ----------------------------------------------------------------------------

int	S3SetMACAddr(const unsigned char *MAC)
{
	for (unsigned char i = 0; i < MAC_LEN; i++)
		S3Data->m_MACAddr[i] = MAC[i];

	return 0;
}

// ----------------------------------------------------------------------------

/*
int	S3SetIPAddr(const unsigned char *IPAddr)
{
	for (unsigned char i = 0; i < 4; i++)
		S3Data->m_IPv4Addr[i] = IPAddr[i];

	return 0;
}
*/

// ----------------------------------------------------------------------------

int	S3SetIPAddrStr(const char *str)
{
	strcpy_s(S3Data->m_IPv4Addr, S3_MAX_IP_ADDR_LEN, str);

	return 0;
}

// ----------------------------------------------------------------------------

int	S3GetIPAddrStr(char *str)
{
	strcpy_s(str, S3_MAX_IP_ADDR_LEN, S3Data->m_IPv4Addr);

	return 0;
}
// ----------------------------------------------------------------------------

int	S3SetIPMaskStr(const char *str)
{
	strcpy_s(S3Data->m_IPv4Mask, S3_MAX_IP_ADDR_LEN, str);

	return 0;
}

// ----------------------------------------------------------------------------

int	S3GetIPMaskStr(char *str)
{
	strcpy_s(str, S3_MAX_IP_ADDR_LEN, S3Data->m_IPv4Mask);

	return 0;
}
// ---------------------------------------------------------------------------

unsigned short S3GetIPPort()
{
	return S3Data->m_IPPort;
}

// ---------------------------------------------------------------------------

int S3SetIPPort(unsigned short port)
{
	if (port == 0)
	{
		S3Data->m_IPPort = 0;
		return 0;
	}
	
	if (S3Data->m_IPPort == port)
		return 0;

	if (port < 49152 || port > 65535)
		return 1;
	
	S3Data->m_IPPort = port;

	return 0;
}

// ----------------------------------------------------------------------------

int	S3GetMACAddrStr(char *addr)
{
	for(unsigned char i = 0; i < 6; i++)
	{
		if (S3Data->m_MACAddr[i] != 0)
		{
			sprintf_s(addr, S3_MAX_IP_ADDR_LEN, "%02x:%02x:%02x:%02x:%02x:%02x",
				S3Data->m_MACAddr[0], S3Data->m_MACAddr[1], S3Data->m_MACAddr[2],
				S3Data->m_MACAddr[3], S3Data->m_MACAddr[4], S3Data->m_MACAddr[5]);

			return 0;
		}
	}

	strcpy_s(addr, S3_MAX_IP_ADDR_LEN, "--:--:--:--:--:--");

	return 0;
}

// ----------------------------------------------------------------------------

pS3DataModel S3GetSys()
{
	return S3Data;
}

// ----------------------------------------------------------------------------
// Obsolete

pS3RxData S3RxGetPtr(char Rx)
{
	return &S3Data->m_Rx[Rx];
}



// ----------------------------------------------------------------------------

S3DataModel *S3Copy(const S3DataModel *src)
{
	S3DataModel	*dest = new S3DataModel;

	return dest;
}

// ----------------------------------------------------------------------------

int S3ParseSerialNumber(const char *sn)
{
	return 0;
}

// ----------------------------------------------------------------------------

int S3ParseModelId(const char *id)
{
	return 0;
}

// ----------------------------------------------------------------------------

int S3ConfigInit(pS3Config config)
{
	config->m_Gain =			S3_DEFAULT_GAIN;
	config->m_Tau =				TauNone;		// None, p01uS, p1uS, p10uS
	config->m_InputZ =			W50;			// W50, W1M
	config->m_LowNoiseMode =	false;
	config->m_WindowTracking =	false;

	config->m_MaxInput =		-10000000.0;	// S3CalcMaxIP(config->m_Gain);

// 	config->m_PassiveIntegrator = false;

	return 0;
}

// ----------------------------------------------------------------------------

const wchar_t *S3GetTypeStr(char Rx, char Tx)
{
	if (Rx == -1)
		RxTypeStrings[0];
		
	if (Tx == -1)
	{
		return RxTypeStrings[S3Data->m_Rx[Rx].m_Type];
	}

	return TxTypeStrings[S3Data->m_Rx[Rx].m_Tx[Tx].m_Type];
}

// ----------------------------------------------------------------------------

char S3GetType(char Rx, char Tx)
{
	if (Rx == -1)
		return -1;
		
	if (Tx == -1)
	{
		return S3Data->m_Rx[Rx].m_Type;
	}

	return S3Data->m_Rx[Rx].m_Tx[Tx].m_Type;
}

// ----------------------------------------------------------------------------

int S3SysInit(pS3DataModel node)
{
	// File/build versions
	*node->m_ImageID = '\0';
	*node->m_ImageOS = '\0';
	*node->m_ImageDate = '\0';
	*node->m_ImageTime = '\0';
	*node->m_AppDateTime = '\0';
	*node->m_BuildNum = '\0';

	node->m_PeakPulse = 0;

	S3SysSetSW(S3_SYS_SW);
	S3SysSetModel(S3_SYS_MODEL);
	S3OSGetImageID();

#ifndef S3_AGENT
	S3SetAppDateTime();
#endif

	S3ConfigInit(&(node->m_Config));

	node->m_DisplayUnits = S3_UNITS_DBM;

	node->m_IPPort = S3_DEFAULT_IP_PORT;

	node->m_SleepAll = false;
	node->m_WakeAll = false;

	node->m_USBOpen = false;

	return 0;
}

// ----------------------------------------------------------------------------
// Push CurrentNode's config parameters to all children
int	S3PushConfig(unsigned char *CurrentNode)
{
	unsigned char i = 0;

	while (CurrentNode[i] != 0 && i < 3)
		i++;

	switch (i)
	{
	case 0:
		S3SysPushConfig();
		break;
	case 1:
		S3RxPushConfig(CurrentNode[0] - 1);
		break;
	case 2:
		S3TxPushConfig(CurrentNode[0] - 1, CurrentNode[1] - 1);
		break;
	}

	return 0;
}

// ----------------------------------------------------------------------------

int	S3SysPushConfig()
{
	pS3DataModel	dm = S3Data;

	for (char Rx = 0; Rx < S3_MAX_RXS; Rx++)
	{
		S3RxData	*md = &(dm->m_Rx[Rx]);

		S3CopyConfig(&(md->m_Config), &(dm->m_Config));

		S3RxPushConfig(Rx);
	}

	return 0;
}

// ----------------------------------------------------------------------------

int	S3RxPushConfig(char Rx)
{
	S3RxData	*md = &(S3Data->m_Rx[Rx]);

	for (char Tx = 0; Tx < S3_MAX_TXS; Tx++)
	{
		pS3TxData pTx = &(md->m_Tx[Tx]);

		S3CopyConfig(&(pTx->m_Config), &(md->m_Config));

		S3TxPushConfig(Rx, Tx);
	}

	return 0;
}

// ----------------------------------------------------------------------------

int S3CopyConfig(S3Config *dest, const S3Config *src)
{
	dest->m_Gain = src->m_Gain;
	dest->m_Tau = src->m_Tau;
	dest->m_InputZ = src->m_InputZ;
	dest->m_LowNoiseMode = src->m_LowNoiseMode;
	dest->m_WindowTracking = src->m_WindowTracking;
//	dest->m_PassiveIntegrator = src->m_PassiveIntegrator;

	return 0;
}

// ----------------------------------------------------------------------------

int S3CompConfig(const S3Config *dest, const S3Config *src)
{
	if (dest->m_Gain != src->m_Gain)
		return 1;

	if (dest->m_Tau != src->m_Tau)
		return 1;

	if (dest->m_InputZ != src->m_InputZ)
		return 1;

	if (dest->m_LowNoiseMode != src->m_LowNoiseMode)
		return 1;

	if (dest->m_WindowTracking != src->m_WindowTracking)
		return 1;

//	if (dest->m_PassiveIntegrator != src->m_PassiveIntegrator)
//		return 1;

	return 0;
}

// ----------------------------------------------------------------------------

int	S3SetNodeName(char *CurrentNode, char *NodeName)
{
	unsigned char i = 0;

	while (CurrentNode[i] != 0 && i < 4)
		i++;

	switch (i)
	{
	case 0:
		S3SysSetNodeName(CurrentNode, NodeName);
		break;
	case 1:
		S3RxSetNodeName(CurrentNode, NodeName);
		break;
	case 2:
		S3TxSetNodeName(CurrentNode, NodeName);
		break;
	case 3:
		S3IPSetNodeName(CurrentNode, NodeName);
		break;
	}

	return 0;
}

// ----------------------------------------------------------------------------

int S3SysSetNodeName(const char *CurrentNode, char *NodeName)
{
	strcpy_s(S3Data->m_NodeName, S3_MAX_NODE_NAME_LEN, NodeName);
	
	return 0;
}

// ----------------------------------------------------------------------------

int S3StartUpReadRack()
{
	for (char i = 0; i < S3_MAX_RXS; i++)
	{
		S3StartUpReadSlot(i);
	}

	return 0;
}

// ----------------------------------------------------------------------------

int S3StartUpReadSlot(char Rx)
{
	// Read type - deduce number of inputs
	// Read serial number
	// Read part number

	// For each Tx
		// Get type - deduce number of inputs
		// Get serial number
		// Get part number

		// Set parameters
		// Get alarms

		// For each input
			// Set parameters
			// Get alarms

	return 0;
}




// ----------------------------------------------------------------------------

int S3SetSysTimeStr(const char *str)
{
	char	*eptr;
	char h = (char)strtol(str, &eptr, 10);

	if (*eptr != ':')
		return 1;

	char m = (char)strtol(eptr + 1, &eptr, 10);

	if (*eptr != ':')
		return 1;

	char s = (char)strtol(eptr + 1, &eptr, 10);

	if (*eptr != '\0')
		return 1;
	
	return 0;
}



// ----------------------------------------------------------------------------

int S3RxGetNTx(char Rx)
{
	switch (S3Data->m_Rx[Rx].m_Type)
	{
	case S3_Rx1:
		return 1;
		break;
	case S3_Rx2:
		return 2;
		break;
	case S3_Rx6:
		return 6;
		break;
	}

	return 0;
}

// ----------------------------------------------------------------------------
// TODO: Why worry about what's installed - just apply to all at each sub-level?
// TODO: Any value in this?
int S3SetGainPush(char cRx, char cTx, char cIP, char gain)
{
	if (gain < S3_MIN_GAIN)
		gain = S3_MIN_GAIN;
	else if (gain > S3_MAX_GAIN)
		gain = S3_MAX_GAIN;

	if (cRx == -2)
	{
		S3Data->m_Config.m_Gain = gain;
		return 0;
	}
	
	if (cIP != -1)
	{
		S3IPSetGain(&S3Data->m_Rx[cRx].m_Tx[cTx].m_Input[cIP], gain);
	}
	else if (cTx != -1)
	{
		S3Data->m_Rx[cRx].m_Tx[cTx].m_Config.m_Gain = gain;
		for (char IP = 0; IP < S3TxGetNIP(cRx, cTx); IP++)
			S3IPSetGain(&S3Data->m_Rx[cRx].m_Tx[cTx].m_Input[IP], gain);
	}
	else if (cRx != -1)
	{
		S3Data->m_Rx[cRx].m_Config.m_Gain = gain;

		for (char Tx = 0; Tx < S3RxGetTxN(cRx); Tx++)
		{
			S3Data->m_Rx[cRx].m_Tx[Tx].m_Config.m_Gain = gain;
			for (char IP = 0; IP < S3TxGetNIP(cRx, Tx); IP++)
				S3IPSetGain(&S3Data->m_Rx[cRx].m_Tx[Tx].m_Input[IP], gain);
		}
	}
	else
	{
		// 'All'
		S3Data->m_Config.m_Gain = gain;

		for (char Rx = 0; Rx < S3_MAX_RXS; Rx++)
		{
			S3Data->m_Rx[Rx].m_Config.m_Gain = gain;

			if (S3RxExistQ(Rx))
			{
				for (char Tx = 0; Tx < S3RxGetTxN(Rx); Tx++)
				{
					S3Data->m_Rx[Rx].m_Tx[Tx].m_Config.m_Gain = gain;

					if (S3TxExistQ(Rx, Tx))
					{
						for (char IP = 0; IP < S3TxGetNIP(Rx, Tx); IP++)
							S3IPSetGain(&S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP], gain);
					}
				}
			}
		}
	}
	
	// I2C

	return 0;
}

// ----------------------------------------------------------------------------

int S3SetGain(char Rx, char Tx, char IP, char gain)
{
	// TODO: refer to: m_ContGainTComp

	// No! Gain may not have changed, but limits may have
	// if (S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_Gain == gain)
	//	return 0;

	if (S3IPGetAlarms(Rx, Tx, IP) & S3_IP_OVERDRIVE)
	 	return 2;

	char	low, high;
	int		GainLimited = 0;

	S3GetGainLimits(Rx, Tx, IP, &low, &high);

	if (gain < low)
	{
		GainLimited = 1;
		gain = low;
	}
	else if (gain > high)
	{
		GainLimited = 1;
		gain = high;
	}

#ifdef S3_AGENT
    CString Command, Args, Response;
    Command = L"GAIN";
    
    if(Rx == -1 && Tx == -1 && IP == -1)
    {
        Args.Format(_T(" DEF %d"), gain);
    }
    else
    {
        Args.Format(_T(" %d %d %d %d"), (Rx + 1), (Tx + 1), (IP + 1), gain);
    }
    Command.Append(Args);

    Response = SendSentinel3Message(Command);
#else
	if (Rx == -1)
	{
		S3Data->m_Config.m_Gain = gain;
	}
	else if (Tx == -1)
	{
		S3Data->m_Rx[Rx].m_Config.m_Gain = gain;
	}
	else if (IP == -1)
	{
		S3Data->m_Rx[Rx].m_Tx[Tx].m_Config.m_Gain = gain;
	}
	else
	{
		double maxip = S3CalcMaxIP(gain);
		double P1dB = S3CalcP1dB(gain);

		S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_Gain = gain;
		
		S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_MaxInput = maxip;
		S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_P1dB = P1dB;

		// Update display parameter - actual TCOMP done by transmitter so no need
		// to do anything
		if (S3GetTCompMode() == S3_TCOMP_GAIN)
			S3TxSetTempComp(Rx, Tx, S3TxGetTemp(Rx, Tx));

		S3IPSetGainSent(Rx, Tx, IP, SCHAR_MIN); // Invalidate to force update
	}
#endif

	return GainLimited;

}


// ----------------------------------------------------------------------------

InputZ S3GetImpedance(char Rx, char Tx, char IP)
{
	if (Rx == -1)
		return S3Data->m_Config.m_InputZ;
		
	if (Tx == -1)
		return S3Data->m_Rx[Rx].m_Config.m_InputZ;

	if (IP == -1)
		return S3Data->m_Rx[Rx].m_Tx[Tx].m_Config.m_InputZ;

	return S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_InputZ;
}

// ----------------------------------------------------------------------------

SigmaT S3GetSigmaTau(char Rx, char Tx, char IP)
{
	if (Rx == -1)
		return S3Data->m_Config.m_Tau;
		
	if (Tx == -1)
		return S3Data->m_Rx[Rx].m_Config.m_Tau;

	if (IP == -1)
		return S3Data->m_Rx[Rx].m_Tx[Tx].m_Config.m_Tau;

	return S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_Tau;
}

// ----------------------------------------------------------------------------

bool S3GetLowNoiseMode(char Rx, char Tx, char IP)
{
	if (Rx == -1)
		return S3Data->m_Config.m_LowNoiseMode;
		
	if (Tx == -1)
		return S3Data->m_Rx[Rx].m_Config.m_LowNoiseMode;

	if (IP == -1)
		return S3Data->m_Rx[Rx].m_Tx[Tx].m_Config.m_LowNoiseMode;

	return S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_LowNoiseMode;
}

// ----------------------------------------------------------------------------

int S3SetLowNoiseMode(char Rx, char Tx, char IP, bool On)
{
	int GainChanged = 0;
	
	if (Rx == -1)
		S3Data->m_Config.m_LowNoiseMode = On;
	else if (Tx == -1)
		S3Data->m_Rx[Rx].m_Config.m_LowNoiseMode = On;
	else if (IP == -1)
		S3Data->m_Rx[Rx].m_Tx[Tx].m_Config.m_LowNoiseMode = On;
	else
	{
		GainChanged = S3IPSetLowNoiseMode(Rx, Tx, IP, On);
	}

	return GainChanged;
}

// ----------------------------------------------------------------------------
// TODO: May require further inputs from specific RF input 

double S3CalcMaxIP(int g)
{
	// Max RF input power for link gain of g
	double maxip = (double)(-g - 22);

	switch(S3Data->m_DisplayUnits)
	{
	case S3_UNITS_DBM :	break;
	case S3_UNITS_DBUV:	maxip += 106; break;
	case S3_UNITS_MV:	maxip = 
			pow((pow(10.0, (maxip / 10.0)) / 20.0), 0.5) *
												pow(2.0, 0.5); // Volts
		break;
	}

	return maxip;
}

// ----------------------------------------------------------------------------
// Input power that will cause 1dB of gain loss due to compression.
// If gain <= -16dB assume intereted in pulse power, so convert to V
// whatever the user preference.

double S3CalcP1dB(int g_dB)
{
	double p1;

	// Search until gain < one of the measurement points so always
	// conservative
	for(unsigned char i = 0; i < S3_N_P1DB - 1; i++)
	{	
		if (g_dB > S3P1dBTable[i + 1][0])
		{
			p1 = S3P1dBTable[i][1];
			break;
		}
	}

	unsigned char DisplayUnits;
	if (g_dB <= -16)
		DisplayUnits = S3_UNITS_MV;
	else
		DisplayUnits = S3Data->m_DisplayUnits;

	switch(DisplayUnits)
	{
	case S3_UNITS_DBM :	break;
	case S3_UNITS_DBUV:	p1 += 106; break;
	case S3_UNITS_MV:	p1 = 
			pow((pow(10.0, (p1 / 10.0)) / 20.0), 0.5) *
												pow(2.0, 0.5); // Volts
		break;
	}

	return p1;
}

// ----------------------------------------------------------------------------

double S3CalcP1dBOld(int g_dB)
{
	int i = g_dB - S3GainTable[0][UGAIN];
	double p1;

	if (i >= 0 && i < S3_GAIN_VALUES)
		p1 = S3P1dBTableOld[i];
	else
		p1 = 0.0; // Use NaN?

	return p1;
}

// ----------------------------------------------------------------------------

int S3IPCalcGain(double maxip)
{
	double g;

	switch(S3Data->m_DisplayUnits)
	{
	case S3_UNITS_DBM :	g = -maxip - 22; break;
	case S3_UNITS_DBUV:	g = -(maxip - 106) - 22; break;
	case S3_UNITS_MV:	g = -(10.0 * log10(10.0 * maxip * maxip)) - 22; break;
	}

	return (int)g;
}

// ----------------------------------------------------------------------------

int S3SetUnits(unsigned char Units)
{
#ifdef S3_AGENT
    CString Command, Args, Response, InZ;
    Command = L"UNITS";

    switch(Units)
    {
    case S3_UNITS_DBM:
        Args = L" DBM";
        break;
    case S3_UNITS_DBUV:
        Args = L" DBUV";
        break;
    case S3_UNITS_MV:
        Args = L" MV";
        break;
    }

    Command.Append(Args);

    Response = SendSentinel3Message(Command);
#else
	if (Units != 0xFF)
		S3Data->m_DisplayUnits = Units;

	double maxip;

	maxip = S3CalcMaxIP(S3Data->m_Config.m_Gain);
	S3Data->m_Config.m_MaxInput = maxip;

	for (char Rx = 0; Rx < S3_MAX_RXS; Rx++)
	{
		maxip = S3CalcMaxIP(S3Data->m_Rx[Rx].m_Config.m_Gain);
		S3Data->m_Rx[Rx].m_Config.m_MaxInput = maxip;
		
		for (char Tx = 0; Tx < S3_MAX_TXS; Tx++)
		{
			maxip = S3CalcMaxIP(S3Data->m_Rx[Rx].m_Tx[Tx].m_Config.m_Gain);
			S3Data->m_Rx[Rx].m_Tx[Tx].m_Config.m_MaxInput = maxip;
			
			for (char IP = 0; IP < S3_MAX_IPS; IP++)
			{
				maxip = S3CalcMaxIP(S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_Gain);
				S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_MaxInput = maxip;

				double p1db = S3CalcP1dB(S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_Gain);
				S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_P1dB = p1db;
			}
		}
	}
#endif
	return 0;
}

// ----------------------------------------------------------------------------

unsigned char S3GetUnits()
{
	return	S3Data->m_DisplayUnits;
}

// ----------------------------------------------------------------------------

wchar_t *S3GetUnitString()
{
	// TEMP: Fix for file reads
	if (S3Data->m_DisplayUnits == 0)
		S3Data->m_DisplayUnits = 1;

	return UnitStrings[S3Data->m_DisplayUnits - 1];
}

// ----------------------------------------------------------------------------

wchar_t *S3GetUnitStrings(unsigned char i)
{
	return UnitStrings[i - 1];
}

// ----------------------------------------------------------------------------

int S3TestToneAll(unsigned char On)
{
	// Initialise
	for (char Rx = 0; Rx < S3_MAX_RXS; Rx++)
	{
		S3RxData	*pRx = &(S3Data->m_Rx[Rx]);

		if (S3RxExistQ(Rx))
		{
			for (char Tx = 0; Tx < S3_MAX_TXS; Tx++)
			{
				if (S3TxExistQ(Rx, Tx)) // t[Tx])
				{
					for (char IP = 0; IP < S3_MAX_IPS; IP++)
					{
						if (S3IPExistQ(Rx, Tx, IP))
						{
							// S3CalSignal(Rx, Tx, IP, On != 0);
							S3IPSetTestToneEnable(Rx, Tx, IP, On);
						}
					}
				}
			}
		}
	}

	return 0;
}



// ----------------------------------------------------------------------------

int S3CalSignal(char Rx, char Tx, char IP, unsigned char On)
{
	// I2C

	return 0;
}

// ----------------------------------------------------------------------------

const char *S3GetConfigName()
{
	return S3Data->m_ConfigName;
}

// ---------------------------------------------------------------------------

const char *S3GetEventLogName()
{
	return S3Data->m_EventLogName;
}

// ---------------------------------------------------------------------------

int S3SetRemote(bool remote)
{
	S3Data->m_Remote = remote;

	if (S3Data->m_Remote)
	{
		//S3SetSelected(-1, -1, -1);
	}

	*S3Data->m_TestName = '\0';

	return 0;
}

// ---------------------------------------------------------------------------

bool S3GetRemote()
{
	return S3Data->m_Remote;
}

// ---------------------------------------------------------------------------

void S3SetUSBOpen(bool open)
{
	S3Data->m_USBOpen = open;
}

bool S3GetUSBOpen()
{
	return (S3Data->m_USBOpen == true);
}


// ---------------------------------------------------------------------------

unsigned short S3TxGetAlarms(char Rx, char Tx)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_Alarms;
}

// ---------------------------------------------------------------------------

unsigned char S3TxBattGetAlarms(char Rx, char Tx)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_BattAlarms;
}

// ---------------------------------------------------------------------------

unsigned char S3ChGetAlarms(char Ch)
{
	return S3Data->m_Chargers[Ch].m_Alarms;
}

// ---------------------------------------------------------------------------
// This determines the Rx6 Tx shown larger at the bottom - not the active or
// selected one. There is no other significance.
char S3RxGetHighlightedTx(char Rx)
{
	return S3Data->m_Rx[Rx].m_SelectedTx;
}

// ---------------------------------------------------------------------------
// Returns the value even if something else (ie a Tx) is selected below

char S3GetCurrentRx(void)
{
	return S3Data->m_SelectedRx;
}

// ---------------------------------------------------------------------------
// Only returns value if nothing else (ie a Tx) is selected. CF S3GetCurrentRx.

char S3GetSelectedRx(void)
{
	if (S3Data->m_SelectedTx != -1)
		return -1;

	return S3Data->m_SelectedRx;
}

// ---------------------------------------------------------------------------

void S3RxSetHighlightedTx(char Rx, char Tx)
{
	if (S3Data->m_Rx[Rx].m_Type != S3_Rx2 && S3Data->m_Rx[Rx].m_Type != S3_Rx6
		|| Tx < 0)
		return;

	if (S3Data->m_Rx[Rx].m_Type == S3_Rx2 && Tx > 1)
		return;

	if (S3Data->m_Rx[Rx].m_Type == S3_Rx6 && Tx > 5)
		return;

	if (S3Data->m_Rx[Rx].m_Tx[Tx].m_Type != S3_TxUnconnected)
		S3Data->m_Rx[Rx].m_SelectedTx = Tx;
}

// ---------------------------------------------------------------------------

int S3SysSetPN(const char *s)
{
	strcpy_s(S3Data->m_PN, S3_MAX_PN_LEN, s);

	return 0;
}

const char *S3SysGetPN()
{
	return S3Data->m_PN;
}

// ---------------------------------------------------------------------------

const char *S3SysGetModel()
{
	return S3Data->m_ModelId;
}

int S3SysSetModel(const char *s)
{
	strcpy_s(S3Data->m_ModelId, S3_MAX_MODEL_ID_LEN, s);

	return 0;
}

// ---------------------------------------------------------------------------

int S3SysSetSW(const char *s)
{
	strcpy_s(S3Data->m_SW, S3_MAX_SW_VER_LEN, s);

	if (!S3GetSoftShutdownOption())
	{
		if (strlen(S3Data->m_SW) < S3_MAX_SW_VER_LEN - 1)
			strcat_s(S3Data->m_SW, S3_MAX_SW_VER_LEN, "h");
	}

	return 0;
}

const char *S3SysGetSW()
{
	return S3Data->m_SW;
}

// ---------------------------------------------------------------------------

const char *S3SysGetImageDate()
{
	return S3Data->m_ImageDate;
}

const char *S3SysGetImageTime()
{
	return S3Data->m_ImageTime;
}

// ---------------------------------------------------------------------------

const char *S3SysGetAppDateTime()
{
	return S3Data->m_AppDateTime;
}

// ---------------------------------------------------------------------------
int S3SysSetSN(const char *s)
{
	strcpy_s(S3Data->m_SN, S3_MAX_SN_LEN, s);

	return 0;
}

// ---------------------------------------------------------------------------

const char *S3SysGetSN()
{
	return S3Data->m_SN;
}

// ---------------------------------------------------------------------------

const char *S3GetSelNodeName()
{
	return S3GetNodeName(S3Data->m_SelectedRx, S3Data->m_SelectedTx, -1);
}

// ---------------------------------------------------------------------------

int S3RxGetInfoStr(char *info, char Rx)
{
	assert(info != NULL);

	char tmp[S3_MAX_INFO_STR_LEN];

	S3ConfigGetInfoStr(tmp, Rx, -1, -1);

	if (strlen(S3Data->m_Rx[Rx].m_NodeName))
		sprintf_s(info, S3_MAX_INFO_STR_LEN, "%s:\n%s",
			S3Data->m_Rx[Rx].m_NodeName, tmp);
	else
		sprintf_s(info, S3_MAX_INFO_STR_LEN, "%s:\n%s",
			"Unnamed", tmp);

	return 0;
}


// ---------------------------------------------------------------------------

int S3SoftwareUpdate()
{
	// S3OSImageUpdate();
	
	char *eptr;
	double d = strtod(S3Data->m_SW, &eptr);

	d += 0.1;

	sprintf_s(S3Data->m_SW, S3_MAX_SW_VER_LEN, "%.2f", d);
	
	return 0;
}

// ---------------------------------------------------------------------------
// Mark all Txs for sleep or waking

int	S3SetSleepAll(bool sleep)
{
#ifdef S3_AGENT
	if (S3Data->m_SleepAll == sleep)
		return 0;
	
	S3Data->m_SleepAll = sleep;
	
	if (sleep)
	{
		CString Response = SendSentinel3Message(L"SLEEPALL");
	}
	else
	{
		CString Response = SendSentinel3Message(L"WAKEALL");
	}

	return 0;
#else
	if (S3Data->m_SleepAll == sleep)
		return 0;
	
	S3Data->m_SleepAll = sleep;
	
	if (sleep)
	{
		S3TxSetPowerStat(-1, -1, S3_TX_SLEEP);
	}
	else
	{
		S3TxSetPowerStat(-1, -1, S3_TX_ON);
	}

	return 0;

#endif
}

// ---------------------------------------------------------------------------

int	S3SetWakeAll(bool wake)
{
#ifdef S3_AGENT
	if (S3Data->m_WakeAll == wake)
		return 0;
	
	S3Data->m_WakeAll = wake;
	
	if (wake)
	{
		CString Response = SendSentinel3Message(L"WAKEALL");
	}
	else
	{
		CString Response = SendSentinel3Message(L"SLEEPALL");
	}

	return 0;
#else
	if (S3Data->m_WakeAll == wake)
		return 0;
	
	S3Data->m_WakeAll = wake;
	
	if (wake)
	{
		S3TxSetPowerStat(-1, -1, S3_TX_ON);
	}
	else
	{
		S3TxSetPowerStat(-1, -1, S3_TX_SLEEP);
	}

	return 0;
#endif
}

// ---------------------------------------------------------------------------

bool S3GetSleepAll()
{
	return S3Data->m_SleepAll;
}

// ---------------------------------------------------------------------------

bool S3GetWakeAll()
{
	return S3Data->m_WakeAll;
}

// ---------------------------------------------------------------------------

bool S3AllAsleep()
{
	char Rxs;
		
	if (S3GetDemoMode())
		Rxs = 1;
	else
		Rxs = S3_MAX_RXS;

	for(char Rx = 0; Rx < Rxs; Rx++)
	{
		if (S3RxGetType(Rx) != S3_RxEmpty)
		{
			for(char Tx = 0; Tx < S3_MAX_TXS; Tx++)
			{
				if (S3TxGetType(Rx, Tx) != S3_TxUnconnected)
				{
					if (S3TxGetPowerStat(Rx, Tx) != S3_TX_SLEEP)
						return false;
				}
			}
		}
	}

	S3Data->m_SleepAll = false;

	return true;
}

// ---------------------------------------------------------------------------

bool S3AllAwake()
{
	char Rxs = S3_MAX_RXS;;
		
	for(char Rx = 0; Rx < Rxs; Rx++)
	{
		if (S3RxGetType(Rx) != S3_RxEmpty)
		{
			for(char Tx = 0; Tx < S3_MAX_TXS; Tx++)
			{
				if (S3TxGetType(Rx, Tx) != S3_TxUnconnected)
				{
					if (S3TxGetPowerStat(Rx, Tx) != S3_TX_ON)
						return false;
				}
			}
		}
	}

	S3Data->m_WakeAll = false;

	return true;
}

// ---------------------------------------------------------------------------

int S3SysSetBuildNum(const char *bn)
{
	strcpy_s(S3Data->m_BuildNum, S3_MAX_BUILD_NUM_LEN, bn);

	return 0;
}

// ---------------------------------------------------------------------------

const char *S3SysGetBuildNum()
{
	return S3Data->m_BuildNum;
}

// ---------------------------------------------------------------------------

unsigned char S3GetAGC()
{
	return S3Data->m_AGC;
}

// ---------------------------------------------------------------------------

int S3SetAGC(unsigned char AGC)
{
	if (AGC >= 100)
		S3Data->m_AGC = AGC - 100;
	else
		S3Data->m_AGC = AGC;

	// Backwards compatibility only
	// OBSOLETE
	for(char Rx = 0; Rx < S3_MAX_RXS; Rx++)
	{
		S3RxSetAGC(Rx, 0, AGC);
		S3RxSetAGC(Rx, 1, AGC);
	}

	return 0;
}

// ---------------------------------------------------------------------------

int S3SetTCompMode(unsigned char ContMode)
{
#ifdef S3_AGENT
    CString Command, Args, Response;
    Command = L"TCOMPMODE";
    switch(ContMode)
    {
    default:
    case 0: 
        Args = L" OFF";
        break;
    case 1: 
        Args = L" CONT";
        break;
    case 2: 
        Args = L" GAIN";
        break;
    }
    Command.Append(Args);

    Response = SendSentinel3Message(Command);	
#else
	if (ContMode < 100)
	{
		// Make pending
		if (ContMode != S3Data->m_ContTComp)
		{
			S3Data->m_ContTComp = ContMode;
			S3TxSetTCompMode(-1, -1, ContMode + 100);
		}
	}
	else
	{
		// Already pending (but may have changed)
		S3Data->m_ContTComp = ContMode - 100;
		S3TxSetTCompMode(-1, -1, ContMode);
	}
#endif

	return 0;
}

// ---------------------------------------------------------------------------

unsigned char S3GetTCompMode()
{
	return S3Data->m_ContTComp;
}

// ---------------------------------------------------------------------------

int S3TempComp()
{
#ifdef S3_AGENT
    CString Command, Args, Response;
    Command = L"TCOMP";
    Args = L" ALL";
	Command.Append(Args);

    Response = SendSentinel3Message(Command);
#endif
	return 0;
}

// ---------------------------------------------------------------------------

int S3DoComp(char Rx, char Tx)
{
	// TODO refer to: m_ContGainTComp

	if (Rx == -1)
	{
		for(Rx = 0; Rx < S3_MAX_RXS; Rx++)
			for(Tx = 0; Tx < S3RxGetNTx(Rx); Tx++)
				S3TxDoComp(Rx, Tx);
	}
	else if (Tx == -1)
	{
		for(Tx = 0; Tx < S3RxGetNTx(Rx); Tx++)
			S3TxDoComp(Rx, Tx);
	}
	else
	{
		S3TxDoComp(Rx, Tx);
	}

	return 0;

}

// ---------------------------------------------------------------------------

bool S3GetDemoMode()
{
	return S3Data->m_DemoMode;
}

// ---------------------------------------------------------------------------

int S3SetDemoMode(bool DemoMode)
{
	S3Init(DemoMode);

	return 0;
}

// ---------------------------------------------------------------------------
// Disable Rx polling and switch to Rx[0], Tx[0]

int S3SetFactoryMode(char Rx, char Tx, bool mode)
{
#ifndef S3_AGENT
	bool OldMode = S3Data->m_FactoryMode;
	S3Data->m_FactoryMode = mode;

	if (mode == true)
	{
		// Special case where may specify a channel rather than a Tx
		if (Rx != -1 && S3RxGetType(Rx) != S3_Rx2)
		{
			if (Tx != -1)
				if (!S3TxExistQ(Rx, Tx))
					return 1;
		}

		if (Rx != -1)
			if (S3I2CRxMS(Rx))
				return 1;

		if (S3I2CChMS(0))
			return 1;

		S3I2CSetUpOptAddr(Rx, Tx);

		if (Tx != -1)
			if (S3I2CRxSwitchTx(Rx, Tx))
				return 1;
	}

	if (mode != OldMode)
	{
		if (S3Data->m_FactoryMode)
			S3EventLogAdd("S3SetFactoryMode: Entered factory mode", 1, Rx, Tx, -1);
		else
			S3EventLogAdd("S3SetFactoryMode: Closed factory mode", 1, Rx, Tx, -1);
	}

#endif

	return 0;
}

// ---------------------------------------------------------------------------

bool S3GetFactoryMode()
{
	return S3Data->m_FactoryMode;
}

// ---------------------------------------------------------------------------

void S3SetPrevRxedMsg(const char *Msg)
{
	strcpy_s(S3Data->m_PreviousRecievedMessage, S3_MAX_MESSAGE_LEN, Msg);
}

// ---------------------------------------------------------------------------

const char* S3GetPrevRxedMsg()
{
	return S3Data->m_PreviousRecievedMessage;
}

// ---------------------------------------------------------------------------

char S3GetPrevRemoteSrc()
{
	return S3Data->m_PrevMsgSrc;
}

// ---------------------------------------------------------------------------

void S3SetPrevRemoteSrc(char MsgSrc)
{
	S3Data->m_PrevMsgSrc = MsgSrc;
}

// ---------------------------------------------------------------------------
// TODO: Only while gain-change T compensation is not a standard feature

bool S3GetTCompGainOption()
{
	return S3Data->m_TCompGainOption;
}

// ---------------------------------------------------------------------------

bool S3GetWinTrackOption()
{
	return S3Data->m_WinTrackOption;
}

// ---------------------------------------------------------------------------
// Should be per Tx
//bool S3TxGetOverdriveOption(char Rx, char Tx)
//{
//	return S3Data->m_Rx[Rx].m_Tx[Tx].m_OverdriveOption;
//}

// ---------------------------------------------------------------------------

bool S3GetLowNoiseOption()
{
	return S3Data->m_LowNoiseOption;
}

// ---------------------------------------------------------------------------

bool S3GetSoftShutdownOption()
{
	return S3Data->m_SoftShutdownOption;
}

// ---------------------------------------------------------------------------

unsigned char S3GetTxStartState()
{
	return S3Data->m_TxStartState;
}

int S3SetTxStartState(unsigned char state)
{
	S3Data->m_TxStartState = state;

#ifdef S3_AGENT

    CString Command, Args, Response, Taustr;
    Command = L"TXSTARTSTATE";

    if (state == S3_TXSTART_USER)
        Args.Format(_T(" USER"));
    else if (state == S3_TXSTART_SLEEP)
        Args.Format(_T(" SLEEP"));
    else if (state == S3_TXSTART_ON)
        Args.Format(_T(" ON"));

    Command.Append(Args);
    Response = SendSentinel3Message(Command);

#endif
	return 0;
}

// ---------------------------------------------------------------------------

bool S3GetTxSelfTest()
{
	return S3Data->m_TxSelfTest;
}

int S3SetTxSelfTest(bool on)
{
	S3Data->m_TxSelfTest = on;

#ifdef S3_AGENT
    CString Command, Args, Response, Taustr;
    Command = L"TXSELFTEST";

    if(on)
        Args.Format(_T(" ON"));
    else
        Args.Format(_T(" OFF"));

    Command.Append(Args);
    Response = SendSentinel3Message(Command);
#endif

	return 0;
}


// ---------------------------------------------------------------------------

int	S3SetDateTime(	unsigned short Hour,
					unsigned short Minute,
					unsigned short Second,
					unsigned short Year,
					unsigned short Month,
					unsigned short Day)
{
	S3Data->wHour = Hour;
	S3Data->wMinute = Minute;
	S3Data->wSecond = Second;

	S3Data->wYear = Year;
	S3Data->wMonth = Month;
	S3Data->wDay = Day;

	return 0;
}

// ---------------------------------------------------------------------------
