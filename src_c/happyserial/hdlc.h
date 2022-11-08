#ifndef __HDLC_H
#define __HDLC_H

#include <stdint.h>

//=========================== define ==========================================

//=========================== typedef =========================================

typedef void (*hdlc_rx_cbt)(uint8_t* buf, uint8_t bufLen);

//=========================== prototypes ======================================

void hdlc_init(hdlc_rx_cbt hdlc_rx_cb);
void hdlc_tx(uint8_t* buf, uint8_t bufLen);

#endif