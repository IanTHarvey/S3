// ----------------------------------------------------------------------------
// Tx diagnostic dumps

#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "S3DataModel.h"

#ifdef TRIZEPS
#include "drvLib_app.h"
#endif

#include "S3I2C.h"
#include "S3Gain.h"

extern pS3DataModel S3Data;

// Diag only
void S3PrintByte(FILE *fid, char x)
{
	for(int n = 0; n < 8; n++)
	{
		if ((x & 0x80) != 0)
			fprintf(fid, "1");
		else
			fprintf(fid, "0");

		if (n == 3)
			fprintf(fid, " "); // Space between nibbles

		x = x << 1;
	}

	fprintf(fid, "\n");
}

// ----------------------------------------------------------------------------

int S3I2CTxDumpOptConfig(char Rx, char Tx)
{
	char Filename[S3_MAX_TIME_STR_LEN], t[S3_MAX_TIME_STR_LEN];
  	S3GetTimeStr(t);

	strcpy_s(Filename, S3_MAX_TIME_STR_LEN, t);
	Filename[4] = Filename[7] = Filename[10] = Filename[13] = Filename[16] = '_';

	char Path[S3_MAX_FILENAME_LEN];
	sprintf_s(Path, S3_MAX_FILENAME_LEN, "\\Flashdisk\\S3\\TxOptConfig_%s.txt", Filename);

	FILE	*fid;

	int err = fopen_s(&fid, Path, "w");

	if (err)
		return 1;

	char tmp[20];
	const char *buf = (char *)S3I2CTxReadBuf;
	unsigned char Start = 0;

	fprintf(fid, "\nTX Optical I2C Map: Rx %d, Tx %d\n================================\n\n", Rx, Tx);

	fprintf(fid, "\n%s\n", t);

	fprintf(fid, "\nApp: %s\n\n", S3SysGetAppDateTime());

	fprintf(fid, "\n=========================================================\n");
	fprintf(fid, "Identity\n");
	fprintf(fid, "=========================================================\n");

	if (!S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_IDENT, 28))
	{
		strncpy_s(tmp, 20, buf, 15);
		tmp[15] = '\0';
		fprintf(fid, "%03d: PN:\t\t%s\n", Start, tmp);
		Start += 15;

		strncpy_s(tmp, 20, buf + 15, 3);
		tmp[3] = '\0';
		fprintf(fid, "%03d: FW:\t\t%s\n", Start, tmp);
		Start += 3;

		strncpy_s(tmp, 20, buf + 15 + 3, 7);
		tmp[7] = '\0';
		fprintf(fid, "%03d: Date:\t\t%s\n", Start, tmp);
		Start += 7;

		strncpy_s(tmp, 20, buf + 15 + 3 + 7, 4);
		tmp[4] = '\0';
		fprintf(fid, "%03d: PW:\t\t%s\n", Start, tmp);
		Start += 4;
	}

	if (!S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_IDENT + Start, 38))
	{
		strncpy_s(tmp, 20, buf, S3I2C_PN_LEN);
		tmp[S3I2C_PN_LEN] = '\0';
		fprintf(fid, "%03d: Module PN:\t%s\n", Start, tmp);
		Start += S3I2C_PN_LEN;

		strncpy_s(tmp, 20, buf + S3I2C_PN_LEN, 2);
		tmp[2] = '\0';
		fprintf(fid, "%03d: OEM No.:\t%s\n", Start, tmp);
		Start += 2;

		strncpy_s(tmp, 20, buf + S3I2C_PN_LEN + 2, S3I2C_SN_LEN);
		tmp[S3I2C_SN_LEN] = '\0';
		fprintf(fid, "%03d: SN:\t\t%s\n", Start, tmp);
		Start += S3I2C_SN_LEN;

		strncpy_s(tmp, 20, buf + S3I2C_PN_LEN + 2 + S3I2C_SN_LEN, 8);
		tmp[8] = '\0';
		fprintf(fid, "%03d: Cust ID:\t%s\n", Start, tmp);
		Start += 8;

		fprintf(fid, "%03d: Type:\t\t0x%02X\n", Start, *(S3I2CTxReadBuf + S3I2C_PN_LEN + 2 + S3I2C_SN_LEN + 8));
	}
	
	fprintf(fid, "\n=========================================================\n");
	fprintf(fid, "Thresholds\n");
	fprintf(fid, "=========================================================\n");

	if (!S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_THRESH, 48))
	{
		char i = 0;

		fprintf(fid, "%03d Vcc:\t\t%05d - %05d\n", S3I2C_TX_OPT_THRESH + i * 4,
			S3RevByteShort(S3I2CTxReadBuf + i * 4),
			S3RevByteShort(S3I2CTxReadBuf + i * 4 + 2)); i++;

		fprintf(fid, "%03d Bias:\t\t%05d - %05d\n", S3I2C_TX_OPT_THRESH + i * 4,
			S3RevByteShort(S3I2CTxReadBuf + i * 4),
			S3RevByteShort(S3I2CTxReadBuf + i * 4 + 2)); i++;

		fprintf(fid, "%03d Power:\t\t%05d - %05d\n", S3I2C_TX_OPT_THRESH + i * 4,
			S3RevByteShort(S3I2CTxReadBuf + i * 4),
			S3RevByteShort(S3I2CTxReadBuf + i * 4 + 2)); i++;

		fprintf(fid, "%03d RF level:\t%05d - %05d\n", S3I2C_TX_OPT_THRESH + i * 4,
			S3RevByteShort(S3I2CTxReadBuf + i * 4),
			S3RevByteShort(S3I2CTxReadBuf + i * 4 + 2)); i++;

		fprintf(fid, "%03d RF gain:\t%05d - %05d\n", S3I2C_TX_OPT_THRESH + i * 4,
			S3RevByteShort(S3I2CTxReadBuf + i * 4),
			S3RevByteShort(S3I2CTxReadBuf + i * 4 + 2)); i++;

		fprintf(fid, "%03d Feed I:\t\t%05d - %05d\n", S3I2C_TX_OPT_THRESH + i * 4,
			S3RevByteShort(S3I2CTxReadBuf + i * 4),
			S3RevByteShort(S3I2CTxReadBuf + i * 4 + 2)); i++;

		fprintf(fid, "%03d Peak:\t\t%05d - %05d\n", S3I2C_TX_OPT_THRESH + i * 4,
			S3RevByteShort(S3I2CTxReadBuf + i * 4),
			S3RevByteShort(S3I2CTxReadBuf + i * 4 + 2)); i++;

		fprintf(fid, "%03d Laser T:\t%05d - %05d\n", S3I2C_TX_OPT_THRESH + i * 4,
			S3RevByteShort(S3I2CTxReadBuf + i * 4),
			S3RevByteShort(S3I2CTxReadBuf + i * 4 + 2)); i++;

		fprintf(fid, "%03d Gain soft:\t%05d - %05d\n", S3I2C_TX_OPT_THRESH + i * 4,
			S3RevByteShort(S3I2CTxReadBuf + i * 4),
			S3RevByteShort(S3I2CTxReadBuf + i * 4 + 2)); i++;

		fprintf(fid, "%03d AGC target:\t%05d - %05d\n", S3I2C_TX_OPT_THRESH + i * 4,
			S3RevByteShort(S3I2CTxReadBuf + i * 4),
			S3RevByteShort(S3I2CTxReadBuf + i * 4 + 2)); i++;

		fprintf(fid, "%03d TEC I:\t\t%05d - %05d\n", S3I2C_TX_OPT_THRESH + i * 4,
			S3RevByteShort(S3I2CTxReadBuf + i * 4),
			S3RevByteShort(S3I2CTxReadBuf + i * 4 + 2)); i++;

		fprintf(fid, "%03d Module T:\t%05d - %05d\n", S3I2C_TX_OPT_THRESH + i * 4,
			S3RevByteShort(S3I2CTxReadBuf + i * 4),
			S3RevByteShort(S3I2CTxReadBuf + i * 4 + 2)); i++;
	}	

	fprintf(fid, "\n=========================================================\n");
	fprintf(fid, "Config\n");
	fprintf(fid, "=========================================================\n");

	if (!S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_CONFIG, 48))
	{
		fprintf(fid, "--------------------\nRaw: %03d-%03d\n--------------------\n",
			S3I2C_TX_OPT_CONFIG, S3I2C_TX_OPT_CONFIG + 48);

		for(char i = 0; i < 47; i++)
			fprintf(fid, "%03d: %02x\n", i + S3I2C_TX_OPT_CONFIG, S3I2CTxReadBuf[i]);

		fprintf(fid, "\n");

		fprintf(fid, "%03d (0xCC): ", S3I2C_TX_OPT_CONFIG + 44);
		S3PrintByte(fid, S3I2CTxReadBuf[44]);
		fprintf(fid, "%03d (0xCD): ", S3I2C_TX_OPT_CONFIG + 45);
		S3PrintByte(fid, S3I2CTxReadBuf[45]);

		fprintf(fid, "\n");

		for(char i = 0; i < 17; i++)
		{
			if (i == 0)
				fprintf(fid, "%03d: %.2fV\n",
					i * 2 + S3I2C_TX_OPT_CONFIG, (double)S3RevByteShort(S3I2CTxReadBuf + i * 2) / 10000.0);
			else if (i == 1 || i == 5)
				fprintf(fid, "%03d: %.2fdBm\n",
					i * 2 + S3I2C_TX_OPT_CONFIG, (double)S3RevByteShort(S3I2CTxReadBuf + i * 2) / 100.0);
			else if (i == 3 || i == 4 || i == 6 || i == 7)
				fprintf(fid, "%03d: %.2fdB\n",
					i * 2 + S3I2C_TX_OPT_CONFIG, (double)S3RevByteShort(S3I2CTxReadBuf + i * 2) / 100.0);
			else if (i == 9 || i == 15)
				fprintf(fid, "%03d: %.2fC\n",
					i * 2 + S3I2C_TX_OPT_CONFIG, (double)S3RevByteShort(S3I2CTxReadBuf + i * 2) / 256.0);
			else if (i == 16)
				fprintf(fid, "%03d: %.2f\n",
					i * 2 + S3I2C_TX_OPT_CONFIG, (double)S3RevByteShort(S3I2CTxReadBuf + i * 2) / 10000.0);
			else
				fprintf(fid, "%03d: %d\n",
					i * 2 + S3I2C_TX_OPT_CONFIG, S3RevByteShort(S3I2CTxReadBuf + i * 2));
		}
	}

	fprintf(fid, "\n=========================================================\n");
	fprintf(fid, "Monitors\n");
	fprintf(fid, "=========================================================\n");

	if (!S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_MON, 32))
	{
		unsigned char i = 0;

		fprintf(fid, "%03d: Life:\t%d\n",
					S3I2C_TX_CTRL_MON, + 2 * i, S3RevByteShort(S3I2CTxReadBuf + 2 * i++));

		fprintf(fid, "%03d: Vcc:\t%.2fV\n",
					S3I2C_TX_CTRL_MON + 2 * i, (double)S3RevByteShort(S3I2CTxReadBuf + 2 * i++) / 1000.0);

		fprintf(fid, "%03d: Bias:\t%.2fmA\n",
					S3I2C_TX_CTRL_MON + 2 * i, (double)S3RevByteShort(S3I2CTxReadBuf + 2 * i++) / 2000.0);

		fprintf(fid, "%03d: Power:\t%.2fdBm\n",
					S3I2C_TX_CTRL_MON + 2 * i, (double)S3RevByteShort(S3I2CTxReadBuf + 2 * i++) / 100.0);

		fprintf(fid, "%03d: PowerADC:\t%d\n",
					S3I2C_TX_CTRL_MON + 2 * i, S3RevByteShort(S3I2CTxReadBuf + 2 * i++));

		fprintf(fid, "%03d: RFMon:\t%.2f\n",
					S3I2C_TX_CTRL_MON + 2 * i, (double)S3RevByteShort(S3I2CTxReadBuf + 2 * i++) / 100.0);

		fprintf(fid, "%03d: RFGain:\t%.2fdB\n",
					S3I2C_TX_CTRL_MON + 2 * i, (double)S3RevByteShort(S3I2CTxReadBuf + 2 * i++) / 100.0);
		
		fprintf(fid, "%03d: FeedI:\t%.2fmA\n",
					S3I2C_TX_CTRL_MON + 2 * i, (double)S3RevByteShort(S3I2CTxReadBuf + 2 * i++) / 10.0);
		
		fprintf(fid, "%03d: FeedV:\t%dmV\n",
					S3I2C_TX_CTRL_MON + 2 * i, S3RevByteShort(S3I2CTxReadBuf + 2 * i++));
		
		fprintf(fid, "%03d: LaserT:\t%dC\n",
					S3I2C_TX_CTRL_MON + 2 * i, S3RevByteShort(S3I2CTxReadBuf + 2 * i++) / 256);

		fprintf(fid, "%03d: TECI:\t%.2fmA\n",
					S3I2C_TX_CTRL_MON + 2 * i, (double)S3RevByteShort(S3I2CTxReadBuf + 2 * i++) / 10.0);
		
		fprintf(fid, "%03d: RFIP:\t%.2fdBm\n",
					S3I2C_TX_CTRL_MON + 2 * i, (double)S3RevByteShort(S3I2CTxReadBuf + 2 * i++) / 100.0);
		
		fprintf(fid, "%03d: ModuleT:\t%dC\n",
					S3I2C_TX_CTRL_MON + 2 * i, S3RevByteShort(S3I2CTxReadBuf + 2 * i++) / 256);
		
		fprintf(fid, "%03d: ModuleTADC:\t%d\n",
					S3I2C_TX_CTRL_MON + 2 * i, S3RevByteShort(S3I2CTxReadBuf + 2 * i++));

		fprintf(fid, "%03d: PeakP:\t%.2f\n",
					S3I2C_TX_CTRL_MON + 2 * i, (double)S3RevByteShort(S3I2CTxReadBuf + 2 * i++) / 100.0);

		fprintf(fid, "%03d: PeakPHold:\t%.2f\n",
					S3I2C_TX_CTRL_MON + 2 * i, (double)S3RevByteShort(S3I2CTxReadBuf + 2 * i++) / 100.0);
	}
	
	fprintf(fid, "\n=========================================================\n");
	fprintf(fid, "Alarms\n");
	fprintf(fid, "=========================================================\n");

	if (!S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_ALARM, 3))
	{
		for(char i = 0; i < 3; i++)
		{
			fprintf(fid, "%03d: 0x%02x: ",
					i + S3I2C_TX_OPT_ALARM, *(S3I2CTxReadBuf + i));

			S3PrintByte(fid, *(S3I2CTxReadBuf + i));
		}
	}

	fprintf(fid, "\n=========================================================\n");
	fprintf(fid, "Status\n");
	fprintf(fid, "=========================================================\n");

	if (!S3I2CReadSerialData(S3I2C_TX_OPT_ADDR, S3I2C_TX_OPT_ALARM + 3, 2))
	{
		for(char i = 0; i < 2; i++)
		{
			fprintf(fid, "%03d: 0x%02x: ",
					i + S3I2C_TX_OPT_ALARM + 3, *(S3I2CTxReadBuf + i));

			S3PrintByte(fid, *(S3I2CTxReadBuf + i));
		}
	}

	fprintf(fid, "\n\n============================================================\n");
	fprintf(fid, "Le Fin\n============================================================\n");

	fclose(fid);

	return 0;
}

// ----------------------------------------------------------------------------

int S3I2CTxDumpCtrlConfig(char Rx, char Tx)
{
	char Filename[S3_MAX_TIME_STR_LEN], t[S3_MAX_TIME_STR_LEN];
	S3GetTimeStr(t);

	strcpy_s(Filename, S3_MAX_TIME_STR_LEN, t);
	Filename[4] = Filename[7] = Filename[10] = Filename[13] = Filename[16] = '_';

	char Path[S3_MAX_FILENAME_LEN];
	sprintf_s(Path, S3_MAX_FILENAME_LEN, "\\Flashdisk\\S3\\TxCtrlConfig_%s.txt", Filename);

	FILE	*fid;

	int err = fopen_s(&fid, Path, "w");

	if (err)
		return 1;

	char tmp[20];
	const char *buf = (char *)S3I2CTxReadBuf;
	unsigned char Start = 0;

	fprintf(fid, "\nTX Control I2C Map: Rx %d, Tx %d\n================================\n", Rx, Tx);

	fprintf(fid, "\n%s\n", t);

	fprintf(fid, "\nApp: %s\n\n", S3SysGetAppDateTime());

	fprintf(fid, "\n=========================================================\n");
	fprintf(fid, "Identity\n");
	fprintf(fid, "=========================================================\n");

	if (!S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_CTRL_IDENT, 29))
	{
		strncpy_s(tmp, 20, buf, 15);
		tmp[15] = '\0';
		fprintf(fid, "%03d: PN:\t%s\n", Start, tmp);
		Start += 15;

		strncpy_s(tmp, 20, buf + Start, 3);
		tmp[3] = '\0';
		fprintf(fid, "%03d: FW:\t%s\n", Start, tmp);
		Start += 3;

		strncpy_s(tmp, 20, buf + Start, 7);
		tmp[7] = '\0';
		fprintf(fid, "%03d: Date:\t%s\n", Start, tmp);
		Start += 7;

		strncpy_s(tmp, 20, buf + Start, 4);
		tmp[4] = '\0';
		fprintf(fid, "%03d: PW:\t%s\n", Start, tmp);
		Start += 4;
	}

	unsigned char Start2 = Start;

	if (!S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_CTRL_IDENT + Start, 37))
	{
		strncpy_s(tmp, 20, buf, S3I2C_PN_LEN);
		tmp[S3I2C_PN_LEN] = '\0';
		fprintf(fid, "%03d: Module PN:\t%s\n", Start, tmp);
		Start += S3I2C_PN_LEN;

		// strncpy_s(tmp, 20, buf + Start - Start2, 2);
		// tmp[2] = '\0';
		// fprintf(fid, "%d: OEM No.: %s\n", Start, tmp);
		Start += 2;

		strncpy_s(tmp, 20, buf + Start - Start2, S3I2C_SN_LEN);
		tmp[S3I2C_SN_LEN] = '\0';
		fprintf(fid, "%03d: SN:\t%s\n", Start, tmp);
		Start += S3I2C_SN_LEN;

		strncpy_s(tmp, 20, buf + Start - Start2, 8);
		tmp[8] = '\0';
		fprintf(fid, "%03d: Cust ID:\t%s\n", Start, tmp);
		Start += 8;
	}

	Start2 = Start;

	if (!S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_CTRL_IDENT + Start, 7))
	{
		fprintf(fid, "%03d: Type: 0x%02X\n", Start, *(S3I2CTxReadBuf + Start - Start2));
		Start += 1;

		fprintf(fid, "\nIntegrator Taus:\n----------------\n");
		for (char i = 0; i < 3; i++)
		{
			fprintf(fid, "%03d: %dE%d\n", Start,
						*(S3I2CTxReadBuf + Start - Start2),
						*(S3I2CTxReadBuf + Start - Start2 + 1));
			Start += 2;
		}
	}

	fprintf(fid, "\n=========================================================\n");
	fprintf(fid, "Thresholds\n");
	fprintf(fid, "=========================================================\n");

	if (!S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_CTRL_THRESH, 20))
	{
		char i = 0;

		fprintf(fid, "%03d Vcc:\t%d - %d\n", S3I2C_TX_CTRL_THRESH + i * 4,
				S3RevByteShort(S3I2CTxReadBuf + i * 4),
				S3RevByteShort(S3I2CTxReadBuf + i * 4 + 2)); i++;

		fprintf(fid, "%03d 12V:\t%d - %d\n", S3I2C_TX_CTRL_THRESH + i * 4,
				S3RevByteShort(S3I2CTxReadBuf + i * 4),
				S3RevByteShort(S3I2CTxReadBuf + i * 4 + 2)); i++;

		fprintf(fid, "%03d 6V:\t\t%d - %d\n", S3I2C_TX_CTRL_THRESH + i * 4,
				S3RevByteShort(S3I2CTxReadBuf + i * 4),
				S3RevByteShort(S3I2CTxReadBuf + i * 4 + 2)); i++;

		fprintf(fid, "%03d TxOpt:\t%d - %d\n", S3I2C_TX_CTRL_THRESH + i * 4,
				S3RevByteShort(S3I2CTxReadBuf + i * 4),
				S3RevByteShort(S3I2CTxReadBuf + i * 4 + 2)); i++;

		fprintf(fid, "%03d RxOpt:\t%d - %d\n", S3I2C_TX_CTRL_THRESH + i * 4,
				S3RevByteShort(S3I2CTxReadBuf + i * 4),
				S3RevByteShort(S3I2CTxReadBuf + i * 4 + 2)); i++;
	}

	fprintf(fid, "\n=========================================================\n");
	fprintf(fid, "Config\n");
	fprintf(fid, "=========================================================\n");

	if (!S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_CTRL_CONFIG, 24))
	{
		fprintf(fid, "--------------------\nRaw: %03d-%03d\n--------------------\n",
			S3I2C_TX_CTRL_CONFIG, S3I2C_TX_OPT_CONFIG + 24);

		for(char i = 0; i < 24; i++)
			fprintf(fid, "%03d: %02x\n", i + S3I2C_TX_CTRL_CONFIG, S3I2CTxReadBuf[i]);
		fprintf(fid, "--------------------\n\n");

		fprintf(fid, "%03d: Bias (init):\t%d\n",
			0 + S3I2C_TX_CTRL_CONFIG, *(S3I2CTxReadBuf + 0));

		fprintf(fid, "%03d: Bias (cal):\t%d\n",
			1 + S3I2C_TX_CTRL_CONFIG, *(S3I2CTxReadBuf + 1));

		fprintf(fid, "%03d: CalMode:\t\t%d\n",
			4 + S3I2C_TX_CTRL_CONFIG, *(S3I2CTxReadBuf + 4));

		fprintf(fid, "%03d: Path:\t\t\t%d\n",
			5 + S3I2C_TX_CTRL_CONFIG, *(S3I2CTxReadBuf + 5));

		fprintf(fid, "%03d: Atten (RF):\t%d\n",
			6 + S3I2C_TX_CTRL_CONFIG, *(S3I2CTxReadBuf + 6));

		fprintf(fid, "%03d: RF IP:\t\t\t%d\n",
			7 + S3I2C_TX_CTRL_CONFIG, *(S3I2CTxReadBuf + 7));

		fprintf(fid, "\nFactory cal:\n");

		for(char i = 0; i < 8; i++)
		{
			unsigned char WANK0 = *(S3I2CTxReadBuf + 8 + i * 2);
			unsigned char WANK1 = *(S3I2CTxReadBuf + 8 + i * 2 + 1);
						
			fprintf(fid, "%03d: %d\n",
				i * 2 + S3I2C_TX_CTRL_CONFIG + 8, S3RevByteShort(S3I2CTxReadBuf + 8 + i * 2));
		}

		S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_CTRL_CONFIG + 7, 1);
		fprintf(fid, "%03d: IPSelect (RF):\t%d\n",
			7 + S3I2C_TX_CTRL_CONFIG, *S3I2CTxReadBuf);
		
		S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_CTRL_CONFIG + 6, 1);
		fprintf(fid, "%03d: Atten2 (RF):\t%d\n",
			6 + S3I2C_TX_CTRL_CONFIG, *S3I2CTxReadBuf);
	}

	if (!S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_CTRL_CONFIG + 32, 16))
	{
		fprintf(fid, "--------------------\nRaw: %03d-%03d\n--------------------\n",
			S3I2C_TX_CTRL_CONFIG + 32, S3I2C_TX_CTRL_CONFIG + 32 + 16);

		for(char i = 0; i < 16; i++)
			fprintf(fid, "%03d: %02x\n", i + S3I2C_TX_OPT_CONFIG + 32, S3I2CTxReadBuf[i]);
		fprintf(fid, "--------------------\n\n");

		fprintf(fid, "204: CompMode (0xCC):\t");
		S3PrintByte(fid, S3I2CTxReadBuf[12]);
		fprintf(fid, "205: CalMode (0xCD):\t");
		S3PrintByte(fid, S3I2CTxReadBuf[13]);
		fprintf(fid, "206: Osc (0xCE):\t\t");
		S3PrintByte(fid, S3I2CTxReadBuf[14]);
		fprintf(fid, "207: Standby (0xCF):\t");
		S3PrintByte(fid, S3I2CTxReadBuf[15]);
	}

	fprintf(fid, "\n=========================================================\n");
	fprintf(fid, "Monitors\n");
	fprintf(fid, "=========================================================\n");

	if (!S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_CTRL_MON, 14))
	{
		fprintf(fid, "%03d: Life:\t%d\n",
					S3I2C_TX_CTRL_MON, S3RevByteShort(S3I2CTxReadBuf + 0));

		fprintf(fid, "%03d: Vcc:\t%d\n",
					S3I2C_TX_CTRL_MON + 2, S3RevByteShort(S3I2CTxReadBuf + 2));

		fprintf(fid, "%03d: 12V:\t%d\n",
					S3I2C_TX_CTRL_MON + 4, S3RevByteShort(S3I2CTxReadBuf + 4));

		fprintf(fid, "%03d: 6V:\t%d\n",
					S3I2C_TX_CTRL_MON + 6, S3RevByteShort(S3I2CTxReadBuf + 6));

		fprintf(fid, "%03d: TxOpt:\t%d\n",
					S3I2C_TX_CTRL_MON + 8, S3RevByteShort(S3I2CTxReadBuf + 8));

		fprintf(fid, "%03d: RxOpt:\t%d\n",
					S3I2C_TX_CTRL_MON + 10, S3RevByteShort(S3I2CTxReadBuf + 10));

		fprintf(fid, "%03d: Bias:\t%d\n",
					S3I2C_TX_CTRL_MON + 12, S3RevByteShort(S3I2CTxReadBuf + 12));
	}

	fprintf(fid, "\n=========================================================\n");
	fprintf(fid, "Alarms\n");
	fprintf(fid, "=========================================================\n");

	if (!S3I2CReadSerialData(S3I2C_TX_CTRL_ADDR, S3I2C_TX_CTRL_ALARM, 2))
	{
		fprintf(fid, "%03d: Alarm1: 0x%02x: ",
				S3I2C_TX_CTRL_MON + 14, *(S3I2CTxReadBuf + 0));

		S3PrintByte(fid, *(S3I2CTxReadBuf + 0));

		fprintf(fid, "%03d: Alarm2: 0x%02x: ",
				S3I2C_TX_CTRL_MON + 15, *(S3I2CTxReadBuf + 1));

		S3PrintByte(fid, *(S3I2CTxReadBuf + 1));
	}

	fprintf(fid, "\n\n============================================================\n");
	fprintf(fid, "Le Fin\n============================================================\n");

	fclose(fid);

	return 0;
}

// ----------------------------------------------------------------------------

#define S3_TLOG_FILENAME "TLog.csv"

int S3I2CTempLogStart()
{
	return 0;
}

int S3I2CTempLog(char Rx, char Tx)
{
	if (S3Data->wSecond == 0)
	{
		FILE *fid;

		int err = fopen_s(&fid, S3_TLOG_FILENAME, "a");
		if (!err)
		{
			fprintf(fid, "%d,%d,%d,%d\n",
				S3Data->wMinute,
				S3TxGetTemp(Rx, Tx),
				S3TxGetTempTEC(Rx, Tx) / 256,
				S3TxGetBattTemp(Rx, Tx));
			fclose(fid);

			return err;
		}
	}

	return 0;
}

// ----------------------------------------------------------------------------