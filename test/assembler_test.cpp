#include "utils/assembler.h"
#include "gtest/gtest.h"

#define R0 0
#define R1 1
#define R3 3

// should correctly encode an `adc r3, r0` instruction
TEST(adc, assembler) { EXPECT_EQ(opcodeADCS(R3, R0), 0x4143); }

// should correctly encode an `adds r1, #1` instruction
TEST(adds, assembler) { EXPECT_EQ(opcodeADDS2(R1, 1), 0x3101); }

// should correctly encode an `ldrb r0, [r1, #0]` instruction
TEST(ldrb, assembler) { EXPECT_EQ(opcodeLDRB(R0, R1, 0), 0x7808); }

// should correctly encode an `rsbs r0, r3` instruction
TEST(rsbs, assembler) { EXPECT_EQ(opcodeRSBS(R0, R3), 0x4258); }

// should correctly encode an `subs r3, #13` instruction
TEST(subs, assembler) { EXPECT_EQ(opcodeSUBS2(R3, 13), 0x3b0d); }

// should correctly encode an `uxtb r3, r3` instruction
TEST(uxtb, assembler) { EXPECT_EQ(opcodeUXTB(R3, R3), 0xb2db); }