#include "peripherals/uart.h"
#include "rp2040.h"
#include <iostream>

number RPUART::readUint32(number offset) {
  switch (offset) {
  case UARTFR:
    return 0;
  }
  return LoggingPeripheral::readUint32(offset);
}

void RPUART::writeUint32(number offset, number value) {
  switch (offset) {
  case UARTDR:
    this->onByte(value & 0xff);
    break;

  default:
    LoggingPeripheral::writeUint32(offset, value);
  }
}
