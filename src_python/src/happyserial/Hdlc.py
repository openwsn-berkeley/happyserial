from happyserial import BreakfastSerial, \
                        Crc

class HdlcException(Exception):
    pass

class Hdlc(object):
    
    HDLC_FLAG           = 0x7e
    HDLC_FLAG_ESCAPED   = 0x5e
    HDLC_ESCAPE         = 0x7d
    HDLC_ESCAPE_ESCAPED = 0x5d
    
    def __init__(self,serialport,rx_cb):
        
        # store params
        self.serialport = serialport
        self.rx_cb      = rx_cb
        
        # local variables
        self.crc        = Crc.Crc()
        self.serial     = BreakfastSerial.BreakfastSerial(self.serialport,self._serial_rx_cb)
    
    #============================ public ======================================
    
    def tx(self,buf):
        hdlcbuf = self._hdlcify(buf)
        for b in hdlcbuf:
            self.serial.tx(b)
    
    #============================ private =====================================
    
    def _serial_rx_cb(self,b):
        raise NotImplementedError()
    
    def _hdlcify(self,inBuf):
        
        # make copy of input
        outBuf     = inBuf[:]
        
        # calculate CRC
        crc        = self.crc.CRCINIT
        for b in outBuf:
            crc    = self.crc.crc_iteration(crc,b)
        crc        = 0xffff-crc
        
        # append CRC
        outBuf    += [ crc & 0xff]
        outBuf    += [(crc & 0xff00) >> 8]
        
        # espace bytes
        # FIXME
        #outBuf     = outBuf.replace(self.HDLC_ESCAPE, self.HDLC_ESCAPE+self.HDLC_ESCAPE_ESCAPED)
        #outBuf     = outBuf.replace(self.HDLC_FLAG,   self.HDLC_ESCAPE+self.HDLC_FLAG_ESCAPED)
        
        # add flags
        outBuf     = [self.HDLC_FLAG] + outBuf + [self.HDLC_FLAG]
        
        return outBuf

    def _dehdlcify(self,inBuf):
        assert inBuf[ 0]==self.HDLC_FLAG
        assert inBuf[-1]==self.HDLC_FLAG
        
        # make copy of input
        outBuf     = inBuf[:]
        
        # remove flags
        outBuf     = outBuf[1:-1]
        
        # unstuff
        outBuf     = outBuf.replace(self.HDLC_ESCAPE+self.HDLC_FLAG_ESCAPED,   self.HDLC_FLAG)
        outBuf     = outBuf.replace(self.HDLC_ESCAPE+self.HDLC_ESCAPE_ESCAPED, self.HDLC_ESCAPE)
        
        if len(outBuf)<2:
            raise HdlcException('packet too short')
        
        # check CRC
        crc        = self.HDLC_CRCINIT
        for b in outBuf:
            crc    = self.crc.crc_iteration(crc,b)
        if crc!=self.crc.CRCGOOD:
           raise HdlcException('wrong CRC')
        
        # remove CRC
        outBuf     = outBuf[:-2] # remove CRC
        
        return outBuf
