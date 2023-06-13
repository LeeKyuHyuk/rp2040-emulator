#include "uart.h"
#include "rp2040.h"
#include <iostream>

RPUART::RPUART(RP2040 *mcu, uint32_t baseAddress) {
  this->mcu = mcu;
  this->baseAddress = baseAddress;

  mcu->writeHooks.emplace(baseAddress + UARTDR,
                          [&](uint32_t address, uint32_t value) -> void {
                            this->onByte(value & 0xff);
                          });

  mcu->readHooks.emplace(baseAddress + UARTFR,
                         [](uint32_t address) -> uint32_t { return 0; });
}