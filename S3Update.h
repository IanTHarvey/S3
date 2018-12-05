#pragma once


#define S3_MAX_FILENAME_LEN		128
#define S3_UPDATE_HEADER_LEN	4096

#define S3_VERSION_POS			0
#define S3_HEADER_LEN_POS		4
#define S3_PAYLOAD_LEN_POS		8
#define S3_DATETIME_POS			32
#define S3_FILE_TYPE_POS		128
#define S3_HASH_POS				256

#define S3_LAST_BUILD_FILENAME	"C:\\PPM\\Sentinel3\\S3Controller\\LastBuild.txt"

class S3Update
{
public:
	S3Update(void);
	S3Update(CString _PayloadSrc, CString _PayloadDest, CString _UpdFilename,
		CString _UpdLocation);
	~S3Update(void);

	int Unwrap();
	int Wrap();

	CString	GetVersion();
	CString	GetDateTime();
	int		GetError();
	CString	GetErrorStr();
	void	Clear();

	// Use as a marker/file type confirmation
	void		SetType(const char *type);
	const char	*GetType();

	int		WritePayload();
	int		readUpdFile();

	bool	Unwrapping; // Thread flag

	CString	PayloadSrc;
	CString	PayloadDest;
	CString	UpdFilename;
	CString	UpdRoot;

private:
	char	*exe;
	size_t	exe_len;
	char	ver[3];
	int		err;
	int		datetime[5];
	char	type[32];

	int readPayloadFile(char **source, size_t *len);

	int writeUpdFile(char *source, size_t data_len, const char *sha256hash);

	int writeExeFile();
	int GetLastBuild(int *dt);
};

// -----------------------------------------------------------------------------
