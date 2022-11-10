#include "nrf52840.h"
#include "happyserial.h"

//=========================== defines =========================================

//=========================== prototypes ======================================

void _happyserial_rx_cb(uint8_t* buf, uint8_t bufLen);

//=========================== variables =======================================

typedef struct {
    uint32_t       dummy;
} app_vars_t;

app_vars_t app_vars;

typedef struct {
    uint32_t       dummy;
} app_dbg_t;

app_dbg_t app_dbg;

//=========================== main ============================================

int main(void) {
    
    happyserial_init(_happyserial_rx_cb);

    // main loop
    while(1) {

        // wait for event
        __SEV(); // set event
        __WFE(); // wait for event
        __WFE(); // wait for event
    }
}

//=========================== interrupt handlers ==============================

void _happyserial_rx_cb(uint8_t* buf, uint8_t bufLen) {
    happyserial_tx(buf,bufLen);
}
