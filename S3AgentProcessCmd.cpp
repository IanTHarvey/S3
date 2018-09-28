
#include "stdafx.h"

#include "S3DataModel.h"
#include "S3GPIB.h"

#define NODE_CLASS_SEPARATOR "\035"
#define PACKETSIZE 1024
extern pS3DataModel S3Data;
class CS3GDIScreenMain;

extern char		GPIBCmdBuf[];
extern char		GPIBRetBuf[];
extern unsigned	GPIBBufLen;
extern char		GPIBCurrentRx;
extern char		GPIBCurrentTx; // For settings etc
extern char		GPIBCurrentIP;

extern unsigned char	GPIBNArgs;
extern char			GPIBCmd[S3_MAX_GPIB_CMD_LEN];
extern char			*GPIBCmdArgs[S3_MAX_ARGS];

int CmdGetAll(void);
int CmdCopyLog(char *Inbuf);

int S3AgentProcessCmd()
{
    int err;
	char *Cmd = GPIBCmdArgs[0];

    if (!STRCMP(Cmd, "S3GETBATT"))      err = CmdGetBatt(GPIBRetBuf);
    else if(!STRCMP(Cmd, "S3GETINIT"))  err = CmdGetInit(GPIBRetBuf);
    else if(!STRCMP(Cmd, "S3GETCONN"))  err = CmdGetConn(GPIBRetBuf);
    else if(!STRCMP(Cmd, "S3GETSYSI"))  err = CmdGetSysI(GPIBRetBuf);
    else if(!STRCMP(Cmd, "S3COPYLOG"))  err = CmdCopyLog(GPIBRetBuf);
    else if(!STRCMP(Cmd, "S3GETRXMOD"))
    {
        if (GPIBNArgs != 2)
        {
            strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Incorrect number of parameters");
		    return S3_GPIB_ERR_NUMBER_PARAS;
        }

        char *cmdRx;
        cmdRx = GPIBCmdArgs[1];
        int Rx = (strtol(cmdRx, &cmdRx, 10)) - 1;
        err = CmdGetRXMod(GPIBRetBuf, Rx);
    }
    else if(!STRCMP(Cmd, "S3GETTXMOD"))
    {
        if (GPIBNArgs != 3)
        {
            strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Incorrect number of parameters");
		    return S3_GPIB_ERR_NUMBER_PARAS;
        }
        char *cmdRx, *cmdTx;
        cmdRx = GPIBCmdArgs[1];
        cmdTx = GPIBCmdArgs[2];
        int Rx = (strtol(cmdRx, &cmdRx, 10)) - 1;
        int Tx = (strtol(cmdTx, &cmdTx, 10)) - 1;
        err = CmdGetTXMod(GPIBRetBuf, Rx, Tx);
    }
    else if(!STRCMP(Cmd, "S3GETINPUT"))
    {
        if (GPIBNArgs != 4)
        {
            strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Incorrect number of parameters");
		    return S3_GPIB_ERR_NUMBER_PARAS;
        }
        char *cmdRx, *cmdTx, *cmdIP;
        cmdRx = GPIBCmdArgs[1];
        cmdTx = GPIBCmdArgs[2];
        cmdIP = GPIBCmdArgs[3];
        int Rx = (strtol(cmdRx, &cmdRx, 10)) - 1;
        int Tx = (strtol(cmdTx, &cmdTx, 10)) - 1;
        int IP = (strtol(cmdIP, &cmdIP, 10)) - 1;
        err = CmdGetInput(GPIBRetBuf, Rx, Tx, IP);
    }
    else if(!STRCMP(Cmd, "S3GETALL"))  err = CmdGetAll();
    else if(!STRCMP(Cmd, "S3CLEARCURALARM"))
    {
        if((GPIBNArgs < 2) || (GPIBNArgs > 3))
        {
            strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Incorrect number of parameters");
		    return S3_GPIB_ERR_NUMBER_PARAS;
        }

        char *cmdRx, *cmdTx;
        cmdRx = GPIBCmdArgs[1];
        int Rx = (strtol(cmdRx, &cmdRx, 10)) - 1;
        if(GPIBNArgs == 2)
        {
            S3RxCancelCurAlarm(Rx);
        }
        if(GPIBNArgs == 3)
        {
            cmdTx = GPIBCmdArgs[2];
            int Tx = (strtol(cmdTx, &cmdTx, 10)) - 1;
            S3TxCancelCurAlarm(Rx, Tx);
        }
    }
    else if(!STRCMP(Cmd, "S3CLROVERDRIVE"))
    {
        if (GPIBNArgs != 4)
        {
            strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Incorrect number of parameters");
		    return S3_GPIB_ERR_NUMBER_PARAS;
        }
        char *cmdRx, *cmdTx, *cmdIP;
        cmdRx = GPIBCmdArgs[1];
        cmdTx = GPIBCmdArgs[2];
        cmdIP = GPIBCmdArgs[3];
        int Rx = (strtol(cmdRx, &cmdRx, 10)) - 1;
        int Tx = (strtol(cmdTx, &cmdTx, 10)) - 1;
        int IP = (strtol(cmdIP, &cmdIP, 10)) - 1;
        err = S3IPCancelAlarm(Rx, Tx, IP, S3_IP_OVERDRIVE);
        S3TxClearPeakHold(Rx, Tx, 0);
    }
    else	strcpy_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "E: Unrecognised command");

    if(!err)
    {
        //Indicate Success & output
        //strcat_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN - strlen(GPIBRetBuf), "Ok");
    }
    return err;
}


int CmdGetBatt(char *Inbuf)
{
    int i, valid[4], TTC[4], amp[4], temp[4];
    char SoC[4], type[4];
    double volt[4];
    for(i = 0; i < 4; i++)
    {
        if(S3Data->m_Chargers[i].m_Occupied)
        {
            if(S3Data->m_Chargers[i].m_BattValidated)
            {
                valid[i] = 1;
            }
            else
            {
                valid[i] = 0;
            }
        }
        else
        {
            valid[i] = -1;
        }
        SoC[i] = S3Data->m_Chargers[i].m_SoC;
        TTC[i] = S3Data->m_Chargers[i].m_ATTF;
        volt[i] = S3Data->m_Chargers[i].m_V;
        amp[i] = S3Data->m_Chargers[i].m_I;
        type[i] = S3Data->m_Chargers[i].m_Type;
        temp[i] = S3Data->m_Chargers[i].m_BattTemp;
    }

    sprintf_s(Inbuf, S3_MAX_GPIB_RET_LEN,
                " %i\037 %i\037 %i\037 %f\037 %i\037 %i\037 %s\037 %s\037 %s\037 %s\037 %i\037 %s\037 %i\037 %i\037 %i\036"
                " %i\037 %i\037 %i\037 %f\037 %i\037 %i\037 %s\037 %s\037 %s\037 %s\037 %i\037 %s\037 %i\037 %i\037 %i\036"
                " %i\037 %i\037 %i\037 %f\037 %i\037 %i\037 %s\037 %s\037 %s\037 %s\037 %i\037 %s\037 %i\037 %i\037 %i\036"
                " %i\037 %i\037 %i\037 %f\037 %i\037 %i\037 %s\037 %s\037 %s\037 %s\037 %i\037 %s\037 %i\037 %i\037 %i\036",
                valid[0],SoC[0],TTC[0],volt[0],amp[0],type[0],S3Data->m_Chargers[0].m_BattSN,
                                        S3Data->m_Chargers[0].m_BattPN,
                                        S3Data->m_Chargers[0].m_HW,
                                        S3Data->m_Chargers[0].m_FW,temp[0],
                                        S3Data->m_Chargers[0].m_MfrData,
                                        S3Data->m_Chargers[0].m_BattType,
                                        S3Data->m_Chargers[0].m_Alarms,
										S3Data->m_Chargers[0].stat_h,
                valid[1],SoC[1],TTC[1],volt[1],amp[1],type[1],S3Data->m_Chargers[1].m_BattSN,
                                        S3Data->m_Chargers[1].m_BattPN,
                                        S3Data->m_Chargers[1].m_HW,
                                        S3Data->m_Chargers[1].m_FW,temp[1],
                                        S3Data->m_Chargers[1].m_MfrData,
                                        S3Data->m_Chargers[1].m_BattType,
                                        S3Data->m_Chargers[1].m_Alarms,
										S3Data->m_Chargers[1].stat_h,
                valid[2],SoC[2],TTC[2],volt[2],amp[2],type[2],S3Data->m_Chargers[2].m_BattSN,
                                        S3Data->m_Chargers[2].m_BattPN,
                                        S3Data->m_Chargers[2].m_HW,
                                        S3Data->m_Chargers[2].m_FW,temp[2],
                                        S3Data->m_Chargers[2].m_MfrData,
                                        S3Data->m_Chargers[2].m_BattType,
                                        S3Data->m_Chargers[2].m_Alarms,
										S3Data->m_Chargers[2].stat_h,
                valid[3],SoC[3],TTC[3],volt[3],amp[3],type[3],S3Data->m_Chargers[3].m_BattSN,
                                        S3Data->m_Chargers[3].m_BattPN,
                                        S3Data->m_Chargers[3].m_HW,
                                        S3Data->m_Chargers[3].m_FW,temp[3],
                                        S3Data->m_Chargers[3].m_MfrData,
                                        S3Data->m_Chargers[3].m_BattType,
                                        S3Data->m_Chargers[3].m_Alarms,
										S3Data->m_Chargers[3].stat_h);

    return 0;
}

int CmdGetConn(char *Inbuf)
{
    sprintf_s(Inbuf, S3_MAX_GPIB_RET_LEN,
        " %s\037 %s\037 %i\037 %i:%i:%i:%i:%i:%i\037 %i\037 %s\037 %s\037 %i\037 %s", 
        S3Data->m_IPv4Addr,
        S3Data->m_IPv4Subnet,
        S3Data->m_IPPort,
        S3Data->m_MACAddr[0],
        S3Data->m_MACAddr[1],
        S3Data->m_MACAddr[2],
        S3Data->m_MACAddr[3],
        S3Data->m_MACAddr[4],
        S3Data->m_MACAddr[5],
		S3Data->m_DHCPEnabled,
        S3Data->m_DisplayedUSBPort,
        S3Data->m_DisplayedUSBDriver,
        S3Data->m_PrevMsgSrc,
        S3Data->m_PreviousRecievedMessage);
    return 0;
}
int CmdGetSysI(char *Inbuf)
{
    CTime CurrentTime = CTime::GetCurrentTime();
    CString str1, str2;
    //GetDateStr(str1);
    	str1.Format(_T("%02d-%02d-%02d"), CurrentTime.GetYear(), CurrentTime.GetMonth(), CurrentTime.GetDay());
    	str2.Format(_T("%02d:%02d:%02d"), CurrentTime.GetHour(), CurrentTime.GetMinute(), CurrentTime.GetSecond());
    //GetTimeStr(str2);

    sprintf_s(Inbuf, S3_MAX_GPIB_RET_LEN,
        " %s\037 %s\037 %s\037 %s\037 %s\037 %s\037 %s\037 %s\037 %02d-%02d-%02d\037 %02d:%02d:%02d\037"
		" %i\037 %s\037 %f\037 %s\037 %s\037 %s\037 %s\037"
        " %s\037 %i\037 %i\037 %i\037 %i\037 %i\037 %i\037 %i\037 %i\037 %f\037 %i\037"
		" %s\037 %s\037 %s\037 %i\037 %i\037 %i\037 %i\037 %i",
        S3Data->m_NodeName,	// 0
        S3Data->m_SN,
        S3Data->m_PN,
        S3Data->m_HW,
        S3Data->m_SW,
        S3Data->m_ImageDate,
        S3Data->m_BuildNum,
        S3Data->m_ModelId,
        CurrentTime.GetYear(), CurrentTime.GetMonth(), CurrentTime.GetDay(),
        CurrentTime.GetHour(), CurrentTime.GetMinute(), CurrentTime.GetSecond(),	// 9

        S3GetRemote(),
        S3Data->m_ConfigName,
        S3Data->m_FileVersion,
        S3Data->m_ConfigPath,
        S3Data->m_EventLogName,
        S3Data->m_EventLogPath,
        S3Data->m_TestName,			// 16
        
        S3Data->m_AppDateTime,
        S3Data->m_Type,
        S3Data->m_TCompGainOption,
        S3Data->m_WinTrackOption,
        S3Data->m_LowNoiseOption,
        S3Data->m_SoftShutdownOption,
        S3Data->m_AGC,
        S3Data->m_TxStartState,
        S3Data->m_TxSelfTest,
        S3Data->m_SWVersionD,
		S3Data->m_Terminator,		// 27

        S3Data->m_ImageID,
        S3Data->m_ImageOS,
        S3Data->m_ImageTime,
        S3Data->m_OSUpdateFail,
        S3Data->m_PowerDownPending,
        S3Data->m_PowerDownFailed,
        S3Data->m_SleepAll,
		S3Data->m_Locked);			// 35
    return 0;
}

// ----------------------------------------------------------------------------

int CmdGetInit(char *Inbuf)
{
    sprintf_s(Inbuf, S3_MAX_GPIB_RET_LEN,
         " %i\037 %i\037 %i\037 %i\037 %i\037 %i\037 %i\037 %i\037 %i\037 %f\037 %i",
        S3Data->m_Config.m_Gain,
        S3Data->m_ContTComp,
        (int)S3Data->m_Config.m_Tau,
        (int)S3Data->m_Config.m_InputZ,
        (int)S3Data->m_Config.m_LowNoiseMode,
        S3Data->m_DisplayUnits,
		S3Data->m_DisplayScale,
		S3Data->m_SigSize,
		S3Data->m_3PCLinearity,
        S3Data->m_Config.m_MaxInput,
        (int)S3Data->m_Config.m_WindowTracking);
    return 0;
}

// ----------------------------------------------------------------------------

int CmdGetRXMod(char *Inbuf, int Rx)
{
    if ((Rx != -1 && S3RxValidQ(Rx) && S3Data->m_Rx[Rx].m_Type != S3_RxEmpty))
	{
        sprintf_s(Inbuf, S3_MAX_GPIB_RET_LEN,
            " %i\037 %i\037 %s\037 %i\037 %i\037 %i\037"
            " %s\037 %s\037 %s\037 %s\037 %s\037"
            " %i\037 %i\037 %i\037 %i\037 %i\037 %i\037"		// RLL
            " %i\037 %i\037 %i\037 %i\037 %i\037 %i\037"		// RFGain
            " %i\037 %i\037 %i\037 %i\037 %i\037 %i\037"		// Link gain
            " %i\037 %i\037 %i\037 %i\037 %i\037 %i\037 %i\037 %i\037 %i\037"
            " %i\037 %f\037 %i\037 %i\037 %i\037 %i\037"
			" %i\037 %i\037 %i\037"								// RxCtrl alarms
            " %i\037 %i\037 %i\037 %i\037 %i\037 %i\037"		// Tx alarms
            " %i\037 %i\037 %i\037 %i\037 %i\037 %s\037"
            " %i\037 %i\037 %i\037 %i\037 %i\037 %i"
			" %i\037",											// Extra gain,
            S3Data->m_Rx[Rx].m_Type,
            S3Data->m_Rx[Rx].m_Detected,
            S3Data->m_Rx[Rx].m_NodeName,
            S3Data->m_Rx[Rx].m_Id,
            S3Data->m_Rx[Rx].m_SelectedTx,
            S3Data->m_Rx[Rx].m_ActiveTx,

            S3Data->m_Rx[Rx].m_SN,
            S3Data->m_Rx[Rx].m_PN,
            S3Data->m_Rx[Rx].m_FW,
            S3Data->m_Rx[Rx].m_HW,
            S3Data->m_Rx[Rx].m_ModelName,

            S3Data->m_Rx[Rx].m_RLL[0], S3Data->m_Rx[Rx].m_RLL[1], S3Data->m_Rx[Rx].m_RLL[2],
                    S3Data->m_Rx[Rx].m_RLL[3], S3Data->m_Rx[Rx].m_RLL[4], S3Data->m_Rx[Rx].m_RLL[5],
            S3Data->m_Rx[Rx].m_RFGain[0], S3Data->m_Rx[Rx].m_RFGain[1], S3Data->m_Rx[Rx].m_RFGain[2],
                    S3Data->m_Rx[Rx].m_RFGain[3], S3Data->m_Rx[Rx].m_RFGain[4], S3Data->m_Rx[Rx].m_RFGain[5],
            S3Data->m_Rx[Rx].m_LinkGain[0], S3Data->m_Rx[Rx].m_LinkGain[1], S3Data->m_Rx[Rx].m_LinkGain[2],
                    S3Data->m_Rx[Rx].m_LinkGain[3], S3Data->m_Rx[Rx].m_LinkGain[4], S3Data->m_Rx[Rx].m_LinkGain[5],

            S3Data->m_Rx[Rx].m_CalGain[0],
            S3Data->m_Rx[Rx].m_CalGain[1],
            S3Data->m_Rx[Rx].m_Vcc,
            S3Data->m_Rx[Rx].m_AGC[0],
            S3Data->m_Rx[Rx].m_AGC[1],
            S3Data->m_Rx[Rx].m_Alarms,
            S3Data->m_Rx[Rx].m_Temp,
            S3Data->m_Rx[Rx].m_TempHi,
            S3Data->m_Rx[Rx].m_TempLo,
            
            S3Data->m_Rx[Rx].m_Config.m_Gain,
            S3Data->m_Rx[Rx].m_Config.m_MaxInput,
            S3Data->m_Rx[Rx].m_Config.m_Tau,
            S3Data->m_Rx[Rx].m_Config.m_InputZ,
            S3Data->m_Rx[Rx].m_Config.m_LowNoiseMode,
            S3Data->m_Rx[Rx].m_Config.m_WindowTracking,

			S3Data->m_Rx[Rx].m_RxAlarms[0],
            S3Data->m_Rx[Rx].m_RxAlarms[1],
            S3Data->m_Rx[Rx].m_RxAlarms[2],
            
            S3Data->m_Rx[Rx].m_TxAlarms[0],
            S3Data->m_Rx[Rx].m_TxAlarms[1],
            S3Data->m_Rx[Rx].m_TxAlarms[2],
            S3Data->m_Rx[Rx].m_TxAlarms[3],
            S3Data->m_Rx[Rx].m_TxAlarms[4],
            S3Data->m_Rx[Rx].m_TxAlarms[5],
            
            S3Data->m_Rx[Rx].m_CurAlarmSrc,
            S3Data->m_Rx[Rx].m_CurAlarm,
            S3Data->m_Rx[Rx].m_RLLHi,
            S3Data->m_Rx[Rx].m_RLLLo,
            S3Data->m_Rx[Rx].m_Fmax,
            S3Data->m_Rx[Rx].m_FWDate,
            
            S3Data->m_Rx[Rx].m_RFLevel[0],
            S3Data->m_Rx[Rx].m_RFLevel[1],
            S3Data->m_Rx[Rx].m_RFLevel[2],
            S3Data->m_Rx[Rx].m_RFLevel[3],
            S3Data->m_Rx[Rx].m_RFLevel[4],
            S3Data->m_Rx[Rx].m_RFLevel[5],

			S3Data->m_Rx[Rx].m_ExtraGainCap);
    }
    else
    {
        sprintf_s(Inbuf, S3_MAX_GPIB_RET_LEN, "E: Invalid Address");
    }

    return 0;
}

// ----------------------------------------------------------------------------

int CmdGetTXMod(char *Inbuf, int Rx, int Tx)
{
    //If it is a valid address
    if ((Rx != -1 && S3RxValidQ(Rx) && S3Data->m_Rx[Rx].m_Type != S3_RxEmpty) &&
		(Tx != -1 && S3TxValidQ(Rx, Tx) && S3Data->m_Rx[Rx].m_Tx[Tx].m_Type != S3_TxUnconnected))
    {
        sprintf_s(Inbuf, S3_MAX_GPIB_RET_LEN,
            " %i\037 %i\037 %s\037 %i\037 %i\037 %i\037"
            " %s\037 %s\037 %s\037 %s\037 %s\037"
            " %s\037 %s\037 %s\037 %s\037"
            " %i\037 %i\037 %i\037 %i\037 %i\037"
            " %i\037 %i\037 %i\037 %i\037"
            " %i\037 %i\037 %i\037 %i\037"
            " %i\037 %i\037 %i\037 %i\037 %i\037 %i\037 %i\037 %i\037"	// Alarms
            " %i\037 %f\037 %i\037 %i\037 %i\037 %i\037"				// Config
            " %i\037 %i\037 %i\037 %s\037 %i\037 %i\037 %i\037"
            " %i\037 %i\037 %i\037 %i\037 %i\037"
            " %f\037 %f\037 %f\037 %f",
            S3Data->m_Rx[Rx].m_Tx[Tx].m_Type,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_Detected,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_NodeName,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_Id,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_ParentId,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_Wavelength,					// 6

            S3Data->m_Rx[Rx].m_Tx[Tx].m_SN,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_PN,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_FW,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_HW,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_ModelName,					// 11

            S3Data->m_Rx[Rx].m_Tx[Tx].m_BattSN,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_BattPN,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_BattHW,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_BattFW,						// 15

            S3Data->m_Rx[Rx].m_Tx[Tx].m_BattTemp,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_BattValidated,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_SoC,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_ATTE,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_PowerStat,					// 20

            S3Data->m_Rx[Rx].m_Tx[Tx].m_ActiveInput,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_TestSigInput,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_TempTx,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_TempComp,					// 24

            S3Data->m_Rx[Rx].m_Tx[Tx].m_LaserPow,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_LaserLo,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_LaserHi,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_CompMode,					// 28
            
            S3Data->m_Rx[Rx].m_Tx[Tx].m_Alarms,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_OptAlarms[0],
            S3Data->m_Rx[Rx].m_Tx[Tx].m_OptAlarms[1],
            S3Data->m_Rx[Rx].m_Tx[Tx].m_OptAlarms[2],
            S3Data->m_Rx[Rx].m_Tx[Tx].m_CtrlAlarms[0],
            S3Data->m_Rx[Rx].m_Tx[Tx].m_CtrlAlarms[1],
            S3Data->m_Rx[Rx].m_Tx[Tx].m_BattAlarms,
			S3Data->m_Rx[Rx].m_Tx[Tx].m_RLLStableCnt,				// 36

            S3Data->m_Rx[Rx].m_Tx[Tx].m_Config.m_Gain,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_Config.m_MaxInput,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_Config.m_Tau,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_Config.m_InputZ,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_Config.m_LowNoiseMode,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_Config.m_WindowTracking,	// 42
            
            S3Data->m_Rx[Rx].m_Tx[Tx].m_Fmax,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_Uptime,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_CalOpt,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_FWDate,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_I,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_UserSleep,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_EmergencySleep,				// 49
            
            S3Data->m_Rx[Rx].m_Tx[Tx].m_CurAlarmSrc,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_CurAlarm,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_TempTEC,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_PeakThresh,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_PeakHold,					//54
            
            S3Data->m_Rx[Rx].m_Tx[Tx].m_Tau_ns[0],
            S3Data->m_Rx[Rx].m_Tx[Tx].m_Tau_ns[1],
            S3Data->m_Rx[Rx].m_Tx[Tx].m_Tau_ns[2],
            S3Data->m_Rx[Rx].m_Tx[Tx].m_Tau_ns[3]);					// 58
    }
    else
    {
        sprintf_s(Inbuf, S3_MAX_GPIB_RET_LEN, "E: Invalid Address");
    }

    return 0;
}
int CmdGetInput(char *Inbuf, int Rx, int Tx, int IP)
{
    //If it is a valid address
    if (!S3IPInvalidQ(Rx, Tx, IP))
	{
        sprintf_s(Inbuf, S3_MAX_GPIB_RET_LEN,
            " %s\037 %f\037 %f\037 %i\037"
            " %i\037 %f\037 %i\037 %i\037 %i\037 %i\037"
            " %i\037 %i\037 %i",
            S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_NodeName,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_P1dB,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_MaxInput,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Alarms,


            S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_Gain,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_MaxInput,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_Tau,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_InputZ,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_LowNoiseMode,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config.m_WindowTracking,

            S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_RFLevel,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_RFGain,
            S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_TestToneEnable);
    }
    else
    {
        sprintf_s(Inbuf, S3_MAX_GPIB_RET_LEN, "E: Invalid Address");
    }

    return 0;
}



int CmdGetAll()
{   
    char tempstr[S3_MAX_GPIB_RET_LEN], finalbuf[S3_MAX_GPIB_RET_LEN];
    CString GPIBmsg;

    CmdGetBatt(finalbuf);
    strcat_s(finalbuf, NODE_CLASS_SEPARATOR);
    //strcat(finalbuf, tempstr); Not needed - this first call went straight into finalbuf

    CmdGetConn(tempstr);
    strcat_s(finalbuf, NODE_CLASS_SEPARATOR);
    strcat_s(finalbuf, tempstr);

    CmdGetSysI(tempstr);
    strcat_s(finalbuf, NODE_CLASS_SEPARATOR);
    strcat_s(finalbuf, tempstr);

    CmdGetInit(tempstr);
    strcat_s(finalbuf, NODE_CLASS_SEPARATOR);
    strcat_s(finalbuf, tempstr);
    
    int Rx, Tx, Ip;
    for(Rx = 0; Rx < S3_MAX_RXS; Rx++)
    {
        CmdGetRXMod(tempstr, Rx);
        strcat_s(finalbuf, NODE_CLASS_SEPARATOR);
        strcat_s(finalbuf, tempstr);
        for(Tx = 0; Tx < S3_MAX_TXS; Tx++)
        {
            CmdGetTXMod(tempstr, Rx, Tx);
            strcat_s(finalbuf, NODE_CLASS_SEPARATOR);
            strcat_s(finalbuf, tempstr);
            for(Ip = 0; Ip < S3_MAX_IPS; Ip++)
            {
                CmdGetInput(tempstr, Rx, Tx, Ip);
                strcat_s(finalbuf, NODE_CLASS_SEPARATOR);
                strcat_s(finalbuf, tempstr);
            }
        }
    }
    sprintf_s(GPIBRetBuf, S3_MAX_GPIB_RET_LEN, "%s", finalbuf);
    return 0;
}

int CmdCopyLog(char *Inbuf)
{
    if (GPIBNArgs < 2)
    {
		strcpy_s(Inbuf, S3_MAX_GPIB_RET_LEN, "E: Incorrect Number of Parameters");
        return 0;
    }

    char *cmd;
    cmd = GPIBCmdArgs[1];
    
    char FullPathName[S3_MAX_FILENAME_LEN];
    char Response[S3_MAX_GPIB_RET_LEN];

    sprintf_s(FullPathName, S3_MAX_FILENAME_LEN, "%s\\%s.s3l",
	    S3Data->m_EventLogPath, S3Data->m_EventLogName);

    FILE *fid;
    errno_t err = fopen_s(&fid, FullPathName, "rb");

    if (err)
    {
	    strcpy_s(Inbuf, S3_MAX_GPIB_RET_LEN, "E: No Log File To Copy");
	    return 0;
    }
    
    fseek(fid, 0, SEEK_END);
    long filesize = ftell(fid);
    fseek(fid, 0, SEEK_SET); // Rewind not in CE CRT?

    long MaxPacketNo = filesize/PACKETSIZE + 1;

    if(!STRNCMP(cmd, "SIZE", 10))
    {
        sprintf_s(Response, S3_MAX_GPIB_RET_LEN, "%d", MaxPacketNo);
        strcpy_s(Inbuf, S3_MAX_GPIB_RET_LEN, Response);

	    fclose(fid);
    }
    else 
    {
        //Remote agent requests it chunk by chunk.
        int packet = strtol(cmd, &cmd, 10);
        if((packet <= MaxPacketNo) && (packet > 0))
        {
            char PacketPayload[PACKETSIZE];
            fseek(fid, ((packet - 1)*PACKETSIZE), SEEK_SET);
            
            long result = fread(&PacketPayload, 1, PACKETSIZE , fid);
            fclose(fid);
            if(result != PACKETSIZE && packet != MaxPacketNo)
            {
                strcpy_s(Inbuf, S3_MAX_GPIB_RET_LEN, "E: Read Error");
                return 0;
            }
            else if(result != PACKETSIZE && packet == MaxPacketNo)
            {
                PacketPayload[result] = '\0';
            }
            strcpy_s(Inbuf, S3_MAX_GPIB_RET_LEN, PacketPayload);

        }
        else
        {
            strcpy_s(Inbuf, S3_MAX_GPIB_RET_LEN, "E: Invalid Packet requested");
        }
    }
    return 0;
}

int CmdCancelCurAlarm(char *Inbuf)
{
    int Rx = -1, Tx = -1;
    if (GPIBNArgs < 2)
        strcpy_s(Inbuf, S3_MAX_GPIB_RET_LEN, "E: Incorrect number of parameters");
	    return S3_GPIB_ERR_NUMBER_PARAS;

    if (GPIBNArgs == 2)
	{

        char *cmd;
        cmd = GPIBCmdArgs[1];
        int Rx = (strtol(cmd, &cmd, 10)) - 1;
    }
    

    return 0;
}
