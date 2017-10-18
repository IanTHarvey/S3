// $Id$
// ----------------------------------------------------------------------------
//
//	S3DataModel.h:	Main defines and structures for the Sentinel 3 controller.
//					8-byte strucure boundaries are assumed.
//
//	TODO:
//
// ----------------------------------------------------------------------------
//

#pragma once

#define REM_SHUTDOWNREQ (WM_USER + 0x200)
#define isS3GUIWatermarked false

#define ROUND(x)	(x<0?ceil((x) - 0.5):floor((x) + 0.5))
#define ABS(A)		((A)<0?-(A):A)

#ifdef S3_AGENT
class CS3AgentDlg;
// #include "S3AgentDlg.h"
#else
class CS3ControllerDlg;
// #include "S3ControllerDlg.h"
#endif


#ifdef TRIZEPS
#define	S3_ROOT_DIR					"\\FlashDisk\\S3"
#else
#define	S3_ROOT_DIR					"C:\\FlashDisk\\S3"
#endif

#define S3_LOG_FILE_SUFF			".s3l"
#define S3_CFG_FILE_SUFF			".s3c"
#define S3_SCR_FILE_SUFF			".s3s"

// ----------------------------------------------------------------------------

#ifdef _DEBUG
#define DBGLOG 0
#else
#define DBGLOG 0
#endif

// #define S3_TRACE_FILE

#ifdef S3_TRACE_FILE
#define debug_printw(fmt, ...) do { if (DBGLOG) fwprintf(S3DbgLog, fmt, __VA_ARGS__); } while(0)
#define debug_print(fmt, ...) do { if (DBGLOG) fprintf(S3DbgLog, fmt, __VA_ARGS__); } while(0)
// #define debug_printw	TRACE
#else
#define debug_print(A, ...)		TRACE(_T(A), __VA_ARGS__)
#define debug_printw(A, ...)	TRACE(_T(A), __VA_ARGS__)
#endif

extern FILE *S3DbgLog;

extern unsigned char S3Tx8IPMap[];

// TODO: Don't access directly
extern wchar_t *ScaleStrings[];
extern wchar_t *SigSizeStrings[];

// ----------------------------------------------------------------------------

#define S3_DEF_DEMO_CONFIG_FILENAME	"S3Demo"
#define S3_DEF_CONFIG_FILENAME		"S3Default"
#define S3_DEF_EVENT_LOG_FILENAME	"S3Default"

#define S3_LOCK_FILENAME			"S3Lock"
#define S3_UNLOCK_FILENAME			"S3Unlock"
#define S3_SN_FILENAME				"S3SN"
#define S3_PN_FILENAME				"S3PN"
#define S3_SCREEN_OFFSET_FILENAME	"S3ScreenOffset"
#define S3_OSDATE_FILENAME			"S3OSDate"

// All REVERSE byte order
#define S3_BATT_UNSEAL_KEY			"67D8FF9A"	// Unseal code
#define S3_BATT_FAS_KEY				"1DD34FF2"	// Full access
#define S3_BATT_SHA1_HMAC_KEY		"79C56EFA0D0535D8E7138A082FE5C527" // 128-bit

// #define	S3_FILE_VERSION		1.3 // double
// #define	S3_FILE_VERSION		1.4 // Save input tone enable 20-12-16
// #define	S3_FILE_VERSION		1.5 // Save Tx user sleep flag 17-01-17
// #define	S3_FILE_VERSION		1.6 // Save default Tx start state 17-01-17
// #define	S3_FILE_VERSION		1.7 // Save global AGC setting 17-01-17
// #define	S3_FILE_VERSION		1.8 // Save Rx active Tx 03-03-17
// #define	S3_FILE_VERSION		1.9 // Save Tx self test 18-04-17
// #define	S3_FILE_VERSION		2.0 // Save units scale and signal type 16-06-17
#define	S3_FILE_VERSION			2.1 // Save 3% linearity setting 16-06-17


// TODO: Move all 'common' configurables to this section
// ----------------------------------------------------------------------------
// Configurables

#define S3_RLL_STABLE_CNT		3	// No. of polls for which RLL mut be stable
#define S3_RLL_STABLE_THRESH	50	// Metric of RLL stability (x10mdBm = 0.5dBm)

// ----------------------------------------------------------------------------

// Some Unicode symbols
#define S3_SYM_DEGREE			0x00b0
#define S3_SYM_MU_LC			0x03bc
#define S3_SYM_OMEGA_UC			0x03a9

// 
#define S3_CONTROLLER_VERSION	0.0

#define S3_CMD_TERMINATOR		'\n' // Line feed/0xA/10

#define S3_PENDING_FG			0x80	// Use to flag/clear pending updates
#define S3_PENDING				0x64	// Use to flag/clear pending updates

// Message sources
#define S3_USB					1
#define S3_ETH					2

#include "S3DataModelDefs.h"

#define S3_MAX_BATT_SN_LEN		9
#define S3_MAX_BATT_PN_LEN		21

#define S3_MAX_EVENTS_LOG		1023	// End lines retained from previous log
#define S3_EVENTS_LINE_LEN		128		// Characters per line

#define S3_MAX_CFG_NAME_LEN		64
#define S3_MAX_FILENAME_LEN		128

#define S3_MAX_NODE_NAME_LEN	32
#define S3_MAX_EDIT_LEN			32

// Pop-up editors
#define S3_MAX_IP_ADDRESS_LEN	(4 * 3 + 3) + 1
#define S3_MAX_IP_PORT_LEN		5

#define S3_DEF_WIN_TRACK_OPTION	false
#define S3_DEF_OVERDRIVE_OPTION	false

#define S3_NULL_GAIN			SCHAR_MIN
#define S3_INVALID_GAIN			SCHAR_MIN // Indicate gain needs sending to Tx

// SC m_Type
typedef unsigned char			S3SCType;
#define S3_SCEmpty				0 // No real meaning?
#define S3_SC3					3
#define S3_SC6					6

// Rx m_Type
typedef unsigned char			S3RxType;
#define S3_RxEmpty				0
#define S3_Rx1					1
#define S3_Rx2					2
#define S3_Rx6					6

// Tx m_Type
typedef unsigned char			S3TxType;
#define S3_TxUnconnected		0
#define S3_Tx1					1
#define S3_Tx8					8

// Tx/Rx max frequency capability/compatibility
typedef unsigned char			S3Fmax;
#define S3_1GHZ					1
#define S3_3GHZ					3

#define S3_L_Unknown			0
#define	S3_1310nm				1
#define S3_1550nm				2

// Battery m_Type
typedef unsigned char			S3BattType;
#define S3_BattUnknown			0
#define S3_BattInvalid			1
#define S3_Batt2S1P				4
#define S3_Batt2S2P				5

typedef unsigned char			S3TxPwrMode;
#define	S3_TX_NOT_CONNECTED		0 // DONE: Just replicating m_Exist[] array?
#define S3_TX_ON				1
#define S3_TX_ON_PENDING		101
#define S3_TX_OFF				2
#define S3_TX_SLEEP				10
#define S3_TX_SLEEP_PENDING		110

#define S3_RLL_WARMUP_POLLS		15 // TODO: Define in terms of polling interval 

// Sys m_DisplayUnits
#define S3_UNITS_DBM			1
#define S3_UNITS_DBUV			2
#define S3_UNITS_MV				3	// Stored in Volts...

#define S3_UNITS_WATTS			1
#define S3_UNITS_VOLTS			2

//Sys m_SigSize
#define S3_UNITS_SMALL			1
#define S3_UNITS_LARGE			2

// Sys m_DisplayScale
#define S3_SCALE_LOG			1
#define S3_SCALE_LIN			2

#define S3_TCOMP_OFF			0
#define S3_TCOMP_CONT			1
#define S3_TCOMP_GAIN			2

#define S3_AGC_OFF				0
#define S3_AGC_CONT				1
#define S3_AGC_GAIN				2

#define S3_TXSTART_USER			0
#define S3_TXSTART_SLEEP		1
#define S3_TXSTART_ON			2

typedef enum InputZ				{W50, W1M, ZUnknown};	

// These are now run-time values, 0-4
typedef enum SigmaT				{TauNone, TauLo, TauMd, TauHi, TauUnknown};
#define S3_MAX_TAU_UNITS_LEN	16

// User-settable gain limits (inclusive)
#define	S3_MIN_GAIN				-78
#define	S3_MAX_GAIN				40
#define S3_DEFAULT_GAIN			0
#define S3_ATTEN_GAIN_OFFSET	6	// Available on some Txs where gain
									// is available on attenuation paths.

#define CONFIG(A, B)			(A->m_Config.B)

#define S3_MAX_IP_ADDR_LEN		64
#define S3_MAX_PORT_LEN			256
#define MAC_LEN					6
#define S3_MAX_MESSAGE_LEN		512
#define S3_DEFAULT_IP_ADDR		"10.0.0.102"
#define S3_DEFAULT_IP_PORT		65000

#define S3_IP_TEST_FAIL			0x01
#define S3_IP_OVERDRIVE			0x02

#define S3_ALARMS_ALL			0xFF
#define S3_TX_ALARMS_ALL		0xFFFF

#define S3_TX_OPT_ALARM_BYTES	3
#define S3_TX_CTRL_ALARM_BYTES	2

#define S3_RX_CTRL_ALARM_BYTES	3

// Tx alarm bits TODO: Move battery alarms to below
#define S3_TX_BATT_WARN			0x0001
#define S3_TX_BATT_ALARM		0x0002
#define S3_TX_COMM_FAIL			0x0004
#define S3_TX_TEMP_COMM_FAIL	0x0008
#define S3_TX_BATT_INVALID		0x0010
#define S3_TX_RECOMP_REQ		0x0020
#define S3_TX_LASER_HI			0x0040
#define S3_TX_LASER_LO			0x0080
#define S3_TX_SELF_TEST_FAIL	0x0100
#define S3_TX_SELF_TEST_NOT_RUN	0x0200
#define S3_TX_INIT_FAIL			0x0400
#define S3_TX_OVER_TEMP			0x0800
#define S3_TX_UNDER_TEMP		0x1000
#define S3_TX_RLL_UNSTABLE		0x2000
#define S3_TX_SELF_TEST_RETRY	0x4000
#define S3_TX_15				0x8000

// Tx battery alarms
#define S3_TX_BATT_COMM_FAIL	0x01
#define S3_TX_BATT_1			0x02
#define S3_TX_BATT_HOT			0x04
#define S3_TX_BATT_COLD			0x08
#define S3_TX_BATT_4			0x10
#define S3_TX_BATT_5			0x20
#define S3_TX_BATT_6			0x40
#define S3_TX_BATT_7			0x80

// Tx Opt alarms [0]
#define S3_TX_OPT_MAJOR			0x01
#define S3_TX_OPT_MINOR			0x02
#define S3_TX_OPT_LASER			0x04
#define S3_TX_OPT_AGC			0x08
#define S3_TX_OPT_TEC			0x10
#define S3_TX_OPT_0_5			0x20
#define S3_TX_OPT_0_6			0x40
#define S3_TX_OPT_0_7			0x80

// Tx Opt alarms [1]
#define S3_TX_OPT_VCC			0x01
#define S3_TX_OPT_BIAS			0x02
#define S3_TX_OPT_POWER			0x04
#define S3_TX_OPT_FEED_I		0x08 // Not used
#define S3_TX_OPT_PEAK			0x10
#define S3_TX_OPT_TEMP			0x20
#define S3_TX_OPT_LEVEL			0x40
#define S3_TX_OPT_GAIN			0x80

// Tx Opt alarms [2]
#define S3_TX_OPT_TEC_I			0x01
#define S3_TX_OPT_MODULE_TEMP	0x02
#define S3_TX_OPT_2_2			0x04
#define S3_TX_OPT_2_3			0x08
#define S3_TX_OPT_2_4			0x10
#define S3_TX_OPT_2_5			0x20
#define S3_TX_OPT_2_6			0x40
#define S3_TX_OPT_2_7			0x80

// Tx Ctrl alarms
#define S3_TX_CTRL_MAJOR		0x80
#define S3_TX_CTRL_MINOR		0x40

// Rx alarm bits
#define S3_RX_INT_FAIL			0x01	// Internal comms fail
#define S3_RX_OVER_TEMP			0x02
#define S3_RX_UNDER_TEMP		0x04
#define S3_RX_OVER_VOLT			0x08
#define S3_RX_UNDER_VOLT		0x10
#define S3_RX_INIT_FAIL			0x20
#define S3_RX_06				0x40
#define S3_RX_COMM_FAIL			0x80	// External (serial) comms fail

// Rx per input (Tx) alarm bits
#define S3_RX_RLL_LOW			0x01
#define S3_RX_RLL_HIGH			0x02
#define S3_RX_TX_02				0x04
#define S3_RX_TX_03				0x08
#define S3_RX_TX_04				0x10
#define S3_RX_TX_05				0x20
#define S3_RX_TX_06				0x40
#define S3_RX_TX_07				0x80

#define S3_RX_CTRL_FAN_FAIL		0x01
#define S3_RX_CTRL_01			0x02
#define S3_RX_CTRL_02			0x04
#define S3_RX_CTRL_03			0x08
#define S3_RX_CTRL_04			0x10
#define S3_RX_CTRL_05			0x20
#define S3_RX_CTRL_06			0x40
#define S3_RX_CTRL_07			0x80

// ----------------------------------------------------------------------------
// Parameter to be edited (see S3GDIScreenTx.cpp)
#define	S3_GAIN					80	// dB. Upper: ?; Lower: ?
#define	S3_MAX_INPUT			81	// dBm 
#define	S3_SIGMA_TAU			82	// uS
#define	S3_INPUT_IMP			83	// W
#define	S3_LOW_NOISE			84	//
#define	S3_WIN_TRACK			85	// 
#define	S3_PASSIVE_INT			86	// 
#define	S3_ALARM_LED			87	//
#define	S3_TXTX_NODENAME		88
#define	S3_TEST_TONE			89

// Input methods
#define S3_TXIP_NODENAME		90
#define S3_ACTIVE_INPUT			91

// See S3GDIScreenSettings.cpp
#define S3_IP_PORT				100
#define S3_USB_ENABLE			101
#define S3_IP_POWER_UNITS		102
#define S3_T_COMP_MODE			103
#define S3_LOG_COPY_USB			104
#define S3_ACCESS				105
#define S3_TX_START_STATE		106
#define S3_GLOBAL_AGC			107
#define S3_TX_SELF_TEST			108
#define S3_ID_USB_PORT			109
#define S3_DEF_GAIN				110
#define S3_DEF_IMP				111
#define S3_DEF_LOW_NOISE		112
#define S3_IP_ADDRESS			113
#define S3_IP_SUBNET			114

// See S3GDIScreenRx.cpp
#define S3_ACTIVE_TX			30
#define S3_RXTX_NODENAME		31
#define S3_RLL					32
#define S3_RX_CANCEL_ALARM		33

// See S3GDIScreenTx.cpp
#define S3_TX_POWER_MODE		73
#define S3_TX_DO_COMP			74
#define S3_TX_CANCEL_ALARM		75
#define S3_TX_TESTTONE_ALL		76

#define S3_RXRX_NODENAME		40
#define S3_RXRX_AGC				41

#define S3_OS_UPDATE			50
#define S3_APP_UPDATE			51

#define S3_DATE_EDIT			60
#define S3_TIME_EDIT			61

#define S3_IP_POWER_SCALE		70
#define S3_IP_SIG_SIZE			71
#define S3_3PC_LINEARITY		72

// ----------------------------------------------------------------------------

#define S3GDI_MAX_IP_PARAS		16
#define S3GDI_MAX_TX_PARAS		8

#define S3_MULT_CHOICE			0
#define S3_INT					1
#define S3_FP					2
#define S3_TEXT					3
#define S3_TOGGLE				4

//S3_GAIN, S3_INT, 0, 0, -55, 55
//S3_MAX_INPUT, S3_FP, 0, 0, -55 * 10.0, 55 * 10.0 
//S3_SIGMA_TAU, S3_MULT_CHOICE, 3, {0.1, 1.0, 10}, 0
//S3_INPUT_IMP, S3_TOGGLE, 2, {50, 1000000}, 0
//S3_LOW_NOISE, S3_TOGGLE, 0, 0, 0
//S3_WIN_TRACK, S3_TOGGLE, 0, 0, 0
//S3_PASSIVE_INT, S3_TOGGLE, 0, 0, 0


// Charger battery alarms bits (re-use Tx's?)
#define S3_CH_BATT_WARN			0x01
#define S3_CH_BATT_ALARM		0x02
#define S3_CH_BATT_HOT			0x04
#define S3_CH_BATT_COLD	   		0x08
#define S3_CH_BATT_INVALID		0x10
#define S3_CH_NO_CHARGE_VOLTAGE	0x20
#define S3_CH_CHARGE_FAULT		0x40

// TODO: Arbitrary percentages currently - need tying to battery
// icon colouring flashing etc
// SoC levels
#define S3_SOC_WARN				10
#define S3_SOC_ALARM			5
#define S3_SOC_MIN				3

// Ambient temperatures
#define S3_BATT_CHARGE_MIN_T	-100	// 0.1 degC
#define S3_BATT_CHARGE_MAX_T	400		// 0.1 degC

#define S3_BATT_DISCHG_MIN_T	-200	// 0.1 degC
#define S3_BATT_DISCHG_MAX_T	600		// 0.1 degC

// Shutdown temperatures
#define	S3_RX_UNDER_TEMP_LIM	-20	// degC
#define	S3_RX_OVER_TEMP_LIM		40	// degC

#define	S3_TX_UNDER_TEMP_LIM	-20	// degC
#define	S3_TX_OVER_TEMP_LIM		60	// degC

#define S3_COMP_TEMP_DIFF		5	// degC - Difference between current temp
									// and compensation temp at which some
									// form of notification raised.

#define S3_TEMP_STABLE_INTERVAL	3	// Number of poll cycles at stable temp
									// before compensation temperature is updated

#define	S3_RX_UNDER_VOLT_LIM	11000	// mV
#define	S3_RX_OVER_VOLT_LIM		13000	// mV

#define S3_INVALID_TEMP			SCHAR_MAX

#define S3_RLL_MAX_DBM			15.0	// Display upper
#define S3_RLL_MIN_DBM			6.0		// Display lower
#define S3_RLL_GOOD_HI_DBM		13.0	// Good upper
#define S3_RLL_GOOD_LO_DBM		7.0		// Good lower
#define S3_RLL_TX_ALIVE			800		// 10mdBm

#define S3_RLL_MAX_10MDBM		1500
#define S3_RLL_MIN_10MDBM		600
#define S3_RLL_GOOD_HI_10MDBM	1300
#define S3_RLL_GOOD_LO_10MDBM	800

#define S3_DATETIME_LEN			64

#define S3_RX_ZERO_RLL			-1000	// 10mdBm

typedef char S3Addr; // Rx, Tx, IP addressing

// TODO: Use for chargers and Txs
typedef struct sS3Battery
{
	unsigned char	m_Type;				// S2P1, S2P2
	char			m_Location;			// Charger: 0; Tx: 1.
	char			m_SN[S3_MAX_SN_LEN];
	char			m_PN[S3_MAX_PN_LEN];	
	char			m_HW[S3_MAX_SW_VER_LEN];
	char			m_FW[S3_MAX_SW_VER_LEN];
	unsigned char	m_SoC;	// %age SoC from bq34z100-G1
	char			m_Temp;	// DegC
} *pS3Battery, S3Battery;

// ----------------------------------------------------------------------------
// Inheritables (Actual input parameters)
typedef struct sS3Config
{
	int				m_Gain;					// dB. Upper: ?; Lower: ?
	double			m_MaxInput;				// dBm DO NOT USE
	SigmaT			m_Tau;					// uS
	InputZ			m_InputZ;				// W
	bool			m_LowNoiseMode;			// RF input mode
	bool			m_WindowTracking;		// RF input mode

} *pS3Config, S3Config;

// ----------------------------------------------------------------------------
// Data stored for each Tx input
typedef struct sS3IPData
{
	char			m_NodeName[S3_MAX_NODE_NAME_LEN];
		
	// Inheritables (Actual input parameters)
	S3Config		m_Config;

	short			m_RFLevel;
	short			m_RFGain;

	double			m_P1dB;		// 1dB compression point
	double			m_MaxInput;	// dBm

	InputZ			m_PrevZ;	// Reset to this previous value on disabling
								// passive integrator

	char			m_TestToneEnable;

	unsigned char	m_Alarms;
	char			m_Para;		// Used to mark selected parameter on GDI
								// GUI screens
} *pS3IPData, S3IPData;

// ----------------------------------------------------------------------------
// Data stored for each Tx head unit
typedef struct sS3TxData
{
	S3TxType		m_Type;		// enum Tx1, 2, or unknown
	S3Fmax			m_Fmax;	
	bool			m_Detected;
	char			m_NodeName[S3_MAX_NODE_NAME_LEN];
	char			m_Id, m_ParentId;
	unsigned char	m_Uptime;

	// Frig for RF1 hardware change - paths 4 & 5 swapped
	bool			m_OldTauOrder;
	bool			m_PeakHoldCap;
	bool			m_AttenGainCap;
	bool			m_PeakDetCap;
	
	char			m_AttenGainOffset; // S3_ATTEN_GAIN_OFFSET

	unsigned char	m_RLLStableCnt;

	// Inheritables (Tx defaults)
	S3Config		m_Config;

	short			m_CalOpt;	// 10mdB
	short			m_CalRF[7];	// 10mdB

	char			m_SN[S3_MAX_SN_LEN];				// PPM serial no.
	char			m_PN[S3_MAX_PN_LEN];
	char			m_FW[S3_MAX_SW_VER_LEN];
	char			m_FWDate[S3_MAX_FW_DATE_LEN];	// Ctrl board
	char			m_HW[S3_MAX_SW_VER_LEN];
	char			m_ModelName[S3_MAX_MODEL_ID_LEN];	// PPM model name

	unsigned char	m_Wavelength;	// S3_L_Unknown, S3_1310nm, S3_1550nm				2

	char			m_BattSN[S3_MAX_SN_LEN];
	char			m_BattPN[S3_MAX_PN_LEN];
	
	short			m_BattTemp;			// 0.1 DegC
	short			m_I;				// mA
	bool			m_BattValidated;	// PPM 'approved'
	char			m_BattHW[S3_MAX_SW_VER_LEN];
	char			m_BattFW[S3_MAX_SW_VER_LEN];

	unsigned char	m_SoC;				// State of Charge (%)
	unsigned short	m_ATTE;				// Average time to empty (min)

	S3TxPwrMode		m_PowerStat;		// On, Sleep
	bool			m_UserSleep;		// Explicitly sleeped
	bool			m_EmergencySleep;	// TODO: Combine with above?

	bool			m_SelfTestPending;
	unsigned char	m_SelfTestRetries;
	int				m_SelfTestErr;

	S3IPData		m_Input[S3_MAX_IPS];
	char			m_ActiveInput;		// Selected RF input
	char			m_TestSigInput;		// -1 for no test signal	

	char			m_CurAlarmSrc;		// -1:	No alarm;
										// 0:	S3Ctrl;
										// 1-3:	TxOpt;
										// 4-5:	TxCtrl;
	unsigned short	m_CurAlarm;			// Alarm currently displayed

	unsigned short	m_Alarms;			// S3Ctrl alarms
	unsigned char	m_OptAlarms[S3_TX_OPT_ALARM_BYTES];	// TxOpt I2C alarm registers
	unsigned char	m_CtrlAlarms[S3_TX_CTRL_ALARM_BYTES];	// TxCtrl I2C alarm registers
	unsigned char	m_BattAlarms;

	short			m_TempTEC;

	char			m_TempTx;			// Current Tx temperature (DegC). -128: Unknown
	char			m_TempComp;			// Temperature at last compensation (DegC)
	unsigned char	m_TempStableCnt;	// Count of temperature updates unchanged
	unsigned char	m_TempChange;		// Signal to update temperature
	char			m_TempReport;		// Reported 'stable' temperatute

	short			m_LaserPow, m_LaserLo, m_LaserHi;		// Laser power 10mdBm
	short			m_PeakThresh;		// 0.1mV
	short			m_PeakHold;			// Obsolete
	unsigned char	m_ClearPeakHold;

	unsigned char	m_CompMode;

	wchar_t			m_TauUnits[4][S3_MAX_TAU_UNITS_LEN];
	double			m_Tau_ns[4];
	
	// Links to GUI
	int				m_Xref, m_Yref;		// Used to find on main screen
	char			m_Para;				// Used to mark selected parameter
} *pS3TxData, S3TxData;

// ----------------------------------------------------------------------------
// Data stored for each inserted Rx module
typedef struct sS3RxData
{
	S3RxType		m_Type;				// S3_RxEmpty, S3_Rx1, 2, or 6
	S3Fmax			m_Fmax;
	bool			m_Detected;
	char			m_NodeName[S3_MAX_NODE_NAME_LEN];
	char			m_Id;

	// Inheritables (module defaults)
	S3Config		m_Config;

	// This determines the Rx6 Tx shown larger at the bottom - not the active or
	// selected one. TODO: RENAME! Kin confusing!
	char			m_SelectedTx;	// Only meaningful in UI sense... so should
									// it be here? This is only used to determine
									// the RX6 Tx that is *user-selected* (not
									// necessarily the active one).

	char			m_ActiveTx; // Only valid for Rx6s, 0 by default
	
	char			m_SN[S3_MAX_SN_LEN];
	char			m_PN[S3_MAX_PN_LEN];
	char			m_FW[S3_MAX_SW_VER_LEN];
	char			m_FWDate[S3_MAX_FW_DATE_LEN];
	char			m_HW[S3_MAX_SW_VER_LEN];
	char			m_ModelName[S3_MAX_MODEL_ID_LEN]; // Redundant?
	
	// These could logically be in the Tx struct, but 'physically' belong here
	short			m_RLL[S3_MAX_TXS];		// 10mdBm
	short			m_RFGain[S3_MAX_TXS];
	short			m_RFLevel[S3_MAX_TXS];
	char			m_LinkGain[S3_MAX_TXS];

	unsigned short	m_Vcc;			// mV
	unsigned char	m_AGC[2];		// True: on // OBSOLETE
	short			m_CalGain[2];	// 10mdB
	S3TxData		m_Tx[S3_MAX_TXS];

	// bool			m_TestSig; // If true will turn all Tx test sigs

	unsigned char	m_Alarms;
	unsigned char	m_RxAlarms[S3_RX_CTRL_ALARM_BYTES];
	unsigned char	m_TxAlarms[S3_MAX_TXS]; // Alarms generated on a per-Tx level

	char			m_CurAlarmSrc;	// -1:		No alarm
									// 0:		S3
									// > 100:	Tx - 100
									// 1-3:		RxOpt	NYI
									// 4-5:		RxCtrl	NYI
	unsigned char	m_CurAlarm;

	char			m_Temp;			// Current Rx temperature (DegC). -128: Unknown
	char			m_TempHi, m_TempLo;	// Read from Rx

	short			m_RLLHi, m_RLLLo;	// Read from Rx

	// Links with GUI
	int				m_Xref, m_Yref;	// Used to find on main screen
	char			m_Para;			// Used to mark selected parameter
} *pS3RxData, S3RxData;

// ----------------------------------------------------------------------------
// Per charging bay structure. See TI bq34z-100-G1 fuel gauge datasheet
typedef struct sS3Charger
{
	unsigned char	m_Type;

	bool			m_Detected; // Set only when physically detected - only
								// current use is to distinguish between real
								// and fake batteries in Demo Mode.
	bool			m_Occupied;

	unsigned char	stat_h, stat_l;

	char			m_MfrData[S3_MAX_SN_LEN];
	
	// Charge info; TODO: Much else is available
	bool			m_Charged;
	unsigned short	m_ATTF;		// Average time to full

	double			m_V;		// Charge/discharge (mV)
	short			m_I;		// Charge/discharge current (mA, +ve = charging)

	// Battery pack info TODO: Ditto
	char			m_BattType;
	char			m_BattSN[S3_MAX_SN_LEN];
	char			m_BattPN[S3_MAX_PN_LEN];	// PPM part number
	unsigned char	m_SoC;						// State of charge (%)
	short			m_BattTemp;					// 0.1 DegC
	bool			m_BattValidated;			// PPM approved

	// Model ID?
	char			m_HW[S3_MAX_SW_VER_LEN];
	char			m_FW[S3_MAX_SW_VER_LEN];
	
	unsigned char	m_Alarms;

	char			m_Para;		// Used to mark selected parameter
} *pS3Charger, S3Charger;

// ----------------------------------------------------------------------------
// Root data structure

typedef struct sS3DataModel
{
	S3SCType		m_Type;

	bool			m_DemoMode;
	bool			m_FactoryMode;

	bool			m_TCompGainOption;
	bool			m_WinTrackOption;	// Is window tracking offered?
	bool			m_LowNoiseOption;
	bool			m_SoftShutdownOption;

	char			m_NodeName[S3_MAX_NODE_NAME_LEN];	// User defined name
	char			m_AppDateTime[S3_DATETIME_LEN];

	// TODO: Distribute to IPs
	int				m_GainSent[S3_MAX_RXS][S3_MAX_TXS][S3_MAX_IPS];
	char			m_PathSent[S3_MAX_RXS][S3_MAX_TXS][S3_MAX_IPS];

	// Inheritables (System defaults)
	S3Config		m_Config;

	unsigned char	m_ContTComp;	// 0: No compensatiom
									// 1: Continuous gain temperature compensation
									// 2: Compensation only performed on gain
									// changes. User alerted if temperature drifts
									// +/-xC from temperature when gain set.

	unsigned char	m_AGC;			// 0: No RLL compensation
									// 1: Continuous RLL compensation
									// 2: Compensation only performed on gain
									// changes (including start-up). 

	unsigned char	m_TxStartState; // 0: User set
									// 1: Always sleep
									// 2: Always on

	short			m_PeakPulse; // 0.1mV

	char			m_ConfigName[S3_MAX_CFG_NAME_LEN]; // Load/save filename
	char			m_ConfigPath[S3_MAX_FILENAME_LEN]; // Load/save path

	char			m_EventLogName[S3_MAX_FILENAME_LEN];
	char			m_EventLogPath[S3_MAX_FILENAME_LEN];

	// System identity files - should probably use registry
	char			m_LockFileName[S3_MAX_FILENAME_LEN];	// Path & file
	char			m_UnlockFileName[S3_MAX_FILENAME_LEN];	// Path & file
	char			m_SNFileName[S3_MAX_FILENAME_LEN];		// Path & file
	char			m_PNFileName[S3_MAX_FILENAME_LEN];		// Path & file
	char			m_ScreenOffsetFileName[S3_MAX_FILENAME_LEN]; // Path & file

	bool			m_Locked;
	bool			m_TxSelfTest;
	
	short			m_ScrnOSx, m_ScrnOSy;

	char			m_SN[S3_MAX_SN_LEN];		// PPM S3 rack
	char			m_PN[S3_MAX_PN_LEN];
	char			m_SW[S3_MAX_SW_VER_LEN];
	char			m_HW[S3_MAX_SW_VER_LEN];
	char			m_ModelId[S3_MAX_MODEL_ID_LEN];		// PPM model id

	double			m_FileVersion;
	double			m_SWVersionD;

	char			m_ReportFileName[S3_MAX_FILENAME_LEN];
	char			m_TestName[S3_MAX_FILENAME_LEN]; // For display only

	// Ethernet
	char			m_IPv4Addr[S3_MAX_IP_ADDR_LEN];
	char			m_IPv4Subnet[S3_MAX_IP_ADDR_LEN];
	unsigned short	m_IPv6Addr[8];
	unsigned char	m_MACAddr[MAC_LEN];
	unsigned short	m_IPPort;
	bool			m_DHCPEnabled;

	bool			m_Remote;		// Exclusive local/remote config mode
	bool			m_Modified;		// Changes pending
	char			m_MsgSrc;		// S3_USB or S3_ETH
	bool			m_USBOpen;

	// Batteries on charge
	S3Charger		m_Chargers[S3_N_CHARGERS];

	// OS and image info
	char			m_ImageID[S3_MAX_OS_IMAGE_ID_LEN];
	char			m_ImageOS[S3_MAX_OS_IMAGE_ID_LEN];
	char			m_ImageDate[S3_MAX_OS_IMAGE_ID_LEN];
	char			m_ImageTime[S3_MAX_OS_IMAGE_ID_LEN];
	char			m_BuildNum[S3_MAX_BUILD_NUM_LEN];

	bool			m_OSUpdateFail;
	bool			m_OSAppUpdateFail;

	bool			m_PowerDownPending;
	bool			m_PowerDownFailed;

	// Test/dev only
	bool			m_CloseAppPending;
	bool			m_CloseAppFailed;

	unsigned char	m_DisplayUnits;	// System-wide-only setting
	unsigned char	m_DisplayScale;
	unsigned char	m_SigSize;
	bool			m_3PCLinearity;

	S3RxData		m_Rx[S3_MAX_RXS];

	// For tie-in to UI
	char			m_Para;
	char			m_SelectedTx, m_SelectedRx, m_SelectedIP;

	bool			m_SleepAll;
	bool			m_WakeAll;

    char         m_DisplayedUSBPort[S3_MAX_USB_PORT_LEN];
    char         m_DisplayedUSBDriver[S3_MAX_USB_DRIVER_LEN];
    char         m_PreviousRecievedMessage[S3_MAX_MESSAGE_LEN];
    char		 m_PrevMsgSrc;		// S3_USB or S3_ETH

	// SYSTEMTIME	m_SysTime;
	unsigned short	wHour;
	unsigned short	wMinute;
	unsigned short	wSecond;

	unsigned short	wYear;
	unsigned short	wMonth;
	unsigned short	wDay;

#ifndef S3_AGENT
	CS3ControllerDlg	*m_GUI;
#endif

} *pS3DataModel, S3DataModel;



// ----------------------------------------------------------------------------

#define	S3_LOG_OK				0
#define S3_LOG_FAIL_OPEN		1
#define S3_LOG_FAIL_OPEN_READ	2
#define S3_LOG_FAIL_OPEN_WRITE	4
#define S3_LOG_FAIL_OPEN_APPEND	8

typedef struct sS3Event
{
	char			t[32];
	char			msg[S3_EVENTS_LINE_LEN];
	char			severity;	// 1: minor, 3: major
	char			Rx, Tx, IP;
} *pS3Event, S3Event;

// ----------------------------------------------------------------------------

typedef struct sS3EventLog
{
	char			LogAlarm; // Has logging failed
	unsigned int	Count;
	unsigned int	NextInsert, Earliest;

	S3Event			Events[S3_MAX_EVENTS_LOG];

} *pS3EventLog, S3EventLog;

// ----------------------------------------------------------------------------
// Global function declarations

S3DataModel		*S3Copy(const S3DataModel *src);
int				S3Save(const char *Filename);
int				S3Read(const char *Filename);

int				S3Save2(const char *Filename);
int				S3Read2(const char *Filename);

int				S3UpdateConfigName(const char *Filename);

int				S3ParseSerialNumber(const char *sn);
int				S3ParseModelId(const char *id);

int				S3End(void);

int				S3BackupConfig();

bool			S3FileExist(const wchar_t *FilePath);
bool			S3DirExist(const wchar_t *FilePath);

// ----------------------------------------------------------------------------
// Structure accessors
extern "C" {
pS3DataModel	S3Init(bool DemoMode);
int				S3DataModelInit(pS3DataModel dm, bool DemoMode);

int				S3Reset();

int				S3SoftwareUpdate();

int				S3SetDateTime(	unsigned short Hour,
								unsigned short Minute,
								unsigned short Second,
								unsigned short Year,
								unsigned short Month,
								unsigned short Day);

int				S3SetPowerDownPending(bool set);
bool			S3GetPowerDownPending();

int				S3SetPowerDownFailed(bool set);
bool			S3GetPowerDownFailed();

// Test only
int				S3SetCloseAppPending(bool set);
bool			S3GetCloseAppPending();

int				S3SetCloseAppFailed(bool set);
bool			S3GetCloseAppFailed();

int				S3SetMACAddr(	const unsigned char *MAC);
int				S3SetIPAddrStr(		const wchar_t *addr, bool user);
int				S3SetIPSubnetStr(	const wchar_t *addr, bool user);
bool			S3ValidateIPAddress(const wchar_t *addr);

const char		*S3GetIPAddrStr();
const char		*S3GetIPSubnetStr();

unsigned short	S3GetIPPort();
int				S3SetIPPort(	unsigned short port);
int				S3GetMACAddrStr(char *addr);

int				S3SetSleepAll(bool sleep);
int				S3SetWakeAll(bool wake);
bool			S3GetSleepAll();
bool			S3GetWakeAll();
bool			S3AllAsleep(); // Query whether all Txs asleep
bool			S3AllAwake();

int				S3IPInit(	pS3IPData node);
int				S3TxInit(	pS3TxData node);
int				S3RxInit(	pS3RxData node);
int				S3ChInit(	unsigned char Ch);
int				S3SysInit(	pS3DataModel node);
int				S3ConfigInit(pS3Config config);

// TODO: ...and the rest?
int				S3TxCopy(pS3TxData node, pS3TxData src);

int				S3PushConfig(	unsigned char *CurrentNode);
int				S3SysPushConfig();
int				S3RxPushConfig(	char Node);
int				S3TxPushConfig(	char Rx, char Node);

int				S3CopyConfig(S3Config *dest, const S3Config *src);

// Return 0 if identical, 1 otherwise
int				S3CompConfig(const S3Config *dest, const S3Config *src);

#ifdef S3_AGENT
int				S3Poll(CS3AgentDlg *parent);
#else
int				S3Poll(CS3ControllerDlg *parent);
#endif

int				S3DbgPollInit();
// int				S3PollSys();
int				S3DbgPollRx(char Rx);
int				S3PollRxSetType(pS3RxData Rx);
int				S3PollTxSetType(pS3TxData Tx);

// For Demo Mode only
int				S3PollSetDummyData();

const char		*S3GetTestName();

pS3DataModel	S3GetSys();
pS3RxData		S3RxGetPtr(	char Rx);
pS3TxData		S3TxGetPtr(	char Rx, char Tx);
pS3IPData		S3IPGetPtr(	char Rx, char Tx, char IP);

int S3RxSetType(	pS3RxData Rx, S3RxType type);
int S3TxSetTypeP(	pS3TxData Tx, S3TxType type);

int S3TxSetType(	char Rx, char Tx, S3TxType type);

int S3SaveConfig(	FILE *fid, pS3Config Cfg);
int S3IPSave(		FILE *fid, pS3IPData IP);
int S3TxSave(		FILE *fid, pS3TxData Tx);
int S3RxSave(		FILE *fid, pS3RxData Rx);
int S3SysSave(		FILE *fid, pS3DataModel Sys);

int S3ReadConfig(	FILE *fid, pS3Config Cfg);
int S3IPRead(		FILE *fid, pS3IPData IP);
int S3TxRead(		FILE *fid, pS3TxData Tx);
int S3RxRead(		FILE *fid, pS3RxData Rx);
int S3SysRead(		FILE *fid, pS3DataModel Sys);

int S3SysSetNodeName(	const char *Node, char *NodeName);
int S3RxSetNodeName(	const char *Node, char *NodeName);
int S3TxSetNodeName(	const char *Node, char *NodeName);
int S3IPSetNodeName(	const char *Node, char *NodeName);

int S3SetNodeNameNew(	char Rx, char Tx, char IP, char *NodeName);
int S3IPSetParaTxt(		char Rx, char Tx, char IP, char Para, const wchar_t *Txt);

const char *S3GetSelNodeName();
const char *S3GetNodeName(char Rx, char Tx, char IP);

// TODO: rationalise?
int S3RxExistQ(	char Rx);
int S3RxValidQ(	char Rx);
int S3RxGetNTx(	char Rx);

int S3TxExistQ(	char Rx, char Tx);
int S3TxValidQ(	char Rx, char Tx);
int S3TxGetNIP(	char Rx, char Tx);

bool S3TxFOLLive(char Rx, char Tx);

int S3IPExistQ(		char Rx, char Tx, char IP);

int S3IPValidQ(		char Rx, char Tx, char IP);		// Deprecated
int S3IPInvalidQ(	char Rx, char Tx, char IP); // 
char S3RxGetTxN(	char Rx);

// TODO: Use pointer or index, not both
int S3TxSetPowerStat(			char Rx, char Tx, S3TxPwrMode s); // 1 power on, 0 power off
S3TxPwrMode S3TxGetPowerStat(	char Rx, char Tx); // 1 power on, 0 power off
int S3TxSetPowerStatP(pS3TxData Tx, S3TxPwrMode s);

bool S3TxGetPeakHoldCap(	char Rx, char Tx);
int S3TxSetPeakHoldCap(		char Rx, char Tx, bool peakholdcap);

// bool S3TxGetOverdriveOption(char Rx, char Tx);


// Indicate Tx shut down by system for some alarm condition,
// just over-temp for now
int S3TxSetEmergency(	char Rx, char Tx, bool on);
bool S3TxGetEmergency(	char Rx, char Tx);

int S3TxSetTCompMode(			char Rx, char Tx, unsigned char mode);
unsigned char S3TxGetTCompMode(	char Rx, char Tx);

int S3SetGain(			char Rx, char Tx, char IP, char		val);

// TODO: Deprecate:
int S3SetGainPush(		char Rx, char Tx, char IP, char		val);

// Generic gets
InputZ	S3GetImpedance(		char Rx, char Tx, char IP);
SigmaT	S3GetSigmaTau(		char Rx, char Tx, char IP);
bool	S3GetLowNoiseMode(	char Rx, char Tx, char IP);

int		S3TxSetTauUnits(	char Rx, char Tx, const unsigned char *units);
wchar_t *S3TxGetTauUnits(	char Rx, char Tx, char IP);
wchar_t *S3TxGetTauLabel(	char Rx, char Tx, SigmaT T);
int		S3TxGetTauUnitsA(char *str,
							char Rx, char Tx, char IP);
int S3TxSetPeakThresh(		char Rx, char Tx, short power);
short S3TxGetPeakThresh(	char Rx, char Tx);

int S3TxSetPeakPulse(		short pulse);
short S3TxGetPeakPulse();

char S3TxGetAttenGainOffset(char Rx, char Tx);

// TODO: Obsolete
int S3TxSetPeakHold(		char Rx, char Tx, short hold);
short S3TxGetPeakHold(		char Rx, char Tx);

int S3TxSetStableCnt(			char Rx, char Tx, unsigned char StableCnt);
unsigned char S3TxGetStableCnt(	char Rx, char Tx);
bool S3TxRLLStable(				char Rx, char Tx);

unsigned char S3TxGetClearPeakHold(	char Rx, char Tx);
int S3TxClearPeakHold(				char Rx, char Tx, unsigned char ack);

// Generic sets
int S3SetImpedance(		char Rx, char Tx, char IP, InputZ	z);
int S3SetSigmaTau(		char Rx, char Tx, char IP, SigmaT	t);
int S3SetLowNoiseMode(	char Rx, char Tx, char IP, bool		on);


int S3IPSetImpedance(	char Rx, char Tx, char IP, InputZ			val);
int S3IPCal(			char Rx, char Tx, char IP, int				On);
int S3CalSignal(		char Rx, char Tx, char IP, unsigned char	On);
int S3IPSetLowNoiseMode(char Rx, char Tx, char IP, bool				On);
int S3IPSetSigmaTau(	char Rx, char Tx, char IP, SigmaT			Tau);
int S3IPSetSigmaTauS(	char Rx, char Tx, char IP, const char		*Tau);
int S3IPWindowTrack(	char Rx, char Tx, char IP, unsigned char	On);

int S3IPSetRFGain(		char Rx, char Tx, char IP, short			gain);
int S3IPSetRFLevel(		char Rx, char Tx, char IP, short			level);
short S3IPGetRFGain(	char Rx, char Tx, char IP);
short S3IPGetRFLevel(	char Rx, char Tx, char IP);

int S3IPSetGain(pS3IPData pIP, int gain);
int S3IPGetGain(		char Rx, char Tx, char IP);

#define S3_PARA_NAME_LEN 12 // TODO:
int S3IPGetSigmaTauS(	char *str, char Rx, char Tx, char IP);
int S3IPGetInputZS(		char *str, char Rx, char Tx, char IP);

// Store transmitted gains and paths to detect user changes
char	S3IPGetGainSent(char Rx, char Tx, char IP);
int		S3IPSetGainSent(char Rx, char Tx, char IP, char Gain);

char	S3IPGetPathSent(char Rx, char Tx, char IP);
int		S3IPSetPathSent(char Rx, char Tx, char IP, char Path);

int S3IPSetMaxInput(				char Rx, char Tx, char IP, double maxip);

double			S3IPGetP1dB(		char Rx, char Tx, char IP);

double			S3IPGetMaxInput(	char Rx, char Tx, char IP);
InputZ			S3IPGetImpedance(	char Rx, char Tx, char IP);
unsigned char	S3IPGetLowNoiseMode(char Rx, char Tx, char IP);
SigmaT			S3IPGetSigmaTau(	char Rx, char Tx, char IP);
unsigned char	S3IPGetWindowTrack(	char Rx, char Tx, char IP);

InputZ			S3IPGetPrevZ(		char Rx, char Tx, char IP);
int				S3IPSetPrevZ(		char Rx, char Tx, char IP, InputZ z);

int		S3IPSetTestToneEnable(		char Rx, char Tx, char IP, char Enable);
int		S3TxSetTestToneEnableAll(	char Rx, char Tx, char Enable);
char	S3IPGetTestToneEnable(		char Rx, char Tx, char IP);

double	S3CalcP1dB(int gain);
double	S3CalcMaxIP(int gain);
int		S3IPCalcGain(double maxip);
int S3GetGainLimits(char Rx, char Tx, char IP, char *low, char *high);

int				S3SetUnits(unsigned char Units);
unsigned char	S3GetUnits();
wchar_t			*S3GetUnitString();
wchar_t			*S3GetUnitStrings(unsigned char i);

wchar_t			*S3GetScaleString();
wchar_t			*S3GetSigSizeString();

int				S3SetScale(unsigned char Scale);
int				S3SetSigSize(unsigned char Size);

unsigned char	S3GetScale();
unsigned char	S3GetSigSize();

bool			S3Get3PCLinearity();
int				S3Set3PCLinearity(bool show3PC);

int S3TxSetActiveIP(	char Rx, char Tx, char IP);
char S3TxGetActiveIP(	char Rx, char Tx);
int S3RxSetActiveTx(	char Rx, char Tx);
char S3RxGetActiveTx(	char Rx);
bool S3RxIsActiveTx(	char Rx, char Tx);
bool S3TxConnected(		char Rx, char Tx);

char S3TxGetBattValidated(	char Rx, char Tx);
int S3TxSetBattValidated(	char Rx, char Tx, bool valid);

bool S3TxSelfTestPending(char Rx, char Tx);
int S3TxSetSelfTestPending(char Rx, char Tx, bool pending);

int S3AllReport(		char *Buf);
int S3SysReport(		char *Buf);
int S3TopologyReport(	char *Buf);
int S3RxReport(			char *Buf, char Rx);
int S3TxReport(			char *Buf, char Rx, char Tx);
int S3IPReport(			char *Buf, char Rx, char Tx, char IP);
int S3CfgReport(		char *Buf, char Rx, char Tx, char IP);

int S3SetSelectedPara(	char Rx, char Tx, char IP, char Para);
char S3GetSelectedPara(	char Rx, char Tx, char IP);
int S3SetParaValue(		char Rx, char Tx, char IP, char Para, char MenuItem);

int S3ReportToFile(char *Buf);

int S3TestToneAll(	unsigned char On);
int S3TxPowerAll(	unsigned char On);

int S3GetSelPathStr(	char **str);
int S3GetPathStr(		char Rx, char Tx, char IP, char **str);
char *S3GetPathStrStr(	unsigned char *cData);

const wchar_t *S3GetTypeStr(char Rx, char Tx);
char S3GetType(char Rx, char Tx);

const char *S3GetConfigName();
const char *S3GetEventLogName();

int S3SetRemote(bool remote);
bool S3GetRemote();

void S3SetUSBOpen(bool open);
bool S3GetUSBOpen();

// Better to use IP = -1 to turn off
int		S3TxSetTestToneIP(	char Rx, char Tx, char IP); //, unsigned char SigOn);
char	S3TxGetTestToneIP(	char Rx, char Tx);

int S3CancelAlarms();

// ----------------------------------------------------------------------------
// Event logging fns
int			S3EventLogInit(		const char *filename);
int			S3SetEventLogName(	const char *filename);

void		S3GetTimeStr(char *str);

int			S3EventLogAdd(const char *msg, char severity, char Rx, char Tx, char IP);
int			S3EventAddNotifyFn(void (*fn)(void));
int			S3EventOutstanding(void);
int			S3EventLogClose(void);
pS3EventLog	S3EventGetLog(void);
pS3Event	S3EventGetEvent(unsigned int i);
pS3Event	S3EventGetLastEvent(void);
int			S3EventLogSysInfo(void);

// ----------------------------------------------------------------------------
// Alarm functions

int S3TxSetAlarm(				char Rx, char Tx, unsigned short alarms);
int S3TxCancelAlarm(			char Rx, char Tx, unsigned short alarms);
unsigned short S3TxGetAlarms(	char Rx, char Tx);
int S3TxAlarmGetString(			char Rx, char Tx, char *S3AlarmString, int len);
int S3TxCancelCurAlarm(			char Rx, char Tx);

int S3TxBattSetAlarm(			char Rx, char Tx, unsigned char alarms);
int S3TxBattCancelAlarm(		char Rx, char Tx, unsigned char alarms);
unsigned char S3TxBattGetAlarms(char Rx, char Tx);

int S3TxOptSetAlarm(			char Rx, char Tx, const unsigned char *alarms);
int S3TxCtrlSetAlarm(			char Rx, char Tx, const unsigned char *alarms);
char S3TxGetAnyAlarm(			char Rx, char Tx); // Non-specific alarm state

// Set Tx = -1 if not input-specific
int S3RxSetAlarm(				char Rx, char Tx, unsigned char alarms);
int S3RxCancelAlarm(			char Rx, char Tx, unsigned char alarms);
int S3RxAlarmGetString(			char Rx, char *S3AlarmString, int len);
int S3RxCancelCurAlarm(			char Rx);
int S3RxCtrlSetAlarm(			char Rx, const unsigned char *alarms);
unsigned char S3RxGetAlarms(char Rx);
const unsigned char *S3RxCtrlGetAlarms(char Rx);

int S3IPSetAlarm(				char Rx, char Tx, char IP, unsigned char alarms);
int S3IPCancelAlarm(			char Rx, char Tx, char IP, unsigned char alarms);
unsigned char S3IPGetAlarms(	char Rx, char Tx, char IP);


int S3ChSetAlarm(				char Ch, unsigned char alarms);
int S3ChCancelAlarm(			char Ch, unsigned char alarms);
unsigned char S3ChGetAlarms(	char Ch);

// ----------------------------------------------------------------------------

int S3TxFindSN(			char *Rx, char *Tx, char *SN);

int S3SetDummyTxSN();

// Hardware notifications
int S3RxInserted(		char Rx, unsigned char type);
int S3RxRemoved(		char Rx);

char	S3RxGetTemp(	char Rx);
int 	S3RxSetTemp(	char Rx, char t);

char	S3TxGetTemp(	char Rx, char Tx);
int		S3TxSetTemp(	char Rx, char Tx, char t);

int S3TxSetTempTEC(		char Rx, char Tx, short t);
short S3TxGetTempTEC(	char Rx, char Tx);

char	S3TxGetTempComp(char Rx, char Tx);
int		S3TxSetTempComp(char Rx, char Tx, char t);
int		S3TxDoComp(		char Rx, char Tx);

int		S3TxSetUserSleep(char Rx, char Tx, bool user);
bool	S3TxGetUserSleep(char Rx, char Tx);

// Get IP from Rx
int S3TxInserted(	char Rx, char Tx, unsigned char type);
int S3TxRemoved(	char Rx, char Tx);

int S3SysSetPN(		const char *s);
int S3SysSetSN(		const char *s);

int S3SetAppDateTime();

const char *S3SysGetPN();
const char *S3SysGetSN();
const char *S3SysGetHW();
const char *S3SysGetSW();
const char *S3SysGetImageDate();
const char *S3SysGetImageTime();
const char *S3SysGetAppDateTime();

int			S3SysSetPN(	const char *s);
int			S3SysSetSN(	const char *s);
int			S3SysSetHW(	const char *s);
int			S3SysSetSW(	const char *s);

const char	*S3SysGetModel();
int			S3SysSetModel(const char *s);

int			S3RxSetPN(		char Rx, const char *s);
int			S3RxSetSN(		char Rx, const char *s);
int			S3RxSetHW(		char Rx, const char *s);
int			S3RxSetFW(		char Rx, const char *s);
int			S3RxSetFWDate(	char Rx, const char *s);

int			S3RxSetFmax(char Rx, const char *s);
S3Fmax		S3RxGetFmax(char Rx);

int			S3RxSetDetected(char Rx, bool detected);
bool		S3RxGetDetected(char Rx);

const char *S3RxGetPN(		char Rx);
const char *S3RxGetSN(		char Rx);
const char *S3RxGetHW(		char Rx);
const char *S3RxGetFW(		char Rx);
const char *S3RxGetFWDate(	char Rx);

int			S3TxSetPN(		char Rx, char Tx, const char *s);
int			S3TxSetSN(		char Rx, char Tx, const char *s);
int			S3TxSetHW(		char Rx, char Tx, const char *s);
int			S3TxSetFW(		char Rx, char Tx, const char *s);
int			S3TxSetFWDate(	char Rx, char Tx, const char *s); // TxCtrl

int			S3TxSetFmax(char Rx, char Tx, const char *s);
S3Fmax		S3TxGetFmax(char Rx, char Tx);

bool		S3TxIsDetected(char Rx, char Tx);

const char *S3TxGetPN(		char Rx, char Tx);
const char *S3TxGetSN(		char Rx, char Tx);
const char *S3TxGetHW(		char Rx, char Tx);
const char *S3TxGetFW(		char Rx, char Tx);
const char *S3TxGetFWDate(	char Rx, char Tx); // TxCtrl

int S3TxOptSetFW(			char Rx, char Tx, const unsigned char *s); // a.b.c

int S3TxSetUnconnected(	pS3TxData Tx);
int S3TxSetConnected(	pS3TxData Tx);

int S3TxSetLaserPow(	char Rx, char Tx, short p);
short S3TxGetLaserPow(	char Rx, char Tx);

int				S3TxGetInfo(	char Rx, char Tx,
								const char **SN, const char **PN,
								const char **HWV, const char **FWV);

int				S3RxGetInfo(	char Rx,
								const char **SN, const char **PN,
								const char **HWV, const char **FWV);

unsigned char	S3TxGetType(	char Rx, char Tx);
unsigned char	S3RxGetType(	char Rx);

short			S3RxGetRLL(		char Rx, char Tx);
int				S3RxSetRLL(		char Rx, char Tx, short RLL);

short			S3RxGetRLLLo(	char Rx);
short			S3RxGetRLLHi(	char Rx);

unsigned char	S3RxGetAGC(		char Rx, char Tx);
int				S3RxSetAGC(		char Rx, char Tx, unsigned char on);

short			S3RxGetRFGain(	char Rx, char Tx);
int				S3RxSetRFGain(	char Rx, char Tx, short gain);

short			S3RxGetRFLevel(	char Rx, char Tx);
int				S3RxSetRFLevel(	char Rx, char Tx, short level);

unsigned short	S3RxGetVcc(		char Rx);
int				S3RxSetVcc(		char Rx, unsigned short vcc);

char			S3RxGetHighlightedTx(	char Rx);
void			S3RxSetHighlightedTx(	char Rx, char Tx);

char			S3GetSelected(	char *Rx, char *Tx, char *IP);
int 			S3SetSelected(	char Rx, char Tx, char IP);
char			S3IsSelected(	char Rx, char Tx, char IP);

char			S3GetSelectedRx(void);	// Only if THE selected node
char			S3GetCurrentRx(void);	// On the selection path

// int S3SetOverdrive(char Rx, char Tx, char IP);
unsigned char S3RxGetConnectedTxs(	char Rx); 
unsigned char S3RxGetConnectedTx(	char Rx, char Tx); // Rx2 only

void S3RxSetCoords(char Rx, int x, int y);
void S3RxGetCoords(char Rx, int *x, int *y);

void S3TxSetCoords(char Rx, char Tx, int x, int y);
void S3TxGetCoords(char Rx, char Tx, int *x, int *y);

// ----------------------------------------------------------------------------
// Charger parameters
bool			S3ChOccupied(		char Ch);
bool			S3ChFullyCharged(	char Ch);
char			S3ChGetSoC(			char Ch);
char			S3ChSetSoC(			char Ch, char ChLevel);
const wchar_t	*S3ChGetBattTypeStr(char Ch);
unsigned char	S3ChGetBattType(	char Ch);
int				S3ChSetBattType(	char Ch, unsigned char Type);
const char		*S3ChGetBattSN(		char Ch);
const char		*S3ChGetBattPN(		char Ch);
int				S3ChSetBattSN(		char Ch, const char *SN);
int				S3ChSetBattPN(		char Ch, const char *PN);
int				S3ChSetTimeToFull(	char Ch, unsigned short tmin);
unsigned short	S3ChGetTimeToFull(	char Ch);
const char		*S3ChGetTimeToFullStr(	char Ch);

int				S3ChInsert(				char Ch, char *SN, char *PN);
int				S3ChRemove(				char Ch);
bool			S3ChBattValidated(		char Ch);
int				S3ChSetBattValidated(	char Ch, bool valid);
bool			S3ChBattValidate(		char Ch);	// Placeholder

// ----------------------------------------------------------------------------
// Chargers collectively
unsigned char	S3ChGetNOnCharge();
int				S3ChInitAll();
												
// Battery-on-charge
const char		*S3ChGetBattHW(		char Ch);
int				S3ChSetBattHW(		char Ch, const char *Ver);
int				S3ChSetBattStatus(	char Ch, const unsigned char *stat);
unsigned char	S3ChGetBattStatus(	char Ch);
const char		*S3ChGetBattFW(		char Ch);
int				S3ChSetBattFW(		char Ch, const char *Ver);
int				S3ChSetBattTemp(	char Ch, short t);
short			S3ChGetBattTemp(	char Ch);

int				S3ChSetBattV(	char Ch, double v);
double			S3ChGetBattV(	char Ch);

int				S3ChSetBattI(	char Ch, short i);
short			S3ChGetBattI(	char Ch);

const char		*S3ChGetBattMfr(char Ch);
int				S3ChSetBattMfr(	char Ch, const char *MfrData);

// ----------------------------------------------------------------------------
// Tx battery
int				S3TxSetBattInfo(char Rx, char Tx,
								const char *BattSN,	const char *BattPN,
								const char *BattHW,	const char *BattFW);

int				S3TxGetBattInfo(char Rx, char Tx,
								const char **BattSN, const char **BattPN,
								const char **BattHW, const char **BattFW);

unsigned char	S3TxGetBattSoC(		char Rx, char Tx);
int				S3TxSetBattSoC(		char Rx, char Tx, unsigned char charge);
int				S3TxSetBattTemp(	char Rx, char Tx, short t);
short			S3TxGetBattTemp(	char Rx, char Tx);

int				S3TxSetBattI(		char Rx, char Tx, short i);
short			S3TxGetBattI(		char Rx, char Tx);

int				S3TxSetATTE(		char Rx, char Tx, unsigned short atte);
unsigned short	S3TxGetATTE(		char Rx, char Tx);

char			S3TxGetBattValidated(	char Rx, char Tx);


bool			S3BattValidate(	const char *Ch);	// Placeholder

int				S3RxSetLinkGain(	char Rx, char Tx, char Gain);
char			S3RxGetLinkGain(	char Rx, char Tx);

int				S3ExtractSN(		int *iSN, const char *SN);

// Comparitors
int S3SystemIdentical(pS3DataModel last, pS3DataModel current);
int S3SystemCompatible(pS3DataModel last, pS3DataModel current);

int S3SysCmp(	pS3DataModel	s1, pS3DataModel	s2);
int S3RxCmp(	pS3RxData		r1, pS3RxData		r2);
int S3TxCmp(	pS3TxData		t1, pS3TxData		t2);

// Utilities
int				S3GetPathFile(char *Path, char *File, const char *Filename);

int S3RxGetInfoStr(		char *info, char Rx); 
int S3TxGetInfoStr(		char *info, char Rx, char Tx);
int S3IPGetInfoStr(		char *info, char Rx, char Tx, char IP); 

int S3ConfigGetInfoStr(	char *tmp, char Rx, char Tx, char IP);

int S3I2CChGetStatus(	unsigned char Ch);
int S3I2CRxGetStatus(	char Rx);
int S3I2CTxGetStatus(	char Rx, char Tx);
int S3I2CRxProcessTx(	char Rx, char Tx);
int S3I2CRxSetAGC(		char Rx, char Tx);

// Read/write battery serial and part numbers
int S3I2CChReadSNPN(	char Ch, char		*SN, char		*PN);
int S3I2CChWriteSNPN(	char Ch, const char	*SN, const char	*PN);

// ----------------------------------------------------------------------------
// Global temperature compensation scheme 
int				S3SetTCompMode(unsigned char ContMode);
unsigned char	S3GetTCompMode();
int				S3TempComp();
int				S3DoComp(char Rx, char Tx);

// Global AGC setting
unsigned char	S3GetAGC();
int				S3SetAGC(unsigned char AGCOn);

// ----------------------------------------------------------------------------
// Disable factory set-up features for deliverable system.

int		S3GetLockFile();
int		S3SetLockFile();
bool	S3GetLocked();
int		S3SetLocked(bool lock);

int		S3SetFactoryMode(char Rx, char Tx, bool mode);
bool	S3GetFactoryMode();

int		S3GetScreenOffsets(		short *x,	short *y);
int		S3WriteScreenOffsets(	short x,	short y);
int		S3ReadScreenOffsets();

bool	S3GetDemoMode();
int		S3SetDemoMode(bool DemoMode);

// ----------------------------------------------------------------------------
// OS Interface

int		S3OSInit();
int		S3OSImageUpdate();
int		S3OSGetImageID();

int		S3OSAppUpdate();
bool	S3OSGetAppUpdateFail();
void	S3OSSetAppUpdateFail(bool fail);

bool	S3OSUpdateFail();
void	S3OSSetUpdateFail(bool fail);

bool	S3OSRestartRequest();
int		S3OSRestart();
int		S3OSTest();

int		S3OSSWUpdateRequest();
int		S3OSAppUpdateRequest();

int		S3LogFileToUSBRequest();
int		S3LogFileToUSB();

int		S3SysReadSN();
int		S3SysWriteSN();

int		S3SysReadPN();
int		S3SysWritePN();

int		S3WriteEthConfig();
int		S3ReadEthConfig();

int			S3SysSetBuildNum(const char *bn);
const char	*S3SysGetBuildNum();

int S3SetStaticIP(const char *IPAddress,
				  const char *SMAddress,
				  const char *GWAddress);

bool S3GetDHCP();
int S3SetDHCP(bool en);

int S3SetSIPRegKey(DWORD data);

// ----------------------------------------------------------------------------
// From I2C interfaces
int	S3TxSetLaserLim(	char Rx, char Tx, short hi, short lo);

int	S3RxSetTempLimits(	char Rx, char hi, char lo);
int	S3RxSetRLLLimits(	char Rx, short hi, short lo);
int S3RxSetTemp(		char Rx, char t);

// int S3RxSetCalGain(		char Rx, short cal);
int S3RxSetCalGain(		char Rx, char Tx, short cal); // Tx for Rx2 only
int S3TxSetCalOpt(		char Rx, char Tx, short cal);
int S3TxSetCalOpt2(		char Rx, char Tx, char IP, short cal);

short	S3TxGetCalRF(	char Rx, char Tx, unsigned char Path);
int		S3TxSetCalRF(	char Rx, char Tx, unsigned char Path, short cal);

short S3RxGetCalGain(	char Rx, char Tx);
short S3TxGetCalOpt(	char Rx, char Tx);
short S3TxGetCalOpt2(	char Rx, char Tx, char IP);

int				S3TxSetWavelength(	char Rx, char Tx, unsigned char w);
unsigned char	S3TxGetWavelength(	char Rx, char Tx);

unsigned char S3GetTxStartState();
int S3SetTxStartState(unsigned char state);

bool S3GetTxSelfTest();
int S3SetTxSelfTest(bool on);

bool S3GetTCompGainOption(); // TODO: Frig-to-go
bool S3GetLowNoiseOption();
bool S3GetWinTrackOption();
bool S3GetSoftShutdownOption();


void S3SetPrevRxedMsg(const char *Msg);
const char* S3GetPrevRxedMsg(void);
char S3GetPrevRemoteSrc();
void S3SetPrevRemoteSrc(char MsgSrc);

int S3GetLinkParas(char Rx, char Tx, char IP,
				   double *P1dBIn, double *P1dBOut, double *Sens);

int S3Redraw();

}; // extern "C"

// ----------------------------------------------------------------------------