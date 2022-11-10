from happyserial import BreakfastSerial, \
                        Crc

class HdlcException(Exception):
    pass

class Hdlc(object):
    
    HDLC_FLAG           = 0x7e
    HDLC_ESCAPE         = 0x7d
    HDLC_ESCAPE_MASK    = 0x20
    
    def __init__(self,serialport,rx_cb):
        
        # store params
        self.serialport = serialport
        self.rx_cb      = rx_cb
        
        # local variables
        self.crc             = Crc.Crc()
        self.serial          = BreakfastSerial.BreakfastSerial(
            self.serialport,
            self._serial_rx_cb,
        )
        self.rxBusyReceiving = False
        self.rxEscaping      = False
        self.rxLastByte      = None
    
    #============================ public ======================================
    
    def tx(self,buf):
    
        # HDLC'ify
        self._hdlc_tx_open()
        for b in buf:
            self._hdlc_tx_write(b)
        self._hdlc_tx_close()
        
        # send
        for b in self.txBuf:
            self.serial.tx(b)
    
    #============================ private =====================================
    
    #=== tx
    
    def _hdlc_tx_open(self):
        self.txCrc      = self.crc.CRCINIT
        self.txBuf      = []
        self.txBuf     += [self.HDLC_FLAG]

    def _hdlc_tx_write(self,b):
        self.txCrc      = self.crc.crc_iterate(self.txCrc,b)
        if b == self.HDLC_FLAG or b == self.HDLC_ESCAPE:
            self.txBuf += [self.HDLC_ESCAPE]
            b           = b ^ self.HDLC_ESCAPE_MASK;
        self.txBuf     += [b]

    def _hdlc_tx_close(self):
        
        finalCrc = 0xffff-self.txCrc;
        
        self._hdlc_tx_write((finalCrc >> 0) & 0xff); # CRC is escaped
        self._hdlc_tx_write((finalCrc >> 8) & 0xff); # CRC is escaped
        
        self.txBuf     += [self.HDLC_FLAG]

    def _hdlc_rx_open(self):
    
        self.rxBuf      = []
        self.rxCrc      = self.crc.CRCINIT;

    def _hdlc_rx_write(self,b):
        
        if b == self.HDLC_ESCAPE:
            self.rxEscaping = True
        else:
            if self.rxEscaping == True:
                b = b ^ self.HDLC_ESCAPE_MASK
                self.rxEscaping = False

            # add byte to input buffer
            self.rxBuf += [b]

            # iterate through CRC calculator
            self.rxCrc = self.crc.crc_iterate(self.rxCrc, b)

    def _hdlc_rx_close(self):
        
        # verify the validity of the frame
        if self.rxCrc == self.crc.CRCGOOD:
            # the CRC is correct

            # remove the CRC from the input buffer
            self.rxBuf = self.rxBuf[:-2]

            crcValid = True
        else:
            # the CRC is incorrect

            crcValid = False

        return crcValid
    
    #=== serial
    
    def _serial_rx_cb(self,rxByte):
        
        if (
            self.rxBusyReceiving == False and
            self.rxLastByte == self.HDLC_FLAG and
            rxByte != self.HDLC_FLAG
        ):
            # start of frame

            # I'm now receiving
            self.rxBusyReceiving = True

            # create the HDLC frame
            self._hdlc_rx_open();

            # add the byte just received
            self._hdlc_rx_write(rxByte);
        elif (
            self.rxBusyReceiving == True and
            rxByte != self.HDLC_FLAG
        ):
            # middle of frame

            # add the byte just received
            self._hdlc_rx_write(rxByte);

        elif (
            self.rxBusyReceiving == True and
            rxByte == self.HDLC_FLAG
        ):
            # end of frame

            # finalize the HDLC frame
            self.rxBusyReceiving = False
            crcValid = self._hdlc_rx_close()

            if crcValid:
                
                # call the callback
                self.rx_cb(self.rxBuf)
            
            self.rxBuf       = []

        self.rxLastByte = rxByte
