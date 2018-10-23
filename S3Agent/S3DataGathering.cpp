#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stdafx.h"

#include "afxwin.h"

#include "S3DataGathering.h"

#include <ctime>

#define S3_MAX_DATAGATHER_ATTEMPTS 4
#define S3_REFRESH_INTERVAL_MS 500
#define S3_REFRESH_INTERVAL_SERIAL_MS 5000

#define DATA_ITEM_SEPARATOR "\037"
#define NODE_SEPARATOR "\036"
#define NODE_CLASS_SEPARATOR "\035"

extern pS3DataModel S3Data;

UINT AutoUpdateSentinelDataBundleThread(LPVOID pParam)
{
    HWND *phObjectHandle = static_cast<HWND *>(pParam);
    CS3AgentDlg *pObject = (CS3AgentDlg *)pParam;
    
    int failedresultcount = 0, result;

    if (Sentinel3.isConnected == false)
    {
        return 2;
    }
    pObject->AutoCheckS3Data = true;
    pObject->m_AutoUpdateGUITick.SetCheck(true);
    
    //Re-initialise the Sentinel3 Data Model (remove any previous data)
    //Set up the Remote GUI
    pObject->m_S3Data = S3Init(false);
    result = Sentinel3AllDataGather();
    pObject->m_GDIStatic.ShowWindow(SW_SHOW);
    pObject->m_IMGStatic.ShowWindow(SW_HIDE);
    pObject->m_GDIStatic.SetSelection(true);
    
    while ((pObject->AutoCheckS3Data == true) && (Sentinel3.isConnected == true) && (failedresultcount < S3_MAX_DATAGATHER_ATTEMPTS))
    {
        
        result = Sentinel3AllDataGather();
        
        if (result > 0)
        {
            failedresultcount++;
        }
        else
        {
            failedresultcount = 0;
        }

        //Update any needed UI controls (outside the GDI region)
        pObject->m_RemoteLocalCheck.SetCheck(Sentinel3.isRemoteAndManualAccess);

        if(connectionmethod == ETHERNET)
        {
            Sleep(S3_REFRESH_INTERVAL_MS);//Wait S3_REFRESH_INTERVAL_MS before updating again.
        }
        else
        {
            Sleep(S3_REFRESH_INTERVAL_SERIAL_MS);//Wait S3_REFRESH_INTERVAL_MS before updating again.
        }
    }

    if (failedresultcount >= S3_MAX_DATAGATHER_ATTEMPTS)
    {

        CString Query = L"*IDN?";
        CString Response;

        Response = SendSentinel3Message(Query);

        if ((Response.Mid(0, 6).CompareNoCase(L"ERROR:") == 0) || (Response.GetLength() == 0))
        {
            AfxGetMainWnd()->PostMessage(S3_DISCONNECT, 0, 0);
            AfxMessageBox(_T("Connection error: No Sentinel 3 available."));
        }
        else
        {
            //Errors were from the sentinel 3  - therefore it is still connected, but did not recognise the commands
            AfxMessageBox(_T("This Sentinel 3 is incompatible with Sentinel 3 Remote View.\r\nPlease use scripts or manual commands to configure."));
            pObject->m_AutoUpdateGUITick.SetCheck(false);
        }
        
    }
    pObject->m_GDIStatic.ShowWindow(SW_HIDE);
    pObject->m_IMGStatic.ShowWindow(SW_SHOW);
    pObject->AutoCheckS3Data = false;

    return 0;
}

//TODO: Add routines to actually collect data from the Sentinel 3. Need to confirm the command set first. The current command set (Sentinel 3 software version 0.3) does not incorporate enough getter commands to support a remote GUI


int Sentinel3AllDataGather()
{
    int ParamIndex = 0;

    CString Query = L"S3GETALL";
    CString Response, Packet;

    Response = SendSentinel3Message(Query);
    //If we recieve a Comms Error message, or an error message from the Sentinel
    if ((Response.Mid(0, 6).CompareNoCase(L"ERROR:") == 0) || (Response.Mid(0, 2).CompareNoCase(L"E:") == 0) || Response.IsEmpty() || (Response.Mid(0, 3).CompareNoCase(L"OK:") == 0))
    {
        return FAIL;
    }
    int MessageTokenPos = 0;

    Packet = Response.Tokenize(_T(NODE_CLASS_SEPARATOR), MessageTokenPos);
    DecodeChargerDetails(Packet);

    Packet = Response.Tokenize(_T(NODE_CLASS_SEPARATOR), MessageTokenPos);
    DecodeConnectionDetails(Packet);

    Packet = Response.Tokenize(_T(NODE_CLASS_SEPARATOR), MessageTokenPos);
    DecodeSystemDetails(Packet); 

    Packet = Response.Tokenize(_T(NODE_CLASS_SEPARATOR), MessageTokenPos);
    DecodeDefaultsDetails(Packet);

    int Rx, Tx, Ip;
    for(Rx = 0; Rx < S3_MAX_RXS; Rx++)
    {
        Packet = Response.Tokenize(_T(NODE_CLASS_SEPARATOR), MessageTokenPos);
        DecodeRxModuleDetails(Packet, Rx);
        for(Tx = 0; Tx < S3_MAX_TXS; Tx++)
        {
            Packet = Response.Tokenize(_T(NODE_CLASS_SEPARATOR), MessageTokenPos);
            DecodeTxModuleDetails(Packet, Rx, Tx);
            for(Ip = 0; Ip < S3_MAX_IPS; Ip++)
            {
                Packet = Response.Tokenize(_T(NODE_CLASS_SEPARATOR), MessageTokenPos);
                DecodeIPModuleDetails(Packet, Rx, Tx, Ip);
            }
        }
    }

    return SUCCESS;
}

int UpdateSentinelControllerDetails()
{
    int resCON, resSYS, resDEF;

    resCON = UpdateSentinelConnectionDetails();
    resSYS = UpdateSentinelSystemDetails();
    resDEF = UpdateSentinelDefaultsDetails();
    return (resCON || resSYS || resDEF);

}

int UpdateSentinelRxModuleDetails(int Rx_visual)
{
    int result = SUCCESS, ParamIndex = 0, RxIndex = 0;
    int Rx;
    CString Query = L"S3GETRXMOD", Arg;
    CString Response, Line, Data;
    Arg.Format(_T(" %d"), Rx_visual);
    Query.Append(Arg);

    Response = SendSentinel3Message(Query);

    //If we recieve a Comms Error message, or an error message from the Sentinel
    if ((Response.Mid(0, 6).CompareNoCase(L"ERROR:") == 0) || ((Response.Mid(0, 2).CompareNoCase(L"E:") == 0) && (Response.Mid(0, 18).CompareNoCase(L"E: Invalid Address") != 0)))
    {
        return FAIL;
    }
    Rx = Rx_visual - 1;
    DecodeRxModuleDetails(Response, Rx);

    return SUCCESS;
}

int UpdateSentinelTxModuleDetails(int Rx_visual, int Tx_visual)
{
    int result = SUCCESS, ParamIndex = 0, RxIndex = 0;
    int Rx, Tx;
    CString Query = L"S3GETTXMOD", Arg;
    CString Response, Line, Data;
    Arg.Format(_T(" %d %d"), Rx_visual, Tx_visual);
    Query.Append(Arg);

    Response = SendSentinel3Message(Query);

    //If we recieve a Comms Error message, or an error message from the Sentinel
    if ((Response.Mid(0, 6).CompareNoCase(L"ERROR:") == 0) || ((Response.Mid(0, 2).CompareNoCase(L"E:") == 0) && (Response.Mid(0, 18).CompareNoCase(L"E: Invalid Address") != 0)))
    {
        return FAIL;
    }
    Rx = Rx_visual - 1;
    Tx = Tx_visual - 1;
    DecodeTxModuleDetails(Response, Rx, Tx);

    return SUCCESS;
}

int UpdateSentinelIpModuleDetails(int Rx_visual, int Tx_visual, int Ip_visual)
{
    int result = SUCCESS, ParamIndex = 0, RxIndex = 0;
    int Rx, Tx, Ip;
    CString Query = L"S3GETINPUT", Arg;
    CString Response, Line, Data;
    Arg.Format(_T(" %d %d %d"), Rx_visual, Tx_visual, Ip_visual);
    Query.Append(Arg);

    Response = SendSentinel3Message(Query);

    //If we recieve a Comms Error message, or an error message from the Sentinel
    if ((Response.Mid(0, 6).CompareNoCase(L"ERROR:") == 0) || ((Response.Mid(0, 2).CompareNoCase(L"E:") == 0) && (Response.Mid(0, 18).CompareNoCase(L"E: Invalid Address") != 0)))
    {
        return FAIL;
    }
    Rx = Rx_visual - 1;
    Tx = Tx_visual - 1;
    Ip = Ip_visual - 1;

    DecodeIPModuleDetails(Response, Rx, Tx, Ip);

    return SUCCESS;
}

int UpdateSentinelBatteryChargerDetails()
{
    CString Query = L"S3GETBATT ";
    CString Response;

    Response = SendSentinel3Message(Query);

    //If we recieve a Comms Error message, or an error message from the Sentinel
    if ((Response.Mid(0, 6).CompareNoCase(L"ERROR:") == 0) || (Response.Mid(0, 2).CompareNoCase(L"E:") == 0))
    {
        return FAIL;
    }
    DecodeChargerDetails(Response);    
    return SUCCESS;
}
int UpdateSentinelConnectionDetails()
{
    CString Query = L"S3GETCONN ";
    CString Response;

    Response = SendSentinel3Message(Query);

    //If we recieve a Comms Error message, or an error message from the Sentinel
    if ((Response.Mid(0, 6).CompareNoCase(L"ERROR:") == 0) || (Response.Mid(0, 2).CompareNoCase(L"E:") == 0))
    {
        return FAIL;
    }
    DecodeConnectionDetails(Response);
    return SUCCESS;
}
int UpdateSentinelSystemDetails()
{
    CString Query = L"S3GETSYSI";
    CString Response, Line, Data;

    Response = SendSentinel3Message(Query);

    //If we recieve a Comms Error message, or an error message from the Sentinel
    if ((Response.Mid(0, 6).CompareNoCase(L"ERROR:") == 0) || (Response.Mid(0, 2).CompareNoCase(L"E:") == 0))
    {
        return FAIL;
    }
    DecodeSystemDetails(Response);
    return SUCCESS;
}
int UpdateSentinelDefaultsDetails()
{
    CString Query = L"S3GETINIT";
    CString Response, Line, Data;

    Response = SendSentinel3Message(Query);

    //If we recieve a Comms Error message, or an error message from the Sentinel
    if ((Response.Mid(0, 6).CompareNoCase(L"ERROR:") == 0) || (Response.Mid(0, 2).CompareNoCase(L"E:") == 0))
    {
        return FAIL;
    }
    DecodeDefaultsDetails(Response);
    return SUCCESS;
}

//
//
void DecodeDefaultsDetails(CString Response)
{
    CString Line;
    int ParamIndex = 0;
    int MessageTokenPos = 0;
    Line = Response.Tokenize(_T(DATA_ITEM_SEPARATOR), MessageTokenPos);
    
    while (!Line.IsEmpty())
    {   
        switch(ParamIndex)
        {
        case GAIN:
            {
                int gain = _ttoi(Line);
                S3Data->m_Config.m_Gain = gain;
            }
            break;
        case TEMPCOMPENSATION:
            {
                unsigned char tcomp = _ttoi(Line);
                S3Data->m_ContTComp = tcomp;
            }
            break;
        case TIMECONSTANT:
            {
                unsigned char tau = _ttoi(Line);
                S3Data->m_Config.m_Tau = (SigmaT)tau;
            }
            break;
        case INPUTIMPEDANCE:
            {
                unsigned char ipz = _ttoi(Line);
                S3Data->m_Config.m_InputZ = (InputZ)ipz;
            }
            break;
        case LOWNOISEMODE:
            {
                unsigned char lnz = _ttoi(Line);
                S3Data->m_Config.m_LowNoiseMode = (lnz != 0);
            }
            break;
        case UNITS:
            {
                unsigned char units = _ttoi(Line);
                S3Data->m_DisplayUnits = units;
            }
            break;
		case SCALE:
            {
                unsigned char scale = _ttoi(Line);
                S3Data->m_DisplayScale = scale;
            }
            break;
		case SIZE:
            {
                unsigned char size = _ttoi(Line);
                S3Data->m_SigSize = size;
            }
            break;
		case THREE_PC_LINEAR:
            {
                unsigned char on = _ttoi(Line);
                S3Data->m_3PCLinearity = (on == 1);
            }
            break;
        case MAXIP:
            {
                double maxIP = _ttoi(Line);
                S3Data->m_Config.m_MaxInput = maxIP;
            }
            break;
        case WINDOWTRACKING:
            {
                unsigned char windowtrack = _ttoi(Line);
                S3Data->m_Config.m_WindowTracking = (windowtrack != 0);
            }
            break;
        }
        Line = Response.Tokenize(_T(DATA_ITEM_SEPARATOR), MessageTokenPos);
        ParamIndex++;
    }

}
void DecodeSystemDetails(CString Response)
{
    CString Line;
    int ParamIndex = 0;
    int MessageTokenPos = 0;
    Line = Response.Tokenize(_T(DATA_ITEM_SEPARATOR), MessageTokenPos);
    
    while (!Line.IsEmpty())
    {   
        switch(ParamIndex)
        {
        case SYSNAME:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_NodeName, S3_MAX_NODE_NAME_LEN, ascii.m_psz);
            }
            break;
        case SYSSN:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_SN, S3_MAX_SN_LEN, ascii.m_psz);
            }
            break;
        case SYSPN:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_PN, S3_MAX_PN_LEN, ascii.m_psz);
            }
            break;
        case SYSHW:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_HW, S3_MAX_SW_VER_LEN, ascii.m_psz);
            }
            break;
        case SYSSW:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_SW, S3_MAX_SW_VER_LEN, ascii.m_psz);
            }
            break;
        case IMAGEDATE:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_ImageDate, S3_MAX_OS_IMAGE_ID_LEN, ascii.m_psz);
            }
            break;
        case BUILDNO:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_BuildNum, S3_MAX_BUILD_NUM_LEN, ascii.m_psz);
            }
            break;
        case MODELID:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_ModelId, S3_MAX_MODEL_ID_LEN, ascii.m_psz);
            }
            break;
        case DATE:
            {
                LastUpdateDateStr = Line.Trim();
            }
            break;
        case TIME:
            {
                LastUpdateTimeStr= Line.Trim();
            }
            break;
        case ACCESSMODE:
            {
                unsigned char accessmode = _ttoi(Line.Trim());
                int res = S3SetRemote(accessmode != 0);
                if(accessmode == 1)
                {
                    Sentinel3.isRemoteAndManualAccess = false;
                }
                else
                {
                    Sentinel3.isRemoteAndManualAccess = true;
                }
            }
            break;
        case CONFIGFILENAME:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_ConfigName, S3_MAX_CFG_NAME_LEN, ascii.m_psz);
            }
            break;
        case CONFIGFILEVER:
            {
                double ver = _tstof(Line);
                S3Data->m_FileVersion = ver;
            }
            break;
        case CONFIGFILELOC:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_ConfigPath, S3_MAX_FILENAME_LEN, ascii.m_psz);
            }
            break;
        case LOGFILENAME:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_EventLogName, S3_MAX_FILENAME_LEN, ascii.m_psz);
            }
            break;
        case LOGFILELOC:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_EventLogPath, S3_MAX_FILENAME_LEN, ascii.m_psz);
            }
            break;
        case TESTNAME:
            {
                Line.Trim();
                CT2A ascii(Line);
                strcpy_s(S3Data->m_TestName, S3_MAX_FILENAME_LEN, ascii.m_psz);
            }
            break;
            
        case APPDATETIME:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_AppDateTime, S3_DATETIME_LEN, ascii.m_psz);
            }
            break;
        case S3TYPE:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_Type = (S3SCType)temp;
            }
            break;
        case TCOMPGAINOPT:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_TCompGainOption = (temp != 0);
            }
            break;
        case WINTRACKOPT:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_WinTrackOption = (temp != 0);
            }
            break;
        case LOWNOISEOPT:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_LowNoiseOption = (temp != 0);
            }
            break;
        case SOFTSHUTDOWNOPT:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_SoftShutdownOption = (temp != 0);
            }
            break;
        case SYSAGC:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_AGC = temp;
            }
            break;
        case TXSSTARTSTATE:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_TxStartState = temp;
            }
            break;
        case SELFTEST:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_TxSelfTest = (temp != 0);
            }
            break;
        case SWVERSIOND:
            {
                double temp = _tstof(Line);
                S3Data->m_SWVersionD = temp;
            }
            break;
		case TERMINATOR:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_Terminator = temp;
            }
            break;
        case IMAGEID:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_ImageID, S3_MAX_OS_IMAGE_ID_LEN, ascii.m_psz);
            }
            break;
        case IMAGEOS:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_ImageOS, S3_MAX_OS_IMAGE_ID_LEN, ascii.m_psz);
            }
            break;
        case IMAGETIME:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_ImageTime, S3_MAX_OS_IMAGE_ID_LEN, ascii.m_psz);
            }
            break;
        case OSUPDATEFAIL:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_OSUpdateFail = (temp != 0);
            }
            break;
        case POWERDOWNPENDING:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_PowerDownPending = (temp != 0);
            }
            break;
        case POWERDOWNFAIL:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_PowerDownFailed = (temp != 0);
            }
            break;
        case SLEEPALL:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_SleepAll = (temp != 0);
            }
            break;
		case SYSTEM_LOCKED:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_Locked = (temp != 0);
            }
            break;
        }
        Line = Response.Tokenize(_T(DATA_ITEM_SEPARATOR), MessageTokenPos);
        ParamIndex++;
    }

}
void DecodeConnectionDetails(CString Response)
{    
    CString Line;
    int ParamIndex = 0;
    int MessageTokenPos = 0;
    Line = Response.Tokenize(_T(DATA_ITEM_SEPARATOR), MessageTokenPos);
    
    while (!Line.IsEmpty())
    {   
        switch(ParamIndex)
        {
        case IPV4ADDR:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_IPv4Addr, S3_MAX_IP_ADDR_LEN, ascii.m_psz);
            }
            break;
        case IPV4MASK:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_IPv4Subnet, S3_MAX_IP_ADDR_LEN, ascii.m_psz);
            }
            break;
        case IPV4PORT:
            {
                    unsigned short port = _ttoi(Line);
                    S3Data->m_IPPort = port;
            }
            break;
        case EMACADDR:
            {
                int MACAddrCount = 0, bytecount = 0;
                CString byte = Line.Tokenize(_T(":"), MACAddrCount);
                while(!byte.IsEmpty())
                {
                    S3Data->m_MACAddr[bytecount] = _ttoi(byte);
                    byte = Line.Tokenize(_T(":"), MACAddrCount);
                    bytecount++;
                }
            }
            break;
		case ETH_DHCP:
			{
                unsigned char temp = _ttoi(Line);
				S3Data->m_DHCPEnabled = (temp != 0);
            }
			break;
        case USBPORT:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_DisplayedUSBPort, S3_MAX_USB_PORT_LEN, ascii.m_psz);
            }
            break;
        case USBDRVR:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_DisplayedUSBDriver, S3_MAX_USB_DRIVER_LEN, ascii.m_psz);
            }
            break;
        case MSGSRC:
            {
                unsigned char msgsrc = _ttoi(Line);
                S3Data->m_PrevMsgSrc = msgsrc;
            }
            break;
        case RXDMSG:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_PreviousRecievedMessage, S3_MAX_MESSAGE_LEN, ascii.m_psz);
            }
            break;
        }

        Line = Response.Tokenize(_T(DATA_ITEM_SEPARATOR), MessageTokenPos);
        ParamIndex++;
    }
}
void DecodeChargerDetails(CString Response)
{    
    CString Line, Data;
    int ParamIndex = 0, BattIndex = 0;
    int MessageTokenPos = 0;
    Line = Response.Tokenize(_T(NODE_SEPARATOR), MessageTokenPos);
    
    while (!Line.IsEmpty())
    {   
        int LineTokenPos = 0;
        Data = Line.Tokenize(_T(DATA_ITEM_SEPARATOR), LineTokenPos);
        while(!Data.IsEmpty())
        {
            switch(ParamIndex)
            {
            case BATTVALIDITY:
                {
                    int valid = _ttoi(Data);
                    switch (valid)
                    {
                    case BATTISVALID:
                        S3Data->m_Chargers[BattIndex].m_BattValidated = true;
                        S3ChCancelAlarm(BattIndex, S3_CH_BATT_INVALID);
                        S3Data->m_Chargers[BattIndex].m_Detected = true;
                        S3Data->m_Chargers[BattIndex].m_Occupied = true;
                        break;
                    case BATTISINVALID:
                        S3Data->m_Chargers[BattIndex].m_BattValidated = false;
                        S3ChSetAlarm(BattIndex, S3_CH_BATT_INVALID);
                        S3Data->m_Chargers[BattIndex].m_Detected = true;
                        S3Data->m_Chargers[BattIndex].m_Occupied = true;
                        break;
                    case BATTISNOTPRESENT:
                        S3Data->m_Chargers[BattIndex].m_BattValidated = false;
                        S3ChCancelAlarm(BattIndex, S3_CH_BATT_INVALID);
                        S3Data->m_Chargers[BattIndex].m_Detected = false;
                        S3Data->m_Chargers[BattIndex].m_Occupied = false;
                        break;
                    }
                }
                break;
            case BATTCHARGE:
                {
                    unsigned int SoC = _ttoi(Data);
                    // S3Data->m_Chargers[BattIndex].m_SoC = (unsigned char)SoC;
					S3ChSetSoC(BattIndex, SoC);
                }
                break;
            case TIMETOCHARGE:
                {
                    unsigned short ATTF = _ttoi(Data);
                    S3Data->m_Chargers[BattIndex].m_ATTF = ATTF;
                }
                break;
            case VOLTAGE:
                {
                    double charge = _tstof(Data);
                    S3Data->m_Chargers[BattIndex].m_V = charge;
                }
                break;
            case CURRENT:
                {
                    unsigned short current = _ttoi(Data);
                    S3Data->m_Chargers[BattIndex].m_I = current;
                }
                break;
            case BATTYPE:
                {
                    unsigned int type = _ttoi(Data);
                    //S3Data->m_Chargers[BattIndex].m_BattType = (unsigned char)type;
                    S3Data->m_Chargers[BattIndex].m_Type = (unsigned char)type;
                }
                break;
            case BATTSN:
                {
                    CT2A ascii(Data);
                    strcpy_s(S3Data->m_Chargers[BattIndex].m_BattSN, S3_MAX_SN_LEN, ascii.m_psz);
                }
                break;
            case BATTPN:
                {
                    CT2A ascii(Data);
                    strcpy_s(S3Data->m_Chargers[BattIndex].m_BattPN, S3_MAX_PN_LEN, ascii.m_psz);
                }
                break;
            case BATTHWVER:
                {
                    CT2A ascii(Data);
                    strcpy_s(S3Data->m_Chargers[BattIndex].m_HW, S3_MAX_SW_VER_LEN, ascii.m_psz);
                }
                break;
            case BATTFWVER:
                {
                    CT2A ascii(Data);
                    strcpy_s(S3Data->m_Chargers[BattIndex].m_FW, S3_MAX_SW_VER_LEN, ascii.m_psz);
                }
                break;
            case BATTTEMP:
                {
                    int temp = _ttoi(Data), update = 0;
                    S3Data->m_Chargers[BattIndex].m_BattTemp = (short)temp;
                }
                break;

            case BATTMFRDATA:
                {
                    CT2A ascii(Data);
                    strcpy_s(S3Data->m_Chargers[BattIndex].m_MfrData, S3_MAX_SN_LEN, ascii.m_psz);
                }
                break;
            case BATTTYPE:
                {
                    char temp = _ttoi(Data);
                    S3Data->m_Chargers[BattIndex].m_BattType = temp;
                }
                break;
            case BATTALARMS:
                {
                    char temp = _ttoi(Data);
                    S3Data->m_Chargers[BattIndex].m_Alarms = temp;
                }
                break;
		    case BATTSTATUS:
                {
                    char temp = _ttoi(Data);
                    S3Data->m_Chargers[BattIndex].stat_h = temp;
                }
                break;

            }
            Data = Line.Tokenize(_T(DATA_ITEM_SEPARATOR), LineTokenPos);
            ParamIndex++;
        }
        
        // if((S3Data->m_Chargers[BattIndex].m_SoC == 100) && (S3Data->m_Chargers[BattIndex].m_I == 0))
        // {
        //	S3Data->m_Chargers[BattIndex].m_Charged = true;
        // }

        if(S3Data->m_Chargers[BattIndex].m_BattValidated == false)
        {
            S3Data->m_Chargers[BattIndex].m_SoC = 0;
            S3Data->m_Chargers[BattIndex].m_I = 0;
            S3Data->m_Chargers[BattIndex].m_ATTF = 0;
            S3Data->m_Chargers[BattIndex].m_Charged = false;
        }

        Line = Response.Tokenize(_T(NODE_SEPARATOR), MessageTokenPos);
        ParamIndex = 0;
        BattIndex++;
    }
}

void DecodeRxModuleDetails(CString Response, int Rx)
{
    CString Line;
    int MessageTokenPos = 0, ParamIndex = 0;
    Line = Response.Tokenize(_T(DATA_ITEM_SEPARATOR), MessageTokenPos);
   
    while (!Line.IsEmpty())
    {   
        switch(ParamIndex)
        {
        case RXTYPE:
            {
                unsigned char type = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Type = type;
            }
            break;
        case RXDETECTED:
            {
                unsigned char detect = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Detected = (detect != 0);
            }
            break;
        case RXNAME:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_Rx[Rx].m_NodeName, S3_MAX_NODE_NAME_LEN, ascii.m_psz);
            }
            break;
        case RXID:
            {
                char id = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Id = id;
            }
            break;
        case RXSELTX:
            {
                //This is not needed, as it only shows which Tx the user last pressed on in the GUI
                // (not which is currently being used for the active RF input. This is one of the 
                // parameters that can be, and is OK to be different between the remote GUI and 
                // the actual Sentinel 3 Chassis.
                //unsigned char tx = _ttoi(Line);
                //S3Data->m_Rx[Rx].m_SelectedTx = tx;
            }
            break;
        case RXACTIVETX:
            {
                char tx = _ttoi(Line);
                S3Data->m_Rx[Rx].m_ActiveTx = tx;
            }
            break;
        case RXSN:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_Rx[Rx].m_SN, S3_MAX_SN_LEN, ascii.m_psz);
            }
            break;
        case RXPN:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_Rx[Rx].m_PN, S3_MAX_PN_LEN, ascii.m_psz);
            }
            break;
        case RXFW:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_Rx[Rx].m_FW, S3_MAX_SW_VER_LEN, ascii.m_psz);
            }
            break;
        case RXHW:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_Rx[Rx].m_HW, S3_MAX_SW_VER_LEN, ascii.m_psz);
            }
            break;
        case RXMODELNAME:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_Rx[Rx].m_ModelName, S3_MAX_MODEL_ID_LEN, ascii.m_psz);
            }
            break;
        case RXRLL:
            {
                int Tx = 0;
                //Current Data packet is Tx0
                short tx = _ttoi(Line);
                S3Data->m_Rx[Rx].m_RLL[Tx] = tx;

                for(Tx = 1; Tx <6; Tx++)
                {
                    //Progress forward and process data packet for Tx1-5
                    Line = Response.Tokenize(_T(DATA_ITEM_SEPARATOR), MessageTokenPos);
                    short tx = _ttoi(Line);
                    S3Data->m_Rx[Rx].m_RLL[Tx] = tx;
                    
                }
            }
            break;
        case RXRFGAIN:
            {
                int Tx = 0;
                short tx = _ttoi(Line);
                S3Data->m_Rx[Rx].m_RFGain[Tx] = tx;
                for(Tx = 1; Tx <6; Tx++)
                {
                    Line = Response.Tokenize(_T(DATA_ITEM_SEPARATOR), MessageTokenPos);
                    short tx = _ttoi(Line);
                    S3Data->m_Rx[Rx].m_RFGain[Tx] = tx;
                }
            }
            break;
        case RXLINKGAIN:
            {
                int Tx = 0;
                char tx = _ttoi(Line);
                S3Data->m_Rx[Rx].m_LinkGain[Tx] = tx;
                for(Tx = 1; Tx <6; Tx++)
                {
                    Line = Response.Tokenize(_T(DATA_ITEM_SEPARATOR), MessageTokenPos);
                    char tx = _ttoi(Line);
                    S3Data->m_Rx[Rx].m_LinkGain[Tx] = tx;
                    
                }
            }
            break;
        case RXCALGAIN:
            {
                char n = 0;
                short tx = _ttoi(Line);
                S3Data->m_Rx[Rx].m_CalGain[n] = tx;
                for(n = 1; n <2; n++)
                {
                    Line = Response.Tokenize(_T(DATA_ITEM_SEPARATOR), MessageTokenPos);
                    tx = _ttoi(Line);
                    S3Data->m_Rx[Rx].m_CalGain[n] = tx; 
                }
            }
            break;
        case RXVCC:
            {
                unsigned short tx = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Vcc = tx;
            }
            break;
        case RXAGC:
            {
                char n = 0;
                unsigned char tx = _ttoi(Line);
                S3Data->m_Rx[Rx].m_AGC[n] = tx;
                for(n = 1; n <2; n++)
                {
                    Line = Response.Tokenize(_T(DATA_ITEM_SEPARATOR), MessageTokenPos);
                    tx = _ttoi(Line);
                    S3Data->m_Rx[Rx].m_AGC[n] = tx; 
                }
            }
            break;
        case RXALARMS:
            {
                unsigned char tx = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Alarms = tx;
            }
            break;
        case RXTEMP:
            {
                char tx = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Temp = tx;
            }
            break;
        case RXTEMPHI:
            {
                char tx = _ttoi(Line);
                S3Data->m_Rx[Rx].m_TempHi = tx;
            }
            break;
        case RXTEMPLO:
            {
                char tx = _ttoi(Line);
                S3Data->m_Rx[Rx].m_TempLo = tx;
            }
            break;
        case RXGAIN:
            {
                int temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Config.m_Gain = temp;
            }
            break;
        case RXMAXIPINHERIT:
            {
                double temp = _tstof(Line);
                S3Data->m_Rx[Rx].m_Config.m_MaxInput = temp;
            }
            break;
        case RXTAU:
            {
                char temp = _ttoi(Line); 
                S3Data->m_Rx[Rx].m_Config.m_Tau = (SigmaT)temp;
            }
            break;
        case RXIMPEDANCE:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Config.m_InputZ = (InputZ)temp;
            }
            break;
        case RXLOWNOISE:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Config.m_LowNoiseMode = (temp != 0);
            }
            break;
        case RXWINDOWTRACK:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Config.m_WindowTracking = (temp != 0);
            }
            break;
		case RXCTRLALARMS:
            {
                char n = 0;
                unsigned char tx = _ttoi(Line);
                S3Data->m_Rx[Rx].m_RxAlarms[n] = tx;
                for(n = 1; n <S3_RX_CTRL_ALARM_BYTES; n++)
                {
                    Line = Response.Tokenize(_T(DATA_ITEM_SEPARATOR), MessageTokenPos);
                    tx = _ttoi(Line);
                    S3Data->m_Rx[Rx].m_RxAlarms[n] = tx; 
                }
            }
            break;
        case RXTXALARMS:
            {
                char n = 0;
                unsigned char tx = _ttoi(Line);
                S3Data->m_Rx[Rx].m_TxAlarms[n] = tx;
                for(n = 1; n <S3_MAX_TXS; n++)
                {
                    Line = Response.Tokenize(_T(DATA_ITEM_SEPARATOR), MessageTokenPos);
                    tx = _ttoi(Line);
                    S3Data->m_Rx[Rx].m_TxAlarms[n] = tx; 
                }
            }
            break;
        case RXCURALMSRC:
            {
                char temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_CurAlarmSrc = temp;
            }
            break;
        case RXCURALARM:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_CurAlarm = temp;
            }
            break;
        case RXRLLHI:
            {
                short temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_RLLHi = temp;
            }
            break;
        case RXRLLLO:
            {
                short temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_RLLLo = temp;
            }
            break;
        case RXFMAX:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Fmax = (S3Fmax)temp;
            }
            break;
        case RXFWDATE:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_Rx[Rx].m_FWDate, S3_MAX_FW_DATE_LEN, ascii.m_psz);
            }
            break;
        case RXRFLEVEL:
            {
                int Tx = 0;
                short tx = _ttoi(Line);
                S3Data->m_Rx[Rx].m_RFLevel[Tx] = tx;
                for(Tx = 1; Tx < S3_MAX_TXS; Tx++)
                {
                    Line = Response.Tokenize(_T(DATA_ITEM_SEPARATOR), MessageTokenPos);
                    tx = _ttoi(Line);
                    S3Data->m_Rx[Rx].m_RFLevel[Tx] = tx;
                }
            }
            break;
		case RXEXTRAGAINCAP:
			{
	            short gain = _ttoi(Line);
                S3Data->m_Rx[Rx].m_ExtraGainCap = (char)gain;
			}
		   break;
        }

        Line = Response.Tokenize(_T(DATA_ITEM_SEPARATOR), MessageTokenPos);
        ParamIndex++;
    }

}
void DecodeTxModuleDetails(CString Response, int Rx, int Tx)
{
    CString Line;
    int MessageTokenPos = 0, ParamIndex = 0;
    Line = Response.Tokenize(_T(DATA_ITEM_SEPARATOR), MessageTokenPos);

    
    while (!Line.IsEmpty())
    {   
        switch(ParamIndex)
        {
         case TXTYPE:
            {
                unsigned char type = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_Type = (S3TxType)type;
            }
            break;
        case TXDETECTED:
            {
                unsigned char detect = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_Detected = (detect != 0);
            }
            break;
        case TXNAME:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_Rx[Rx].m_Tx[Tx].m_NodeName, S3_MAX_NODE_NAME_LEN, ascii.m_psz);
            }
            break;
        case TXID:
            {
                char id = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_Id = id;
            }
            break;
        case TXPARENTID:
            {
                char id = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_ParentId = id;
            }
            break;
        case TXWAVELENGTH:
            {
                unsigned char id = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_Wavelength = id;
            }
            break;
        case TXSN:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_Rx[Rx].m_Tx[Tx].m_SN, S3_MAX_SN_LEN, ascii.m_psz);
            }
            break;
        case TXPN:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_Rx[Rx].m_Tx[Tx].m_PN, S3_MAX_PN_LEN, ascii.m_psz);
            }
            break;
        case TXFW:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_Rx[Rx].m_Tx[Tx].m_FW, S3_MAX_SW_VER_LEN, ascii.m_psz);
            }
            break;
        case TXHW:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_Rx[Rx].m_Tx[Tx].m_HW, S3_MAX_SW_VER_LEN, ascii.m_psz);
            }
            break;
        case TXMODELNAME:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_Rx[Rx].m_Tx[Tx].m_ModelName, S3_MAX_MODEL_ID_LEN, ascii.m_psz);
            }
            break;
        case TXBATTSN:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_Rx[Rx].m_Tx[Tx].m_BattSN, S3_MAX_SN_LEN, ascii.m_psz);
            }
            break;
        case TXBATTPN:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_Rx[Rx].m_Tx[Tx].m_BattPN, S3_MAX_PN_LEN, ascii.m_psz);
            }
            break;
        case TXBATTFW:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_Rx[Rx].m_Tx[Tx].m_BattFW, S3_MAX_SW_VER_LEN, ascii.m_psz);
            }
            break;
        case TXBATTHW:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_Rx[Rx].m_Tx[Tx].m_BattHW, S3_MAX_SW_VER_LEN, ascii.m_psz);
            }
            break;
        case TXBATTTEMP:
            {
                short temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_BattTemp = temp;
            }
            break;
        case TXBATTVALIDATED:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_BattValidated = (temp != 0);
            }
            break;
        case TXBATTSOC:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_SoC = temp;
            }
            break;
        case TXBATTATTE:
            {
                unsigned short ATTE = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_ATTE = ATTE;
            }
            break;
        case TXPOWERSTATE:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_PowerStat = temp;
            }
            break;
        case TXACTIVEINPUT:
            {
                char temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_ActiveInput = temp;
            }
            break;
        case TXTEMP:
            {
                char temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_TempTx = temp;
            }
            break;
        case TXTEMPCOMP:
            {
                char temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_TempComp = temp;
            }
            break;
        case TXLASERPOW:
            {
                short temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_LaserPow = temp;
            }
            break;
        case TXLASERLO:
            {
                short temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_LaserLo = temp;
            }
            break;
        case TXLASERHI:
            {
                short temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_LaserHi = temp;
            }
            break;
        case TXCOMPMODE:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_CompMode = temp;
            }
            break;
        case TXALARMS:
            {
                unsigned short temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_Alarms = temp;
            }
            break;
        case TXOPTALARMS:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_OptAlarms[0] = temp;   
                Line = Response.Tokenize(_T(DATA_ITEM_SEPARATOR), MessageTokenPos);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_OptAlarms[1] = temp;   
                Line = Response.Tokenize(_T(DATA_ITEM_SEPARATOR), MessageTokenPos);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_OptAlarms[2] = temp;   
            }
            break;
        case TXCTRLALARMS:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_CtrlAlarms[0] = temp;   
                Line = Response.Tokenize(_T(DATA_ITEM_SEPARATOR), MessageTokenPos);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_CtrlAlarms[1] = temp;            
            }
            break;
        case TXBATTALARMS:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_BattAlarms = temp;
            }
            break;
		case TXRLLSTABLE:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_RLLStableCnt = temp;
            }
            break;
        case TXGAIN:
            {
                int temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_Config.m_Gain = temp;
            }
            break;
        case TXMAXIPINHERIT:
            {
                double temp = _tstof(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_Config.m_MaxInput = temp;
            }
            break;
        case TXTAU:
            {
                char temp = _ttoi(Line); 
                S3Data->m_Rx[Rx].m_Tx[Tx].m_Config.m_Tau = (SigmaT)temp;
            }
            break;
        case TXIMPEDANCE:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_Config.m_InputZ = (InputZ)temp;
            }
            break;
        case TXLOWNOISE:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_Config.m_LowNoiseMode = (temp != 0);
            }
            break;
        case TXWINDOWTRACK:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_Config.m_WindowTracking = (temp != 0);
            }
            break;
            
        case TXFMAX:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_Fmax = (S3Fmax)temp;
            }
            break;
        case TXUPTIME:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_Uptime = temp;
            }
            break;
        case TXCALOPT:
            {
                short temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_CalOpt = temp;
            }
            break;
        case TXFWDATE:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_Rx[Rx].m_Tx[Tx].m_FWDate, S3_MAX_FW_DATE_LEN, ascii.m_psz);
            }
            break;
        case TXBATTCURRENT:
            {
                short temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_I = temp;
            }
            break;
        case TXUSERSLEEP:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_UserSleep = (temp != 0);
            }
            break;
        case TXEMERGENCYSLEEP:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_EmergencySleep = (temp != 0);
            }
            break;
        case TXCURALMSRC:
            {
                char temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_CurAlarmSrc = temp;
            }
            break;
        case TXCURALARM:
            {
                unsigned short temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_CurAlarm = temp;
            }
            break;
        case TXTEMPTEC:
            {
                short temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_TempTEC = temp;
            }
            break;
        case TXPEAKHOLD:
            {
                short temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_PeakHold = temp;
            }
            break;
        case TXTAUNS:
            {
                int n = 0;
                double temp = _tstof(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_Tau_ns[n] = temp;
                for(n = 1; n < 4; n++)
                {
                    Line = Response.Tokenize(_T(DATA_ITEM_SEPARATOR), MessageTokenPos);
                    double temp = _tstof(Line);
                    S3Data->m_Rx[Rx].m_Tx[Tx].m_Tau_ns[n] = temp;
                }
            }
            break;
          
        }
        Line = Response.Tokenize(_T(DATA_ITEM_SEPARATOR), MessageTokenPos);
        ParamIndex++;
    }
}
void DecodeIPModuleDetails(CString Response, int Rx, int Tx, int Ip)
{
    CString Line;
    int MessageTokenPos = 0, ParamIndex = 0;
    Line = Response.Tokenize(_T(DATA_ITEM_SEPARATOR), MessageTokenPos);
    while (!Line.IsEmpty())
    {   
        switch(ParamIndex)
        {
        case IPNODENAME:
            {
                CT2A ascii(Line);
                strcpy_s(S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[Ip].m_NodeName, S3_MAX_NODE_NAME_LEN, ascii.m_psz);
            }
            break;
        case IP1DBCOMP:
            {
                double temp = _tstof(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[Ip].m_P1dB = temp;
            }
            break;
        case IPMAXIP:
            {
                double temp = _tstof(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[Ip].m_MaxInput = temp;
            }
            break;
        case IPALARMS:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[Ip].m_Alarms = temp;
            }
            break;
        case IPGAIN:
            {
                int temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[Ip].m_Config.m_Gain = temp;
            }
            break;
        case IPMAXIPINHERIT:
            {
                double temp = _tstof(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[Ip].m_Config.m_MaxInput = temp;
            }
            break;
        case IPTAU:
            {
                char temp = _ttoi(Line); 
                S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[Ip].m_Config.m_Tau = (SigmaT)temp;
            }
            break;
        case IPIMPEDANCE:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[Ip].m_Config.m_InputZ = (InputZ)temp;
            }
            break;
        case IPLOWNOISE:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[Ip].m_Config.m_LowNoiseMode = (temp != 0);
            }
            break;
        case IPWINDOWTRACK:
            {
                unsigned char temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[Ip].m_Config.m_WindowTracking = (temp != 0);
            }
            break;
        case IPRFLEVEL:
            {
                short temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[Ip].m_RFLevel = temp;
            }
            break;
        case IPRFGAIN:
            {
                short temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[Ip].m_RFGain = temp;
            }
            break;
        case IPTESTTONE:
            {
                char temp = _ttoi(Line);
                S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[Ip].m_TestToneEnable = temp;
            }
            break;
            
        }
        Line = Response.Tokenize(_T(DATA_ITEM_SEPARATOR), MessageTokenPos);
        ParamIndex++;
    }

}
