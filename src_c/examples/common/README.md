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
- uses the DMA on the nRF so your firmware routines aren't interrupted by every byte received

# wishlist

- retransmission when a frame is corrupted

# using `happyserial`

On the python side:

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
#include "happyserial.h"

void rx_cb(uint8_t* buf,uint8_t* len) {
    // called each time your device receives a frame
}

happyserial_init(rx_cb)

...

const uint8_t txframe[] = {0x00,0x01,0x02,0x03};

happyser_send(txframe,sizeof(txframe));
```
