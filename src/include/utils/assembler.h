#ifndef __ASSEMBLER_H__
#define __ASSEMBLER_H__

#include <cstdint>

uint32_t opcodeADCS(uint32_t Rdn, uint32_t Rm);

uint32_t opcodeADDS1(uint32_t Rd, uint32_t Rn, uint32_t imm3);

uint32_t opcodeADDS2(uint32_t Rdn, uint32_t imm8);

uint32_t opcodeADDsp2(uint32_t imm);

uint32_t opcodeADDSreg(uint32_t Rd, uint32_t Rn, uint32_t Rm);

uint32_t opcodeADDreg(uint32_t Rdn, uint32_t Rm);

uint32_t opcodeADR(uint32_t Rd, uint32_t imm8);

uint32_t opcodeANDS(uint32_t Rn, uint32_t Rm);

uint32_t opcodeBICS(uint32_t Rdn, uint32_t Rm);

uint32_t opcodeBL(uint32_t imm);

uint32_t opcodeBLX(uint32_t Rm);

uint32_t opcodeBX(uint32_t Rm);

uint32_t opcodeLDMIA(uint32_t Rn, uint32_t registers);

uint32_t opcodeLDRreg(uint32_t Rt, uint32_t Rn, uint32_t Rm);

uint32_t opcodeLDRB(uint32_t Rt, uint32_t Rn, uint32_t imm5);

uint32_t opcodeLDRBreg(uint32_t Rt, uint32_t Rn, uint32_t Rm);

uint32_t opcodeLDRH(uint32_t Rt, uint32_t Rn, uint32_t imm5);

uint32_t opcodeLDRHreg(uint32_t Rt, uint32_t Rn, uint32_t Rm);

uint32_t opcodeLDRSB(uint32_t Rt, uint32_t Rn, uint32_t Rm);

uint32_t opcodeLDRSH(uint32_t Rt, uint32_t Rn, uint32_t Rm);

uint32_t opcodeLSLSreg(uint32_t Rdn, uint32_t Rm);

uint32_t opcodeLSRS(uint32_t Rd, uint32_t Rm, uint32_t imm5);

uint32_t opcodeMOV(uint32_t Rd, uint32_t Rm);

uint32_t opcodeMRS(uint32_t Rd, uint32_t specReg);

uint32_t opcodeMSR(uint32_t specReg, uint32_t Rn);

uint32_t opcodeORRS(uint32_t Rn, uint32_t Rm);

uint32_t opcodePOP(bool P, uint32_t registerList);

uint32_t opcodeRSBS(uint32_t Rd, uint32_t Rn);

uint32_t opcodeSTMIA(uint32_t Rn, uint32_t registers);

uint32_t opcodeSBCS(uint32_t Rn, uint32_t Rm);

uint32_t opcodeSTR(uint32_t Rt, uint32_t Rm, uint32_t imm5);

uint32_t opcodeSTRreg(uint32_t Rt, uint32_t Rn, uint32_t Rm);

uint32_t opcodeSTRB(uint32_t Rt, uint32_t Rm, uint32_t imm5);

uint32_t opcodeSTRBreg(uint32_t Rt, uint32_t Rn, uint32_t Rm);

uint32_t opcodeSTRH(uint32_t Rt, uint32_t Rm, uint32_t imm5);

uint32_t opcodeSTRHreg(uint32_t Rt, uint32_t Rn, uint32_t Rm);

uint32_t opcodeSUBS1(uint32_t Rd, uint32_t Rn, uint32_t imm3);

uint32_t opcodeSUBS2(uint32_t Rdn, uint32_t imm8);

uint32_t opcodeSUBSreg(uint32_t Rd, uint32_t Rn, uint32_t Rm);

uint32_t opcodeSUBsp(uint32_t imm);

uint32_t opcodeUXTB(uint32_t Rd, uint32_t Rm);

#endif