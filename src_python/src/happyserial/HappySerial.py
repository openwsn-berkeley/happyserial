import threading
from happyserial import Hdlc

class HappySerial(object):
    
    RESPONSE_TIMEOUT              = 1 # in seconds
    FLAG_RESPOND                  = (1<<0)
    MSG_MAXLEN                    = 124 # due to limited buffer size on embedded side
    
    def __init__(self,serialport,rx_cb):
        
        # store params
        self.serialport           = serialport
        self.rx_cb                = rx_cb
        
        # local variables
        self.hdlc = Hdlc.Hdlc(
            serialport            = self.serialport,
            rx_cb                 = self._hdlc_rx_cb,
        )
        self.dataLock             = threading.Lock()
        self.busywaiting          = False
        self.response             = []
        self.waitsem              = threading.Lock()
        self.waitsem.acquire()
    
    #======================== public ==========================================
    
    def tx(self,buf,waitforresponse=False):
        flags = 0
        if waitforresponse:
            flags |= self.FLAG_RESPOND
            with self.dataLock:
                self.busywaiting  = True
        self.hdlc.tx(buf) # FIXME: add flags
        if waitforresponse:
            if self.waitsem.acquire(timeout=self.RESPONSE_TIMEOUT):
                # was released
                return self.response
            else:
                # timedout
                raise TimeoutError()
    
    #======================== private =========================================
    
    def _hdlc_rx_cb(self,buf):
        with self.dataLock:
            busywaiting = self.busywaiting
        if busywaiting:
            self.response = buf
            self.waitsem.release()
        else:
            self.rx_cb(buf)
