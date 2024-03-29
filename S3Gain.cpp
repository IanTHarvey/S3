// ----------------------------------------------------------------------------
// Convert user gain to individual DSA parameters

#include "stdafx.h"

#include "S3DataModel.h"
#include "S3Gain.h"

#ifdef TRIZEPS
#include "drvLib_app.h"
#endif

#include "S3I2C.h"

extern int		S3I2CSetPath(		char p);
extern int		S3I2CSetRFAtten(	char p);
extern int		S3I2CSetTxOptDSA(	char Rx, char Tx, char dsa, char gain);
extern int		S3I2CSetRxOptDSA(	char Rx, char dsa);


int S3I2CSetIPGain(char Rx, char Tx, char IP);

#include "S3TableModeA.h"
#include "S3TableModeB.h"

#include "S3TableRxPlusModeA.h"
#include "S3TableRxPlusModeB.h"

// User gain, user power, DSA1, DSA2, DSA3

// Tables from Y:\Project files\Sentinel3\System\s3_lineup.xlsm

// User power, User gain, PAD1, PAD2, LNA1, LNA2, OTX-RF1 DSA, OTX-OPT DSA, ORX-OPT DSA
// -62,40,32,16,,,Ask,Ask,Ask

/*
#define UPOW	0
#define UGAIN	1
#define	PAD1	2
#define	PAD2	3
#define	LNA1	4
#define	LNA2	5
#define	DSA1	6
#define	OTX_OPT	7
#define	ORX_OPT	8
*/

// ----------------------------------------------------------------------------
// From measurements by QP
// Gain, @500MHz, @1GHz
double S3P1dBTable[S3_N_P1DB][3] = 
{
	{40,	-30.0,	-30.4},
	{0,		7.0,	4.9},
	{-16,	23.0,	7.0},
	{-32,	39.0,	7.1},
	{-48,	55.0,	7.2},
	{-100,	55.0,	16.0}
};

/*
double S3P1dBTable[S3_N_P1DB][3] = 
{
	{40,	-30.6,	-30.4},
	{35,	-29.0,	-28.6}, 
	{30,	-28.5,	-28.3},
	{25,	-28.3,	-30.0},
	{20,	-28.3,	-30.0},
	{15,	-28.3,	-30.0},
	{10,	-28.2,	-30.0},
	{5,		-28.4,	-30.0},
	{1,		-28.4,	-30.0},
	{0,		7.9,	4.9},
	{-5,	8.0,	7.0},
	{-10,	8.0,	7.1},
	{-15,	8.0,	7.2},
	{-20,	16.0,	16.0},
	{-100,	16.0,	16.0}
};
*/

// ----------------------------------------------------------------------------

const char S3GainTable[S3_GAIN_VALUES][9] = {
{56,-78,32,16,0,0,0,15,15},
{55,-77,32,16,0,0,0,14,15},
{54,-76,32,16,0,0,0,13,15},
{53,-75,32,16,0,0,0,12,15},
{52,-74,32,16,0,0,0,11,15},
{51,-73,32,16,0,0,0,10,15},
{50,-72,32,16,0,0,0,9,15},
{49,-71,32,16,0,0,0,8,15},
{48,-70,32,16,0,0,0,7,15},
{47,-69,32,16,0,0,0,6,15},
{46,-68,32,16,0,0,0,5,15},
{45,-67,32,16,0,0,0,4,15},
{44,-66,32,16,0,0,0,3,15},
{43,-65,32,16,0,0,0,2,15},
{42,-64,32,16,0,0,0,1,15},
{41,-63,32,16,0,0,0,0,15},
{40,-62,32,16,0,0,0,0,14},
{39,-61,32,16,0,0,0,0,13},
{38,-60,32,16,0,0,0,0,12},
{37,-59,32,16,0,0,0,0,11},
{36,-58,32,16,0,0,0,0,10},
{35,-57,32,16,0,0,0,0,9},
{34,-56,32,16,0,0,0,0,8},
{33,-55,32,16,0,0,0,0,7},
{32,-54,32,16,0,0,0,0,6},
{31,-53,32,16,0,0,0,0,5},
{30,-52,32,16,0,0,0,0,4},
{29,-51,32,16,0,0,0,0,3},
{28,-50,32,16,0,0,0,0,2},
{27,-49,32,16,0,0,0,0,1},
{26,-48,32,16,0,0,0,0,0},
{25,-47,32,0,0,0,0,0,15},
{24,-46,32,0,0,0,0,0,14},
{23,-45,32,0,0,0,0,0,13},
{22,-44,32,0,0,0,0,0,12},
{21,-43,32,0,0,0,0,0,11},
{20,-42,32,0,0,0,0,0,10},
{19,-41,32,0,0,0,0,0,9},
{18,-40,32,0,0,0,0,0,8},
{17,-39,32,0,0,0,0,0,7},
{16,-38,32,0,0,0,0,0,6},
{15,-37,32,0,0,0,0,0,5},
{14,-36,32,0,0,0,0,0,4},
{13,-35,32,0,0,0,0,0,3},
{12,-34,32,0,0,0,0,0,2},
{11,-33,32,0,0,0,0,0,1},
{10,-32,32,0,0,0,0,0,0},
{9,-31,0,16,0,0,0,0,15},
{8,-30,0,16,0,0,0,0,14},
{7,-29,0,16,0,0,0,0,13},
{6,-28,0,16,0,0,0,0,12},
{5,-27,0,16,0,0,0,0,11},
{4,-26,0,16,0,0,0,0,10},
{3,-25,0,16,0,0,0,0,9},
{2,-24,0,16,0,0,0,0,8},
{1,-23,0,16,0,0,0,0,7},
{0,-22,0,16,0,0,0,0,6},
{-1,-21,0,16,0,0,0,0,5},
{-2,-20,0,16,0,0,0,0,4},
{-3,-19,0,16,0,0,0,0,3},
{-4,-18,0,16,0,0,0,0,2},
{-5,-17,0,16,0,0,0,0,1},
{-6,-16,0,16,0,0,0,0,0},
{-7,-15,0,0,0,0,0,0,15},
{-8,-14,0,0,0,0,0,0,14},
{-9,-13,0,0,0,0,0,0,13},
{-10,-12,0,0,0,0,0,0,12},
{-11,-11,0,0,0,0,0,0,11},
{-12,-10,0,0,0,0,0,0,10},
{-13,-9,0,0,0,0,0,0,9},
{-14,-8,0,0,0,0,0,0,8},
{-15,-7,0,0,0,0,0,0,7},
{-16,-6,0,0,0,0,0,0,6},
{-17,-5,0,0,0,0,0,0,5},
{-18,-4,0,0,0,0,0,0,4},
{-19,-3,0,0,0,0,0,0,3},
{-20,-2,0,0,0,0,0,0,2},
{-21,-1,0,0,0,0,0,0,1},
{-22,0,0,0,0,0,0,0,0},
{-23,1,0,0,1,1,15,15,9},
{-24,2,0,0,1,1,15,15,8},
{-25,3,0,0,1,1,15,15,7},
{-26,4,0,0,1,1,15,15,6},
{-27,5,0,0,1,1,15,15,5},
{-28,6,0,0,1,1,15,15,4},
{-29,7,0,0,1,1,15,15,3},
{-30,8,0,0,1,1,15,15,2},
{-31,9,0,0,1,1,15,15,1},
{-32,10,0,0,1,1,15,15,0},
{-33,11,0,0,1,1,15,14,0},
{-34,12,0,0,1,1,15,13,0},
{-35,13,0,0,1,1,15,12,0},
{-36,14,0,0,1,1,15,11,0},
{-37,15,0,0,1,1,15,10,0},
{-38,16,0,0,1,1,15,9,0},
{-39,17,0,0,1,1,15,8,0},
{-40,18,0,0,1,1,15,7,0},
{-41,19,0,0,1,1,15,6,0},
{-42,20,0,0,1,1,15,5,0},
{-43,21,0,0,1,1,15,4,0},
{-44,22,0,0,1,1,15,3,0},
{-45,23,0,0,1,1,15,2,0},
{-46,24,0,0,1,1,15,1,0},
{-47,25,0,0,1,1,15,0,0},
{-48,26,0,0,1,1,14,0,0},
{-49,27,0,0,1,1,13,0,0},
{-50,28,0,0,1,1,12,0,0},
{-51,29,0,0,1,1,11,0,0},
{-52,30,0,0,1,1,10,0,0},
{-53,31,0,0,1,1,9,0,0},
{-54,32,0,0,1,1,8,0,0},
{-55,33,0,0,1,1,7,0,0},
{-56,34,0,0,1,1,6,0,0},
{-57,35,0,0,1,1,5,0,0},
{-58,36,0,0,1,1,4,0,0},
{-59,37,0,0,1,1,3,0,0},
{-60,38,0,0,1,1,2,0,0},
{-61,39,0,0,1,1,1,0,0},
{-62,40,0,0,1,1,0,0,0}
};

// TODO: Get some values from somewhere
short S3PeakThreshTabled[S3_GAIN_VALUES] = {
0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

double S3P1dBTableOld[S3_GAIN_VALUES] = {
61.3, 61.2, 61.1, 61.0, 60.8, 60.6, 60.4, 60.2, 59.9, 59.5,
59.1, 58.7, 58.2, 57.6, 57.0, 56.3, 56.2, 56.3, 56.2, 56.2,
56.3, 56.2, 56.2, 56.2, 56.1, 56.1, 56.1, 56.0, 56.0, 55.9,
55.8, 40.3, 40.3, 40.3, 40.3, 40.3, 40.2, 40.2, 40.2, 40.2,
40.2, 40.1, 40.1, 40.0, 40.0, 39.9, 39.8, 24.3, 24.3, 24.2,
24.3, 24.2, 24.2, 24.2, 24.2, 24.2, 24.1, 24.1, 24.1, 24.0,
24.0, 23.9, 23.8,  8.3,  8.3,  8.3,  8.3,  8.2,  8.3,  8.2,
 8.2,  8.2,  8.1,  8.2,  8.0,  8.0,  8.0,  7.9,  7.8,
//-13.9,
//-13.8,
//-13.9,
//-13.9,
//-13.9,
//-13.8,
-13.9, -13.9, -13.9, -13.9, -13.9, -13.9, -13.9, -13.9, -13.9, -13.9,
-14.0, -14.0, -14.1, -14.2, -14.4, -14.6, -14.8, -15.0, -15.3, -15.6,
-16.0, -16.5, -17.0, -17.5, -18.2, -19.0, -19.9, -20.7, -21.7, -22.6,
-23.5, -24.4, -25.4, -26.4, -27.3, -28.3, -29.3, -30.3, -31.3, -32.3
};

// ----------------------------------------------------------------------------

const char *S3GetGainParas_dB(char g_dB)
{
  	return S3GainTable[g_dB - S3GainTable[0][UGAIN]];
}

// ----------------------------------------------------------------------------
// TODO: Obsolete

const char *S3GetGainParas_Pmax(char p_dBm)
{
	return S3GetGainParas_dB(-(p_dBm + 22));
}

// ----------------------------------------------------------------------------
// TODO: This may need to be split over a number of polls for large systems.
// Not used for Tx1, may be required for Rx6

// TODO: Obsolete

/*
int S3I2CSetGain(char Rx)
{
	return 0;
	
	for(char Tx = 0; Tx < S3RxGetNTx(Rx); Tx++)
	{
		for(char IP = 0; IP < S3TxGetNIP(Rx, Tx); IP++)
		{
			S3I2CSetIPGain(Rx, Tx, IP);
		}
	}

	return 0;
}
*/

// ----------------------------------------------------------------------------
// Path is 1-indexed.

char S3TxGetRFPath(char Rx, char Tx, char IP)
{
	char	RFPath;
	InputZ  Imp = S3GetImpedance(Rx, Tx, IP);
	int		Gain = S3IPGetGain(Rx, Tx, IP);

	switch (S3GetSigmaTau(Rx, Tx, IP))
	{
	case TauLo: RFPath = 4; break;
	case TauMd: RFPath = 5; break;
	case TauHi: RFPath = 6; break;
	default:
		if (Imp == W1M)
			RFPath = 7;
		else if (Gain <= (0 + S3RxGetExtraGainCap(Rx)))
			RFPath = 3;
		else
			RFPath = 1;
	}

	return RFPath;
}

// ----------------------------------------------------------------------------

extern pS3DataModel S3Data;

int S3GetLinkParas(char Rx, char Tx, char IP,
				   double *P1dBIn, double *P1dBOut, double *Sens)
{
	int		Gain = S3IPGetGain(Rx, Tx, IP);

	char Idx = (char)(Gain - S3GetMinGain(Rx, Tx));

	double (*TableModeA)[15];
	double (*TableModeB)[15];

	if (S3RxGetExtraGainCap(Rx) == 0)
	{
		TableModeA = S3TableModeA;
		TableModeB = S3TableModeB;
	}
	else
	{
		TableModeA = S3TableRxPlusModeA;
		TableModeB = S3TableRxPlusModeB;
	}

	if (S3Data->m_SigSize == S3_UNITS_SMALL)
	{
		if (!S3Get3PCLinearity())
		{
			if (S3Data->m_DisplayUnits == S3_UNITS_WATTS)
			{
				*P1dBIn =	TableModeA[Idx][4];
				*P1dBOut =	TableModeA[Idx][8];
				*Sens =		TableModeA[Idx][2];
			}
			else
			{
				*P1dBIn =	TableModeA[Idx][5];
				*P1dBOut =	TableModeA[Idx][9];
				*Sens =		TableModeA[Idx][3];
			}
		}
		else
		{
			if (S3Data->m_DisplayUnits == S3_UNITS_WATTS)
			{
				*P1dBIn =	TableModeA[Idx][6];
				*P1dBOut =	TableModeA[Idx][10];
				*Sens =		TableModeA[Idx][2];
			}
			else
			{
				*P1dBIn =	TableModeA[Idx][7];
				*P1dBOut =	TableModeA[Idx][11];
				*Sens =		TableModeA[Idx][3];
			}
		}
	}
	else
	{
		if (!S3Get3PCLinearity())
		{
			if (S3Data->m_DisplayUnits == S3_UNITS_WATTS)
			{
				*P1dBIn =	TableModeB[Idx][4];
				*P1dBOut =	TableModeB[Idx][8];
				*Sens =		TableModeB[Idx][2];
			}
			else
			{
				*P1dBIn =	TableModeB[Idx][5];
				*P1dBOut =	TableModeB[Idx][9];
				*Sens =		TableModeB[Idx][3];
			}
		}
		else
		{
			if (S3Data->m_DisplayUnits == S3_UNITS_WATTS)
			{
				*P1dBIn =	TableModeB[Idx][6];
				*P1dBOut =	TableModeB[Idx][10];
				*Sens =		TableModeB[Idx][2];
			}
			else
			{
				*P1dBIn =	TableModeB[Idx][7];
				*P1dBOut =	TableModeB[Idx][11];
				*Sens =		TableModeB[Idx][3];
			}
		}
	}

	return 0;
}

// ----------------------------------------------------------------------------
// Set these according to Rx and (potential) Tx capabilities

char S3GetMinGain(char Rx, char Tx)
{
	return S3_MIN_GAIN + S3Data->m_Rx[Rx].m_ExtraGainCap;
}

// ----------------------------------------------------------------------------

char S3GetMaxGain(char Rx, char Tx)
{
	return S3_MAX_GAIN + S3Data->m_Rx[Rx].m_ExtraGainCap;
}

// ----------------------------------------------------------------------------