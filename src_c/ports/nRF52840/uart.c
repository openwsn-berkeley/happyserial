#include <string.h>
#include <stdbool.h>
#include "nrf52840.h"
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
    uint32_t  num_ISR_UARTE0_UART0_IRQHandler;
    uint32_t  num_ISR_UARTE0_UART0_IRQHandler_ENDTX;
    uint32_t  num_ISR_UARTE0_UART0_IRQHandler_ENDRX;
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
    
    // start hfclock
    NRF_CLOCK->EVENTS_HFCLKSTARTED     = 0;
    NRF_CLOCK->TASKS_HFCLKSTART        = 0x00000001;
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);

    // https://infocenter.nordicsemi.com/topic/ug_nrf52840_dk/UG/dk/vir_com_port.html
    // P0.05	RTS
    // P0.06	TXD <===
    // P0.07	CTS
    // P0.08	RXD <===
    
    // configure
    NRF_UARTE0->RXD.PTR                = (uint32_t)uart_vars.rxBuf;
    NRF_UARTE0->RXD.MAXCNT             = UART_RXBUF_LEN;
    NRF_UARTE0->TXD.PTR                = (uint32_t)uart_vars.txBuf;
    NRF_UARTE0->TXD.MAXCNT             = UART_TXBUF_LEN;
    NRF_UARTE0->PSEL.TXD               = 0x00000006; // 0x00000006==P0.6
    NRF_UARTE0->PSEL.RXD               = 0x00000008; // 0x00000008==P0.8
    NRF_UARTE0->CONFIG                 = 0x00000000; // 0x00000000==no flow control, no parity bits, 1 stop bit
    NRF_UARTE0->BAUDRATE               = 0x004EA000; // 0x004EA000==Baud19200
    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... ..C. .... C: ENDRX_STARTRX
    // .... .... .... .... .... .... .D.. .... D: ENDRX_STOPRX
    // xxxx xxxx xxxx xxxx xxxx xxxx xx1x xxxx 
    //    0    0    0    0    0    0    2    0 0x00000020
    NRF_UARTE0->SHORTS                 = 0x00000020;
    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... .... ...A A: CTS
    // .... .... .... .... .... .... .... ..B. B: NCTS
    // .... .... .... .... .... .... .... .C.. C: RXDRDY
    // .... .... .... .... .... .... ...D .... D: ENDRX
    // .... .... .... .... .... .... E... .... E: TXDRDY
    // .... .... .... .... .... ...F .... .... F: ENDTX
    // .... .... .... .... .... ..G. .... .... G: ERROR
    // .... .... .... ..H. .... .... .... .... H: RXTO
    // .... .... .... I... .... .... .... .... I: RXSTARTED
    // .... .... ...J .... .... .... .... .... J: TXSTARTED
    // .... .... .L.. .... .... .... .... .... L: TXSTOPPED
    // xxxx xxxx x0x0 0x0x xxxx xx00 0xx1 x000 
    //    0    0    0    0    0    0    1    0 0x00000010
    NRF_UARTE0->INTENSET               = 0x00000010;
    NRF_UARTE0->ENABLE                 = 0x00000008; // 0x00000008==enable
    
    // enable interrupts
    NVIC_SetPriority(UARTE0_UART0_IRQn, 1);
    NVIC_ClearPendingIRQ(UARTE0_UART0_IRQn);
    NVIC_EnableIRQ(UARTE0_UART0_IRQn);

    // start receiving
    NRF_UARTE0->EVENTS_RXSTARTED       = 0;
    NRF_UARTE0->TASKS_STARTRX          = 0x00000001; // 0x00000001==start RX state machine; read received byte from RXD register
    while (NRF_UARTE0->EVENTS_RXSTARTED == 0);

    // debug
    uart_dbg.init = true;
}

void uart_txByte(uint8_t b) {
    
    // prepare
    uart_vars.txBuf[0] = b;
    
    // start sending
    NRF_UARTE0->TASKS_STARTTX    = 0x00000001;
    // note: don't wait, interrupt-driven
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

void UARTE0_UART0_IRQHandler(void) {

    // debug
    uart_dbg.num_ISR_UARTE0_UART0_IRQHandler++;

    if (NRF_UARTE0->EVENTS_ENDTX == 0x00000001) {
        // byte sent to computer

        // clear
        NRF_UARTE0->EVENTS_ENDTX = 0x00000000;

        // debug
        uart_dbg.num_ISR_UARTE0_UART0_IRQHandler_ENDTX++;

        // handle
        uart_vars.uart_txByteDone_cb();
    }

    if (NRF_UARTE0->EVENTS_ENDRX == 0x00000001) {
        // byte received from computer

        // clear
        NRF_UARTE0->EVENTS_ENDRX = 0x00000000;

        // debug
        uart_dbg.num_ISR_UARTE0_UART0_IRQHandler_ENDRX++;

        // handle
        uart_vars.uart_rxByte_cb(uart_vars.rxBuf[0]);
    }
}
