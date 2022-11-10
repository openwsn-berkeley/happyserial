#ifndef __HAPPYSERIAL_H
#define __HAPPYSERIAL_H

#include <stdint.h>

#define HAPPYVERSION {1,1,0}

//=========================== define ==========================================

//=========================== typedef =========================================

typedef void (*happyserial_rx_cbt)(uint8_t* buf, uint8_t bufLen);

//=========================== prototypes ======================================

void happyserial_init(happyserial_rx_cbt happyserial_rx_cb);
void happyserial_tx(uint8_t* buf, uint8_t bufLen);

#endif