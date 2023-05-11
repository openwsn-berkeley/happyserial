#include <string.h>
#include <stdbool.h>
#include "nrf5340_application.h"
#include "uart.h"

//=========================== defines =========================================

#define UART_TXBUF_LEN 1
#define UART_RXBUF_LEN 1

//=========================== variables =======================================

typedef struct {
    // callbacks
    uart_txByteDone_cbt uart_txByteDone_cb;
    uart_rxByte_cbt     uart_rxByte_cb;
    // tx
    uint8_t txBuf[UART_TXBUF_LEN];
    // rx
    uint8_t rxBuf[UART_RXBUF_LEN];
} uart_vars_t;

uart_vars_t uart_vars;

typedef struct {
    bool      init;
    uint32_t  num_ISR_SERIAL0_IRQHandler;
    uint32_t  num_ISR_SERIAL0_IRQHandler_ENDTX;
    uint32_t  num_ISR_SERIAL0_IRQHandler_ENDRX;
} uart_dbg_t;

uart_dbg_t uart_dbg;

//=========================== prototypes ======================================

//=========================== public ==========================================

void uart_init(uart_txByteDone_cbt uart_txByteDone_cb, uart_rxByte_cbt uart_rxByte_cb) {

    // reset variable
    memset(&uart_vars, 0x00, sizeof(uart_vars_t));
    memset(&uart_dbg,  0x00, sizeof(uart_dbg_t));

    // store params
    uart_vars.uart_txByteDone_cb       = uart_txByteDone_cb;
    uart_vars.uart_rxByte_cb           = uart_rxByte_cb;
    
    // start HFCLOCK
    NRF_CLOCK_S->HFCLKSRC              = 0x00000001; // 0x00000001==HFXO
    NRF_CLOCK_S->HFCLK192MALWAYSRUN    = 0x00000001; // 0x00000001==AlwaysRun
    NRF_CLOCK_S->EVENTS_HFCLKSTARTED   = 0;
    NRF_CLOCK_S->TASKS_HFCLKSTART      = 0x00000001;
    while (NRF_CLOCK_S->EVENTS_HFCLKSTARTED == 0);

    // https://infocenter.nordicsemi.com/topic/ug_nrf5340_dk/UG/dk/vir_com_port.html
    // nRF5340 UART_1
    // P0.11	RTS
    // P1.01	TXD <===
    // P0.10	CTS
    // P1.00	RXD <===
    
    // configure
    NRF_UARTE0_S->PSEL.TXD             = 0x00000021; // 0x00000021==P1.01
    NRF_UARTE0_S->PSEL.RXD             = 0x00000020; // 0x00000020==P1.00
    NRF_UARTE0_S->ENABLE               = 0x00000008; // 8 = enable
    NRF_UARTE0_S->CONFIG               = 0x00000000; // 0x00000000==no flow control, no parity, no stop bits
    NRF_UARTE0_S->BAUDRATE             = 0x004EA000; // 0x004EA000==Baud19200
    NRF_UARTE0_S->SHORTS               = 0x00000020; // 0x00000020==only ENDRX_STARTRX
    NRF_UARTE0_S->TXD.PTR              = (uint32_t)(uart_vars.txBuf);
    NRF_UARTE0_S->TXD.MAXCNT           = sizeof(uart_vars.txBuf);
    NRF_UARTE0_S->RXD.PTR              = (uint32_t)(uart_vars.rxBuf);
    NRF_UARTE0_S->RXD.MAXCNT           = sizeof(uart_vars.rxBuf);
    NRF_UARTE0_S->INTENSET             = 0x00000110; // 0x00000110==enable ENDTX and ENDRX
    
    // enable interrupts
    NVIC_SetPriority(    SERIAL0_IRQn, 1);
    NVIC_ClearPendingIRQ(SERIAL0_IRQn);
    NVIC_EnableIRQ(      SERIAL0_IRQn);

    // start receiving
    NRF_UARTE0_S->EVENTS_RXSTARTED     = 0;
    NRF_UARTE0_S->TASKS_STARTRX        = 0x00000001; // 0x00000001==start RX state machine
    while (NRF_UARTE0_S->EVENTS_RXSTARTED == 0);

    // debug
    uart_dbg.init = true;
}

void uart_txByte(uint8_t b) {
    
    // prepare
    uart_vars.txBuf[0] = b;
    
    // start sending
    NRF_UARTE0_S->TASKS_STARTTX        = 0x00000001;
    // note: don't wait, interrupt-driven
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

void SERIAL0_IRQHandler(void) {

    // debug
    uart_dbg.num_ISR_SERIAL0_IRQHandler++;

    if (NRF_UARTE0_S->EVENTS_ENDTX == 0x00000001) {
        // byte sent to computer

        // clear
        NRF_UARTE0_S->EVENTS_ENDTX = 0x00000000;

        // debug
        uart_dbg.num_ISR_SERIAL0_IRQHandler_ENDTX++;

        // handle
        uart_vars.uart_txByteDone_cb();
    }

    if (NRF_UARTE0_S->EVENTS_ENDRX == 0x00000001) {
        // byte received from computer

        // clear
        NRF_UARTE0_S->EVENTS_ENDRX = 0x00000000;

        // debug
        uart_dbg.num_ISR_SERIAL0_IRQHandler_ENDRX++;

        // handle
        uart_vars.uart_rxByte_cb(uart_vars.rxBuf[0]);
    }
}
