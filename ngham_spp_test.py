import struct, serial, sys, time, crcmod

# States
STATE_START = 0
STATE_HEADER = 1
STATE_PAYLOAD = 2

NGHAM_TYPE_RX = 0
NGHAM_TYPE_TX = 1
NGHAM_TYPE_LOCAL = 2
NGHAM_TYPE_COMMAND = 3

# Types
TYPES = ["rx-data", "tx-data", "local-data", "command"]

# Constants
NGHAM_START = '\x24'

class ngham_spp:
    def __init__(self):
        pass
        
    def __del__(self):
        self.close()

    def open(self, port, baud):
        self.state = STATE_START
        self.pl = ""
        self.ser = serial.Serial(port,baud,timeout=0)
        self.crc = 0
        self.type = 0
        self.len = 0
        self.crc_func = crcmod.mkCrcFun(0x11021, rev=True, initCrc=0xffff, xorOut=0x0000)
        self.crc_calc = 0

    def close(self):
        if self.ser:
            self.ser.close()
            self.ser = False
        
    def cmd(self, pl):
        crc = self.crc_func(struct.pack('<BB', 0x02, len(pl)) + pl)
        crc ^= 0xffff
        pkt = struct.pack('<BHBB', ord(NGHAM_START), crc, 0x02, len(pl)) + pl
        self.ser.write(pkt)

    def send(self, pl, flags=0):
        crc = self.crc_func(struct.pack('<BB', 0x00, len(pl)+1) + chr(flags) + pl)
        crc ^= 0xffff
        pkt = struct.pack('<BHBB', ord(NGHAM_START), crc, 0x00, len(pl)+1) + chr(flags) + pl
        self.ser.write(pkt)

    def tick(self):
        while self.ser.inWaiting():
            c = self.ser.read(1)
            if (self.state == STATE_START):
                if (c == NGHAM_START):
                    # Start character detected
                    self.state = STATE_HEADER
                    self.pl = ""
                                
            elif (self.state == STATE_HEADER):
                self.pl += c
                if (len(self.pl) == 4):
                    # End of header reached
                    self.state = STATE_PAYLOAD
                    [self.crc, self.type, self.len] = struct.unpack("<HBB", self.pl)
                    self.crc_calc = self.crc_func(self.pl[2:])
                    self.pl = ""

            elif (self.state == STATE_PAYLOAD):
                self.pl += c
                if (len(self.pl) == self.len):
                    # Target payload length reached
                    self.state = STATE_START
                    # Check CRC
                    self.crc_calc = self.crc_func(self.pl, self.crc_calc)
                    self.crc_calc ^= 0xffff
                    if (self.crc_calc == self.crc):
                        sys.stdout.write("[%s, len=%d] " % (TYPES[self.type], self.len))
                        if (self.type == NGHAM_TYPE_RX):
                            [time_us, noise, rssi, errors, flags] = struct.unpack("<LBBBB", self.pl[:8])
                            sys.stdout.write("TOH=%d, noise=%d, rssi=%d, errors=%d, flags=0x%.02x" % (time_us, noise-200, rssi-200, errors, flags))
                        else:
                            print self.pl
			sys.stdout.write("\n")
                    else:
                        print "CRC fail!"

n = ngham_spp()
n.open("/dev/ttyUSB2", 38400)
#n.ser.write("\r\npac0 ngham_spp\r\nportset\r\n")
#n.cmd("access csma-rssi")
#n.cmd("pac0")
#n.cmd("HELP")
#n.cmd("freq 155575000")
##n.cmd("stat-int 5")
##n.cmd("tdma-tslots 2")
#n.cmd("tdma-tper 1000")
#n.cmd("help mode")
#n.cmd("mode ax25-1k2")
#n.cmd("freq 144850000")
#n.send("INB1.2V")
##n.cmd("MODE NGHAM")
time.sleep(1)
#n.send("N")
n.send("1234567891234567891234567891234567891234567891234567891234567893269841769879698698432176398746982163498716239874612983764198726349871624398716239874")

while(1):
    n.tick()
    time.sleep(0.01)
##            
##except Exception as e:
##    n.close()
##    print e, "exited"
