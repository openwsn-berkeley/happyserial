import sys
import os
sys.path.append(os.path.join('..','src'))

import time
from happyserial import HappySerial

def _happyserial_rx_cb(buf):
    print('rx: {}'.format(buf))
    
happy = HappySerial.HappySerial(
    serialport = 'COM41', # first port of the two that appear when plugging in the nRF52840-DK
    rx_cb      = _happyserial_rx_cb,
)

while True:
    happy.tx([0x01,0x02,0x03])
    time.sleep(1.000)
