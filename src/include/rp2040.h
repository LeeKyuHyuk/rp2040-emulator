#ifndef __RP2040_H__
#define __RP2040_H__

#include "bootrom.h"
#include "uart.h"
#include "utils/dataview.h"
#include <cstdint>
#include <functional>
#include <map>
#include <string>

#define SRAM_SIZE 264 * 1024
#define FLASH_SIZE 16 * 1024 * 1024

typedef uint64_t number;

using namespace std;

const number FLASH_START_ADDRESS = 0x10000000;
const number FLASH_END_ADDRESS = 0x14000000;
const number RAM_START_ADDRESS = 0x20000000;
const number SIO_START_ADDRESS = 0xD0000000;

class RP2040 {
private:
  const number SIO_CPUID_OFFSET = 0;

  const number XIP_SSI_BASE = 0x18000000;
  const number SSI_SR_OFFSET = 0x00000028;
  const number SSI_DR0_OFFSET = 0x00000060;
  const number SSI_SR_BUSY_BITS = 0x00000001;
  const number SSI_SR_TFE_BITS = 0x00000004;
  const number CLOCKS_BASE = 0x40008000;
  const number CLK_REF_SELECTED = 0x38;
  const number CLK_SYS_SELECTED = 0x44;

  const number SYSTEM_CONTROL_BLOCK = 0xe000ed00;
  const number OFFSET_VTOR = 0x8;

  const number PC_REGISTER = 15;

  number dr0 = 0;
  number VTOR = 0;

  number signExtend8(number value);
  number signExtend16(number value);

  bool stopped = false;
  number breakCount = 0;

public:
  uint32_t bootrom[BOOT_ROM_B1_SIZE] = {
      0x00,
  };
  uint8_t sram[SRAM_SIZE] = {
      0x00,
  };
  DataView *sramView = new DataView(this->sram, SRAM_SIZE);
  uint8_t flash[FLASH_SIZE] = {
      0x00,
  };
  uint16_t *flash16 = (uint16_t *)flash;
  DataView *flashView = new DataView(this->flash, FLASH_SIZE);
  uint32_t registers[16] = {
      0x00,
  };

  map<number, function<void(number, number)>> writeHooks;
  map<number, function<number(number)>> readHooks;

  RPUART *uart[2] = {new RPUART(this, UART0_BASE),
                     new RPUART(this, UART1_BASE)};

  // APSR fields
  bool N = false;
  bool C = false;
  bool Z = false;
  bool V = false;

  // Debugging
  void onBreak(number code);
  uint64_t getBreakCount();

  RP2040();
  void loadBootrom(const uint32_t *bootromData, number bootromSize);
  void reset();

  number getSP();
  void setSP(number value);
  number getLR();
  void setLR(number value);
  number getPC();
  void setPC(number value);

  bool checkCondition(number cond);
  number readUint32(number address);
  number readUint16(number address);
  number readUint8(number address);
  void writeUint32(number address, number value);
  void writeUint16(number address, number value);
  void writeUint8(number address, number value);

  void executeInstruction();
  void execute();
  void stop();
};

#endif