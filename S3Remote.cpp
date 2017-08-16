
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "stdafx.h"
#include "tchar.h"
#include <ifdef.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>


#include "S3DataModel.h"
#include "S3GPIB.h"
#include "S3ControllerDlg.h"

char	WSAErrString[WSA_ERR_STR_LEN];

#define NO_USE_TREE_ICONS

// Socket stuff
UINT ListenThreadProc(LPVOID pParam);
// SOCKET m_ListenSocket = INVALID_SOCKET;

char	RxBuf[S3_MAX_GPIB_CMD_LEN];
char	TxBuf[S3_MAX_GPIB_RET_LEN];

extern pS3DataModel S3Data;

// Temporary
extern "C" {
	int S3GetPrimaryMACaddress();
}

// ----------------------------------------------------------------------------
// This will force re-initialisation of the listening socket if ethernet
// activity is not received - if the connection is lost, there'e no way
// for S3 to determine this without the remote client providing some sort
// of keepawake protocol. So easier to assume connection has been lost
// if no remote commands received for S3_ETH_ACTIVITY_TIMEOUT, and reset
// the listening socket.

int CS3ControllerDlg::RemoteOpenEth(void)
{
	int err = 0;
	
	if (m_EthEnabled && m_EthInactivityTimer > S3_ETH_ACTIVITY_TIMEOUT)
	{
		debug_print("Attempting restart: Inactivity: %d\n", m_EthInactivityTimer);

		// TODO: WSAECONNRESET not required
		if (1) // ClientSocket == INVALID_SOCKET ||
			// (r == SOCKET_ERROR)) // && WSAGetLastError() == WSAECONNRESET))
		{
			// Kill the listener thread
			m_IPThreadRun = false;
			
			// This will kill the listener thread as accept() will fail with
			// WSAEINTR (10004)
			err = CloseSocket();

			if (!err)
			{	
				err = S3GetPrimaryMACaddress();

				if (!err)
				{
					err = InitSocket();
					
					if (err)
						debug_print("Reset socket failed: %d\n", err);

					m_EthInactivityTimer = 0;
				}
			}
		}
		
		if (err)
			debug_print("RemoteOpenEth: Restart failed\n");
	}

	return err;
}

// ----------------------------------------------------------------------------

int CS3ControllerDlg::RemoteOpenUSB(void)
{
	if (m_USBEnabled)
	{
		if (m_COMPort.IsPortOpen())
			return 0;

		if (m_COMPort.Init(this) == 1)
		{
			if (!m_COMPort.OpenPort())
				return 2;
		}
		else
		{
			return 3;
		}
	}

	return 0;
}

// ----------------------------------------------------------------------------

CString CS3ControllerDlg::GetUSBPortName(void)
{
	CString	tmp;

	if (m_USBEnabled)
		m_COMPort.GetOpenPortName(tmp);
	else
		tmp = _T("Disabled");

	return tmp;
}

// ----------------------------------------------------------------------------

CString CS3ControllerDlg::GetUSBDriverType(void)
{
	if (m_USBEnabled)
		return	m_COMPort.GetDriverType();
	else return _T("N/A");
}

// ----------------------------------------------------------------------------

bool CS3ControllerDlg::GetUSBEnabled()
{
	return (m_USBEnabled == true);
}

// ----------------------------------------------------------------------------

void CS3ControllerDlg::SetUSBEnabled(bool enable)
{
	if (m_USBEnabled && !enable)
		m_COMPort.ClosePort();
	else if (!m_USBEnabled && enable)
		m_COMPort.OpenPort();
	
	m_USBEnabled = enable;
}

// ----------------------------------------------------------------------------

int CS3ControllerDlg::InitSocket(void)
{
	int		iResult;

	ADDRINFOA *result = NULL, *ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family =	AF_INET;
	hints.ai_socktype =	SOCK_STREAM;
	hints.ai_protocol =	IPPROTO_TCP;
	hints.ai_flags =	AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	char port[8];
	sprintf_s(port, 8, "%d", S3GetIPPort());
	iResult = getaddrinfo(NULL, port, &hints, &result);
	
	if (iResult != 0)
	{
		debug_print("InitSocket: getaddrinfo failed: %s\n", S3GetWSAErrString());
		return 1;
	}
	
	struct sockaddr_in *sockaddr_ipv4 = (struct sockaddr_in *) result->ai_addr;
	
	// Create a SOCKET to listen for client connections
	m_ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (m_ListenSocket == INVALID_SOCKET)
	{
		debug_print("InitSocket: socket failed: %s\n", S3GetWSAErrString());
		
		freeaddrinfo(result);
		return 1;
	}

	// Set up the TCP listening socket
	iResult = bind(m_ListenSocket, result->ai_addr, (int)result->ai_addrlen);

	freeaddrinfo(result);

	if (iResult == SOCKET_ERROR)
	{
		debug_print("InitSocket: bind failed: %S\n", S3GetWSAErrString());
	
		closesocket(m_ListenSocket);
		return 1;
	}

	// Listen (waiting) for a connection
	if (listen(m_ListenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		debug_print("InitSocket: listen failed: %S\n", S3GetWSAErrString());
		
		closesocket(m_ListenSocket);
		return 1;
	}

	m_IPThread = AfxBeginThread(ListenThreadProc, this);

	debug_print("InitSocket: Started thread: 0x%x\n", (int)m_IPThread);

	return 0;
}

// ----------------------------------------------------------------------------

int CS3ControllerDlg::CloseSocket(void)
{
	int err = 0;

	// if (opensocket(m_ListenSocket)
	if (m_ListenSocket != INVALID_SOCKET)
	{
		err = closesocket(m_ListenSocket);

		m_ListenSocket = INVALID_SOCKET;

		if (err)
			debug_print("CloseSocket: closesocket failed: %S\n", S3GetWSAErrString());
	}

	return err;
}

// ----------------------------------------------------------------------------

int CS3ControllerDlg::ResetSocket(void)
{
	int err;
	
	err = CloseSocket();
	err = InitSocket();
	
	return err;
}

// ----------------------------------------------------------------------------

void CS3ControllerDlg::RemoteClose()
{
	CloseSocket();

	if (m_USBEnabled)
	{
		m_COMPort.Close();
	}
}

// ----------------------------------------------------------------------------

UINT ListenThreadProc(LPVOID pParam)
{
	int				iResult, iSendResult;
	struct sockaddr sockaddr_ipv4; 
	int				sockaddr_ipv4_len = sizeof(sockaddr_ipv4);
	UINT			err = 0;

	CS3ControllerDlg *pObject = (CS3ControllerDlg *)pParam;

	pObject->m_IPThreadRun = true;

	debug_print("0x%x: Started listening on socket...\n", (int)pObject->m_IPThread);

	while (pObject->m_IPThreadRun && S3Data)
	{	
		// Wait for a client socket request
		SOCKET ClientSocket = accept(pObject->m_ListenSocket, &sockaddr_ipv4, &sockaddr_ipv4_len);

		if (ClientSocket == INVALID_SOCKET)
		{
			int err = WSAGetLastError();
			
			// If accept() interuppted by external closesocket(), we won't worry
			if (err != WSAEINTR)
			{
				debug_print("ListenThreadProc: Accept failed: %S\n", S3GetWSAErrString());
				err = 10;
			}
			else
				err = 0;

			break;
		}
		else
		{
			debug_print("ListenThreadProc: Accept OK:\n");

			// Receive until the client shuts down the connection
			do
			{
				// Wait for data...
				iResult = recv(ClientSocket, RxBuf, S3_MAX_GPIB_CMD_LEN, 0);
				
				// We may have closed in the meantime...
				if (!S3Data)
					return 0;
				
				if (iResult > 0)
				{
					S3Data->m_FactoryMode = true;

					debug_print("ListenThreadProc: Bytes received: %d\n", iResult);
					RxBuf[iResult] = '\0';

					// Send a response back to the client
					int err = pObject->ParseMsg(RxBuf, S3_ETH);
					strcpy_s(TxBuf, S3_MAX_GPIB_RET_LEN, S3GPIBGetRetBuf());

					int len = strlen(TxBuf);
					TxBuf[len] = '\0'; // For safety - NOT added to buffer!

					// Do NOT send '\0' terminator
					iSendResult = send(ClientSocket, TxBuf, len, 0);
					
					if (iSendResult == SOCKET_ERROR)
					{
						debug_print("ListenThreadProc: Send failed: %s\n", S3GetWSAErrString());
						err = 2;
						break;
					}

					debug_print("ListenThreadProc: Bytes sent: %d\n", iSendResult);
					S3Data->m_FactoryMode = false;
				}
				else if (iResult == 0)
				{
					debug_print("ListenThreadProc: Connection closing...\n");
				}
				else
				{
					debug_print("ListenThreadProc: Recv failed: %s\n", S3GetWSAErrString());
					err = 3;
					break;
				}
			} while (iResult > 0 && S3Data);

			if (err)
				break;

			// Shutdown the connection since we're done
			iResult = shutdown(ClientSocket, SD_SEND); 
			if (iResult == SOCKET_ERROR)
			{
				debug_print("ListenThreadProc: Shutdown failed with error: %S\n", S3GetWSAErrString());
				err = 4;
				break;
			}
		} // accept OK
	}

	if (!err)
		debug_print("0x%x: Listen thread terminated normally\n", (int)pObject->m_IPThread);
	else
	{
		debug_print("0x%x: Listen thread terminated: %d\n", (int)pObject->m_IPThread, err);
		closesocket(pObject->m_ListenSocket);
	}

	return err;
}

// ----------------------------------------------------------------------------
// Interface to S3DataModel. Do not attempt do anything with UI from here, it
// should only be updated from the main UI thread, not the network listener
// thread.

int CS3ControllerDlg::ParseMsg(const char *pMsg, char MsgSrc)
{
	int		i = 0;
	char	Msg[S3_MAX_MESSAGE_LEN];
	int		err = 0;

	if (MsgSrc == S3_ETH)
		m_EthInactivityTimer = 0;

	S3SetRemoteSrc(MsgSrc);

	strcpy_s(Msg, S3_MAX_MESSAGE_LEN, pMsg);

    if(STRNCMP(Msg,"S3",2))
    { 
        //If this message was not a S3Agent prefixed command, save it to
		// display in the remote mode header
        S3SetPrevRxedMsg(Msg);
        S3SetPrevRemoteSrc(MsgSrc);
    }
    
	// Handle any non-S3 messages...

	// IP address handshake
	if (!STRNCMP(Msg, "IP:", 3))
	{
		// TODO: Verify valid and conformant IP address here
		char *tmp = S3GPIBGetRetBuf();
		strcpy_s(tmp, 3, "OK");
	}
	else
	{
		// Pass on S3-only messages
		err = S3ProcessGPIBCommand(Msg);

		m_GDIStatic.S3GDIRemoteCmd();

		if (err)
		{
			CString tmp, tmp1;

			tmp = S3GPIBGetRetBuf();
		}
	}

	return err;
}

// ----------------------------------------------------------------------------

int CS3ControllerDlg::SendMsg(const char *pMsg)
{
/*
	int iSendResult, iResult;
	char recvbuf[S3_MAX_GPIB_CMD_LEN];

	// strcpy_s(recvbuf, S3_MAX_BUFLEN, S3GPIBGetRetBuf());
	strcpy_s(recvbuf, S3_MAX_GPIB_CMD_LEN, "Test message");

	iResult = strlen(recvbuf);

	iSendResult = send(ClientSocket, recvbuf, iResult, 0);
	if (iSendResult == SOCKET_ERROR)
	{
		debug_print("SendMsg: Send failed: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		return 1;
	}

	debug_print("SendMsg: Bytes sent: %d\n", iSendResult);
	*/

	return 0;
}

// ----------------------------------------------------------------------------

extern "C" {

int S3GetPrimaryMACaddress()
{
	IP_ADAPTER_INFO AdapterInfo[16];       // Allocate for up to 16 NICs
	DWORD dwBufLen = sizeof(AdapterInfo);  // Save memory size of buffer

	DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
	if (dwStatus)
	{
		debug_print("S3GetPrimaryMACaddress: GetAdaptersInfo failed. err = %d\n", GetLastError());
		return 1;
	}

	unsigned long	min = MAXULONG32;
	unsigned char	cnt = 0;

	PIP_ADDR_STRING pIPAddrString, pIPGwString;

	PIP_ADAPTER_INFO primary = NULL;
	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo; // Pointer to  current adapter info

	// See: http://stackoverflow.com/questions/5946704/c-get-local-ip-address-differentiate-between-the-vpn-and-local-network
	// ...cycle through the list and pick the first with a valid subnet mask
	// and a default gateway (my VPN adapter didn't have one). If you have
	// both WiFi and Ethernet with default gateways! I guess you can use the
	// Type field to pick Ethernet..
	
	do
	{
		pIPAddrString =	&pAdapterInfo->IpAddressList;
        pIPGwString =	&pAdapterInfo->GatewayList;
		
		while(pIPAddrString)
		{
			ULONG ulIPSubnet, ulIPGateway;

			ulIPSubnet = ntohl(inet_addr( pIPAddrString->IpMask.String));
            ulIPGateway = ntohl(inet_addr( pIPGwString->IpAddress.String));

            if (!ulIPSubnet)
			{
                pIPAddrString = pIPAddrString->Next;
                continue;
            }

#ifdef TRIZEPS
			// We know what it'll be called
			if (!strcmp("ENET1", pAdapterInfo->AdapterName))
			{
				primary = pAdapterInfo;
				break;
            }
#else
			// Use first adapter with a default gateway on a proper Windows PC
			if (ulIPGateway)
			{
				primary = pAdapterInfo;
				break;
            }
#endif
            pIPAddrString = pIPAddrString->Next;
		}

		pAdapterInfo = pAdapterInfo->Next; // Next in linked list
	} while(pAdapterInfo); // Terminate if last adapter

	unsigned char	MAC[MAC_LEN];
	wchar_t			IP[S3_MAX_IP_ADDR_LEN];
	wchar_t			IPSubnet[S3_MAX_IP_ADDR_LEN];
	
	int err = 0;
	
	if (primary)
	{
		// debug_print("S3GetPrimaryMACaddress: GetAdaptersInfo: Found primary\n");
		for (char i = 0; i < MAC_LEN; i++)
			MAC[i] = primary->Address[i];

		swprintf_s(IP, S3_MAX_IP_ADDR_LEN, _T("%S"), 
			primary->IpAddressList.IpAddress.String);
		swprintf_s(IPSubnet, S3_MAX_IP_ADDR_LEN, _T("%S"),
			primary->IpAddressList.IpMask.String);
	}
	else
	{
		debug_print("S3GetPrimaryMACaddress: GetAdaptersInfo: Failed to find primary adapter\n");

		for (char i = 0; i < MAC_LEN; i++)
			MAC[i] = 0;

		wcscpy_s(IP, S3_MAX_IP_ADDR_LEN, _T("No Ethernet"));
		wcscpy_s(IPSubnet, S3_MAX_IP_ADDR_LEN, _T("N/A"));

		err = 1;
	}

	if (1) // S3GetDHCP())
	{	
		S3SetMACAddr(MAC);
		S3SetIPAddrStr(IP, false);
		S3SetIPSubnetStr(IPSubnet);
	}

	// S3WriteEthConfig();

	return 0;
} // S3GetPrimaryMACaddress


} // Extern C

// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------

int isIp_v4(const char *ip);

int S3ValidateIPAddress(const wchar_t *addr)
{
	char c_addr[S3_MAX_IP_ADDRESS_LEN];

	sprintf_s(c_addr, S3_MAX_IP_ADDRESS_LEN, "%S", addr);

	if (!isIp_v4(c_addr))
		return 1;

	unsigned long ulAddr = inet_addr(c_addr);

	if (ulAddr == INADDR_NONE)
		return 1;

	struct sockaddr sockaddr_ipv4;
	int	len = sizeof(sockaddr_ipv4);

	if (WSAStringToAddress((wchar_t *)addr, AF_INET, NULL, &sockaddr_ipv4, &len))
		return 1;

	return 0;
}

// ----------------------------------------------------------------------------
// Just use registry

int S3WriteEthConfig()
{
	return 0;

	if (S3GetDHCP())
	{
		DeleteFile(_T("\\Flashdisk\\S3\\S3EthCfg.txt"));
		return 0;
	}

	FILE *fid;

	int err = fopen_s(&fid, "\\Flashdisk\\S3\\S3EthCfg.txt", "w");

	if (err)
		return 1;

	fprintf(fid, "%s\n", S3GetIPAddrStr());
	fprintf(fid, "%s\n", S3GetIPSubnetStr());

	fclose(fid);

	return 0;
}

// ----------------------------------------------------------------------------

int S3ReadEthConfig()
{
	if (S3FileExist(_T("\\Flashdisk\\S3\\S3EthCfg.txt")))
	{
		FILE *fid;

		int err = fopen_s(&fid, "\\Flashdisk\\S3\\S3EthCfg.txt", "r");

		if (err)
		{
			return 1;
		}

		char	cbuf[3 * (S3_MAX_IP_ADDR_LEN + 5) + 1];
		wchar_t	wbuf[3 * (S3_MAX_IP_ADDR_LEN + 5) + 1];
		wchar_t	IPAddrBuf[3 * (S3_MAX_IP_ADDR_LEN + 5) + 1];

		fgets(cbuf, 3 * (S3_MAX_IP_ADDR_LEN + 5), fid);
		wsprintf(IPAddrBuf, _T("%S"), cbuf);

		fgets(cbuf, 3 * (S3_MAX_IP_ADDR_LEN + 5), fid);
		wsprintf(wbuf, _T("%S"), cbuf);
		S3SetIPSubnetStr(wbuf);

		S3SetIPAddrStr(IPAddrBuf, true);

		//fgets(cbuf, 3 * (S3_MAX_IP_ADDR_LEN + 5), fid);
		//wsprintf(wbuf, _T("%S"), cbuf);
		//S3SetIPGatewayStr(wbuf);

		fclose(fid);
	}
	else
	{
		S3SetDHCP(true);
	}

	return 0;
}

// ----------------------------------------------------------------------------

int isIp_v4(const char *c_addr)
{
	char	ip[S3_MAX_IP_ADDRESS_LEN];

	strcpy_s(ip, S3_MAX_IP_ADDRESS_LEN, c_addr);

	int		num;
	int		flag = 1;
	int		counter = 0;
	char	*context;
	char	*p = strtok_s(ip, ".", &context);

	while (p && flag )
	{
		num = atoi(p);

		if (num >= 0 && num <= 255 && (counter++ < 4))
		{
			flag = 1;
			p = strtok_s(NULL, ".", &context);
		}
		else
		{
			flag = 0;
			break;
		}
	}

	return flag && (counter == 4);
}

// ------------------------------ The End -------------------------------------
