#include "hdlc.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

uint16_t crcIteration(uint16_t crc, uint8_t byte) {
   return (crc >> 8) ^ fcstab[(crc ^ byte) & 0xff];
}

//=========================== private =========================================
