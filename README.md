NGHam protocol
==============

NGHam, short for Next Generation Ham Radio, is a set of protocols for packet radio communication. Its usage is similar to the existing AX.25 protocol. The standard covers three sub-protocols:
- NGHam radio protocol
- NGHam serial port protocol (comparable with KISS for AX.25)
- NGHam extensions - a modular extension using the payload for addressing, position, voice, statistics etc.

Main features:

- **Very robust:** Allows decoding with lower SNR compared to 1200 baud AFSK AX.25 due to the use of a sync word with good autocorrelation properties and Reed Solomon FEC over the whole remaining packet.
- **High throughput:** As the sync word eliminates the need for a long preamble and FEC increases chance of successful packet reception, the throughput will be higher than for 9600 baud AX.25.
- **Better spectral efficiency:** Significantly better spectral efficiency than 1200 baud AFSK, and somewhat better than 9600 baud G3RUH modulation due to the reduced deviation.
- **More efficient channel access:** CSMA is the default access method, but TDMA with automatic time-synchronization is being developed and is tested.
- **Clear distinction between communication layers:** The NGHam radio protocol has no addressing or other higher level information, and can be used without NGHam extensions.

[Youtube video: NGHam demonstration with Owl VHF and TDMA](http://youtu.be/_96td-Y-LLA)


![NGHam radio protocol block schematic](https://github.com/skagmo/ngham/blob/master/other/illustrations/ngham_block_v3.png?raw=true)

Content:

| File/folder       | Description |
|:-----------       |:----------- |
| fec-3.0.1/        | FEC library by Phil Karn for Reed Solomon. (http://www.ka9q.net/code/fec/) |
| other/            | More documentation. |
| ngham.h           | The main NGHam encoder/decoder functions. |
| ngham_packets.h   | Declarations of the TX/RX packet structs etc. |
| ngham_platform.h  | User provided code - MUST BE CUSTOMIZED. |
| crc_ccitt.h       | CRC-code used in NGHam. |
| ccsds_scrambler.h | Pre-generated array from scrambling polynomial. |

Not finished:

| File/folder       | Description |
|:-----------       |:----------- |
| ngham_extensions.h        | An extension of the payload field in the NGHam radio protocol (enabled by a flag in the NGHam header).  |
| ngham_spp.h            | Serial port protocol for communication with the host.  |

Usage:

1. Provide your own sync word correlator and GMSK (de)modulator. 
2. The demodulator should start outputting data only after sync word is detected.
3. ngham_platform.h/c must be customized for the platform you are using.

```
// Initialize Reed Solomon arrays - only do this once!
ngham_init_arrays();

// Initialize state machine
ngham_init();

// Send a packet containing the payload "TEST"
tx_pkt_t p;
p.pl[0] = 'T';
p.pl[1] = 'E';
p.pl[2] = 'S';
p.pl[3] = 'T';
p.pl_len = 4;
p.ngham_flags = 0;
ngham_encode(&p);

// Receive packet. If a complete packet is received, function ngham_action_handle_packet will be called.
while (demodulator_data_available()) ngham_decode(get_byte_from_demodulator());
```

--
Jon Petter Skagmo, LA3JPA, 2014
