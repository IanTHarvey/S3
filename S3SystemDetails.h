// ----------------------------------------------------------------------------
// Sentinel 3 controller configurations - inventory details

// Reference only
// #define S3_DESKTOP_RACK		"S3K-DT-1-AC-00"
// #define S3_19_RACK			"S3K-19-1-AC-00"


// Major	- rewrite/redesign of system.
// Minor	- add/changes to functionality, HB update required.
// Revision - implementation changes, bug fixes committed, no user changes.

#ifdef S3_ATIS_BUILD
#define S3_SYS_SW			"ATiS.3"
#else
#define S3_SYS_SW			"1.00r000"
#endif

#define S3_SYS_MODEL		"Sentinel 3"

// ----------------------------------------------------------------------------