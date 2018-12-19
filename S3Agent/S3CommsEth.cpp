#include "targetver.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "afxwin.h"
#include "S3AgentEth2.h"
#include "S3CommsEth.h"
#include "defines.h"

char LastTerm;

extern void ReadFromSerialPort();

extern int S3TimerInit();
extern int S3TimerStart(unsigned char Tid);
extern int S3TimerStop(unsigned char Tid);

extern const char *S3GetWSAErrString();

extern char RxBuf[];

// ----------------------------------------------------------------------------
// Open the network socket
int OpenSocketSC3(const char *IPv4Addr)
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
  
        DWORD timeout = 1 * 1000; //1ms timeout
        setsockopt(ConnectSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR)
        {
			printf("Connect error: %s\n", S3GetWSAErrString());
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
	}

        }
    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET)
    {
        printf("Unable to connect to server\n");
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
/*
CString SendSentinel3Message(CString message)
{
    CommsMUTEX.lock();

    if (Sentinel3.isConnected == false)
    {
        return (CString)"ERROR: NO S3 Connected\r\n";
    }

    CString RetString;
    CString tmp;

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

				S3TimerStart(0);
                int err = SendMessageSC3_2(TxBuf);
				S3TimerStop(0);

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
*/

// ----------------------------------------------------------------------------
// Once we have an open socket to a Sentinel 3, send a message, and recieve
// its response

#define S3_RECV_RETRIES	10

int SendMessageOpenSocketSC3(char *RetMsg, const char *TxBuf)
{
    size_t len = strlen(TxBuf);
    int iResult; //, ClientErr;

	*RxBuf = '\0';
	int cnt = 0;

	// If running with remote view enabled, buffer may be full of data from
	// S3 update, so read and clear before sending command
	if (false)
		while((iResult = recv(ConnectSocket, RxBuf, DEFAULT_BUFLEN, 0))!= SOCKET_ERROR)
			cnt++;

    *RxBuf = '\0';
    // Send an initial buffer - NOT terminator
	S3TimerStart(1);
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

	int retries = 0;
	while((iResult = recv(ConnectSocket, RxBuf, DEFAULT_BUFLEN, 0)) == SOCKET_ERROR &&
			retries < S3_RECV_RETRIES)
		retries++;

	S3TimerStop(1);

    if (iResult > 0 && retries < S3_RECV_RETRIES)
    {
		// If already terminated by S3, replace with NULL terminator
        if (RxBuf[iResult - 1] == '\n' || RxBuf[iResult - 1] == '\0')
		{
			LastTerm = RxBuf[iResult - 1];
			RxBuf[iResult - 1] = '\0';
		}
		else
		{
			LastTerm = '-';
			RxBuf[iResult] = '\0';
		}

		SaveValidIPAddr();
    }

	strcpy_s(RetMsg, RETURN_MESSAGE_LEN, RxBuf);

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

int SaveValidIPAddr()
{
    // Save 'valid IP' adddress
    char Filename[MAX_SCRIPT_LEN];

    sprintf_s(Filename, MAX_SCRIPT_LEN, "%ls\\S3IP.txt", DataLocStr);

    FILE *fid;
    int err = 0;

    err = fopen_s(&fid, Filename, "w");
    if (err)
    {
        return 1;
    }

    CStringA tmpA(IPv4Addr);
    char	IPAddr[MAX_IP_ADDR_LEN];
    strcpy_s(IPAddr, MAX_IP_ADDR_LEN, (LPCSTR)tmpA);

    fprintf(fid, "%s", IPAddr);

    fclose(fid);

	return 0;
}

// ----------------------------------------------------------------------------