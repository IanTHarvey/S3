#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "stdafx.h"
#include "S3ConnectThread.h"
#include "../S3SystemDetails.h"


UINT AttemptConnectToSentinel3(LPVOID pParam)
{
    HWND *phObjectHandle = static_cast<HWND *>(pParam);
    CS3AgentDlg *pObject = (CS3AgentDlg *)pParam;

    int connectionresult = COMMPASS;
    int modelidx;
	bool	Incompatible = true;
    CString Response;
    CString startupQuery = L"REPORT SYS";
    CString responsecode = L"Sentinel 3";
    
    //Disallow the user to make changes to the connection settings
    pObject->m_EthSelRdoButton.EnableWindow(false);
    pObject->m_USBSelRdoButton.EnableWindow(false);
    pObject->m_IPv4AddrEdit.EnableWindow(false);
    pObject->m_PortEdit.EnableWindow(false);
    pObject->m_SerialDropDown.EnableWindow(false);
    pObject->m_GPIBSelRdoButton.EnableWindow(false);
    pObject->m_SerialSelRdoButton.EnableWindow(false);
    pObject->m_ConnectS3Button.EnableWindow(false);
    pObject->m_ConnectS3Button.SetWindowTextW(L"Attempting to connect to Sentinel 3...");

    //Start by assuming that a Sentinel 3 is present on the other end of the connection
    Sentinel3.isConnected = true;

    switch (connectionmethod)
    {
        case ETHERNET:
            connectionresult = OpenSocketSC3();
            if(connectionresult != 0)
                Response = L"Unable to open Socket";
            break;
        case USB:
            connectionresult = OpenConnectUSB();
            break;
        case GPIB:
            connectionresult = OpenGPIBConnection();
            break;
        default:
            //Connection method not set. Show error to user
            AfxMessageBox(
				_T("Connection Failed: \r\nNo connection method ")
				_T("chosen. Please ensure the connection details are ")
				_T("correct before retrying."));
            Sentinel3.isConnected = false;
            if(pObject->ManualCommandEntryVisible)
            {
                pObject->m_ConnectS3Button.SetWindowTextW(L"Connect to Sentinel 3 Device");
            }
            else
            {
                pObject->m_ConnectS3Button.SetWindowTextW(L"Connect to\r\nSentinel 3\r\nDevice");
            }
            pObject->m_EthSelRdoButton.EnableWindow(true);
            pObject->m_USBSelRdoButton.EnableWindow(true);
            pObject->m_ConnectS3Button.EnableWindow(true);
            pObject->m_IPv4AddrEdit.EnableWindow(true);
            pObject->m_PortEdit.EnableWindow(true);
            pObject->m_SerialDropDown.EnableWindow(true);
            pObject->m_GPIBSelRdoButton.EnableWindow(true);
            pObject->m_SerialSelRdoButton.EnableWindow(true);
            return 10;
    }

	CString versionsw;

    if (connectionresult == COMMPASS)
    {
        //Request ID from the Sentinel 3
        Response = SendSentinel3Message(startupQuery);
        //Include support for legacy software builds which have REPORT SYS mapped to REPORT CTRL
        if ((Response.Mid(0, 2).CompareNoCase(L"E:") == 0))
        {
            startupQuery = L"REPORT CTRL";
            Response = SendSentinel3Message(startupQuery);
        }

        modelidx = Response.Find(L"Model:\t");
        if ((modelidx == -1) && (Response.Mid(0, 6).CompareNoCase(L"ERROR:") == 0))//Function returns an error value
        {
            connectionresult = CONNFAIL;
            //Connection Function returns an error value - we have  not been successful in opening communication.
        }
        else if (Response.Mid(modelidx + 7, 10).CompareNoCase(responsecode) == 0)
        {
            connectionresult = COMMPASS; //Found a Sentinel 3 - we're all happy
        }
        else if (Response.GetLength() != 0)
        {
            connectionresult = COMMPASS; //Device found on the other end, but it is not a Sentinel 3
        }
        else
        {
            connectionresult = CONNPASS;
        }

		int versionidx = Response.Find(L"SW v:\t");

		if (versionidx != -1)
		{
			versionsw = Response.Mid(versionidx + 6, 8);

			if (versionsw == CString(S3_SYS_SW).Left(8))
				Incompatible = false;
		}
    }

    if (connectionresult == COMMPASS)
    {
        if(pObject->AutoSyncTimesOnConnect)
        {
            CTime CurrentTime = CTime::GetCurrentTime();
            CString Args = CurrentTime.Format(_T(" %Y %m %d %H %M %S"));
            CString Command = L"SYSSETTIME";
            Command.Append(Args);
            Response = SendSentinel3Message(Command);
        }

        if(pObject->m_AutoUpdateGUITick.GetCheck())
        {
            pObject->m_DataUpdateThread = AfxBeginThread(AutoUpdateSentinelDataBundleThread, pObject); //Get up-to date information on the Sentinel 3 configuration
        }
        //Socket/Port opened succesfully, and a Sentinel 3 Chassis found on the other end.
        //Re-initialise the Sentinel3 Data Model (remove any previous data)
        pObject->m_ConnectS3Button.SetWindowTextW(L"Disconnect from Sentinel 3 Device");
        pObject->m_ConnectS3Button.EnableWindow(true);
        Sentinel3.isConnected = true;
        pObject->m_RemoteLocalCheck.EnableWindow(true);

		if (Incompatible)
		{
			// Warning only
			CString ErrorString;
			
			ErrorString = "Sentinel 3 Remote Agent and Controller have "
					"different versions (";
			ErrorString += CString(S3_SYS_SW);
			ErrorString += " vs ";
			ErrorString += versionsw;
			ErrorString += "). It is recommended to install the latest Remote Agent ";
			ErrorString += "and Sentinel 3 updates as incompatibilities may cause undefined behaviour.";
            AfxMessageBox(ErrorString);
		}
    }
    else
    {
        //We encountered an error which prevented us from connecting to a valid Sentinel 3.
        pObject->m_ConnectS3Button.EnableWindow(true);
        if(pObject->ManualCommandEntryVisible)
        {
            pObject->m_ConnectS3Button.SetWindowTextW(L"Connect to Sentinel 3 Device");
        }
        else
        {
            pObject->m_ConnectS3Button.SetWindowTextW(L"Connect to\r\nSentinel 3\r\nDevice");
        }
        pObject->m_RemoteLocalCheck.EnableWindow(false);

        CString ErrorString = L"Connection Failed. \r\nIs the Sentinel 3 connected to the Serial Port/GPIB adaptor/Ethernet?\r\n";
        ErrorString.Append(Response);

        // Handle the result of the previous switch block
        switch (connectionresult)
        {
            case CONNPASS:
                AfxGetMainWnd()->PostMessage(S3_DISCONNECT, 0, 0);
                AfxMessageBox(_T("Communcations port was opened successfully.\r\nNo Sentinel 3 device found.\r\nConfirm your connection settings are correct."));
                break;
            case NOTS3:
                AfxGetMainWnd()->PostMessage(S3_DISCONNECT, 0, 0);
                AfxMessageBox(_T("A connection to the remote device was successful.\r\nThe device did not identify as a Sentinel 3.\r\nConfirm your connection settings before continuing."));
                break;
            case USBINITFAIL:
                pObject->m_EthSelRdoButton.EnableWindow(true);
                pObject->m_USBSelRdoButton.EnableWindow(true);
                pObject->m_SerialDropDown.EnableWindow(true);
                AfxMessageBox(_T("Connection Failed: \r\nCOM Port initialisation failed."));
                Sentinel3.isConnected = false;
                break;
            case CONNFAIL:
                AfxGetMainWnd()->PostMessage(S3_DISCONNECT, 0, 0);
                //Detailed error message returned up from message sending functions. Show this to the user.
                AfxMessageBox(ErrorString);
                break;
            default:
                pObject->m_EthSelRdoButton.EnableWindow(true);
                pObject->m_USBSelRdoButton.EnableWindow(true);
                pObject->m_SerialDropDown.EnableWindow(true);
                pObject->m_IPv4AddrEdit.EnableWindow(true);
                pObject->m_PortEdit.EnableWindow(true);
                AfxMessageBox(_T("Connection Failed. \r\nPlease Retry."));
                Sentinel3.isConnected = false;
                break;
        }
    }
    return 0;
}