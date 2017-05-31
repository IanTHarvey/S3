#include "stdafx.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "afxwin.h"
#include "S3Comms.h"
#include "defines.h"
extern void ReadFromSerialPort();
// ----------------------------------------------------------------------------
// Open the USB connection
// - Listens on selected USB/COM port
int OpenConnectUSB()
{
    m_USBEnabled = true;

    // if we already have a COM connection open, close it
    if (m_COMPort->IsPortOpen())
        m_COMPort->Close();

    if (!m_COMPort->OpenPort(COM_name))
    {
        //USB initialisation failure
        return USBINITFAIL;
    }
    else
    {
        // Save 'valid' COM port
        char Filename[MAX_SCRIPT_LEN];

        sprintf_s(Filename, MAX_SCRIPT_LEN, "%ls\\S3COMName.txt", DataLocStr);

        FILE *fid;
        int err = 0;
        int line_no = 0;

        //Alert the user if we cannot save the valid COM port to a file.
        err = fopen_s(&fid, Filename, "w");
        if (err)
        {
            AfxMessageBox(_T("Failed to save COM port name"));
        }

        CStringA tmpA(COM_name);
        char	COMName[MAX_IP_ADDR_LEN];
        strcpy_s(COMName, MAX_IP_ADDR_LEN, (LPCSTR)tmpA);

        fprintf(fid, "%s", COMName);

        fclose(fid);

        //Connection passed, therefore report a pass.
        return 0;
    }
}


// ----------------------------------------------------------------------------
// Close the USB connection
// Closes open COM port
void CloseConnectUSB()
{
    if (m_COMPort)
        m_COMPort->Close();
}


// ----------------------------------------------------------------------------
// Open the network socket
int OpenSocketSC3()
{
    WSADATA wsaData;

    struct addrinfo *result = NULL,
        *ptr = NULL,
        hints;

    int ClientErr = 0;
    ConnectSocket = INVALID_SOCKET;

    int iResult;

    CString	tmp;
    char	IPAddr[MAX_IP_ADDR_LEN], Port[MAX_PORT_LEN];

    CStringA tmpA(IPv4Addr);
    strcpy_s(IPAddr, MAX_IP_ADDR_LEN, (LPCSTR)tmpA);

    tmpA = IPV4Port;
    strcpy_s(Port, MAX_PORT_LEN, (LPCSTR)tmpA);

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return CONNFAIL;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(IPAddr, Port, &hints, &result);
    if (iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return CONNFAIL;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {
        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET)
        {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return CONNFAIL;
        }

#ifdef NONBLOCKING
        // This was an attempt to make the connection non-blocking so that
        // non-existent clients could time-out quicker.
        //
        // Only the first message is successful after S3 is started,
        // subsequent messages don't do anything.
        //
        // The rest of the code probably needs to be re-implemented on a
        // non-blocking basis.
        // 
        unsigned long iMode = 1;
        iResult = ioctlsocket(ConnectSocket, FIONBIO, &iMode);

        if (iResult != NO_ERROR)
        {
            printf("ioctlsocket: failed with error: %ld\n", iResult);
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR)
        {
            int iError = WSAGetLastError();

            //check if error was WSAEWOULDBLOCK, where we'll wait
            if (iError == WSAEWOULDBLOCK)
            {
                printf("Attempting to connect.\n");
                fd_set Write, Err;

                TIMEVAL Timeout;
                int TimeoutSec = 5; // timeout after 10 seconds

                FD_ZERO(&Write);
                FD_ZERO(&Err);
                FD_SET(ConnectSocket, &Write);
                FD_SET(ConnectSocket, &Err);

                Timeout.tv_sec = TimeoutSec;
                Timeout.tv_usec = 0;

                iResult = select(0,        // Ignored
                    NULL,      // Read
                    &Write,    // Write Check
                    &Err,      // Error Check
                    &Timeout);

                if (iResult == 0)
                {
                    closesocket(ConnectSocket);
                    ConnectSocket = INVALID_SOCKET;
                    continue;
                }
                else
                {
                    if (FD_ISSET(ConnectSocket, &Write))
                    {
                        break;
                    }

                    if (FD_ISSET(ConnectSocket, &Err))
                    {
                        // Select error
                        iError = WSAGetLastError();
                        closesocket(ConnectSocket);
                        ConnectSocket = INVALID_SOCKET;
                        continue;
                    }
                }
            }
            else
            {

                // cout << "Failed to connect to server.\n";
                // cout << "Error: " << WSAGetLastError() << endl;
                // WSACleanup();
                // system("pause");
                // return 1;
                closesocket(ConnectSocket);
                ConnectSocket = INVALID_SOCKET;
                continue;

            }
        }
#else   
        DWORD timeout = 1 * 1000; //1ms timeout
        setsockopt(ConnectSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR)
        {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }


        break;
#endif
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET)
    {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return CONNPASS;
    }

    return 0;
}


// ----------------------------------------------------------------------------
// Close the network socket
int CloseSocketSC3()
{
    int ClientErr = 0;

    // shutdown the connection since no more data will be sent
    int iResult = shutdown(ConnectSocket, SD_SEND);

    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // Receive until the peer closes the connection
    do {
        iResult = recv(ConnectSocket, RxBuf, DEFAULT_BUFLEN, 0);
        if (iResult > 0)
        {
            RxBuf[iResult] = '\0';

            printf("Bytes received: %d\n", iResult);

            // Look for Ok at end of buffer
            if (STRCMP(RxBuf + strlen(RxBuf) - 2, "Ok"))
                ClientErr = 666;
        }
        else if (iResult == 0)
            printf("Connection closed\n");
        else
            printf("recv failed with error: %d\n", WSAGetLastError());

    } while (iResult > 0);

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return ClientErr;
}

#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>
extern boost::mutex CommsMUTEX;

// ----------------------------------------------------------------------------
// Send Message wrapper to select the correct interface to send messages to the Sentinel 3
// Employs a MUTEX to limit it to only one thread having access to the Sentinel3 comms at once
CString SendSentinel3Message(CString message)
{
    CommsMUTEX.lock();

    if (Sentinel3.isConnected == false)
    {
        return (CString)"ERROR: NO S3 Connected\r\n";
    }

    CString RetString;
    CString tmp;
    switch (connectionmethod)
    {
        case ETHERNET:
            char	TxBuf[DEFAULT_BUFLEN];
            char	IPAddr[MAX_IP_ADDR_LEN]; // , Port[MAX_PORT_LEN];

            strcpy_s(IPAddr, MAX_IP_ADDR_LEN, (LPCSTR)((CStringA)IPv4Addr));

            if (IPv4ToolsRemLeadZ(IPAddr))
            {
                RetString.Format(_T("ERROR: IPv4 Address is no longer valid\r\n"));
            }
            else
            {
                int i;
                //Write command to Tx Buffer
                for (i = 0; i < message.GetLength(); i++)
                    TxBuf[i] = (char)message[i];

                // TxBuf[i++] = S3_CMD_TERMINATOR;
                TxBuf[i++] = '\0';

                int err = SendMessageSC3_2(TxBuf);

                if (err == 666)
                {
                    //AfxMessageBox(_T("Message rejected by host"));
                    RetString.Format(_T("ERROR: Ethernet: Message rejected by host.\r\n"));
                }
                else if (err)
                {
                    //AfxMessageBox(_T("Message send failed"));
                    RetString.Format(_T("ERROR: Message Send Failed. Is the Sentinel 3 still connected?\r\n"));
                }
                else
                {
                    CString tmp(RxBuf);
                    RetString = tmp;
                }
            }
            break;
        case USB:
            if (m_COMPort)
            {
                int		i;
                char	TxBuf[DEFAULT_BUFLEN];
                //Write command to Tx Buffer
                for (i = 0; i < message.GetLength(); i++)
                    TxBuf[i] = (char)message[i];
                //Delimit command
                TxBuf[i] = '\0';
                //Write to COM Port
                m_COMPort->Write(TxBuf);
                // Wait for ack from read thread
                unsigned timeout = 0;
                while (m_COMPort->m_WaitingAck && timeout < 5000) 
                {
                    Sleep(1);//ms
                    timeout++;
                }

                if (timeout < 5000)
                {
                    RetString = m_COMPort->m_RecvBuf;
                }
                else
                {
                    RetString.Format(_T("ERROR: COM: Timed out\r\n"));
                }



                //Read response from COM port
                //RetString = m_COMPort->Read();
            }
            break;
        case GPIB:
            {
                int		i, BytesRead = 0;
                char	TxBuf[DEFAULT_BUFLEN], GPIBRxbuf[DEFAULT_BUFLEN] = {0}, GPIBRxbufTWO[DEFAULT_BUFLEN] = {0}, endchar;
                //Write command to Tx Buffer
                for (i = 0; i < message.GetLength(); i++)
                    TxBuf[i] = (char)message[i];
                //Delimit command
                TxBuf[i] = '\0';
                Send(0, GPIBAddress, TxBuf, i, NLend);
                //Need to keep recieving until there is no data left.

                Receive(0, GPIBAddress, GPIBRxbuf, DEFAULT_BUFLEN, '\r');
                BytesRead = ibcntl;
                endchar = GPIBRxbuf[BytesRead - 2];
                if(endchar == 0x03)
                {
                    GPIBRxbuf[BytesRead - 2] = '\n';
                    GPIBRxbuf[BytesRead - 1] = 0;
                }
                if(ibsta & ERR)
                {
                     RetString.Format(_T("ERROR: GPIB: Error reading from device\r\n"));
                }
                else
                {
                    while((endchar != 0x03) && (BytesRead > 0))
                    {
                        Receive(0, GPIBAddress, GPIBRxbufTWO, DEFAULT_BUFLEN, '\r');
                        BytesRead = ibcntl;
                        endchar = GPIBRxbufTWO[BytesRead - 2];
                        if(endchar == 0x03)
                        {
                            GPIBRxbufTWO[BytesRead - 2] = '\n';
                            GPIBRxbufTWO[BytesRead - 1] = 0;
                        }
                        else
                        {
                            GPIBRxbufTWO[BytesRead - 1] = '\n';
                            GPIBRxbufTWO[BytesRead] = 0;
                        }
                        strcat_s(GPIBRxbuf, DEFAULT_BUFLEN, GPIBRxbufTWO);
                    }
                    CString tmp(GPIBRxbuf);
                    RetString = tmp;
                }
            }
            break;
    }
    CommsMUTEX.unlock();
    return RetString;
}


// ----------------------------------------------------------------------------
// Open a socket, then send message over ethernet
int SendMessageSC3_2(const char *TxBuf)
{
    int err = 0;

    err = SendMessageOpenSocketSC3(TxBuf);

    return err; // Thread completed successfully
}


// ----------------------------------------------------------------------------
// Once we have an open socket to a Sentinel 3, send a message, and recieve its response 
int SendMessageOpenSocketSC3(const char *TxBuf)
{
    size_t len = strlen(TxBuf);
    int iResult; //, ClientErr;

    *RxBuf = '\0';
    // Send an initial buffer - NOT terminator
    iResult = send(ConnectSocket, TxBuf, (int)len, 0);

    CString tmp;

    if (iResult == SOCKET_ERROR)
    {
        tmp.Format(_T("Response: %s"), _T("Send error"));
        printf("Send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();

        return 1;
    }

    iResult = recv(ConnectSocket, RxBuf, DEFAULT_BUFLEN, 0);

    if (iResult > 0)
    {
        RxBuf[iResult] = '\0';

        printf("Bytes received: %d\n", iResult);
        
        // Look for Ok\n at end of buffer
        if (STRNCMP(&RxBuf[iResult-3], "Ok", 2))
        {
            return 0;
        }
        else
        {
            // Save 'valid IP' adddress
            char Filename[MAX_SCRIPT_LEN];

            sprintf_s(Filename, MAX_SCRIPT_LEN, "%ls\\S3IP.txt", DataLocStr);

            FILE *fid;
            int err = 0;
            int line_no = 0;

            err = fopen_s(&fid, Filename, "w");
            if (err)
            {
                AfxMessageBox(_T("Failed to save IP address"));
            }

            CStringA tmpA(IPv4Addr);
            char	IPAddr[MAX_IP_ADDR_LEN];
            strcpy_s(IPAddr, MAX_IP_ADDR_LEN, (LPCSTR)tmpA);

            fprintf(fid, "%s", IPAddr);

            fclose(fid);

        }
        // ClientErr = 666;

        if (0) // !strncmp(TxBuf, "dbg", 3))
            Sleep(10);
    }

    printf("Bytes Sent: %ld\n", iResult);

    return 0;
}


// ----------------------------------------------------------------------------
// (Limited) Validation of the IPv4 address
int IPv4ToolsRemLeadZ(char *IPAddr)
{
    if (!strcmp(IPAddr, "localhost"))
        return 0;

    char	tmp[4][MAX_IP_ADDR_LEN];
    char	*pbyte[5], *start; // pbyte[5] to allow detection of extra field

    size_t		len = strlen(IPAddr);

    if (len > 15 || len > MAX_IP_ADDR_LEN || len < 7) // 1.2.3.4
        return 1;

    pbyte[0] = IPAddr;

    if (IPAddr[0] == '.')
        return(1);

    unsigned char count = 1;
    unsigned char	i;

    for (i = 1; i < len; i++)
    {
        if (IPAddr[i] == '.')
        {
            pbyte[count++] = IPAddr + i + 1;

            if (count == 5)
                return(1);
        }
    }

    for (i = 0; i < 4; i++)
    {
        int j = 0;
        start = pbyte[i];

        // Skip leading zeros
        while (start[j] == '0' && start[j + 1] != '.' && start[j + 1] != '\0')
        {
            j++;
        }

        // Copy the rest
        count = 0;
        while (start[j] != '.' && start[j] != '\0')
            tmp[i][count++] = start[j++];

        tmp[i][count] = '\0';
    }

    // Reform
    sprintf_s(IPAddr, MAX_IP_ADDR_LEN, "%s.%s.%s.%s",
        tmp[0], tmp[1], tmp[2], tmp[3]);

    return 0;
}

// ----------------------------------------------------------------------------
int OpenGPIBConnection()
{
    bool S3Found = false;
    char buffer[DEFAULT_BUFLEN];
	int i, num_listeners; //the number of listeners on the bus
	unsigned short address; //the address of a listener

	Addr4882_t instruments[GPIB_NUM_DEVICES], result[GPIB_NUM_DEVICES];

    //Initialise the GPIB interface
	SendIFC(0);
    //check for an error
	if(ibsta & ERR)
    {
        printf("Error Opening GPIB bus.");
	    CloseGPIBConnection();
        return CONNFAIL;
    }

    //Address 0 is used by the GPIB adaptor
    for(i = 0; i < GPIB_NUM_DEVICES - 1; i++) 
    {
		instruments[i] = i + 1;
    }
	instruments[GPIB_NUM_DEVICES - 1] = NOADDR;    

    //Find GPIB devices on the bus (listeners), and save their addresses in result
	FindLstn(0, instruments, result, GPIB_NUM_DEVICES);
	//check for error
    if(ibsta & ERR)
    {
        printf("Error: No GPIB devices found.");
	    CloseGPIBConnection();
        return CONNFAIL;
    }
    num_listeners = ibcnt;
	result[num_listeners] = NOADDR; //terminate the array after the last device found

    //Query each found device for it's ID.
	SendList(0, result, "*IDN?", 5L, NLend);
    if (ibsta & ERR)
    {
        printf("Error: Can't Query GPIB Devices.");
	    CloseGPIBConnection();
        return CONNFAIL;
    }

    for(i = 0; i < num_listeners; i++) 
    {
        
        Receive(0, result[i], buffer, DEFAULT_BUFLEN, '\r');
        if (ibsta & ERR)
        {
            printf("Error: Could not read from device.");
	        CloseGPIBConnection();
            return CONNFAIL;
        }

        address = GetPAD(result[i]);
        buffer[ibcnt] = '\0';

        if(!STRNCMP(buffer, "I: PPM,SCT-3,FOL,", 17))
        {
            //This device is a Sentinel 3.

            GPIBAddress = result[i];
            S3Found = true;

            ibconfig(0, IbcTMO, T1s);
            //Break out of the loop, so that we always command 
            // the first Sentinel 3 on the bus.
            break;
        }
    }
    if(!S3Found)
    {
        return CONNPASS;
    }

    return COMMPASS;
}
// ----------------------------------------------------------------------------
int CloseGPIBConnection()
{
    ibonl(0,0); //take the board offline
    return 0;
}
// ----------------------------------------------------------------------------


