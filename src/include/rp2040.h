#ifndef __RP2040_H__
#define __RP2040_H__

#include <cstdint>
#include <string>

#define SRAM_SIZE 264 * 1024
#define FLASH_SIZE 16 * 1024 * 1024

using namespace std;

const uint32_t RAM_START_ADDRESS = 0x20000000;
const uint32_t SIO_START_ADDRESS = 0xD0000000;

class RP2040 {
public:
  uint8_t sram[SRAM_SIZE];
  uint8_t flash[FLASH_SIZE];
  uint16_t *flash16 = (uint16_t *)flash;
  uint32_t registers[16];

  RP2040(string hex);

  uint32_t getSP();
  void setSP(uint32_t value);
  uint32_t getLR();
  void setLR(uint32_t value);
  uint32_t getPC();
  void setPC(uint32_t value);

  void writeUint32(uint32_t address, uint32_t value);

  void executeInstruction();
};

#endif