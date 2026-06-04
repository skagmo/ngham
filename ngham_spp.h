//**************************************************************//
// NGHam protocol - Jon Petter Skagmo, LA3JPA, 2014.            //
// Licensed under LGPL.                                         //
//**************************************************************//

#include <stdint.h>

#include "ngham_packets.h"
#include "ngham_paths.h"
#include PATH_NGHAM_PLATFORM_SPP

#define NGHAM_SPP_TYPE_RX		0x00	// Packet types
#define NGHAM_SPP_TYPE_TX		0x01	// Packet types
#define NGHAM_SPP_TYPE_ROUTED	0x02
#define NGHAM_SPP_TYPE_CMD		0x03

// NGHam SPP header
typedef struct __attribute__ ((packed)){
	uint8_t start;
	uint16_t crc;
	uint8_t pl_type;
	uint8_t pl_len;
}ngh_spphdr_t;

void spp_parse(port_ctx_t* ctx, uint8_t c);
void spp_fill_header(ngh_spphdr_t* hdr, uint8_t type, uint8_t* d, uint16_t d_len);
void spp_print_cmd(port_ctx_t* ctx, uint8_t* d, uint16_t d_len);
void spp_print_rx_pkt(port_ctx_t* ctx, rx_pkt_t* p);
void spp_print_local_pkt(port_ctx_t* ctx, tx_pkt_t* p);