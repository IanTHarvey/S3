// ----------------------------------------------------------------------------
// Tx Comms over SC16IS740 I2C serial bridge functions.

#include "stdafx.h"

#ifdef TRIZEPS
#include "drvLib_app.h"
#endif

#include "S3DataModel.h"
#include "S3I2C.h"
#include "S3Gain.h"

#define S3_TX_RETRIES	1

#define SC16_FCR_ADDR		(0x02 << 3)
#define SC16_LSR_ADDR		(0x05 << 3)

extern unsigned short	S3RevByteUShort(unsigned char *b);
extern short			S3RevByteShort(	unsigned char *b);

extern unsigned char	S3I2CCurTxOptAddr;

int		S3I2CWaitFIFO();
void	S3I2CClearFIFO();

unsigned char S3I2CTxReadBuf[S3_SERIAL_FIFO_LEN]; // Read from optical serial link
unsigned char	*pS3I2CTxReadBuf;
int S3I2CReadSerialStatus();

// ----------------------------------------------------------------------------

int S3I2CWaitForEmpty()
{
#ifdef TRIZEPS
	unsigned char cnt = 0;
	int LSR;
	while(cnt++ < 100) // S3_POLL_SERIAL_RX)
	{
		LSR = I2C_ReadRandom(S3I2CCurTxOptAddr, SC16_LSR_ADDR);

		if (LSR & 0x40) // THR & TSR empty
			break;
	}

	if (cnt < S3_POLL_SERIAL_RX)
	{
		return 0;
	}
#endif
	return 1;
}

// ----------------------------------------------------------------------------
// Set up, send and wait to receive optical serial message to get NBytes
// starting at StartAddr from device at DevAddr
int S3I2CReadSerialData(unsigned char	DevAddr,
						unsigned char	StartAddr,
						unsigned char	NBytes)
{
	if (NBytes > S3_SERIAL_FIFO_LEN)
		NBytes = S3_SERIAL_FIFO_LEN;

#ifdef TRIZEPS

	if (S3I2CWaitForEmpty())
		return 1;

	char retries = 0;

	while(retries++ < S3_TX_RETRIES)
	{
		// Clearing the rx and tx FIFOs allows a clean restart. Without
		// this - it all ends in a bugger's muddle.
		// Most obviously manifested in the display of rubbish from the
		// tx battery.

		S3I2CClearFIFO();

		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, START);
		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, DevAddr);
		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 0x01);
		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, StartAddr);
		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, START);
		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, DevAddr + 1);
		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, NBytes);
		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, STOP);

		// S3I2CReadSerialStatus();

		unsigned char cnt = 0;
		int LSR;
		while(cnt++ < S3_POLL_SERIAL_RX)
		{
			LSR = I2C_ReadRandom(S3I2CCurTxOptAddr, SC16_LSR_ADDR);

			if (LSR & 0x01) // Data in receiver
				break;
		}

		if (cnt < S3_POLL_SERIAL_RX)
		{
			for(unsigned char i = 0; i < NBytes; i++)
				S3I2CTxReadBuf[i] = I2C_ReadRandom(S3I2CCurTxOptAddr, 0x00 << 3);

			return 0;
		}
	}

	return 1;
#else
	return 0;
#endif
}

// ----------------------------------------------------------------------------

int S3I2CReadSerialByte(unsigned char	DevAddr,
						unsigned char	DataAddr,
						unsigned char	*Data)
{
#ifdef TRIZEPS


	if (S3I2CWaitForEmpty())
		return 1;

	char retries = 0;

	while(retries++ < S3_TX_RETRIES)
	{
		// Clearing the rx and tx FIFOs allows a clean restart. Without
		// this - it all ends in a bugger's muddle.
		// Most obviously manifested in the display of rubbish from the
		// tx battery.

		S3I2CClearFIFO();

		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, START);
		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, DevAddr);
		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 0x01);
		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, DataAddr);
		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, START);
		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, DevAddr + 1);
		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 1);
		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, STOP);

		unsigned char cnt = 0;
		char LSR;
		while(cnt++ < S3_POLL_SERIAL_RX)
		{
			LSR = (char)I2C_ReadRandom(S3I2CCurTxOptAddr, SC16_LSR_ADDR);

			if (LSR & 0x01) // Data in receiver
				break;
		}

		if (cnt < S3_POLL_SERIAL_RX)
		{
			*Data = (char)I2C_ReadRandom(S3I2CCurTxOptAddr, 0x00 << 3);

			return 0;
		}
	}

	return 1;
#else
	return 0;
#endif
}

// ------------------------------------------------------------------------------

int S3I2CReadSerialShort(unsigned char	DevAddr,
						unsigned char	DataAddr,
						short			*Data)
{
#ifdef TRIZEPS

	if (S3I2CWaitForEmpty())
		return 1;

	char retries = 0;

	while(retries++ < S3_TX_RETRIES)
	{
		// Clearing the rx and tx FIFOs allows a clean restart. Without
		// this - it all ends in a bugger's muddle.
		// Most obviously manifested in the display of rubbish from the
		// tx battery.

		S3I2CClearFIFO();

		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, START);
		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, DevAddr);
		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 0x01);
		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, DataAddr);
		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, START);
		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, DevAddr + 1);
		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 2);
		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, STOP);

		unsigned char cnt = 0;
		int LSR;
		while(cnt++ < S3_POLL_SERIAL_RX)
		{
			LSR = I2C_ReadRandom(S3I2CCurTxOptAddr, SC16_LSR_ADDR);

			if (LSR & 0x01) // Data in receiver
				break;
		}

		if (cnt < S3_POLL_SERIAL_RX)
		{
			*((char *)Data + 1) = I2C_ReadRandom(S3I2CCurTxOptAddr, 0x00 << 3);
			*((char *)Data + 0) = I2C_ReadRandom(S3I2CCurTxOptAddr, 0x00 << 3);

			return 0;
		}
	}

	return 1;
#else
	return 0;
#endif
}

// ------------------------------------------------------------------------------
// Get Tx I2C Status register - not proved particularly useful

int S3I2CReadSerialStatus()
{
#ifdef TRIZEPS
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 0x52);		// Read SC18 internal register
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 0x0A);		// I2C status
	// I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 0x09);	// I2C timeout
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, STOP);

	unsigned char	cnt = 0;
	int				LSR;
	while(cnt++ < S3_POLL_SERIAL_RX)
	{
		LSR = I2C_ReadRandom(S3I2CCurTxOptAddr, SC16_LSR_ADDR);

		if (LSR & 0x01) // Data in receiver
			break;
	}

	if (cnt < S3_POLL_SERIAL_RX)
	{
		// unsigned char	Tout = I2C_ReadRandom(S3I2CCurTxOptAddr, 0x00 << 3);
		unsigned char	Stat = I2C_ReadRandom(S3I2CCurTxOptAddr, 0x00 << 3);

		// 0xF1 NACK on address; 0xF2 NACK on data; 0xF8 time-out
		if (Stat != 0xF0)
			return 1;

		return 0;
	}
#endif

	return 1;
}

// ------------------------------------------------------------------------------
// Write single byte
int S3I2CWriteSerialByte(	unsigned char	DevAddr,
							unsigned char	RegAddr,
							unsigned char	Data)
{
#ifdef TRIZEPS
	if (S3I2CWaitForEmpty())
		return 1;

	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, START);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, DevAddr);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 2);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, RegAddr);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, Data);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, STOP);

	// S3I2CReadSerialStatus();

	unsigned char	cnt = 0;
	int				LSR;
	while(cnt++ < S3_POLL_SERIAL_RX)
	{
		LSR = I2C_ReadRandom(S3I2CCurTxOptAddr, SC16_LSR_ADDR);

		if (LSR & 0x40) // Transmit buffer empty
			break;
	}

	if (cnt == S3_POLL_SERIAL_RX)
		return 1;
#endif
	return 0;
}

// ------------------------------------------------------------------------------
// Write single signed LE short to buffer as BE.

int S3I2CWriteSerialShort(	unsigned char	DevAddr,
							unsigned char	RegAddr,
							unsigned short	Data)
{
#ifdef TRIZEPS
	if (S3I2CWaitForEmpty())
		return 1;

	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, START);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, DevAddr);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, 2 + 1);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, RegAddr);
	// Swap bytes to BE
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, *((unsigned char *)&Data + 1));
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, *((unsigned char *)&Data + 0));
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, STOP);

	unsigned char	cnt = 0;
	int				LSR;
	while(cnt++ < S3_POLL_SERIAL_RX)
	{
		LSR = I2C_ReadRandom(S3I2CCurTxOptAddr, SC16_LSR_ADDR);

		if (LSR & 0x40) // Transmit buffer empty
			break;
	}

	if (cnt == S3_POLL_SERIAL_RX)
		return 1;
#endif
	return 0;
}

// ------------------------------------------------------------------------------
// Write null-terminated string

int S3I2CWriteSerialStr(	unsigned char	DevAddr,
							unsigned char	RegAddr,
							const char		*Str)
{
#ifdef TRIZEPS
	if (S3I2CWaitForEmpty())
		return 1;

	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, START);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, DevAddr);

	int len = strlen(Str);

	if (len > S3_SERIAL_FIFO_LEN)
		len = S3_SERIAL_FIFO_LEN;

	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, (unsigned char)len + 1);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, RegAddr);

	for(unsigned char i = 0; i < len; i++)
	{
		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, Str[i]);
	}

	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, STOP);
#endif

	return 0;
}

// ------------------------------------------------------------------------------
// Write len data bytes

int S3I2CWriteSerialData(	unsigned char		DevAddr,
							unsigned char		RegAddr,
							const unsigned char	*Data,
							unsigned char		NBytes)
{
#ifdef TRIZEPS
	if (S3I2CWaitForEmpty())
		return 1;
	
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, START);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, DevAddr);

	// Arbitrary
	if (NBytes > S3_SERIAL_FIFO_LEN)
		NBytes = S3_SERIAL_FIFO_LEN;

	// NBytes + register address
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, NBytes + 1);
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, RegAddr);

	for(unsigned char i = 0; i < NBytes; i++)
	{
		I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, Data[i]);

		/*
		unsigned char	cnt = 0;
		int				LSR;
		while(cnt++ < S3_POLL_SERIAL_RX)
		{
			LSR = I2C_ReadRandom(S3I2CCurTxOptAddr, SC16_LSR_ADDR);

			if (LSR & 0x40) // Transmit buffer empty
				break;
		}

		if (cnt == S3_POLL_SERIAL_RX)
			return 1;
		*/
	}

	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x00 << 3, STOP);
#endif
	return 0;
}

// ----------------------------------------------------------------------------
// Wait for something to appear in RX serial FIFO (LSR = Line Status Register)

int S3I2CWaitFIFO()
{
#ifdef TRIZEPS
	unsigned char	cnt = 0;
	int				LSR;

	while(cnt++ < S3_POLL_SERIAL_RX)
	{
		LSR = I2C_ReadRandom(S3I2CCurTxOptAddr, SC16_LSR_ADDR);

		if (LSR & 0x01)
			break;
	}

	if (cnt >= S3_POLL_SERIAL_RX)
		return 1; // Timeout
#endif
	return 0;
}

// ----------------------------------------------------------------------------
// Clears TX and RX FIFOs

void S3I2CClearFIFO()
{
#ifdef TRIZEPS
	unsigned char FCR = I2C_ReadRandom(S3I2CCurTxOptAddr, 0x02 << 3);
	FCR |= 0x06;
	I2C_WriteRandom(S3I2CCurTxOptAddr, 0x02 << 3, FCR);
#endif
}

// ----------------------------------------------------------------------------