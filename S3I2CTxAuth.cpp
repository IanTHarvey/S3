// ----------------------------------------------------------------------------

#include "stdafx.h"
#include "S3DataModel.h"

extern pS3DataModel S3Data;

#ifdef TRIZEPS
#include "drvLib_app.h"
#endif

#include "S3I2C.h"
#include "S3Gain.h"

#define S3_FLASH_BLOCK_SIZE	32

extern int S3GenerateChallenge(BYTE *chal);
extern int HMAC3(unsigned char *Digest, unsigned char *Message, unsigned char *Key);

extern int	HexStr2Hex(BYTE *buf, char *str);
extern char	S3BattAuthKeyStr[];
extern BYTE	S3BattAuthKey[];

// ----------------------------------------------------------------------------

int S3I2CTxAuthenticate(char Rx, char Tx)
{
	int err = 0;
	BYTE ErrCnt = 0;

#ifdef TRIZEPS
	BYTE i;

	for(i = 0; i < S3_SERIAL_FIFO_LEN; i++)
		S3I2CTxReadBuf[i] = 0;

	BYTE	Challenge[SHA1_DIGEST_LEN];		// Random challenge
	BYTE	ChallengeRev[SHA1_DIGEST_LEN];	// Applied in reverse byte-order
	BYTE	Digest[SHA1_DIGEST_LEN];		// Expected response

	S3GenerateChallenge(Challenge);

	for(i = 0; i < SHA1_DIGEST_LEN; i++)
		ChallengeRev[i] = Challenge[SHA1_DIGEST_LEN - i - 1];

	err = HMAC3(Digest, Challenge, S3BattAuthKey);

	// BlockDataControl	
	err = S3I2CWriteSerialByte(S3I2C_TX_BATT_ADDR, 0x61, 0x01);
	if (err)
	{	err = 10; goto ERR;	}

	// DataFlashClass 0x70: 'Security' block data
	err = S3I2CWriteSerialByte(S3I2C_TX_BATT_ADDR, 0x3E, 0x70);
	if (err)
	{	err = 20; goto ERR; }

	// DataFlashBlock 0: Offset
	err = S3I2CWriteSerialByte(S3I2C_TX_BATT_ADDR, 0x3F, 0x00);
	if (err)
	{ err = 30; goto ERR; }

	// Calculate checksum
	unsigned char CheckSum = 0;
	for(i = 0; i < SHA1_DIGEST_LEN; i++)
		CheckSum += ChallengeRev[i];

	CheckSum = 0xFF - CheckSum;

	// Split into 2 blocks - doesn't transfer correctly otherwise
	err = S3I2CWriteSerialData(S3I2C_TX_BATT_ADDR, 0x40, ChallengeRev, SHA1_DIGEST_LEN - 10);
	if (err)
	{	err = 40; goto ERR;	}

	Sleep(50);

	err = S3I2CWriteSerialData(S3I2C_TX_BATT_ADDR, 0x40 + 10,
		ChallengeRev + 10, SHA1_DIGEST_LEN - 10);
	if (err)
	{	err = 40; goto ERR; }

	// Set authenticate checksum for battery's SHA1-HMAC response
	err = S3I2CWriteSerialByte(S3I2C_TX_BATT_ADDR, 0x54, CheckSum);
	if (err)
	{	err = 50; goto ERR;	}

	Sleep(50);

	// Read back response digest
	err = S3I2CReadSerialData(S3I2C_TX_BATT_ADDR, 0x40, SHA1_DIGEST_LEN);
	if (err)
	{	err = 60; goto ERR; }

	BYTE	tmp[SHA1_DIGEST_LEN];
	for(i = 0; i < SHA1_DIGEST_LEN; i++)
		tmp[i] = S3I2CTxReadBuf[SHA1_DIGEST_LEN - i - 1];
	
	for(i = 0; i < SHA1_DIGEST_LEN; i++)
	{
		if (Digest[i] != tmp[i]) // S3I2CTxReadBuf[SHA1_DIGEST_LEN - i - 1])
			ErrCnt++;
	}

ERR:
#endif


	if (err)
	{
		S3EventLogAdd("TxBattAuth: Failed comms", 3, Rx, Tx, -1);
		return err;
	}
	else if (ErrCnt)
	{
		S3EventLogAdd("TxBattAuth: Failed authentication", 3, Rx, Tx, -1);
		return ErrCnt + 100;
	}
	else
		return err;
}

// ----------------------------------------------------------------------------
