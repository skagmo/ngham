//**************************************************************//
// NGHam protocol - Jon Petter Skagmo, LA3JPA, 2014.            //
// Licensed under LGPL.                                         //
//**************************************************************//

#ifndef NGHAM_PACKETS_H
#define NGHAM_PACKETS_H

#include "stdint.h"

// Possible packet conditions
#define PKT_CONDITION_OK		0	// Successfully received packet
#define PKT_CONDITION_FAIL		1	// Receiption failed after receiving the packet
#define PKT_CONDITION_PREFAIL	2	// Reception failed before receiving the whole packet

// Packet priority in transmission
#define PKT_PRIORITY_NORMAL			0
#define PKT_PRIORITY_FIRST_IN_SLOT	10	// Should be put first in the next time slot for timing purposes

// Size of payload in packet struct
#define PKT_PL_SIZE	256

// Basic "not available"-values
#define RSSI_NA				0
#define TIMESTAMP_NA		0 // TODO: Can actually be a valid time stamp

// If the following flag is set in a packet, NGHam extensions are used and first byte is type
#define NGHAM_FLAG_TYPE_EXTENSION	0x01

typedef struct{
	uint16_t pl_len;
	uint8_t pl[PKT_PL_SIZE];
	uint8_t ngham_flags;
	uint8_t rssi;			// In dBm + 200
	uint8_t noise;			// Same as above
	uint8_t errors;			// Recovered symbols
	uint32_t timestamp;		// Time stamp of sync word detection
}rx_pkt_t;

typedef struct{
	uint16_t pl_len;
	uint8_t pl[PKT_PL_SIZE];
	uint8_t ngham_flags;
	uint8_t priority;
}tx_pkt_t;

void rx_pkt_init(rx_pkt_t *p);
void tx_pkt_init(tx_pkt_t *p);

#endif