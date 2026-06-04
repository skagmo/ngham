//**************************************************************//
// NGHam protocol - Jon Petter Skagmo, LA3JPA, 2014.            //
// Licensed under LGPL.                                         //
//**************************************************************//

#include "ngham_buffers.h"

#include <string.h>					// For memcpy
#include "ngham.h"

// RX / TX packet buffers for load distribution
#define RX_PKTS_SIZE 8
rx_pkt_t rx_pkts[RX_PKTS_SIZE];
volatile uint8_t rx_pkts_ip, rx_pkts_op;

#define TX_PKTS_SIZE 8
tx_pkt_t tx_pkts[TX_PKTS_SIZE];
volatile uint8_t tx_pkts_ip, tx_pkts_op;

void ngham_rx_buffer_init(void){
	rx_pkts_ip = rx_pkts_op = 0;
}

void ngham_tx_buffer_init(void){
	tx_pkts_ip = tx_pkts_op = 0;
}

void ngham_tx_push(tx_pkt_t* p){
	if (p->pl_len > 220){
		port_verbose(VERBOSE_LEVEL_ERROR, "Oversize NGHam RF packet");
		return;
	}
    uint8_t next = (tx_pkts_ip+1) & (TX_PKTS_SIZE-1);
	if (next==tx_pkts_op) port_verbose(VERBOSE_LEVEL_ERROR, "TX packet overflow");
    else{
		memcpy((uint8_t*)&tx_pkts[tx_pkts_ip], (uint8_t*)p, p->pl_len+TX_PKT_OVERHEAD);
		tx_pkts_ip = next;
	}
}

void ngham_rx_push(rx_pkt_t* p){
    uint8_t next = (rx_pkts_ip+1) & (RX_PKTS_SIZE-1);
    if (next == rx_pkts_op) port_verbose(VERBOSE_LEVEL_ERROR, "RX packet overflow");
    else{
		memcpy(&rx_pkts[rx_pkts_ip], p, NGH_PL_PAR_SIZE[p->rs_size_nr]+RX_PKT_OVERHEAD);
		rx_pkts_ip = next;
	}
}

void ngham_process_tick(void){
    if (rx_pkts_ip != rx_pkts_op){
        ngham_decode(&rx_pkts[rx_pkts_op]);
        rx_pkts_op = (rx_pkts_op+1) & (RX_PKTS_SIZE-1);
    }
    else if (tx_pkts_ip != tx_pkts_op){
        ngham_encode(&tx_pkts[tx_pkts_op]);
        tx_pkts_op = (tx_pkts_op+1) & (TX_PKTS_SIZE-1);
    }
}
