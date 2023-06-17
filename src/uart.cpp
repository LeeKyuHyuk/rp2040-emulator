#include "uart.h"
#include "rp2040.h"
#include <iostream>

RPUART::RPUART(RP2040 *mcu, number baseAddress) {
  this->mcu = mcu;
  this->baseAddress = baseAddress;

  mcu->writeHooks.emplace(baseAddress + UARTDR,
                          [&](number address, number value) -> void {
                            this->onByte(value & 0xff);
                          });

  mcu->readHooks.emplace(baseAddress + UARTFR,
                         [](number address) -> number { return 0; });
}