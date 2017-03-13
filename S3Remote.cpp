
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef TRIZEPS
#include "S3ControllerX86/targetver.h"
#else
#define WINVER _WIN32_WCE
#include <ceconfig.h>

#endif
#include "afxpriv.h"

#include <ifdef.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>

#include "S3DataModel.h"
#include "S3GPIB.h"
#include "S3ControllerDlg.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif

#define WSA_ERR_STR_LEN	256
char	WSAErrString[WSA_ERR_STR_LEN];
const char *GetWSAErrString();

#define NO_USE_TREE_ICONS

// Socket stuff
UINT ListenThreadProc(LPVOID pParam);
// WSADATA wsaData;
SOCKET ListenSocket = INVALID_SOCKET;
SOCKET ClientSocket = INVALID_SOCKET;

char	RxBuf[S3_MAX_BUFLEN];
char	TxBuf[S3_MAX_BUFLEN];

extern pS3DataModel S3Data;

// Temporary
extern "C" {
	int GetPrimaryMACaddress(unsigned char *MAC, char *IP, char *IPMask);
}

// ----------------------------------------------------------------------------
// TODO: Enforce one or other?

int CS3ControllerDlg::RemoteOpenEth(void)
{
	int err = 0;
	
	if (m_IPThreadRun == false && m_EthEnabled && m_EthInactivityTimer > 3 * 1000)
	{
		int r = recv(ClientSocket, NULL, 0, 0);

		// TODO: WSAECONNRESET not required
		if (ClientSocket == INVALID_SOCKET ||
			(r == SOCKET_ERROR)) // && WSAGetLastError() == WSAECONNRESET))
		{
			unsigned char	MAC[6];
			char			IP[S3_MAX_IP_ADDR_LEN];
			char			IPMask[S3_MAX_IP_ADDR_LEN];
			
			int err = GetPrimaryMACaddress(MAC, IP, IPMask);

			S3SetMACAddr(MAC);
			S3SetIPAddrStr(IP);
			S3SetIPMaskStr(IPMask);

			if (!err)
			{
				err = ResetSocket();
				// err = InitSocket();
				m_EthInactivityTimer = 0;
			}
		}
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

	// Not CE
	// struct addrinfoW *result = NULL, *ptr = NULL, hints;
	ADDRINFOA *result = NULL, *ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	// Not CE
	// iResult = GetAddrInfoW(NULL, _T(S3_DEFAULT_PORT), &hints, &result);
	char port[8];
	sprintf_s(port, 8, "%d", S3GetIPPort());
	iResult = getaddrinfo(NULL, port, &hints, &result);
	
	if (iResult != 0)
	{
		debug_print(_T("InitSocket: getaddrinfo failed: %s\n"), GetWSAErrString());
		return 1;
	}
	
	// Not CE
	// TCHAR IPAddrStr[S3_MAX_IP_ADDR_LEN];
	// char IPAddrStr[S3_MAX_IP_ADDR_LEN];

	struct sockaddr_in *sockaddr_ipv4 = (struct sockaddr_in *) result->ai_addr;
	// Not CE
	// InetNtop(hints.ai_family, &sockaddr_ipv4->sin_addr, IPAddrStr, S3_MAX_IP_ADDR_LEN);
	// inet_ntoa(hints.ai_family, &sockaddr_ipv4->sin_addr, IPAddrStr, S3_MAX_IP_ADDR_LEN);
	// WSAAddressToString(result->ai_addr,*************************************); 

	// strcpy_s(IPAddrStr, S3_MAX_IP_ADDR_LEN, "NoIPString");
	
	// Create a SOCKET for the server to listen for client connections
	ListenSocket = INVALID_SOCKET;

	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET)
	{
		debug_print(_T("InitSocket: Error at socket(): %s\n"), GetWSAErrString());
		
		freeaddrinfo(result);
		return 1;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		debug_print(_T("InitSocket: Bind failed with error: %s\n"), GetWSAErrString());
	
		freeaddrinfo(result);
		closesocket(ListenSocket);
		return 1;
	}

	// Listen (waiting) for a connection
	// if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR)
	if (listen(ListenSocket, 4) == SOCKET_ERROR)
	{
		debug_print(_T("InitSocket: Listen failed with error: %s\n"), GetWSAErrString());
		
		freeaddrinfo(result);
		closesocket(ListenSocket);
		return 1;
	}

	// Not CE
	// FreeAddrInfoW(result);
	freeaddrinfo(result);

	m_IPThread = AfxBeginThread(ListenThreadProc, this);

	debug_print(_T("InitSocket: Started thread: 0x%x\n"), (int)m_IPThread);

	return 0;
}

// ----------------------------------------------------------------------------

int CS3ControllerDlg::CloseSocket(void)
{
	// if (opensocket(ListenSocket)
	closesocket(ListenSocket);
	return 0;
}

// ----------------------------------------------------------------------------

int CS3ControllerDlg::ResetSocket(void)
{
	CloseSocket();
	int err = InitSocket();
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
	int iResult, iSendResult;
	struct sockaddr sockaddr_ipv4; 
	int		sockaddr_ipv4_len;

	HWND *phObjectHandle = static_cast<HWND *>(pParam);

	CS3ControllerDlg *pObject = (CS3ControllerDlg *)pParam;

	pObject->m_IPThreadRun = true;


	debug_print(_T("ListenThreadProc: Started listening on socket...\n"));

	while (pObject->m_IPThreadRun)
	{	
		ClientSocket = INVALID_SOCKET;

		sockaddr_ipv4_len = sizeof(sockaddr_ipv4);

		// Accept a client socket generated by client connection request
		ClientSocket = accept(ListenSocket, &sockaddr_ipv4, &sockaddr_ipv4_len);
		if (ClientSocket == INVALID_SOCKET)
		{
			debug_print(_T("ListenThreadProc: Accept failed: %S\n"), GetWSAErrString());
			closesocket(ListenSocket);
			pObject->m_IPThreadRun = false;
			return 10;
		}

		// Receive until the peer shuts down the connection
		do
		{
			// Wait for data...
			iResult = recv(ClientSocket, RxBuf, S3_MAX_BUFLEN, 0);
			if (iResult > 0)
			{
				S3Data->m_FactoryMode = true;

				debug_print(_T("ListenThreadProc: Bytes received: %d\n"), iResult);
				RxBuf[iResult] = '\0';

				// Send a response back to the sender
				int err = pObject->ParseMsg(RxBuf, S3_ETH);
				strcpy_s(TxBuf, S3_MAX_BUFLEN, S3GPIBGetRetBuf());

				int len = strlen(TxBuf);
				// TxBuf[len++] = '\n';
				TxBuf[len] = '\0'; // For safety - NOT added to buffer!

				// Do NOT send '\0' terminator
				iSendResult = send(ClientSocket, TxBuf, len, 0);
				
				if (iSendResult == SOCKET_ERROR)
				{
					debug_print(_T("ListenThreadProc: Send failed: %d\n"), GetWSAErrString());
					closesocket(ClientSocket);
					pObject->m_IPThreadRun = false;
				
					return 2;
				}

				debug_print(_T("ListenThreadProc: Bytes sent: %d\n"), iSendResult);

				// Obsolete message for MFC dialogs only
				// PostMessage(pObject->GetSafeHwnd(), WM_UPDATE_CONTROL, 0, err);
				S3Data->m_FactoryMode = false;
			}
			else if (iResult == 0)
			{
				debug_print(_T("ListenThreadProc: Connection closing...\n"));
			}
			else
			{
				debug_print(_T("ListenThreadProc: Recv failed: %d\n"), GetWSAErrString());
				closesocket(ClientSocket);
				pObject->m_IPThreadRun = false;

				return 3;
			}

		} while (iResult > 0);

		// Shutdown the connection since we're done
		iResult = shutdown(ClientSocket, SD_SEND); 
		if (iResult == SOCKET_ERROR)
		{
			debug_print(_T("ListenThreadProc: Shutdown failed with error: %d\n"), GetWSAErrString());
			closesocket(ClientSocket);
			pObject->m_IPThreadRun = false;
			return 4;
		}
	}

	pObject->m_IPThreadRun = false;

	debug_print(_T("ListenThreadProc: Listen thread terminated normally\n"));

	return 0;   // Thread completed successfully
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
        //If this message was not a S3Agent prefixed command, save it to display in the remote mode header
        S3SetPrevRxedMsg(Msg);
        S3SetPrevRemoteSrc(MsgSrc);
    }
    
	// Handle any non-S3 messages...

	// IP address handshake
	if (!STRNCMP(Msg, "IP:", 3))
	{
		// TODO: Verify valid and conform IP address here
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
	int iSendResult, iResult;
	char recvbuf[S3_MAX_BUFLEN];

	// strcpy_s(recvbuf, S3_MAX_BUFLEN, S3GPIBGetRetBuf());
	strcpy_s(recvbuf, S3_MAX_BUFLEN, "Test message");

	iResult = strlen(recvbuf);

	iSendResult = send(ClientSocket, recvbuf, iResult, 0);
	if (iSendResult == SOCKET_ERROR)
	{
		debug_print(_T("SendMsg: Send failed: %d\n"), WSAGetLastError());
		closesocket(ClientSocket);
		return 1;
	}

	debug_print(_T("SendMsg: Bytes sent: %d\n"), iSendResult);

	return 0;
}

// ----------------------------------------------------------------------------

extern "C" {

int GetPrimaryMACaddress(unsigned char *MAC, char *IPStr, char *IPMaskStr)
{
	char i;
	for(i = 0; i < 6; i++) MAC[i] = 0;
	*IPStr = '\0';
	*IPMaskStr = '\0';

	IP_ADAPTER_INFO AdapterInfo[16];       // Allocate information for up to 16 NICs
	DWORD dwBufLen = sizeof(AdapterInfo);  // Save memory size of buffer

	DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
	if (dwStatus != ERROR_SUCCESS)
	{
		debug_print(_T("GetPrimaryMACaddress: GetAdaptersInfo failed. err = %d\n"), GetLastError());
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
	// both WiFi and Ethernet with default gateways!!, I guess you can use the
	// Type field to pick Ethernet..
	//
	// ITH: This is unproven...
	
	do
	{
		pIPAddrString =	&pAdapterInfo->IpAddressList;
        pIPGwString =	&pAdapterInfo->GatewayList;

		while(pIPAddrString)
		{
			ULONG ulIPMask, ulIPGateway;

			ulIPMask = ntohl(inet_addr( pIPAddrString->IpMask.String));
            ulIPGateway = ntohl(inet_addr( pIPGwString->IpAddress.String));

            if (!ulIPMask)
			{
                pIPAddrString = pIPAddrString->Next;
                continue;
            }

			// First adapter with a default gateway
            if (ulIPGateway)
			{
				primary = pAdapterInfo;
				break;
            }

            pIPAddrString = pIPAddrString->Next;
		}

		pAdapterInfo = pAdapterInfo->Next;    // Progress through linked list
	} while (pAdapterInfo);                   // Terminate if last adapter

	if (primary)
	{
		debug_print(_T("GetPrimaryMACaddress: GetAdaptersInfo: Found primary\n"));

		for (int i = 0; i < 6; i++)
			MAC[i] = primary->Address[i];

		strcpy_s(IPStr, S3_MAX_IP_ADDR_LEN,
			primary->IpAddressList.IpAddress.String);
		strcpy_s(IPMaskStr, S3_MAX_IP_ADDR_LEN,
			primary->IpAddressList.IpMask.String);
	}
	else
	{
		debug_print(_T("GetPrimaryMACaddress: GetAdaptersInfo: Failed to find primary adapter\n"));

		strcpy_s(IPStr, S3_MAX_IP_ADDR_LEN, "No Ethernet");
		strcpy_s(IPMaskStr, S3_MAX_IP_ADDR_LEN, "N/A");

		return 1;
	}

	return 0;
} // GetPrimaryMACaddress


} // Extern C

// ----------------------------------------------------------------------------

const char *GetWSAErrString()
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

// ------------------------------ The End -------------------------------------
