
// ----------------------------------------------------------------------------
// Comparison functions

#include "windows.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "S3DataModel.h"

// ----------------------------------------------------------------------------
// Compare current system hardware to last known config. Is there any
// component SN that doesn't match exactly?

int S3SystemIdentical(pS3DataModel last, pS3DataModel current)
{
	S3SysCmp(last, current);

	return 0;
}

// ----------------------------------------------------------------------------
// Compare current system hardware to last known config. Is there any
// component to which the last known config isn't applicable? ie type match

int S3SystemCompatible(pS3DataModel last, pS3DataModel current)
{
	return 0;
}

// ----------------------------------------------------------------------------

int S3SysCmp(pS3DataModel s1, pS3DataModel s2)
{
	int diff = 0;

	if (strcmp(s1->m_SN, s2->m_SN))
		diff++;

	for (char Rx = 0; Rx < S3_MAX_RXS; Rx++)
		diff += S3RxCmp(&s1->m_Rx[Rx], &s2->m_Rx[Rx]);

	return diff;
}

// ----------------------------------------------------------------------------

int S3RxCmp(pS3RxData r1, pS3RxData r2)
{
	int diff = 0;

	if (strcmp(r1->m_SN, r2->m_SN))
		diff++;

	if (diff)
		return diff;

	for (char Tx = 0; Tx < S3_MAX_TXS; Tx++)
		diff += S3TxCmp(&r1->m_Tx[Tx], &r2->m_Tx[Tx]);

	return diff;
}

// ----------------------------------------------------------------------------

int S3TxCmp(pS3TxData t1, pS3TxData t2)
{
	int diff = 0;

	if (strcmp(t1->m_SN, t2->m_SN))
		diff++;

	return diff;
}

// ----------------------------------------------------------------------------
