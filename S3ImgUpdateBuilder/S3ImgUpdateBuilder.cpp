// S3ImgUpdateBuilder.cpp : Defines the entry point for the console application.
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
		_T(S3_IMG_UPDATE_WRAP_FILENAME), _T("."));

	U.SetType("S3ImageUpdate");

	wprintf(_T("Wrapping: %s\n"), U.PayloadSrc);
	
	err = U.Wrap();

	if (err)
		wprintf(_T("Failed to wrap\n"));
	else
	{
		class S3Update Uverify(_T(""), _T(""),
			_T(S3_IMG_UPDATE_WRAP_FILENAME), _T("."));
		
		wprintf(_T("Verifying package:\nUnwrapping: %s\n"), Uverify.UpdFilename);
		
		err = Uverify.Unwrap();

		wprintf(_T("Waiting to unwrap...\n"));

		while(Uverify.Unwrapping)
			Sleep(10);

		if (Uverify.GetError())
		{
			wprintf(_T("Failed to unwrap: %s\n"), Uverify.GetErrorStr());
		}
		else
		{
			wprintf(_T("Unwrapped: v%s (%s)\n"),
				Uverify.GetVersion(), Uverify.GetDateTime());

			wprintf(_T("Type: %S\n"), Uverify.GetType());
		}
	}

	wprintf(_T("Press any key\n"));
	char c = getchar();

	return err;
}

// -----------------------------------------------------------------------------