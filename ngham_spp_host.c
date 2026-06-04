//**************************************************************//
// NGHam protocol - Jon Petter Skagmo, LA3JPA, 2014.            //
// Licensed under LGPL.                                         //
//**************************************************************//

#include "ngham_spp_host.h"
#include "crc_ccitt.h"
#include <string.h> // For memcpy

#include <stdio.h>

// States
#define SPP_STATE_START 	0x00	
#define SPP_STATE_HEADER	0x01
#define SPP_STATE_PAYLOAD	0x02

void ngham_spp_parse(spp_ctx_t* ctx, uint8_t c){
	switch(ctx->state){
		case SPP_STATE_START:
			if (c == SPP_START){
				ctx->state = SPP_STATE_HEADER;	// Start found; go to next state
                ctx->d_len = 1; // Starts at next first as SPP_START is already received
			}
			break;
			
		case SPP_STATE_HEADER:
			// Fill ctx->d with header - no check for size, as buffer is much larger than header (5B)
            ctx->d[ctx->d_len++] = c;
			
			if (ctx->d_len >= sizeof(ngh_spphdr_t)){
				ctx->d_target = sizeof(ngh_spphdr_t) + ((ngh_spphdr_t*)ctx->d)->pl_len;
				ctx->state = SPP_STATE_PAYLOAD;
			}
			break;
			
		case SPP_STATE_PAYLOAD:
			// Fill ctx->d with payload, limit
			if (ctx->d_len < SPP_D_SIZE) ctx->d[ctx->d_len++] = c;
			else ctx->state = SPP_STATE_START;
            
			// If received length has met target length (set in STATE_HEADER)
			if (ctx->d_len == ctx->d_target){
				ngh_spphdr_t* hdr = (ngh_spphdr_t*)ctx->d;
                ctx->state = SPP_STATE_START;

				uint16_t crc_calc = crc_ccitt(ctx->d+3, ctx->d_len-3);

				if (hdr->crc == crc_calc)
					ctx->data_cb(hdr->pl_type, ctx->d+sizeof(ngh_spphdr_t), ctx->d_len-sizeof(ngh_spphdr_t));
			}
			break;
	}
}

void ngham_spp_fill_header(ngh_spphdr_t* hdr, uint8_t type, const uint8_t* d, uint16_t d_len){
	uint16_t j, crc;
	
	hdr->start = SPP_START;
	hdr->pl_type = type;
	hdr->pl_len = d_len;
	
	crc = crc_ccitt_byte(hdr->pl_type, 0xffff);
	crc = crc_ccitt_byte(hdr->pl_len, crc);
	for (j=0; j<d_len; j++) crc = crc_ccitt_byte(d[j], crc);
	crc ^= 0xffff;
	
	hdr->crc = crc;
}

void ngham_spp_init(spp_ctx_t* ctx, void(*cb)(uint8_t,uint8_t*,uint16_t)){
	ctx->data_cb = cb;
	ctx->state = SPP_STATE_START;
}
