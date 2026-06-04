//**************************************************************//
// NGHam protocol - Jon Petter Skagmo, LA3JPA, 2014.            //
// Licensed under LGPL.                                         //
//**************************************************************//

#include "ngham.h"

#include "fec-3.0.1/fec.h"			// RS code from Phil Karn
#include "fec-3.0.1/char.h"			// RS code specific to 8-bit symbol size
#include "fec-3.0.1/rs-common.h"	// RS control block struct

#include <stddef.h>					// For NULL etc.
#include <string.h>					// For memcpy
#include <stdlib.h>					// For free

#include "ccsds_scrambler.h"		// Pre-generated array from scrambling polynomial
#include "ngham_packets.h"			// Structs for TX and RX packets
#include "crc_ccitt.h"
#include "ngham_buffers.h"			// Optional ring buffers for packets pending processing

// Platform specific functions
#include "platform_ngham.h"

const uint8_t NGH_PL_SIZE[] =      {28,   60,   92,   124,  156,  188,  220};		// Actual payload
const uint8_t NGH_PL_SIZE_FULL[] = {31,   63,   95,   127,  159,  191,  223};		// Size with LEN, payload and CRC
const uint8_t NGH_PL_PAR_SIZE[] =  {47,   79,   111,  159,  191,  223,  255};		// Size with RS parity added
const uint8_t NGH_PAR_SIZE[] =     {16,   16,   16,   32,   32,   32,   32};

// Decoder states
#define NGH_STATE_SIZE_TAG		0
#define NGH_STATE_SIZE_TAG_2	1
#define NGH_STATE_SIZE_TAG_3	2
#define NGH_STATE_SIZE_KNOWN	3
#define NGH_STATE_STATUS		4
#define NGH_STATE_STATUS_2		5

// The seven different size tag vectors
const uint32_t NGH_SIZE_TAG[] = {
0b001110110100100111001101,
0b010011011101101001010111,
0b011101101001001110011010,
0b100110111011010010101110,
0b101000001111110101100011,
0b110101100110111011111001,
0b111011010010011100110100
};

// Maximum number of errors in the size tag
#define NGH_SIZE_TAG_MAX_ERROR	6

// Preamble and synchronization vector
const uint8_t NGH_PREAMBLE = 0xAA;
const uint8_t NGH_SYNC[] = {0x5D, 0xE6, 0x2A, 0x7E};
const uint8_t NGH_PREAMBLE_FOUR_LEVEL = 0xDD;
const uint8_t NGH_SYNC_FOUR_LEVEL[] = {0x77, 0xf7, 0xfd, 0x7d, 0x5d, 0xdd, 0x7f, 0xfd};

// Reed Solomon control blocks for the different NGHAM sizes
struct rs rs_cb[NGH_SIZES];

void ngham_init(void){
	decoder_state = NGH_STATE_SIZE_TAG;
	ngham_rx_buffer_init();
	ngham_tx_buffer_init();
}

// Run only once - generates reed solomon tables for all 7 packet sizes
// MM=8, genpoly=0x187, fcs=112, prim=11, nroots=32 or 16
void ngham_init_arrays(void){
	unsigned int j;
	
	struct rs* rs_16 = (void*)init_rs_char(8, 0x187, 112, 11, 16, 0);
	for (j=0; j<3; j++)
		memcpy ((void*)&rs_cb[j], (void*)rs_16, sizeof (rs_cb[j]));
	free(rs_16);
	
	struct rs* rs_32 = (void*)init_rs_char(8, 0x187, 112, 11, 32, 0);
	for (; j<NGH_SIZES; j++)
		memcpy ((void*)&rs_cb[j], (void*)rs_32, sizeof (rs_cb[j]));
	free(rs_32);

	// Set padding size for each packet size
	for (j=0; j<NGH_SIZES; j++)
		rs_cb[j].pad = NGH_PL_PAR_SIZE[6]-NGH_PL_PAR_SIZE[j];
}

void ngham_deinit_arrays(void){
	free_rs_char(&rs_cb[0]);	// Free memory for nroots = 16
	free_rs_char(&rs_cb[3]); // Free memory for nroots = 32
}

void ngham_decode(rx_pkt_t* p){
	// Run Reed Solomon decoding, calculate packet length		
	uint8_t* buf = (uint8_t*)&(p->ngham_flags);
	int8_t errors = decode_rs_char(&rs_cb[p->rs_size_nr], buf, 0, 0);
	p->pl_len = NGH_PL_SIZE[p->rs_size_nr] - (buf[0] & NGH_PADDING_bm);
	
	// Check if the packet is decodeable and then if CRC is OK
	if ( (errors != -1) &&
		 (crc_ccitt(buf, p->pl_len+1) == ((buf[p->pl_len+1]<<8) | buf[p->pl_len+2])) ){

		// Copy remaining fields and pass on
		p->timestamp_toh_us = ngham_platform_convert_time(p->timestamp_raw);
		p->errors = errors;
		p->ngham_flags = (buf[0] & NGH_FLAGS_bm) >> NGH_FLAGS_bp;
		ngham_platform_handle_packet(PKT_CONDITION_OK, p);
	}
	// If packet decoding not was successful, count this as an error
	else ngham_platform_handle_packet(PKT_CONDITION_FAIL, NULL);
}

// Packets to be transmitted are passed to this function - max. length 220 B
int ngham_encode(tx_pkt_t* p){
	uint16_t j;
	uint16_t crc;
	uint8_t size_nr = 0;
	uint8_t d[NGH_MAX_TOT_SIZE];
	uint16_t d_len = 0;
	uint8_t codeword_start;
	
	// Check size and find control block for smallest possible RS codeword
	if ((p->pl_len == 0) || (p->pl_len > NGHAM_PL_MAX)) return 0;
	while (p->pl_len > NGH_PL_SIZE[size_nr]) size_nr++;

	// Insert preamble, sync and size-tag
	if (NGHAM_FOUR_LEVEL_MODULATION){
		codeword_start = NGH_PREAMBLE_SIZE_FOUR_LEVEL+NGH_SYNC_SIZE_FOUR_LEVEL+NGH_SIZE_TAG_SIZE;
		for (j=0; j<NGH_PREAMBLE_SIZE_FOUR_LEVEL; j++) d[d_len++] = NGH_PREAMBLE_FOUR_LEVEL;
		for (j=0; j<NGH_SYNC_SIZE_FOUR_LEVEL; j++) d[d_len++] = NGH_SYNC_FOUR_LEVEL[j];
	}
	else{
		codeword_start = NGH_PREAMBLE_SIZE+NGH_SYNC_SIZE+NGH_SIZE_TAG_SIZE;
		for (j=0; j<NGH_PREAMBLE_SIZE; j++) d[d_len++] = NGH_PREAMBLE;
		for (j=0; j<NGH_SYNC_SIZE; j++) d[d_len++] = NGH_SYNC[j];
	}
	d[d_len++] = (NGH_SIZE_TAG[size_nr] >> 16) & 0xff;
	d[d_len++] = (NGH_SIZE_TAG[size_nr] >> 8) & 0xff;
	d[d_len++] = NGH_SIZE_TAG[size_nr] & 0xff;

	// Prepare content of codeword
	d[d_len] = (NGH_PL_SIZE[size_nr] - p->pl_len) & 0x1f;	// Insert padding size
	d[d_len] |= (p->ngham_flags << 5) & 0xe0;				// Insert flags
	d_len++;
	for (j=0; j<p->pl_len; j++) d[d_len++] = p->pl[j];		// Insert data
	crc = crc_ccitt(&d[codeword_start], p->pl_len+1);	// Insert CRC
	d[d_len++] = (crc >> 8) & 0xff;
	d[d_len++] = crc & 0xff;
	for (j=p->pl_len+3; j<NGH_PL_SIZE_FULL[size_nr]; j++) d[d_len++] = 0;	// Insert padding
	
	// Generate parity data
	encode_rs_char(&rs_cb[size_nr], &d[codeword_start], &d[d_len]);
	d_len += NGH_PAR_SIZE[size_nr];

	// Scramble
	for (j=0; j<NGH_PL_PAR_SIZE[size_nr]; j++) d[codeword_start+j] ^= ccsds_poly[j];

	ngham_platform_send_data(d, d_len, p->priority);
	return 1;
}

// Used to check if hamming distance in size tag is smaller than treshold
uint8_t ngham_tag_check(uint32_t x, uint32_t y){
	uint8_t j, distance;
	uint32_t diff;
	diff = x^y;
	if (!diff) return 1;	// Early check to save time

	distance = 0;
	for (j=0; j<24; j++){
		if (diff & 0x01){
			distance++;
			if (distance > NGH_SIZE_TAG_MAX_ERROR) return 0;
		}
		diff >>= 1;
	}
	return 1;
}

void ngham_parse(uint8_t d){
    static unsigned int length;
    static uint32_t size_tag;
	// Buffer for currently parsed packet
	static rx_pkt_t rx_buf;
	// Pointer to first byte in code word
	static uint8_t* code_word = (uint8_t*)&(rx_buf.ngham_flags);

	switch (decoder_state){
		case NGH_STATE_SIZE_TAG:
			size_tag = 0;
			// Intentional fallthrough
			
		case NGH_STATE_SIZE_TAG_2:
			size_tag <<= 8;
			size_tag |= d;
			decoder_state++;
			break;

		case NGH_STATE_SIZE_TAG_3:
			size_tag <<= 8;
			size_tag |= d;
			{
				for (rx_buf.rs_size_nr=0; rx_buf.rs_size_nr<NGH_SIZES; rx_buf.rs_size_nr++){
					// If tag is intact, set known size and proceed
					if (ngham_tag_check(size_tag, NGH_SIZE_TAG[rx_buf.rs_size_nr])){
						decoder_state = NGH_STATE_SIZE_KNOWN;
						length = 0;
						//rx_pkt_init(&rx_buf); // Initialize everything but rs_size_nr
						
						// Get raw clock time and RSSI from time of sync word detection
						// Time must be converted to actual time of hour (timestamp_toh_us) later!
						rx_buf.timestamp_raw = ngham_platform_get_raw_sync_time();
						rx_buf.rssi = ngham_platform_get_rssi();

						// Set desired packet size as soon as possible
						ngham_platform_set_packet_size(NGH_PL_PAR_SIZE[rx_buf.rs_size_nr]+NGH_SIZE_TAG_SIZE);
						break;
					}
				}
				// If size tag is not found, every size can theoretically be attempted
				if (decoder_state != NGH_STATE_SIZE_KNOWN){
					decoder_state = NGH_STATE_SIZE_TAG;
					ngham_platform_handle_packet(PKT_CONDITION_FAIL, NULL);
                    ngham_platform_reinit();
				}
			}
			break;

		case NGH_STATE_SIZE_KNOWN:
			// De-scramble byte and append to buffer
			code_word[length] = d^ccsds_poly[length];
			length++;

			if (length == NGH_PL_PAR_SIZE[rx_buf.rs_size_nr]){			
				// Can be sent directly to ngham_decode (will take some time)
				// or to ngham_rx_push for later processing
                ngham_platform_handle_decoded_pkt(&rx_buf);
				ngham_platform_set_packet_size(255); // Revert to maximum size, as it might be (255+3) % 256 = 2
				decoder_state = NGH_STATE_SIZE_TAG;
			}
			break;
	}
}
