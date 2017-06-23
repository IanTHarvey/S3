// Wrapper for KuK I2C library

class CS3ControllerDlg;

#define S3I2C_EXPANDER_ADDR		0x44
#define S3I2C_CH_BATT_ADDR		0xAA

#define	S3I2C_TX_BATT_ADDR		0xAA
#define	S3I2C_TX_CTRL_ADDR		0xA8

#define S3_RF8_EXPDR_ADDR		0x44	// TCA6424
#define S3_RF1_EXPDR_ADDR		0x46	// TCA6424

#define	S3I2C_TX_CTRL_TEMP_ADDR	0x90
#define S3I2C_TX_CTRL_TEMP_T	0x00
#define S3I2C_TX_CTRL_TEMP_CFG	0x01

//	0xBE-C1:	Gain compensation temp and slope
#define S3I2C_TX_OPT_TCOMP_T	0xBE	// 2B Gain compensation temp
#define S3I2C_TX_OPT_TCOMP_M	0xC0	// 2B Gain compensation slope
#define S3I2C_TX_OPT_TCOMP_ON	0xCC	// bit2 gain switch

#define S3I2C_RX_OPT_ADDR		0xA4
#define S3I2C_RX_OPT_DSA		0xA4	// + A5

#define S3I2C_PW_ADDR			0x19	// Common for all boards

#define S3I2C_TX_OPT_FW_V		0x0F	// 3B
#define S3I2C_TX_OPT_ADDR		0xA4	// 
#define S3I2C_TX_OPT_DSA		0xA8	// + A9
#define S3I2C_TX_OPT_CAL_GAIN	0xAC	// + AD
#define S3I2C_TX_OPT_RF_MON		0xDA
#define S3I2C_TX_OPT_IDENT		0x00	// 66B
#define S3I2C_TX_OPT_CONFIG		0xA0
#define S3I2C_TX_OPT_THRESH		0x70	// 32B
#define S3I2C_TX_OPT_MON		0xD0	// 66B
#define S3I2C_TX_OPT_ALARM		0xFA	// 66B
#define S3I2C_TX_OPT_PEAK_H		0x88	// 2B
#define S3I2C_TX_OPT_PEAK_L		0x8A	// 2B
#define S3I2C_TX_OPT_RF_H		0x7C	// 2B
#define S3I2C_TX_OPT_RF_L		0x7E	// 2B
#define S3I2C_TX_OPT_BIAS_H		0x74	// 2B


#define S3I2C_TX_OPT_WLENGTH	0x45	// 2B nm
#define S3I2C_TX_OPT_TEMP		0xE2	// 2B Laser temp
#define S3I2C_TX_OPT_ALARMS		0xFA	// 3B
#define S3I2C_TX_OPT_CFG		0xCC	// 2B
#define S3I2C_TX_OPT_PEAK_THR	0xC2	// 2B

#define S3I2C_TX_OPT_PEAK_PWR	0xEC	// 2B
#define S3I2C_TX_OPT_PEAK_HOLD	0xEE	// 2B

#define S3I2C_TX_CTRL_ALARMS	0xDE	// 2B
#define S3I2C_TX_CTRL_TX_TYPE	0x43	// 1B
#define S3I2C_TX_CTRL_TCOMP		0xCC
#define S3I2C_TX_CTRL_TEST_TONE	0xCE	// Test-tone 0:off, 0x80:on
#define S3I2C_TX_CTRL_CAL_DATA	0xA8	// Per-path calibration data
#define S3I2C_TX_CTRL_TEMP		0xE0	// Ctrl board temperature (used for compensation)

#define S3I2C_TX_CTRL_FW_V		0x0F	// 3B
#define S3I2C_TX_CTRL_SN		0x32	// 9B
#define S3I2C_TX_CTRL_PN		0x1D	// 19B
#define S3I2C_TX_CTRL_IDENT		0x00	// 73B
#define S3I2C_TX_CTRL_THRESH	0x70	// 20B
#define S3I2C_TX_CTRL_CONFIG	0xA0	// 48B
#define S3I2C_TX_CTRL_MON		0xD0	// 14B
#define S3I2C_TX_CTRL_ALARM		0xDE	// 2B

#define S3I2C_SN_LEN			9		// I2C field len: 0-padded
#define S3I2C_PN_LEN			19		// I2C field len: 0-padded
#define S3I2C_FWDATE_LEN		7		// I2C field len: 0-padded

#define S3I2C_TX_RF_PATH		0xA5	// RF path selector
#define S3I2C_TX_RF_DSA			0xA6	// Attenuation
#define S3I2C_TX_RF_IP			0xA7	// Active RF input

#define S3_TX_CTRL_SLEEP		0xCF	// Addr
#define S3_TX_SLEEP_NONE		0x00
#define S3_TX_SLEEP_PART		0x40	// 0x40 Analog Rx & RF board off
#define S3_TX_SLEEP_FULL		0x80	// 0x80 Only receiver is on

#define S3I2C_RX_ADDR			0x90 // TODO: Don't know this yet

// Rx-opt registers
#define S3I2C_RX_OPT_IDENT		0x00
#define S3I2C_RX_OPT_ADDR_0		0xA4
#define S3I2C_RX_OPT_ADDR_1		0xA6
#define S3I2C_RX_OPT_THRESH		0x70	// 48B
#define S3I2C_RX_OPT_CONFIG		0xA0	// 48B
#define S3I2C_RX_OPT_CAL_GAIN	0xA8
#define S3I2C_RX_OPT_MON		0xD0	// -> E0
#define S3I2C_RX_OPT_RLL		0xD4
#define S3I2C_RX_OPT_RF_GAIN	0xD8
#define S3I2C_RX_OPT_VCC		0xD2
#define S3I2C_RX_OPT_RLL		0xD4
#define S3I2C_RX_OPT_TEMP		0xDE
#define S3I2C_RX_OPT_CFG_FLAGS	0xCC	// + CD
#define S3I2C_RX_OPT_ALARMS		0xFA	// 2B

// Transmitter serial lines
#define S3I2C_RX_OPT0_ADDR			0x90
#define S3I2C_RX_OPT1_ADDR			0x92
#define S3I2C_RX_OPT2_ADDR			0x94
#define S3I2C_RX_OPT3_ADDR			0x96
#define S3I2C_RX_OPT4_ADDR			0x98
#define S3I2C_RX_OPT5_ADDR			0x9A	

#define S3I2C_RX_CTRL_ADDR			0xA8
#define S3I2C_RX_CTRL_RX_TYPE		0x43	// 1B
#define S3I2C_RX_CTRL_IDENT			0x00	// 
#define S3I2C_RX_CTRL_SN			0x32	// 9B
#define S3I2C_RX_CTRL_PN			0x1D	// 19B
#define S3I2C_RX_CTRL_OPT_SW_POS	0xCE
#define S3I2C_RX_CTRL_THRESH		0x70	// 12B
#define S3I2C_RX_CTRL_ALARMS		0xF8	// 3B
#define S3I2C_RX_CTRL_CONFIG		0xA0
#define S3I2C_RX_CTRL_MON			0xD0	// 40B

// #define S3I2C_RX_RFCAL_ADDR		0xA8	// + A9 10mdBm

// Poll attempts
#define S3_POLL_SERIAL_TX	20
#define S3_POLL_SERIAL_RX	20

// I2C
#define STOP	0x50		// 'p'
#define START	0x53		// 's'

#define S3_SERIAL_FIFO_LEN	64

// Charger master-selects
#define MS_BAT_1	2
#define MS_BAT_2	1
#define MS_BAT_3	1
#define MS_BAT_4	2

// Charger enables (0-enabled)
#define EN_BAT_4	4
#define EN_BAT_3	8
#define EN_BAT_2	16
#define EN_BAT_1	32

// Control status bits
#define BQ_RES7		0x80
#define BQ_FAS		0x40
#define BQ_SS		0x20	// Sealed
#define BQ_CALEN	0x10
#define BQ_CCA		0x08
#define BQ_BCA		0x04
#define BQ_CSV		0x02
#define BQ_RES0		0x01

extern const char	S3I2CTxRFFact[7];
extern short		S3I2CRxOptFact[7];
// extern short		S3I2CTxOptFact[7];

extern unsigned char S3I2CCurRxOptAddr;

extern unsigned char	S3I2CTxReadBuf[S3_SERIAL_FIFO_LEN]; // Read from optical serial link
extern unsigned char	S3I2CRxReadBuf[];

unsigned short	S3RevByteUShort(	unsigned char *b);
short			S3RevByteShort(		unsigned char *b);

void			TrailSpaceTo0(char *str, char n);

int S3I2CTxSetStatus(	char Rx, char Tx);
int S3I2CSetUpOptAddr(	char Rx, char Tx);

int S3I2CPoll(CS3ControllerDlg *parent);
int S3I2CInit();
int S3I2CClose();
int S3I2CTest();
int S3I2CWriteByte();
int S3I2CReadByte();

int S3I2CTxSelfTest(	short *v1, short *v2, char Rx, char Tx);
int S3I2CTx8SelfTest(	short *v1, short *v2, char Rx, char Tx);

// I2C interface board IO expander functions
int S3I2CIOInit();
int S3I2CIORead(unsigned char pins[]);
int S3I2CIOWrite(unsigned char pins[]);

int S3I2CReadSerialData(	unsigned char		DevAddr,
							unsigned char		StartAddr,
							unsigned char		NBytes);

int S3I2CReadSerialByte(	unsigned char		DevAddr,
							unsigned char		DataAddr,
							unsigned char		*Data);

int S3I2CReadSerialShort(	unsigned char		DevAddr,
							unsigned char		DataAddr,
							short				*Data);

int S3I2CWriteSerialByte(	unsigned char		DevAddr,
							unsigned char		RegAddr,
							unsigned char		Data);

int S3I2CWriteSerialShort(	unsigned char		DevAddr,
							unsigned char		RegAddr,
							unsigned short		Data);

int S3I2CWriteSerialStr(	unsigned char		DevAddr,
							unsigned char		RegAddr,
							const char			*Str);

int S3I2CWriteSerialData(	unsigned char		DevAddr,
							unsigned char		RegAddr,
							const unsigned char	*Data,
							unsigned char		len);

void S3I2CClearFIFO();

short S3I2CRxGetRFLevel();

extern int S3GetCtrlTemps(double Ta[]);
extern int S3I2CSwitchTestTone(bool on);
extern int S3I2CTxSwitchInput(			char Rx, char Tx, char IP);

// ----------------------------------------------------------------------------
// bq34z100-G1

// Flags high 0x0E
#define OTC			0x80
#define OTD			0x40
#define	BATHI		0x20
#define	BATLOW		0x10
#define	CHG_INH		0x08
#define	XCHG		0x04
#define	FC			0x02
#define	CHG			0x01

// Flags low 0x0F
#define	OCVTAKEN	0x80
#define	CF			0x10
#define	SOC1		0x04
#define	SOCF		0x02
#define	DSG			0x01

// FlagsB 0x12
#define	SOH			0x80
#define	LIFE		0x40
#define	FIRSTDOD	0x20
#define	DODEOC		0x04
#define	DTRC		0x02

// ----------------------------------------------------------------------------

int S3I2CChMS(			unsigned char Ch);
int S3I2CChEn(			unsigned char Ch, bool enable);
int S3I2CChGetStatus(	unsigned char Ch);

int S3I2CChSetBattSealed(char Ch);
int S3I2CChSetBattUnseal();
int S3I2CChSetBattFullAccess();
int S3I2CChReadSecKeys();
int S3I2CChWriteSecKeys();
int S3I2CChWriteAuthKey();

int S3I2CChAuthenticate(char Ch);
int S3I2CTxAuthenticate(char Rx, char Tx);

int S3I2CRxMS(			unsigned char Rx);

bool	S3I2CGetPowerSwitch();

int S3I2CWriteLocalShort(	unsigned char	DevAddr,
							unsigned char	RegAddr,
							unsigned short	Data);

// Set up Tx
int	S3I2CSetPathA(		char p, char a);
int	S3I2CSetPath(		char p);
int	S3I2CSetRFAtten(	char p);
int	S3I2CSetTxOptDSA(	char Rx, char Tx, char IP, char dsa);
int	S3I2CSetRxOptDSA(	char p);

int S3I2CRxSetCalibration(		char Rx, char Tx, double	val);
int S3I2CTxSetRFCalibration(	char Rx, char Tx, char RFPath,	double	val);
int S3I2CTxSetOptCalibration(					double	val);

int S3I2CTxWriteRFCalValue(	char Rx, char Tx, unsigned char Path,	double cal);
int S3I2CTxWriteOptCalValue(										double cal);
int S3I2CRxWriteCalValue(	char Rx, char Tx,						double cal);

int S3I2CGetRxWavelength(char Rx);
int S3I2CGetTxWavelength(char Rx, char Tx);

int S3I2CRxSetActiveTx(		char Rx);
int S3I2CRxSwitchTx(		char Rx, char Tx);

int S3I2CGetTxTemp(			char Rx, char Tx);
int S3I2CGetTxBatt(			char Rx, char Tx);
int S3I2CTxGetLaserPow(		char Rx, char Tx);
int S3I2CTxGetOptAlarms(	char Rx, char Tx);
int S3I2CTxGetCtrlAlarms(	char Rx, char Tx);
int S3I2CTxGetPeakThresh(		char Rx, char Tx);

int S3I2CTxGetBattSN(		char Rx, char Tx);

int S3I2CTxGetOptCalGain(	char Rx, char Tx);
int S3I2CTxGetCalGain2(		char Rx, char Tx, char IP);
int S3I2CTxGetWavelength(	char Rx, char Tx);

char S3I2CTxCtrlGetTemp();	// Direct from TxCtrl TC74 (21462D.pdf)

int S3I2CTxSetCompMode(		char Rx, char Tx);
int S3I2CTxDoComp(			char Rx, char Tx);

int S3I2CTxUpdateTemp(		char Rx, char Tx);
int S3I2CTxUpdateTempPath(	char Rx, char Tx);

int S3I2CTxSetPeakThresh(	char Rx, char Tx, char path);

// int S3I2CTxPeakHoldLatchSet(	char Rx, char Tx);	// WTFWIT?
int S3I2CTxPeakHoldLatchClear(	char Rx, char Tx);

int S3I2CTxSetTestTone(		char Rx, char Tx, char IP);

extern unsigned char S3I2CTxOptAddr[S3_MAX_RXS];		

// Debug/profiling. Up to 8 timers (Tid). 
extern int	S3TimerStart(	unsigned char Tid);
extern int	S3TimerStop(	unsigned char Tid);

int S3I2CTxWriteID(const char * Type, const char *PN, const char *SN);
int S3I2CRxWriteID(const char * Type, const char *PN, const char *SN);

int S3I2CTempLog(char Rx, char Tx);

int S3I2CTxDumpOptConfig(		char Rx, char Tx);
int S3I2CTxDumpCtrlConfig(		char Rx, char Tx);
int S3I2CRxDumpOptConfig(		char Rx);
int S3I2CRxDumpCtrlConfig(		char Rx);

// ----------------------------------------------------------------------------
