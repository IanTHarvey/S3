#include "stdafx.h"

#include "S3GPIO.h"

#define  CPLUSPLUS 1
#ifdef TRIZEPS
#include "c:\Program Files (x86)\Windows CE Tools\SDKs\Trizeps-VII-2015-Q2\Include\Armv4i\drvlib_app.h"
// #include "P:\PPM\S3\gpio\drvlib_app.h"
#endif

#define PLATFORM_BASE_VA_CS3 0x04000000

char set = 0;

int S3GPIOtest()
{
#ifdef TRIZEPS
	//Func PVOID		OEMMapIoSpace( DWORD Address, ULONG NumberOfBytes, BOOL CacheEnable);
	//Func VOID			OEMUnmapIoSpace( PVOID BaseAddress, ULONG NumberOfBytes);
	//Func int			IoWriteU8( unsigned char* addr, unsigned char data);
	//Func int			IoWriteU16( unsigned short* addr, unsigned short data);
	//Func int			IoWriteU32( unsigned int* addr, unsigned int data);
	//Func unsigned char	IoReadU8( unsigned char* addr);
	//Func unsigned short IoReadU16( unsigned short* addr);
	//Func unsigned int	IoReadU32( unsigned int* addr);
	
	/*
	unsigned char io[2];
	
	OEMMapIoSpace( (DWORD)io, 2, true);

	IoWriteU16( (unsigned short *)io, 0xFFFF);

//	OEMUnmapIoSpace( (PVOID)io, 2);


	volatile short *padr = (volatile short*)PLATFORM_BASE_VA_CS3;
	*padr = 123;
	*/

	/*
	PGPIO_REGS ptr = GPIO_AllocSpace();

	GPIO_Init_Pin( ptr, (unsigned char) 0, TRUE, TRUE);
	GPIO_Init_Pin( ptr, (unsigned char) 1, TRUE, TRUE);
	GPIO_Init_Pin( ptr, (unsigned char) 2, TRUE, TRUE);
	GPIO_Init_Pin( ptr, (unsigned char) 3, TRUE, TRUE);
	GPIO_Init_Pin( ptr, (unsigned char) 4, TRUE, TRUE);
	GPIO_Init_Pin( ptr, (unsigned char) 5, TRUE, TRUE);
	GPIO_Init_Pin( ptr, (unsigned char) 6, TRUE, TRUE);
	GPIO_Init_Pin( ptr, (unsigned char) 7, TRUE, TRUE);
	
	GPIO_Set_Pin( ptr, (unsigned char) 0, TRUE);
	GPIO_Set_Pin( ptr, (unsigned char) 1, TRUE);
	GPIO_Set_Pin( ptr, (unsigned char) 2, TRUE);
	GPIO_Set_Pin( ptr, (unsigned char) 3, TRUE);
	GPIO_Set_Pin( ptr, (unsigned char) 4, TRUE);
	GPIO_Set_Pin( ptr, (unsigned char) 5, TRUE);
	GPIO_Set_Pin( ptr, (unsigned char) 6, TRUE);
	GPIO_Set_Pin( ptr, (unsigned char) 7, TRUE);

	Sleep(5000);

	GPIO_Set_Pin( ptr, (unsigned char) 0, FALSE);
	GPIO_Set_Pin( ptr, (unsigned char) 1, FALSE);
	GPIO_Set_Pin( ptr, (unsigned char) 2, FALSE);
	GPIO_Set_Pin( ptr, (unsigned char) 3, FALSE);
	GPIO_Set_Pin( ptr, (unsigned char) 4, FALSE);
	GPIO_Set_Pin( ptr, (unsigned char) 5, FALSE);
	GPIO_Set_Pin( ptr, (unsigned char) 6, FALSE);
	GPIO_Set_Pin( ptr, (unsigned char) 7, FALSE);

	GPIO_FreeSpace();
	*/

	BOOL ok;
	ok = Open_TTLPort(0);
	if (set)
	{
		ok = Set_TTLPortBit( 0x80, 0);
		set = 0;
	}
	else
	{
		ok = Clr_TTLPortBit( 0x80, 0);
		set = 1;
	}

	Close_TTLPort(0);

#endif

	return 0;
}

// ----------------------------------------------------------------------------

int S3GPIOInit()
{
#ifdef TRIZEPS
	BOOL ok = Open_TTLPort(0);

	if (!ok)
		return 1;

	ok = S3GPIOSetBits(0x00);

	if (!ok)
		return 2;

#endif

	return 0;
}

// ----------------------------------------------------------------------------

int S3GPIOClose()
{
#ifdef TRIZEPS
	BOOL ok = Close_TTLPort(0);

	if (!ok)
		return 1;

#endif

	return 0;
}

// ----------------------------------------------------------------------------

BOOL S3GPIOSetBit(unsigned char bit, BOOL high)
{
	BOOL ok = FALSE;
#ifdef TRIZEPS

	if (high)
		ok = Set_TTLPortBit( 1 << bit, 0);
	else
		ok = Clr_TTLPortBit( 1 << bit, 0);
#endif

	return ok;
}

// ----------------------------------------------------------------------------

int S3GPIOSetBits(unsigned char bits)
{
	BOOL ok = FALSE;

#ifdef TRIZEPS
	ok = Set_TTLPort( bits, 0);
#endif

	return ok;
}

// ----------------------------------------------------------------------------
