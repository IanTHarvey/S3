#include "targetver.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "afxwin.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#include "S3CommsEth.h"

char LastTerm;

extern int S3TimerInit();
extern int S3TimerStart(unsigned char Tid);
extern int S3TimerStop(unsigned char Tid);

extern const char *S3GetWSAErrString();
extern char RxBuf[];

// ----------------------------------------------------------------------------
// Open the network socket

int S3CmdOpenSocket(char *RetMsg, const char *IPAddr, const char *IPPort)
{
    *RetMsg = '\0';
	
	WSADATA wsaData;
    struct addrinfo *result = NULL, hints;

    int ClientErr = 0;
    ConnectSocket = INVALID_SOCKET;

    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        sprintf_s(RetMsg, DEFAULT_BUFLEN,
			"WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(IPAddr, IPPort, &hints, &result);
    if (iResult != 0)
    {
		strcpy_s(RetMsg, DEFAULT_BUFLEN, S3GetWSAErrString());
        WSACleanup();
        return 1;
    }

	// Create a socket for connecting to server
	ConnectSocket = socket(result->ai_family, result->ai_socktype,
		result->ai_protocol);

	if (ConnectSocket == INVALID_SOCKET)
	{
		strcpy_s(RetMsg, DEFAULT_BUFLEN, S3GetWSAErrString());

		WSACleanup();
		return 1;
	}

	// Put socket in non-blocking mode
	u_long block = 1;
	if (ioctlsocket(ConnectSocket, FIONBIO, &block) == SOCKET_ERROR)
	{
		strcpy_s(RetMsg, DEFAULT_BUFLEN, S3GetWSAErrString());

		closesocket(ConnectSocket);
		return 1;
	}

	// Connect to server.
	iResult = connect(ConnectSocket, result->ai_addr, (int)result->ai_addrlen);

	freeaddrinfo(result);

	if (iResult == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			// Connection failed
			strcpy_s(RetMsg, DEFAULT_BUFLEN, S3GetWSAErrString());

			closesocket(ConnectSocket);
			return 1;
		}

		// Connection pending
		fd_set setW, setE; // Write & exception

		FD_ZERO(&setW);
		FD_SET(ConnectSocket, &setW);
		FD_ZERO(&setE);
		FD_SET(ConnectSocket, &setE);

		timeval time_out = {0};
		time_out.tv_sec = 1;
		time_out.tv_usec = 0; 

		int ret = select(0, NULL, &setW, &setE, &time_out);
		if (ret <= 0)
		{
			// select() failed or connection timed out
			if (ret == 0)
				WSASetLastError(WSAETIMEDOUT);

			strcpy_s(RetMsg, DEFAULT_BUFLEN, S3GetWSAErrString());

			closesocket(ConnectSocket);

			return 1;
		}

		if (FD_ISSET(ConnectSocket, &setE))
		{
			// Connection failed due to exception
			char	err;
			int		err_s;

			getsockopt(ConnectSocket, SOL_SOCKET, SO_ERROR, &err, &err_s);
			closesocket(ConnectSocket);
			WSASetLastError(err);

			return 1;
		}
	}

	// Put socket in blocking mode as we have a good connection
    block = 0;
    if (ioctlsocket(ConnectSocket, FIONBIO, &block) == SOCKET_ERROR)
    {
		strcpy_s(RetMsg, DEFAULT_BUFLEN, S3GetWSAErrString());

        closesocket(ConnectSocket);
        return 1;
    }

	strcpy_s(IPv4Addr, MAX_IP_ADDR_LEN, IPAddr);

    return 0;
}


// ----------------------------------------------------------------------------
// Close the network socket gracefully

int S3CmdCloseSocket(char *RetMsg)
{
	*RetMsg = '\0';
    int ClientErr = 0;

    // Shut down the connection since no more data will be sent
    int iResult = shutdown(ConnectSocket, SD_SEND);

    if (iResult == SOCKET_ERROR)
    {
		strcpy_s(RetMsg, DEFAULT_BUFLEN, S3GetWSAErrString());
		closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // Receive until the peer closes the connection
    do {
        iResult = recv(ConnectSocket, RxBuf, DEFAULT_BUFLEN, 0);
        if (iResult > 0)
            RxBuf[iResult] = '\0';
        else
			strcpy_s(RetMsg, DEFAULT_BUFLEN, S3GetWSAErrString());

    } while (iResult > 0);

    // Cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return ClientErr;
}

// ----------------------------------------------------------------------------
// Once we have an open socket to a Sentinel 3, send a message, and recieve
// its response

#define S3_RECV_RETRIES	10

int S3CmdSend(char *RetMsg, const char *TxBuf)
{
    size_t len = strlen(TxBuf);
    int iResult;

	*RetMsg = '\0';
	*RxBuf = '\0';
	int cnt = 0;

	// If running with remote view enabled, buffer may be full of data from
	// S3 update, so read and clear before sending command
	if (false)
		while((iResult = recv(ConnectSocket, RxBuf, DEFAULT_BUFLEN, 0))!= SOCKET_ERROR)
			cnt++;

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
    }

	strcpy_s(RetMsg, DEFAULT_BUFLEN, RxBuf);

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
        return 1;

    unsigned char count = 1;
    unsigned char	i;

    for (i = 1; i < len; i++)
    {
        if (IPAddr[i] == '.')
        {
            pbyte[count++] = IPAddr + i + 1;

            if (count == 5)
                return 1;
        }
    }

	if (count != 4)
		return 1;

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
