#pragma once

#define S3_ALARMS_ALL			0xFF
#define S3_TX_ALARMS_ALL		0xFFFF

#define S3_TX_OPT_ALARM_BYTES	3
#define S3_TX_CTRL_ALARM_BYTES	2

#define S3_RX_CTRL_ALARM_BYTES	3

// Tx alarm bits
#define S3_TX_BATT_WARN			0x0001
#define S3_TX_BATT_ALARM		0x0002
#define S3_TX_COMM_FAIL			0x0004
#define S3_TX_TEMP_COMM_FAIL	0x0008
#define S3_TX_BATT_INVALID		0x0010
#define S3_TX_RECOMP_REQ		0x0020
#define S3_TX_LASER_HI			0x0040
#define S3_TX_LASER_LO			0x0080
#define S3_TX_SELF_TEST_FAIL	0x0100
#define S3_TX_SELF_TEST_NOT_RUN	0x0200
#define S3_TX_INIT_FAIL			0x0400
#define S3_TX_OVER_TEMP			0x0800
#define S3_TX_UNDER_TEMP		0x1000
#define S3_TX_RLL_UNSTABLE		0x2000
#define S3_TX_SELF_TEST_RETRY	0x4000
#define S3_TX_NOT_ACTIVE		0x8000

// Tx battery alarms
#define S3_TX_BATT_COMM_FAIL	0x01
#define S3_TX_BATT_1			0x02
#define S3_TX_BATT_HOT			0x04
#define S3_TX_BATT_COLD			0x08
#define S3_TX_BATT_4			0x10
#define S3_TX_BATT_5			0x20
#define S3_TX_BATT_6			0x40
#define S3_TX_BATT_7			0x80

// Tx Opt alarms [0]
#define S3_TX_OPT_MAJOR			0x01
#define S3_TX_OPT_MINOR			0x02
#define S3_TX_OPT_LASER			0x04
#define S3_TX_OPT_AGC			0x08
#define S3_TX_OPT_TEC			0x10
#define S3_TX_OPT_0_5			0x20
#define S3_TX_OPT_0_6			0x40
#define S3_TX_OPT_0_7			0x80

// Tx Opt alarms [1]
#define S3_TX_OPT_VCC			0x01
#define S3_TX_OPT_BIAS			0x02
#define S3_TX_OPT_POWER			0x04
#define S3_TX_OPT_FEED_I		0x08 // Not used
#define S3_TX_OPT_PEAK			0x10
#define S3_TX_OPT_TEMP			0x20
#define S3_TX_OPT_LEVEL			0x40
#define S3_TX_OPT_GAIN			0x80

// Tx Opt alarms [2]
#define S3_TX_OPT_TEC_I			0x01
#define S3_TX_OPT_MODULE_TEMP	0x02
#define S3_TX_OPT_2_2			0x04
#define S3_TX_OPT_2_3			0x08
#define S3_TX_OPT_2_4			0x10
#define S3_TX_OPT_2_5			0x20
#define S3_TX_OPT_2_6			0x40
#define S3_TX_OPT_2_7			0x80

// Tx Ctrl alarms
#define S3_TX_CTRL_MAJOR		0x80
#define S3_TX_CTRL_MINOR		0x40

// Rx alarm bits
#define S3_RX_INT_FAIL			0x01	// Internal comms fail
#define S3_RX_OVER_TEMP			0x02
#define S3_RX_UNDER_TEMP		0x04
#define S3_RX_OVER_VOLT			0x08
#define S3_RX_UNDER_VOLT		0x10
#define S3_RX_INIT_FAIL			0x20
#define S3_RX_06				0x40
#define S3_RX_COMM_FAIL			0x80	// External (serial) comms fail

// Rx per input (Tx) alarm bits
#define S3_RX_RLL_LOW			0x01
#define S3_RX_RLL_HIGH			0x02
#define S3_RX_TX_02				0x04
#define S3_RX_TX_03				0x08
#define S3_RX_TX_04				0x10
#define S3_RX_TX_05				0x20
#define S3_RX_TX_06				0x40
#define S3_RX_TX_07				0x80

// Rx Ctrl alarms [3] 0xF8-FA
#define S3_RX_CTRL_00			0x01
#define S3_RX_CTRL_01			0x02
#define S3_RX_CTRL_02			0x04
#define S3_RX_CTRL_03			0x08
#define S3_RX_CTRL_04			0x10
#define S3_RX_CTRL_05			0x20
#define S3_RX_CTRL_MINOR		0x40
#define S3_RX_CTRL_MAJOR		0x80

#define S3_RX_CTRL_FAN_FAIL		0x01
#define S3_RX_CTRL_TX1			0x02
#define S3_RX_CTRL_TX2			0x04
#define S3_RX_CTRL_TX3			0x08
#define S3_RX_CTRL_TX4			0x10
#define S3_RX_CTRL_TX5			0x20
#define S3_RX_CTRL_TX6			0x40
#define S3_RX_CTRL_VCC_OOR		0x80

#define S3_RX_CTRL_00			0x01
#define S3_RX_CTRL_RX1			0x02
#define S3_RX_CTRL_RX2			0x04
#define S3_RX_CTRL_RX3			0x08
#define S3_RX_CTRL_RX4			0x10
#define S3_RX_CTRL_RX5			0x20
#define S3_RX_CTRL_RX6			0x40
#define S3_RX_CTRL_OPT_SW		0x80