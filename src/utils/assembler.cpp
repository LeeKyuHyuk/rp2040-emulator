#include "utils/assembler.h"

uint32_t opcodeADCS(uint32_t Rdn, uint32_t Rm) {
  return (0b0100000101 << 6) | ((Rm & 7) << 3) | (Rdn & 7);
}

uint32_t opcodeADDS2(uint32_t Rdn, uint32_t imm8) {
  return (0b00110 << 11) | ((Rdn & 7) << 8) | (imm8 & 0xff);
}

uint32_t opcodeBL(uint32_t imm) {
  const uint32_t imm11 = (imm >> 1) & 0x7ff;
  const uint32_t imm10 = (imm >> 12) & 0x3ff;
  const uint32_t s = (int)imm < 0 ? 1 : 0;
  const uint32_t j2 = 1 - (((imm >> 22) & 0x1) ^ s);
  const uint32_t j1 = 1 - (((imm >> 23) & 0x1) ^ s);
  const uint32_t opcode = (0b1101 << 28) | (j1 << 29) | (j2 << 27) |
                          (imm11 << 16) | (0b11110 << 11) | (s << 10) | imm10;
  return opcode >> 0;
}

uint32_t opcodeLDMIA(uint32_t Rn, uint32_t registers) {
  return (0b11001 << 11) | ((Rn & 0x7) << 8) | (registers & 0xff);
}

uint32_t opcodeLDRB(uint32_t Rt, uint32_t Rn, uint32_t imm5) {
  return (0b01111 << 11) | ((imm5 & 0x1f) << 6) | ((Rn & 0x7) << 3) |
         (Rt & 0x7);
}

uint32_t opcodeLSRS(uint32_t Rd, uint32_t Rm, uint32_t imm5) {
  return (0b00001 << 11) | ((imm5 & 0x1f) << 6) | ((Rm & 0x7) << 3) |
         (Rd & 0x7);
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