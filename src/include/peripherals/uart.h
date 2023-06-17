#ifndef __UART_H__
#define __UART_H__

#include "peripheral.h"
#include <cstdint>
#include <functional>

typedef uint64_t number;

using namespace std;

class RP2040;

const number UARTDR = 0x0;
const number UARTFR = 0x18;

class RPUART : public LoggingPeripheral {
public:
  RPUART(RP2040 *rp2040, string name) : LoggingPeripheral(rp2040, name) {}

  function<void(number)> onByte;

  number readUint32(number offset);
  void writeUint32(number offset, number value);
};

#endif