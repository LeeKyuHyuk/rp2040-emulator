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
#define LR 14
#define PC 15

// should correctly encode an `adc r3, r0` instruction
TEST(adc, assembler) { EXPECT_EQ(opcodeADCS(R3, R0), 0x4143); }

// should correctly encode an `add sp, #12` instruction
TEST(add, assembler) { EXPECT_EQ(opcodeADDsp2(12), 0xb003); }

// should correctly encode an `adds r0, r3, #0` instruction
TEST(adds_1, assembler) { EXPECT_EQ(opcodeADDS1(R0, R3, 0), 0x1c18); }

// should correctly encode an `adds r1, r1, r3` instruction
TEST(adds_2, assembler) { EXPECT_EQ(opcodeADDSreg1(R1, R1, R3), 0x18c9); }

// should correctly encode an `adds r1, #1` instruction
TEST(adds_3, assembler) { EXPECT_EQ(opcodeADDS2(R1, 1), 0x3101); }

// should correctly encode an `ands r5, r0` instruction
TEST(adds_4, assembler) { EXPECT_EQ(opcodeANDS(R5, R0), 0x4005); }

// should correctly encode an `adr r4, #52` instruction
TEST(adr, assembler) { EXPECT_EQ(opcodeADR(R4, 52), 0xa40d); }

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

// should correctly encode an `ldmia r0!, {r1, r2}` instruction
TEST(ldmia, assembler) {
  EXPECT_EQ(opcodeLDMIA(R0, (1 << R1) | (1 << R2)), 0xc806);
}

// should correctly encode an `lsrs r1, r1, #1` instruction
TEST(lsrs, assembler) { EXPECT_EQ(opcodeLSRS(R1, R1, 1), 0x0849); }

// should correctly encode an `ldrb r0, [r1, #0]` instruction
TEST(ldrb, assembler) { EXPECT_EQ(opcodeLDRB(R0, R1, 0), 0x7808); }

// should correctly encode an `ldrh r3, [r7, #0]` instruction
TEST(ldrh, assembler) { EXPECT_EQ(opcodeLDRH(R3, R0, 2), 0x8843); }

// should correctly encode an `mov r3, r8` instruction
TEST(mov, assembler) { EXPECT_EQ(opcodeMOV(R3, R8), 0x4643); }

// should correctly encode an `prrs r3, r0` instruction
TEST(prrs, assembler) { EXPECT_EQ(opcodeORRS(R3, R0), 0x4303); }

// should correctly encode an `pop {r0, r1, pc}` instruction
TEST(pop, assembler) {
  EXPECT_EQ(opcodePOP(true, (1 << R0) | (1 << R1)), 0xbd03);
}

// should correctly encode an `rsbs r0, r3` instruction
TEST(rsbs, assembler) { EXPECT_EQ(opcodeRSBS(R0, R3), 0x4258); }

// should correctly encode an `sbcs r0, r3` instruction
TEST(sbcs, assembler) { EXPECT_EQ(opcodeSBCS(R0, R3), 0x4198); }

// should correctly encode an `stmia r2!, {r0}` instruction
TEST(stmia, assembler) { EXPECT_EQ(opcodeSTMIA(R2, 1 << R0), 0xc201); }

// should correctly encode an `sub sp, #12` instruction
TEST(sub, assembler) { EXPECT_EQ(opcodeSUBsp(12), 0xb083); }

// should correctly encode an `subs r3, r0, #1` instruction
TEST(subs_1, assembler) { EXPECT_EQ(opcodeSUBS1(R3, R0, 1), 0x1e43); }

// should correctly encode an `subs r1, r1, r0` instruction
TEST(subs_2, assembler) { EXPECT_EQ(opcodeSUBSreg(R1, R1, R0), 0x1a09); }

// should correctly encode an `subs r3, #13` instruction
TEST(subs_3, assembler) { EXPECT_EQ(opcodeSUBS2(R3, 13), 0x3b0d); }

// should correctly encode an `uxtb r3, r3` instruction
TEST(uxtb, assembler) { EXPECT_EQ(opcodeUXTB(R3, R3), 0xb2db); }