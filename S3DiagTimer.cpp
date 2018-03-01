// ----------------------------------------------------------------------------
// High precision timer, see:
// http://stackoverflow.com/questions/1739259/how-to-use-queryperformancecounter

#include "stdafx.h"
#include "S3DataModel.h"
//#include "S3ControllerDlg.h"

#ifdef S3_DIAG_TIMING
double PCFreq = 0.0;
__int64 CounterStart[8] = {0, 0, 0, 0, 0, 0, 0, 0};
FILE	*TimerLog = NULL;
#define S3_DIAG_TIMER_FILE	"TimeLog.log"
char S3DiagTimerFileName[S3_MAX_FILENAME_LEN];
#endif

// int S3TimerInit(CS3ControllerDlg *dlg)
int S3TimerInit()
{
#ifdef S3_DIAG_TIMING
	
	// Write to root as writing to \flashdisk significantly affects timings
	sprintf_s(S3DiagTimerFileName, S3_MAX_FILENAME_LEN, "%s",
			S3_DIAG_TIMER_FILE);

	//// Open and clear file
	//int err = fopen_s(&TimerLog, S3DiagTimerFileName, "w");
	//if (!err)
	//{
	//	char str[S3_DATETIME_LEN];
	//	dlg->GetDateTimeStrA(str);

	//	fprintf(TimerLog, "%s\n", str);
	//	fclose(TimerLog);
	//}
#endif
	return 0;
}

int S3TimerStart(unsigned char Tid)
{
#ifdef S3_DIAG_TIMING
	
	LARGE_INTEGER li;
    if(!QueryPerformanceFrequency(&li))
		return 1;

    PCFreq = double(li.QuadPart)/1000.0;

    QueryPerformanceCounter(&li);
    CounterStart[Tid] = li.QuadPart;
#endif
	return 0;
}

int S3TimerStop(unsigned char Tid)
{
#ifdef S3_DIAG_TIMING
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
	double t = (double)(li.QuadPart - CounterStart[Tid]) / PCFreq;

	int err = fopen_s(&TimerLog, S3DiagTimerFileName, "a");
	if (!err)
	{
		fprintf(TimerLog, "%d: %.2fms\n", Tid, t);
		fclose(TimerLog);
	}
	else return 1;
#endif
    return 0;
}

// -----------------------------------------------------------------------------