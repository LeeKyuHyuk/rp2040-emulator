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
#define R8 8
#define LR 14
#define PC 15

// should initialize PC and SP according to bootrom's vector table
TEST(boot_rom_initialize, RP2040) {
  RP2040 *rp2040 = new RP2040("");
  EXPECT_EQ(rp2040->getSP(), 0x20041f00);
  EXPECT_EQ(rp2040->getPC(), 0xEE);
}

// should execute a `pop pc, {r4, r5, r6}` instruction
TEST(execute_pop_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->setSP(RAM_START_ADDRESS + 0xf0);
  rp2040->flash16[0] = opcodePOP(true, (1 << R4) | (1 << R5) | (1 << R6));
  rp2040->sram[0xf0] = 0x40;
  rp2040->sram[0xf4] = 0x50;
  rp2040->sram[0xf8] = 0x60;
  rp2040->sram[0xfc] = 0x42;
  rp2040->executeInstruction();
  // assert that the values of r4, r5, r6, lr were pushed into the stack
  EXPECT_EQ(rp2040->registers[R4], 0x40);
  EXPECT_EQ(rp2040->registers[R5], 0x50);
  EXPECT_EQ(rp2040->registers[R6], 0x60);
  EXPECT_EQ(rp2040->getPC(), 0x42);
}

// should execute a `push {r4, r5, r6, lr}` instruction
TEST(execute_push_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->setSP(RAM_START_ADDRESS + 0x100);
  rp2040->flash16[0] = 0xB570; // push {r4, r5, r6, lr}
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
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = 0x2580; // movs r5, #128
  EXPECT_EQ(rp2040->flash[0], 0x80);
  EXPECT_EQ(rp2040->flash[1], 0x25);
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R5], 128);
  EXPECT_EQ(rp2040->getPC(), 0x10000002);
}

// should execute a `lsrs r1, r1, #1` instruction
TEST(execute_lsrs_instruction_1, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeLSRS(R1, R1, 1);
  rp2040->registers[R1] = 0b10;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R1], 0b1);
  EXPECT_EQ(rp2040->getPC(), 0x10000002);
  EXPECT_EQ(rp2040->C, false);
}

// should execute a `lsrs r1, r1, 0` instruction
TEST(execute_lsrs_instruction_2, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeLSRS(R1, R1, 0);
  rp2040->registers[R1] = 0xffffffff;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R1], 0);
  EXPECT_EQ(rp2040->getPC(), 0x10000002);
  EXPECT_EQ(rp2040->C, true);
}

// should execute a `movs r6, r5` instruction
TEST(execute_mov_instruction_2, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = 0x002E; // movs r6, r5
  rp2040->registers[R5] = 0x50;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R6], 0x50);
}

// should execute a `mov r3, r8` instruction
TEST(execute_mov_instruction_3, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeMOV(R3, R8);
  rp2040->registers[R8] = 55;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R3], 55);
}

// should execute a `mov r3, pc` instruction
TEST(execute_mov_instruction_4, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeMOV(R3, PC);
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R3], 0x10000004);
}

// should execute `orrs r5, r0` instruction
TEST(execute_orrs_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeORRS(R5, R0);
  rp2040->registers[R5] = 0xf00f0000;
  rp2040->registers[R0] = 0xf000ffff;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R5], 0xf00fffff);
  EXPECT_EQ(rp2040->N, true);
  EXPECT_EQ(rp2040->Z, false);
}

// should execute a `ldmia r0!, {r1, r2}` instruction
TEST(execute_ldmia_instruction_1, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeLDMIA(R0, (1 << R1) | (1 << R2));
  rp2040->registers[R0] = 0x20000000;
  uint32_t *sram32 = (uint32_t *)rp2040->sram;
  sram32[0] = 0xf00df00d;
  sram32[1] = 0x4242;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->getPC(), 0x10000002);
  EXPECT_EQ(rp2040->registers[R0], 0x20000008);
  EXPECT_EQ(rp2040->registers[R1], 0xf00df00d);
  EXPECT_EQ(rp2040->registers[R2], 0x4242);
}

// should execute a `ldmia r5!, {r5}` instruction
// without writing back the address to r5
TEST(execute_ldmia_instruction_2, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeLDMIA(R5, 1 << R5);
  rp2040->registers[R5] = 0x20000000;
  uint32_t *sram32 = (uint32_t *)rp2040->sram;
  sram32[0] = 0xf00df00d;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->getPC(), 0x10000002);
  EXPECT_EQ(rp2040->registers[R5], 0xf00df00d);
}

// should execute an `ldr r0, [pc, #148]` instruction
TEST(execute_ldr_instruction_1, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = 0x4825; // ldr r0, [pc, #148]
  rp2040->flash[152] = 0x42;
  memset(&rp2040->flash[153], 0x00, sizeof(uint32_t));
  rp2040->executeInstruction();
  rp2040->registers[R5] = 0x50;
  EXPECT_EQ(rp2040->registers[R0], 0x42);
  EXPECT_EQ(rp2040->getPC(), 0x10000002);
}

// should execute an `ldr r3, [r2, #24]` instruction
TEST(execute_ldr_instruction_2, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = 0x6993; // ldr r3, [r2, #24]
  rp2040->registers[R2] = 0x20000000;
  rp2040->sram[24] = 0x55;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R3], 0x55);
}

// should execute an `ldrb r4, [r2, 5]` instruction
TEST(execute_ldrb_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeLDRB(R4, R2, 5);
  rp2040->registers[R2] = 0x20000000;
  rp2040->sram[5] = 0x66;
  rp2040->sram[6] = 0x77;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R4], 0x66);
}

// should execute an `ldrh r3, [r7, #4]` instruction
TEST(execute_ldrh_instruction_1, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeLDRH(R3, R7, 4);
  rp2040->registers[R7] = 0x20000000;
  rp2040->sram[4] = 0x66;
  rp2040->sram[5] = 0x77;
  rp2040->sram[6] = 0xff;
  rp2040->sram[7] = 0xff;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R3], 0x7766);
}

// should execute an `ldrh r3, [r7, #6]` instruction
TEST(execute_ldrh_instruction_2, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeLDRH(R3, R7, 6);
  rp2040->registers[R7] = 0x20000000;
  rp2040->sram[4] = 0x66;
  rp2040->sram[5] = 0x77;
  rp2040->sram[6] = 0x44;
  rp2040->sram[7] = 0x33;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R3], 0x3344);
}

// should execute an `ldrsh r5, [r3, r5]` instruction
TEST(execute_ldrsh_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
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
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = 0x04AD; // lsls r5, r5, #18
  rp2040->registers[R5] = 0b00000000000000000011;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R5], 0b11000000000000000000);
  EXPECT_EQ(rp2040->getPC(), 0x10000002);
  EXPECT_EQ(rp2040->C, false);
}

// should execute a `lsls r5, r5, #18` instruction with carry
TEST(execute_lsls_instruction_with_carry, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = 0x04AD; // lsls r5, r5, #18
  rp2040->registers[R5] = 0x00004001;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R5], 0x40000);
  EXPECT_EQ(rp2040->C, true);
}

// should execute a `rsbs r0, r3` instruction
TEST(execute_rsbs_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeRSBS(R0, R3);
  rp2040->registers[R3] = 100;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R0] | 0, -100);
  EXPECT_EQ(rp2040->N, true);
  EXPECT_EQ(rp2040->Z, false);
  EXPECT_EQ(rp2040->C, false);
  EXPECT_EQ(rp2040->V, false);
}

// should execute a `sbcs r0, r3` instruction
TEST(execute_sbcs_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeSBCS(R0, R3);
  rp2040->registers[R0] = 100;
  rp2040->registers[R3] = 55;
  rp2040->C = false;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R0], 44);
  EXPECT_EQ(rp2040->N, false);
  EXPECT_EQ(rp2040->Z, false);
  EXPECT_EQ(rp2040->C, true);
  EXPECT_EQ(rp2040->V, false);
}

// should execute a `sdmia r0!, {r1, r2}` instruction
TEST(execute_sdmia_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeSTMIA(R0, (1 << R1) | (1 << R2));
  rp2040->registers[R0] = 0x20000000;
  rp2040->registers[R1] = 0xf00df00d;
  rp2040->registers[R2] = 0x4242;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->getPC(), 0x10000002);
  EXPECT_EQ(rp2040->registers[R0], 0x20000008);
  uint32_t *sram32 = (uint32_t *)rp2040->sram;
  EXPECT_EQ(sram32[0], 0xf00df00d);
  EXPECT_EQ(sram32[1], 0x4242);
}

// should execute a `str r6, [r4, #20]` instruction
TEST(execute_str_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = 0x6166; // str r6, [r4, #20]
  rp2040->registers[R4] = RAM_START_ADDRESS + 0x20;
  rp2040->registers[R6] = 0xF00D;
  rp2040->executeInstruction();
  uint32_t value;
  memcpy(&value, &(rp2040->sram[0x20 + 20]), sizeof(uint32_t));
  EXPECT_EQ(value, 0xF00D);
  EXPECT_EQ(rp2040->getPC(), 0x10000002);
}

// should execute `bl 0x34` instruction
TEST(execute_bl_instruction_1, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  cout << "0x" << hex << opcodeBL(0x34) << endl;
  uint32_t *flash32 = (uint32_t *)rp2040->flash;
  flash32[0] = opcodeBL(0x34);
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->getPC(), 0x10000038);
  EXPECT_EQ(rp2040->getLR(), 0x10000004);
}

// should execute `bl -0x10` instruction
TEST(execute_bl_instruction_2, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  uint32_t *flash32 = (uint32_t *)rp2040->flash;
  flash32[0] = opcodeBL(-0x10);
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->getPC(), 0x10000004 - 0x10);
  EXPECT_EQ(rp2040->getLR(), 0x10000004);
}

// should execute `bl -3242` instruction
TEST(execute_bl_instruction_3, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  uint32_t *flash32 = (uint32_t *)rp2040->flash;
  flash32[0] = opcodeBL(-3242);
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->getPC(), 0x10000004 - 3242);
  EXPECT_EQ(rp2040->getLR(), 0x10000004);
}

// should execute `blx r3` instruction
TEST(execute_blx_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->registers[R3] = 0x10000201;
  uint32_t *flash32 = (uint32_t *)rp2040->flash;
  flash32[0] = opcodeBLX(R3);
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->getPC(), 0x10000200);
  EXPECT_EQ(rp2040->getLR(), 0x10000002);
}

// should execute a `b.n .-20` instruction
TEST(execute_bn_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000 + 0x9 * 0x2);
  rp2040->flash16[9] = 0xe7f6; // b.n .-20
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->getPC(), 0x10000002);
}

// should execute a `bne.n .-6` instruction
TEST(execute_bnen_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000 + 0x9 * 0x2);
  rp2040->Z = false;
  rp2040->flash16[9] = 0xD1FC; // bne.n .-6
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->getPC(), 0x1000000E);
}

// should execute `bx lr` instruction
TEST(execute_bx_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->setLR(0x10000200);
  uint32_t *flash32 = (uint32_t *)rp2040->flash;
  flash32[0] = opcodeBX(LR);
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->getPC(), 0x10000200);
}

// should execute an `cmp r5, #66` instruction
TEST(execute_cmp_instruction_1, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
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
  rp2040->setPC(0x10000000);
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
  rp2040->setPC(0x10000000);
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
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeSUBS2(R1, 1);
  rp2040->registers[R1] = -0x80000000;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R1], 0x7fffffff);
  EXPECT_EQ(rp2040->N, false);
  EXPECT_EQ(rp2040->Z, false);
  EXPECT_EQ(rp2040->C, true);
  EXPECT_EQ(rp2040->V, true);
}

// should execute a `sub sp, 0x10` instruction
TEST(execute_subs_instruction_3, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->setSP(0x10000040);
  rp2040->flash16[0] = opcodeSUBsp(0x10);
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->getSP(), 0x10000030);
}

// should execute a `subs r5, r3, 5` instruction
TEST(execute_subs_instruction_4, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeSUBS1(R5, R3, 5);
  rp2040->registers[R3] = 0;
  rp2040->executeInstruction();
  EXPECT_EQ((int)rp2040->registers[R5] | 0, -5);
  EXPECT_EQ(rp2040->N, true);
  EXPECT_EQ(rp2040->Z, false);
  EXPECT_EQ(rp2040->C, false);
  EXPECT_EQ(rp2040->V, false);
}

// should execute a `subs r5, r3, r2` instruction
TEST(execute_subs_instruction_5, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeSUBSreg(R5, R3, R2);
  rp2040->registers[R3] = 6;
  rp2040->registers[R2] = 5;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R5], 1);
  EXPECT_EQ(rp2040->N, false);
  EXPECT_EQ(rp2040->Z, false);
  EXPECT_EQ(rp2040->C, true);
  EXPECT_EQ(rp2040->V, false);
}

// should execute an `tst r1, r3` instruction when the result is negative
TEST(execute_tst_instruction_1, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
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
  rp2040->setPC(0x10000000);
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
  rp2040->setPC(0x10000000);
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
  rp2040->setPC(0x10000000);
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

// should execute a `add sp, 0x10` instruction
TEST(execute_add_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->setSP(0x10000040);
  rp2040->flash16[0] = opcodeADDsp2(0x10);
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->getSP(), 0x10000050);
}

// should execute `adds r1, #1` instruction
TEST(execute_adds_instruction_1, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeADDS2(R1, 1);
  rp2040->registers[R1] = 0xFFFFFFFF;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R1], 0);
  EXPECT_EQ(rp2040->N, false);
  EXPECT_EQ(rp2040->Z, true);
  EXPECT_EQ(rp2040->C, true);
  EXPECT_EQ(rp2040->V, false);
}

// should execute `adds r1, #1` instruction and
// set the overflow flag correctly
TEST(execute_adds_instruction_2, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeADDS2(R1, 1);
  rp2040->registers[R1] = 0x7FFFFFFF;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R1], 0x80000000);
  EXPECT_EQ(rp2040->N, true);
  EXPECT_EQ(rp2040->Z, false);
  EXPECT_EQ(rp2040->C, false);
  EXPECT_EQ(rp2040->V, true);
}

// should execute `adds r1, r2, #3` instruction
TEST(execute_adds_instruction_3, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeADDS1(R1, R2, 3);
  rp2040->registers[R2] = 2;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R1], 5);
  EXPECT_EQ(rp2040->N, false);
  EXPECT_EQ(rp2040->Z, false);
  EXPECT_EQ(rp2040->C, false);
  EXPECT_EQ(rp2040->V, false);
}

// should execute `adds r1, r2, r7` instruction
TEST(execute_adds_instruction_4, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeADDSreg1(R1, R2, R7);
  rp2040->registers[R2] = 2;
  rp2040->registers[R7] = 27;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R1], 29);
  EXPECT_EQ(rp2040->N, false);
  EXPECT_EQ(rp2040->Z, false);
  EXPECT_EQ(rp2040->C, false);
  EXPECT_EQ(rp2040->V, false);
}

// should execute `adr r4, #0x50` instruction and
// set the overflow flag correctly
TEST(execute_adr_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeADR(R4, 0x50);
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R4], 0x10000054);
}

// should execute `ands r5, r0` instruction
TEST(execute_ands_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeANDS(R5, R0);
  rp2040->registers[R5] = 0xffff0000;
  rp2040->registers[R0] = 0xf00fffff;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R5], 0xf00f0000);
  EXPECT_EQ(rp2040->N, true);
  EXPECT_EQ(rp2040->Z, false);
}

// should execute `bics r0, r3` correctly
TEST(execute_bics_instruction_1, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->registers[R0] = 0xff;
  rp2040->registers[R3] = 0x0f;
  rp2040->flash16[0] = opcodeBICS(R0, R3);
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R0], 0xF0);
  EXPECT_EQ(rp2040->N, false);
  EXPECT_EQ(rp2040->Z, false);
}

// should execute `bics r0, r3` instruction
// and set the negative flag correctly
TEST(execute_bics_instruction_2, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->registers[R0] = 0xffffffff;
  rp2040->registers[R3] = 0x0000ffff;
  rp2040->flash16[0] = opcodeBICS(R0, R3);
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R0], 0xffff0000);
  EXPECT_EQ(rp2040->N, true);
  EXPECT_EQ(rp2040->Z, false);
}

// should execute an `uxtb r5, r3` instruction the registers are equal
TEST(execute_uxtb_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeUXTB(R5, R3);
  rp2040->registers[R3] = 0x12345678;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R5], 0x78);
}