// S3Agent-specific comms functions

#pragma once
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <winsock2.h>
#include <ws2tcpip.h>

#include "defines.h"

extern Sentinel3DataBundle Sentinel3;
extern int connectionmethod; 
extern SOCKET ConnectSocket;
// TODO: Make static members
extern SOCKET ConnectSocket;
extern char	RxBuf[DEFAULT_BUFLEN];

extern CString IPv4Addr;
extern CString IPV4Port;

int OpenSocketSC3(const char *IPv4Addr);
int CloseSocketSC3(void);
CString SendSentinel3Message(CString message);
int SendMessageSC3_2(const char *msg);
int SendMessageOpenSocketSC3(const char *msg);
int IPv4ToolsRemLeadZ(char *IPAddr);

int SaveValidIPAddr();
