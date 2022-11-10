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

```
import happyserial

def rx_cb(self,rxframe):
    print(rxframe) # called each time Python receives a frame

happyser = happyserial.Happyserial(rx_cb)

...

happyser.send(txframe=[0x00,0x01,0x02,0x03])
```

On the C side:

```
...
#include "happyserial.h"

int main(void) {
    
    happyserial_init(_happyserial_rx_cb);

    ...
    
    happyserial_tx(buf,bufLen);
}

//=========================== interrupt handlers ==============================

void _happyserial_rx_cb(uint8_t* buf, uint8_t bufLen) {
    ...
}
```
