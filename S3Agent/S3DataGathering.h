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
#define ETH_DHCP 4
#define USBPORT 5
#define USBDRVR 6
#define MSGSRC 7
#define RXDMSG 8

//Parameter order for S3GETSYSI reply
#define SYSNAME				0
#define SYSSN				1
#define SYSPN				2
#define SYSHW				3
#define SYSSW				4
#define IMAGEDATE			5
#define BUILDNO				6
#define MODELID				7
#define DATE				8
#define TIME				9

#define ACCESSMODE			10
#define CONFIGFILENAME		11
#define CONFIGFILEVER		12
#define CONFIGFILELOC		13
#define LOGFILENAME			14
#define LOGFILELOC			15
#define TESTNAME			16

#define APPDATETIME			17
#define S3TYPE				18
#define TCOMPGAINOPT		19
#define WINTRACKOPT			20
#define LOWNOISEOPT			21
#define SOFTSHUTDOWNOPT		22
#define SYSAGC				23
#define TXSSTARTSTATE		24
#define SELFTEST			25
#define SWVERSIOND			26
#define TERMINATOR			27

#define IMAGEID				28
#define IMAGEOS				29
#define IMAGETIME			30
#define OSUPDATEFAIL		31
#define POWERDOWNPENDING	32
#define POWERDOWNFAIL		33
#define SLEEPALL			34
#define SYSTEM_LOCKED		35

//Parameter order for S3GETINIT reply
#define GAIN 0
#define TEMPCOMPENSATION 1
#define TIMECONSTANT 2
#define INPUTIMPEDANCE 3
#define LOWNOISEMODE 4
#define UNITS 5
#define SCALE 6
#define SIZE 7
#define THREE_PC_LINEAR 8
#define MAXIP 9
#define WINDOWTRACKING 10

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
#define RXEXTRAGAINCAP 36

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
#define TXTEMP 21
#define TXTEMPCOMP 22
#define TXLASERPOW 23
#define TXLASERLO 24
#define TXLASERHI 25
#define TXCOMPMODE 26
#define TXALARMS 27
#define TXOPTALARMS 28
#define TXCTRLALARMS 29
#define TXBATTALARMS 30
#define TXRLLSTABLE 31
#define TXGAIN 32
#define TXMAXIPINHERIT 33
#define TXTAU 34
#define TXIMPEDANCE 35
#define TXLOWNOISE 36
#define TXWINDOWTRACK 37
#define TXFMAX 38
#define TXUPTIME 39
#define TXCALOPT 40
#define TXFWDATE 41
#define TXBATTCURRENT 42
#define TXUSERSLEEP 43
#define TXEMERGENCYSLEEP 44
#define TXCURALMSRC 45
#define TXCURALARM 46
#define TXTEMPTEC 47
#define TXPEAKPOWER 48
#define TXPEAKHOLD 49
#define TXTAUNS 50

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