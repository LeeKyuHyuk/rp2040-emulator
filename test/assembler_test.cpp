#include "utils/assembler.h"
#include "gtest/gtest.h"

#define R0 0
#define R1 1
#define R2 2
#define R3 3
#define R4 4
#define R5 5
#define R6 6
#define R7 7
#define R8 8
#define IP 12
#define LR 14
#define PC 15

#define PRIMASK 16

// should correctly encode an `adc r3, r0` instruction
TEST(adc, assembler) { EXPECT_EQ(opcodeADCS(R3, R0), 0x4143); }

// should correctly encode an `add sp, #12` instruction
TEST(add_1, assembler) { EXPECT_EQ(opcodeADDsp2(12), 0xb003); }

// should correctly encode an `add r1, ip` instruction
TEST(add_2, assembler) { EXPECT_EQ(opcodeADDreg(R1, IP), 0x4461); }

// should correctly encode an `add r1, sp, #4
TEST(add_3, assembler) { EXPECT_EQ(opcodeADDspPlusImm(R1, 4), 0xa901); }

// should correctly encode an `adds r0, r3, #0` instruction
TEST(adds_1, assembler) { EXPECT_EQ(opcodeADDS1(R0, R3, 0), 0x1c18); }

// should correctly encode an `adds r1, r1, r3` instruction
TEST(adds_2, assembler) { EXPECT_EQ(opcodeADDSreg(R1, R1, R3), 0x18c9); }

// should correctly encode an `adds r1, #1` instruction
TEST(adds_3, assembler) { EXPECT_EQ(opcodeADDS2(R1, 1), 0x3101); }

// should correctly encode an `ands r5, r0` instruction
TEST(adds_4, assembler) { EXPECT_EQ(opcodeANDS(R5, R0), 0x4005); }

// should correctly encode an `adr r4, #52` instruction
TEST(adr, assembler) { EXPECT_EQ(opcodeADR(R4, 52), 0xa40d); }

// should correctly encode an `asrs r3, r2, #31` instruction
TEST(asrs, assembler) { EXPECT_EQ(opcodeASRS(R3, R2, 31), 0x17d3); }

// should correctly encode an `bics r0, r3` instruction
TEST(bics, assembler) { EXPECT_EQ(opcodeBICS(R0, R3), 0x4398); }

// should correctly encode an `bl .-198` instruction
TEST(bl_1, assembler) { EXPECT_EQ(opcodeBL(-198), 0xff9df7ff); }

// 'should correctly encode an `bl .+10` instruction
TEST(bl_2, assembler) { EXPECT_EQ(opcodeBL(10), 0xf805f000); }

// should correctly encode an `bl .-3242` instruction
TEST(bl_3, assembler) { EXPECT_EQ(opcodeBL(-3242), 0xf9abf7ff); }

// should correctly encode an `blx r1` instruction
TEST(blx, assembler) { EXPECT_EQ(opcodeBLX(R1), 0x4788); }

// should correctly encode an `bx lr` instruction
TEST(bx, assembler) { EXPECT_EQ(opcodeBX(LR), 0x4770); }

// should correctly encode an `eors r1, r3` instruction
TEST(eors, assembler) { EXPECT_EQ(opcodeEORS(R1, R3), 0x4059); }

// should correctly encode an `ldmia r0!, {r1, r2}` instruction
TEST(ldmia, assembler) {
  EXPECT_EQ(opcodeLDMIA(R0, (1 << R1) | (1 << R2)), 0xc806);
}

// should correctly encode an `lsls r5, r0` instruction
TEST(lsls, assembler) { EXPECT_EQ(opcodeLSLSreg(R5, R0), 0x4085); }

// should correctly encode an `lsrs r1, r1, #1` instruction
TEST(lsrs_1, assembler) { EXPECT_EQ(opcodeLSRS(R1, R1, 1), 0x0849); }

// should correctly encode an `lsrs r1, r1, #1` instruction
TEST(lsrs_2, assembler) { EXPECT_EQ(opcodeLSRSreg(R0, R4), 0x40e0); }

// should correctly encode an `ldr r3, [r3, r4]` instruction
TEST(ldr, assembler) { EXPECT_EQ(opcodeLDRreg(R3, R3, R4), 0x591b); }

// should correctly encode an `ldrb r0, [r1, #0]` instruction
TEST(ldrb_1, assembler) { EXPECT_EQ(opcodeLDRB(R0, R1, 0), 0x7808); }

// should correctly encode an `ldrb r2, [r5, r4]` instruction
TEST(ldrb_2, assembler) { EXPECT_EQ(opcodeLDRBreg(R2, R5, R4), 0x5d2a); }

// should correctly encode an `ldrh r3, [r7, #0]` instruction
TEST(ldrh_1, assembler) { EXPECT_EQ(opcodeLDRH(R3, R0, 2), 0x8843); }

// should correctly encode an `ldrh r4, [r0, r1]` instruction
TEST(ldrh_2, assembler) { EXPECT_EQ(opcodeLDRHreg(R4, R0, R1), 0x5a44); }

// should correctly encode an `ldrsb r3, [r2, r3]` instruction
TEST(ldrsb, assembler) { EXPECT_EQ(opcodeLDRSB(R3, R2, R3), 0x56d3); }

// should correctly encode an `ldrsh r5, [r3, r5]` instruction
TEST(ldrsh, assembler) { EXPECT_EQ(opcodeLDRSH(R5, R3, R5), 0x5f5d); }

// should correctly encode an `mov r3, r8` instruction
TEST(mov, assembler) { EXPECT_EQ(opcodeMOV(R3, R8), 0x4643); }

// should correctly encode an `mrs r6, PRIMASK` instruction
TEST(mrs, assembler) { EXPECT_EQ(opcodeMRS(R6, PRIMASK), 0x8610f3ef); }

// should correctly encode an `msr PRIMASK, r6` instruction
TEST(msr, assembler) { EXPECT_EQ(opcodeMSR(PRIMASK, R6), 0x8810f386); }

// should correctly encode an `muls r2, r0` instruction
TEST(muls, assembler) { EXPECT_EQ(opcodeMULS(R2, R0), 0x4350); }

// should correctly encode an `mvns r3, r3` instruction
TEST(mvns, assembler) { EXPECT_EQ(opcodeMVNS(R3, R3), 0x43db); }

// should correctly encode an `prrs r3, r0` instruction
TEST(prrs, assembler) { EXPECT_EQ(opcodeORRS(R3, R0), 0x4303); }

// should correctly encode an `pop {r0, r1, pc}` instruction
TEST(pop, assembler) {
  EXPECT_EQ(opcodePOP(true, (1 << R0) | (1 << R1)), 0xbd03);
}

// should correctly encode an `rev r3, r1` instruction
TEST(rev, assembler) { EXPECT_EQ(opcodeREV(R3, R1), 0xba0b); }

// should correctly encode an `rsbs r0, r3` instruction
TEST(rsbs, assembler) { EXPECT_EQ(opcodeRSBS(R0, R3), 0x4258); }

// should correctly encode an `sbcs r0, r3` instruction
TEST(sbcs, assembler) { EXPECT_EQ(opcodeSBCS(R0, R3), 0x4198); }

// should correctly encode an `stmia r2!, {r0}` instruction
TEST(stmia, assembler) { EXPECT_EQ(opcodeSTMIA(R2, 1 << R0), 0xc201); }

// should correctly encode an `str r6, [r4, #20]` instruction
TEST(str_1, assembler) { EXPECT_EQ(opcodeSTR(R6, R4, 20), 0x6166); }

// should correctly encode an `str r2, [r1, r4]` instruction
TEST(str_2, assembler) { EXPECT_EQ(opcodeSTRreg(R2, R1, R4), 0x510a); }

// should correctly encode an `str r1, [sp, #4]` instruction
TEST(str_3, assembler) { EXPECT_EQ(opcodeSTRsp(R1, 4), 0x9101); }

// should correctly encode an `strb r3, [r2, #0]` instruction
TEST(strb_1, assembler) { EXPECT_EQ(opcodeSTRB(R3, R2, 0), 0x7013); }

// should correctly encode an `strb r3, [r2, r5]` instruction
TEST(strb_2, assembler) { EXPECT_EQ(opcodeSTRBreg(R3, R2, R5), 0x5553); }

// should correctly encode an `strh r1, [r3, #4]` instruction
TEST(strh_1, assembler) { EXPECT_EQ(opcodeSTRH(R1, R3, 4), 0x8099); }

//
TEST(strh_2, assembler) { EXPECT_EQ(opcodeSTRHreg(R1, R3, R2), 0x5299); }

// should correctly encode an `sub sp, #12` instruction
TEST(sub, assembler) { EXPECT_EQ(opcodeSUBsp(12), 0xb083); }

// should correctly encode an `subs r3, r0, #1` instruction
TEST(subs_1, assembler) { EXPECT_EQ(opcodeSUBS1(R3, R0, 1), 0x1e43); }

// should correctly encode an `subs r1, r1, r0` instruction
TEST(subs_2, assembler) { EXPECT_EQ(opcodeSUBSreg(R1, R1, R0), 0x1a09); }

// should correctly encode an `subs r3, #13` instruction
TEST(subs_3, assembler) { EXPECT_EQ(opcodeSUBS2(R3, 13), 0x3b0d); }

// should correctly encode an `sxtb r2, r2` instruction
TEST(sxtb, assembler) { EXPECT_EQ(opcodeSXTB(R2, R2), 0xb252); }

// should correctly encode an `uxtb r3, r3` instruction
TEST(uxtb, assembler) { EXPECT_EQ(opcodeUXTB(R3, R3), 0xb2db); }

// should correctly encode `uxth r3, r0
TEST(uxth, assembler) { EXPECT_EQ(opcodeUXTH(R3, R0), 0xb283); }