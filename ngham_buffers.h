//**************************************************************//
// NGHam protocol - Jon Petter Skagmo, LA3JPA, 2014.            //
// Licensed under LGPL.                                         //
//**************************************************************//

#pragma once

#include <stdint.h>
#include "ngham_packets.h" // tx_pkt_t

// Platform specific functions
#include "platform_ngham.h"

void ngham_rx_buffer_init(void);
void ngham_tx_buffer_init(void);

// Push packet to buffers for later processing
void ngham_tx_push(tx_pkt_t* p);
void ngham_rx_push(rx_pkt_t* p);

// Check if TX/RX packets are waiting in ring buffers
// Will process a maximum of one packet per tick
void ngham_process_tick(void);
