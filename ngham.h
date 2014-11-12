//**************************************************************//
// NGHam protocol - Jon Petter Skagmo, LA3JPA, 2014.            //
// Licensed under LGPL.                                         //
//**************************************************************//

#ifndef NGHAM_H
#define NGHAM_H

#include <stdint.h>

#include "ngham_packets.h" // tx_pkt_t

extern const uint8_t NGH_SYNC[];

#define NGH_PREAMBLE_SIZE		4
#define NGH_SYNC_SIZE			4
#define NGH_SIZE_TAG_SIZE		3
#define NGH_MAX_CODEWORD_SIZE	255
#define NGH_CODEWORD_START		(NGH_PREAMBLE_SIZE+NGH_SYNC_SIZE+NGH_SIZE_TAG_SIZE)
#define NGH_MAX_TOT_SIZE		(NGH_PREAMBLE_SIZE+NGH_SYNC_SIZE+NGH_SIZE_TAG_SIZE+NGH_MAX_CODEWORD_SIZE)

void ngham_init_arrays(void);
void ngham_init(void);
void ngham_encode(tx_pkt_t* p);
void ngham_decode(uint8_t d);

#endif
