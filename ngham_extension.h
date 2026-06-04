//**************************************************************//
// NGHam protocol - Jon Petter Skagmo, LA3JPA, 2014.            //
// Licensed under LGPL.                                         //
//**************************************************************//

#pragma once

#include <stdint.h>
#include "ngham_packets.h"

// NGHam extension is a suggested use of the NGHam RF packet payload
// An RF packet can contain multiple NGHam extension packets.
// Each NGHam extension packet is built up like this:
// 1 byte type, 1 byte length, [length] bytes payload, for example
// 0x07, 0x04, 'H', 'E', 'L', 'P'
// Which corresponds to a EXT_TYPE_CMD_REQ of length 4 with payload "HELP"
// To send an extension packet, the NGHam RF packet must have the NGHam Extension flag set.
// (LSB bit in the byte before payload)

// Possible values for the type field. After type byte, length follows.
#define EXT_TYPE_DATA		0
#define EXT_TYPE_SRC		1
#define EXT_TYPE_STAT		2
#define EXT_TYPE_SIMPLEHOP	3
#define EXT_TYPE_POS		4
#define EXT_TYPE_TOH		5
#define EXT_TYPE_DEST		6 // Destination/receiver callsign
#define EXT_TYPE_CMD_REQ	7 // Command packet
#define EXT_TYPE_TEXT   	8 // Printable text, such as command reply
#define EXT_TYPE_REQUEST	9
#define EXT_TYPE_CAR_BAT	10
#define EXT_TYPE_CAR_CHARGE	11
#define EXT_TYPE_CAR_TRIP	12
#define EXT_TYPE_RX_SRC     13
#define EXT_TYPE_IPV4_SRC   14
#define EXT_TYPE_IPV6_SRC   15
#define EXT_TYPE_CAR_CHG_V2 16
#define EXT_TYPES 17
#define EXT_SIZE_VARIABLE 0xffff

extern const char* EXT_TYPE_STRINGS[];
extern const uint16_t EXT_TYPE_SIZES[];

// Additional NA-values
#define TEMP_NA		0xff
#define VOLT_NA		0xff
#define UINT8_NA	0xff
#define INT32_NA 	0x7fffffff
#define COG_NA 		0x7ff

// Ensure compatibility when used with MinGW etc.
#ifdef _WIN32
#define ATTRIBUTE_PACKED __attribute__ ((packed,gcc_struct))
#else
#define ATTRIBUTE_PACKED __attribute__ ((packed))
#endif

// TOH packet
typedef struct ATTRIBUTE_PACKED{
	uint32_t toh_us;			// Time of hour in microseconds
	uint8_t toh_val;			// Validity
}ngham_toh_t;

// Statistics packet
typedef struct ATTRIBUTE_PACKED{
	uint16_t hw_ver;			// 10b company, 6b product
	uint16_t serial;			// Serial nr.
	uint16_t sw_ver;			// 4b major, 4b minor, 8b build
	uint32_t uptime_s;			// Time in whole seconds since startup
	uint8_t voltage;			// Input voltage in desivolts (0-25.5)
	int8_t temp;				// System temp in deg. celsius (-128 to 127)
	uint8_t signal;				// Received signal strength in dBm - 200, -200 to 54 (0xff=N/A)
	uint8_t noise;				// Noise floor, same as above
	uint16_t cntr_rx_ok;		// Packets successfully received
	uint16_t cntr_rx_fix;		// Packets with corrected errors
	uint16_t cntr_rx_err;		// Packets with uncorrectable errors
	uint16_t cntr_tx;			// Packets sent
}ngham_stat_t;

// Position packet
typedef struct ATTRIBUTE_PACKED{
	int32_t latitude;          // In degrees * 10^7
	int32_t longitude;         // In degrees * 10^7
	int32_t altitude;          // In centimeters
	unsigned int sog:20;     // Hundreds of meters per second
	unsigned int cog:12;   // Tenths of degrees
	uint8_t hdop;              // In tenths
}ngham_pos_t;

// Always first in a packet, except when resent by another station.
typedef struct ATTRIBUTE_PACKED{
	uint8_t call_ssid[6]; // 7 x 6 bit (SIXBIT DEC, which is ASCII-32 and limited to 0-64) empty characters padded with 0, 6 bit SSID
	uint8_t sequence;    // Wraps around from 255 to 0
}ngham_src_t;

typedef struct ATTRIBUTE_PACKED{
	uint8_t call_ssid[6]; // 7 x 6 bit (SIXBIT DEC, which is ASCII-32 and limited to 0-64) empty characters padded with 0, 6 bit SSID
}ngham_dest_t;

typedef struct ATTRIBUTE_PACKED{
	unsigned int reserved:2;
	unsigned int hops_total:3;
	unsigned int hops_remaining:3;
}ngham_simplehop_t;

typedef struct ATTRIBUTE_PACKED{
	uint8_t soc;			// State of charge, percent
	signed int power : 24;	// In watt, +/- 8388 kW
}ngham_car_battery_t;

typedef struct ATTRIBUTE_PACKED{
	uint8_t volt;			// Charger input voltage
	uint8_t amp;			// Charger input current, tenth of amps
}ngham_car_charge_t;

typedef struct ATTRIBUTE_PACKED{
	uint8_t speed_kmh;
	signed int trip_energy : 24;	// Energy used in Wh, +/- 8388 kWh
	unsigned int trip_time : 24;	// In seconds, max 194 days
	unsigned int trip_dist : 24;	// In meters, max 16777 km
}ngham_car_trip_t;

typedef struct ATTRIBUTE_PACKED{
	uint32_t timestamp_toh_us;	// Time stamp of sync word detection
	uint8_t noise;				// Same as above
	uint8_t rssi;				// In dBm + 200
	uint8_t errors;				// Recovered symbols
}ngham_rx_src_t;

typedef struct ATTRIBUTE_PACKED{
	uint8_t ipv4[4];
}ngham_ipv4_src_t;

typedef struct ATTRIBUTE_PACKED{
	uint8_t ipv6[16];
}ngham_ipv6_src_t;

typedef struct ATTRIBUTE_PACKED{
	uint8_t soc;			// State of charge, percent
	signed int power : 24;	// In watt, +/- 8388 W
	uint16_t energy;		// In Wh*10
}ngham_car_battery_v2_t;

typedef struct ATTRIBUTE_PACKED{
	uint8_t volt;			// Charger input voltage
	uint8_t amp;			// Charger input current, tenth of amps
	uint16_t energy;		// In Wh*10
}ngham_car_charge_v2_t;


uint16_t ngh_ext_numpkts(uint8_t* d, uint16_t d_len);
uint8_t ngh_ext_encode_callsign(uint8_t* enc_callsign, char* callsign);
void ngh_ext_decode_callsign(char* callsign, uint8_t* enc_callsign);

uint8_t* ngh_ext_allocate_pkt(tx_pkt_t* p, uint8_t pkt_type, uint16_t data_len);
void ngh_ext_append_pkt(tx_pkt_t* p, uint8_t type, uint8_t* data, uint16_t size);
uint8_t* ngh_ext_allocate_buffer(uint8_t* d, uint16_t* d_len, uint16_t d_size, uint8_t pkt_type, uint16_t data_len);
void ngh_ext_append_buffer(uint8_t* d, uint16_t* d_len, uint16_t d_size, uint8_t type, uint8_t* data, uint16_t size);
