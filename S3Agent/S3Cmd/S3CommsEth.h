// S3Agent-specific comms functions

#pragma once



#define DEFAULT_BUFLEN		32768
#define MAX_IP_ADDR_LEN		256
#define RETURN_MESSAGE_LEN	1024

extern SOCKET ConnectSocket;
extern char	RxBuf[DEFAULT_BUFLEN];

extern char IPv4Addr[];
extern char IPV4Port[];

// -----------------------------------------------------------------------------