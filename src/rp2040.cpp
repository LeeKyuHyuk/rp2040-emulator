#include "rp2040.h"
#include "bootrom.h"
#include "intelhex.h"
#include <cstring>
#include <format>
#include <iostream>
#include <vector>

uint32_t RP2040::signExtend8(uint32_t value) {
  return value & 0x80 ? 0x80000000 + (value & 0x7f) : value;
}
uint32_t RP2040::signExtend16(uint32_t value) {
  return value & 0x8000 ? 0x80000000 + (value & 0x7fff) : value;
}

RP2040::RP2040(string hex) {
  setSP(bootrom[0]);
  setPC(bootrom[1] & 0xFFFFFFFE);
  memset(this->flash, 0xFFFFFFFF, FLASH_SIZE);
  this->readHooks.emplace(SIO_START_ADDRESS + SIO_CPUID_OFFSET,
                          [](uint32_t address) -> uint32_t {
                            // Returns the current CPU core id
                            // (always 0 for now)
                            return 0;
                          });
  this->readHooks.emplace(
      XIP_SSI_BASE + SSI_SR_OFFSET,
      [&](uint32_t address) -> uint32_t { return this->SSI_SR_TFE_BITS; });

  uint32_t dr0 = 0;
  this->writeHooks.emplace(XIP_SSI_BASE + SSI_DR0_OFFSET,
                           [&](uint32_t address, uint32_t value) -> void {
                             const uint8_t CMD_READ_STATUS = 0x05;
                             if (value == CMD_READ_STATUS) {
                               dr0 = 1; // tell stage2 that we completed a write
                             }
                           });
  this->readHooks.emplace(XIP_SSI_BASE + SSI_DR0_OFFSET,
                          [&](uint32_t address) -> uint32_t { return dr0; });

  this->readHooks.emplace(CLOCKS_BASE + CLK_REF_SELECTED,
                          [&](uint32_t address) -> uint32_t { return 1; });
  this->readHooks.emplace(CLOCKS_BASE + CLK_SYS_SELECTED,
                          [&](uint32_t address) -> uint32_t { return 1; });

  uint64_t VTOR = 0;
  this->writeHooks.emplace(SYSTEM_CONTROL_BLOCK + OFFSET_VTOR,
                           [&](uint32_t address, uint32_t value) -> void {
                             cout << "we write to VicTOR" << endl;
                             VTOR = value;
                           });
  this->readHooks.emplace(SYSTEM_CONTROL_BLOCK + OFFSET_VTOR,
                          [&](uint32_t address) -> uint32_t {
                            cout << "we read from VicTOR" << endl;
                            return VTOR;
                          });

  loadHex(hex, this->flash);
}

uint32_t RP2040::getSP() { return this->registers[13]; }

void RP2040::setSP(uint32_t value) { this->registers[13] = value; }

uint32_t RP2040::getLR() { return this->registers[14]; }

void RP2040::setLR(uint32_t value) { this->registers[14] = value; }

uint32_t RP2040::getPC() { return this->registers[15]; }

void RP2040::setPC(uint32_t value) { this->registers[15] = value; }

bool RP2040::checkCondition(uint32_t cond) { // Evaluate base condition.
  bool result = false;
  switch (cond >> 1) {
  case 0b000:
    result = this->Z;
    break;
  case 0b001:
    result = this->C;
    break;
  case 0b010:
    result = this->N;
    break;
  case 0b011:
    result = this->V;
    break;
  case 0b100:
    result = this->C && !this->Z;
    break;
  case 0b101:
    result = this->N == this->V;
    break;
  case 0b110:
    result = this->N == this->V && !this->Z;
    break;
  case 0b111:
    result = true;
    break;
  }
  return cond & 0b1 && cond != 0b1111 ? !result : result;
}

uint32_t RP2040::readUint32(uint32_t address) {
  if (address & 0x3) {
    throw new runtime_error("read from address " + format("0x{:x}", address) +
                            ", which is not 32 bit aligned");
  }
  uint32_t value;
  if (address < BOOT_ROM_SIZE * 4) {
    return bootrom[address / 4];
  } else if (address >= FLASH_START_ADDRESS && address < FLASH_END_ADDRESS) {
    memcpy(&value, &(this->flash[address - FLASH_START_ADDRESS]),
           sizeof(uint32_t));
    return value;
  } else if (address >= RAM_START_ADDRESS &&
             address < RAM_START_ADDRESS + SRAM_SIZE) {
    memcpy(&value, &(this->sram[address - RAM_START_ADDRESS]),
           sizeof(uint32_t));
    return value;
  } else {
    map<uint32_t, function<uint32_t(uint32_t)>>::iterator iter =
        (this->readHooks).find(address);
    // Operates when there is a value for the
    // corresponding key in the readHooks map
    if (iter != this->readHooks.end()) {
      return (iter->second)(address);
    }
  }
  cout << "Read from invalid memory address "
       << "0x" << hex << address << endl;
  return 0xffffffff;
}

/** We assume the address is 16-bit aligned */
uint16_t RP2040::readUint16(uint32_t address) {
  uint32_t value = this->readUint32(address & 0xfffffffc);
  return address & 0x2 ? (value & 0xffff0000) >> 16 : value & 0xffff;
}

uint8_t RP2040::readUint8(uint32_t address) {
  const uint16_t value = this->readUint16(address & 0xfffffffe);
  return (address & 0x1 ? (value & 0xff00) >> 8 : value & 0xff) >> 0;
}

void RP2040::writeUint32(uint32_t address, uint32_t value) {
  if (address >= RAM_START_ADDRESS && address < RAM_START_ADDRESS + SRAM_SIZE) {
    memcpy(&(this->sram[address - RAM_START_ADDRESS]), &value,
           sizeof(uint32_t));
  } else if (address >= SIO_START_ADDRESS &&
             address < SIO_START_ADDRESS + 0x10000000) {
    const uint32_t sioAddress = address - SIO_START_ADDRESS;
    // SIO write
    vector<uint32_t> pinList = {};
    for (uint8_t index = 0; index < 32; index++) {
      if (value & (1 << index)) {
        pinList.push_back(index);
      }
    }
    if (sioAddress == 20) {
      cout << "GPIO pins ";
      for (uint64_t index = 0; index < pinList.size(); index++) {
        if (index != 0) {
          cout << ", ";
        }
        cout << pinList.at(index);
      }
      cout << " set to HIGH" << endl;
    } else if (sioAddress == 24) {
      printf("GPIO pins ");
      for (uint64_t index = 0; index < pinList.size(); index++) {
        if (index != 0) {
          cout << ", ";
        }
        cout << pinList.at(index);
      }
      cout << " set to LOW" << endl;
    } else {
      cout << "Someone wrote "
           << "0x" << hex << value << " to "
           << "0x" << hex << sioAddress << endl;
    }
  } else {
    map<uint32_t, function<void(uint32_t, uint32_t)>>::iterator iter =
        (this->writeHooks).find(address);
    // Operates when there is a value for the
    // corresponding key in the writeHooks map
    if (iter != this->writeHooks.end()) {
      return (iter->second)(address, value);
    }
  }
}

void RP2040::writeUint16(uint32_t address, uint16_t value) {
  // we assume that addess is 16-bit aligned.
  // Ideally we should generate a fault if not!
  const uint32_t alignedAddress = address & 0xfffffffc;
  const uint32_t offset = address & 0x3;
  const uint32_t originalValue = this->readUint32(alignedAddress);
  uint8_t newValue[4] = {(uint8_t)originalValue, (uint8_t)(originalValue >> 8),
                         (uint8_t)(originalValue >> 16),
                         (uint8_t)(originalValue >> 24)};
  newValue[offset] = (uint8_t)value;
  newValue[offset + 1] = (uint8_t)(value >> 8);
  this->writeUint32(alignedAddress, newValue[0] | (newValue[1] << 8) |
                                        (newValue[2] << 16) |
                                        (newValue[3] << 24));
}

void RP2040::writeUint8(uint32_t address, uint8_t value) {
  const uint32_t alignedAddress = address & 0xfffffffc;
  const uint32_t offset = address & 0x3;
  const uint32_t originalValue = this->readUint32(alignedAddress);
  uint8_t newValue[4] = {(uint8_t)originalValue, (uint8_t)(originalValue >> 8),
                         (uint8_t)(originalValue >> 16),
                         (uint8_t)(originalValue >> 24)};
  newValue[offset] = value;
  this->writeUint32(alignedAddress, newValue[0] | (newValue[1] << 8) |
                                        (newValue[2] << 16) |
                                        (newValue[3] << 24));
}

void RP2040::executeInstruction() {
  // ARM Thumb instruction encoding - 16 bits / 2 bytes
  const uint64_t opcode = this->readUint16(this->getPC());
  const uint64_t opcode2 = this->readUint16(this->getPC() + 2);
  uint64_t opcodePC = this->getPC();
  this->setPC(this->getPC() + 2);
  // ADCS
  if (opcode >> 6 == 0b0100000101) {
    const uint64_t Rm = (opcode >> 3) & 0x7;
    const uint64_t Rdn = opcode & 0x7;
    const uint64_t leftValue = this->registers[Rdn];
    const uint64_t rightValue = this->registers[Rm];
    const uint64_t result = leftValue + rightValue + (this->C ? 1 : 0);
    this->registers[Rdn] = result;
    this->N = !!(result & 0x80000000);
    this->Z = (result & 0xffffffff) == 0;
    this->C = result >= 0xffffffff;
    this->V = (((int)leftValue | 0) >= 0 && ((int)rightValue | 0) >= 0 &&
               ((int)result | 0) < 0) ||
              (((int)leftValue | 0) <= 0 && ((int)rightValue | 0) <= 0 &&
               ((int)result | 0) > 0);
  }
  // ADD (SP plus immediate)
  else if (opcode >> 7 == 0b101100000) {
    const uint64_t imm32 = (opcode & 0x7f) << 2;
    this->setSP(this->getSP() + imm32);
  }
  // ADDS (Encoding T1)
  else if (opcode >> 9 == 0b0001110) {
    const uint64_t imm3 = (opcode >> 6) & 0x7;
    const uint64_t Rn = (opcode >> 3) & 0x7;
    const uint64_t Rd = opcode & 0x7;
    const uint64_t leftValue = this->registers[Rn];
    const uint64_t result = leftValue + imm3;
    this->registers[Rd] = result;
    this->N = !!(result & 0x80000000);
    this->Z = (result & 0xffffffff) == 0;
    this->C = result >= 0xffffffff;
    this->V = ((int)leftValue | 0) > 0 && imm3 < 0x80 && ((int)result | 0) < 0;
  }
  // ADDS (Encoding T2)
  else if (opcode >> 11 == 0b00110) {
    const uint64_t imm8 = opcode & 0xff;
    const uint64_t Rdn = (opcode >> 8) & 0x7;
    const uint64_t leftValue = this->registers[Rdn];
    const uint64_t result = leftValue + imm8;
    this->registers[Rdn] = result;
    this->N = !!(result & 0x80000000);
    this->Z = (result & 0xffffffff) == 0;
    this->C = result >= 0xffffffff;
    this->V = ((int)leftValue | 0) > 0 && imm8 < 0x80 && ((int)result | 0) < 0;
  }
  // ADDS (register)
  else if (opcode >> 9 == 0b0001100) {
    const uint64_t Rm = (opcode >> 6) & 0x7;
    const uint64_t Rn = (opcode >> 3) & 0x7;
    const uint64_t Rd = opcode & 0x7;
    const uint64_t leftValue = this->registers[Rn];
    const uint64_t rightValue = this->registers[Rm];
    const uint64_t result = leftValue + rightValue;
    this->registers[Rd] = result;
    this->N = !!(result & 0x80000000);
    this->Z = (result & 0xffffffff) == 0;
    this->C = result >= 0xffffffff;
    this->V =
        ((int)leftValue | 0) > 0 && rightValue < 0x80 && ((int)result | 0) < 0;
  }
  // ADD (register)
  else if (opcode >> 8 == 0b01000100) {
    const uint64_t regSP = 13;
    const uint64_t regPC = 15;
    const uint64_t Rm = (opcode >> 3) & 0xf;
    const uint64_t Rdn = ((opcode & 0x80) >> 4) | (opcode & 0x7);
    const uint64_t leftValue =
        Rdn == regPC ? this->getPC() + 2 : this->registers[Rdn];
    const uint64_t rightValue = this->registers[Rm];
    const uint64_t result = leftValue + rightValue;
    this->registers[Rdn] = Rdn == regPC ? result & ~0x1 : result;
    if (Rdn != regSP && Rdn != regPC) {
      this->N = !!(result & 0x80000000);
      this->Z = (result & 0xffffffff) == 0;
      this->C = result >= 0xffffffff;
      this->V = ((int)leftValue | 0) > 0 && rightValue < 0x80 &&
                ((int)result | 0) < 0;
    }
  }
  // ADR
  else if (opcode >> 11 == 0b10100) {
    const uint64_t imm8 = opcode & 0xff;
    const uint64_t Rd = (opcode >> 8) & 0x7;
    this->registers[Rd] = (opcodePC & 0xfffffffc) + 4 + (imm8 << 2);
  }
  // ANDS (Encoding T2)
  else if (opcode >> 6 == 0b0100000000) {
    const uint64_t Rm = (opcode >> 3) & 0x7;
    const uint64_t Rdn = opcode & 0x7;
    const uint64_t result = this->registers[Rdn] & this->registers[Rm];
    this->registers[Rdn] = result;
    this->N = !!(result & 0x80000000);
    this->Z = (result & 0xffffffff) == 0;
  }
  // B (with cond)
  else if (opcode >> 12 == 0b1101) {
    uint64_t imm8 = (opcode & 0xff) << 1;
    const uint64_t cond = (opcode >> 8) & 0xf;
    if (imm8 & (1 << 8)) {
      imm8 = (imm8 & 0x1ff) - 0x200;
    }
    if (this->checkCondition(cond)) {
      this->setPC(getPC() + imm8 + 2);
    }
  }
  // B
  else if (opcode >> 11 == 0b11100) {
    uint64_t imm11 = (opcode & 0x7ff) << 1;
    if (imm11 & (1 << 11)) {
      imm11 = (imm11 & 0x7FF) - 0x800;
    }
    this->setPC(getPC() + imm11 + 2);
  }
  // BICS
  else if (opcode >> 6 == 0b0100001110) {
    uint64_t Rm = (opcode >> 3) & 0x7;
    uint64_t Rdn = opcode & 0x7;
    const uint64_t result = (this->registers[Rdn] &= ~this->registers[Rm]);
    this->N = !!(result & 0x80000000);
    this->Z = result == 0;
  }
  // BL
  else if (opcode >> 11 == 0b11110 && opcode2 >> 14 == 0b11 &&
           ((opcode2 >> 12) & 0x1) == 1) {
    const uint64_t imm11 = opcode2 & 0x7ff;
    const uint64_t J2 = (opcode2 >> 11) & 0x1;
    const uint64_t J1 = (opcode2 >> 13) & 0x1;
    const uint64_t imm10 = opcode & 0x3ff;
    const uint64_t S = (opcode >> 10) & 0x1;
    const uint64_t I1 = 1 - (S ^ J1);
    const uint64_t I2 = 1 - (S ^ J2);
    const uint64_t imm32 =
        ((S ? 0b11111111 : 0) << 24) |
        ((I1 << 23) | (I2 << 22) | (imm10 << 12) | (imm11 << 1));
    this->setLR(this->getPC() + 2);
    this->setPC(this->getPC() + 2 + imm32);
  }
  // BLX
  else if (opcode >> 7 == 0b010001111 && (opcode & 0x7) == 0) {
    const uint64_t Rm = (opcode >> 3) & 0xf;
    this->setLR(this->getPC());
    this->setPC(this->registers[Rm] & ~1);
  }
  // BX
  else if (opcode >> 7 == 0b010001110 && (opcode & 0x7) == 0) {
    const uint64_t Rm = (opcode >> 3) & 0xf;
    this->setPC(this->registers[Rm] & ~1);
  }
  // CMP immediate
  else if (opcode >> 11 == 0b00101) {
    const uint64_t Rn = (opcode >> 8) & 0x7;
    const uint64_t imm8 = opcode & 0xff;
    const uint64_t value = this->registers[Rn] | 0;
    const uint64_t result = (value - imm8) | 0;
    this->N = value < imm8;
    this->Z = value == imm8;
    this->C = value >= imm8;
    this->V = value < 0 && imm8 > 0 && result > 0;
  }
  // CMP (register)
  else if (opcode >> 6 == 0b0100001010) {
    const uint64_t Rm = (opcode >> 3) & 0x7;
    const uint64_t Rn = opcode & 0x7;
    const uint64_t leftValue = this->registers[Rn] | 0;
    const uint64_t rightValue = this->registers[Rm] | 0;
    const uint64_t result = (leftValue - rightValue) | 0;
    this->N = leftValue < rightValue;
    this->Z = leftValue == rightValue;
    this->C = leftValue >= rightValue;
    this->V = (leftValue > 0 && rightValue < 0 && result < 0) ||
              (leftValue < 0 && rightValue > 0 && result > 0);
  }
  // DMB SY
  else if (opcode == 0xf3bf && opcode2 == 0x8f5f) {
    this->setPC(this->getPC() + 2);
  }
  // LDMIA
  else if (opcode >> 11 == 0b11001) {
    const uint64_t Rn = (opcode >> 8) & 0x7;
    const uint64_t registers = opcode & 0xff;
    uint64_t address = this->registers[Rn];
    for (uint8_t i = 0; i < 8; i++) {
      if (registers & (1 << i)) {
        this->registers[i] = this->readUint32(address);
        address += 4;
      }
    }
    // Write back
    if (!(registers & (1 << Rn))) {
      this->registers[Rn] = address;
    }
  }
  // LDR (immediate)
  else if (opcode >> 11 == 0b01101) {
    const uint64_t imm5 = ((opcode >> 6) & 0x1f) << 2;
    const uint64_t Rn = (opcode >> 3) & 0x7;
    const uint64_t Rt = opcode & 0x7;
    const uint64_t addr = this->registers[Rn] + imm5;
    this->registers[Rt] = this->readUint32(addr);
  }
  // LDR (literal)
  else if (opcode >> 11 == 0b01001) {
    const uint64_t imm8 = (opcode & 0xff) << 2;
    const uint64_t Rt = (opcode >> 8) & 7;
    const uint64_t nextPC = this->getPC() + 2;
    const uint64_t addr = (nextPC & 0xfffffffc) + imm8;
    cout << "reading from 0x" << hex << addr << endl;
    cout << "value: 0x" << hex << this->readUint32(addr) << endl;
    this->registers[Rt] = this->readUint32(addr);
  }
  // LDR (register)
  else if (opcode >> 9 == 0b0101100) {
    const uint64_t Rm = (opcode >> 6) & 0x7;
    const uint64_t Rn = (opcode >> 3) & 0x7;
    const uint64_t Rt = opcode & 0x7;
    const uint64_t addr = this->registers[Rm] + this->registers[Rn];
    cout << "reading from 0x" << hex << addr << endl;
    cout << "value: 0x" << hex << this->readUint32(addr) << endl;
    this->registers[Rt] = this->readUint32(addr);
  }
  // LDRB (immediate)
  else if (opcode >> 11 == 0b01111) {
    const uint64_t imm5 = (opcode >> 6) & 0x1f;
    const uint64_t Rn = (opcode >> 3) & 0x7;
    const uint64_t Rt = opcode & 0x7;
    const uint64_t addr = this->registers[Rn] + imm5;
    this->registers[Rt] = this->readUint8(addr);
  }
  // LDRB (register)
  else if (opcode >> 9 == 0b0101110) {
    const uint64_t Rm = (opcode >> 6) & 0x7;
    const uint64_t Rn = (opcode >> 3) & 0x7;
    const uint64_t Rt = opcode & 0x7;
    const uint64_t addr = this->registers[Rm] + this->registers[Rn];
    this->registers[Rt] = this->readUint8(addr);
  }
  // LDRH (immediate)
  else if (opcode >> 11 == 0b10001) {
    const uint64_t imm5 = (opcode >> 6) & 0x1f;
    const uint64_t Rn = (opcode >> 3) & 0x7;
    const uint64_t Rt = opcode & 0x7;
    const uint64_t addr = this->registers[Rn] + (imm5 << 1);
    this->registers[Rt] = this->readUint16(addr);
  }
  // LDRH (register)
  else if (opcode >> 9 == 0b0101101) {
    const uint64_t Rm = (opcode >> 6) & 0x7;
    const uint64_t Rn = (opcode >> 3) & 0x7;
    const uint64_t Rt = opcode & 0x7;
    const uint64_t addr = this->registers[Rm] + this->registers[Rn];
    this->registers[Rt] = this->readUint16(addr);
  }
  // LDRSB
  else if (opcode >> 9 == 0b0101011) {
    const uint64_t Rm = (opcode >> 6) & 0x7;
    const uint64_t Rn = (opcode >> 3) & 0x7;
    const uint64_t Rt = opcode & 0x7;
    const uint64_t addr = this->registers[Rm] + this->registers[Rn];
    this->registers[Rt] = signExtend8(this->readUint8(addr));
  }
  // LDRSH
  else if (opcode >> 9 == 0b0101111) {
    const uint64_t Rm = (opcode >> 6) & 0x7;
    const uint64_t Rn = (opcode >> 3) & 0x7;
    const uint64_t Rt = opcode & 0x7;
    const uint64_t addr = this->registers[Rm] + this->registers[Rn];
    this->registers[Rt] = signExtend16(this->readUint16(addr));
  }
  // LSLS (immediate)
  else if (opcode >> 11 == 0b00000) {
    const uint64_t imm5 = (opcode >> 6) & 0x1f;
    const uint64_t Rm = (opcode >> 3) & 0x7;
    const uint64_t Rd = opcode & 0x7;
    const uint64_t input = this->registers[Rm];
    const uint64_t result = input << imm5;
    this->registers[Rd] = result;
    this->N = !!(result & 0x80000000);
    this->Z = result == 0;
    this->C = imm5 ? !!(input & (1 << (32 - imm5))) : this->C;
  }
  // LSLS (register)
  else if (opcode >> 6 == 0b0100000010) {
    const uint64_t Rm = (opcode >> 3) & 0x7;
    const uint64_t Rdn = opcode & 0x7;
    const uint64_t input = this->registers[Rdn];
    const uint64_t shiftCount = this->registers[Rm] & 0xff;
    const uint64_t result = input << shiftCount;
    this->registers[Rdn] = result;
    this->N = !!(result & 0x80000000);
    this->Z = result == 0;
    this->C = shiftCount ? !!(input & (1 << (32 - shiftCount))) : this->C;
  }
  // LSLR (immediate)
  else if (opcode >> 11 == 0b00001) {
    const uint64_t imm5 = (opcode >> 6) & 0x1f;
    const uint64_t Rm = (opcode >> 3) & 0x7;
    const uint64_t Rd = opcode & 0x7;
    const uint64_t input = this->registers[Rm];
    const uint64_t result = imm5 ? input >> imm5 : 0;
    this->registers[Rd] = result;
    this->N = !!(result & 0x80000000);
    this->Z = result == 0;
    this->C = !!((input >> (imm5 ? imm5 - 1 : 31)) & 0x1);
  }
  // MOV
  else if (opcode >> 8 == 0b01000110) {
    const uint64_t Rm = (opcode >> 3) & 0xf;
    const uint64_t Rd = ((opcode >> 4) & 0x8) | (opcode & 0x7);
    this->registers[Rd] =
        Rm == PC_REGISTER ? this->getPC() + 2 : this->registers[Rm];
  }
  // MOVS
  else if (opcode >> 11 == 0b00100) {
    const uint64_t value = opcode & 0xff;
    const uint64_t Rd = (opcode >> 8) & 7;
    this->registers[Rd] = value;
    this->N = !!(value & 0x80000000);
    this->Z = value == 0;
  }
  // MRS
  else if (opcode == 0b1111001111101111 && opcode2 >> 12 == 0b1000) {
    this->setPC(this->getPC() + 2);
    cout << "MRS!" << endl;
  }
  // MSR
  else if (opcode >> 4 == 0b111100111000 && opcode2 >> 8 == 0b10001000) {
    this->setPC(this->getPC() + 2);
    cout << "MSR!" << endl;
  }
  // ORRS (Encoding T2)
  else if (opcode >> 6 == 0b0100001100) {
    const uint64_t Rm = (opcode >> 3) & 0x7;
    const uint64_t Rdn = opcode & 0x7;
    const uint64_t result = this->registers[Rdn] | this->registers[Rm];
    this->registers[Rdn] = result;
    this->N = !!(result & 0x80000000);
    this->Z = (result & 0xffffffff) == 0;
  }
  // POP
  else if (opcode >> 9 == 0b1011110) {
    uint64_t address = this->getSP();
    for (uint8_t i = 0; i <= 7; i++) {
      if (opcode & (1 << i)) {
        this->registers[i] = this->readUint32(address);
        address += 4;
      }
    }
    if ((opcode >> 8) & 1) {
      this->setPC(this->readUint32(address));
      this->writeUint32(address, this->registers[14]);
      address += 4;
    }
    this->setSP(address);
  }
  // PUSH
  else if (opcode >> 9 == 0b1011010) {
    uint64_t bitCount = 0;
    for (uint64_t i = 0; i <= 8; i++) {
      if (opcode & (1 << i)) {
        bitCount++;
      }
    }
    uint64_t address = this->getSP() - 4 * bitCount;
    for (uint64_t i = 0; i <= 7; i++) {
      if (opcode & (1 << i)) {
        this->writeUint32(address, this->registers[i]);
        address += 4;
      }
    }
    if (opcode & (1 << 8)) {
      this->writeUint32(address, this->registers[14]);
    }
    this->setSP(getSP() - 4 * bitCount);
  }
  // NEGS
  else if (opcode >> 6 == 0b0100001001) {
    uint64_t Rn = (opcode >> 3) & 0x7;
    uint64_t Rd = opcode & 0x7;
    const uint64_t value = this->registers[Rn] | 0;
    this->registers[Rd] = -value;
    this->N = value > 0;
    this->Z = value == 0;
    this->C = value == 0xFFFFFFFF;
    this->V = value == 0x7fffffff;
  }
  // SBCS (Encoding T2)
  else if (opcode >> 6 == 0b0100000110) {
    uint64_t Rm = (opcode >> 3) & 0x7;
    uint64_t Rdn = opcode & 0x7;
    const uint64_t operand1 = this->registers[Rdn];
    const uint64_t operand2 = this->registers[Rm] + (this->C ? 0 : 1);
    const uint64_t result = (int)(operand1 - operand2) | 0;
    this->registers[Rdn] = result;
    this->N = operand1 < operand2;
    this->Z = operand1 == operand2;
    this->C = operand1 >= operand2;
    this->V = ((int)operand1 | 0) < 0 && operand2 > 0 && result > 0;
  }
  // STMIA
  else if (opcode >> 11 == 0b11000) {
    const uint64_t Rn = (opcode >> 8) & 0x7;
    const uint64_t registers = opcode & 0xff;
    uint64_t address = this->registers[Rn];
    for (uint8_t i = 0; i < 8; i++) {
      if (registers & (1 << i)) {
        this->writeUint32(address, this->registers[i]);
        address += 4;
      }
    }
    // Write back
    if (!(registers & (1 << Rn))) {
      this->registers[Rn] = address;
    }
  }
  // STR (immediate)
  else if (opcode >> 11 == 0b01100) {
    const uint64_t imm5 = ((opcode >> 6) & 0x1f) << 2;
    const uint64_t Rn = (opcode >> 3) & 0x7;
    const uint64_t Rt = opcode & 0x7;
    const uint64_t address = this->registers[Rn] + imm5;
    this->writeUint32(address, this->registers[Rt]);
  }
  // STR (register)
  else if (opcode >> 9 == 0b0101000) {
    const uint64_t Rm = (opcode >> 6) & 0x7;
    const uint64_t Rn = (opcode >> 3) & 0x7;
    const uint64_t Rt = opcode & 0x7;
    const uint64_t addres = this->registers[Rm] + this->registers[Rn];
    this->writeUint32(addres, this->registers[Rt]);
  }
  // STRB (immediate)
  else if (opcode >> 11 == 0b01110) {
    const uint64_t imm5 = (opcode >> 6) & 0x1f;
    const uint64_t Rn = (opcode >> 3) & 0x7;
    const uint64_t Rt = opcode & 0x7;
    const uint64_t address = this->registers[Rn] + imm5;
    this->writeUint8(address, this->registers[Rt]);
  }
  // STRB (register)
  else if (opcode >> 9 == 0b0101010) {
    const uint64_t Rm = (opcode >> 6) & 0x7;
    const uint64_t Rn = (opcode >> 3) & 0x7;
    const uint64_t Rt = opcode & 0x7;
    const uint64_t addres = this->registers[Rm] + this->registers[Rn];
    this->writeUint8(addres, this->registers[Rt]);
  }
  // STRH (immediate)
  else if (opcode >> 11 == 0b10000) {
    const uint64_t imm5 = ((opcode >> 6) & 0x1f) << 1;
    const uint64_t Rn = (opcode >> 3) & 0x7;
    const uint64_t Rt = opcode & 0x7;
    const uint64_t address = this->registers[Rn] + imm5;
    this->writeUint16(address, this->registers[Rt]);
  }
  // STRH (register)
  else if (opcode >> 9 == 0b0101001) {
    const uint64_t Rm = (opcode >> 6) & 0x7;
    const uint64_t Rn = (opcode >> 3) & 0x7;
    const uint64_t Rt = opcode & 0x7;
    const uint64_t addres = this->registers[Rm] + this->registers[Rn];
    this->writeUint16(addres, this->registers[Rt]);
  }
  // SUB (SP minus immediate)
  else if (opcode >> 7 == 0b101100001) {
    const uint64_t imm32 = (opcode & 0x7f) << 2;
    this->setSP(this->getSP() - imm32);
  }
  // SUBS (Encoding T1)
  else if (opcode >> 9 == 0b0001111) {
    const uint64_t imm3 = (opcode >> 6) & 0x7;
    const uint64_t Rn = (opcode >> 3) & 0x7;
    const uint64_t Rd = opcode & 0x7;
    const uint64_t value = this->registers[Rn];
    const uint64_t result = (int)(value - imm3) | 0;
    this->registers[Rd] = result;
    this->N = value < imm3;
    this->Z = value == imm3;
    this->C = value >= imm3;
    this->V = ((int)value | 0) < 0 && imm3 > 0 && result > 0;
  }
  // SUBS (Encoding T2)
  else if (opcode >> 11 == 0b00111) {
    const uint64_t imm8 = opcode & 0xff;
    const uint64_t Rdn = (opcode >> 8) & 0x7;
    const uint64_t value = this->registers[Rdn];
    const uint64_t result = (value - imm8) | 0;
    this->registers[Rdn] = result;
    this->N = value < imm8;
    this->Z = value == imm8;
    this->C = value >= imm8;
    this->V = ((int)value | 0) < 0 && imm8 > 0 && result > 0;
  }
  // SUBS (register)
  else if (opcode >> 9 == 0b0001101) {
    const uint64_t Rm = (opcode >> 6) & 0x7;
    const uint64_t Rn = (opcode >> 3) & 0x7;
    const uint64_t Rd = opcode & 0x7;
    const uint64_t leftValue = this->registers[Rn];
    const uint64_t rightValue = this->registers[Rm];
    const uint64_t result = (int)(leftValue - rightValue) | 0;
    this->registers[Rd] = result;
    this->N = leftValue < rightValue;
    this->Z = leftValue == rightValue;
    this->C = leftValue >= rightValue;
    this->V = ((int)leftValue | 0) < 0 && rightValue > 0 && result > 0;
  }
  // TST
  else if (opcode >> 6 == 0b0100001000) {
    const uint64_t Rm = (opcode >> 3) & 0x7;
    const uint64_t Rn = opcode & 0x7;
    const uint64_t result = this->registers[Rn] & this->registers[Rm];
    this->N = !!(result & 0x80000000);
    this->Z = result == 0;
  }
  // UXTB
  else if (opcode >> 6 == 0b1011001011) {
    const uint64_t Rm = (opcode >> 3) & 0x7;
    const uint64_t Rd = opcode & 0x7;
    this->registers[Rd] = this->registers[Rm] & 0xff;
  } else {
    cout << "Warning: Instruction at "
         << "0x" << hex << opcodePC << " is not implemented yet!" << endl;
    cout << "Opcode: 0x" << hex << opcode << " (0x" << hex << opcode2 << ")"
         << endl;
  }
}