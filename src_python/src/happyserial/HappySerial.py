from happyserial import Hdlc

class HappySerial(object):
    
    def __init__(self,serialport,rx_cb):
        
        # store params
        self.serialport = serialport
        self.rx_cb      = rx_cb
        
        # local variables
        self.hdlc = Hdlc.Hdlc(
            serialport  = self.serialport,
            rx_cb       = self._hdlc_rx_cb,
        )
    
    #======================== public ==========================================
    
    def tx(self,buf):
        self.hdlc.tx(buf)
    
    #======================== private =========================================
    
    def _hdlc_rx_cb(self,buf):
        self.rx_cb(buf)
