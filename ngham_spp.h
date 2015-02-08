//**************************************************************//
// NGHam protocol - Jon Petter Skagmo, LA3JPA, 2014.            //
// Licensed under LGPL.                                         //
//**************************************************************//

#include <stdint.h>

#include "ngham_paths.h"
#include PATH_NGHAM_PLATFORM_SPP

void ngham_pack_byte(port_ctx_t* ctx, uint8_t c);
void ngham_unpack(rx_pkt_t* p);