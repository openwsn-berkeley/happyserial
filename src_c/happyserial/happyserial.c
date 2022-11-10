#include <stdbool.h>
#include <string.h>
#include "happyserial.h"
#include "hdlc.h"
#include "crc.h"
#include "uart.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
    happyserial_rx_cbt  happyserial_rx_cb;
} happyserial_vars_t;

happyserial_vars_t happyserial_vars;

typedef struct {
    uint32_t            dummy;
} happyserial_dbg_t;

happyserial_dbg_t happyserial_dbg;

//=========================== prototypes ======================================

void _hdlc_rx_cb(uint8_t* buf, uint8_t bufLen);

//=========================== public ==========================================

void happyserial_init(happyserial_rx_cbt happyserial_rx_cb) {
    
    // reset variable
    memset(&happyserial_vars, 0x00, sizeof(happyserial_vars_t));
    memset(&happyserial_dbg,  0x00, sizeof(happyserial_dbg_t));

    // store params
    happyserial_vars.happyserial_rx_cb = happyserial_rx_cb;

    // initialize HDLC
    hdlc_init(_hdlc_rx_cb);
}

void happyserial_tx(uint8_t* buf, uint8_t bufLen) {
    hdlc_tx(buf, bufLen);
}

//=========================== private =========================================

void _hdlc_rx_cb(uint8_t* buf, uint8_t bufLen) {
    happyserial_vars.happyserial_rx_cb(buf,bufLen);
}
