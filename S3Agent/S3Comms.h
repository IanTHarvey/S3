#pragma once
#include "S3AgentDlg.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "defines.h"

#include "ni4882.h"

extern Sentinel3DataBundle Sentinel3;
extern bool	m_USBEnabled;
extern CS3USBVCP *m_COMPort;
extern int connectionmethod; 
extern SOCKET ConnectSocket;
// TODO: Make static members
extern SOCKET ConnectSocket;
extern char	RxBuf[DEFAULT_BUFLEN];

extern Addr4882_t GPIBAddress;

extern CString COM_name;
extern CString IPv4Addr;
extern CString IPV4Port;


int OpenConnectUSB(void);
void CloseConnectUSB(void);
int OpenSocketSC3(void);
int CloseSocketSC3(void);
CString SendSentinel3Message(CString message);
int SendMessageSC3_2(const char *msg);
int SendMessageOpenSocketSC3(const char *msg);
int IPv4ToolsRemLeadZ(char *IPAddr);
int OpenGPIBConnection();
int CloseGPIBConnection();