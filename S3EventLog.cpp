// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------

#include "stdafx.h"

#include <atltime.h>
#include <errno.h>
#include "S3DataModel.h"

// FILE	*ELFid;
extern pS3DataModel S3Data;

int gS3EventOutstanding;
S3EventLog gS3EventLog;

int (*gS3EventNotifyFn)(void);

// Windows only
void S3GetTimeStr(char *str);

// ----------------------------------------------------------------------------
// TODO: Obsolete if log file name tied to system name

int S3SetEventLogName(const char *filename)
{
	// S3GetPathFile(NULL, S3Data->m_EventLogName, filename);
	// strcpy_s(S3Data->m_EventLogName, S3_MAX_FILENAME_LEN, filename);

	S3EventLogInit(filename);

	return 0;
}

// ----------------------------------------------------------------------------
// Can't pass a class member function here, so probably useless. Provide
// polling function instead.

int S3EventAddNotifyFn(int (*fn)(void))
{
	gS3EventNotifyFn = fn;

	return 0;
}

// ----------------------------------------------------------------------------

int S3EventOutstanding(void)
{
	if (gS3EventOutstanding)
	{
		gS3EventOutstanding = 0;
		return 1;
	}

	return 0;
}

// ----------------------------------------------------------------------------
// TODO: May be too big for stack?

char	S3EventFileBuf[S3_MAX_EVENTS_LOG][S3_EVENTS_LINE_LEN];

// ----------------------------------------------------------------------------
// Minimal file name 'a.b' ie suffix must be at least 1 char

int S3RemSuffix(char *s)
{
	int len = strlen(s);

	if (len < 3) // < 'a.b'
		return 0;

	while (len && s[len] != '.')
		len--;

	// 'a.b' -> 'a' still valid filename
	if (len > 0)
	{
		s[len] = '\0';

		return 1;
	}

	return 0;
}

// ----------------------------------------------------------------------------
// If provided, filename must be full path

int S3EventLogInit(const char *filename)
{
	FILE	*fid;
	int		err;
	bool	SysStart = true;

	gS3EventLog.LogAlarm = S3_LOG_OK;

	// New log name required
	if (filename != NULL)
	{
		// Already been initialised, so need to sign off old log
		if (*S3Data->m_EventLogName != '\0')
		{
			char tmpF[S3_MAX_FILENAME_LEN];

			S3GetPathFile(NULL, tmpF, filename);

			SysStart = false;
			
			char tmp[S3_EVENTS_LINE_LEN];

			sprintf_s(tmp, S3_EVENTS_LINE_LEN, "This log is closing. Opening: %s",
				tmpF);

			S3EventLogAdd(tmp, 1, -1, -1, -1);
		}

		S3GetPathFile(S3Data->m_EventLogPath, S3Data->m_EventLogName, filename);
		// S3RemSuffix(S3Data->m_EventLogName);
	}

	char tmp[S3_MAX_FILENAME_LEN];

	sprintf_s(tmp, S3_MAX_FILENAME_LEN, "%s\\%s.s3l",
			S3Data->m_EventLogPath, S3Data->m_EventLogName);

	char line[S3_EVENTS_LINE_LEN];
	int len = 0;
	
	
	if (err = fopen_s(&fid, tmp, "r"))
	{
		// If file doesn't exist, do nothing

		if (err != ENOENT) // Not existing is OK
		{		
			gS3EventLog.LogAlarm = S3_LOG_FAIL_OPEN;
			return err;
		}		
	}
	else
	{
		while (!err && fgets(line, sizeof(line), fid))
		{
			len++;
		}

		fclose(fid);

		// Only copy the last S3_MAX_EVENTS_LOG lines from the existing log file,
		// if it exists.
		if (len > S3_MAX_EVENTS_LOG)
		{
			if (err = fopen_s(&fid, tmp, "r"))
			{			
				gS3EventLog.LogAlarm = S3_LOG_FAIL_OPEN_READ;
				return err;
			}

			int cnt = 0, outcnt = 0;
			while (fgets(line, sizeof(line), fid))
			{
				if (cnt >= len - S3_MAX_EVENTS_LOG)
				{
					strcpy_s(S3EventFileBuf[outcnt], S3_EVENTS_LINE_LEN, line);
					outcnt++;
				}

				cnt++;
			}

			fclose(fid);

			if (err = fopen_s(&fid, tmp, "w"))
			{
				gS3EventLog.LogAlarm = S3_LOG_FAIL_OPEN_WRITE;
				return err;
			}

			int		i;
			for (i = 0; i < outcnt; i++)
				fprintf(fid, "%s", S3EventFileBuf[i]);

			fclose(fid);
		}
	}
		
	// Get file size:
	// fseek(fid, 0, SEEK_END);
	// lSize = ftell(fid);
	// rewind(fid);

	gS3EventNotifyFn = NULL;
	gS3EventOutstanding = 0;

	gS3EventLog.Earliest = gS3EventLog.NextInsert = 0;
	gS3EventLog.Count = 0;

	if (SysStart)
		S3EventLogAdd("Log opened: System start", 1, -1, -1, -1);
	else
		S3EventLogAdd("Log opened: New log file", 1, -1, -1, -1);

	S3EventLogSysInfo();

	return 0;
}

// ----------------------------------------------------------------------------

int S3EventLogClose()
{
	S3EventLogAdd("Log closed. Good-bye.", 1, -1, -1, -1);

	return 0;
}

// ----------------------------------------------------------------------------
// System information header

int	S3EventLogSysInfo(void)
{
	FILE	*fid = NULL;

	char tmp[S3_MAX_FILENAME_LEN];

	gS3EventLog.LogAlarm = S3_LOG_OK;

	sprintf_s(tmp, S3_MAX_FILENAME_LEN, "%s\\%s.s3l",
		S3Data->m_EventLogPath, S3Data->m_EventLogName);

	// TODO: Need to test this result - create if not exist? 
	int err = fopen_s(&fid, tmp, "a");

	if (err)
	{
		gS3EventLog.LogAlarm = S3_LOG_FAIL_OPEN_APPEND;
		return err;
	}

	fprintf(fid, "\n\n=====================================================\n");
	fprintf(fid, "System Information\n");
	fprintf(fid, "=====================================================\n\n");

	fprintf(fid, "SN:\t\t%s\n", S3SysGetSN());
	fprintf(fid, "PN:\t\t%s\n", S3SysGetPN());
	fprintf(fid, "OS:\t\t%s %s\n", S3SysGetImageDate(), S3SysGetImageTime());
	fprintf(fid, "App:\t%s\n", S3SysGetAppDateTime());
	fprintf(fid, "\n=====================================================\n\n");

	if (S3GetDemoMode())
	{
		fprintf(fid, "=====================================================\n");
		fprintf(fid, "\tRUNNING IN DEMONSTRATION MODE\n");
		fprintf(fid, "=====================================================\n\n");
	}

	fflush(fid); // TODO: fclose() force this?
	fclose(fid);
	return 0;
}

// ----------------------------------------------------------------------------

int S3EventLogAdd(const char *msg, char severity, char Rx, char Tx, char IP)
{
	FILE	*fid = NULL;

	char tmp[S3_MAX_FILENAME_LEN];

	gS3EventLog.LogAlarm = S3_LOG_OK;

	sprintf_s(tmp, S3_MAX_FILENAME_LEN, "%s\\%s.s3l",
		S3Data->m_EventLogPath, S3Data->m_EventLogName);

	// TODO: Need to test this result - create if not exist? 
	int err = fopen_s(&fid, tmp, "a");

	if (err)
	{
		gS3EventLog.LogAlarm = S3_LOG_FAIL_OPEN_APPEND;
		return err;
	}

	char t[S3_MAX_TIME_STR_LEN];
	S3GetTimeStr(t);

	fprintf(fid, "%s (%d): %s; ", t, severity, msg);

	// Use this pattern (nonsense address) to indicate charger event
	if (IP != -1 && Tx == -1)
	{
		fprintf(fid, "Ch: %d;", IP + 1);
	}
	else
	{
		// Add any real address info
		if (IP != -1)
			fprintf(fid, "Rx: %d; Tx: %d; IP: %d; ", Rx + 1, Tx + 1, IP + 1);
		else if (Tx != -1)
			fprintf(fid, "Rx: %d; Tx: %d;", Rx + 1, Tx + 1);
		else if (Rx != -1)
			fprintf(fid, "Rx: %d;", Rx + 1);
	}

	fprintf(fid, "\n");

	fflush(fid); // TODO: fclose() force this?
	fclose(fid);

	strcpy_s(gS3EventLog.Events[gS3EventLog.NextInsert].t, S3_MAX_TIME_STR_LEN, t); 
	gS3EventLog.Events[gS3EventLog.NextInsert].severity = severity;
	strcpy_s(gS3EventLog.Events[gS3EventLog.NextInsert].msg, S3_EVENTS_LINE_LEN, msg);
	
	gS3EventLog.Events[gS3EventLog.NextInsert].Rx = Rx;
	gS3EventLog.Events[gS3EventLog.NextInsert].Tx = Tx;
	gS3EventLog.Events[gS3EventLog.NextInsert].IP = IP;

	gS3EventLog.NextInsert++;
	gS3EventLog.Count++;

	if (gS3EventLog.NextInsert == S3_MAX_EVENTS_LOG)
	{
		gS3EventLog.NextInsert = 0;
	}

	// Now wrapping...
	if (gS3EventLog.Count >= S3_MAX_EVENTS_LOG)
	{
		gS3EventLog.Count = S3_MAX_EVENTS_LOG;

		gS3EventLog.Earliest = gS3EventLog.NextInsert;
	}

	// Let anyone who's interested know... as long as not a class member,
	// which it needs to be

	if (gS3EventNotifyFn != NULL)
		gS3EventNotifyFn();

	if (gS3EventOutstanding < S3_MAX_EVENTS_LOG)
		gS3EventOutstanding++;

	return 0;
}

// ----------------------------------------------------------------------------
// Windows-applicable only
void S3GetTimeStr(char *str)
{
	CTime CurrentTime = CTime::GetCurrentTime();

	int iHours = CurrentTime.GetHour();
	int iMinutes = CurrentTime.GetMinute();
	int iSeconds = CurrentTime.GetSecond();

	int iYear = CurrentTime.GetYear();
	int iMonth = CurrentTime.GetMonth();
	int iDay = CurrentTime.GetDay();

	sprintf_s(str, S3_MAX_TIME_STR_LEN,	"%02d-%02d-%02d %02d:%02d:%02d",
		iYear, iMonth, iDay, iHours, iMinutes, iSeconds);
}

// ----------------------------------------------------------------------------

pS3EventLog S3EventGetLog(void)
{
	return &gS3EventLog;
}

// ----------------------------------------------------------------------------
// Get event, 0 is earliest 
pS3Event S3EventGetEvent(unsigned int i)
{
	if (i >= gS3EventLog.Count || i >= S3_MAX_EVENTS_LOG)
		return NULL;

	unsigned int	j = gS3EventLog.Earliest + i;

	if (j >= S3_MAX_EVENTS_LOG)
		j -= S3_MAX_EVENTS_LOG;

	return &gS3EventLog.Events[j];
}

// ----------------------------------------------------------------------------

pS3Event S3EventGetLastEvent()
{
	if (gS3EventLog.NextInsert)
		return &gS3EventLog.Events[gS3EventLog.NextInsert - 1];
	else
		return &gS3EventLog.Events[S3_MAX_EVENTS_LOG - 1];
}

// ----------------------------------------------------------------------------
