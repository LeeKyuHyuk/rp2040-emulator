#ifndef __UART_H__
#define __UART_H__

#include <cstdint>
#include <functional>

typedef uint64_t number;

using namespace std;

const number UART0_BASE = 0x40034000;
const number UART1_BASE = 0x40038000;

class RP2040;

class RPUART {
private:
  const number UARTDR = 0x0;
  const number UARTFR = 0x18;

  RP2040 *mcu;
  number baseAddress = UART0_BASE;

public:
  function<void(number)> onByte;

  RPUART(RP2040 *mcu, number baseAddress);
};

#endif