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
//			Also, correcting it caused failure to track certain gain changes.
// 1.03.004 
// 1.03.005 Rx+ support added, mods and fixes
// 1.04.000 Release to production
// 1.04.001 Release to BAE
// 1.04.002 Added Rx+ compression/sensitivity look-ups
// 1.04.003 Fixed read of on-charge battery S/N & P/N
//			Added protection for selecting -ve active Tx & IP
// 1.05.000 Tx8 input bug fixed
//			Added remote commands
//			USB response terminator changed (temporary)
// 1.05.001 06/12/18 CR3961
//			Corrected response to multiple Tx8 input select commands
//			Corrected response to double tap of Rx6 active transmitter buttons
//			Reduced FOL switching time 
//			Fixed possible deadlock on sleep all/wake all screen
//			Report Rx+ types correctly in reporting commands
//			Improved handling of multiple test-tone switching remote commands
//			Improved robustness of image and application update processes
//			Minor bug fixes and enhancements
//

// 'c': suffix for candidate releases
#define S3_SYS_SW			"1.05.001"

#define S3_SYS_MODEL		"Sentinel 3"

// ----------------------------------------------------------------------------