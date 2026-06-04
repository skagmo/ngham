//**************************************************************//
// NGHam protocol - Jon Petter Skagmo, LA3JPA, 2014.            //
// Licensed under LGPL.                                         //
//**************************************************************//

#pragma once

#include <stdint.h>

#define NGHAM_SPP_TYPE_RX		0x00	// Packet types
#define NGHAM_SPP_TYPE_TX		0x01	// Packet types
#define NGHAM_SPP_TYPE_LOCAL	0x02
#define NGHAM_SPP_TYPE_CMD		0x03

#define SPP_D_SIZE (255+5)

// Packet start byte definition
#define SPP_START			0x24	

typedef struct __attribute__ ((packed)){
	uint8_t d[SPP_D_SIZE];
	uint16_t d_len;
	uint16_t d_target;
	uint8_t state;
	void (*data_cb)(uint8_t, uint8_t*, uint16_t);
}spp_ctx_t;

// NGHam SPP header
typedef struct __attribute__ ((packed)){
	uint8_t start;
	uint16_t crc;
	uint8_t pl_type;
	uint8_t pl_len;
}ngh_spphdr_t;

typedef struct __attribute__ ((packed)){
	uint32_t timestamp_toh_us;		// Time stamp of sync word detection
	uint8_t noise;			// Same as above
	uint8_t rssi;			// In dBm + 200
	uint8_t errors;			// Recovered symbols
	uint8_t ngham_flags;
}spp_rxhdr_t;

typedef struct __attribute__ ((packed)){
	uint8_t ngham_flags;
}spp_txhdr_t;

void ngham_spp_parse(spp_ctx_t* ctx, uint8_t c);
void ngham_spp_fill_header(ngh_spphdr_t* hdr, uint8_t type, const uint8_t* d, uint16_t d_len);
void ngham_spp_init(spp_ctx_t* ctx, void(*cb)(uint8_t,uint8_t*,uint16_t));
