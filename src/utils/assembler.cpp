#include "utils/assembler.h"

number opcodeADCS(number Rdn, number Rm) {
  return (0b0100000101 << 6) | ((Rm & 7) << 3) | (Rdn & 7);
}

number opcodeADDS1(number Rd, number Rn, number imm3) {
  return (0b0001110 << 9) | ((imm3 & 0x7) << 6) | ((Rn & 7) << 3) | (Rd & 7);
}
number opcodeADDS2(number Rdn, number imm8) {
  return (0b00110 << 11) | ((Rdn & 7) << 8) | (imm8 & 0xff);
}

number opcodeADDspPlusImm(number Rd, number imm8) {
  return (0b10101 << 11) | ((Rd & 7) << 8) | ((imm8 >> 2) & 0xff);
}
number opcodeADDsp2(number imm) {
  return (0b101100000 << 7) | ((imm >> 2) & 0x7f);
}

number opcodeADDSreg(number Rd, number Rn, number Rm) {
  return (0b0001100 << 9) | ((Rm & 0x7) << 6) | ((Rn & 7) << 3) | (Rd & 7);
}

number opcodeADDreg(number Rdn, number Rm) {
  return (0b01000100 << 8) | ((Rdn & 0x8) << 4) | ((Rm & 0xf) << 3) |
         (Rdn & 0x7);
}
number opcodeADR(number Rd, number imm8) {
  return (0b10100 << 11) | ((Rd & 7) << 8) | ((imm8 >> 2) & 0xff);
}
number opcodeANDS(number Rn, number Rm) {
  return (0b0100000000 << 6) | ((Rm & 7) << 3) | (Rn & 0x7);
}
number opcodeASRS(number Rd, number Rm, number imm5) {
  return (0b00010 << 11) | ((imm5 & 0x1f) << 6) | ((Rm & 0x7) << 3) |
         (Rd & 0x7);
}

number opcodeASRSreg(number Rdn, number Rm) {
  return (0b0100000100 << 6) | ((Rm & 0x7) << 3) | ((Rm & 0x7) << 3) |
         (Rdn & 0x7);
}

number opcodeBICS(number Rdn, number Rm) {
  return (0b0100001110 << 6) | ((Rm & 7) << 3) | (Rdn & 7);
}
number opcodeBL(number imm) {
  const number imm11 = (imm >> 1) & 0x7ff;
  const number imm10 = (imm >> 12) & 0x3ff;
  const number s = (int)imm < 0 ? 1 : 0;
  const number j2 = 1 - (((imm >> 22) & 0x1) ^ s);
  const number j1 = 1 - (((imm >> 23) & 0x1) ^ s);
  const number opcode = (0b1101 << 28) | (j1 << 29) | (j2 << 27) |
                        (imm11 << 16) | (0b11110 << 11) | (s << 10) | imm10;
  return (uint32_t)opcode;
}

number opcodeBLX(number Rm) { return (0b010001111 << 7) | (Rm << 3); }

number opcodeBX(number Rm) { return (0b010001110 << 7) | (Rm << 3); }

number opcodeEORS(number Rdn, number Rm) {
  return (0b0100000001 << 6) | ((Rm & 0x7) << 3) | (Rdn & 0x7);
}

number opcodeLDMIA(number Rn, number registers) {
  return (0b11001 << 11) | ((Rn & 0x7) << 8) | (registers & 0xff);
}

number opcodeLDRreg(number Rt, number Rn, number Rm) {
  return (0b0101100 << 9) | ((Rm & 0x7) << 6) | ((Rn & 0x7) << 3) | (Rt & 0x7);
}

number opcodeLDRB(number Rt, number Rn, number imm5) {
  return (0b01111 << 11) | ((imm5 & 0x1f) << 6) | ((Rn & 0x7) << 3) |
         (Rt & 0x7);
}

number opcodeLDRsp(number Rt, number imm8) {
  return (0b10011 << 11) | ((Rt & 7) << 8) | ((imm8 >> 2) & 0xff);
}

number opcodeLDRBreg(number Rt, number Rn, number Rm) {
  return (0b0101110 << 9) | ((Rm & 0x7) << 6) | ((Rn & 0x7) << 3) | (Rt & 0x7);
}

number opcodeLDRH(number Rt, number Rn, number imm5) {
  return (0b10001 << 11) | (((imm5 >> 1) & 0xf) << 6) | ((Rn & 0x7) << 3) |
         (Rt & 0x7);
}

number opcodeLDRHreg(number Rt, number Rn, number Rm) {
  return (0b0101101 << 9) | ((Rm & 0x7) << 6) | ((Rn & 0x7) << 3) | (Rt & 0x7);
}

number opcodeLDRSB(number Rt, number Rn, number Rm) {
  return (0b0101011 << 9) | ((Rm & 0x7) << 6) | ((Rn & 0x7) << 3) | (Rt & 0x7);
}

number opcodeLDRSH(number Rt, number Rn, number Rm) {
  return (0b0101111 << 9) | ((Rm & 0x7) << 6) | ((Rn & 0x7) << 3) | (Rt & 0x7);
}

number opcodeLSLSreg(number Rdn, number Rm) {
  return (0b0100000010 << 6) | ((Rm & 0x7) << 3) | (Rdn & 0x7);
}

number opcodeLSRS(number Rd, number Rm, number imm5) {
  return (0b00001 << 11) | ((imm5 & 0x1f) << 6) | ((Rm & 0x7) << 3) |
         (Rd & 0x7);
}

number opcodeLSRSreg(number Rdn, number Rm) {
  return (0b0100000011 << 6) | ((Rm & 0x7) << 3) | (Rdn & 0x7);
}

number opcodeMOV(number Rd, number Rm) {
  return (0b01000110 << 8) | ((Rd & 0x8 ? 1 : 0) << 7) | (Rm << 3) | (Rd & 0x7);
}

number opcodeMOVS(number Rd, number imm8) {
  return (0b00100 << 11) | ((Rd & 0x7) << 8) | (imm8 & 0xff);
}

number opcodeMRS(number Rd, number specReg) {
  return (uint32_t)((0b1000 << 28) | ((Rd & 0xf) << 24) |
                    ((specReg & 0xff) << 16) | 0b1111001111101111);
}

number opcodeMSR(number specReg, number Rn) {
  return (uint32_t)((0b10001000 << 24) | ((specReg & 0xff) << 16) |
                    (0b111100111000 << 4) | (Rn & 0xf));
}

number opcodeMULS(number Rn, number Rdm) {
  return (0b0100001101 << 6) | ((Rn & 7) << 3) | (Rdm & 7);
}

number opcodeMVNS(number Rd, number Rm) {
  return (0b0100001111 << 6) | ((Rm & 7) << 3) | (Rd & 7);
}

number opcodeORRS(number Rn, number Rm) {
  return (0b0100001100 << 6) | ((Rm & 0x7) << 3) | (Rn & 0x7);
}

number opcodePOP(bool P, number registerList) {
  return (0b1011110 << 9) | ((P ? 1 : 0) << 8) | registerList;
}

number opcodeREV(number Rd, number Rn) {
  return (0b1011101000 << 6) | ((Rn & 0x7) << 3) | (Rd & 0x7);
}

number opcodeRSBS(number Rd, number Rn) {
  return (0b0100001001 << 6) | ((Rn & 0x7) << 3) | (Rd & 0x7);
}

number opcodeSTMIA(number Rn, number registers) {
  return (0b11000 << 11) | ((Rn & 0x7) << 8) | (registers & 0xff);
}

number opcodeSBCS(number Rn, number Rm) {
  return (0b0100000110 << 6) | ((Rm & 0x7) << 3) | (Rn & 0x7);
}

number opcodeSTR(number Rt, number Rm, number imm5) {
  return (0b01100 << 11) | (((imm5 >> 2) & 0x1f) << 6) | ((Rm & 0x7) << 3) |
         (Rt & 0x7);
}

number opcodeSTRsp(number Rt, number imm8) {
  return (0b10010 << 11) | ((Rt & 7) << 8) | ((imm8 >> 2) & 0xff);
}

number opcodeSTRreg(number Rt, number Rn, number Rm) {
  return (0b0101000 << 9) | ((Rm & 0x7) << 6) | ((Rn & 0x7) << 3) | (Rt & 0x7);
}

number opcodeSTRB(number Rt, number Rm, number imm5) {
  return (0b01110 << 11) | ((imm5 & 0x1f) << 6) | ((Rm & 0x7) << 3) |
         (Rt & 0x7);
}

number opcodeSTRBreg(number Rt, number Rn, number Rm) {
  return (0b0101010 << 9) | ((Rm & 0x7) << 6) | ((Rn & 0x7) << 3) | (Rt & 0x7);
}

number opcodeSTRH(number Rt, number Rm, number imm5) {
  return (0b10000 << 11) | (((imm5 >> 1) & 0x1f) << 6) | ((Rm & 0x7) << 3) |
         (Rt & 0x7);
}

number opcodeSTRHreg(number Rt, number Rn, number Rm) {
  return (0b0101001 << 9) | ((Rm & 0x7) << 6) | ((Rn & 0x7) << 3) | (Rt & 0x7);
}

number opcodeSUBS1(number Rd, number Rn, number imm3) {
  return (0b0001111 << 9) | ((imm3 & 0x7) << 6) | ((Rn & 7) << 3) | (Rd & 7);
}
number opcodeSUBS2(number Rdn, number imm8) {
  return (0b00111 << 11) | ((Rdn & 7) << 8) | (imm8 & 0xff);
}

number opcodeSUBSreg(number Rd, number Rn, number Rm) {
  return (0b0001101 << 9) | ((Rm & 0x7) << 6) | ((Rn & 7) << 3) | (Rd & 7);
}

number opcodeSUBsp(number imm) {
  return (0b101100001 << 7) | ((imm >> 2) & 0x7f);
}

number opcodeSVC(number imm8) { return (0b11011111 << 8) | (imm8 & 0xff); }

number opcodeSXTB(number Rd, number Rm) {
  return (0b1011001001 << 6) | ((Rm & 7) << 3) | (Rd & 7);
}

number opcodeUXTB(number Rd, number Rm) {
  return (0b1011001011 << 6) | ((Rm & 7) << 3) | (Rd & 7);
}

number opcodeUXTH(number Rd, number Rm) {
  return (0b1011001010 << 6) | ((Rm & 7) << 3) | (Rd & 7);
}
