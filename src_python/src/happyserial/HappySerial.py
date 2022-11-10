import Hdlc

class HappySerial(object):
    
    def __init__(self,serialport,happyserial_rx_cb)
        
        # store params
        self.happyserial_rx_cb = happyserial_rx_cb
        
        # local variables
        self.hdlc = Hdlc(self._hdlc_rx_cb)
    
    #======================== public ==========================================
    
    def tx(buf):
        self.hdlc.tx(buf)
    
    #======================== private =========================================
    
    def _hdlc_rx_cb(buf):
        self.happyserial_rx_cb(buf)
