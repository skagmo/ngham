//**************************************************************//
// NGHam protocol - Jon Petter Skagmo, LA3JPA, 2014.            //
// Licensed under LGPL.                                         //
//**************************************************************//

#include "spp_host_qt.h"
#include <string.h> // For memcpy
#include <stdio.h>

extern "C" {
#include "crc_ccitt.h"
}

SppHost::SppHost(void){
	d_len = 0;
	state = START;
}

SppHost::~SppHost(void){
}

void SppHost::parse(uint8_t c){
	switch(this->state){
		case START:
			if (c == SPP_START){
                state = HEADER;	// Start found; go to next state
                d_len = 1; // Starts at next first as SPP_START is already received
			}
			break;
			
		case HEADER:
			// Fill ctx->d with header - no check for size, as buffer is much larger than header (5B)
            d[d_len++] = c;
			
			if (d_len >= sizeof(ngh_spphdr_t)){
				d_target = sizeof(ngh_spphdr_t) + ((ngh_spphdr_t*)d)->pl_len;
                state = PAYLOAD;
			}
			break;
			
		case PAYLOAD:
			// Fill ctx->d with payload, limit
			if (d_len < SPP_D_SIZE) d[d_len++] = c;
            else state = START;
            
			// If received length has met target length (set in STATE_HEADER)
			if (d_len == d_target){
				ngh_spphdr_t* hdr = (ngh_spphdr_t*)d;
                state = START;

                uint16_t crc_calc = crc_ccitt(d+3, d_len-3);

                if (hdr->crc == crc_calc){
                    emit newPacket(hdr->pl_type, d+sizeof(ngh_spphdr_t), d_len-sizeof(ngh_spphdr_t), m_source);
                }
			}
			break;
	}
}

QByteArray SppHost::packCommand(QString cmd)
{
	QByteArray data;
	
    data.resize(sizeof(ngh_spphdr_t));
	data.append(cmd.toLocal8Bit());

    ngh_spphdr_t* hdr = (ngh_spphdr_t*)(data.data());
	hdr->start = SPP_START;
	hdr->pl_type = NGHAM_SPP_TYPE_CMD;
	hdr->pl_len = cmd.length();

	// CRC can be calculated now
    hdr->crc = crc_ccitt((uint8_t*)(data.data()+3), data.size()-3);
	
	return data;
}

QByteArray SppHost::packTransmit(QByteArray data, uint8_t flags){
    return packTransmit((uint8_t*)(data.data()), data.size(), flags);
}

QByteArray SppHost::packTransmit(const uint8_t* data, unsigned int data_size, uint8_t flags)
{
	// Preliminary check of buffer size
	if (data_size > 220) return 0;

	QByteArray output;
	
    output.resize(sizeof(ngh_spphdr_t) + 1);
    output.append((char*)data, data_size);

    ngh_spphdr_t* hdr = (ngh_spphdr_t*)(output.data());
	hdr->start = SPP_START;
	hdr->pl_type = NGHAM_SPP_TYPE_TX;
	hdr->pl_len = 1+data_size;
	
	// Flags, first byte after SPP-header
    output.data()[sizeof(ngh_spphdr_t)] = flags;

	// CRC can be calculated now
    uint16_t j, crc = 0xffff;
    for (j=3; j<output.length(); j++) crc = crc_ccitt_byte(output[j], crc);
    crc ^= 0xffff;

    hdr->crc = crc;

	return output;
}
