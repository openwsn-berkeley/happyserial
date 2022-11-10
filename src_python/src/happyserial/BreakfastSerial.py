# built-in
import threading
# third-party
import serial
# local

class BreakfastSerial(threading.Thread):
    
    def __init__(self,serialport,rx_cb):
        
        # store params
        self.serialport = serialport
        self.rx_cb      = rx_cb
        
        # local variables
        self.serial     = serial.Serial(self.serialport, 19200)
        
        # start thread
        super().__init__()
        self.name       = 'BreakfastSerial'
        self.daemon     = True
        self.start()
    
    def run(self):
        while True:
            b = self.serial.read()
            self.rx_cb(b)
    
    #======================== public ==========================================

    def tx(self,b):
        self.serial.write(b)

    #======================== private =========================================
