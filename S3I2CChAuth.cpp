// ----------------------------------------------------------------------------

#include <windows.h>

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

int HexStr2Hex(BYTE *buf, char *str);
BYTE hextobyte(const char *pHex);

char MessageStr[2 * SHA1_DIGEST_LEN + 1] =
		{"1234560000000000000000000000000000000000"};
char KeyStr[2 * SHA1_KEY_LEN + 1] =
		{"10000000000000000000000000000000"};

int S3GenerateChallenge(BYTE *chal)
{
	srand(GetTickCount());
	unsigned short n;

	for(BYTE i = 0; i < 10; i++)
	{
		n = (unsigned short)rand();
		*(unsigned short *)(chal + i * 2) = n;
	}

	return 0;
}

// ----------------------------------------------------------------------------
// Write 16-byte authentication key to battery.

int S3I2CChWriteAuthKey()
{
#ifdef TRIZEPS
	unsigned char cmd[3] = {0x00, 0x00, 0x00};
	unsigned char cmdSec[S3_FLASH_BLOCK_SIZE + 1];
	BOOL ok;
	unsigned char i2cStartAddr = 0x00;
	unsigned char i2cStdBufRead[S3_FLASH_BLOCK_SIZE];
	unsigned char AuthKey[SHA1_KEY_LEN];

	HexStr2Hex(AuthKey, KeyStr);

	cmd[0] = 0x61;	// BlockDataControl
	cmd[1] = 0x00;	// Enable authentication commands
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

	if (!ok)
		return 1;

	// Get 'Security' block data
	cmd[0] = 0x3e;	// DataFlashClass
	cmd[1] = 0x70;	// Subclass 'Security'
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

	if (!ok)
		return 2;

	cmd[0] = 0x3f;	// DataFlashBlock
	cmd[1] = 0x00;	// Offset bank: 0: '0-31', 1: 32-63

	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

	if (!ok)
		return 3;

	// Read and copy old data block
	i2cStartAddr = 0x40;
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR,
				&i2cStartAddr,	1,
				i2cStdBufRead,	S3_FLASH_BLOCK_SIZE);

	if (!ok)
		return 4;

	// Build write command and data
	unsigned char i;
	for(i = 0; i < S3_FLASH_BLOCK_SIZE + 1; i++)
		cmdSec[i] = 0;

	cmdSec[0] = 0x40; // Write data to offset 0x40 + offset

	for(i = 0; i < S3_FLASH_BLOCK_SIZE; i++)
		cmdSec[i + 1] = i2cStdBufRead[i];

	for(i = 0; i < SHA1_KEY_LEN; i++)
		cmdSec[i + 1 + 0x08] = AuthKey[i];

	// Copy data then write to flash

	// Calculate checksum for the WHOLE block
	unsigned char CheckSum = 0;
	for(unsigned char i = 1; i < S3_FLASH_BLOCK_SIZE + 1; i++)
		CheckSum += cmdSec[i];

	CheckSum = 0xFF - CheckSum;

	// Write the WHOLE data block
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmdSec, S3_FLASH_BLOCK_SIZE + 1, NULL, 0);

	// Set checksum to force transfer to data flash
	cmd[0] = 0x60;
	cmd[1] = CheckSum;
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

	if (!ok)
		return 5;

	// Do reset
	//cmd[0] = 0x00;
	//cmd[1] = 0x41;
	//cmd[2] = 0x00;
	//ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 3, NULL, 0);

	//if (!ok)
	//	return 1;

	cmd[0] = 0x61;	// BlockDataControl
	cmd[1] = 0x00;	// Enable authentication commands
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

	// Get 'Security' block data
	cmd[0] = 0x3e;	// DataFlashClass
	cmd[1] = 0x70;	// Subclass 'Security'
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

	// Read back new data
	i2cStartAddr = 0x40;
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR,
				&i2cStartAddr,	1,
				i2cStdBufRead,	S3_FLASH_BLOCK_SIZE);

	if (ok)
	{
		for(i = 0; i < S3_FLASH_BLOCK_SIZE; i++)
			if (i2cStdBufRead[i] != cmdSec[i + 1])
				return 6;
	}
	else return 7;

#endif

	return 0;
}

// ----------------------------------------------------------------------------
extern int HMAC(unsigned char *Digest, unsigned char *Message, char *Key);
extern int HMAC3(unsigned char *Digest, unsigned char *Message, unsigned char *Key);

int S3I2CChAuthenticate()
{
	BYTE ErrCnt = 0;

#ifdef TRIZEPS
	BYTE	Message[SHA1_DIGEST_LEN];
	BYTE	MessageTmp[SHA1_DIGEST_LEN];
	BYTE	Digest[SHA1_DIGEST_LEN];

	BYTE i;

	unsigned char AuthKey[SHA1_KEY_LEN];

	S3GenerateChallenge(MessageTmp);

	// HexStr2Hex(MessageTmp, MessageStr);
	HexStr2Hex(AuthKey, S3_BATT_SHA1_HMAC_KEY);

	for(i = 0; i < SHA1_DIGEST_LEN; i++)
	{
		Message[i] = MessageTmp[SHA1_DIGEST_LEN - i - 1];
	}

	unsigned char cmd[3] = {0x00, 0x00, 0x00};
	unsigned char cmdSec[SHA1_DIGEST_LEN + 1];
	BOOL ok;
	unsigned char i2cStartAddr = 0x00;
	unsigned char i2cStdBufRead[SHA1_DIGEST_LEN];
	// unsigned char i2cRead[S3_FLASH_BLOCK_SIZE];

	int err = HMAC3(Digest, MessageTmp, (unsigned char *)AuthKey);

	cmd[0] = 0x61;	// BlockDataControl
	cmd[1] = 0x01;	// Enable authentication commands (unsealed)
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

	// Get 'Security' block data
	cmd[0] = 0x3e;	// DataFlashClass
	cmd[1] = 0x70;	// Subclass 'Security'
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

	cmd[0] = 0x3f;	// DataFlashBlock
	cmd[1] = 0x00;	// Offset bank: 0: '0-31', 1: 32-63
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

	cmdSec[0] = 0x40; // Write data to offset 0x40
	for(i = 0; i < SHA1_DIGEST_LEN; i++)
		cmdSec[i + 1] = Message[i];

	// Calculate checksum
	unsigned char CheckSum = 0;
	for(unsigned char i = 0; i < SHA1_DIGEST_LEN; i++)
		CheckSum += cmdSec[i + 1];

	CheckSum = 0xFF - CheckSum;

	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmdSec, SHA1_DIGEST_LEN + 1, NULL, 0);

	// Set authenticate checksum for battery's SHA1-HMAC response
	cmd[0] = 0x54;
	cmd[1] = CheckSum;
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

	if (!ok)
		return 1;

	Sleep(100);

	// Read back response digest
	i2cStartAddr = 0x40;
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR,
				&i2cStartAddr,	1,
				i2cStdBufRead,	SHA1_DIGEST_LEN);

	

	for(i = 0; i < SHA1_DIGEST_LEN; i++)
	{
		if (Digest[i] != i2cStdBufRead[SHA1_DIGEST_LEN - i - 1])
			ErrCnt++;
	}
#endif
	return ErrCnt;
}

// ----------------------------------------------------------------------------
// This works for the TI example (see SLUA389A), but can't replicate on bq34z-100.
// Based on: https://github.com/Yubico/yubico-c-client

#include "sha.h"
#include "sha-private.h"

int SHA1Reset (SHA1Context * context);
int SHA1Input (SHA1Context * context,
	   const uint8_t * message_array, unsigned length);

/* Local Function Prototypes */
static void SHA1Finalize (SHA1Context * context, uint8_t Pad_Byte);
static void SHA1PadMessage (SHA1Context *, uint8_t Pad_Byte);
static void SHA1ProcessMessageBlock (SHA1Context *);

// #define BYTE unsigned char

int HMAC3(unsigned char *Digest, unsigned char *Message, unsigned char *Key)
{
	int i;

	SHA1Context context;
	SHA1Reset(&context);

	for(i = 0; i < SHA1_Message_Block_Size; i++)
		context.Message_Block[i] = 0x00;

	for(i = 0; i < 16; i++)
		context.Message_Block[i] = Key[i];
	
	for(i = 0; i < SHA1HashSize; i++)
		context.Message_Block[i + 16] = Message[i];

	context.Message_Block[16 + SHA1HashSize] = 0x80;
	context.Message_Block[SHA1_Message_Block_Size - 2] = 0x01;
	context.Message_Block[SHA1_Message_Block_Size - 1] = 0x20;

	SHA1ProcessMessageBlock(&context);

	// Copy intermediate has into new message and re-run SHA1
	for(i = 0; i < 5; i++)
	{
		for(uint8_t j = 0; j < 4; j++)
		{
			uint8_t *c = (uint8_t *)(&context.Intermediate_Hash[i]) + 4 - j - 1;
			context.Message_Block[i * 4 + j + 16] = *c;
		}
	}

	SHA1Reset(&context);
	SHA1ProcessMessageBlock(&context);

	// BYTE tmp[20];

	// Create digest from intermediate hash
	for(i = 0; i < 5; i++)
	{
		for(uint8_t j = 0; j < 4; j++)
		{
			uint8_t *c = (uint8_t *)(&context.Intermediate_Hash[i]) + 4 - j - 1;
			Digest[i * 4 + j] = *c;
		}
	}

	// Reverse
	//for(i = 0; i < 20; i++)
	//	Digest[i] = tmp[20 - i - 1];

	return 0;
}

// ----------------------------------------------------------------------------

#define SHA1_ROTL(bits,word) \
                (((word) << (bits)) | ((word) >> (32-(bits))))

/*
 * add "length" to the length
 */
static uint32_t addTemp;

#define SHA1AddLength(context, length)                     \
    (addTemp = (context)->Length_Low,                      \
     (context)->Corrupted =                                \
        (((context)->Length_Low += (length)) < addTemp) && \
        (++(context)->Length_High == 0) ? 1 : 0)

/*
 *  SHA1Reset
 *
 *  Description:
 *      This function will initialize the SHA1Context in preparation
 *      for computing a new SHA1 message digest.
 *
 *  Parameters:
 *      context: [in/out]
 *          The context to reset.
 *
 *  Returns:
 *      sha Error Code.
 *
 */
int SHA1Reset (SHA1Context * context)
{
  if (!context)
    return shaNull;

  context->Length_Low = 0;
  context->Length_High = 0;
  context->Message_Block_Index = 0;

  /* Initial Hash Values: FIPS-180-2 section 5.3.1 */
  context->Intermediate_Hash[0] = 0x67452301;
  context->Intermediate_Hash[1] = 0xEFCDAB89;
  context->Intermediate_Hash[2] = 0x98BADCFE;
  context->Intermediate_Hash[3] = 0x10325476;
  context->Intermediate_Hash[4] = 0xC3D2E1F0;

  context->Computed = 0;
  context->Corrupted = 0;

  return shaSuccess;
}

/*
 *  SHA1Input
 *
 *  Description:
 *      This function accepts an array of octets as the next portion
 *      of the message.
 *
 *  Parameters:
 *      context: [in/out]
 *          The SHA context to update
 *      message_array: [in]
 *          An array of characters representing the next portion of
 *          the message.
 *      length: [in]
 *          The length of the message in message_array
 *
 *  Returns:
 *      sha Error Code.
 *
 */
int SHA1Input (SHA1Context * context,
	   const uint8_t * message_array, unsigned length)
{
  if (!length)
    return shaSuccess;

  if (!context || !message_array)
    return shaNull;

  if (context->Computed)
    {
      context->Corrupted = shaStateError;
      return shaStateError;
    }

  if (context->Corrupted)
    return context->Corrupted;

  while (length-- && !context->Corrupted)
    {
      context->Message_Block[context->Message_Block_Index++] =
	(*message_array & 0xFF);

      if (!SHA1AddLength (context, 8) &&
	  (context->Message_Block_Index == SHA1_Message_Block_Size))
	SHA1ProcessMessageBlock (context);

      message_array++;
    }

  return shaSuccess;
}

/*
 * SHA1FinalBits
 *
 * Description:
 *   This function will add in any final bits of the message.
 *
 * Parameters:
 *   context: [in/out]
 *     The SHA context to update
 *   message_bits: [in]
 *     The final bits of the message, in the upper portion of the
 *     byte. (Use 0b###00000 instead of 0b00000### to input the
 *     three bits ###.)
 *   length: [in]
 *     The number of bits in message_bits, between 1 and 7.
 *
 * Returns:
 *   sha Error Code.
 */
int SHA1FinalBits (SHA1Context * context, const uint8_t message_bits,
	       unsigned int length)
{
  uint8_t masks[8] = {
    /* 0 0b00000000 */ 0x00, /* 1 0b10000000 */ 0x80,
    /* 2 0b11000000 */ 0xC0, /* 3 0b11100000 */ 0xE0,
    /* 4 0b11110000 */ 0xF0, /* 5 0b11111000 */ 0xF8,
    /* 6 0b11111100 */ 0xFC, /* 7 0b11111110 */ 0xFE
  };
  uint8_t markbit[8] = {
    /* 0 0b10000000 */ 0x80, /* 1 0b01000000 */ 0x40,
    /* 2 0b00100000 */ 0x20, /* 3 0b00010000 */ 0x10,
    /* 4 0b00001000 */ 0x08, /* 5 0b00000100 */ 0x04,
    /* 6 0b00000010 */ 0x02, /* 7 0b00000001 */ 0x01
  };

  if (!length)
    return shaSuccess;

  if (!context)
    return shaNull;

  if (context->Computed || (length >= 8) || (length == 0))
    {
      context->Corrupted = shaStateError;
      return shaStateError;
    }

  if (context->Corrupted)
    return context->Corrupted;

  SHA1AddLength (context, length);
  SHA1Finalize (context,
		(uint8_t) ((message_bits & masks[length]) | markbit[length]));

  return shaSuccess;
}

/*
 * SHA1Result
 *
 * Description:
 *   This function will return the 160-bit message digest into the
 *   Message_Digest array provided by the caller.
 *   NOTE: The first octet of hash is stored in the 0th element,
 *      the last octet of hash in the 19th element.
 *
 * Parameters:
 *   context: [in/out]
 *     The context to use to calculate the SHA-1 hash.
 *   Message_Digest: [out]
 *     Where the digest is returned.
 *
 * Returns:
 *   sha Error Code.
 *
 */
int SHA1Result (SHA1Context * context, uint8_t Message_Digest[SHA1HashSize])
{
  int i;

  if (!context || !Message_Digest)
    return shaNull;

  if (context->Corrupted)
    return context->Corrupted;

  if (!context->Computed)
    SHA1Finalize (context, 0x80);

  for (i = 0; i < SHA1HashSize; ++i)
    Message_Digest[i] = (uint8_t) (context->Intermediate_Hash[i >> 2]
				   >> 8 * (3 - (i & 0x03)));

  return shaSuccess;
}

/*
 * SHA1Finalize
 *
 * Description:
 *   This helper function finishes off the digest calculations.
 *
 * Parameters:
 *   context: [in/out]
 *     The SHA context to update
 *   Pad_Byte: [in]
 *     The last byte to add to the digest before the 0-padding
 *     and length. This will contain the last bits of the message
 *     followed by another single bit. If the message was an
 *     exact multiple of 8-bits long, Pad_Byte will be 0x80.
 *
 * Returns:
 *   sha Error Code.
 *
 */
static void SHA1Finalize (SHA1Context * context, uint8_t Pad_Byte)
{
  int i;
  SHA1PadMessage (context, Pad_Byte);
  /* message may be sensitive, clear it out */
  for (i = 0; i < SHA1_Message_Block_Size; ++i)
    context->Message_Block[i] = 0;
  context->Length_Low = 0;	/* and clear length */
  context->Length_High = 0;
  context->Computed = 1;
}

/*
 * SHA1PadMessage
 *
 * Description:
 *   According to the standard, the message must be padded to an
 *   even 512 bits. The first padding bit must be a '1'. The last
 *   64 bits represent the length of the original message. All bits
 *   in between should be 0. This helper function will pad the
 *   message according to those rules by filling the Message_Block
 *   array accordingly. When it returns, it can be assumed that the
 *   message digest has been computed.
 *
 * Parameters:
 *   context: [in/out]
 *     The context to pad
 *   Pad_Byte: [in]
 *     The last byte to add to the digest before the 0-padding
 *     and length. This will contain the last bits of the message
 *     followed by another single bit. If the message was an
 *     exact multiple of 8-bits long, Pad_Byte will be 0x80.
 *
 * Returns:
 *   Nothing.
 */
static void SHA1PadMessage (SHA1Context * context, uint8_t Pad_Byte)
{
  /*
   * Check to see if the current message block is too small to hold
   * the initial padding bits and length. If so, we will pad the
   * block, process it, and then continue padding into a second
   * block.
   */
  if (context->Message_Block_Index >= (SHA1_Message_Block_Size - 8))
    {
      context->Message_Block[context->Message_Block_Index++] = Pad_Byte;
      while (context->Message_Block_Index < SHA1_Message_Block_Size)
	context->Message_Block[context->Message_Block_Index++] = 0;

      SHA1ProcessMessageBlock (context);
    }
  else
    context->Message_Block[context->Message_Block_Index++] = Pad_Byte;

  while (context->Message_Block_Index < (SHA1_Message_Block_Size - 8))
    context->Message_Block[context->Message_Block_Index++] = 0;

  /*
   * Store the message length as the last 8 octets
   */
  context->Message_Block[56] = (uint8_t) (context->Length_High >> 24);
  context->Message_Block[57] = (uint8_t) (context->Length_High >> 16);
  context->Message_Block[58] = (uint8_t) (context->Length_High >> 8);
  context->Message_Block[59] = (uint8_t) (context->Length_High);
  context->Message_Block[60] = (uint8_t) (context->Length_Low >> 24);
  context->Message_Block[61] = (uint8_t) (context->Length_Low >> 16);
  context->Message_Block[62] = (uint8_t) (context->Length_Low >> 8);
  context->Message_Block[63] = (uint8_t) (context->Length_Low);

  SHA1ProcessMessageBlock (context);
}

/*
 * SHA1ProcessMessageBlock
 *
 * Description:
 *   This helper function will process the next 512 bits of the
 *   message stored in the Message_Block array.
 *
 * Parameters:
 *   None.
 *
 * Returns:
 *   Nothing.
 *
 * Comments:
 *   Many of the variable names in this code, especially the
 *   single character names, were used because those were the
 *   names used in the publication.
 */
static void SHA1ProcessMessageBlock (SHA1Context * context)
{
  /* Constants defined in FIPS-180-2, section 4.2.1 */
  const uint32_t K[4] = {
    0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6
  };
  int t;			/* Loop counter */
  uint32_t temp;		/* Temporary word value */
  uint32_t W[80];		/* Word sequence */
  uint32_t A, B, C, D, E;	/* Word buffers */

  /*
   * Initialize the first 16 words in the array W
   */
  for (t = 0; t < 16; t++)
    {
      W[t] = ((uint32_t) context->Message_Block[t * 4]) << 24;
      W[t] |= ((uint32_t) context->Message_Block[t * 4 + 1]) << 16;
      W[t] |= ((uint32_t) context->Message_Block[t * 4 + 2]) << 8;
      W[t] |= ((uint32_t) context->Message_Block[t * 4 + 3]);
    }
  for (t = 16; t < 80; t++)
    W[t] = SHA1_ROTL (1, W[t - 3] ^ W[t - 8] ^ W[t - 14] ^ W[t - 16]);

  A = context->Intermediate_Hash[0];
  B = context->Intermediate_Hash[1];
  C = context->Intermediate_Hash[2];
  D = context->Intermediate_Hash[3];
  E = context->Intermediate_Hash[4];

  for (t = 0; t < 20; t++)
    {
      temp = SHA1_ROTL (5, A) + SHA_Ch (B, C, D) + E + W[t] + K[0];
      E = D;
      D = C;
      C = SHA1_ROTL (30, B);
      B = A;
      A = temp;
    }

  for (t = 20; t < 40; t++)
    {
      temp = SHA1_ROTL (5, A) + SHA_Parity (B, C, D) + E + W[t] + K[1];
      E = D;
      D = C;
      C = SHA1_ROTL (30, B);
      B = A;
      A = temp;
    }

  for (t = 40; t < 60; t++)
    {
      temp = SHA1_ROTL (5, A) + SHA_Maj (B, C, D) + E + W[t] + K[2];
      E = D;
      D = C;
      C = SHA1_ROTL (30, B);
      B = A;
      A = temp;
    }

  for (t = 60; t < 80; t++)
    {
      temp = SHA1_ROTL (5, A) + SHA_Parity (B, C, D) + E + W[t] + K[3];
      E = D;
      D = C;
      C = SHA1_ROTL (30, B);
      B = A;
      A = temp;
    }

  context->Intermediate_Hash[0] += A;
  context->Intermediate_Hash[1] += B;
  context->Intermediate_Hash[2] += C;
  context->Intermediate_Hash[3] += D;
  context->Intermediate_Hash[4] += E;

  context->Message_Block_Index = 0;
}

// ----------------------------------------------------------------------------

#include "Wincrypt.h"

HCRYPTPROV hCryptProv;
HCRYPTHASH hHash;

int CryptTest()
{
	if(CryptAcquireContext(
		&hCryptProv, 
		NULL, 
		NULL, 
		PROV_RSA_FULL, 
		0)) 
	{
		printf("CryptAcquireContext complete. \n");
	}
	else
	{
		 printf("Acquisition of context failed.\n");
		exit(1);
	}
	//--------------------------------------------------------------------
	// Acquire a hash object handle.

	if(CryptCreateHash(
	   hCryptProv, 
	   CALG_MD5, 
	   0, 
	   0, 
	   &hHash)) 
	{
		printf("An empty hash object has been created. \n");
	}
	else
	{
		printf("Error during CryptBeginHash!\n");
	}

	if (hHash) 
		CryptDestroyHash(hHash);
	
	if (hCryptProv) 
		CryptReleaseContext(hCryptProv, 0);

	return 0;
}

// ---------------------------------------------------------------------------
// From MS - doesn't work as expected

int S3HMAC()
{
	//--------------------------------------------------------------------
	// Declare variables.
	//
	// hProv:           Handle to a cryptographic service provider (CSP). 
	//                  This example retrieves the default provider for  
	//                  the PROV_RSA_FULL provider type.  
	// hHash:           Handle to the hash object needed to create a hash.
	// hKey:            Handle to a symmetric key. This example creates a 
	//                  key for the RC4 algorithm.
	// hHmacHash:       Handle to an HMAC hash.
	// pbHash:          Pointer to the hash.
	// dwDataLen:       Length, in bytes, of the hash.
	// Data1:           Password string used to create a symmetric key.
	// Data2:           Message string to be hashed.
	// HmacInfo:        Instance of an HMAC_INFO structure that contains 
	//                  information about the HMAC hash.
	// 
	HCRYPTPROV  hProv       = NULL;
	HCRYPTHASH  hHash       = NULL;
	HCRYPTKEY   hKey        = NULL;
	HCRYPTHASH  hHmacHash   = NULL;
	PBYTE       pbHash      = NULL;
	DWORD       dwDataLen   = 0;
	// BYTE        Data1[]     = {0x70,0x61,0x73,0x73,0x77,0x6F,0x72,0x64};
	// BYTE        Data2[]     = {0x6D,0x65,0x73,0x73,0x61,0x67,0x65};
	HMAC_INFO   HmacInfo;


	// char key[] = "0x0123456789ABCDEFFEDCBA987654321"; // Plain text key
	char Key[] = "key";
	int KeyLen = strlen(Key);
	char Message[] = "The quick brown fox jumps over the lazy dog";
	int MsgLen = strlen(Message);

	//--------------------------------------------------------------------
	// Zero the HMAC_INFO structure and use the SHA1 algorithm for
	// hashing.

	ZeroMemory(&HmacInfo, sizeof(HmacInfo));
	HmacInfo.HashAlgid = CALG_SHA1;

	//--------------------------------------------------------------------
	// Acquire a handle to the default RSA cryptographic service provider.

	if (!CryptAcquireContext(
			&hProv,                   // handle of the CSP
			NULL,                     // key container name
			NULL,                     // CSP name
			PROV_RSA_FULL,            // provider type
			CRYPT_VERIFYCONTEXT))     // no key access is requested
	{
	   printf(" Error in AcquireContext 0x%08x \n",
			  GetLastError());
	   goto ErrorExit;
	}

	//--------------------------------------------------------------------
	// Derive a symmetric key from a hash object by performing the
	// following steps:
	//    1. Call CryptCreateHash to retrieve a handle to a hash object.
	//    2. Call CryptHashData to add a text string (password) to the 
	//       hash object.
	//    3. Call CryptDeriveKey to create the symmetric key from the
	//       hashed password derived in step 2.
	// You will use the key later to create an HMAC hash object. 

	if (!CryptCreateHash(
			hProv,                    // handle of the CSP
			CALG_SHA1,                // hash algorithm to use
			0,                        // hash key
			0,                        // reserved
			&hHash))                  // address of hash object handle
	{
	   printf("Error in CryptCreateHash 0x%08x \n",
			  GetLastError());
	   goto ErrorExit;
	}

	if (!CryptHashData(
			hHash,                    // handle of the hash object
			(BYTE *)Key,                    // password to hash
			KeyLen,            // number of bytes of data to add
			0))                       // flags
	{
	   printf("Error in CryptHashData 0x%08x \n", 
			  GetLastError());
	   goto ErrorExit;
	}

	if (!CryptDeriveKey(
			hProv,                    // handle of the CSP
			CALG_3DES,                 // algorithm ID
			hHash,                    // handle to the hash object
			0,                        // flags
			&hKey))                   // address of the key handle
	{
	   printf("Error in CryptDeriveKey 0x%08x \n", 
			  GetLastError());
	   goto ErrorExit;
	}

	//--------------------------------------------------------------------
	// Create an HMAC by performing the following steps:
	//    1. Call CryptCreateHash to create a hash object and retrieve 
	//       a handle to it.
	//    2. Call CryptSetHashParam to set the instance of the HMAC_INFO 
	//       structure into the hash object.
	//    3. Call CryptHashData to compute a hash of the message.
	//    4. Call CryptGetHashParam to retrieve the size, in bytes, of
	//       the hash.
	//    5. Call malloc to allocate memory for the hash.
	//    6. Call CryptGetHashParam again to retrieve the HMAC hash.

	if (!CryptCreateHash(
			hProv,                    // handle of the CSP.
			CALG_HMAC,                // HMAC hash algorithm ID
			hKey,                     // key for the hash (see above)
			0,                        // reserved
			&hHmacHash))              // address of the hash handle
	{
	   printf("Error in CryptCreateHash 0x%08x \n", 
			  GetLastError());
	   goto ErrorExit;
	}

	if (!CryptSetHashParam(
			hHmacHash,                // handle of the HMAC hash object
			HP_HMAC_INFO,             // setting an HMAC_INFO object
			(BYTE*)&HmacInfo,         // the HMAC_INFO object
			0))                       // reserved
	{
	   printf("Error in CryptSetHashParam 0x%08x \n", 
			  GetLastError());
	   goto ErrorExit;
	}

	if (!CryptHashData(
			hHmacHash,                // handle of the HMAC hash object
			(BYTE *)Message,          // message to hash
			MsgLen,                   // number of bytes of data to add
			0))                       // flags
	{
	   printf("Error in CryptHashData 0x%08x \n", 
			  GetLastError());
	   goto ErrorExit;
	}

	//--------------------------------------------------------------------
	// Call CryptGetHashParam twice. Call it the first time to retrieve
	// the size, in bytes, of the hash. Allocate memory. Then call 
	// CryptGetHashParam again to retrieve the hash value.

	if (!CryptGetHashParam(
			hHmacHash,                // handle of the HMAC hash object
			HP_HASHVAL,               // query on the hash value
			NULL,                     // filled on second call
			&dwDataLen,               // length, in bytes, of the hash
			0))
	{
	   printf("Error in CryptGetHashParam 0x%08x \n", 
			  GetLastError());
	   goto ErrorExit;
	}

	pbHash = (BYTE*)malloc(dwDataLen);
	if(NULL == pbHash) 
	{
	   printf("unable to allocate memory\n");
	   goto ErrorExit;
	}
	    
	if (!CryptGetHashParam(
			hHmacHash,                 // handle of the HMAC hash object
			HP_HASHVAL,                // query on the hash value
			pbHash,                    // pointer to the HMAC hash value
			&dwDataLen,                // length, in bytes, of the hash
			0))
	{
	   printf("Error in CryptGetHashParam 0x%08x \n", GetLastError());
	   goto ErrorExit;
	}

	// Print the hash to the console.

	printf("The hash is:  ");
	for(DWORD i = 0 ; i < dwDataLen ; i++) 
	{
	   printf("%2.2x ",pbHash[i]);
	}
	printf("\n");

	// Free resources.
	ErrorExit:
		if(hHmacHash)
			CryptDestroyHash(hHmacHash);
		if(hKey)
			CryptDestroyKey(hKey);
		if(hHash)
			CryptDestroyHash(hHash);    
		if(hProv)
			CryptReleaseContext(hProv, 0);
		if(pbHash)
			free(pbHash);
    return 0;
}

// ----------------------------------------------------------------------------

#ifndef CALG_HMAC
#define CALG_HMAC (ALG_CLASS_HASH | ALG_TYPE_ANY | ALG_SID_HMAC)
#endif

#ifndef CRYPT_IPSEC_HMAC_KEY
#define CRYPT_IPSEC_HMAC_KEY 0x00000100
#endif

// #pragma comment(lib, "crypt32.lib")

int HMAC(unsigned char *Digest, unsigned char *Message, char * password);


typedef struct _my_blob
{
	BLOBHEADER header;
	DWORD len;
	BYTE key[100];
} my_blob;

// ----------------------------------------------------------------------------

int S3HMAC2()
{
	BYTE	Message[SHA1_DIGEST_LEN + 1]; // = "0123456789fedcba0123";

	BYTE	Digest[SHA1_DIGEST_LEN];
	char	Key[] = "0123456789fedcba";

	// for(unsigned char i = 0; i < 16; i++) Key[i] = 0;

	int err = HMAC(Digest, Message, Key);
 
	return 0;
}

// ----------------------------------------------------------------------------
// This does work as standard SHA-1 HMAC, but not as TI implementation works.

// See: http://masm32.com/board/index.php?topic=1612.0
int HMAC(unsigned char *Digest, unsigned char *Message, char *password)
{
	DWORD AlgId = CALG_SHA1;
	HCRYPTPROV  hProv = 0;
	HCRYPTHASH  hHash = 0;
	HCRYPTKEY   hKey = 0;
	HCRYPTHASH  hHmacHash = 0;
	BYTE		   *pbHash = 0;
	DWORD       dwDataLen = 0;
	HMAC_INFO   HmacInfo;

	int err = 0;

	ZeroMemory(&HmacInfo, sizeof(HmacInfo));

	HmacInfo.HashAlgid = CALG_SHA1;
	// pbHash = new BYTE[20];
	dwDataLen = SHA1_DIGEST_LEN;
	 
	my_blob *kb = NULL;
	DWORD kbSize = sizeof(my_blob) + strlen(password);

	kb = (my_blob *)malloc(kbSize);
	kb->header.bType = PLAINTEXTKEYBLOB;
	kb->header.bVersion = CUR_BLOB_VERSION;
	kb->header.reserved = 0;
	kb->header.aiKeyAlg = CALG_RC2;
	memcpy(&kb->key, password, strlen(password));
	kb->len = strlen(password);

	if (!CryptAcquireContext(&hProv, NULL, MS_ENHANCED_PROV, PROV_RSA_FULL,
	   CRYPT_VERIFYCONTEXT | CRYPT_NEWKEYSET))
	{
		err = 1;
		goto Exit;
	}

		//if (!CryptGenRandom(hProv, SHA1_DIGEST_LEN, Message))
		//{
		//	err = 1;
		//	goto Exit;
		//}

	// for(unsigned char i = 0; i < SHA1_DIGEST_LEN; i++)
	//	Message[i] = 'a' + i;

	if (!CryptImportKey(hProv,  (BYTE *)kb, kbSize, 0, CRYPT_IPSEC_HMAC_KEY, &hKey))
	{
		err = 1;
		goto Exit;
	}

	if (!CryptCreateHash(hProv, CALG_HMAC, hKey, 0, &hHmacHash))
	{
	   err = 1;
	   goto Exit;
	}

	if (!CryptSetHashParam(hHmacHash, HP_HMAC_INFO, (BYTE*)&HmacInfo, 0))
	{
	   err = 1;
	   goto Exit;
	}

	if (!CryptHashData(hHmacHash, (BYTE*)Message, dwDataLen, 0))
	{
	   err = 1;
	   goto Exit;
	}

	if (!CryptGetHashParam(hHmacHash, HP_HASHVAL, Digest, &dwDataLen, 0))
	{
	   err = 1;
	   goto Exit;
	}

	Exit:
	free(kb);
	if(hHmacHash)
		CryptDestroyHash(hHmacHash);
	if(hKey)
		CryptDestroyKey(hKey);
	if(hHash)
		CryptDestroyHash(hHash);   
	if(hProv)
		CryptReleaseContext(hProv, 0);

	return err;
}

// ----------------------------------------------------------------------------

int S3I2CChSetBattSealed(char Ch)
{
	BOOL ok = TRUE;

#ifdef TRIZEPS
	unsigned char cmd[3] = {0x00, 0x20, 0x00};

	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 3, NULL, 0);

	if (ok)
	{
		BOOL ok;
		unsigned char i2cCmdBufRead[2];
		unsigned char cmd[3] = {0, 0, 0};
		unsigned char i2cStartAddr = 0x00;

		ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 3, NULL, 0);
		ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, &i2cStartAddr, 1, i2cCmdBufRead, 2);
		S3ChSetBattStatus(Ch, i2cCmdBufRead);

		// Check FAS and Sealed bits set
		if ((S3ChGetBattStatus(Ch) & (BQ_SS | BQ_FAS)) == (BQ_SS | BQ_FAS))
			return 1;

		return 0;
	}

#endif
	return (int)(ok != TRUE);
}

// ----------------------------------------------------------------------------
// #define S3_BATT_UNSEAL_KEY			"67D8FF9A"	// Unseal code

int S3I2CChSetBattUnseal()
{
	BOOL ok = TRUE;

#ifdef TRIZEPS

	// BQ34Z100 defaults
	// unsigned char cmd1[3] = {0x00, 0x14, 0x04};
	// unsigned char cmd2[3] = {0x00, 0x72, 0x36};

	unsigned char cmd1[3] = {0x00, 0x00, 0x00};
	unsigned char cmd2[3] = {0x00, 0x00, 0x00};

	BYTE buf[4];
	HexStr2Hex(buf, S3_BATT_UNSEAL_KEY);

	cmd1[1] = buf[1];
	cmd1[2] = buf[0];

	cmd2[1] = buf[3];
	cmd2[2] = buf[2];

	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd2, 3, NULL, 0);
	if (ok)
		ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd1, 3, NULL, 0);
#endif
	return (int)(ok != TRUE);
}

// ----------------------------------------------------------------------------
// #define S3_BATT_FAS_KEY				"1DD34FF2"	// Full access

int S3I2CChSetBattFullAccess()
{
	BOOL ok = TRUE;

#ifdef TRIZEPS

	// BQ34Z100 defaults
	// unsigned char cmd1[3] = {0x00, 0xFF, 0xFF};
	// unsigned char cmd2[3] = {0x00, 0xFF, 0xFF};

	unsigned char cmd1[3] = {0x00, 0x00, 0x00};
	unsigned char cmd2[3] = {0x00, 0x00, 0x00};

	BYTE buf[4];
	HexStr2Hex(buf, S3_BATT_FAS_KEY);

	cmd1[1] = buf[1];
	cmd1[2] = buf[0];

	cmd2[1] = buf[3];
	cmd2[2] = buf[2];

	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd2, 3, NULL, 0);
	if (ok)
		ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd1, 3, NULL, 0);

#endif

	return (int)(ok != TRUE);
}

// ----------------------------------------------------------------------------

int S3I2CChReadSecKeys()
{
#ifdef TRIZEPS
	unsigned char cmd[3] = {0x00, 0x00, 0x00};
	BOOL ok;
	unsigned char i2cStartAddr = 0x00;
	unsigned char i2cStdBufRead[S3_FLASH_BLOCK_SIZE];

	// Get 'Security' block data
	if (1)
	{
		cmd[0] = 0x3e;	// DataFlashClass
		cmd[1] = 112;	// Subclass 'Security'
		ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

		if (!ok)
			return 1;
	}

	cmd[0] = 0x3f;	// DataFlashBlock
	if (1)
		cmd[1] = 0x00;	// Offset bank: 0: '0-31', 1: 32-63
	else
		cmd[1] = 0x01;	// Offset bank: 0: '0-31', 1: 32-63

	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

	if (!ok)
		return 1;

	i2cStartAddr = 0x40;
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR,
				&i2cStartAddr,	1,
				i2cStdBufRead,	8);

	if (!ok)
		return 1;

#endif

	return 0;
}
// ----------------------------------------------------------------------------
// Assumes charger set up and battery has full access

int S3I2CChWriteSecKeys()
{
#ifdef TRIZEPS
	unsigned char cmd[3] = {0x00, 0x00, 0x00};
	unsigned char cmdSec[S3_FLASH_BLOCK_SIZE + 1];
	unsigned char datablock[S3_FLASH_BLOCK_SIZE];
	BOOL ok;
	unsigned char i2cStartAddr = 0x00;
	unsigned char i2cStdBufRead[S3_FLASH_BLOCK_SIZE];

	unsigned char i;
	for(i = 0; i < S3_FLASH_BLOCK_SIZE + 1; i++)
		cmdSec[i] = 0;
	
	for(i = 0; i < S3_FLASH_BLOCK_SIZE + 1; i++)
		datablock[i] = 0;

	cmd[0] = 0x61;	// BlockDataControl
	cmd[1] = 0x00;	// Enable authentication commands
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

	if (!ok)
		return 1;

	// Get 'Security' block data
	cmd[0] = 0x3e;	// DataFlashClass
	cmd[1] = 112;	// Subclass 'Security'
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

	if (!ok)
		return 1;

	cmd[0] = 0x3f;	// DataFlashBlock
	cmd[1] = 0x00;	// Offset bank: 0: '0-31', 1: 32-63
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

	if (!ok)
		return 2;

	// Build write command and data
	cmdSec[0] = 0x40; // Write data to offset 0x40 + offset

	HexStr2Hex(datablock + 0, S3_BATT_UNSEAL_KEY);
	HexStr2Hex(datablock + 4, S3_BATT_FAS_KEY);
	HexStr2Hex(datablock + 8, S3_BATT_SHA1_HMAC_KEY);

	// Copy
	for(i = 0; i < S3_FLASH_BLOCK_SIZE; i++)
		cmdSec[i + 1] = datablock[i];

	// Copy data then write to flash
	// Calculate checksum for the WHOLE block
	unsigned char CheckSum = 0;
	for(unsigned char i = 1; i < S3_FLASH_BLOCK_SIZE + 1; i++)
		CheckSum += cmdSec[i];

	CheckSum = 0xFF - CheckSum;

	// Write the WHOLE data block
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmdSec, S3_FLASH_BLOCK_SIZE + 1, NULL, 0);
	if (!ok)
		return 3;

	// Set checksum to force transfer to data flash
	cmd[0] = 0x60;
	cmd[1] = CheckSum;
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);
	if (!ok)
		return 4;

	/*
	// Do reset
	cmd[0] = 0x00;
	cmd[1] = 0x41;
	cmd[2] = 0x00;
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 3, NULL, 0);

	if (!ok)
		return 5;


	cmd[0] = 0x61;	// BlockDataControl
	cmd[1] = 0x00;	// Enable authentication commands
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

	if (!ok)
		return 10;

	// Get 'Security' block data
	cmd[0] = 0x3e;	// DataFlashClass
	cmd[1] = 112;	// Subclass 'Security'
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

	if (!ok)
		return 11;

	cmd[0] = 0x3f;	// DataFlashBlock
	cmd[1] = 0x00;	// Offset bank: 0: '0-31', 1: 32-63
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

	if (!ok)
		return 12;
	*/

	Sleep(100);

	// Read back new data
	i2cStartAddr = 0x40;
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR,
				&i2cStartAddr,	1,
				i2cStdBufRead,	S3_FLASH_BLOCK_SIZE);

	if (ok)
	{
		for(i = 0; i < S3_FLASH_BLOCK_SIZE; i++)
		{
			if (i2cStdBufRead[i] != cmdSec[i + 1])
			{
				int err = 100 + i;
				return err;
			}
		}
	}
	else return 7;

	if (0)
	{
		// Seal the battery
		cmd[0] = 0x00; // Seal key
		cmd[1] = 0x20;
		cmd[2] = 0x00; 
		ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 3, NULL, 0);

		if (!ok)
			return 8;
	}

#endif

	return 0;
}


// ----------------------------------------------------------------------------

int HexStr2Hex(BYTE *buf, char *str)
{
	int len = strlen(str);
	int len_res = len / 2; 

	BYTE i;

	for(i = 0; i < len_res; i++)
	{
		char d[3];

		d[0] = str[i * 2];
		d[1] = str[i * 2 + 1];
		d[2] = '\0';

		buf[i] = hextobyte(d);
	}

	return 0;
}

// ----------------------------------------------------------------------------

 BYTE hextobyte(const char *pHex) 
 {  
   int result = 0; 
   char ch; 
 
   while (ch = *pHex++) {  
     result <<= 4; 
 
     if ((ch >= '0') && (ch <= '9')) result += ch - '0';  
     else if ((ch >= 'a') && (ch <= 'f')) result += ch - 'a' + 10;  
     else if ((ch >= 'A') && (ch <= 'F')) result += ch - 'A' + 10;  
     else break;  
   }  
   return (result); 
 }

 // ----------------------------------------------------------------------------

 void AuthDiag(const char *AuthKey, const char *Message,
	 const char *Challenge, const char *Digest)
 {
 	FILE *diag;
	int err = fopen_s(&diag, "auth_diag.txt", "w");
	BYTE	i;

	fprintf(diag, "\nKey:\t\t\t\t");
	for(i = 0; i < 16; i++)
		fprintf(diag, "%02x ", AuthKey[i]);

	fprintf(diag, "\nMessage:\t\t\t");

	for(i = 0; i < SHA1_DIGEST_LEN; i++)
		fprintf(diag, "%02x ", Message[i]);

	fprintf(diag, "\n\nLocal digest:\t\t");

	for(i = 0; i < SHA1_DIGEST_LEN; i++)
		fprintf(diag, "%02x ", Challenge[i]);

	fprintf(diag, "\nReturned digest:\t");

	for(i = 0; i < SHA1_DIGEST_LEN; i++)
		fprintf(diag, "%02x ", Digest[i]);
	
	fprintf(diag, "\n");
	
	fclose(diag);
 }

 // ----------------------------------------------------------------------------
