#ifndef __RP2040_H__
#define __RP2040_H__

#include <cstdint>
#include <functional>
#include <map>
#include <string>

#define SRAM_SIZE 264 * 1024
#define FLASH_SIZE 16 * 1024 * 1024

using namespace std;

const uint32_t FLASH_START_ADDRESS = 0x10000000;
const uint32_t FLASH_END_ADDRESS = 0x14000000;
const uint32_t RAM_START_ADDRESS = 0x20000000;
const uint32_t SIO_START_ADDRESS = 0xD0000000;

// const uint32_t APSR_N = 0x80000000;
// const uint32_t APSR_Z = 0x40000000;
// const uint32_t APSR_C = 0x20000000;
// const uint32_t APSR_V = 0x10000000;

class RP2040 {
private:
  const uint32_t SIO_CPUID_OFFSET = 0;

  const uint32_t XIP_SSI_BASE = 0x18000000;
  const uint32_t SSI_SR_OFFSET = 0x00000028;
  const uint32_t SSI_DR0_OFFSET = 0x00000060;
  const uint32_t SSI_SR_BUSY_BITS = 0x00000001;
  const uint32_t SSI_SR_TFE_BITS = 0x00000004;

  uint32_t signExtend8(uint32_t value);
  uint32_t signExtend16(uint32_t value);

public:
  uint8_t sram[SRAM_SIZE] = {
      0x00,
  };
  uint8_t flash[FLASH_SIZE] = {
      0x00,
  };
  uint16_t *flash16 = (uint16_t *)flash;
  uint32_t registers[16] = {
      0x00,
  };

  map<uint32_t, function<void(uint32_t, uint32_t)>> writeHooks;
  map<uint32_t, function<uint32_t(uint32_t)>> readHooks;

  // APSR fields
  bool N = false;
  bool C = false;
  bool Z = false;
  bool V = false;

  RP2040(string hex);

  uint32_t getSP();
  void setSP(uint32_t value);
  uint32_t getLR();
  void setLR(uint32_t value);
  uint32_t getPC();
  void setPC(uint32_t value);

  bool checkCondition(uint32_t cond);
  uint32_t readUint32(uint32_t address);
  uint16_t readUint16(uint32_t address);
  uint8_t readUint8(uint32_t address);
  void writeUint32(uint32_t address, uint32_t value);

  void executeInstruction();
};

#endif