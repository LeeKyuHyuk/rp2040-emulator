#ifndef __ASSEMBLER_H__
#define __ASSEMBLER_H__

#include <cstdint>

typedef uint64_t number;

number opcodeADCS(number Rdn, number Rm);

number opcodeADDS1(number Rd, number Rn, number imm3);

number opcodeADDS2(number Rdn, number imm8);

number opcodeADDspPlusImm(number Rd, number imm8);

number opcodeADDsp2(number imm);

number opcodeADDSreg(number Rd, number Rn, number Rm);

number opcodeADDreg(number Rdn, number Rm);

number opcodeADR(number Rd, number imm8);

number opcodeANDS(number Rn, number Rm);

number opcodeASRS(number Rd, number Rm, number imm5);

number opcodeBICS(number Rdn, number Rm);

number opcodeBL(number imm);

number opcodeBLX(number Rm);

number opcodeBX(number Rm);

number opcodeEORS(number Rdn, number Rm);

number opcodeLDMIA(number Rn, number registers);

number opcodeLDRreg(number Rt, number Rn, number Rm);

number opcodeLDRB(number Rt, number Rn, number imm5);

number opcodeLDRsp(number Rt, number imm8);

number opcodeLDRBreg(number Rt, number Rn, number Rm);

number opcodeLDRH(number Rt, number Rn, number imm5);

number opcodeLDRHreg(number Rt, number Rn, number Rm);

number opcodeLDRSB(number Rt, number Rn, number Rm);

number opcodeLDRSH(number Rt, number Rn, number Rm);

number opcodeLSLSreg(number Rdn, number Rm);

number opcodeLSRS(number Rd, number Rm, number imm5);

number opcodeLSRSreg(number Rdn, number Rm);

number opcodeMOV(number Rd, number Rm);

number opcodeMOVS(number Rd, number imm8);

number opcodeMRS(number Rd, number specReg);

number opcodeMSR(number specReg, number Rn);

number opcodeMULS(number Rn, number Rdm);

number opcodeMVNS(number Rd, number Rm);

number opcodeORRS(number Rn, number Rm);

number opcodePOP(bool P, number registerList);

number opcodeREV(number Rd, number Rn);

number opcodeRSBS(number Rd, number Rn);

number opcodeSTMIA(number Rn, number registers);

number opcodeSBCS(number Rn, number Rm);

number opcodeSTR(number Rt, number Rm, number imm5);

number opcodeSTRsp(number Rt, number imm8);

number opcodeSTRreg(number Rt, number Rn, number Rm);

number opcodeSTRB(number Rt, number Rm, number imm5);

number opcodeSTRBreg(number Rt, number Rn, number Rm);

number opcodeSTRH(number Rt, number Rm, number imm5);

number opcodeSTRHreg(number Rt, number Rn, number Rm);

number opcodeSUBS1(number Rd, number Rn, number imm3);

number opcodeSUBS2(number Rdn, number imm8);

number opcodeSUBSreg(number Rd, number Rn, number Rm);

number opcodeSUBsp(number imm);

number opcodeSVC(number imm8);

number opcodeSXTB(number Rd, number Rm);

number opcodeUXTB(number Rd, number Rm);

number opcodeUXTH(number Rd, number Rm);

#endif