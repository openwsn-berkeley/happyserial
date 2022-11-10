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
    buf = [0x04,0x04,0x04]
    happy.tx(buf)
    print('tx: {}'.format(buf))
    time.sleep(0.500)
