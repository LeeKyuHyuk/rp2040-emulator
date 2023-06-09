#include "rp2040.h"
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

// should execute a `push {r4, r5, r6, lr}` instruction
TEST(execute_push_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x0);
  rp2040->setSP(RAM_START_ADDRESS + 0x100);
  rp2040->flash16[0] = 0xB570; // push	{r4, r5, r6, lr}
  EXPECT_EQ(rp2040->flash[0], 0x70);
  EXPECT_EQ(rp2040->flash[1], 0xB5);
  rp2040->registers[R4] = 0x40;
  rp2040->registers[R5] = 0x50;
  rp2040->registers[R6] = 0x60;
  rp2040->setLR(0x42);
  rp2040->executeInstruction();
  // assert that the values of r4, r5, r6, lr were pushed into the stack
  EXPECT_EQ(rp2040->getSP(), RAM_START_ADDRESS + 0xF0);
  EXPECT_EQ(rp2040->sram[0xF0], 0x40);
  EXPECT_EQ(rp2040->sram[0xF4], 0x50);
  EXPECT_EQ(rp2040->sram[0xF8], 0x60);
  EXPECT_EQ(rp2040->sram[0xFC], 0x42);
}

// should execute a `movs r5, #128` instruction
TEST(execute_mov_instruction_1, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x0);
  rp2040->flash16[0] = 0x2580; // movs r5, #128
  EXPECT_EQ(rp2040->flash[0], 0x80);
  EXPECT_EQ(rp2040->flash[1], 0x25);
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R5], 128);
  EXPECT_EQ(rp2040->getPC(), 0x2);
}

// should execute a `movs r6, r5` instruction
TEST(execute_mov_instruction_2, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x0);
  rp2040->flash16[0] = 0x002E; // movs r6, r5
  rp2040->registers[R5] = 0x50;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R6], 0x50);
}

// should execute an `ldr r0, [pc, #148]` instruction
TEST(execute_ldr_instruction_1, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x0);
  rp2040->flash16[0] = 0x4825; // ldr r0, [pc, #148]
  rp2040->flash[152] = 0x42;
  memset(&rp2040->flash[153], 0x00, sizeof(uint32_t));
  rp2040->executeInstruction();
  rp2040->registers[R5] = 0x50;
  EXPECT_EQ(rp2040->registers[R0], 0x42);
  EXPECT_EQ(rp2040->getPC(), 0x2);
}

// should execute an `ldr r3, [r2, #24]` instruction
TEST(execute_ldr_instruction_2, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x0);
  rp2040->flash16[0] = 0x6993; // ldr r3, [r2, #24]
  rp2040->registers[R2] = 0x20000000;
  rp2040->sram[24] = 0x55;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R3], 0x55);
}

// should execute an `ldrb r4, [r2, 5]` instruction
TEST(execute_ldrb_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x0);
  rp2040->flash16[0] = opcodeLDRB(R4, R2, 5);
  rp2040->registers[R2] = 0x20000000;
  rp2040->sram[5] = 0x66;
  rp2040->sram[6] = 0x77;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R4], 0x66);
}

// should execute an `ldrsh r5, [r3, r5]` instruction
TEST(execute_ldrsh_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x0);
  rp2040->flash16[0] = 0x5F5D; // ldrsh r5, [r3, r5]
  rp2040->registers[R3] = 0x20000000;
  rp2040->registers[R5] = 0x6;
  rp2040->sram[6] = 0x55;
  rp2040->sram[7] = 0xF0;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R5], 0x80007055);
}

// should execute a `lsls r5, r5, #18` instruction
TEST(execute_lsls_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x0);
  rp2040->flash16[0] = 0x04AD; // lsls r5, r5, #18
  rp2040->registers[R5] = 0b00000000000000000011;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R5], 0b11000000000000000000);
  EXPECT_EQ(rp2040->getPC(), 0x2);
}

// should execute a `lsls r5, r5, #18` instruction with carry
TEST(execute_lsls_instruction_with_carry, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x0);
  rp2040->flash16[0] = 0x04AD; // lsls r5, r5, #18
  rp2040->registers[R5] = 0x00004001;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R5], 0x40000);
  EXPECT_EQ(rp2040->C, true);
}

// should execute a `rsbs r0, r3` instruction
TEST(execute_rsbs_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x0);
  rp2040->flash16[0] = opcodeRSBS(R0, R3);
  rp2040->registers[R3] = 100;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R0] | 0, -100);
  EXPECT_EQ(rp2040->N, true);
  EXPECT_EQ(rp2040->Z, false);
  EXPECT_EQ(rp2040->C, false);
  EXPECT_EQ(rp2040->V, false);
}

// should execute a `str r6, [r4, #20]` instruction
TEST(execute_str_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x0);
  rp2040->flash16[0] = 0x6166; // str r6, [r4, #20]
  rp2040->registers[R4] = RAM_START_ADDRESS + 0x20;
  rp2040->registers[R6] = 0xF00D;
  rp2040->executeInstruction();
  uint32_t value;
  memcpy(&value, &(rp2040->sram[0x20 + 20]), sizeof(uint32_t));
  EXPECT_EQ(value, 0xF00D);
  EXPECT_EQ(rp2040->getPC(), 0x2);
}

// should execute a `b.n .-20` instruction
TEST(execute_bn_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x9 * 0x2);
  rp2040->flash16[9] = 0xe7f6; // b.n .-20
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->getPC(), 0x2);
}

// should execute a `bne.n .-6` instruction
TEST(execute_bnen_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x9 * 0x2);
  rp2040->Z = false;
  rp2040->flash16[9] = 0xD1FC; // bne.n .-6
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->getPC(), 0xE);
}

// should execute an `cmp r5, #66` instruction
TEST(execute_cmp_instruction_1, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x0);
  rp2040->flash16[0] = 0x2D42; // cmp r5, #66
  rp2040->registers[R5] = 60;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->N, true);
  EXPECT_EQ(rp2040->Z, false);
  EXPECT_EQ(rp2040->C, false);
  EXPECT_EQ(rp2040->V, false);
}

// should execute an `cmp r5, r0` instruction
TEST(execute_cmp_instruction_2, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x0);
  rp2040->flash16[0] = 0x4285; // cmp r5, r0
  rp2040->registers[R5] = 60;
  rp2040->registers[R0] = 56;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->N, false);
  EXPECT_EQ(rp2040->Z, false);
  EXPECT_EQ(rp2040->C, true);
  EXPECT_EQ(rp2040->V, false);
}

// should execute a `subs r5, #10` instruction
TEST(execute_subs_instruction_1, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x0);
  rp2040->flash16[0] = opcodeSUBS2(R5, 10);
  rp2040->registers[R5] = 100;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R5], 90);
  EXPECT_EQ(rp2040->N, false);
  EXPECT_EQ(rp2040->Z, false);
  EXPECT_EQ(rp2040->C, true);
  EXPECT_EQ(rp2040->V, false);
}

// should execute a `subs r1, #1` instruction with overflow
TEST(execute_subs_instruction_2, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x0);
  rp2040->flash16[0] = opcodeSUBS2(R1, 1);
  rp2040->registers[R1] = -0x80000000;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R1], 0x7fffffff);
  EXPECT_EQ(rp2040->N, false);
  EXPECT_EQ(rp2040->Z, false);
  EXPECT_EQ(rp2040->C, true);
  EXPECT_EQ(rp2040->V, true);
}

// should execute an `tst r1, r3` instruction when the result is negative
TEST(execute_tst_instruction_1, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x0);
  rp2040->flash16[0] = 0x4219; // tst r1, r3
  rp2040->registers[R1] = 0xF0000000;
  rp2040->registers[R3] = 0xF0004000;
  rp2040->sram[24] = 0x55;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->N, true);
}

// should execute an `tst r1, r3` instruction the registers are equal
TEST(execute_tst_instruction_2, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x0);
  rp2040->flash16[0] = 0x4219; // tst r1, r3
  rp2040->registers[R1] = 0x00;
  rp2040->registers[R3] = 0x37;
  rp2040->sram[24] = 0x55;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->Z, true);
}

// should execute `adcs r5, r4` instruction
TEST(execute_adcs_instruction_1, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x0);
  rp2040->flash16[0] = opcodeADCS(R5, R4);
  rp2040->registers[R4] = 55;
  rp2040->registers[R5] = 66;
  rp2040->C = true;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R5], 122);
  EXPECT_EQ(rp2040->N, false);
  EXPECT_EQ(rp2040->Z, false);
  EXPECT_EQ(rp2040->C, false);
  EXPECT_EQ(rp2040->V, false);
}

// should execute `adcs r5, r4` instruction
TEST(execute_adcs_instruction_2, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x0);
  rp2040->flash16[0] = opcodeADCS(R5, R4);
  rp2040->registers[R4] = 0x7FFFFFFF; // Max signed INT32
  rp2040->registers[R5] = 0;
  rp2040->C = true;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R5], 0x80000000);
  EXPECT_EQ(rp2040->N, true);
  EXPECT_EQ(rp2040->Z, false);
  EXPECT_EQ(rp2040->C, false);
  EXPECT_EQ(rp2040->V, true);
}

// should execute `adds r1, #1` instruction
TEST(execute_adds_instruction_1, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x0);
  rp2040->flash16[0] = opcodeADDS2(R1, 1);
  rp2040->registers[R1] = 0xFFFFFFFF;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R1], 0);
  EXPECT_EQ(rp2040->N, false);
  EXPECT_EQ(rp2040->Z, true);
  EXPECT_EQ(rp2040->C, true);
  EXPECT_EQ(rp2040->V, false);
}

// should execute `adds r1, #1` instruction and set the overflow flag correctly
TEST(execute_adds_instruction_2, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x0);
  rp2040->flash16[0] = opcodeADDS2(R1, 1);
  rp2040->registers[R1] = 0x7FFFFFFF;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R1], 0x80000000);
  EXPECT_EQ(rp2040->N, true);
  EXPECT_EQ(rp2040->Z, false);
  EXPECT_EQ(rp2040->C, false);
  EXPECT_EQ(rp2040->V, true);
}

// should execute an `uxtb	r5, r3` instruction the registers are equal
TEST(execute_uxtb_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x0);
  rp2040->flash16[0] = opcodeUXTB(R5, R3);
  rp2040->registers[R3] = 0x12345678;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R5], 0x78);
}