#include "rp2040.h"
#include "intelhex.h"
#include <cstring>
#include <iostream>
#include <vector>

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
  // B
  if (opcode >> 11 == 0b11100) {
    uint32_t imm11 = (opcode & 0x7ff) << 1;
    if (imm11 & (1 << 11)) {
      imm11 = (imm11 & 0x7FF) - 0x800;
    }
    this->setPC(getPC() + imm11 + 2);
  }
  // BL
  if (opcode >> 11 == 0b11110 && opcode2 >> 14 == 0b11) {
    // right now we just ignore it. but let's print it!
    cout << "BL ignored" << endl;
  }
  // LSLS
  else if (opcode >> 11 == 0b00000) {
    const uint32_t imm5 = (opcode >> 6) & 0x1f;
    const uint32_t Rm = (opcode >> 3) & 0x7;
    const uint32_t Rd = opcode & 0x7;
    this->registers[Rd] = this->registers[Rm] << imm5;
    // update flags
    // APSR.N = result<31>;
    // APSR.Z = IsZeroBit(result);
    // APSR.C = carry;
    // APSR.V unchanged
  }
  // MOVS
  else if (opcode >> 11 == 0b00100) {
    const uint32_t value = opcode & 0xff;
    const uint32_t Rd = (opcode >> 8) & 7;
    this->registers[Rd] = value;
    // update status flags (if InITBlock)?
    // APSR.N = result<31>;
    // APSR.Z = IsZeroBit(result);
    // APSR.C = carry;
    // APSR.V unchanged
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

  this->setPC(getPC() + 2);
}