#include "peripherals/syscfg.h"
#include "rp2040.h"

number RP2040SysCfg::readUint32(number offset) {
  switch (offset) {
  case PROC0_NMI_MASK:
    return this->rp2040->interruptNMIMask;
  }
  return LoggingPeripheral::readUint32(offset);
}

void RP2040SysCfg::writeUint32(number offset, number value) {
  switch (offset) {
  case PROC0_NMI_MASK:
    this->rp2040->interruptNMIMask = value;
    break;

  default:
    return LoggingPeripheral::writeUint32(offset, value);
  }
}
