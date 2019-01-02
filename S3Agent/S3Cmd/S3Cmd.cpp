// S3Open.cpp : Defines the entry point for the console application.
//

/* docopt spec:
S3Arg

Usage:
  s3cmd -i <ip_address> [-p <port_number>] <s3_cmd> [<argument>]...

Options:
-p <port_number>
-i <ip_address>
*/

#include "stdafx.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#include "S3CommsEth.h"

extern int S3SendMessage(char *RetMsg,
						 const char *IPAddr, const char *IPPort,
						 const char *Msg);
extern int IPv4ToolsRemLeadZ(char *IPAddr);

int _tmain(int argc, _TCHAR* argv[])
{
	char Cmd[MAX_CMD_LEN];
	char IPAddr[MAX_IP_ADDR_LEN];
	char IPPort[MAX_IP_ADDR_LEN];
	size_t nconv;

	strcpy_s(IPPort, MAX_IP_ADDR_LEN, IPV4Port);

	if (argc < 3)
	{
		printf("Not enough arguments\n");
		return 0;
	}

	int		lastarg = 1;
	bool	IPAddrGiven = false;

	for(int i = 1;  i < argc; i++)
	{
		if (!wcscmp(_T("-i"), argv[i]))
		{
			if (i < argc - 1)
			{
				wcstombs_s(&nconv, IPAddr, MAX_IP_ADDR_LEN, argv[i + 1], wcslen(argv[i + 1]) + 1);
				lastarg = i + 1;

				if (IPv4ToolsRemLeadZ(IPAddr))
				{
					printf("Invalid IP address\n");
					return 1;
				}

				IPAddrGiven = true;
			}
			else
			{
				printf("No IP address specified\n");
				return 1;
			}
		}
		else if (!wcscmp(_T("-p"), argv[i]))
		{
			if (i < argc - 1)
			{
				wchar_t *end;

				long p = wcstol(argv[i + 1], &end, 10);
				if (errno || end != argv[i + 1] + wcslen(argv[i + 1])
					|| end == argv[i + 1] || p < 0 || p > 65535)
				{
					printf("Invalid IP port number\n");
					return 1;
				}

				wcstombs_s(&nconv, IPPort, MAX_IP_ADDR_LEN, argv[i + 1], wcslen(argv[i + 1]) + 1);
				lastarg = i + 1;
			}
			else
			{
				printf("No port specified\n");
				return 1;
			}
		}
	}

	if (!IPAddrGiven)
	{
		printf("No IP address given\n");
		return 1;
	}

	if (lastarg >= argc - 1)
	{
		printf("No remote command\n");
		return 1;
	}

	*Cmd = '\0';

	char tmp[MAX_CMD_LEN];
	for(int i = lastarg + 1;  i < argc; i++)
	{
		strcat_s(Cmd, MAX_CMD_LEN, " ");	
		wcstombs_s(&nconv, tmp, MAX_CMD_LEN, argv[i], wcslen(argv[i]) + 1);
		strcat_s(Cmd, MAX_CMD_LEN, tmp);
	}

	char res[DEFAULT_BUFLEN];
	
	int ret = S3SendMessage(res, IPAddr, IPPort, Cmd);

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
