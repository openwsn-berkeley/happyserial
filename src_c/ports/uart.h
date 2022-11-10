#ifndef __UART_H
#define __UART_H

#include <stdint.h>

//=========================== define ==========================================

//=========================== typedef =========================================

typedef void (*uart_txByteDone_cbt)(void);
typedef void (*uart_rxByte_cbt)(uint8_t b);

//=========================== prototypes ======================================

void uart_init(uart_txByteDone_cbt uart_txByteDone_cb, uart_rxByte_cbt uart_rxByte_cb);
void uart_txByte(uint8_t b);

#endif
