// S3UpdateBuilder.cpp : Defines the entry point for the console application.
//

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdafx.h"
#include "../S3Boot/SHA256.h"

#define S3_MAX_FILENAME_LEN		128
#define S3_UPDATE_HEADER_LEN	4096

int readFile(char **source, const _TCHAR *filename);
int writeFile(char *source, size_t data_len, const char *sha256hash,
			  const _TCHAR *filename);

int _tmain(int argc, _TCHAR* argv[])
{
	_TCHAR ExeFile[S3_MAX_FILENAME_LEN];
	wcscpy_s(ExeFile, S3_MAX_FILENAME_LEN, argv[1]);

	char *data;
	size_t len = (size_t)readFile(&data, ExeFile);

	if (len > 0)
	{
		char buf[2 * SHA256::DIGEST_SIZE + 1];
		sha256(buf, data, len);

		printf("Buf: %s\n", buf);
		writeFile(data, len, buf, _T("S3TestUpdate.upd"));

		free(data);
	}

	return 0;
}

// -----------------------------------------------------------------------------

int readFile(char **source, const _TCHAR *filename)
{
	size_t newLen = 0;
	*source = NULL;

	FILE *fid;
	errno_t err = _wfopen_s(&fid, filename, _T("rb"));

	if (!err)
	{
		// Go to the end of the file
		if (fseek(fid, 0L, SEEK_END) == 0)
		{
			// Get the size of the file
			long bufsize = ftell(fid);
        
			if (bufsize == -1)
			{
				wprintf(_T("Failed to get file size: %s"), filename);
				return -1;
			}

			// Allocate our buffer to that size
			*source = (char *)malloc(sizeof(char) * bufsize);

			// Go back to the start of the file
			if (fseek(fid, 0L, SEEK_SET) != 0)
			{
				wprintf(_T("Failed to rewind: %s"), filename);
				return -1;
			}

			// Read the entire file into memory
			newLen = fread(*source, sizeof(char), bufsize, fid);
			if (newLen == 0)
			{
				fputs("Error reading file", stderr);
				return -1;
			}
		}
		else
		{
			wprintf(_T("Failed to rewind: %s"), filename);
			return -1;
		}

		fclose(fid);
	}
	else
	{
		wprintf(_T("Failed to find end of file: %s"), filename);
		return -1;
	}

	return newLen;
}

// -----------------------------------------------------------------------------

int writeFile(char *source, size_t data_len, const char *sha256hash,
			  const _TCHAR *filename)
{
	size_t newLen = 0;
	*source = NULL;

	FILE *fid;
	errno_t err = _wfopen_s(&fid, filename, _T("wb"));

	char *header = (char *)malloc(S3_UPDATE_HEADER_LEN * sizeof(char));
	memset(header, 0, S3_UPDATE_HEADER_LEN * sizeof(char));

	header[0] = 0;
	header[1] = 0;
	header[2] = 0;

	*((int *)(header + 3)) = S3_UPDATE_HEADER_LEN;
	*((int *)(header + 3)) = data_len;

	strcpy_s((header + 256), 2 * SHA256::DIGEST_SIZE + 1, sha256hash);

	if (!err)
	{
		fwrite(header, sizeof(char), S3_UPDATE_HEADER_LEN, fid);
		fwrite(source, sizeof(char), data_len, fid);

		fclose(fid);
	}
	else
		return 1;

	return 0;
}

// -----------------------------------------------------------------------------