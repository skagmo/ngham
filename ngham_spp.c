//**************************************************************//
// NGHam protocol - Jon Petter Skagmo, LA3JPA, 2014.            //
// Licensed under LGPL.                                         //
// This is made somewhat platform specific for the Owl VHF      //
// with serial port contexts, and is meant as an guidance.      //
//**************************************************************//

#include "ngham_spp.h"
#include "crc_ccitt.h"
#include <string.h> // For memcpy
// Definition of port context port_ctx_t and port_* functions,
// as well as packer_call which sends data to transmit chain
#include "ngham_paths.h"
#include PATH_NGHAM_PLATFORM_SPP

// Packet start byte definition
#define SPP_START			0x24	

// States
#define SPP_STATE_START 	0x00	
#define SPP_STATE_HEADER	0x01
#define SPP_STATE_PAYLOAD	0x02

void spp_parse(port_ctx_t* ctx, uint8_t c){
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
				// Target length in d_op
				ctx->pkt_target = ((ngh_spphdr_t*)ctx->d)->pl_len;
				if (ctx->pkt_target){
					ctx->pkt.pl_len = 0;
					ctx->state = SPP_STATE_PAYLOAD;
					// Start filling in the byte before payload, for TX packet flags
					ctx->ptr = (uint8_t*)&(ctx->pkt.ngham_flags);
				}
				else ctx->state = SPP_STATE_START;
			}
			break;
			
		case SPP_STATE_PAYLOAD:
			// Fill ctx->d with payload, limit of 220+1 with respect to RX packet
			if (ctx->pkt.pl_len<PKT_PL_SIZE) ctx->ptr[ctx->pkt.pl_len++] = c;
			else ctx->state = SPP_STATE_START;
            
			// If received length has met target length (set in STATE_HEADER)
			if (ctx->pkt.pl_len == ctx->pkt_target){
				ngh_spphdr_t* hdr = (ngh_spphdr_t*)ctx->d;
                
                uint16_t j, crc = 0xffff;
                crc = crc_ccitt_byte(hdr->pl_type, crc);
                crc = crc_ccitt_byte(hdr->pl_len, crc);
                for (j=0; j<ctx->pkt.pl_len; j++) crc = crc_ccitt_byte(ctx->ptr[j], crc);
                crc ^= 0xffff;

				if (crc == hdr->crc){
					switch(hdr->pl_type){
						
						// Data to be sent
						case NGHAM_SPP_TYPE_TX:
							{
                                // Flags already included, but length is one too long
								ctx->pkt.pl_len -= 1;
								// Length checked in packer
                                /* Don't transmit SPP data if port is muted */
                                if (config.port_rf_mute == 0) packer_call(&(ctx->pkt));
							}
							break;
							
						// Command
						case NGHAM_SPP_TYPE_CMD:
							{
								uint8_t rep[REPLY_SIZE];	// CMD can be longer than SPP_PL_MAX
								uint16_t rep_len = 0;
								cmd(ctx->ptr, hdr->pl_len, rep, &rep_len, REPLY_SIZE, 1<<ctx->uart_nr);	// Run command
								str_cpycl(rep, &rep_len, REPLY_SIZE, "\r\n");
								spp_print_cmd(ctx, rep, rep_len);	// Send reply
							}
							break;
							
						case NGHAM_SPP_TYPE_ROUTED:
							{
                                // Flags already included, but length is one too long
								ctx->pkt.pl_len -= 1;
								
								if (ctx->pkt.ngham_flags == NGHAM_FLAG_TYPE_EXTENSION)
									// Use bitshifting to map UART0 to PORT_0_bm etc.
									port_handle_ngham_ext(1 << ctx->uart_nr, &(ctx->pkt));
							}
							break;
					}
				}
				
				ctx->state = SPP_STATE_START;
			}
			break;
	}
}

void spp_fill_header(ngh_spphdr_t* hdr, uint8_t type, uint8_t* d, uint16_t d_len){
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

void spp_print_cmd(port_ctx_t* ctx, uint8_t* d, uint16_t d_len){
	ngh_spphdr_t hdr;
	uint16_t shortened_len, offset = 0;
	
	// Split into multiple packets if necessary
	while (d_len){
		if (d_len > SPP_PL_MAX) shortened_len = SPP_PL_MAX;
		else shortened_len = d_len;
		
		spp_fill_header(&hdr, NGHAM_SPP_TYPE_CMD, d+offset, shortened_len);
		
		// Copy remaining and send to port
		port_output(ctx, (uint8_t*)&hdr, sizeof(ngh_spphdr_t));
		port_output(ctx, d+offset, hdr.pl_len);
		
		d_len -= shortened_len;
		offset += shortened_len;
	}
}

// Output buffer should be prefilled with 
void spp_print_rx_pkt(port_ctx_t* ctx, rx_pkt_t* p){
	ngh_spphdr_t hdr;
	
	spp_fill_header(&hdr, NGHAM_SPP_TYPE_RX, (uint8_t*)&(p->timestamp_toh_us), p->pl_len+8);
	port_unpacker_output(ctx, (uint8_t*)&hdr, sizeof(ngh_spphdr_t));
	port_unpacker_output(ctx, (uint8_t*)&(p->timestamp_toh_us), hdr.pl_len);
}

void spp_print_local_pkt(port_ctx_t* ctx, tx_pkt_t* p){
	ngh_spphdr_t hdr;
	spp_fill_header(&hdr, NGHAM_SPP_TYPE_ROUTED, (uint8_t*)&(p->ngham_flags), p->pl_len+1);
	port_unpacker_output(ctx, (uint8_t*)&hdr, sizeof(ngh_spphdr_t));
	port_unpacker_output(ctx, (uint8_t*)&(p->ngham_flags), hdr.pl_len);
}
