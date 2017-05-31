// Header file to collect common definitions into one location
#pragma once
#define S3AGENT_APPNAME		"Sentinel 3 Remote Agent"	

#define DEFAULT_IP_ADDR	"127.0.0.1"
#define DEFAULT_PORT	"65000"

#define DEFAULT_BUFLEN		32768
#define COM_BUF_SIZE		256

#define S3_MAX_GPIB_RET_LEN	32768
#define S3_MAX_GPIB_CMD_LEN	256

#define MAX_SCRIPT_LEN		128
#include <shlobj.h>
extern TCHAR DataLocStr[MAX_PATH];

// Create and put in sentinel.h
#define MAX_IP_ADDR_LEN	256
#define MAX_PORT_LEN	256
#define MAX_FILNAME_LEN	256

#define STRNCMP	_strnicmp
#define STRCMP	_stricmp

//Connection Method codes
#define ETHERNET 0x0F
#define USB 0xF0
#define GPIB 0xFF

#define GPIB_NUM_DEVICES 31

//Connection Result codes
#define CONNECTMETHODFAIL 0xFE //No connection mode defined
#define INVALIDIPV4ADDR 0x0F //Invalid IPv4 address provided by the user
#define ETHMSGREJECT 0x05
#define ETHMSGFAILED 0x0E
#define USBINITFAIL 0xF0 //unable to initialise USB port
#define CONNFAIL 0xFF //Unable to open port
#define NOTS3 0xEE
#define CONNPASS 0x01 //Port open, but no valid Sentinel3 on the other end.
#define COMMPASS 0x00 //Communications pass - socket/usb open & Sentinel3 present 

//(General) Success/Failure codes
#define FAIL 0xFF
#define SUCCESS 0x00

#ifndef SENTINELDATABUNDLES
#define SENTINELDATABUNDLES

struct Sentinel3DataBundle
{
    bool isConnected;
    bool isRemoteAndManualAccess;
};

#endif




