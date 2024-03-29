// S3AgentEth2.cpp : Defines the exported functions for the DLL application.
//

#include "targetver.h"
#include "afxwin.h"
#include "stdafx.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "S3CommsEth.h"

// extern "C" {
extern int S3CmdCloseSocket(char *RetMsg);
extern int S3CmdOpenSocket(char *RetMsg,
						   const char *IPv4Addr,
						   const char *IPPort);
extern int S3CmdSend(char *ReMsg, const char *TxBuf);
// };

SOCKET ConnectSocket;

char	IPV4Port[MAX_IP_ADDR_LEN] = {"65000"};
char	IPv4Addr[MAX_IP_ADDR_LEN];
char	RxBuf[DEFAULT_BUFLEN];

// ----------------------------------------------------------------------------
#define WSA_ERR_STR_LEN	256
char	WSAErrString[WSA_ERR_STR_LEN];

const char *S3GetWSAErrString()
{
	wchar_t *s = NULL;
	int err = WSAGetLastError();
	FormatMessageW(	FORMAT_MESSAGE_ALLOCATE_BUFFER |
					FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&s, 0, NULL);

	sprintf_s(WSAErrString, WSA_ERR_STR_LEN, "%d: %S", err, s);

	LocalFree(s); // FormatMessageW uses LocalAlloc()

	return WSAErrString;
}

// -----------------------------------------------------------------------------

int S3Open(char *RetMsg, const char *IPAddr, const char *IPPort)
{
	int ret = S3CmdOpenSocket(RetMsg, IPAddr, IPPort);

	return ret;
}

// -----------------------------------------------------------------------------

int S3SendMessage(char *RetMsg, const char *IPAddr,  const char *IPPort, const char *Msg)
{
	int ret = S3CmdOpenSocket(RetMsg, IPAddr, IPPort);

	if (!ret)
	{
		ret = S3CmdSend(RetMsg, Msg);

		if (ret)
			strcpy_s(RetMsg, DEFAULT_BUFLEN, "Error sending message");

		char CloseMsg[DEFAULT_BUFLEN];

		ret = S3CmdCloseSocket(CloseMsg);

		if (ret)
			strcpy_s(RetMsg, DEFAULT_BUFLEN, CloseMsg);
	}

	return ret;
}

// -----------------------------------------------------------------------------