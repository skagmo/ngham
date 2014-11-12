//**************************************************************//
// NGHam protocol - Jon Petter Skagmo, LA3JPA, 2014.            //
// Licensed under LGPL.                                         //
//**************************************************************//

#ifndef NGHAM_EXTENSIONS_H
#define NGHAM_EXTENSIONS_H

#include "stdint.h"

// Possible values for the type field. After type byte, length follows.
#define PKT_TYPE_DATA				0
#define PKT_TYPE_ADDRESS			1
#define PKT_TYPE_STATISTICS			2	// Statistics contains information about channel access / timing source
#define PKT_TYPE_STATISTICS_TIMING	3	// Same as the above, but aligned with time slot to give absolute time reference

// Additional NA-values
#define TEMP_NA				-128
#define VOLT_NA				0

// Statistics packet
typedef struct __attribute__ ((packed)){
	uint16_t hw_ver;		// 10b company, 6b product
	uint16_t serial;		// Serial nr.
	uint16_t sw_ver;		// 4b major, 4b minor, 8b build
	uint32_t uptime_s;		// In whole seconds since startup
	uint32_t tow_ms;		// Time of week in milliseconds
	uint8_t tow_source;		// Timing source. 0=None, 1=Remote, 2=Remote w/GNSS, 3=GNSS
    uint8_t voltage;		// Input voltage in desivolts (0-25.5)
    int8_t temp;			// System temp in deg. celsius (-128 to 127)
	uint8_t signal;			// Received signal strength in dBm - 200, -200 to 55
    uint8_t noise;			// Noise floor in dBm - 200, -200 to 55
    uint16_t cntr_rx_ok;	// Packets successfully received
    uint16_t cntr_rx_fix;	// Packets with corrected errors
	uint16_t cntr_rx_err;	// Packets with uncorrectable errors
    uint16_t cntr_tx;		// Packets sent
}ngham_statistics_t;

#endif