#ifndef __ASSEMBLER_H__
#define __ASSEMBLER_H__

#include <cstdint>

uint32_t opcodeADCS(uint32_t Rdn, uint32_t Rm);

uint32_t opcodeADDS2(uint32_t Rdn, uint32_t imm8);

uint32_t opcodeLDRB(uint32_t Rt, uint32_t Rn, uint32_t imm5);

uint32_t opcodeRSBS(uint32_t Rd, uint32_t Rn);

uint32_t opcodeSUBS2(uint32_t Rdn, uint32_t imm8);

uint32_t opcodeUXTB(uint32_t Rd, uint32_t Rm);

#endif