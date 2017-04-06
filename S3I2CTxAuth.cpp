// ----------------------------------------------------------------------------

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "S3DataModel.h"

extern pS3DataModel S3Data;

#ifdef TRIZEPS
#include "drvLib_app.h"
#endif

#include "S3I2C.h"
#include "S3Gain.h"

#define S3_FLASH_BLOCK_SIZE	32

extern int HexStr2Hex(BYTE *buf, char *str);
extern int S3GenerateChallenge(BYTE *chal);
extern int HMAC3(unsigned char *Digest, unsigned char *Message, unsigned char *Key);

extern char MessageStr[];

extern int S3I2CWaitForEmpty();
extern unsigned char	S3I2CCurTxOptAddr;

int S3I2CTxAuthenticate()
{
	int err;
	BYTE i;

	for(i = 0; i < S3_SERIAL_FIFO_LEN; i++)
		S3I2CTxReadBuf[i] = 0;

// -----------
	if (0)
	{
		// BlockDataControl	
		err = S3I2CWriteSerialByte(S3I2C_TX_BATT_ADDR, 0x61, 0x01);
		// DataFlashClass 58: Manufacturer info
		err = S3I2CWriteSerialByte(S3I2C_TX_BATT_ADDR, 0x3E, 58);
		// DataFlashBlock 0: Offset
		err = S3I2CWriteSerialByte(S3I2C_TX_BATT_ADDR, 0x3F, 0x01);
		err = S3I2CReadSerialData(S3I2C_TX_BATT_ADDR, 0x40, 24);
	}
// -----------

	


	BYTE	Message[SHA1_DIGEST_LEN];
	BYTE	MessageTmp[SHA1_DIGEST_LEN];
	BYTE	Digest[SHA1_DIGEST_LEN];

	unsigned char AuthKey[16];
	HexStr2Hex(AuthKey, S3_BATT_SHA1_HMAC_KEY);

	S3GenerateChallenge(MessageTmp);

	HexStr2Hex(MessageTmp, MessageStr);



	for(i = 0; i < SHA1_DIGEST_LEN; i++)
		Message[i] = 6; // MessageTmp[SHA1_DIGEST_LEN - i - 1];

	err = S3I2CWriteSerialData(0xAA, 0x40, Message, SHA1_DIGEST_LEN);

	// err = HMAC3(Digest, MessageTmp, (unsigned char *)AuthKey);
	// Sleep(500);

	// BlockDataControl	
	err = S3I2CWriteSerialByte(S3I2C_TX_BATT_ADDR, 0x61, 0x01);
	if (err)
	{
		err = 10;
		goto ERR;
	}

	// DataFlashClass 0x70: 'Security' block data
	err = S3I2CWriteSerialByte(S3I2C_TX_BATT_ADDR, 0x3E, 0x70);
	if (err)
	{
		err = 20;
		goto ERR;
	}

	// DataFlashBlock 0: Offset
	err = S3I2CWriteSerialByte(S3I2C_TX_BATT_ADDR, 0x3F, 0x00);
	if (err)
	{
		err = 30;
		goto ERR;
	}

	// Calculate checksum
	unsigned char CheckSum = 0;
	for(i = 0; i < SHA1_DIGEST_LEN; i++)
		CheckSum += Message[i];

	CheckSum = 0xFF - CheckSum;



	if (0)
	{
		unsigned char cmd[30];
		cmd[0] = START;
		cmd[1] = S3I2C_CH_BATT_ADDR;
		cmd[2] = 0x40;
		cmd[3] = SHA1_DIGEST_LEN;

		for(i = 0; i < SHA1_DIGEST_LEN; i++)
			cmd[i + 4] = Message[i];

		cmd[i] = STOP;

		BOOL ok = I2C_WriteRead(S3I2CCurTxOptAddr, cmd, SHA1_DIGEST_LEN + 5, NULL, 0);

		if (!ok)
		{
			err = 40;
			goto ERR;
		}
	}
	else
	{
		err = S3I2CWriteSerialData(S3I2C_TX_BATT_ADDR, 0x40, Message, SHA1_DIGEST_LEN);

		if (err)
		{
			err = 40;
			goto ERR;
		}
	}

	// Sleep(10);

	// Set authenticate checksum for battery's SHA1-HMAC response
	err = S3I2CWriteSerialByte(S3I2C_TX_BATT_ADDR, 0x54, CheckSum);
	if (err)
	{
		err = 50;
		goto ERR;
	}

	// Sleep(2000);

	// Read back response digest
	err = S3I2CReadSerialData(S3I2C_TX_BATT_ADDR, 0x40, SHA1_DIGEST_LEN);
	if (err)
	{
		err = 60;
		goto ERR;
	}

	BYTE	tmp[SHA1_DIGEST_LEN];
	for(i = 0; i < SHA1_DIGEST_LEN; i++)
		tmp[i] = S3I2CTxReadBuf[SHA1_DIGEST_LEN - i - 1];

	BYTE ErrCnt = 0;
	
	for(i = 0; i < SHA1_DIGEST_LEN; i++)
	{
		if (Digest[i] != S3I2CTxReadBuf[SHA1_DIGEST_LEN - i - 1])
			ErrCnt++;
	}

ERR:
	return err;
}

// ----------------------------------------------------------------------------
