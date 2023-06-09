#include "utils/assembler.h"

uint32_t opcodeADCS(uint32_t Rdn, uint32_t Rm) {
  return (0b0100000101 << 6) | ((Rm & 7) << 3) | (Rdn & 7);
}

uint32_t opcodeADDS2(uint32_t Rdn, uint32_t imm8) {
  return (0b00110 << 11) | ((Rdn & 7) << 8) | (imm8 & 0xff);
}

uint32_t opcodeLDRB(uint32_t Rt, uint32_t Rn, uint32_t imm5) {
  return (0b01111 << 11) | ((imm5 & 0x1f) << 6) | ((Rn & 0x7) << 3) |
         (Rt & 0x7);
}

uint32_t opcodeRSBS(uint32_t Rd, uint32_t Rn) {
  return (0b0100001001 << 6) | ((Rn & 0x7) << 3) | (Rd & 0x7);
}

uint32_t opcodeSUBS2(uint32_t Rdn, uint32_t imm8) {
  return (0b00111 << 11) | ((Rdn & 7) << 8) | (imm8 & 0xff);
}

uint32_t opcodeUXTB(uint32_t Rd, uint32_t Rm) {
  return (0b1011001011 << 6) | ((Rm & 7) << 3) | (Rd & 7);
}