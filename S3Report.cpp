// ----------------------------------------------------------------------------
// Reporting functions

#include "stdafx.h"

#include "S3DataModel.h"
#include "S3GPIB.h"

extern pS3DataModel S3Data;

// ----------------------------------------------------------------------------
// MODIFsfxsxsxsxssYh

int S3ReportFile(char *Buf, char *f)
{
	strcpy_s(S3Data->m_ReportFileName, S3_MAX_FILENAME_LEN, f);

	return 0;
}

// ----------------------------------------------------------------------------

int S3AllReport(char *Buf)
{
	// Copy & format system data to BufS
	return 0;
}

// ----------------------------------------------------------------------------

int S3SysReport(char *Buf)
{
	int len;

	sprintf_s(Buf, S3_MAX_GPIB_RET_LEN,
		"\n\nController Report\n------------------\nName:\t%s\nConfig:\t%s\nFile:\t%s\nFile v:\t%.2f\n",
		S3Data->m_NodeName,
		S3Data->m_ConfigName,
		S3Data->m_ConfigPath,
		S3Data->m_FileVersion);

	len = strlen(Buf);

	sprintf_s(Buf + len, S3_MAX_GPIB_RET_LEN - len,
		"S/N:\t%s\nP/N:\t%s\nSW v:\t%s\nModel:\t%s\n",
		S3Data->m_SN,
		S3Data->m_PN,
		S3Data->m_SW,
		S3Data->m_ModelId);

	len = strlen(Buf);

	char Addr[S3_MAX_IP_ADDR_LEN];

	// S3GetIPAddrStr(Addr);

	sprintf_s(Buf + len, S3_MAX_GPIB_RET_LEN - len, "IP:\t%s\n", S3GetIPAddrStr());

	len = strlen(Buf);

	S3GetMACAddrStr(Addr);

	sprintf_s(Buf + len, S3_MAX_GPIB_RET_LEN - len, "MAC:\t%s\n", Addr);

	len = strlen(Buf);

	return 0;
}

// ----------------------------------------------------------------------------
// TODO: Stub

// Fixed length so taking liberties with safe string fns.
// Returns a 6x7, space delimited table. 

int S3TopologyReport(char *Buf)
{
	*Buf = '\0';
	char tmp[6];
	
	for(char Rx = 0; Rx < S3_MAX_RXS; Rx++)
	{
		sprintf_s(tmp, 6, "%2d", S3RxGetType(Rx));
		strcat_s(Buf, S3_MAX_GPIB_RET_LEN, tmp);

		for(char Tx = 0; Tx < S3_MAX_TXS; Tx++)
		{
			sprintf_s(tmp, 6, "%2d", S3TxGetType(Rx, Tx));
			strcat_s(Buf, S3_MAX_GPIB_RET_LEN, tmp);
		}

		strcat_s(Buf, S3_MAX_GPIB_RET_LEN, "\n");
	}

	strcat_s(Buf, S3_MAX_GPIB_RET_LEN, "\n");
	
	return 0;
}

// ----------------------------------------------------------------------------

int S3RxReport(char *Buf, char Rx)
{
	if (Rx < 0)
		return 1;

	pS3RxData rx = &S3Data->m_Rx[Rx];

	int len = strlen(Buf);

	sprintf_s(Buf + len, S3_MAX_GPIB_RET_LEN - len,
		"\nReceiver Report\n-------------------\nName:\t%s\nType:\tRx%d\n",
		rx->m_NodeName,
		rx->m_Type);

	len = strlen(Buf);

	sprintf_s(Buf + len, S3_MAX_GPIB_RET_LEN - len,
		"S/N:\t%s\nP/N:\t%s\nFW:\t%s\n",
		rx->m_SN,
		rx->m_PN,
		rx->m_FW);

	len = strlen(Buf);

	strcat_s(Buf, S3_MAX_GPIB_RET_LEN - len, "Active TXs:\n");
	len = strlen(Buf);

	// TODO: Do according to RX type
	for (char Tx = 0; Tx < S3_MAX_TXS; Tx++)
	{
		if (S3TxExistQ(Rx, Tx))
		{
			sprintf_s(Buf + len, S3_MAX_GPIB_RET_LEN - len, "\t\t%d\t%s\n", Tx + 1,
				S3Data->m_Rx[Rx].m_Tx[Tx].m_ModelName); // S3TxGetType(Rx, Tx));
		}

		len = strlen(Buf);
	}

	return 0;
}

// ----------------------------------------------------------------------------

int S3TxReport(char *Buf, char Rx, char Tx)
{
	pS3TxData pTx = &S3Data->m_Rx[Rx].m_Tx[Tx];

	int len = strlen(Buf);

	sprintf_s(Buf + len, S3_MAX_GPIB_RET_LEN - len,
		"\nTransmitter Report\n-----------------------\nName:\t%s\nType:\tTx%d\n",
		pTx->m_NodeName,
		pTx->m_Type);

	len = strlen(Buf);

	sprintf_s(Buf + len, S3_MAX_GPIB_RET_LEN - len,
		"S/N:\t%s\nP/N:\t%s\nFW:\t%s\n",
		pTx->m_SN,
		pTx->m_PN,
		pTx->m_FW);

	len = strlen(Buf);

	// strcat_s(Buf, S3_MAX_GPIB_RET_LEN - len, "Active IPs:\n");
	// len = strlen(Buf);

	sprintf_s(Buf + len, S3_MAX_GPIB_RET_LEN - len,
		"\n\nTx Battery\n-------------\nSN:\t%s\nCharge (%c):\t%02d\nTemp.(C):\t%.1f\n",
		pTx->m_BattSN,
		'%',
		pTx->m_SoC,
		pTx->m_BattTemp);

	len = strlen(Buf);

	if (pTx->m_BattValidated)
		strcat_s(Buf, S3_MAX_GPIB_RET_LEN - len, "Valid:\tYes\n");
	else
		strcat_s(Buf, S3_MAX_GPIB_RET_LEN - len, "Valid:\tNo\n");

	return 0;
}

// ----------------------------------------------------------------------------

int S3IPReport(char *Buf, char Rx, char Tx, char IP)
{
	pS3IPData pIP = &S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP];

	int len = strlen(Buf);

	sprintf_s(Buf, S3_MAX_GPIB_RET_LEN - len,
		"\n\nInput Report\n----------------\nName:\t\t%s\n",
		pIP->m_NodeName);

	
	if (S3Data->m_DisplayUnits != S3_UNITS_MV)
	{
		len = strlen(Buf);
		sprintf_s(Buf + len, S3_MAX_GPIB_RET_LEN - len,
			"Max input (%S):\t%.2f\n", S3GetUnitString(), pIP->m_MaxInput);
	}
	else
	{
		len = strlen(Buf);

		if (pIP->m_MaxInput > 1.00)
			sprintf_s(Buf + len, S3_MAX_GPIB_RET_LEN - len,
				"Max input (%S):\t%.2f\n", "V", pIP->m_MaxInput);
		else
			sprintf_s(Buf + len, S3_MAX_GPIB_RET_LEN - len,
				"Max input (%S):\t%.2f\n", "mV", 1000.0 * pIP->m_MaxInput);
	}

	S3CfgReport(Buf, Rx, Tx, IP);

	return 0;
}

// ----------------------------------------------------------------------------

int S3ReportToFile(char *Buf)
{
	if (strcmp(S3Data->m_ReportFileName, ""))
	{
		FILE	*fid;

		int err = fopen_s(&fid, S3Data->m_ReportFileName, "a");

		// Else: just one of those things
		if (!err)
		{
			fwrite(Buf, sizeof(char), strlen(Buf), fid);
			fclose(fid);
		}
	}

	return 0;
}

// ----------------------------------------------------------------------------

int S3CfgReport(char *Buf, char Rx, char Tx, char IP)
{
	S3Config *cfg = &S3Data->m_Rx[Rx].m_Tx[Tx].m_Input[IP].m_Config;
	int len = strlen(Buf);

	sprintf_s(Buf + len, S3_MAX_GPIB_RET_LEN - len, "Gain (dBm):\t%d\n", cfg->m_Gain);

	strcat_s(Buf, S3_MAX_GPIB_RET_LEN - len, "Int T:\t\t");

	char str[S3_MAX_TAU_UNITS_LEN];
	S3TxGetTauUnitsA(str, Rx, Tx, IP);

	strcat_s(Buf, S3_MAX_GPIB_RET_LEN - len, str);
	strcat_s(Buf, S3_MAX_GPIB_RET_LEN - len, "\n");

	len = strlen(Buf);

	if (cfg->m_Tau == W50)
		strcat_s(Buf, S3_MAX_GPIB_RET_LEN - len, "Inp Z (Ohms):\t50\n");
	else
		strcat_s(Buf, S3_MAX_GPIB_RET_LEN - len, "Inp Z (Ohms):\t1M\n");

	len = strlen(Buf);

	if (cfg->m_LowNoiseMode)
		strcat_s(Buf, S3_MAX_GPIB_RET_LEN - len, "Low noise:\tOn\n");
	else
		strcat_s(Buf, S3_MAX_GPIB_RET_LEN - len, "Low noise:\tOff\n");

	len = strlen(Buf);

#ifdef WINTRACK
	if (S3Data->m_WinTrackOption)
	{
		if (cfg->m_WindowTracking)
			strcat_s(Buf, S3_MAX_GPIB_RET_LEN - len, "Win track:\tON\n");
		else
			strcat_s(Buf, S3_MAX_GPIB_RET_LEN - len, "Win track:\tOFF\n");

		len = strlen(Buf);
	}
#endif

	return 0;
}

// ----------------------------------------------------------------------------
// Buf must be pre-allocated: S3_MAX_INFO_STR_LEN
// TODO: Or pass buffer len?

int S3ConfigGetInfoStr(char *Buf, char Rx, char Tx, char IP)
{
	S3Config *cfg;
	
	if (Tx == -1)
		cfg = &S3Data->m_Rx[Rx].m_Config;
	if (IP == -1)
		cfg = &S3Data->m_Rx[Rx].m_Tx[Tx].m_Config;

	int len = 0; // strlen(Buf);

	sprintf_s(Buf, S3_MAX_INFO_STR_LEN,
		"Gain: %d dB; IPmax: %.2f dBm; ",
		cfg->m_Gain, cfg->m_MaxInput);

	if (IP != -1)
	{
		strcat_s(Buf, S3_MAX_GPIB_RET_LEN - len, "Int T: ");

		char str[S3_MAX_TAU_UNITS_LEN];
		S3TxGetTauUnitsA(str, Rx, Tx, IP);

		strcat_s(Buf, S3_MAX_GPIB_RET_LEN - len, str);
		strcat_s(Buf, S3_MAX_GPIB_RET_LEN - len, "; ");
	}

	len = strlen(Buf);

	if (cfg->m_InputZ == W50)
		strcat_s(Buf, S3_MAX_INFO_STR_LEN, "IPz: 50; ");
	else
		strcat_s(Buf, S3_MAX_INFO_STR_LEN, "IPz: 1M; ");

	len = strlen(Buf);

#ifdef S3LOWNOISE
	if (cfg->m_LowNoiseMode)
		strcat_s(Buf, S3_MAX_INFO_STR_LEN, "LN mode: ON; ");
	else
		strcat_s(Buf, S3_MAX_INFO_STR_LEN, "LN mode: OFF; ");

	len = strlen(Buf);
#endif

	if (S3Data->m_WinTrackOption)
	{
		if (cfg->m_WindowTracking)
			strcat_s(Buf, S3_MAX_INFO_STR_LEN, "Win track: ON; ");
		else
			strcat_s(Buf, S3_MAX_INFO_STR_LEN, "Win track: OFF; ");

		len = strlen(Buf);
	}

	return 0;
}

// ----------------------------------------------------------------------------
