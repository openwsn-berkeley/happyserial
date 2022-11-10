import pytest
import threading
import random

class RxClass(object):
    def __init__(self,rxSem):
        self.rxSem       = rxSem
        self.msg         = None
    def rx(self,msg):
        self.msg         = msg
        self.rxSem.release()

MSG_MAXLEN  = 124

FIXTURE_MSG  = []
# worst case
FIXTURE_MSG += [[0x7e]*MSG_MAXLEN]
FIXTURE_MSG += [[0x7d]*MSG_MAXLEN]
# random messages
for m in range(1000):
    msg = []
    for b in range(random.randint(1,MSG_MAXLEN)):
        msg += [random.randint(0x00,0xff)]
    FIXTURE_MSG += [msg]

rxSem   = threading.Lock()
rxSem.acquire()
rxClass = RxClass(rxSem)

@pytest.fixture(scope="module")
def connectedHappySerial():
    import happyserial
    return happyserial.HappySerial.HappySerial(
        serialport = 'COM41', # first port of the two that appear when plugging in the nRF52840-DK
        rx_cb      = rxClass.rx,
    )

@pytest.mark.parametrize("msg", FIXTURE_MSG)
def test_echo(connectedHappySerial,msg):
    
    connectedHappySerial.tx(msg)
    rxSem.acquire(timeout=5)
    assert rxClass.msg == msg
