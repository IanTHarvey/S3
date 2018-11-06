// S3UpdateBuilder.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "../S3SystemDetails.h"
#include "../S3Update.h"

#define S3_IMG_UPDATE_WRAP_FILENAME	"S3ImageUpdate.upd"
#define S3_IMG_PAYLOAD_FILENAME		"C:\\Flashdisk\\S3TestOS.nb0"

using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	int err;

	class S3Update U(argv[1], _T(S3_IMG_PAYLOAD_FILENAME),
		_T(S3_IMG_UPDATE_WRAP_FILENAME));

	U.SetType("S3ImageUpdate");

	class S3Update Uverify(argv[1], _T(S3_IMG_PAYLOAD_FILENAME),
		_T(S3_IMG_UPDATE_WRAP_FILENAME));

	wprintf(_T("Wrapping: %s\n"), U.PayloadSrc);
	
	err = U.Wrap();

	if (err)
		wprintf(_T("Failed to wrap\n"));

	if (!err)
	{
		wprintf(_T("Unwrapping: %s\n"), Uverify.UpdFilename);
		err = Uverify.Unwrap();

		Sleep(1); // Give thread time to start
		wprintf(_T("Waiting to unwrap...\n"));

		while(Uverify.Unwrapping);

		if (err)
			wprintf(_T("Failed to unwrap\n"));
		else
		{
			wprintf(_T("Unwrapped: v%s (%s)\n"),
				Uverify.GetVersion(), Uverify.GetDateTime());

			wprintf(_T("Type: %S\n"), Uverify.GetType());
		}
	}

	char c = getchar();

	return err;
}

// -----------------------------------------------------------------------------