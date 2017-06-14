#pragma once
#include "S3AgentDlg.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "defines.h"

extern Sentinel3DataBundle Sentinel3;

int Sentinel3AllDataGather(void);
int UpdateSentinelDefaultsDetails(void);
int UpdateSentinelSystemDetails(void);
int UpdateSentinelConnectionDetails(void);
int UpdateSentinelBatteryChargerDetails(void);
int UpdateSentinelRxModuleDetails(int Rx);
int UpdateSentinelTxModuleDetails(int Rx, int Tx);
int UpdateSentinelIpModuleDetails(int Rx, int Tx, int Ip);
int UpdateSentinelControllerDetails(void);
void DecodeDefaultsDetails(CString Response);
void DecodeSystemDetails(CString Response);
void DecodeConnectionDetails(CString Response);
void DecodeChargerDetails(CString Response);
void DecodeRxModuleDetails(CString Response, int Rx);
void DecodeTxModuleDetails(CString Response, int Rx, int Tx);
void DecodeIPModuleDetails(CString Response, int Rx, int Tx, int Ip);
UINT AutoUpdateSentinelDataBundleThread(LPVOID pParam);


//Parameter order for S3GETBATT reply
#define BATTVALIDITY 0
#define BATTCHARGE 1
#define TIMETOCHARGE 2
#define VOLTAGE 3
#define CURRENT 4
#define BATTYPE 5
#define BATTSN 6
#define BATTPN 7
#define BATTHWVER 8
#define BATTFWVER 9
#define BATTTEMP 10
#define BATTMFRDATA 11
#define BATTTYPE 12
#define BATTALARMS 13
#define BATTSTATUS 14

//Decode the Batteryvalidity response
#define BATTISVALID 1
#define BATTISINVALID 0
#define BATTISNOTPRESENT -1

//Parameter order for S3GETCONN reply
#define IPV4ADDR 0
#define IPV4MASK 1
#define IPV4PORT 2
#define EMACADDR 3
#define USBPORT 4
#define USBDRVR 5
#define MSGSRC 6
#define RXDMSG 7

//Parameter order for S3GETSYSI reply
#define SYSNAME 0
#define SYSSN 1
#define SYSPN 2
#define SYSHW 3
#define SYSSW 4
#define IMAGEDATE 5
#define BUILDNO 6
#define MODELID 7
#define DATE 8
#define TIME 9
#define ACCESSMODE 10
#define CONFIGFILENAME 11
#define CONFIGFILEVER 12
#define CONFIGFILELOC 13
#define LOGFILENAME 14
#define LOGFILELOC 15
#define TESTNAME 16
#define APPDATETIME 17
#define S3TYPE 18
#define TCOMPGAINOPT 19
#define WINTRACKOPT 20
#define LOWNOISEOPT 21
#define SOFTSHUTDOWNOPT 22
#define SYSAGC 23
#define TXSSTARTSTATE 24
#define SELFTEST 25
#define SWVERSIOND 26
#define IMAGEID 27
#define IMAGEOS 28
#define IMAGETIME 29
#define OSUPDATEFAIL 30
#define POWERDOWNPENDING 31
#define POWERDOWNFAIL 32
#define SLEEPALL 33
#define SYSTEM_LOCKED 34

//Parameter order for S3GETINIT reply
#define GAIN 0
#define TEMPCOMPENSATION 1
#define TIMECONSTANT 2
#define INPUTIMPEDANCE 3
#define LOWNOISEMODE 4
#define UNITS 5
#define MAXIP 6
#define WINDOWTRACKING 7

//Parameter order for the S3GETRXMOD reply
#define RXTYPE 0
#define RXDETECTED 1
#define RXNAME 2
#define RXID 3
#define RXSELTX 4
#define RXACTIVETX 5
#define RXSN 6
#define RXPN 7
#define RXFW 8
#define RXHW 9
#define RXMODELNAME 10
#define RXRLL 11
#define RXRFGAIN 12
#define RXLINKGAIN 13
#define RXCALGAIN 14
#define RXVCC 15
#define RXAGC 16
#define RXALARMS 17
#define RXTEMP 18
#define RXTEMPHI 19
#define RXTEMPLO 20
#define RXGAIN 21
#define RXMAXIPINHERIT 22
#define RXTAU 23
#define RXIMPEDANCE 24
#define RXLOWNOISE 25
#define RXWINDOWTRACK 26
#define RXCTRLALARMS 27
#define RXTXALARMS 28
#define RXCURALMSRC 29
#define RXCURALARM 30
#define RXRLLHI 31
#define RXRLLLO 32
#define RXFMAX 33
#define RXFWDATE 34
#define RXRFLEVEL 35

//Parameter order for the S3GETTXMOD reply
#define TXTYPE 0
#define TXDETECTED 1
#define TXNAME 2
#define TXID 3
#define TXPARENTID 4
#define TXWAVELENGTH 5
#define TXSN 6
#define TXPN 7
#define TXFW 8
#define TXHW 9
#define TXMODELNAME 10
#define TXBATTSN 11
#define TXBATTPN 12
#define TXBATTHW 13
#define TXBATTFW 14
#define TXBATTTEMP 15
#define TXBATTVALIDATED 16
#define TXBATTSOC 17
#define TXBATTATTE 18
#define TXPOWERSTATE 19
#define TXACTIVEINPUT 20
#define TXTESTSIGINPUT 21
#define TXTEMP 22
#define TXTEMPCOMP 23
#define TXLASERPOW 24
#define TXLASERLO 25
#define TXLASERHI 26
#define TXCOMPMODE 27
#define TXALARMS 28
#define TXOPTALARMS 29
#define TXCTRLALARMS 30
#define TXBATTALARMS 31
#define TXRLLSTABLE 32
#define TXGAIN 33
#define TXMAXIPINHERIT 34
#define TXTAU 35
#define TXIMPEDANCE 36
#define TXLOWNOISE 37
#define TXWINDOWTRACK 38
#define TXFMAX 39
#define TXUPTIME 40
#define TXCALOPT 41
#define TXFWDATE 42
#define TXBATTCURRENT 43
#define TXUSERSLEEP 44
#define TXEMERGENCYSLEEP 45
#define TXCURALMSRC 46
#define TXCURALARM 47
#define TXTEMPTEC 48
#define TXPEAKPOWER 49
#define TXPEAKHOLD 50
#define TXTAUNS 51

//Parameter order for the S3GETINPUT reply
#define IPNODENAME 0
#define IP1DBCOMP 1
#define IPMAXIP 2
#define IPALARMS 3
#define IPGAIN 4
#define IPMAXIPINHERIT 5
#define IPTAU 6
#define IPIMPEDANCE 7
#define IPLOWNOISE 8
#define IPWINDOWTRACK 9
#define IPRFLEVEL 10
#define IPRFGAIN 11
#define IPTESTTONE 12