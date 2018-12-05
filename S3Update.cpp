
#include "stdafx.h"
#include "S3Boot/SHA256.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "S3SystemDetails.h"
#include "S3Datamodel.h"
#include "S3Update.h"

S3Update::~S3Update(void)
{
	delete exe;
}

// -----------------------------------------------------------------------------

S3Update::S3Update(void)
{
	Clear();
}

S3Update::S3Update(CString _PayloadSrc, CString _PayloadDest,
				   CString _UpdFilename, CString _UpdLocation)
{
	Clear();

	PayloadSrc = _PayloadSrc;
	PayloadDest = _PayloadDest;
	UpdFilename = _UpdFilename;
	UpdRoot = _UpdLocation; 
}

// -----------------------------------------------------------------------------

void S3Update::Clear(void)
{
	exe = NULL;
	exe_len = 0;
	memset(ver, 0, 3 * sizeof(char));
	suffix = '\0';
	err = 0;
	Unwrapping = false;
	memset(datetime, 0, sizeof(datetime));
	memset(type, 0, sizeof(type));
}

// -----------------------------------------------------------------------------

int S3Update::Wrap()
{
	char *data;
	size_t len;

	if (!readPayloadFile(&data, &len)) 
	{
		char buf[2 * SHA256::DIGEST_SIZE + 1];
		sha256(buf, data, len);

		if (writeUpdFile(data, len, buf))
			err = 2000;
	}
	else err = 1000;

	free(data);

	return err;
}

// -----------------------------------------------------------------------------
// Threaded as may take significant time.

UINT __cdecl UnwrapThread( LPVOID pParam )
{
	S3Update *pObject = (S3Update *)pParam;

	pObject->readUpdFile();
	pObject->Unwrapping = false;

	return 0;
}

// -----------------------------------------------------------------------------

int S3Update::Unwrap()
{
	Unwrapping = true;
	CWinThread *m_UnwrapThread = AfxBeginThread(UnwrapThread, this);

	return err;
}

// -----------------------------------------------------------------------------

int S3Update::WritePayload()
{
	// Write executable part
	err = writeExeFile();

	return err;
}

// -----------------------------------------------------------------------------

CString S3Update::GetVersion()
{
	CString ret;

	ret.Format(_T("%d.%02d.%03d"), ver[0], ver[1], ver[2]);

	if (suffix != '\0')
		ret.AppendChar(suffix);

	return ret;
}

// -----------------------------------------------------------------------------

void S3Update::SetType(const char *_type)
{
	strcpy_s(type, 32, _type);
}

// -----------------------------------------------------------------------------

const char *S3Update::GetType()
{
	return type;
}

// -----------------------------------------------------------------------------

CString S3Update::GetDateTime()
{
	CString ret;

	ret.Format(_T("%02d-%02d-%d, %02d:%02d"),
		datetime[0], datetime[1], datetime[2], 
		datetime[3], datetime[4]);

	return ret;
}

// -----------------------------------------------------------------------------

int S3Update::GetError()
{
	return err;
}

// -----------------------------------------------------------------------------

CString S3Update::GetErrorStr()
{
	CString ret;
	
	if (err)
	{
		if (err == 2010)
			ret = _T("No USB HDD drive found");
		else if (err == 2001)
			ret = _T("Update file not found");
		else if (err == 2002 || err == 2003 || err == 2004 || err == 2005)
			ret = _T("Failed to open update file");
		else if (err == 3000)
			ret = _T("Payload incorrect length");
		else if (err = 3001)
			ret = _T("SHA-256 test failed\n");
		else
			ret = _T("Failed to unpack update file");
	}
	else
		ret = _T("OK");

	return ret;
}

// -----------------------------------------------------------------------------

int S3Update::readUpdFile()
{
	err = 0;

	DWORD fileAtt = GetFileAttributes(UpdRoot);

	if (fileAtt == INVALID_FILE_ATTRIBUTES ||
								(fileAtt & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		err = 2010;
		return err;
	}

	FILE *fid;
	errno_t ferr = _wfopen_s(&fid, UpdFilename, _T("rb"));

	if (ferr)
	{
		err = 2001;
		return err;
	}

	if (fseek(fid, 0L, SEEK_END) != 0)
	{
		err = 2002;
		goto FILEFAIL;
	}

	long bufsize = ftell(fid);
    
	if (bufsize == -1)
	{
		err = 2003;
		goto FILEFAIL;
	}

	if (fseek(fid, 0L, SEEK_SET) != 0)
	{
		err = 2004;
		goto FILEFAIL;
	}

	char *source = (char *)malloc(sizeof(char) * bufsize);
	size_t len = fread(source, sizeof(char), bufsize, fid);
	size_t payload_len = len - S3_UPDATE_HEADER_LEN;
	
	if (len == 0)
	{
		err = 2005;
		goto FILEFAIL;
	}

	// Validate
	size_t declared_exe_len = *((size_t *)(source + 8));

	if (declared_exe_len != payload_len)
	{
		err = 3000;
	}

	if (!err)
	{
		memcpy(type, source + S3_FILE_TYPE_POS, sizeof(type));

		char sha256hash[2 * SHA256::DIGEST_SIZE + 1];
		strcpy_s(sha256hash, 2 * SHA256::DIGEST_SIZE + 1,
			(source + S3_HASH_POS));

		char buf[2 * SHA256::DIGEST_SIZE + 1];
		sha256(buf, source + S3_UPDATE_HEADER_LEN, payload_len);

		if (strcmp(sha256hash, buf))
		{
			err = 3001;
		}
		else
		{
			err = 0;

			memcpy(ver, source, 3 * sizeof(char));
			suffix = *(source + 3 * sizeof(char));
			exe_len = payload_len;

			memcpy(datetime, source + S3_DATETIME_POS, sizeof(datetime));

			exe = (char *)malloc(exe_len * sizeof(char));
			memcpy(exe, source + S3_UPDATE_HEADER_LEN, payload_len);
		}
	}

	free(source);

FILEFAIL:
	fclose(fid);

	return err;
}

// -----------------------------------------------------------------------------
// Allocate buffer and read executable file. Caller must delete.

int S3Update::readPayloadFile(char **source, size_t *len)
{
	*len = 0;
	*source = NULL;

	FILE *fid;
	errno_t err = _wfopen_s(&fid, PayloadSrc, _T("rb"));

	if (err)
		return 1;

	if (fseek(fid, 0L, SEEK_END) == 0)
	{
		long bufsize = ftell(fid);
    
		if (bufsize == -1)
			{ err = 1; goto FILEFAIL; }

		*source = (char *)malloc(sizeof(char) * bufsize);

		if (fseek(fid, 0L, SEEK_SET) != 0)
			{ err = 1; goto FILEFAIL; }

		*len = fread(*source, sizeof(char), bufsize, fid);
		if (*len == 0)
			{ err = 1; goto FILEFAIL; }
	}
	else
		err = 1;

FILEFAIL:
	fclose(fid);

	return err;
}

// -----------------------------------------------------------------------------

int S3Update::writeUpdFile(char *source, size_t data_len,
						   const char *sha256hash)
{	
	if (GetLastBuild(datetime))
		return -5;

	size_t newLen = 0;
	
	FILE *fid;
	errno_t ferr = _wfopen_s(&fid, UpdFilename, _T("wb"));

	if (ferr)
		return -1;

	char *header = (char *)malloc(S3_UPDATE_HEADER_LEN * sizeof(char));
	memset(header, 0, S3_UPDATE_HEADER_LEN * sizeof(char));

	CString v(S3_SYS_SW);

	int nTokenPos = 0;
	CString strToken = v.Tokenize(_T("."), nTokenPos);

	int cnt = 0;
	while (!strToken.IsEmpty())
	{
		ver[cnt] = *(header + S3_VERSION_POS + cnt++) = (char)_ttoi(strToken);
		strToken = v.Tokenize(_T("."), nTokenPos);
	}

	int vl = v.GetLength();
	suffix = '\0';
	
	if (vl == 9)
		suffix = (char)v[vl - 1];

	*(header + S3_VERSION_POS + cnt) = suffix;

	*((int *)(header + S3_HEADER_LEN_POS)) = S3_UPDATE_HEADER_LEN;
	*((size_t *)(header + S3_PAYLOAD_LEN_POS)) = data_len;

	memcpy(header + S3_FILE_TYPE_POS, type, sizeof(type));

	memcpy(header + S3_DATETIME_POS, datetime, sizeof(datetime));
	strcpy_s((header + S3_HASH_POS), 2 * SHA256::DIGEST_SIZE + 1, sha256hash);

	size_t n;
	int lerr = 0;
	
	n = fwrite(header, sizeof(char), S3_UPDATE_HEADER_LEN, fid);
	
	if (n != S3_UPDATE_HEADER_LEN)
	{
		fclose(fid);
		lerr = -1;
	}
	
	if (!lerr)
	{
		n = fwrite(source, sizeof(char), data_len, fid);

		if (n != data_len)
			lerr = -2;
	}

	fclose(fid);
	free(header);

	return lerr;
}

// -----------------------------------------------------------------------------

int S3Update::writeExeFile()
{
	FILE *fid;

	errno_t ferr = _wfopen_s(&fid, PayloadDest, _T("wb"));

	if (ferr)
		return -1;

	int err = 0;
	size_t n = fwrite(exe, sizeof(char), exe_len, fid);

	if (n != exe_len)
		err = -1;

	fclose(fid);

	return err;
}

// -----------------------------------------------------------------------------

int S3Update::GetLastBuild(int *dt)
{
#ifndef TRIZEPS
	FILE *fid;

	errno_t ferr = _wfopen_s(&fid, _T(S3_LAST_BUILD_FILENAME), _T("r"));

	if (ferr)
	{
		char msg[1024];
		strerror_s(msg, 1024, ferr);

		printf("Failed to open LastBuild file: %s\n", msg);
		return 1;
	}

	char date[128];
	char time[128];
	fgets(date, 128, fid);
	fgets(time, 128, fid);

	CString DateTime(date);
	DateTime += "-";
	DateTime += CString(time);

	DateTime.Remove(_T('\n'));
	DateTime.Remove(_T('\r'));
	DateTime.Remove(_T(' '));

	dt[0] = (int)_ttoi(DateTime.Mid(0, 2));
	dt[1] = (int)_ttoi(DateTime.Mid(3, 2));
	dt[2] = (int)_ttoi(DateTime.Mid(6, 4));
	dt[3] = (int)_ttoi(DateTime.Mid(11, 2));
	dt[4] = (int)_ttoi(DateTime.Mid(14, 2));
	
	fclose(fid);
#endif

	return 0;
}

// -----------------------------------------------------------------------------
