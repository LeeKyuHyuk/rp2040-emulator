#include "rp2040.h"
#include "bootrom.h"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <vector>

number RP2040::signExtend8(number value) { return (char)value; }
number RP2040::signExtend16(number value) { return (short)value; }

void RP2040::onBreak(number code) {
  // TODO: raise HardFault exception
  // cerr << "Breakpoint! 0x" << hex << code << endl;
  this->stopped = true;
  breakCount += 1;
}

number RP2040::getBreakCount() { return this->breakCount; }

RP2040::RP2040() {
  this->readHooks.emplace(SIO_START_ADDRESS + SIO_CPUID_OFFSET,
                          [](number address) -> number {
                            // Returns the current CPU core id
                            // (always 0 for now)
                            return 0;
                          });
  this->readHooks.emplace(
      XIP_SSI_BASE + SSI_SR_OFFSET,
      [&](number address) -> number { return SSI_SR_TFE_BITS; });

  dr0 = 0;
  this->writeHooks.emplace(XIP_SSI_BASE + SSI_DR0_OFFSET,
                           [&](number address, number value) -> void {
                             const number CMD_READ_STATUS = 0x05;
                             if (value == CMD_READ_STATUS) {
                               // tell stage2 that we completed a write
                               dr0 = 0;
                             }
                           });
  this->readHooks.emplace(XIP_SSI_BASE + SSI_DR0_OFFSET,
                          [&](number address) -> number { return dr0; });

  this->readHooks.emplace(CLOCKS_BASE + CLK_REF_SELECTED,
                          [&](number address) -> number { return 1; });
  this->readHooks.emplace(CLOCKS_BASE + CLK_SYS_SELECTED,
                          [&](number address) -> number { return 1; });

  VTOR = 0;
  this->writeHooks.emplace(
      PPB_BASE + OFFSET_VTOR,
      [&](number address, number newValue) -> void { VTOR = newValue; });
  this->readHooks.emplace(PPB_BASE + OFFSET_VTOR,
                          [&](number address) -> number { return VTOR; });
  this->writeHooks.emplace(PPB_BASE + OFFSET_NVIC_ISPR,
                           [&](number address, number newValue) -> void {
                             this->pendingInterrupts |= newValue;
                             this->interruptsUpdated = true;
                           });
  this->writeHooks.emplace(PPB_BASE + OFFSET_NVIC_ICPR,
                           [&](number address, number newValue) -> void {
                             this->pendingInterrupts &= ~newValue;
                           });
  this->writeHooks.emplace(PPB_BASE + OFFSET_NVIC_ISER,
                           [&](number address, number newValue) -> void {
                             this->enabledInterrupts |= newValue;
                             this->interruptsUpdated = true;
                           });
  this->writeHooks.emplace(PPB_BASE + OFFSET_NVIC_ICER,
                           [&](number address, number newValue) -> void {
                             this->enabledInterrupts &= ~newValue;
                           });

  /* NVIC */
  this->readHooks.emplace(
      PPB_BASE + OFFSET_NVIC_ISPR,
      [&](number address) -> number { return this->pendingInterrupts; });
  this->readHooks.emplace(
      PPB_BASE + OFFSET_NVIC_ICPR,
      [&](number address) -> number { return this->pendingInterrupts; });
  this->readHooks.emplace(
      PPB_BASE + OFFSET_NVIC_ISER,
      [&](number address) -> number { return this->pendingInterrupts; });
  this->readHooks.emplace(
      PPB_BASE + OFFSET_NVIC_ICER,
      [&](number address) -> number { return this->pendingInterrupts; });
  for (number regIndex = 0; regIndex < 8; regIndex++) {
    this->writeHooks.emplace(
        PPB_BASE + OFFSET_NVIC_IPRn[regIndex],
        [&](number address, number newValue) -> void {
          for (number byteIndex = 0; byteIndex < 4; byteIndex++) {
            const number interruptNumber = regIndex * 4 + byteIndex;
            const number newPriority = (newValue >> (8 * byteIndex + 6)) & 0x3;
            for (number priority = 0; priority < INTERRUPT_PRIORITIES_SIZE;
                 priority++) {
              this->interruptPriorities[priority] &= ~(1 << interruptNumber);
            }
            this->interruptPriorities[newPriority] |= 1 << interruptNumber;
          }
          this->interruptsUpdated = true;
        });
    this->readHooks.emplace(
        PPB_BASE + OFFSET_NVIC_IPRn[regIndex], [&](number address) -> number {
          number result = 0;
          for (number byteIndex = 0; byteIndex < 4; byteIndex++) {
            const number interruptNumber = regIndex * 4 + byteIndex;
            for (number priority = 0; priority < INTERRUPT_PRIORITIES_SIZE;
                 priority++) {
              if (this->interruptPriorities[priority] &
                  (1 << interruptNumber)) {
                result |= priority << (8 * byteIndex + 6);
              }
            }
          }
          return result;
        });
  }
}

void RP2040::loadBootrom(const uint32_t *bootromData, number bootromSize) {
  memcpy(this->bootrom, bootromData, sizeof(uint32_t) * bootromSize);
  this->reset();
}

void RP2040::reset() {
  setSP(bootrom[0]);
  setPC(bootrom[1] & 0xFFFFFFFE);
  memset(this->flash, 0xFFFFFFFF, FLASH_SIZE);
}

number RP2040::getSP() { return this->registers[13]; }

void RP2040::setSP(number value) { this->registers[13] = value; }

number RP2040::getLR() { return this->registers[14]; }

void RP2040::setLR(number value) { this->registers[14] = value; }

number RP2040::getPC() { return this->registers[15]; }

void RP2040::setPC(number value) { this->registers[15] = value; }

number RP2040::getAPSR() {
  return ((this->N ? 0x8000000 : 0) | (this->Z ? 0x4000000 : 0) |
          (this->C ? 0x2000000 : 0) | (this->V ? 0x1000000 : 0));
}

void RP2040::setAPSR(number value) {
  this->N = !!(value & 0x8000000);
  this->Z = !!(value & 0x4000000);
  this->C = !!(value & 0x2000000);
  this->V = !!(value & 0x1000000);
}

number RP2040::getxPSR() { return this->getAPSR() | this->IPSR | (1 << 24); }

void RP2040::setxPSR(number value) {
  this->setAPSR(value);
  this->IPSR = value & 0x3f;
}

Peripheral *RP2040::findPeripheral(number address) {
  map<number, Peripheral *>::iterator iter =
      (this->peripherals).find(((uint32_t)address >> 14) << 2);
  // Operates when there is a value for the
  // corresponding key in the peripherals map
  if (iter != this->peripherals.end()) {
    return iter->second;
  }
  return NULL;
}

bool RP2040::checkCondition(number cond) { // Evaluate base condition.
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

number RP2040::readUint32(number address) {
  if (address & 0x3) {
    cout << endl;
    cout << "[ERROR] read from address 0x" << hex << address
         << ", which is not 32 bit aligned" << endl;
    throw new runtime_error("Read from address is not 32 bit aligned");
  }
  address = (uint32_t)address; // round to 32-bits, unsigned
  Peripheral *peripheral = this->findPeripheral(address);
  if (peripheral != NULL) {
    return peripheral->readUint32(address & 0x3fff);
  }
  if (address < BOOT_ROM_B1_SIZE * 4) {
    return this->bootrom[address / 4];
  } else if (address >= FLASH_START_ADDRESS && address < FLASH_END_ADDRESS) {
    return this->flashView->getUint32(address - FLASH_START_ADDRESS);
  } else if (address >= RAM_START_ADDRESS &&
             address < RAM_START_ADDRESS + SRAM_SIZE) {
    return this->sramView->getUint32(address - RAM_START_ADDRESS);
  } else {
    map<number, function<number(number)>>::iterator iter =
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
number RP2040::readUint16(number address) {
  const number value = this->readUint32(address & 0xfffffffc);
  return address & 0x2 ? (uint32_t)(value & 0xffff0000) >> 16 : value & 0xffff;
}

number RP2040::readUint8(number address) {
  const number value = this->readUint16(address & 0xfffffffe);
  return (uint32_t)(address & 0x1 ? (uint32_t)(value & 0xff00) >> 8
                                  : value & 0xff);
}

void RP2040::writeUint32(number address, number value) {
  Peripheral *peripheral = this->findPeripheral(address);
  if (peripheral != NULL) {
    peripheral->writeUint32(address & 0x3fff, value);
  } else if (address < BOOT_ROM_B1_SIZE * 4) {
    this->bootrom[address / 4] = value;
  } else if (address >= FLASH_START_ADDRESS && address < FLASH_END_ADDRESS) {
    this->flashView->setUint32(address - FLASH_START_ADDRESS, value);
  } else if (address >= RAM_START_ADDRESS &&
             address < RAM_START_ADDRESS + SRAM_SIZE) {
    this->sramView->setUint32(address - RAM_START_ADDRESS, value);
  } else if (address >= SIO_START_ADDRESS &&
             address < SIO_START_ADDRESS + 0x10000000) {
    const number sioAddress = address - SIO_START_ADDRESS;
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
    }
  } else {
    map<number, function<void(number, number)>>::iterator iter =
        (this->writeHooks).find(address);
    // Operates when there is a value for the
    // corresponding key in the writeHooks map
    if (iter != this->writeHooks.end()) {
      return (iter->second)(address, value);
    } else {
      cerr << "Write to undefined address: 0x" << hex << address << endl;
    }
  }
}

void RP2040::writeUint16(number address, number value) {
  // we assume that addess is 16-bit aligned.
  // Ideally we should generate a fault if not!
  const number alignedAddress = address & 0xfffffffc;
  const number offset = address & 0x3;
  Peripheral *peripheral = this->findPeripheral(address);
  if (peripheral) {
    peripheral->writeUint32(alignedAddress & 0x3fff,
                            (value & 0xffff) | ((value & 0xffff) << 16));
    return;
  }
  const number originalValue = this->readUint32(alignedAddress);
  uint32_t newValue[] = {(uint32_t)originalValue};
  DataView(&newValue[0], 1).setUint16(offset, (uint16_t)value);
  this->writeUint32(alignedAddress, newValue[0]);
}

void RP2040::writeUint8(number address, number value) {
  const number alignedAddress = address & 0xfffffffc;
  const number offset = address & 0x3;
  Peripheral *peripheral = this->findPeripheral(address);
  if (peripheral != NULL) {
    peripheral->writeUint32(alignedAddress & 0x3fff,
                            (value & 0xff) | ((value & 0xff) << 8) |
                                ((value & 0xff) << 16) |
                                ((value & 0xff) << 24));
    return;
  }
  const number originalValue = this->readUint32(alignedAddress);
  uint32_t newValue[] = {(uint32_t)originalValue};
  DataView(&newValue[0], 1).setUint8(offset, (uint8_t)value);
  this->writeUint32(alignedAddress, newValue[0]);
}

void RP2040::switchStack(STACK_POINTER_BANK stack) {
  if (this->SPSEL != stack) {
    const number temp = this->getSP();
    this->setSP(this->bankedSP);
    this->bankedSP = temp;
    this->SPSEL = stack;
  }
}

number RP2040::getSPprocess() {
  return this->SPSEL == SP_PROCESS ? this->getSP() : this->bankedSP;
}

void RP2040::setSPprocess(number value) {
  if (this->SPSEL == SP_PROCESS) {
    this->setSP(value);
  } else {
    this->bankedSP = (uint32_t)value;
  }
}

number RP2040::getSPmain() {
  return this->SPSEL == SP_MAIN ? this->getSP() : this->bankedSP;
}

void RP2040::setSPmain(number value) {
  if (this->SPSEL == SP_MAIN) {
    this->setSP(value);
  } else {
    this->bankedSP = (uint32_t)value;
  }
}

void RP2040::exceptionEntry(number exceptionNumber) {
  // PushStack:
  number framePtr = 0;
  number framePtrAlign = 0;
  if (this->SPSEL && this->currentMode == MODE_THREAD) {
    framePtrAlign = this->getSPprocess() & 0b100 ? 1 : 0;
    this->setSPprocess((this->getSPprocess() - 0x20) & ~0b100);
    framePtr = this->getSPprocess();
  } else {
    framePtrAlign = this->getSPmain() & 0b100 ? 1 : 0;
    this->setSPmain((this->getSPmain() - 0x20) & ~0b100);
    framePtr = this->getSPmain();
  }
  /* only the stack locations, not the store order, are architected */
  this->writeUint32(framePtr, this->registers[0]);
  this->writeUint32(framePtr + 0x4, this->registers[1]);
  this->writeUint32(framePtr + 0x8, this->registers[2]);
  this->writeUint32(framePtr + 0xc, this->registers[3]);
  this->writeUint32(framePtr + 0x10, this->registers[12]);
  this->writeUint32(framePtr + 0x14, this->getLR());
  this->writeUint32(framePtr + 0x18,
                    this->getPC()); // ReturnAddress(ExceptionType);
  this->writeUint32(framePtr + 0x1c,
                    (this->getxPSR() & ~(1 << 9)) | (framePtrAlign << 9));
  if (this->currentMode == MODE_HANDLER) {
    this->setLR(0xfffffff1);
  } else {
    if (!this->SPSEL) {
      this->setLR(0xfffffff9);
    } else {
      this->setLR(0xfffffffd);
    }
  }
  // ExceptionTaken:
  this->currentMode = MODE_HANDLER; // Enter Handler Mode, now Privileged
  this->IPSR = exceptionNumber;
  this->switchStack(SP_MAIN);
  // SetEventRegister(); // See WFE instruction for details
  const number vectorTable = this->readUint32(PPB_BASE + OFFSET_VTOR);
  this->setPC(this->readUint32(vectorTable + 4 * exceptionNumber));
}

void RP2040::exceptionReturn(number excReturn) {
  number framePtr = this->getSPmain();
  switch (excReturn & 0xf) {
  case 0b0001: // Return to Handler
    this->currentMode = MODE_HANDLER;
    this->switchStack(SP_MAIN);
    break;
  case 0b1001: // Return to Thread using Main stack
    this->currentMode = MODE_THREAD;
    this->switchStack(SP_MAIN);
    break;
  case 0b1101: // Return to Thread using Process stack
    framePtr = this->getSPprocess();
    this->currentMode = MODE_THREAD;
    this->switchStack(SP_PROCESS);
    break;
    // Assigning CurrentMode to Mode_Thread causes a drop in privilege
    // if CONTROL.nPRIV is set to 1
  }

  // PopStack:
  this->registers[0] = this->readUint32(
      framePtr); // Stack accesses are performed as Unprivileged accesses if
  this->registers[1] = this->readUint32(
      framePtr +
      0x4); // CONTROL<0>=='1' && EXC_RETURN<3>=='1' Privileged otherwise
  this->registers[2] = this->readUint32(framePtr + 0x8);
  this->registers[3] = this->readUint32(framePtr + 0xc);
  this->registers[12] = this->readUint32(framePtr + 0x10);
  this->setLR(this->readUint32(framePtr + 0x14));
  this->setPC(this->readUint32(framePtr + 0x18));
  const number psr = this->readUint32(framePtr + 0x1c);

  const number framePtrAlign = psr & (1 << 9) ? 0b100 : 0;

  switch (excReturn & 0xf) {
  case 0b0001: // Returning to Handler mode
    this->setSPmain((this->getSPmain() + 0x20) | framePtrAlign);
  case 0b1001: // Returning to Thread mode using Main stack
    this->setSPmain((this->getSPmain() + 0x20) | framePtrAlign);
  case 0b1101: // Returning to Thread mode using Process stack
    this->setSPprocess((this->getSPprocess() + 0x20) | framePtrAlign);
  }

  this->setAPSR(psr & 0xf0000000);
  const number forceThread = this->currentMode == MODE_THREAD && this->nPRIV;
  this->IPSR = forceThread ? 0 : psr & 0x3f;
  // Thumb bit should always be one! EPSR<24> = psr<24>; // Load valid EPSR bits
  // from memory SetEventRegister(); // See WFE instruction for more details if
  // CurrentMode == Mode_Thread && SCR.SLEEPONEXIT == '1' then SleepOnExit(); //
  // IMPLEMENTATION DEFINED
}

number RP2040::exceptionPriority(number n) {
  switch (n) {
  case EXC_RESET:
    return -3;
  case EXC_NMI:
    return -2;
  case EXC_HARDFAULT:
    return -1;
  case EXC_SVCALL:
    return this->readUint32(PPB_BASE + OFFSET_SHPR2) >> 30;
  case EXC_PENDSV:
    return (this->readUint32(PPB_BASE + OFFSET_SHPR3) >> 22) & 0x3;
  case EXC_SYSTICK:
    return this->readUint32(PPB_BASE + OFFSET_SHPR3) >> 30;
  default:
    if (n < 16) {
      return LOWEST_PRIORITY;
    }
    const number intNum = n - 16;
    for (number priority = 0; priority < 4; priority++) {
      if (this->interruptPriorities[priority] & (1 << intNum)) {
        return priority;
      }
    }
    return LOWEST_PRIORITY;
  }
}

void RP2040::checkForInterrupts() {
  const number currentPriority =
      min(this->exceptionPriority(this->IPSR), this->PM ? 0 : LOWEST_PRIORITY);
  const number interruptSet = this->pendingInterrupts & this->enabledInterrupts;
  for (number priority = 0; priority < currentPriority; priority++) {
    const number levelInterrupts =
        interruptSet & this->interruptPriorities[priority];
    if (levelInterrupts) {
      for (number interruptNumber = 0; interruptNumber < 32;
           interruptNumber++) {
        if (levelInterrupts & (1 << interruptNumber)) {
          this->exceptionEntry(16 + interruptNumber);
          return;
        }
      }
    }
  }
  this->interruptsUpdated = false;
}

number RP2040::readSpecialRegister(number sysm) {
  switch (sysm) {
  case SYSM_APSR:
    return this->getAPSR();

  case SYSM_IPSR:
    return this->IPSR;

  case SYSM_PRIMASK:
    return this->PM ? 1 : 0;

  case SYSM_MSP:
    return this->getSPmain();

  case SYSM_PSP:
    return this->getSPprocess();

  case SYSM_CONTROL:
    return (this->SPSEL == SP_PROCESS ? 2 : 0) | (this->nPRIV ? 1 : 0);

  default:
    cout << "MRS with unimplemented SYSm value: 0x" << hex << sysm << endl;
    return 0;
  }
}

void RP2040::writeSpecialRegister(number sysm, number value) {
  switch (sysm) {
  case SYSM_APSR:
    this->setAPSR(value);
    break;

  case SYSM_IPSR:
    this->IPSR = value;
    break;

  case SYSM_PRIMASK:
    this->PM = !!(value & 1);

  case SYSM_MSP:
    this->setSPmain(value);
    break;

  case SYSM_PSP:
    this->setSPprocess(value);
    break;

  case SYSM_CONTROL:
    this->nPRIV = !!(value & 1);
    this->switchStack(value & 2 ? SP_PROCESS : SP_MAIN);
    break;

  default:
    cout << "MSR with unimplemented SYSm value: 0x" << hex << sysm << endl;
  }
}

void RP2040::BXWritePC(number address) {
  if (this->currentMode == MODE_HANDLER && (uint32_t)address >> 28 == 0b1111) {
    this->exceptionReturn(address & 0x0fffffff);
  } else {
    this->setPC(address & ~1);
  }
}

void RP2040::executeInstruction() {
  if (this->interruptsUpdated) {
    this->checkForInterrupts();
  }
  // ARM Thumb instruction encoding - 16 bits / 2 bytes
  const number opcode = this->readUint16(this->getPC());
  const number opcode2 = this->readUint16(this->getPC() + 2);
  const number opcodePC = this->getPC();
  this->setPC(this->getPC() + 2);
  // ADCS
  if (opcode >> 6 == 0b0100000101) {
    const number Rm = (opcode >> 3) & 0x7;
    const number Rdn = opcode & 0x7;
    const number leftValue = this->registers[Rdn];
    const number rightValue = this->registers[Rm];
    const number result = leftValue + rightValue + (this->C ? 1 : 0);
    this->registers[Rdn] = result;
    this->N = !!(result & 0x80000000);
    this->Z = (result & 0xffffffff) == 0;
    this->C = result >= 0xffffffff;
    this->V =
        ((int)leftValue >= 0 && (int)rightValue >= 0 && (int)result < 0) ||
        ((int)leftValue <= 0 && (int)rightValue <= 0 && (int)result > 0);
  }
  // ADD (register = SP plus immediate)
  else if (opcode >> 11 == 0b10101) {
    const number imm8 = opcode & 0xff;
    const number Rd = (opcode >> 8) & 0x7;
    this->registers[Rd] = this->getSP() + (imm8 << 2);
  }
  // ADD (SP plus immediate)
  else if (opcode >> 7 == 0b101100000) {
    const number imm32 = (opcode & 0x7f) << 2;
    this->setSP(this->getSP() + imm32);
  }
  // ADDS (Encoding T1)
  else if (opcode >> 9 == 0b0001110) {
    const number imm3 = (opcode >> 6) & 0x7;
    const number Rn = (opcode >> 3) & 0x7;
    const number Rd = opcode & 0x7;
    const number leftValue = this->registers[Rn];
    const number result = leftValue + imm3;
    this->registers[Rd] = result;
    this->N = !!(result & 0x80000000);
    this->Z = (result & 0xffffffff) == 0;
    this->C = result >= 0xffffffff;
    this->V = (int)leftValue > 0 && imm3 < 0x80 && (int)result < 0;
  }
  // ADDS (Encoding T2)
  else if (opcode >> 11 == 0b00110) {
    const number imm8 = opcode & 0xff;
    const number Rdn = (opcode >> 8) & 0x7;
    const number leftValue = this->registers[Rdn];
    const number result = leftValue + imm8;
    this->registers[Rdn] = result;
    this->N = !!(result & 0x80000000);
    this->Z = (result & 0xffffffff) == 0;
    this->C = result >= 0xffffffff;
    this->V = (int)leftValue > 0 && imm8 < 0x80 && (int)result < 0;
  }
  // ADDS (register)
  else if (opcode >> 9 == 0b0001100) {
    const number Rm = (opcode >> 6) & 0x7;
    const number Rn = (opcode >> 3) & 0x7;
    const number Rd = opcode & 0x7;
    const number leftValue = this->registers[Rn];
    const number rightValue = this->registers[Rm];
    const number result = leftValue + rightValue;
    this->registers[Rd] = result;
    this->N = !!(result & 0x80000000);
    this->Z = (result & 0xffffffff) == 0;
    this->C = result >= 0xffffffff;
    this->V = (int)leftValue > 0 && rightValue < 0x80 && (int)result < 0;
  }
  // ADD (register)
  else if (opcode >> 8 == 0b01000100) {
    const number regSP = 13;
    const number regPC = 15;
    const number Rm = (opcode >> 3) & 0xf;
    const number Rdn = ((opcode & 0x80) >> 4) | (opcode & 0x7);
    const number leftValue =
        Rdn == regPC ? this->getPC() + 2 : this->registers[Rdn];
    const number rightValue = this->registers[Rm];
    const number result = leftValue + rightValue;
    this->registers[Rdn] = Rdn == regPC ? result & ~0x1 : result;
    if (Rdn != regSP && Rdn != regPC) {
      this->N = !!(result & 0x80000000);
      this->Z = (result & 0xffffffff) == 0;
      this->C = result >= 0xffffffff;
      this->V = (int)leftValue > 0 && rightValue < 0x80 && (int)result < 0;
    }
  }
  // ADR
  else if (opcode >> 11 == 0b10100) {
    const number imm8 = opcode & 0xff;
    const number Rd = (opcode >> 8) & 0x7;
    this->registers[Rd] = (opcodePC & 0xfffffffc) + 4 + (imm8 << 2);
  }
  // ANDS (Encoding T2)
  else if (opcode >> 6 == 0b0100000000) {
    const number Rm = (opcode >> 3) & 0x7;
    const number Rdn = opcode & 0x7;
    const number result = this->registers[Rdn] & this->registers[Rm];
    this->registers[Rdn] = result;
    this->N = !!(result & 0x80000000);
    this->Z = (result & 0xffffffff) == 0;
  }
  // ASRS (immediate)
  else if (opcode >> 11 == 0b00010) {
    const number imm5 = (opcode >> 6) & 0x1f;
    const number Rm = (opcode >> 3) & 0x7;
    const number Rd = opcode & 0x7;
    const number input = this->registers[Rm];
    const number result = imm5 ? (int)(this->registers[Rm]) >> imm5 : 0;
    this->registers[Rd] = result;
    this->N = !!(result & 0x80000000);
    this->Z = (result & 0xffffffff) == 0;
    this->C = !!(((uint32_t)input >> (imm5 ? imm5 - 1 : 31)) & 0x1);
  }
  // B (with cond)
  else if (opcode >> 12 == 0b1101 && ((opcode >> 9) & 0x7) != 0b111) {
    number imm8 = (opcode & 0xff) << 1;
    const number cond = (opcode >> 8) & 0xf;
    if (imm8 & (1 << 8)) {
      imm8 = ((imm8 & 0x1ff) - 0x200);
    }
    if (this->checkCondition(cond)) {
      this->setPC(this->getPC() + imm8 + 2);
    }
  }
  // B
  else if (opcode >> 11 == 0b11100) {
    number imm11 = (opcode & 0x7ff) << 1;
    if (imm11 & (1 << 11)) {
      imm11 = (imm11 & 0x7ff) - 0x800;
    }
    this->setPC(this->getPC() + imm11 + 2);
  }
  // BICS
  else if (opcode >> 6 == 0b0100001110) {
    number Rm = (opcode >> 3) & 0x7;
    number Rdn = opcode & 0x7;
    const number result = (this->registers[Rdn] &= ~this->registers[Rm]);
    this->N = !!(result & 0x80000000);
    this->Z = result == 0;
  }
  // BKPT
  else if (opcode >> 8 == 0b10111110) {
    const number imm8 = opcode & 0xff;
    this->onBreak(imm8);
  }
  // BL
  else if (opcode >> 11 == 0b11110 && opcode2 >> 14 == 0b11 &&
           ((opcode2 >> 12) & 0x1) == 1) {
    const number imm11 = opcode2 & 0x7ff;
    const number J2 = (opcode2 >> 11) & 0x1;
    const number J1 = (opcode2 >> 13) & 0x1;
    const number imm10 = opcode & 0x3ff;
    const number S = (opcode >> 10) & 0x1;
    const number I1 = 1 - (S ^ J1);
    const number I2 = 1 - (S ^ J2);
    const number imm32 =
        ((S ? 0b11111111 : 0) << 24) |
        ((I1 << 23) | (I2 << 22) | (imm10 << 12) | (imm11 << 1));
    this->setLR(this->getPC() + 2);
    this->setPC(this->getPC() + 2 + imm32);
  }
  // BLX
  else if (opcode >> 7 == 0b010001111 && (opcode & 0x7) == 0) {
    const number Rm = (opcode >> 3) & 0xf;
    this->setLR(this->getPC());
    this->setPC(this->registers[Rm] & ~1);
  }
  // BX
  else if (opcode >> 7 == 0b010001110 && (opcode & 0x7) == 0) {
    const number Rm = (opcode >> 3) & 0xf;
    this->BXWritePC(this->registers[Rm]);
  }
  // CMP immediate
  else if (opcode >> 11 == 0b00101) {
    const number Rn = (opcode >> 8) & 0x7;
    const number imm8 = opcode & 0xff;
    const number value = (int)this->registers[Rn];
    const number result = (int)(value - imm8);
    this->N = value < imm8;
    this->Z = value == imm8;
    this->C = value >= imm8;
    this->V = value < 0 && imm8 > 0 && result > 0;
  }
  // CMP (register)
  else if (opcode >> 6 == 0b0100001010) {
    const number Rm = (opcode >> 3) & 0x7;
    const number Rn = opcode & 0x7;
    const number leftValue = (int)this->registers[Rn];
    const number rightValue = (int)this->registers[Rm];
    const number result = (int)(leftValue - rightValue);
    this->N = leftValue < rightValue;
    this->Z = leftValue == rightValue;
    this->C = leftValue >= rightValue;
    this->V = (leftValue > 0 && rightValue < 0 && result < 0) ||
              (leftValue < 0 && rightValue > 0 && result > 0);
    // CMP (register) encoding T2
  } else if (opcode >> 8 == 0b01000101) {
    const number Rm = (opcode >> 3) & 0xf;
    const number Rn = ((opcode >> 4) & 0x8) | (opcode & 0x7);
    const number leftValue = (int)this->registers[Rn];
    const number rightValue = (int)this->registers[Rm];
    const number result = (int)(leftValue - rightValue);
    this->N = leftValue < rightValue;
    this->Z = leftValue == rightValue;
    this->C = leftValue >= rightValue;
    this->V = (leftValue > 0 && rightValue < 0 && result < 0) ||
              (leftValue < 0 && rightValue > 0 && result > 0);
  }
  // CPSID i
  else if (opcode == 0xb672) {
    this->PM = true;
  }
  // CPSIE i
  else if (opcode == 0xb662) {
    this->PM = false;
  }
  // DMB SY
  else if (opcode == 0xf3bf && opcode2 == 0x8f5f) {
    this->setPC(this->getPC() + 2);
  }
  // EORS
  else if (opcode >> 6 == 0b0100000001) {
    const number Rm = (opcode >> 3) & 0x7;
    const number Rdn = opcode & 0x7;
    const number result = this->registers[Rm] ^ this->registers[Rdn];
    this->registers[Rdn] = result;
    this->N = !!(result & 0x80000000);
    this->Z = result == 0;
  }
  // LDMIA
  else if (opcode >> 11 == 0b11001) {
    const number Rn = (opcode >> 8) & 0x7;
    const number registers = opcode & 0xff;
    number address = this->registers[Rn];
    for (number i = 0; i < 8; i++) {
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
    const number imm5 = ((opcode >> 6) & 0x1f) << 2;
    const number Rn = (opcode >> 3) & 0x7;
    const number Rt = opcode & 0x7;
    const number addr = this->registers[Rn] + imm5;
    this->registers[Rt] = this->readUint32(addr);
  }
  // LDR (sp + immediate)
  else if (opcode >> 11 == 0b10011) {
    const number Rt = (opcode >> 8) & 0x7;
    const number imm8 = opcode & 0xff;
    const number addr = this->getSP() + (imm8 << 2);
    this->registers[Rt] = this->readUint32(addr);
  }
  // LDR (literal)
  else if (opcode >> 11 == 0b01001) {
    const number imm8 = (opcode & 0xff) << 2;
    const number Rt = (opcode >> 8) & 7;
    const number nextPC = this->getPC() + 2;
    const number addr = (nextPC & 0xfffffffc) + imm8;
    this->registers[Rt] = this->readUint32(addr);
  }
  // LDR (register)
  else if (opcode >> 9 == 0b0101100) {
    const number Rm = (opcode >> 6) & 0x7;
    const number Rn = (opcode >> 3) & 0x7;
    const number Rt = opcode & 0x7;
    const number addr = this->registers[Rm] + this->registers[Rn];
    this->registers[Rt] = this->readUint32(addr);
  }
  // LDRB (immediate)
  else if (opcode >> 11 == 0b01111) {
    const number imm5 = (opcode >> 6) & 0x1f;
    const number Rn = (opcode >> 3) & 0x7;
    const number Rt = opcode & 0x7;
    const number addr = this->registers[Rn] + imm5;
    this->registers[Rt] = this->readUint8(addr);
  }
  // LDRB (register)
  else if (opcode >> 9 == 0b0101110) {
    const number Rm = (opcode >> 6) & 0x7;
    const number Rn = (opcode >> 3) & 0x7;
    const number Rt = opcode & 0x7;
    const number addr = this->registers[Rm] + this->registers[Rn];
    this->registers[Rt] = this->readUint8(addr);
  }
  // LDRH (immediate)
  else if (opcode >> 11 == 0b10001) {
    const number imm5 = (opcode >> 6) & 0x1f;
    const number Rn = (opcode >> 3) & 0x7;
    const number Rt = opcode & 0x7;
    const number addr = this->registers[Rn] + (imm5 << 1);
    this->registers[Rt] = this->readUint16(addr);
  }
  // LDRH (register)
  else if (opcode >> 9 == 0b0101101) {
    const number Rm = (opcode >> 6) & 0x7;
    const number Rn = (opcode >> 3) & 0x7;
    const number Rt = opcode & 0x7;
    const number addr = this->registers[Rm] + this->registers[Rn];
    this->registers[Rt] = this->readUint16(addr);
  }
  // LDRSB
  else if (opcode >> 9 == 0b0101011) {
    const number Rm = (opcode >> 6) & 0x7;
    const number Rn = (opcode >> 3) & 0x7;
    const number Rt = opcode & 0x7;
    const number addr = this->registers[Rm] + this->registers[Rn];
    this->registers[Rt] = signExtend8(this->readUint8(addr));
  }
  // LDRSH
  else if (opcode >> 9 == 0b0101111) {
    const number Rm = (opcode >> 6) & 0x7;
    const number Rn = (opcode >> 3) & 0x7;
    const number Rt = opcode & 0x7;
    const number addr = this->registers[Rm] + this->registers[Rn];
    this->registers[Rt] = signExtend16(this->readUint16(addr));
  }
  // LSLS (immediate)
  else if (opcode >> 11 == 0b00000) {
    const number imm5 = (opcode >> 6) & 0x1f;
    const number Rm = (opcode >> 3) & 0x7;
    const number Rd = opcode & 0x7;
    const number input = this->registers[Rm];
    const number result = input << imm5;
    this->registers[Rd] = result;
    this->N = !!(result & 0x80000000);
    this->Z = result == 0;
    this->C = imm5 ? !!(input & (1 << (32 - imm5))) : this->C;
  }
  // LSLS (register)
  else if (opcode >> 6 == 0b0100000010) {
    const number Rm = (opcode >> 3) & 0x7;
    const number Rdn = opcode & 0x7;
    const number input = this->registers[Rdn];
    const number shiftCount = this->registers[Rm] & 0xff;
    const number result = input << shiftCount;
    this->registers[Rdn] = result;
    this->N = !!(result & 0x80000000);
    this->Z = result == 0;
    this->C = shiftCount ? !!(input & (1 << (32 - shiftCount))) : this->C;
  }
  // LSRS (immediate)
  else if (opcode >> 11 == 0b00001) {
    const number imm5 = (opcode >> 6) & 0x1f;
    const number Rm = (opcode >> 3) & 0x7;
    const number Rd = opcode & 0x7;
    const number input = this->registers[Rm];
    const number result = imm5 ? (uint32_t)input >> imm5 : 0;
    this->registers[Rd] = result;
    this->N = !!(result & 0x80000000);
    this->Z = result == 0;
    this->C = !!(((uint32_t)input >> (imm5 ? imm5 - 1 : 31)) & 0x1);
  }
  // LSRS (register)
  else if (opcode >> 6 == 0b0100000011) {
    const number Rm = (opcode >> 3) & 0x7;
    const number Rdn = opcode & 0x7;
    const number shiftAmount = this->registers[Rm] & 0xff;
    const number input = this->registers[Rdn];
    const number result = (uint32_t)input >> shiftAmount;
    this->registers[Rdn] = result;
    this->N = !!(result & 0x80000000);
    this->Z = result == 0;
    this->C = !!(((uint32_t)input >> (shiftAmount - 1)) & 0x1);
  }
  // MOV
  else if (opcode >> 8 == 0b01000110) {
    const number Rm = (opcode >> 3) & 0xf;
    const number Rd = ((opcode >> 4) & 0x8) | (opcode & 0x7);
    this->registers[Rd] =
        Rm == PC_REGISTER ? this->getPC() + 2 : this->registers[Rm];
  }
  // MOVS
  else if (opcode >> 11 == 0b00100) {
    const number value = opcode & 0xff;
    const number Rd = (opcode >> 8) & 7;
    this->registers[Rd] = value;
    this->N = !!(value & 0x80000000);
    this->Z = value == 0;
  }
  // MRS
  else if (opcode == 0b1111001111101111 && opcode2 >> 12 == 0b1000) {
    const number SYSm = opcode2 & 0xff;
    const number Rd = (opcode2 >> 8) & 0xf;
    this->registers[Rd] = this->readSpecialRegister(SYSm);
    this->setPC(this->getPC() + 2);
  }
  // MSR
  else if (opcode >> 4 == 0b111100111000 && opcode2 >> 8 == 0b10001000) {
    const number SYSm = opcode2 & 0xff;
    const number Rn = opcode & 0xf;
    this->writeSpecialRegister(SYSm, this->registers[Rn]);
    this->setPC(this->getPC() + 2);
  }
  // MULS
  else if (opcode >> 6 == 0b0100001101) {
    const number Rn = (opcode >> 3) & 0x7;
    const number Rdm = opcode & 0x7;
    const number result = this->registers[Rn] * this->registers[Rdm];
    this->registers[Rdm] = result;
    this->N = !!(result & 0x80000000);
    this->Z = (result & 0xffffffff) == 0;
  }
  // MVNS
  else if (opcode >> 6 == 0b0100001111) {
    const number Rm = (opcode >> 3) & 7;
    const number Rd = opcode & 7;
    const number result = ~this->registers[Rm];
    this->registers[Rd] = result;
    this->N = !!(result & 0x80000000);
    this->Z = result == 0;
  }
  // ORRS (Encoding T2)
  else if (opcode >> 6 == 0b0100001100) {
    const number Rm = (opcode >> 3) & 0x7;
    const number Rdn = opcode & 0x7;
    const number result = this->registers[Rdn] | this->registers[Rm];
    this->registers[Rdn] = result;
    this->N = !!(result & 0x80000000);
    this->Z = (result & 0xffffffff) == 0;
  }
  // POP
  else if (opcode >> 9 == 0b1011110) {
    const number P = (opcode >> 8) & 1;
    number address = this->getSP();
    for (number i = 0; i <= 7; i++) {
      if (opcode & (1 << i)) {
        this->registers[i] = this->readUint32(address);
        address += 4;
      }
    }
    if (P) {
      this->BXWritePC(this->readUint32(address));
      address += 4;
    }
    this->setSP(address);
  }
  // PUSH
  else if (opcode >> 9 == 0b1011010) {
    number bitCount = 0;
    for (number i = 0; i <= 8; i++) {
      if (opcode & (1 << i)) {
        bitCount++;
      }
    }
    number address = this->getSP() - 4 * bitCount;
    for (number i = 0; i <= 7; i++) {
      if (opcode & (1 << i)) {
        this->writeUint32(address, this->registers[i]);
        address += 4;
      }
    }
    if (opcode & (1 << 8)) {
      this->writeUint32(address, this->registers[14]);
    }
    this->setSP(this->getSP() - (4 * bitCount));
  }
  // REV
  else if (opcode >> 6 == 0b1011101000) {
    number Rm = (opcode >> 3) & 0x7;
    number Rd = opcode & 0x7;
    const number input = this->registers[Rm];
    this->registers[Rd] =
        ((input & 0xff) << 24) | (((input >> 8) & 0xff) << 16) |
        (((input >> 16) & 0xff) << 8) | ((input >> 24) & 0xff);
  }
  // NEGS / RSBS
  else if (opcode >> 6 == 0b0100001001) {
    number Rn = (opcode >> 3) & 0x7;
    number Rd = opcode & 0x7;
    const number value = (int)this->registers[Rn];
    this->registers[Rd] = -value;
    this->N = value > 0;
    this->Z = value == 0;
    this->C = value == 0;
    this->V = value == 0x7fffffff;
  }
  // SBCS (Encoding T2)
  else if (opcode >> 6 == 0b0100000110) {
    number Rm = (opcode >> 3) & 0x7;
    number Rdn = opcode & 0x7;
    const number operand1 = this->registers[Rdn];
    const number operand2 = this->registers[Rm] + (this->C ? 0 : 1);
    const number result = (int)(operand1 - operand2);
    this->registers[Rdn] = result;
    this->N = operand1 < operand2;
    this->Z = operand1 == operand2;
    this->C = operand1 >= operand2;
    this->V = (int)operand1 < 0 && operand2 > 0 && result > 0;
  }
  // SEV
  else if (opcode == 0b1011111101000000) {
    cout << "SEV" << endl;
  }
  // STMIA
  else if (opcode >> 11 == 0b11000) {
    const number Rn = (opcode >> 8) & 0x7;
    const number registers = opcode & 0xff;
    number address = this->registers[Rn];
    for (number i = 0; i < 8; i++) {
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
    const number imm5 = ((opcode >> 6) & 0x1f) << 2;
    const number Rn = (opcode >> 3) & 0x7;
    const number Rt = opcode & 0x7;
    const number address = this->registers[Rn] + imm5;
    this->writeUint32(address, this->registers[Rt]);
  }
  // STR (sp + immediate)
  else if (opcode >> 11 == 0b10010) {
    const number Rt = (opcode >> 8) & 0x7;
    const number imm8 = opcode & 0xff;
    const number address = this->getSP() + (imm8 << 2);
    this->writeUint32(address, this->registers[Rt]);
  }
  // STR (register)
  else if (opcode >> 9 == 0b0101000) {
    const number Rm = (opcode >> 6) & 0x7;
    const number Rn = (opcode >> 3) & 0x7;
    const number Rt = opcode & 0x7;
    const number address = this->registers[Rm] + this->registers[Rn];
    this->writeUint32(address, this->registers[Rt]);
  }
  // STRB (immediate)
  else if (opcode >> 11 == 0b01110) {
    const number imm5 = (opcode >> 6) & 0x1f;
    const number Rn = (opcode >> 3) & 0x7;
    const number Rt = opcode & 0x7;
    const number address = this->registers[Rn] + imm5;
    this->writeUint8(address, this->registers[Rt]);
  }
  // STRB (register)
  else if (opcode >> 9 == 0b0101010) {
    const number Rm = (opcode >> 6) & 0x7;
    const number Rn = (opcode >> 3) & 0x7;
    const number Rt = opcode & 0x7;
    const number addres = this->registers[Rm] + this->registers[Rn];
    this->writeUint8(addres, this->registers[Rt]);
  }
  // STRH (immediate)
  else if (opcode >> 11 == 0b10000) {
    const number imm5 = ((opcode >> 6) & 0x1f) << 1;
    const number Rn = (opcode >> 3) & 0x7;
    const number Rt = opcode & 0x7;
    const number address = this->registers[Rn] + imm5;
    this->writeUint16(address, this->registers[Rt]);
  }
  // STRH (register)
  else if (opcode >> 9 == 0b0101001) {
    const number Rm = (opcode >> 6) & 0x7;
    const number Rn = (opcode >> 3) & 0x7;
    const number Rt = opcode & 0x7;
    const number addres = this->registers[Rm] + this->registers[Rn];
    this->writeUint16(addres, this->registers[Rt]);
  }
  // SUB (SP minus immediate)
  else if (opcode >> 7 == 0b101100001) {
    const number imm32 = (opcode & 0x7f) << 2;
    this->setSP(this->getSP() - imm32);
  }
  // SUBS (Encoding T1)
  else if (opcode >> 9 == 0b0001111) {
    const number imm3 = (opcode >> 6) & 0x7;
    const number Rn = (opcode >> 3) & 0x7;
    const number Rd = opcode & 0x7;
    const number value = this->registers[Rn];
    const number result = (int)(value - imm3);
    this->registers[Rd] = result;
    this->N = value < imm3;
    this->Z = value == imm3;
    this->C = value >= imm3;
    this->V = (int)value < 0 && imm3 > 0 && result > 0;
  }
  // SUBS (Encoding T2)
  else if (opcode >> 11 == 0b00111) {
    const number imm8 = opcode & 0xff;
    const number Rdn = (opcode >> 8) & 0x7;
    const number value = this->registers[Rdn];
    const number result = (int)(value - imm8);
    this->registers[Rdn] = result;
    this->N = value < imm8;
    this->Z = value == imm8;
    this->C = value >= imm8;
    this->V = (int)value < 0 && imm8 > 0 && result > 0;
  }
  // SUBS (register)
  else if (opcode >> 9 == 0b0001101) {
    const number Rm = (opcode >> 6) & 0x7;
    const number Rn = (opcode >> 3) & 0x7;
    const number Rd = opcode & 0x7;
    const number leftValue = this->registers[Rn];
    const number rightValue = this->registers[Rm];
    const number result = (int)(leftValue - rightValue);
    this->registers[Rd] = result;
    this->N = leftValue < rightValue;
    this->Z = leftValue == rightValue;
    this->C = leftValue >= rightValue;
    this->V = (int)leftValue < 0 && rightValue > 0 && result > 0;
  }
  // SXTB
  else if (opcode >> 6 == 0b1011001001) {
    const number Rm = (opcode >> 3) & 0x7;
    const number Rd = opcode & 0x7;
    this->registers[Rd] = signExtend8(this->registers[Rm]);
  }
  // TST
  else if (opcode >> 6 == 0b0100001000) {
    const number Rm = (opcode >> 3) & 0x7;
    const number Rn = opcode & 0x7;
    const number result = this->registers[Rn] & this->registers[Rm];
    this->N = !!(result & 0x80000000);
    this->Z = result == 0;
  }
  // UDF
  else if (opcode >> 8 == 0b11011110) {
    const number imm8 = opcode & 0xff;
    this->onBreak(imm8);
  }
  // UXTB
  else if (opcode >> 6 == 0b1011001011) {
    const number Rm = (opcode >> 3) & 0x7;
    const number Rd = opcode & 0x7;
    this->registers[Rd] = this->registers[Rm] & 0xff;
  }
  // UXTH
  else if (opcode >> 6 == 0b1011001010) {
    const number Rm = (opcode >> 3) & 0x7;
    const number Rd = opcode & 0x7;
    this->registers[Rd] = this->registers[Rm] & 0xffff;
  }
  // WFE
  else if (opcode == 0b1011111100100000) {
    // do nothing for now. Wait for event!
  } else {
    cout << "Warning: Instruction at 0x" << hex << opcodePC
         << " is not implemented yet!" << endl;
    cout << "Opcode: 0x" << hex << opcode << " (0x" << hex << opcode2 << ")"
         << endl;
  }
}

void RP2040::execute() {
  this->stopped = false;
  while (!this->stopped) {
    this->executeInstruction();
  }
}

void RP2040::stop() { this->stopped = true; }
