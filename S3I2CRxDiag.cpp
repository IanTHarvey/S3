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

// ----------------------------------------------------------------------------
// Rx diagnostic dumps

// Diag only
extern void S3PrintByte(FILE *fid, char x);

// ----------------------------------------------------------------------------

int S3I2CRxDumpOptConfig(char Rx)
{
#ifdef TRIZEPS
	char Filename[S3_MAX_TIME_STR_LEN], t[S3_MAX_TIME_STR_LEN];
	S3GetTimeStr(t);

	strcpy_s(Filename, S3_MAX_TIME_STR_LEN, t);

	Filename[4] = Filename[7] = Filename[10] = Filename[13] = Filename[16] = '_';

	char Path[S3_MAX_FILENAME_LEN];
	sprintf_s(Path, S3_MAX_FILENAME_LEN, "\\Flashdisk\\S3\\RxOptConfig_%x_%s.txt",
		S3I2CCurRxOptAddr, Filename);
	
	FILE	*fid;

	int err = fopen_s(&fid, Path, "w");

	if (err)
		return 1;

	char tmp[20];
	const char *buf = (char *)S3I2CRxReadBuf;
	unsigned char Start = 0;

	fprintf(fid, "\nRx Optical I2C Map: Rx %d: Address: 0x%x\n================================\n\n", Rx, S3I2CCurRxOptAddr);

	fprintf(fid, "\n%s\n", t);

	fprintf(fid, "\nApp: %s\n\n", S3SysGetAppDateTime());

	fprintf(fid, "\n=========================================================\n");
	fprintf(fid, "Identity\n");
	fprintf(fid, "=========================================================\n");

	unsigned char wbuf[3];
	wbuf[0] = S3I2C_RX_CTRL_IDENT;
	BOOL ok = I2C_WriteRead(S3I2CCurRxOptAddr, wbuf, 1, S3I2CRxReadBuf, 29);

	if (ok)
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

	// if (!S3I2CReadSerialData(S3I2CCurRxOptAddr, S3I2C_RX_OPT_IDENT + Start, 38))
	
	wbuf[0] = S3I2C_RX_CTRL_IDENT + Start;
	ok = I2C_WriteRead(S3I2CCurRxOptAddr, wbuf, 1, S3I2CRxReadBuf, 38);
	
	if (ok)
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

		fprintf(fid, "%03d: Type:\t\t0x%02X\n", Start, *(S3I2CRxReadBuf + S3I2C_PN_LEN + 2 + S3I2C_SN_LEN + 8));
	}
	
	fprintf(fid, "\n=========================================================\n");
	fprintf(fid, "Thresholds\n");
	fprintf(fid, "=========================================================\n");

	// if (!S3I2CReadSerialData(S3I2CCurRxOptAddr, S3I2C_RX_OPT_THRESH, 48))
	wbuf[0] = S3I2C_RX_OPT_THRESH;
	ok = I2C_WriteRead(S3I2CCurRxOptAddr, wbuf, 1, S3I2CRxReadBuf, 8 * 4);
	
	unsigned char i = 0;

	if (ok)
	{
		fprintf(fid, "%03d Vcc:\t\t%05d - %05d\n", S3I2C_RX_OPT_THRESH + i * 4,
			S3RevByteShort(S3I2CRxReadBuf + i * 4),
			S3RevByteShort(S3I2CRxReadBuf + i * 4 + 2)); i++;

		fprintf(fid, "%03d RLL:\t\t%05d - %05d\n", S3I2C_RX_OPT_THRESH + i * 4,
			S3RevByteShort(S3I2CRxReadBuf + i * 4),
			S3RevByteShort(S3I2CRxReadBuf + i * 4 + 2)); i++;

		fprintf(fid, "%03d RF level:\t%05d - %05d\n", S3I2C_RX_OPT_THRESH + i * 4,
			S3RevByteShort(S3I2CRxReadBuf + i * 4),
			S3RevByteShort(S3I2CRxReadBuf + i * 4 + 2)); i++;

		fprintf(fid, "%03d RF gain:\t%05d - %05d\n", S3I2C_RX_OPT_THRESH + i * 4,
			S3RevByteShort(S3I2CRxReadBuf + i * 4),
			S3RevByteShort(S3I2CRxReadBuf + i * 4 + 2)); i++;

		fprintf(fid, "%03d GainSoft:\t%05d - %05d\n", S3I2C_RX_OPT_THRESH + i * 4,
			S3RevByteShort(S3I2CRxReadBuf + i * 4),
			S3RevByteShort(S3I2CRxReadBuf + i * 4 + 2)); i++;

		fprintf(fid, "%03d AGCTgt:\t\t%05d - %05d\n", S3I2C_RX_OPT_THRESH + i * 4,
			S3RevByteShort(S3I2CRxReadBuf + i * 4),
			S3RevByteShort(S3I2CRxReadBuf + i * 4 + 2)); i++;

		fprintf(fid, "%03d BUC feed:\t%05d - %05d\n", S3I2C_RX_OPT_THRESH + i * 4,
			S3RevByteShort(S3I2CRxReadBuf + i * 4),
			S3RevByteShort(S3I2CRxReadBuf + i * 4 + 2)); i++;

		fprintf(fid, "%03d Temp:\t\t%05d - %05d\n", S3I2C_RX_OPT_THRESH + i * 4,
			S3RevByteShort(S3I2CRxReadBuf + i * 4) / 256,
			S3RevByteShort(S3I2CRxReadBuf + i * 4 + 2) / 256); i++;
	}	

	fprintf(fid, "\n=========================================================\n");
	fprintf(fid, "Config\n");
	fprintf(fid, "=========================================================\n");

	// if (!S3I2CReadSerialData(S3I2CCurRxOptAddr, S3I2C_RX_OPT_CONFIG, 48))
	wbuf[0] = S3I2C_RX_OPT_CONFIG;
	ok = I2C_WriteRead(S3I2CCurRxOptAddr, wbuf, 1, S3I2CRxReadBuf,48);

	if (ok)
	{
		fprintf(fid, "--------------------\nRaw: %03d-%03d\n--------------------\n",
			S3I2C_RX_OPT_CONFIG, S3I2C_RX_OPT_CONFIG + 48);

		for(i = 0; i < 47; i++)
			fprintf(fid, "%03d: %02x\n", i + S3I2C_RX_OPT_CONFIG, S3I2CRxReadBuf[i]);

		fprintf(fid, "\n");

		fprintf(fid, "%03d: (0xCC): ", S3I2C_RX_OPT_CONFIG + 44);
		S3PrintByte(fid, S3I2CRxReadBuf[44]);
		fprintf(fid, "%03d: (0xCD): ", S3I2C_RX_OPT_CONFIG + 45);
		S3PrintByte(fid, S3I2CRxReadBuf[45]);

		fprintf(fid, "\n");

		i = 0;

		fprintf(fid, "%03d: RLL:\t\t%.2fmV\n",
				i * 2 + S3I2C_RX_OPT_CONFIG, (double)S3RevByteShort(S3I2CRxReadBuf + i * 2) / 10.0); i++;

		fprintf(fid, "%03d: Gain Def:\t%.2fdB\n",
				i * 2 + S3I2C_RX_OPT_CONFIG, (double)S3RevByteShort(S3I2CRxReadBuf + i * 2) / 100.0); i++;

		fprintf(fid, "%03d: Gain set:\t%.2fdB\n",
				i * 2 + S3I2C_RX_OPT_CONFIG, (double)S3RevByteShort(S3I2CRxReadBuf + i * 2) / 100.0); i++;

		fprintf(fid, "%03d: AGC tgt:\t%.2fdBm\n",
				i * 2 + S3I2C_RX_OPT_CONFIG, (double)S3RevByteShort(S3I2CRxReadBuf + i * 2) / 100.0); i++;

		fprintf(fid, "%03d: RFGain cal:\t%.2fdB\n",
				i * 2 + S3I2C_RX_OPT_CONFIG, (double)S3RevByteShort(S3I2CRxReadBuf + i * 2) / 100.0); i++;
		
		fprintf(fid, "%03d: RFMon cal:\t%.2fdB\n",
				i * 2 + S3I2C_RX_OPT_CONFIG, (double)S3RevByteShort(S3I2CRxReadBuf + i * 2) / 100.0); i++;

		fprintf(fid, "%03d: FSK freq:\t%d\n",
				i * 2 + S3I2C_RX_OPT_CONFIG, S3RevByteShort(S3I2CRxReadBuf + i * 2)); i++;

		fprintf(fid, "%03d: FSK slice:\t%d\n",
				i * 2 + S3I2C_RX_OPT_CONFIG, S3RevByteShort(S3I2CRxReadBuf + i * 2)); i++;

		fprintf(fid, "%03d: RLL cal:\t%.2fdBm\n",
				i * 2 + S3I2C_RX_OPT_CONFIG, (double)S3RevByteShort(S3I2CRxReadBuf + i * 2) / 100.0); i++;

		fprintf(fid, "%03d: RFMon cal:\t%.2fmV\n",
				i * 2 + S3I2C_RX_OPT_CONFIG, (double)S3RevByteShort(S3I2CRxReadBuf + i * 2) / 10.0); i++;

		fprintf(fid, "%03d: T-10 cal:\t%d\n",
				i * 2 + S3I2C_RX_OPT_CONFIG, S3RevByteShort(S3I2CRxReadBuf + i * 2)); i++;

		fprintf(fid, "%03d: T+50 cal:\t%d\n",
				i * 2 + S3I2C_RX_OPT_CONFIG, S3RevByteShort(S3I2CRxReadBuf + i * 2)); i++;

		fprintf(fid, "%03d: MonMask:\t0x%02x: ",
				S3I2C_RX_OPT_CONFIG + 41, *(S3I2CRxReadBuf + 41));
		S3PrintByte(fid, *(S3I2CRxReadBuf + 41));

		fprintf(fid, "%03d: AlrmMask:\t0x%02x: ",
				S3I2C_RX_OPT_CONFIG + 43, *(S3I2CRxReadBuf + 43));
		S3PrintByte(fid, *(S3I2CRxReadBuf + 43));

		fprintf(fid, "%03d: RxCtrl:\t0x%02x: ",
				S3I2C_RX_OPT_CONFIG + 45, *(S3I2CRxReadBuf + 45));
		S3PrintByte(fid, *(S3I2CRxReadBuf + 45));
	}

	fprintf(fid, "\n=========================================================\n");
	fprintf(fid, "Monitors\n");
	fprintf(fid, "=========================================================\n");

	// if (!S3I2CReadSerialData(S3I2CCurRxOptAddr, S3I2C_RX_OPT_MON, 32))
	wbuf[0] = S3I2C_RX_OPT_MON;
	ok = I2C_WriteRead(S3I2CCurRxOptAddr, wbuf, 1, S3I2CRxReadBuf,48);

	if (ok)
	{
		i = 0;
		
		fprintf(fid, "%03d: Life:\t\t%d\n",
			i * 2 + S3I2C_RX_OPT_MON, S3RevByteShort(S3I2CRxReadBuf + i * 2)); i++;

		fprintf(fid, "%03d: Vcc:\t\t%dmV\n",
			i * 2 + S3I2C_RX_OPT_MON, S3RevByteShort(S3I2CRxReadBuf + i * 2)); i++;

		fprintf(fid, "%03d: RLL:\t\t%.1fdBm\n",
			i * 2 + S3I2C_RX_OPT_MON, (double)S3RevByteShort(S3I2CRxReadBuf + i * 2) / 100.0); i++;

		fprintf(fid, "%03d: RF:\t\t%.1fdBm\n",
			i * 2 + S3I2C_RX_OPT_MON, (double)S3RevByteShort(S3I2CRxReadBuf + i * 2) / 100.0); i++;

		fprintf(fid, "%03d: RFGain:\t%.1fdB\n",
			i * 2 + S3I2C_RX_OPT_MON, (double)S3RevByteShort(S3I2CRxReadBuf + i * 2) / 100.0); i++;

		fprintf(fid, "%03d: RFOP:\t\t%.1fdB\n",
			i * 2 + S3I2C_RX_OPT_MON, (double)S3RevByteShort(S3I2CRxReadBuf + i * 2) / 100.0); i++;
		
		fprintf(fid, "%03d: BUC:\t\t%dmV\n",
			i * 2 + S3I2C_RX_OPT_MON, S3RevByteShort(S3I2CRxReadBuf + i * 2)); i++;
		
		fprintf(fid, "%03d: Temp:\t\t%d degC\n",
			i * 2 + S3I2C_RX_OPT_MON, S3RevByteShort(S3I2CRxReadBuf + i * 2) / 256); i++;

		fprintf(fid, "%03d: TempADC:\t%d\n",
			i * 2 + S3I2C_RX_OPT_MON, S3RevByteShort(S3I2CRxReadBuf + i * 2)); i++;
		
		fprintf(fid, "%03d: RLLDAC:\t%dmV\n",
			i * 2 + S3I2C_RX_OPT_MON, S3RevByteShort(S3I2CRxReadBuf + i * 2)); i++;
	}


	fprintf(fid, "\n=========================================================\n");
	fprintf(fid, "Alarms\n");
	fprintf(fid, "=========================================================\n");

	// if (!S3I2CReadSerialData(S3I2CCurRxOptAddr, S3I2C_RX_OPT_ALARM, 3))
	
	wbuf[0] = S3I2C_RX_OPT_ALARMS;
	ok = I2C_WriteRead(S3I2CCurRxOptAddr, wbuf, 1, S3I2CRxReadBuf, 3);

	if (ok)
	{
		for(i = 0; i < 3; i++)
		{
			fprintf(fid, "%03d: 0x%02x: ",
					S3I2C_RX_OPT_ALARMS + i, *(S3I2CRxReadBuf + i));

			S3PrintByte(fid, *(S3I2CRxReadBuf + i));
		}
	}


	fprintf(fid, "\n=========================================================\n");
	fprintf(fid, "Status\n");
	fprintf(fid, "=========================================================\n");

	// if (!S3I2CReadSerialData(S3I2CCurRxOptAddr, S3I2C_RX_OPT_ALARM + 3, 2))
	wbuf[0] = S3I2C_RX_OPT_ALARMS + 3;
	ok = I2C_WriteRead(S3I2CCurRxOptAddr, wbuf, 1, S3I2CRxReadBuf, 1);
	
	if (ok)
	{
		fprintf(fid, "%03d: 0x%02x: ",
				S3I2C_RX_OPT_ALARMS + 3, *(S3I2CRxReadBuf));

		S3PrintByte(fid, *(S3I2CRxReadBuf + i));
	}

	fprintf(fid, "\n\n============================================================\n");
	fprintf(fid, "Le Fin\n============================================================\n");

	fclose(fid);

#endif

	return 0;
}


// ----------------------------------------------------------------------------

int S3I2CRxDumpCtrlConfig(char Rx)
{
#ifdef TRIZEPS

	char Filename[S3_MAX_TIME_STR_LEN], t[S3_MAX_TIME_STR_LEN];
	S3GetTimeStr(t);

	strcpy_s(Filename, S3_MAX_TIME_STR_LEN, t);

	Filename[4] = Filename[7] =	Filename[10] = Filename[13] = Filename[16] = '_';

	char Path[S3_MAX_FILENAME_LEN];
	sprintf_s(Path, S3_MAX_FILENAME_LEN, "\\Flashdisk\\S3\\RxCtrlConfig_%s.txt", Filename);
	
	FILE	*fid;

	int err = fopen_s(&fid, Path, "w");

	if (err)
		return 1;

	char tmp[20];
	const char *buf = (char *)S3I2CRxReadBuf;
	unsigned char Start = 0;

	fprintf(fid, "\nRX Control I2C Map: Rx %d\n================================\n", Rx);

	fprintf(fid, "\n%s\n", t);

	fprintf(fid, "\nApp: %s\n\n", S3SysGetAppDateTime());

	fprintf(fid, "\n=========================================================\n");
	fprintf(fid, "Identity\n");
	fprintf(fid, "=========================================================\n");

	unsigned char wbuf[3];
	wbuf[0] = S3I2C_RX_CTRL_IDENT;
	BOOL ok = I2C_WriteRead(S3I2C_RX_CTRL_ADDR, wbuf, 1, S3I2CRxReadBuf, 29);

	if (ok)
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

	// if (!S3I2CReadSerialData(S3I2C_RX_CTRL_ADDR, S3I2C_RX_CTRL_IDENT + Start, 37))

	wbuf[0] = S3I2C_RX_CTRL_IDENT + Start;
	ok = I2C_WriteRead(S3I2C_RX_CTRL_ADDR, wbuf, 1, S3I2CRxReadBuf, 37);

	if (ok)
	{
		strncpy_s(tmp, 20, buf + Start - Start2, S3I2C_PN_LEN);
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

	// if (!S3I2CReadSerialData(S3I2C_RX_CTRL_ADDR, S3I2C_RX_CTRL_IDENT + Start, 1))

	wbuf[0] = S3I2C_RX_CTRL_IDENT + Start;
	ok = I2C_WriteRead(S3I2C_RX_CTRL_ADDR, wbuf, 1, S3I2CRxReadBuf, 1);

	if (ok)
	{
		fprintf(fid, "%03d: Type: 0x%02X\n", Start, *(buf + Start - Start2));
		Start += 1;
	}

	fprintf(fid, "\n=========================================================\n");
	fprintf(fid, "Thresholds\n");
	fprintf(fid, "=========================================================\n");


	// if (!S3I2CReadSerialData(S3I2C_RX_CTRL_ADDR, S3I2C_RX_CTRL_THRESH, 20))
	wbuf[0] = S3I2C_RX_CTRL_THRESH;
	ok = I2C_WriteRead(S3I2C_RX_CTRL_ADDR, wbuf, 1, S3I2CRxReadBuf, 12);
	if (ok)
	{
		char i = 0;

		fprintf(fid, "%03d: Vcc:\t%d - %d\n", S3I2C_RX_CTRL_THRESH + i * 4,
				S3RevByteShort(S3I2CRxReadBuf + i * 4),
				S3RevByteShort(S3I2CRxReadBuf + i * 4 + 2)); i++;

		fprintf(fid, "%03d: TxMon:\t%d - %d\n", S3I2C_RX_CTRL_THRESH + i * 4,
				S3RevByteShort(S3I2CRxReadBuf + i * 4),
				S3RevByteShort(S3I2CRxReadBuf + i * 4 + 2)); i++;

		fprintf(fid, "%03d: RxMon:\t%d - %d\n", S3I2C_RX_CTRL_THRESH + i * 4,
				S3RevByteShort(S3I2CRxReadBuf + i * 4),
				S3RevByteShort(S3I2CRxReadBuf + i * 4 + 2)); i++;
	}


	fprintf(fid, "\n=========================================================\n");
	fprintf(fid, "Config\n");
	fprintf(fid, "=========================================================\n");

	// if (!S3I2CReadSerialData(S3I2C_RX_CTRL_ADDR, S3I2C_RX_CTRL_CONFIG, 24))

	wbuf[0] = S3I2C_RX_CTRL_CONFIG;
	ok = I2C_WriteRead(S3I2C_RX_CTRL_ADDR, wbuf, 1, S3I2CRxReadBuf, 48);

	if (ok)
	{
		fprintf(fid, "--------------------\nRaw: %03d-%03d\n--------------------\n",
			S3I2C_RX_CTRL_CONFIG, S3I2C_RX_CTRL_CONFIG + 48);

		for(char i = 0; i < 40; i++)
			fprintf(fid, "%03d: %02x\n", i + S3I2C_RX_CTRL_CONFIG, S3I2CRxReadBuf[i]);
		fprintf(fid, "--------------------\n\n");

		fprintf(fid, "\nTx Power Calc:\n");
		for(char i = 0; i < S3_MAX_TXS; i++)
			fprintf(fid, "%03d: %d\n",
				i + S3I2C_RX_CTRL_CONFIG, *(S3I2CRxReadBuf + i));
		fprintf(fid, "--------------------\n");

		fprintf(fid, "%03d: CalMode1:\t%d\n",
			6 + S3I2C_RX_CTRL_CONFIG, *(S3I2CRxReadBuf + 6));

		fprintf(fid, "%03d: DigPot.:\t%d\n",
			7 + S3I2C_RX_CTRL_CONFIG, *(S3I2CRxReadBuf + 7));

		fprintf(fid, "%03d: CalMode2:\t%d\n",
			45 + S3I2C_RX_CTRL_CONFIG, *(S3I2CRxReadBuf + 45));

		fprintf(fid, "%03d: OptSwPos:\t%d\n",
			46 + S3I2C_RX_CTRL_CONFIG, *(S3I2CRxReadBuf + 46));

		fprintf(fid, "%03d: LaserOff (0xCF):\t", 46 + S3I2C_RX_CTRL_CONFIG);
		S3PrintByte(fid, S3I2CRxReadBuf[47]);
	}


	fprintf(fid, "\n=========================================================\n");
	fprintf(fid, "Monitors\n");
	fprintf(fid, "=========================================================\n");

	// if (!S3I2CReadSerialData(S3I2C_RX_CTRL_ADDR, S3I2C_RX_CTRL_MON, 40))

	wbuf[0] = S3I2C_RX_CTRL_MON;
	ok = I2C_WriteRead(S3I2C_RX_CTRL_ADDR, wbuf, 1, S3I2CRxReadBuf, 4);

	if (ok)
	{
		fprintf(fid, "%03d: Life:\t%d\n",
					S3I2C_RX_CTRL_MON, S3RevByteShort(S3I2CRxReadBuf + 0));

		fprintf(fid, "%03d: Vcc:\t%dmV\n",
					S3I2C_RX_CTRL_MON + 2, S3RevByteShort(S3I2CRxReadBuf + 2));
	}


	// if (!S3I2CReadSerialData(S3I2C_RX_CTRL_ADDR, S3I2C_RX_CTRL_MON + 4, 24))
	wbuf[0] = S3I2C_RX_CTRL_MON + 4;
	ok = I2C_WriteRead(S3I2C_RX_CTRL_ADDR, wbuf, 1, S3I2CRxReadBuf, 24);

	if (ok)
	{
		for(unsigned char i = 0; i < S3_MAX_TXS; i++) 
		{
			fprintf(fid, "%03d: TxOpt %d:\t%duW\n",
					S3I2C_RX_CTRL_MON + 4 + 4 * i, i, S3RevByteShort(S3I2CRxReadBuf + 4 * i));

			fprintf(fid, "%03d: RxOpt %d:\t%duW\n\n",
					S3I2C_RX_CTRL_MON + 4 + 4 * i + 2, i, S3RevByteShort(S3I2CRxReadBuf + 4 * i + 2));
		}
	}

	// if (!S3I2CReadSerialData(S3I2C_RX_CTRL_ADDR, S3I2C_RX_CTRL_MON + 28, 24))
	wbuf[0] = S3I2C_RX_CTRL_MON + 28;
	ok = I2C_WriteRead(S3I2C_RX_CTRL_ADDR, wbuf, 1, S3I2CRxReadBuf, 6);

	if (ok)
	{
		for(unsigned char i = 0; i < S3_MAX_TXS; i++) 
		{
			fprintf(fid, "%03d: Bias %d:\t%d\n",
					S3I2C_RX_CTRL_MON + 28 + i, i, *(S3I2CRxReadBuf + i));
		}
	}

	fprintf(fid, "\n=========================================================\n");
	fprintf(fid, "Alarms\n");
	fprintf(fid, "=========================================================\n");

	// if (!S3I2CReadSerialData(S3I2C_RX_CTRL_ADDR, S3I2C_RX_CTRL_ALARMS, 3))
	wbuf[0] = S3I2C_RX_CTRL_ALARMS;
	ok = I2C_WriteRead(S3I2C_RX_CTRL_ADDR, wbuf, 1, S3I2CRxReadBuf, 3);

	if (ok)
	{
		fprintf(fid, "%03d: Alarm1: 0x%02x: ",
				S3I2C_RX_CTRL_ALARMS + 0, *(S3I2CRxReadBuf + 0));
		S3PrintByte(fid, *(S3I2CRxReadBuf + 0));

		fprintf(fid, "%03d: Alarm2: 0x%02x: ",
				S3I2C_RX_CTRL_ALARMS + 1, *(S3I2CRxReadBuf + 1));
		S3PrintByte(fid, *(S3I2CRxReadBuf + 1));

		fprintf(fid, "%03d: Alarm3: 0x%02x: ",
				S3I2C_RX_CTRL_ALARMS + 2, *(S3I2CRxReadBuf + 2));
		S3PrintByte(fid, *(S3I2CRxReadBuf + 2));
	}

	fprintf(fid, "\n\n============================================================\n");
	fprintf(fid, "Le Fin\n============================================================\n");

	fclose(fid);

#endif

	return 0;
}

// ----------------------------------------------------------------------------