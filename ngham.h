//**************************************************************//
// NGHam protocol - Jon Petter Skagmo, LA3JPA, 2014.            //
// Licensed under LGPL.                                         //
//**************************************************************//

#ifndef NGHAM_H
#define NGHAM_H

#include <stdint.h>

#include "ngham_packets.h" // tx_pkt_t

// There are seven different sizes.
// Each size has a correlation tag for size, a total size, a maximum payload size and a parity data size.
#define NGH_SIZES	7
extern const uint8_t NGH_PL_SIZE[];			// Actual payload
extern const uint8_t NGH_PL_SIZE_FULL[];	// Size with LEN, payload and CRC
extern const uint8_t NGH_PL_PAR_SIZE[];		// Size with RS parity added
extern const uint8_t NGH_PAR_SIZE[];

extern const uint8_t NGH_SYNC[];

#define NGH_PREAMBLE_SIZE				4
#define NGH_SYNC_SIZE					4
#define NGH_PREAMBLE_SIZE_FOUR_LEVEL	8
#define NGH_SYNC_SIZE_FOUR_LEVEL		8
#define NGH_SIZE_TAG_SIZE				3
#define NGH_MAX_CODEWORD_SIZE			255
#define NGH_MAX_TOT_SIZE				(NGH_PREAMBLE_SIZE_FOUR_LEVEL+NGH_SYNC_SIZE_FOUR_LEVEL+NGH_SIZE_TAG_SIZE+NGH_MAX_CODEWORD_SIZE)

#define NGH_PADDING_bm                  0x1f
#define NGH_FLAGS_bm                    0xe0
#define NGH_FLAGS_bp                    5

void ngham_init_arrays(void);
void ngham_deinit_arrays(void);
void ngham_init(void);
void ngham_tx_push(tx_pkt_t* p);

// Takes single bytes at at time after sync word is detected
// Requires little resources, and can run in an interrupt routine
// 
// Data is placed in rx_pkts_current_buf, which is a uint8_t buffer one byte larger than rx_pkt_t
// rx_pkts_current is simply an rx_pkt_t pointer pointing at (rx_pkts_current_buf+1)

void ngham_parse(uint8_t d);
void ngham_decode(rx_pkt_t* p);
int ngham_encode(tx_pkt_t* p);

// Will decode parsed packets and encode packets ready to transmit
// Will only process one packet at a time
//void ngham_process_tick(void);

#endif
