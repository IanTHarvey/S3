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

#include <stddef.h>
#include <wchar.h>

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
	wchar_t Filename[MAX_PATH];
	size_t nconv;
	FILE *fid = NULL;

	int ret;

	// Set default
	strcpy_s(IPPort, MAX_IP_ADDR_LEN, IPV4Port);

	// Parse arguments - jumping out at the first sign of trouble is not ideal
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
			// Mandatory IPv4 address
			if (i < argc - 1)
			{
				wcstombs_s(&nconv, IPAddr, MAX_IP_ADDR_LEN, argv[i + 1],
					wcslen(argv[i + 1]) + 1);
				lastarg = i + 1;

				if (IPv4ToolsRemLeadZ(IPAddr))
				{
					fprintf(stderr, "Invalid IP address\n");
					ret = 1;
					goto RETURN;
				}

				IPAddrGiven = true;
			}
			else
			{
				fprintf(stderr, "No IP address specified\n");
				ret = 1;
				goto RETURN;
			}
		}
		else if (!wcscmp(_T("-p"), argv[i]))
		{
			// Optional IP port number
			if (i < argc - 1)
			{
				wchar_t *end;

				long p = wcstol(argv[i + 1], &end, 10);
				if (errno || end != argv[i + 1] + wcslen(argv[i + 1])
					|| end == argv[i + 1] || p < 0 || p > 65535)
				{
					fprintf(stderr, "Invalid IP port number\n");
					ret = 1;
					goto RETURN;
				}

				wcstombs_s(&nconv, IPPort, MAX_IP_ADDR_LEN, argv[i + 1],
					wcslen(argv[i + 1]) + 1);
				lastarg = i + 1;
			}
			else
			{
				fprintf(stderr, "No port specified\n");
				ret = 1;
				goto RETURN;
			}
		}
		else if (!wcscmp(_T("-o"), argv[i]))
		{
			// Optional S3 output file
			if (i < argc - 1)
			{
				errno_t err = _wfopen_s(&fid, argv[i + 1], _T("w"));
				lastarg = i + 1;

				if (err)
				{
					fprintf(stderr, "Failed to open file: %S (%d)",
						argv[i + 1], err);
					ret = 1;
					goto RETURN;
				}

				wcscpy_s(Filename, MAX_PATH, argv[i + 1]);
			}
		}
		else if (argv[i][0] == '-')
		{
			// Any known options handled previously
			fprintf(stderr, "Unknown command line argument: %S", argv[i]);
			ret = 1;
			goto RETURN;
		}
	}

	if (!IPAddrGiven)
	{
		fprintf(stderr, "No IP address given\n");
		ret = 1;
		goto RETURN;
	}

	if (lastarg >= argc - 1)
	{
		fprintf(stderr, "No remote command\n");
		ret = 1;
		goto RETURN;
	}

	*Cmd = '\0';

	// Extract S3 command
	char tmp[MAX_CMD_LEN];
	for(int i = lastarg + 1;  i < argc; i++)
	{
		strcat_s(Cmd, MAX_CMD_LEN, " ");	
		wcstombs_s(&nconv, tmp, MAX_CMD_LEN, argv[i], wcslen(argv[i]) + 1);
		strcat_s(Cmd, MAX_CMD_LEN, tmp);
	}

	// Try sending and get reply
	char res[DEFAULT_BUFLEN];
	ret = S3SendMessage(res, IPAddr, IPPort, Cmd);

	if (ret)
	{
		// S3Cmd command failed
		fprintf(stderr, "Result: Error (%d): %s\n", ret, res);
	}
	else
	{
		// S3Cmd succeeded, but S3 may have returned an error
		if (	!_strnicmp("OK:",	res, 3) ||
				!_strnicmp("I:",	res, 2) ||
				!_strnicmp("W:",	res, 2))
			ret = 0;
		else if (!_strnicmp("E:",	res, 2))
			ret = 2;
		else
			ret = 1; // Nonsense returned

		printf("Result(%d): %s\n", ret, res);

		if (fid)
		{
			fprintf(fid, "%s", res);
		}
	}

RETURN:	// Clean up

	if (fid)
	{
		fclose(fid);
	}

	return ret;
}

// -----------------------------------------------------------------------------
