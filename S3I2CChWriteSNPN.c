// ----------------------------------------------------------------------------
// Factory-only routine
//

int S3I2CChWriteSNPN(const char *SN, const char *PN)
{
#ifdef TRIZEPS
	// char SNc[] =	{"Demo001"};
	// char PNc[] =	{"S3-BAT-2P-00"};

	if (strlen(SN) >= S3_FLASH_SN_SIZE)
		return 2;

	if (strlen(PN) >= S3_FLASH_PN_SIZE)
		return 2;

	BOOL	ok;
	unsigned char cmd[32];
	unsigned char cmdSN[S3_FLASH_BLOCK_SIZE + 1];
	unsigned char i2cStartAddr = 0x00;

	// Get 'Manufacturer Info' block data
	cmd[0] = 0x3e;	// DataFlashClass
	cmd[1] = 58;	// Subclass 'Manufacturer Info'
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

	cmd[0] = 0x3f;	// DataFlashBlock
	cmd[1] = 0x00;	// Offset bank: 0: '0-31', 1: 32-63
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

	unsigned char i;
	for(i = 0; i < S3_FLASH_BLOCK_SIZE + 1; i++)
		cmdSN[i] = 0;
	cmdSN[0] = 0x40; // Write data to offset 0x40 + offset
	cmdSN[1] = 0x00;

	for(i = 0; i < strlen(SN); i++)
		cmdSN[i + 1] = SN[i];

	for(i = 0; i < strlen(PN); i++)
		cmdSN[i + 1 + S3_FLASH_SN_SIZE] = PN[i];

	unsigned char len = 1 + strlen(PN) + S3_FLASH_SN_SIZE + 1;

	// Copy data then write to flash

	// Could write part of block but would need to read previous
	// block to calculate (or adjust) checksum.

	// Calculate checksum for the WHOLE block
	unsigned char CheckSum = 0;
	for(unsigned char i = 1; i < S3_FLASH_BLOCK_SIZE + 1; i++)
		CheckSum += cmdSN[i];

	CheckSum = 0xFF - CheckSum;

	// Get checksum - don't need as rewriting the whold block
	// cmd[0] = 0x60;
	// ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 1, i2cCmdBufRead, 1);

	// Write the WHOLE data block
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmdSN, S3_FLASH_BLOCK_SIZE + 1, NULL, 0);

	// Set checksum to force transfer to data flash
	cmd[0] = 0x60;
	cmd[1] = CheckSum;
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 2, NULL, 0);

	// Do reset
	cmd[0] = 0x00;
	cmd[1] = 0x41;
	cmd[2] = 0x00;
	ok = I2C_WriteRead(S3I2C_CH_BATT_ADDR, cmd, 3, NULL, 0);

	// Read back and check
	char SNrd[S3_MAX_SN_LEN], PNrd[S3_MAX_PN_LEN];
	if (!S3I2CChReadSNPN(SNrd, PNrd))
	{
		if (strcmp(SNrd, SN))
			return 1;

		if (strcmp(PNrd, PN))
			return 1;
	}
	else return 1;

#endif // TRIZEPS

	return 0;	
}

// ----------------------------------------------------------------------------