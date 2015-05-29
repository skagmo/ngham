//**************************************************************//
// NGHam protocol - Jon Petter Skagmo, LA3JPA, 2014.            //
// Licensed under LGPL.                                         //
//**************************************************************//

#include "ngham_spp.h"

#include "ngham_packets.h"
#include "crc_ccitt.h"
#include "ngham_paths.h"
#include PATH_NGHAM_PLATFORM_SPP

// Include your command interpreter here, if you have one.
#include PATH_OWL_CMD_INTERPRETER

#define SPP_START			0x24	// Packet start byte definition

#define SPP_STATE_START 	0x00	// States
#define SPP_STATE_HEADER	0x01
#define SPP_STATE_PAYLOAD	0x02

#define SPP_TYPE_RX			0x00	// Packet types
#define SPP_TYPE_TX			0x01	// Packet types
#define SPP_TYPE_LOCAL		0x02
#define SPP_TYPE_CMD		0x03

void ngham_pack_byte(port_ctx_t* ctx, uint8_t c){
	switch(ctx->state){
		case SPP_STATE_START:
			if (c == SPP_START){
				ctx->state = SPP_STATE_HEADER;	// Start found; go to next state
				ctx->d_ip = 1;	// Starts at next first as SPP_START is already received
			}
			break;
			
		case SPP_STATE_HEADER:
			((uint8_t*)&ctx->ngh_spp)[ctx->d_ip++] = c;	// Fill header
			if (ctx->d_ip >= sizeof(ngh_spphdr_t)){
				ctx->d_ip = 0;
				ctx->state = SPP_STATE_PAYLOAD;
			}
			break;
			
		case SPP_STATE_PAYLOAD:
			if (ctx->d_ip < PORT_BUF_SIZE) ctx->d[ctx->d_ip++] = c;	// Fill ctx->d with payload
			if (ctx->d_ip >= ctx->ngh_spp.pl_len){
				ctx->state = SPP_STATE_START;

				// Calc CRC of type, length and payload
				int j;
				uint16_t crc = 0xffff;
				for (j=3; j<5; j++) crc = crc_ccitt_byte(((uint8_t*)&ctx->ngh_spp)[j], crc);
				for (j=0; j<ctx->d_ip; j++) crc = crc_ccitt_byte(ctx->d[j], crc);
				crc ^= 0xffff;
				if (crc == ctx->ngh_spp.crc){
					switch(ctx->ngh_spp.pl_type){
						case SPP_TYPE_TX:
							{
								// Data to be sent
								tx_pkt_t p;
								tx_pkt_init(&p);
								p.ngham_flags = ctx->d[0];	// First byte in NGH SPP RF type is flags
								p.pl_len = ctx->d_ip-1;
								for (j=0; j<p.pl_len; j++) p.pl[j] = ctx->d[j+1];
								PACKER_CALL(&p);
								// TODO: Generate response!
							}
							break;
						case SPP_TYPE_CMD:
							{
								uint8_t rep[REPLY_SIZE];	// CMD can be longer than SPP_PL_MAX
								uint16_t rep_len = 0, rep_pos = 0;
								cmd(ctx->d, ctx->ngh_spp.pl_len, rep, &rep_len, REPLY_SIZE);	// Run command

								// Prepare response
								ngh_spphdr_t hdr;
								uint8_t r[sizeof(hdr)+SPP_PL_MAX];
								hdr.start = SPP_START;
								hdr.pl_type = SPP_TYPE_CMD;

								while (rep_len){
									if (rep_len > 255){
										hdr.pl_len = 255;
										rep_len -= 255;
									}
									else{
										hdr.pl_len = rep_len;
										rep_len = 0;
									}
									for (j=0; j<hdr.pl_len; j++) r[sizeof(hdr)+j] = rep[rep_pos++];
									for (j=3; j<5; j++) r[j] = ((uint8_t*)&hdr)[j];		// Copy length and type into header
									hdr.crc = crc_ccitt(&r[3], hdr.pl_len+2);			// Calc. CRC
									for (j=0; j<3; j++) r[j] = ((uint8_t*)&hdr)[j];		// Copy CRC and stat into header
									port_output(ctx, r, sizeof(hdr)+hdr.pl_len);		// Send to port
								}
							}
							break;
					}
				}
			}
			break;
	}
}

void ngham_unpack(rx_pkt_t* p){
	int j;
	ngh_spphdr_t hdr;
	hdr.start = SPP_START;
	hdr.crc = 0xffff;
	hdr.pl_type = SPP_TYPE_RX;
	hdr.pl_len = p->pl_len+8;
	uint8_t* p_u8 = (uint8_t*)p;
	uint8_t* hdr_u8 = (uint8_t*)&hdr;
	// Calc CRC
	for (j=3; j<5; j++) hdr.crc = crc_ccitt_byte(hdr_u8[j], hdr.crc);
	for (j=0; j<hdr.pl_len; j++) hdr.crc = crc_ccitt_byte(p_u8[j], hdr.crc);	// SPP-protocol must match with rx_pkt_t struct
	hdr.crc ^= 0xffff;

	// Copy remaining and send to port
	UNPACKER_CALL(PACKER_NGHAM, (uint8_t*)&hdr, sizeof(ngh_spphdr_t));
	UNPACKER_CALL(PACKER_NGHAM, p_u8, hdr.pl_len);
}
