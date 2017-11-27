// ----------------------------------------------------------------------------
// Sentinel 3 controller configurations - inventory details

// Reference only
// #define S3_DESKTOP_RACK		"S3K-DT-1-AC-00"
// #define S3_19_RACK			"S3K-19-1-AC-00"


// Major	- rewrite/redesign of system.
// Minor	- add/changes to functionality, HB update required.
// Revision - implementation changes, bug fixes committed, no user changes.

// 1.02.000 R&S release
// 1.03.001 R&S Added static IP assignment to application only
// 1.03.002 As 1.03.001 but new image build
// 1.03.003 Ignore TxOpt 0xCC:0 triggered by peak detection which is not sorted.
//			Also, correcting it caused failure to trac certain gain changes.

#define S3_SYS_SW			"1.03r003"

#define S3_SYS_MODEL		"Sentinel 3"

// ----------------------------------------------------------------------------