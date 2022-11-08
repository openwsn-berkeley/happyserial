#include "nrf52840.h"
#include "happyserial.h"

//=========================== defines =========================================

//=========================== prototypes ======================================

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
    
    // main loop
    while(1) {

        // wait for event
        __SEV(); // set event
        __WFE(); // wait for event
        __WFE(); // wait for event
    }
}

//=========================== interrupt handlers ==============================
