# HappySerial

Implementing a serial protocol is hard.
`happyserial` deals with all the hard parts so you don't need to.
Drop it in your project, both on the C and Python side.

# Scope

`happyserial` is useful when you have a Python program running on a computer that needs to send/receive data reliably over a serial port to an embedded device.
`happyserial` is a set of two libraries, one for the Python side, one for the C side.
Your code ends up sending and receiving serial frames through those library, never having to worry about bytes, retries, framing, etc.

```
+----------+           +-----------+
|  Python  |   serial  |    C      |
| software |===========| firmware  |
|          |           |           |
|(computer)|           | (devices) |
+----------+           +-----------+
       
       scope of happyserial
       <------------------>
```

# Features

- framing of the serial stream
- detect and drop corrupted frames

# wishlist

- retransmission when a frame is corrupted
- uses the DMA on the nRF so your firmware routines aren't interrupted by every byte received

# using `happyserial`

On the Python side:

```python
from happyserial import HappySerial

def _happyserial_rx_cb(buf):
    print('rx: {}'.format(buf))
    
happy = HappySerial.HappySerial(
    serialport = 'COM41',
    rx_cb      = _happyserial_rx_cb,
)

happy.tx([0x01,0x02,0x03])
```

On the C side:

```c
...
#include "happyserial.h"

int main(void) {
    uint8_t buf = {0x00,0x01,0x03};
    
    happyserial_init(_happyserial_rx_cb);
    ...
    happyserial_tx(buf,sizeof(buf));
}

void _happyserial_rx_cb(uint8_t* buf, uint8_t bufLen) {
    ...
}
```
