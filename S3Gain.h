// ----------------------------------------------------------------------------

#define UPOW		0	// Info
#define UGAIN		1	// Key
#define	TX_RF1_PAD1	2	// Determined by path
#define	TX_RF1_PAD2	3	// Determined by path
#define	TX_RF1_LNA1	4	// Determined by path
#define	TX_RF1_LNA2	5	// Determined by path
#define	TX_RF1_DSA	6	// TX-RF1 DSA
#define	TX_OPT_DSA	7	// TX-OPT DSA
#define	RX_OPT_DSA	8	// RX-OPT DSA

//#ifdef __cplusplus
//extern "C" {
//#endif

#define S3_GAIN_VALUES	119 // [1-40] + [-1 - -78] + 0 
#define S3_N_P1DB		7
// #define S3_N_P1DB		15

// Based on -78 -> +40dB (inc.)
extern const char S3GainTable[S3_GAIN_VALUES][9];
extern double S3P1dBTableOld[S3_GAIN_VALUES];
extern double S3P1dBTable[S3_N_P1DB][3];

extern const char *S3GetGainParas_dB(char g_dB);
extern const char *S3GetGainParas_Pmax(char p_dBm);
extern int S3I2CSetIPGain(char Rx, char Tx, char IP);

//#ifdef __cplusplus
//}
//#endif

// ----------------------------------------------------------------------------