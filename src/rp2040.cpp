#include "rp2040.h"
#include "intelhex.h"
#include <cstring>
#include <iostream>
#include <vector>

uint32_t RP2040::signExtend8(uint32_t value) {
  return value & 0x80 ? 0x80000000 + (value & 0x7f) : value;
}
uint32_t RP2040::signExtend16(uint32_t value) {
  return value & 0x8000 ? 0x80000000 + (value & 0x7fff) : value;
}

RP2040::RP2040(string hex) {
  setSP(0x20041000);
  memset(this->flash, 0xFFFFFFFF, FLASH_SIZE);
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
  uint32_t value;
  if (address < FLASH_START_ADDRESS) {
    // TODO: should be readonly from bootrom once we have it
    memcpy(&value, &(this->flash[address]), sizeof(uint32_t));
    return value;
  } else if (address >= FLASH_START_ADDRESS && address < RAM_START_ADDRESS) {
    memcpy(&value, &(this->flash[address - FLASH_START_ADDRESS]),
           sizeof(uint32_t));
    return value;
  } else if (address >= RAM_START_ADDRESS &&
             address < RAM_START_ADDRESS + SRAM_SIZE) {
    memcpy(&value, &(this->sram[address - RAM_START_ADDRESS]),
           sizeof(uint32_t));
    return value;
  } else {
    map<uint32_t, CPUReadCallback>::iterator iter =
        (this->readHooks).find(address);
    // Operates when there is a value for the
    // corresponding key in the readHooks map
    if (iter != this->readHooks.end()) {
      return (*iter->second)(address);
    }
  }
  cout << "Read from invalid memory address "
       << "0x" << hex << address << endl;
  return 0xffffffff;
}

uint16_t RP2040::readUint16(uint32_t address) {
  return this->readUint32(address) & 0xFFFF;
}

void RP2040::writeUint32(uint32_t address, uint32_t value) {
  if (address >= RAM_START_ADDRESS && address < RAM_START_ADDRESS + SRAM_SIZE) {
    memcpy(&(this->sram[address - RAM_START_ADDRESS]), &value,
           sizeof(uint32_t));
  }
  if (address >= SIO_START_ADDRESS &&
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
  }
}

void RP2040::executeInstruction() {
  // ARM Thumb instruction encoding - 16 bits / 2 bytes
  const uint16_t opcode = this->flash16[this->getPC() / 2];
  const uint16_t opcode2 = this->flash16[this->getPC() / 2 + 1];
  // B (with cond)
  if (opcode >> 12 == 0b1101) {
    uint32_t imm8 = (opcode & 0xff) << 1;
    const uint32_t cond = (opcode >> 8) & 0xf;
    if (imm8 & (1 << 8)) {
      imm8 = (imm8 & 0x1ff) - 0x200;
    }
    if (this->checkCondition(cond)) {
      this->setPC(getPC() + imm8 + 2);
    }
  }
  // B
  else if (opcode >> 11 == 0b11100) {
    uint32_t imm11 = (opcode & 0x7ff) << 1;
    if (imm11 & (1 << 11)) {
      imm11 = (imm11 & 0x7FF) - 0x800;
    }
    this->setPC(getPC() + imm11 + 2);
  }
  // BL
  else if (opcode >> 11 == 0b11110 && opcode2 >> 14 == 0b11) {
    // right now we just ignore it. but let's print it!
    cout << "BL ignored" << endl;
    this->setPC(this->getPC() + 2);
  }
  // CMP immediate
  else if (opcode >> 11 == 0b00101) {
    const uint32_t Rn = (opcode >> 8) & 0x7;
    const uint32_t imm8 = signExtend8(opcode & 0xff);
    const uint32_t value = this->registers[Rn] | 0;
    const uint32_t result = (value - imm8) | 0;
    this->N = value < imm8;
    this->Z = value == imm8;
    this->C = value >= imm8;
    this->V = (value > 0 && imm8 < 0 && result < 0) ||
              (value < 0 && imm8 > 0 && result > 0);
  }
  // CMP (register)
  else if (opcode >> 6 == 0b0100001010) {
    const uint32_t Rm = (opcode >> 3) & 0x7;
    const uint32_t Rn = opcode & 0x7;
    const uint32_t leftValue = this->registers[Rn] | 0;
    const uint32_t rightValue = this->registers[Rm] | 0;
    const uint32_t result = (leftValue - rightValue) | 0;
    this->N = leftValue < rightValue;
    this->Z = leftValue == rightValue;
    this->C = leftValue >= rightValue;
    this->V = (leftValue > 0 && rightValue < 0 && result < 0) ||
              (leftValue < 0 && rightValue > 0 && result > 0);
  }
  // LDR (immediate)
  else if (opcode >> 11 == 0b01101) {
    const uint32_t imm5 = ((opcode >> 6) & 0x1f) << 2;
    const uint32_t Rn = (opcode >> 3) & 0x7;
    const uint32_t Rt = opcode & 0x7;
    const uint32_t addr = this->registers[Rn] + imm5;
    this->registers[Rt] = this->readUint32(addr);
  }
  // LDR (literal)
  else if (opcode >> 11 == 0b01001) {
    const uint32_t imm8 = (opcode & 0xff) << 2;
    const uint32_t Rt = (opcode >> 8) & 7;
    const uint32_t nextPC = this->getPC() + 2;
    const uint32_t addr = nextPC - (nextPC % 4) + imm8;
    this->registers[Rt] = this->readUint32(addr);
  }
  // LDRSH (immediate)
  else if (opcode >> 9 == 0b0101111) {
    const uint32_t Rm = (opcode >> 6) & 0x7;
    const uint32_t Rn = (opcode >> 3) & 0x7;
    const uint32_t Rt = opcode & 0x7;
    const uint32_t addr = this->registers[Rm] + this->registers[Rn];
    this->registers[Rt] = signExtend16(this->readUint16(addr));
  }
  // LSLS
  else if (opcode >> 11 == 0b00000) {
    const uint32_t imm5 = (opcode >> 6) & 0x1f;
    const uint32_t Rm = (opcode >> 3) & 0x7;
    const uint32_t Rd = opcode & 0x7;
    const uint32_t input = this->registers[Rm];
    const uint32_t result = input << imm5;
    this->registers[Rd] = result;
    this->N = !!(result & 0x80000000);
    this->Z = result == 0;
    this->C = imm5 ? !!(input & (1 << (32 - imm5))) : this->C;
  }
  // MOVS
  else if (opcode >> 11 == 0b00100) {
    const uint32_t value = opcode & 0xff;
    const uint32_t Rd = (opcode >> 8) & 7;
    this->registers[Rd] = value;
    this->N = !!(value & 0x80000000);
    this->Z = value == 0;
  }
  // PUSH
  else if (opcode >> 9 == 0b1011010) {
    uint32_t bitCount = 0;
    for (uint32_t i = 0; i <= 8; i++) {
      if (opcode & (1 << i)) {
        bitCount++;
      }
    }
    uint32_t address = this->getSP() - 4 * bitCount;
    for (uint32_t i = 0; i <= 7; i++) {
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
  // STR (immediate)
  else if (opcode >> 11 == 0b01100) {
    const uint32_t imm5 = ((opcode >> 6) & 0x1f) << 2;
    const uint32_t Rn = (opcode >> 3) & 0x7;
    const uint32_t Rt = opcode & 0x7;
    const uint32_t address = this->registers[Rn] + imm5;
    this->writeUint32(address, this->registers[Rt]);
  }
  // TST
  else if (opcode >> 6 == 0b0100001000) {
    const uint32_t Rm = (opcode >> 3) & 0x7;
    const uint32_t Rn = opcode & 0x7;
    const uint32_t result = this->registers[Rn] & this->registers[Rm];
    this->N = !!(result & 0x80000000);
    this->Z = result == 0;
  } else {
    cout << "Warning: Instruction at "
         << "0x" << hex << this->getPC() << " is not implemented yet!" << endl;
  }

  this->setPC(getPC() + 2);
}