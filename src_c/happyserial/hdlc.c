#include <stdbool.h>
#include <string.h>
#include "hdlc.h"
#include "crc.h"
#include "uart.h"

//=========================== defines =========================================

#define HDLC_FLAG            0x7e
#define HDLC_ESCAPE          0x7d
#define HDLC_ESCAPE_MASK     0x20

#define HDLC_TXBUF_LEN       256 // leave at 256 to allow for wrap-around
#define HDLC_RXBUF_LEN       256

// worst case length: every byte is escaped
// assuming txBuf is empty and can only hold HDLC_TXBUF_LEN bytes:
// 2 bytes for the flags
// 2 bytes for the CRC
// if HDLC_TXBUF_LEN==256, HDLC_MSG_MAXLEN==124
#define HDLC_MSG_MAXLEN      ((HDLC_TXBUF_LEN/2)-2-2)

//=========================== variables =======================================

typedef struct {
    // callbacks
    hdlc_rx_cbt    hdlc_rx_cb;
    // tx
    uint8_t        txBuf[HDLC_TXBUF_LEN];
    uint8_t        txBufIdxW;
    uint8_t        txBufIdxR;
    uint16_t       txCrc;
    // rx
    uint8_t        rxBuf[HDLC_TXBUF_LEN];
    uint16_t       rxBufFill; //  must be uint16_t so can detect if >256
    uint16_t       rxCrc;
    bool           rxEscaping;
    bool           rxBusyReceiving;
    uint8_t        rxLastByte;
} hdlc_vars_t;

hdlc_vars_t hdlc_vars;

typedef struct {
    uint32_t       numOverflow;
    uint32_t       numRxCrcValid;
    uint32_t       numRxCrcInvalid;
} hdlc_dbg_t;

hdlc_dbg_t hdlc_dbg;

//=========================== prototypes ======================================

// tx
void _hdlc_tx_open(void);
void _hdlc_tx_write(uint8_t b);
void _hdlc_tx_close(void);

// rx
void _hdlc_rx_open(void);
void _hdlc_rx_write(uint8_t b);
bool _hdlc_rx_close(void);

// uart
void _uart_txByteDone(void);
void _uart_rxByte(uint8_t b);

//=========================== public ==========================================

void hdlc_init(hdlc_rx_cbt hdlc_rx_cb) {
    
    // reset variable
    memset(&hdlc_vars, 0x00, sizeof(hdlc_vars_t));
    memset(&hdlc_dbg,  0x00, sizeof(hdlc_dbg_t));

    // store params
    hdlc_vars.hdlc_rx_cb = hdlc_rx_cb;

    // initialize UART
    uart_init(_uart_txByteDone,_uart_rxByte);
}

void hdlc_tx(uint8_t* buf, uint8_t bufLen) {
    uint8_t i;

    _hdlc_tx_open();
    for (i = 0; i < bufLen; i++) {
        _hdlc_tx_write(buf[i]);
    }
    _hdlc_tx_close();

    // start TX'ing
    uart_txByte(hdlc_vars.txBuf[hdlc_vars.txBufIdxR++]);
}

//=========================== private =========================================

//=== tx

void _hdlc_tx_open(void) {
    hdlc_vars.txCrc                         = CRCINIT;
    hdlc_vars.txBuf[hdlc_vars.txBufIdxW++]  = HDLC_FLAG;
}

void _hdlc_tx_write(uint8_t b) {
    hdlc_vars.txCrc = crc_iterate(hdlc_vars.txCrc, b);
    if (b == HDLC_FLAG || b == HDLC_ESCAPE) {
        hdlc_vars.txBuf[hdlc_vars.txBufIdxW++]   = HDLC_ESCAPE;
        b = b ^ HDLC_ESCAPE_MASK;
    }
    hdlc_vars.txBuf[hdlc_vars.txBufIdxW++]  = b;
}

void _hdlc_tx_close(void) {
    uint16_t finalCrc;
    
    finalCrc = ~hdlc_vars.txCrc;
    _hdlc_tx_write((finalCrc >> 0) & 0xff); // CRC is escaped
    _hdlc_tx_write((finalCrc >> 8) & 0xff); // CRC is escaped
    hdlc_vars.txBuf[hdlc_vars.txBufIdxW++]  = HDLC_FLAG;
}

//=== rx

void _hdlc_rx_open(void) {
    
    hdlc_vars.rxBufFill = 0;
    hdlc_vars.rxCrc     = CRCINIT;
}

void _hdlc_rx_write(uint8_t b) {
    
    if (b == HDLC_ESCAPE) {
        hdlc_vars.rxEscaping = true;
    } else {
        if (hdlc_vars.rxEscaping == true) {
            b = b ^ HDLC_ESCAPE_MASK;
            hdlc_vars.rxEscaping = false;
        }

        // add byte to input buffer
        hdlc_vars.rxBuf[hdlc_vars.rxBufFill++] = b;

        // iterate through CRC calculator
        hdlc_vars.rxCrc = crc_iterate(hdlc_vars.rxCrc, b);
    }
}

bool _hdlc_rx_close(void) {
    bool crcValid;

    // verify the validity of the frame
    if (hdlc_vars.rxCrc == CRCGOOD) {
        // the CRC is correct

        // remove the CRC from the input buffer
        hdlc_vars.rxBufFill    -= 2;

        crcValid = true;
    } else {
        // the CRC is incorrect

        // drop the incoming frame
        crcValid = false;
    }

    return crcValid;
}

//=== uart

void _uart_txByteDone(void) {
    
    if (hdlc_vars.txBufIdxW != hdlc_vars.txBufIdxR) {
        // I have some bytes to transmit

        uart_txByte(hdlc_vars.txBuf[hdlc_vars.txBufIdxR++]);
    }
}

void _uart_rxByte(uint8_t rxByte) {
    bool crcValid;
    
    if (
        hdlc_vars.rxBusyReceiving == false &&
        hdlc_vars.rxLastByte == HDLC_FLAG &&
        rxByte != HDLC_FLAG
    ) {
        // start of frame

        // I'm now receiving
        hdlc_vars.rxBusyReceiving = true;

        // create the HDLC frame
        _hdlc_rx_open();

        // add the byte just received
        _hdlc_rx_write(rxByte);
    } else if (
        hdlc_vars.rxBusyReceiving == true &&
        rxByte != HDLC_FLAG
    ) {
        // middle of frame

        // add the byte just received
        _hdlc_rx_write(rxByte);
        if (hdlc_vars.rxBufFill + 1 > HDLC_RXBUF_LEN) {
            // overflow

            // debug
            hdlc_dbg.numOverflow++;

            // abort
            hdlc_vars.rxBufFill       = 0;
            hdlc_vars.rxBusyReceiving = false;
        }
    } else if (
        hdlc_vars.rxBusyReceiving == true &&
        rxByte == HDLC_FLAG
    ) {
        // end of frame

        // finalize the HDLC frame
        hdlc_vars.rxBusyReceiving = false;
        crcValid = _hdlc_rx_close();

        if (crcValid==true) {
            
            // debug
            hdlc_dbg.numRxCrcValid++;

            // call the callback
            hdlc_vars.hdlc_rx_cb(
                hdlc_vars.rxBuf,
                hdlc_vars.rxBufFill
            );
        } else {
            
            // debug
            hdlc_dbg.numRxCrcInvalid++;
        }

        hdlc_vars.rxBufFill       = 0;
        
    }

    hdlc_vars.rxLastByte = rxByte;
}
