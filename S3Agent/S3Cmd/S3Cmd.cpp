// S3Open.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#include "S3CommsEth.h"

extern int S3SendMessage(char *RetMsg, const char *IPAddr, const char *Msg);
extern int IPv4ToolsRemLeadZ(char *IPAddr);

int _tmain(int argc, _TCHAR* argv[])
{
	char Cmd[1024];
	char IPAddr[MAX_IP_ADDR_LEN];
	size_t nconv;

	if (argc < 3)
	{
		printf("Not enough arguments\n");
		return 0;
	}

	wcstombs_s(&nconv, IPAddr, 128, argv[1], wcslen(argv[1]) + 1);

	if (IPv4ToolsRemLeadZ(IPAddr))
	{
		printf("Invalid IP address\n");
		return 1;
	}

	*Cmd = '\0';

	for(int i = 2;  i < argc; i++)
	{
		strcat_s(Cmd, 1024, " ");
		
		char tmp[128];
		wcstombs_s(&nconv, tmp, 128, argv[i], wcslen(argv[i]) + 1);
		strcat_s(Cmd, 1024, tmp);
	}

	char res[RETURN_MESSAGE_LEN];
	
	int ret = S3SendMessage(res, IPAddr, Cmd);

	if (ret)
		printf("Result: Error (%d): %s\n", ret, res);
	else
	{
		if (!_strnicmp("OK:", res, 3) || !_strnicmp("I:", res, 2))
			ret = 0;
		else ret = 1;

		printf("Result(%d): %s\n", ret, res);
	}

	return ret;
}

// -----------------------------------------------------------------------------
