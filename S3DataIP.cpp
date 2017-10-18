//
// TODO: Make Rx/Tx/IP indexing char and use -1 to indicate non-existence

#include "stdafx.h"
#include <assert.h>
#include <float.h>

#include "S3SystemDetails.h"
#include "S3DataModel.h"
#include "S3GPIB.h"
#include "S3I2C.h"
#include "S3Gain.h"

#ifdef S3_AGENT
#include "S3Agent/S3AgentDlg.h" 
#endif

extern pS3DataModel S3Data;

// ----------------------------------------------------------------------------
// Obsolete

pS3IPData S3IPGetPtr( char Rx, char Tx, char IP)
{
	return &(S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP]);
}

// ----------------------------------------------------------------------------

int S3IPInit(pS3IPData node)
{
	memset(node, 0, sizeof(S3IPData));

	S3ConfigInit(&node->m_Config);

	node->m_Alarms = 0x00;
	node->m_Para = -1;

	node->m_MaxInput = 0.0;
	node->m_P1dB = 0.0;

	node->m_PrevZ = node->m_Config.m_InputZ;

	node->m_RFLevel = SHRT_MIN;
	node->m_RFGain = SHRT_MIN;

	return 0;
}

// ----------------------------------------------------------------------------

int S3IPSetNodeName(const char *Node, char *NodeName)
{
	pS3TxData TxHd = &(S3Data->m_Rx[Node[0] - 1].m_Tx[Node[1] - 1]);
	pS3IPData IP = &(TxHd->m_Input[Node[2] - 1]);
	strcpy_s(IP->m_NodeName, S3_MAX_NODE_NAME_LEN, NodeName);
	
	return 0;
}

// ----------------------------------------------------------------------------
// Toggle select/deselect of a parameter. This is no longer limited to just
// Tx input parameters.
//
// The selected m_Para also ties in text/num edit callback to the appropriate
// modifier function of S3DataModel via the callback function
//		S3NumEdit::OnEnUpdate() ->
//		CS3GDIScreenMain::S3GDITextSupplied() ->
//		S3IPSetParaTxt()

int S3SetSelectedPara(char Rx, char Tx, char IP, char Para)
{
	// System parameter
	if (Rx == -1)
	{
		S3Data->m_Para = Para;
		return 0;
	}
	
	// Rx parameter
	if (Tx == -1)
	{
		if (S3Data->m_Rx[Rx].m_Para == Para)
		{
			S3Data->m_Rx[Rx].m_Para = -1;
			return 0; // De-select
		}
		else
		{	
			S3Data->m_Rx[Rx].m_Para = Para;
			return 1; // Select
		}
	}

	// Tx parameter
	if (IP == -1)
	{
		if (S3Data->m_Rx[Rx].m_Tx[Tx].m_Para == Para)
		{
			S3Data->m_Rx[Rx].m_Tx[Tx].m_Para = -1;
			return 0; // De-select
		}
		else
		{	
			S3Data->m_Rx[Rx].m_Tx[Tx].m_Para = Para;
			return 1; // Select
		}
	}

	// Tx IP parameter
	if (S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Para == Para)
	{
		S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Para = -1;
		return 0; // De-select
	}
	else
	{	
		S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Para = Para;
		return 1; // Select
	}

	return 0;
}

// ----------------------------------------------------------------------------
// Multiple choice item from menu selection

int S3SetParaValue(	char Rx, char Tx, char IP, char Para, char MenuItem)
{
	switch(Para)
	{
		case S3_SIGMA_TAU:
			if (MenuItem == 0)
			{
				S3SetSigmaTau(Rx, Tx, IP, TauNone);
				break;
			}
			else if (MenuItem == 1)
			{
				S3SetSigmaTau(Rx, Tx, IP, TauLo);
				break;
			}
			else if (MenuItem == 2)
			{
				S3SetSigmaTau(Rx, Tx, IP, TauMd);
				break;
			}
			else if (MenuItem == 3)
			{
				S3SetSigmaTau(Rx, Tx, IP, TauHi);
				break;
			}
			break;
		case S3_INPUT_IMP:
			if (MenuItem == 0)
			{
				S3SetImpedance(Rx, Tx, IP, W50);
				break;
			}
			else if (MenuItem == 1)
			{
				S3SetImpedance(Rx, Tx, IP, W1M);
				break;
			}
			break;
		case S3_TEST_TONE:
			if (MenuItem == 0)
			{
				S3IPSetTestToneEnable(Rx, Tx, IP, 0);
				break;
			}
			else if (MenuItem == 1)
			{
				S3IPSetTestToneEnable(Rx, Tx, IP, 1);
				break;
			}
			break;
		case S3_LOW_NOISE:
			if (MenuItem == 0)
			{
				S3SetLowNoiseMode(Rx, Tx, IP, false);
				break;
			}
			else if (MenuItem == 1)
			{
				S3SetLowNoiseMode(Rx, Tx, IP, true);
				break;
			}
			break;
		case S3_ALARM_LED:

#ifdef S3_AGENT
			{
				CString Command, Args, Response;
                Command = L"S3CLROVERDRIVE";
                Args.Format(_T(" %d %d %d"), (Rx + 1), (Tx + 1), (IP + 1));
                
                Command.Append(Args);

                Response = SendSentinel3Message(Command);
			}
#else
			S3IPCancelAlarm(Rx, Tx, IP, S3_IP_OVERDRIVE);
			S3TxClearPeakHold(Rx, Tx, 0);
#endif
			break;
		case S3_OS_UPDATE:
			// S3SoftwareUpdate();
			break;
		case S3_APP_UPDATE:
			// S3SoftwareUpdate();
			break;
		case S3_TX_POWER_MODE:
			if (MenuItem == 0)
			{
				S3TxSetPowerStat(Rx, Tx, S3_TX_ON);
				S3TxSetUserSleep(Rx, Tx, false);
			}
			else if (MenuItem == 1)
			{
				S3TxSetPowerStat(Rx, Tx, S3_TX_SLEEP);
				S3TxSetUserSleep(Rx, Tx, true);
			}
			else if (MenuItem == 2)
			{
				// Item only added if S3GetTxSelfTest() == true
				S3TxSetSelfTestPending(Rx, Tx, true);
			}
			break;
		case S3_TX_DO_COMP:
			if (MenuItem == 0)
				S3TxDoComp(Rx, Tx);
			break;
		case S3_TX_CANCEL_ALARM:
			if (MenuItem == 0)
				S3TxCancelCurAlarm(Rx, Tx);
			break;
		case S3_RX_CANCEL_ALARM:
			if (MenuItem == 0)
				S3RxCancelCurAlarm(Rx);
			break;
		//case S3_RXRX_AGC:
		//	// Not used
		//	if (MenuItem == 0)
		//		S3RxSetAGC(Rx, Tx, !S3RxGetAGC(Rx, Tx));
		//	break;
		case S3_TX_TESTTONE_ALL:
			if (MenuItem == 0)
				S3TxSetTestToneEnableAll(Rx, Tx, 1);
			else
				S3TxSetTestToneEnableAll(Rx, Tx, 0);
			break;
	}

	return 0;
}

// ----------------------------------------------------------------------------

int S3SetNodeNameNew(char Rx, char Tx, char IP, char *NodeName)
{
#ifdef S3_AGENT
	CString Command = L"NAME", Name(NodeName), Args;
    
    Name.Remove(' ');

    if (Name.IsEmpty())
        return 0;

	if (Rx == -1)
		return 0;

	if (Tx == -1)
	{
        Args.Format(_T(" %d "), (Rx + 1));
        Command.Append(Args);
        Command.Append(Name);
        CString Response = SendSentinel3Message(Command);
		return 0;
	}

	if (IP == -1)
	{
        Args.Format(_T(" %d %d "), (Rx + 1), (Tx + 1));
        Command.Append(Args);
        Command.Append(Name);
        CString Response = SendSentinel3Message(Command);
		return 0;
	}

    Args.Format(_T(" %d %d %d "), (Rx + 1), (Tx + 1), (IP + 1));
    Command.Append(Args);
    Command.Append(Name);
    CString Response = SendSentinel3Message(Command);
	return 0;
#else
	if (Rx == -1)
		return 0;

	if (Tx == -1)
	{
		strcpy_s(S3Data->m_Rx[Rx].m_NodeName,
			S3_MAX_NODE_NAME_LEN, NodeName);
		return 0;
	}

	if (IP == -1)
	{
		strcpy_s(S3Data->m_Rx[Rx].m_Tx[Tx].m_NodeName,
			S3_MAX_NODE_NAME_LEN, NodeName);
		return 0;
	}

	strcpy_s(S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_NodeName,
		S3_MAX_NODE_NAME_LEN, NodeName);

	return 0;
#endif
}

// ----------------------------------------------------------------------------

int S3IPSetParaTxt(	char Rx, char Tx, char IP, char Para, const wchar_t *Txt)
{
	wchar_t	*eptr;
	long	lgain;
	int		err = 0;

	char ctmp[S3_MAX_EDIT_LEN];
	sprintf_s(ctmp, S3_MAX_EDIT_LEN, "%S", Txt);
	
	switch(Para)
	{
		case S3_TXIP_NODENAME: // IP from Tx screen
				S3SetNodeNameNew(Rx, Tx, IP, ctmp);
			break;
		case S3_RXTX_NODENAME: // Tx from rx screen
				S3SetNodeNameNew(Rx, Tx, -1, ctmp);
			break;
		case S3_TXTX_NODENAME: // Tx from Tx screen
				S3SetNodeNameNew(Rx, Tx, -1, ctmp);
			break;
		case S3_RXRX_NODENAME: // Rx from rx screen
				S3SetNodeNameNew(Rx, -1, -1, ctmp);
			break;
		case S3_GAIN:
			lgain = wcstol(Txt, &eptr, 10);
			
            if (lgain > SCHAR_MAX)
                lgain = SCHAR_MAX;
            else if (lgain < SCHAR_MIN)
                lgain = SCHAR_MIN;

			if (S3SetGain(Rx, Tx, IP, (char)lgain))
				return 1;
			break;
		case S3_MAX_INPUT:
			/*
			{
				double mpow = strtod(Txt, &eptr);
				if (S3IPSetMaxInput(Rx, Tx, IP, mpow))
					return 1;
			}
			*/
			break;
		case S3_IP_PORT:
			{
				unsigned short port = (unsigned short)wcstol(Txt, &eptr, 10);
				err = S3SetIPPort(port);
			}
			break;
		case S3_IP_ADDRESS:
			{
				err = S3SetIPAddrStr(Txt, true);
			}
			break;
		case S3_IP_SUBNET:
			{
				err = S3SetIPSubnetStr(Txt, true);
			}
			break;
		case S3_TIME_EDIT:
			// int err = S3SetSysTimeStr(Txt);

			break;
	}

	return err;
}

// ----------------------------------------------------------------------------

char S3GetSelectedPara(char Rx, char Tx, char IP)
{
	if (Rx == -1)
	{
		return S3Data->m_Para;
	}

	if (Tx == -1)
	{
		return S3Data->m_Rx[Rx].m_Para;
	}
	
	if (IP == -1)
	{
		return S3Data->m_Rx[Rx].m_Tx[Tx].m_Para;
	}

	return S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Para;
}

// ----------------------------------------------------------------------------
// TODO: Deprecate and use S3IPInvalidQ()

int S3IPValidQ(char Rx, char Tx, char IP)
{
	if (Rx < 0 || Tx < 0 || IP < 0)
		return 0;

	if (!S3RxExistQ(Rx))
		return 0;

	if (!S3TxExistQ(Rx, Tx))
		return 0;
	
	switch (S3Data->m_Rx[Rx].m_Tx[Tx].m_Type)
	{
	case S3_Tx1:
		if (IP > 0)
			return 0;
		break;
	case S3_Tx8:
		if (IP > 7)
			return 0;
		break;
	}

	return 1;
}

// ----------------------------------------------------------------------------
// Return reason for invalid address or 0 for good
int S3IPInvalidQ(char Rx, char Tx, char IP)
{
	if (!S3RxExistQ(Rx))
		return S3_GPIB_RX_NOT_EXIST;

	if (!S3TxExistQ(Rx, Tx))
		return S3_GPIB_TX_NOT_EXIST;

	if (IP == -1)
		return S3_GPIB_INVALID_IP;
	
	switch (S3Data->m_Rx[Rx].m_Tx[Tx].m_Type)
	{
	case S3_Tx1:
		if (IP > 0)
			return S3_GPIB_IP_NOT_EXIST;
		break;
	case S3_Tx8:
		if (IP > 7)
			return S3_GPIB_INVALID_IP;
		break;
	}

	return 0;
}

// ----------------------------------------------------------------------------

int S3IPSetGain(pS3IPData pIP, int gain)
{
	if (gain < S3_MIN_GAIN || gain > S3_MAX_GAIN)
		return 1;

	double maxip = S3CalcMaxIP(gain);

	pIP->m_Config.m_Gain = gain;
	pIP->m_MaxInput = maxip;

	pIP->m_P1dB = S3CalcP1dB(gain);
	
	return 0;
}

// ----------------------------------------------------------------------------
// TODO: Make generic according to args

int S3IPGetGain(char Rx, char Tx, char IP)
{
	if (Rx == -1)
		return S3Data->m_Config.m_Gain;
	
	if (Tx == -1)
		return S3_NULL_GAIN;

	if (S3Data->m_Rx[Rx].m_Tx[Tx].m_Type == S3_TxUnconnected)
		return S3_NULL_GAIN;

	return S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_Gain;
}

// ----------------------------------------------------------------------------
// TODO: These always go together so combine

// Store gain actually transmitted
char S3IPGetGainSent(char Rx, char Tx, char IP)
{
	return (char)S3Data->m_GainSent[Rx][Tx][IP];
}

// ----------------------------------------------------------------------------

int S3IPSetGainSent(char Rx, char Tx, char IP, char Gain)
{
	S3Data->m_GainSent[Rx][Tx][IP] = (int)Gain;

	if (Gain == SCHAR_MIN)
		S3Data->m_PathSent[Rx][Tx][IP] = SCHAR_MIN;

	return 0;
}

// ----------------------------------------------------------------------------
// Store path actually transmitted - this may change without a gain change
char S3IPGetPathSent(char Rx, char Tx, char IP)
{
	return S3Data->m_PathSent[Rx][Tx][IP];
}

// ----------------------------------------------------------------------------

int S3IPSetPathSent(char Rx, char Tx, char IP, char Path)
{
	S3Data->m_PathSent[Rx][Tx][IP] = Path;

	return 0;
}

// ----------------------------------------------------------------------------
// TODO: Refer to auto-temperature compensation setting
// Record Tx module temperature when setting gain, detect drift from this
// temperature wrt limits and alert user.

int S3ApplyGainLimits(char Rx, char Tx, char IP)
{
	return S3SetGain(Rx, Tx, IP, 
		S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_Gain);
}


// ----------------------------------------------------------------------------

int S3IPSetMaxInput(char Rx, char Tx, char IP, double maxip)
{
#ifdef S3_AGENT
	int gain = S3IPCalcGain(maxip);

	char low, high;

	S3GetGainLimits(Rx, Tx, IP, &low, &high);

	// TODO: ? This is treated as an error - different to S3SetGain();
	if (gain < low || gain > high)
		return 1;

    CString Command, Args, Response;
    Command = L"MAXIP";
    Args.Format(_T(" %d %d %d %f"), (Rx + 1), (Tx + 1), (IP + 1), maxip);
    
    Command.Append(Args);

    Response = SendSentinel3Message(Command);

	return 0;
#else
	int gain = S3IPCalcGain(maxip);

	char low, high;

	S3GetGainLimits(Rx, Tx, IP, &low, &high);

	// TODO: ? This is treated as an error - different to S3SetGain();
	if (gain < low || gain > high)
		return 1;

	pS3IPData pIP = &S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP];

	pIP->m_Config.m_Gain = gain;
	pIP->m_MaxInput = maxip;
	pIP->m_P1dB = S3CalcP1dB(gain);
	
	return 0;
#endif
}

// ----------------------------------------------------------------------------

double S3IPGetMaxInput(char Rx, char Tx, char IP)
{
	if (Rx == -1 || Tx == -1)
		return DBL_MAX;

	// TODO: Substitute proper calc
	// double maxip = S3CalcMaxIP(S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_Gain);

	return S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_MaxInput;
}

// ----------------------------------------------------------------------------

double S3IPGetP1dB(char Rx, char Tx, char IP)
{
	if (Rx == -1 || Tx == -1)
		return DBL_MAX;

	// TODO: Substitute proper calc
	// double maxip = S3CalcMaxIP(S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_Gain);

	return S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_P1dB;
}


// ----------------------------------------------------------------------------
// If we have to change the impedance, then keep the previous setting and apply
// if turning off the integrator.

int S3IPSetSigmaTau(char Rx, char Tx, char IP, SigmaT Tau)
{
#ifdef S3_AGENT
	if (S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_Tau == Tau)
        return 0;

	int GainChanged = 0;
	CString Command, Args, Response, Taustr;
    Command = L"ITAU";
    switch(Tau)
    {
    case TauNone:
        Taustr = L"OFF";
        break;
    case TauLo:
        Taustr = L"LO";
        break;
    case TauMd:
        Taustr = L"MED";
        break;
    case TauHi:
        Taustr = L"HI";
        break;
    }

    Args.Format(_T(" %d %d %d %s"), (Rx + 1), (Tx + 1), (IP + 1), Taustr);
    
    Command.Append(Args);

    Response = SendSentinel3Message(Command);

    if(Response.CompareNoCase(L"W: Command required gain setting to be adjusted"))
        GainChanged = 1;

	return GainChanged;
#else
	if (S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_Tau == Tau)
		return 0;
	
	SigmaT PrevTau = S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_Tau;

	S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_Tau = Tau;

	// Apply limits according to RF path constraints
	if (Tau != TauNone)
	{
		if (PrevTau == TauNone)
			S3IPSetPrevZ(Rx, Tx, IP, S3IPGetImpedance(Rx, Tx, IP));
		
		S3IPSetImpedance(Rx, Tx, IP, W1M);

		S3IPSetLowNoiseMode(Rx, Tx, IP, false);
	}
	else
	{
		// Restore previous setting
		S3IPSetImpedance(Rx, Tx, IP, S3IPGetPrevZ(Rx, Tx, IP));
	}
	
	int GainChanged = S3ApplyGainLimits(Rx, Tx, IP);
	
	// I2C: Trigger update
	S3IPSetGainSent(Rx, Tx, IP, SCHAR_MIN);

	return GainChanged;
#endif
}

// ----------------------------------------------------------------------------

int S3IPSetLowNoiseMode(char Rx, char Tx, char IP, bool On)
{
	if (S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_LowNoiseMode == On)
		return 0;

	S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_LowNoiseMode = On;

	int GainChanged = 0;

	if (On)
	{
		S3IPSetSigmaTau(Rx, Tx, IP, TauNone);
		S3SetImpedance(Rx, Tx, IP, W50);

		GainChanged = S3ApplyGainLimits(Rx, Tx, IP);
	}

	// I2C: Trigger update
	S3IPSetGainSent(Rx, Tx, IP, SCHAR_MIN);

	return GainChanged;
}

// ----------------------------------------------------------------------------

InputZ S3IPGetImpedance(char Rx, char Tx, char IP)
{
	if (Rx == -1)
		return S3Data->m_Config.m_InputZ;
	
	if (Tx == -1)
		return ZUnknown;

	return S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_InputZ;
}

// ----------------------------------------------------------------------------

unsigned char	S3IPGetLowNoiseMode(char Rx, char Tx, char IP)
{
	if (Rx == -1 || Tx == -1)
		return 2; // Error?

	return S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_LowNoiseMode;
}

// ----------------------------------------------------------------------------

SigmaT S3IPGetSigmaTau(char Rx, char Tx, char IP)
{
	if (Rx == -1 || Tx == -1)
		return TauUnknown;

	return S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_Tau;
}

// ----------------------------------------------------------------------------

unsigned char S3IPGetWindowTrack(char Rx, char Tx, char IP)
{
	if (Rx == -1 || Tx == -1)
		return 2; // Error

	return S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_WindowTracking;
}

// ----------------------------------------------------------------------------
// Determines gain limits based on RF path. 
// TODO: Integrate this with I2C path setting code
int S3GetGainLimits(char Rx, char Tx, char IP, char *low, char *high)
{
	*high = S3_MAX_GAIN;
	*low = S3_MIN_GAIN;

	SigmaT	Tau;
	InputZ	Z; 
	bool	LNMode;

	if (Rx == -1) // System defaults
	{
		Tau = S3Data->m_Config.m_Tau;
		Z = S3Data->m_Config.m_InputZ;
		LNMode = S3Data->m_Config.m_LowNoiseMode;
	}
	else if (IP != -1)
	{
		Tau = S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_Tau;
		Z =  S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_InputZ;
		LNMode = S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_LowNoiseMode;
	}
	else return 1;

	if (LNMode)
	{
		// Path 2
		*low = 0;
		*high = 20;

		return 0;
	}

	if (Tau == TauNone)
	{
		// Path 7
		if (Z == W1M)
		{
			*low = -30 + S3TxGetAttenGainOffset(Rx, Tx);
			*high = 0 + S3TxGetAttenGainOffset(Rx, Tx);
		}
		else
		{
			// Path 3 gain <= 0
			// Path 1 gain > 0
		}
	}		
	else
	{
		// Paths 4, 5, 6
		*high = 0 + S3TxGetAttenGainOffset(Rx, Tx);
	}

	return 0;
}

// ----------------------------------------------------------------------------
// Keep function aligned with S3SetSigmaTau().

int S3SetImpedance(char Rx, char Tx, char IP, InputZ z)
{
#ifdef S3_AGENT
	int GainChanged = 0;

    CString Command, Args, Response, InZ;
    Command = L"IPZ";

    switch(z)
    {
    case W50:
        InZ = L"50";
        break;
    case W1M:
        InZ = L"HIZ";
        break;
    }
    if(Rx != -1)
    {
        Args.Format(_T(" %d %d %d %s"), (Rx + 1), (Tx + 1), (IP + 1), InZ);
    }
    else
    {
        Args.Format(_T(" DEFAULT %s"), InZ);
    }
    
    Command.Append(Args);

    Response = SendSentinel3Message(Command);

    if(Response.CompareNoCase(L"W: Command required gain setting to be adjusted"))
        GainChanged = 1;

	return GainChanged;
#else
	int GainChanged = 0;


	if (Rx == -1)
	{		
		if (S3Data->m_Config.m_InputZ != z)
		{
			S3Data->m_Config.m_InputZ = z;
		
			// Update gain limits if necessary
			S3SetGain(Rx, Tx, IP, S3Data->m_Config.m_Gain);
		}
	}
	else if (Tx == -1)
	{
		// Shouldn't be used
		S3Data->m_Rx[Rx].m_Config.m_InputZ = z;
	}
	else
	{
		if (S3IPGetAlarms(Rx, Tx, IP) & S3_IP_OVERDRIVE)
		 	return 2;

		GainChanged = S3IPSetImpedance(Rx, Tx, IP, z);
	}

	return GainChanged;
#endif
}

// ----------------------------------------------------------------------------
// This is tied into Gain (LNAs and PADs), High-Z enabled etc. Uses SetGain()
// to update gain limits if necessary and kick off I2C comms. Same for
// S3SetImpedance

int S3SetSigmaTau(char Rx, char Tx, char IP, SigmaT Tau)
{
#ifdef S3_AGENT
	int GainChanged = 0;

    CString Command, Args, Response;
    Command = L"ITAU";
    Args.Format(_T(" %d %d %d %d"), (Rx + 1), (Tx + 1), (IP + 1), Tau);
    
    Command.Append(Args);

    Response = SendSentinel3Message(Command);

    if(Response.CompareNoCase(L"W: Command required gain setting to be adjusted"))
        GainChanged = 1;

	return GainChanged;
#else
	int GainChanged = 0;

	if (S3IPGetAlarms(Rx, Tx, IP) & S3_IP_OVERDRIVE)
	 	return 2;

	if (Rx == -1)
	{		
		if (S3Data->m_Config.m_Tau != Tau)
		{
			S3Data->m_Config.m_Tau = Tau;
			S3SetGain(Rx, Tx, IP, S3Data->m_Config.m_Gain);
		}
	}
	else if (Tx == -1)
	{
		// Shouldn't be used
		S3Data->m_Rx[Rx].m_Config.m_Tau = Tau;
	}
	else
	{
		GainChanged = S3IPSetSigmaTau(Rx, Tx, IP, Tau);
	}

	return GainChanged;
#endif
}

// ----------------------------------------------------------------------------

int S3IPSetImpedance(char Rx, char Tx, char IP, InputZ z)
{
#ifdef S3_AGENT
	int GainChanged = 0;
	
	if (S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_InputZ == z)
		return 0;

    CString Command, Args, Response, InZ;
    Command = L"IPZ";

    switch(z)
    {
    case W50:
        InZ = L"50";
        break;
    case W1M:
        InZ = L"HIZ";
        break;
    }

    Args.Format(_T(" %d %d %d %s"), (Rx + 1), (Tx + 1), (IP + 1), InZ);
    
    Command.Append(Args);

    Response = SendSentinel3Message(Command);

    if(Response.CompareNoCase(L"W: Command required gain setting to be adjusted"))
        GainChanged = 1;

	return GainChanged;
#else
	int GainChanged = 0;
	
	if (S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_InputZ == z)
		return 0;

	S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_InputZ = z;
	
	if (z == W50)
		S3IPSetSigmaTau(Rx, Tx, IP, TauNone);
	else
		S3IPSetLowNoiseMode(Rx, Tx, IP, false);

	GainChanged = S3ApplyGainLimits(Rx, Tx, IP);

	// I2C: Invalidate
	S3IPSetGainSent(Rx, Tx, IP, SCHAR_MIN);

	return GainChanged;
#endif
}

// ----------------------------------------------------------------------------

int S3IPSetPrevZ(char Rx, char Tx, char IP, InputZ z)
{
	S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_PrevZ = z;

	return 0;
}

// ----------------------------------------------------------------------------

InputZ S3IPGetPrevZ(char Rx, char Tx, char IP)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_PrevZ;
}

// ----------------------------------------------------------------------------

int S3IPCal(char Rx, char Tx, char IP, int On)
{
	// S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_CalSig = On;

	// I2C:

	return 0;
}

// ----------------------------------------------------------------------------

int S3IPSetSigmaTauS(char Rx, char Tx, char IP, const char *Tau)
{
	if (!STRCMP(Tau, "OFF"))
		S3IPSetSigmaTau(Rx, Tx, IP, TauNone);
	else if (!STRCMP(Tau, "LO"))
		S3IPSetSigmaTau(Rx, Tx, IP, TauLo);
	else if (!STRCMP(Tau, "MED"))
		S3IPSetSigmaTau(Rx, Tx, IP, TauMd);
	else if (!STRCMP(Tau, "HI"))
		S3IPSetSigmaTau(Rx, Tx, IP, TauHi);
	else return 1;

	return 0;
}

// ----------------------------------------------------------------------------

int S3IPGetSigmaTauS(char *str, char Rx, char Tx, char IP)
{
	SigmaT tau = S3IPGetSigmaTau(Rx, Tx, IP);
	
	switch(tau)
	{
	case TauNone:	strcpy_s(str, S3_PARA_NAME_LEN, "OFF"); break;
	case TauLo:		strcpy_s(str, S3_PARA_NAME_LEN, "LO"); break;
	case TauMd:		strcpy_s(str, S3_PARA_NAME_LEN, "MED"); break;
	case TauHi:		strcpy_s(str, S3_PARA_NAME_LEN, "HI"); break;
	default:		strcpy_s(str, S3_PARA_NAME_LEN, "UNKNOWN"); break;
	}

	return 0;
}

// ----------------------------------------------------------------------------

int S3IPGetInputZS(char *str, char Rx, char Tx, char IP)
{
	InputZ z = S3IPGetImpedance(Rx, Tx, IP);

	switch(z)
	{
	case W50:		strcpy_s(str, S3_PARA_NAME_LEN, "50"); break;
	case W1M:		strcpy_s(str, S3_PARA_NAME_LEN, "1M"); break;
	case ZUnknown:	strcpy_s(str, S3_PARA_NAME_LEN, "UNKNOWN"); break;
	default:		strcpy_s(str, S3_PARA_NAME_LEN, "ERROR"); break;
	}
	
	return 0;
}
// ----------------------------------------------------------------------------

int S3IPWindowTrack(char Rx, char Tx, char IP, unsigned char On)
{
	S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_WindowTracking =
		(On == 1);

	// I2C

	return 0;
}

// ----------------------------------------------------------------------------

int S3IPSetTestToneEnable(char Rx, char Tx, char IP, char Enable)
{
#ifdef S3_AGENT
	CString Command, Args, Response, InZ;
    Command = L"TONE";

    if(Enable)
    {
        Args.Format(_T(" %d %d %d ON"), (Rx + 1), (Tx + 1), (IP + 1));
    }
    else
    {
        Args.Format(_T(" %d %d %d OFF"), (Rx + 1), (Tx + 1), (IP + 1));
    }

    Command.Append(Args);

    Response = SendSentinel3Message(Command);

	return 0;
#else
	if (S3IPGetAlarms(Rx, Tx, IP) & S3_IP_OVERDRIVE)
	 	return 2;

	if (Enable >= 2 * S3_PENDING) // ASSERT:
		Enable -= S3_PENDING;
	
	if (Enable >= S3_PENDING)
	{
		S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_TestToneEnable = Enable;
		return 0;
	}
	
	if (S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_TestToneEnable >= S3_PENDING)
	{
		S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_TestToneEnable = Enable;
		return 0;
	}

	if (S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_TestToneEnable != Enable)
	{
		S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_TestToneEnable = Enable + S3_PENDING;
		return 0;
	}

	return 0;
#endif
}

// ----------------------------------------------------------------------------

char S3IPGetTestToneEnable(char Rx, char Tx, char IP)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_TestToneEnable;
}

// ----------------------------------------------------------------------------

unsigned char S3IPGetAlarms(char Rx, char Tx, char IP)
{
	return S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Alarms;
}

// ---------------------------------------------------------------------------

int S3SetSelected(char Rx, char Tx, char IP)
{
	char orx, otx, oip;

	S3GetSelected(&orx, &otx, &oip);

	int changed = 0;

	if (Rx >= 0 && Tx >= 0 && IP >= 0)
		S3SetSelectedPara(Rx, Tx, IP, -1);

	if (S3Data->m_SelectedRx != Rx)
	{
		S3Data->m_SelectedRx = Rx;
		changed = 1;
	}

	if (S3Data->m_SelectedTx != Tx)
	{
		S3Data->m_SelectedTx = Tx;
		changed = 1;
	}

	if (S3Data->m_SelectedIP != IP)
	{
		S3Data->m_SelectedIP = IP;
		changed = 1;
	}

	// Deselect any previous selection
	S3SetSelectedPara(orx, otx, oip, -1);
	// S3SetSelectedPara(Rx, Tx, IP, -1);

	return 0;
}

// ---------------------------------------------------------------------------

const char *S3GetNodeName(char Rx, char Tx, char IP)
{
	if (Rx == -1)
	{
		return S3Data->m_NodeName;
	}
	else if (Tx == -1)
	{
		return S3Data->m_Rx[Rx].m_NodeName;
	}
	else if (IP == -1)
	{
		return S3Data->m_Rx[Rx].m_Tx[Tx].m_NodeName;
	}
	else
	{
		return S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_NodeName;
	}

	return NULL;
}

// ---------------------------------------------------------------------------

char S3IsSelected(char Rx, char Tx, char IP)
{
	if (IP == -1)
	{
		if (Tx == -1)
		{
			if (S3Data->m_SelectedRx == Rx && S3Data->m_SelectedTx == -1)
				return 1;
		}
		else if (S3Data->m_SelectedRx == Rx && S3Data->m_SelectedTx == Tx)
			return 1;
	}
	else if (	S3Data->m_SelectedRx == Rx &&
				S3Data->m_SelectedTx == Tx &&
				S3Data->m_SelectedIP == IP)
		return 1;

	return 0;
}

// ---------------------------------------------------------------------------
// Return value maybe of some use

char S3GetSelected(char *Rx, char *Tx, char *IP)
{
	*Rx = S3Data->m_SelectedRx;
	*Tx = S3Data->m_SelectedTx;
	*IP = S3Data->m_SelectedIP;

	if (*Rx == -1) // && *Tx == -1 && *IP == -1)
		return 0; // Nothing selected

	if (*Tx == -1)
		return 1; // Rx selected

	if (*IP == -1)
		return 2; // Tx selected

	return 3; // IP selected
}

// ---------------------------------------------------------------------------

int S3IPGetInfoStr(char *info, char Rx, char Tx, char IP)
{
	assert(info != NULL);

	char tmp[S3_MAX_INFO_STR_LEN];

	S3ConfigGetInfoStr(tmp, Rx, Tx, IP);

	if (strlen(S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_NodeName))
		sprintf_s(info, S3_MAX_INFO_STR_LEN, "%s:\n%s",
			S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_NodeName, tmp);
	else
		sprintf_s(info, S3_MAX_INFO_STR_LEN, "%s:\n%s",
			"Unnamed", tmp);

	return 0;
}

// ---------------------------------------------------------------------------

int S3IPSetRFLevel(char Rx, char Tx, char IP, short level)
{
	// if (Rx == -1 || Tx == -1 || IP == -1)
	//	return 1;

	S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_RFLevel = level;
	
	return 0;
}

// ---------------------------------------------------------------------------

int S3IPSetRFGain(char Rx, char Tx, char IP, short gain)
{
	// if (Rx == -1 || Tx == -1 || IP == -1)
	//	return 1;

	S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_RFGain = gain;

	return 0;
}

// ---------------------------------------------------------------------------

short S3IPGetRFLevel(char Rx, char Tx, char IP)
{
	// if (Rx == -1 || Tx == -1 || IP == -1)
	//	return 1;

	if (S3TxGetType(Rx, Tx) == S3_Tx8 && IP != S3TxGetActiveIP(Rx, Tx))
		return SHRT_MIN;

	return S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_RFLevel;
}

// ---------------------------------------------------------------------------

short S3IPGetRFGain(char Rx, char Tx, char IP)
{
	// if (Rx == -1 || Tx == -1 || IP == -1)
	//	return 1;

	return S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_RFGain;
}




