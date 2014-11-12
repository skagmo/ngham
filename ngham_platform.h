//**************************************************************//
// NGHam protocol - Jon Petter Skagmo, LA3JPA, 2014.            //
// Licensed under LGPL.                                         //
//**************************************************************//

#include "ngham_packets.h"	// Packet structs, NA-values etc.

// Temporary buffer for the decoder, rx packet variable and state variable.
// Define in ngham_platform.c. The only reason these are here and
// not in ngham.c is because they might be defined elsewhere in your application
// and you want to reuse them.
extern uint8_t rx_buf[];	// Should hold the largest packet - ie. 255 B
extern uint16_t rx_buf_len;
extern rx_pkt_t rx_pkt;
extern uint8_t decoder_state;

// Data to be transmitted (to modulator). Priority can be ignored if NGHam is not used for timing purposes.
void ngham_action_send_data(uint8_t *d, uint16_t d_len, uint8_t priority);

// Set packet size demodulator, if applicable, to make the demodulator stop outputting data when the packet is finished.
void ngham_action_set_packet_size(uint8_t size);

// Should return RSSI in dBm + 200
uint8_t ngham_action_get_rssi(void);

// Should return noise floor in dBm + 200
uint8_t ngham_action_get_noise_floor(void);

// Will always be called after packet reception is finished - whether it was successful or not.
// This function should also handle reinitialization of your decoder/sync word detector.
void ngham_action_handle_packet(uint8_t condition, rx_pkt_t* p);

// Not required: Code to be executed when reception has just started.
// This could be handling the time stamp of the sync word or starting RSSI readout.
// Tthe time stamp itself should be recorded in an interrupt routine.
void ngham_action_reception_started(void);

// Not required: If there is more to do a little after reception start, do it here.
// NGHAM_BYTES_TILL_ACTION_HALFWAY is number of payload bytes until ngham_action_reception_halfway is executed.
// Don't care about this if you don't use the mentioned function.
#define NGHAM_BYTES_TILL_ACTION_HALFWAY 10
void ngham_action_reception_halfway(void);
