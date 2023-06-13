#ifndef __UART_H__
#define __UART_H__

#include <cstdint>
#include <functional>

using namespace std;

const uint32_t UART0_BASE = 0x40034000;
const uint32_t UART1_BASE = 0x40038000;

class RP2040;

class RPUART {
private:
  const uint32_t UARTDR = 0x0;
  const uint32_t UARTFR = 0x18;

  RP2040 *mcu;
  uint32_t baseAddress = UART0_BASE;

public:
  function<void(uint32_t)> onByte;

  RPUART(RP2040 *mcu, uint32_t baseAddress);
};

#endif