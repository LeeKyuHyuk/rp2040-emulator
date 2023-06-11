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
#define IP 12
#define SP 13
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

// should execute a `lsrs r5, r0` instruction
TEST(execute_lsrs_instruction_3, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeLSRSreg(R5, R0);
  rp2040->registers[R5] = 0xff00000f;
  rp2040->registers[R0] = 0xff003302; // Shift amount: 02
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R5], 0x3fc00003);
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

// should execute an `eors r1, r3` instruction
TEST(execute_eors_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeEORS(R1, R3);
  rp2040->registers[R1] = 0xf0f0f0f0;
  rp2040->registers[R3] = 0x08ff3007;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R1], 0xf80fc0f7);
  EXPECT_EQ(rp2040->N, true);
  EXPECT_EQ(rp2040->Z, false);
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

// should execute a `mvns r4, r3` instruction
TEST(execute_mvns_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeMVNS(R4, R3);
  rp2040->registers[R3] = 0x11115555;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R4], 0xeeeeaaaa);
  EXPECT_EQ(rp2040->Z, false);
  EXPECT_EQ(rp2040->N, true);
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

// should execute an `ldr r3, [r5, r6]` instruction
TEST(execute_ldr_instruction_3, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeLDRreg(R3, R5, R6);
  rp2040->registers[R5] = 0x20000000;
  rp2040->registers[R6] = 0x8;
  rp2040->sram[8] = 0x11;
  rp2040->sram[9] = 0x42;
  rp2040->sram[10] = 0x55;
  rp2040->sram[11] = 0xff;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R3], 0xff554211);
}

// should execute an `ldr r3, [sp, #12]` instruction
TEST(execute_ldr_instruction_4, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->setSP(0x20000000);
  rp2040->flash16[0] = opcodeLDRsp(R3, 12);
  rp2040->sram[12] = 0x55;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R3], 0x55);
}

// should execute an `ldrb r4, [r2, 5]` instruction
TEST(execute_ldrb_instruction_1, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeLDRB(R4, R2, 5);
  rp2040->registers[R2] = 0x20000000;
  rp2040->sram[5] = 0x66;
  rp2040->sram[6] = 0x77;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R4], 0x66);
}

// should execute an `ldrb r3, [r5, r6]` instruction
TEST(execute_ldrb_instruction_2, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeLDRBreg(R3, R5, R6);
  rp2040->registers[R5] = 0x20000000;
  rp2040->registers[R6] = 0x8;
  rp2040->sram[8] = 0x11;
  rp2040->sram[9] = 0x42;
  rp2040->sram[10] = 0x55;
  rp2040->sram[11] = 0xff;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R3], 0x11);
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

// should execute an `ldrh r3, [r5, r6]` instruction
TEST(execute_ldrh_instruction_3, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeLDRHreg(R3, R5, R6);
  rp2040->registers[R5] = 0x20000000;
  rp2040->registers[R6] = 0x8;
  rp2040->sram[8] = 0x11;
  rp2040->sram[9] = 0x42;
  rp2040->sram[10] = 0x55;
  rp2040->sram[11] = 0xff;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R3], 0x4211);
}

// should execute an `ldrsb r5, [r3, r5]` instruction
TEST(execute_ldrsb_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeLDRSB(R5, R3, R5);
  rp2040->registers[R3] = 0x20000000;
  rp2040->registers[R5] = 6;
  rp2040->sram[6] = 0x85;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R5], 0xffffff85);
}

// should execute an `ldrsh r5, [r3, r5]` instruction
TEST(execute_ldrsh_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeLDRSH(R5, R3, R5);
  rp2040->registers[R3] = 0x20000000;
  rp2040->registers[R5] = 0x6;
  rp2040->sram[6] = 0x55;
  rp2040->sram[7] = 0xF0;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R5], 0xfffff055);
}

// should execute a `udf 1` instruction
TEST(execute_udf_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = 0xde01; // udf 1
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->getPC(), 0x10000002);
  EXPECT_EQ(rp2040->getBreakCount(), 1);
}

// should execute a `lsls r5, r5, #18` instruction
TEST(execute_lsls_instruction_1, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = 0x04AD; // lsls r5, r5, #18
  rp2040->registers[R5] = 0b00000000000000000011;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R5], 0b11000000000000000000);
  EXPECT_EQ(rp2040->getPC(), 0x10000002);
  EXPECT_EQ(rp2040->C, false);
}

// should execute a `lsls r5, r0` instruction
TEST(execute_lsls_instruction_2, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeLSLSreg(R5, R0);
  rp2040->registers[R5] = 0b00000000000000000011;
  rp2040->registers[R0] = 0xff003302; // bottom byte: 02
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R5], 0b00000000000000001100);
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
TEST(execute_str_instruction_1, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeSTR(R6, R4, 20);
  rp2040->registers[R4] = RAM_START_ADDRESS + 0x20;
  rp2040->registers[R6] = 0xF00D;
  rp2040->executeInstruction();
  uint32_t value;
  memcpy(&value, &(rp2040->sram[0x20 + 20]), sizeof(uint32_t));
  EXPECT_EQ(value, 0xF00D);
  EXPECT_EQ(rp2040->getPC(), 0x10000002);
}

// should execute a `str r6, [r4, r5]` instruction
TEST(execute_str_instruction_2, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeSTRreg(R6, R4, R5);
  rp2040->registers[R4] = RAM_START_ADDRESS + 0x20;
  rp2040->registers[R5] = 20;
  rp2040->registers[R6] = 0xF00D;
  rp2040->executeInstruction();
  uint32_t value;
  memcpy(&value, &(rp2040->sram[0x20 + 20]), sizeof(uint32_t));
  EXPECT_EQ(value, 0xF00D);
  EXPECT_EQ(rp2040->getPC(), 0x10000002);
}

// should execute an `str r3, [sp, #12]` instruction
TEST(execute_str_instruction_3, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->setSP(0x20000000);
  rp2040->flash16[0] = opcodeSTRsp(R3, 12);
  rp2040->registers[R3] = 0xaa55;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->sram[12], 0x55);
  EXPECT_EQ(rp2040->sram[13], 0xaa);
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

// should execute a `strb r6, [r4, #20]` instruction
TEST(execute_strb_instruction_1, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->sram[0x20] = 0xF2;
  rp2040->sram[0x21] = 0xF3;
  rp2040->sram[0x22] = 0xF4;
  rp2040->sram[0x23] = 0xF5;
  rp2040->flash16[0] = opcodeSTRB(R6, R4, 0x1);
  rp2040->registers[R4] = RAM_START_ADDRESS + 0x20;
  rp2040->registers[R6] = 0xf055;
  rp2040->executeInstruction();
  // assert that the 2nd byte (at 0x21) changed to 0x55
  EXPECT_EQ(rp2040->sram[0x20], 0xF2);
  EXPECT_EQ(rp2040->sram[0x21], 0x55);
  EXPECT_EQ(rp2040->sram[0x22], 0xF4);
  EXPECT_EQ(rp2040->sram[0x23], 0xF5);
  EXPECT_EQ(rp2040->getPC(), 0x10000002);
}

// should execute a `strb r6, [r4, r5]` instruction
TEST(execute_strb_instruction_2, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->sram[0x20] = 0xF2;
  rp2040->sram[0x21] = 0xF3;
  rp2040->sram[0x22] = 0xF4;
  rp2040->sram[0x23] = 0xF5;
  rp2040->flash16[0] = opcodeSTRBreg(R6, R4, R5);
  rp2040->registers[R4] = RAM_START_ADDRESS + 0x20;
  rp2040->registers[R5] = 0x1;
  rp2040->registers[R6] = 0xf055;
  rp2040->executeInstruction();
  // assert that the 2nd byte (at 0x21) changed to 0x55
  EXPECT_EQ(rp2040->sram[0x20], 0xF2);
  EXPECT_EQ(rp2040->sram[0x21], 0x55);
  EXPECT_EQ(rp2040->sram[0x22], 0xF4);
  EXPECT_EQ(rp2040->sram[0x23], 0xF5);
  EXPECT_EQ(rp2040->getPC(), 0x10000002);
}

// should execute a `strh r6, [r4, #20]` instruction
TEST(execute_strh_instruction_1, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->sram[0x20] = 0xF2;
  rp2040->sram[0x21] = 0xF3;
  rp2040->sram[0x22] = 0xF4;
  rp2040->sram[0x23] = 0xF5;
  rp2040->flash16[0] = opcodeSTRH(R6, R4, 0x2);
  rp2040->registers[R4] = RAM_START_ADDRESS + 0x20;
  rp2040->registers[R6] = 0x6655;
  rp2040->executeInstruction();
  // assert that the 3rd/4th byte (at 0x22) changed to 0x6655
  EXPECT_EQ(rp2040->sram[0x20], 0xF2);
  EXPECT_EQ(rp2040->sram[0x21], 0xF3);
  EXPECT_EQ(rp2040->sram[0x22], 0x55);
  EXPECT_EQ(rp2040->sram[0x23], 0x66);
  EXPECT_EQ(rp2040->getPC(), 0x10000002);
}

// should execute a `strh r6, [r4, r1]` instruction
TEST(execute_strh_instruction_2, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->sram[0x20] = 0xF2;
  rp2040->sram[0x21] = 0xF3;
  rp2040->sram[0x22] = 0xF4;
  rp2040->sram[0x23] = 0xF5;
  rp2040->flash16[0] = opcodeSTRHreg(R6, R4, R1);
  rp2040->registers[R4] = RAM_START_ADDRESS + 0x20;
  rp2040->registers[R1] = 0x2;
  rp2040->registers[R6] = 0x6655;
  rp2040->executeInstruction();
  // assert that the 3rd/4th byte (at 0x22) changed to 0x6655
  EXPECT_EQ(rp2040->sram[0x20], 0xF2);
  EXPECT_EQ(rp2040->sram[0x21], 0xF3);
  EXPECT_EQ(rp2040->sram[0x22], 0x55);
  EXPECT_EQ(rp2040->sram[0x23], 0x66);
  EXPECT_EQ(rp2040->getPC(), 0x10000002);
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

// should execute a `sxtb r2, r2` instruction with sign bit 1
TEST(execute_sxtb_instruction_1, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeSXTB(R2, R2);
  rp2040->registers[R2] = 0x22446688;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R2], 0xffffff88);
}

// should execute a `sxtb r2, r2` instruction with sign bit 0
TEST(execute_sxtb_instruction_2, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeSXTB(R2, R2);
  rp2040->registers[R2] = 0x12345678;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R2], 0x78);
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
TEST(execute_add_instruction_1, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->setSP(0x10000040);
  rp2040->flash16[0] = opcodeADDsp2(0x10);
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->getSP(), 0x10000050);
}

// should execute `add r1, ip` instruction
TEST(execute_add_instruction_2, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeADDreg(R1, IP);
  rp2040->registers[R1] = 66;
  rp2040->registers[IP] = 44;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R1], 110);
  EXPECT_EQ(rp2040->N, false);
  EXPECT_EQ(rp2040->Z, false);
  EXPECT_EQ(rp2040->C, false);
  EXPECT_EQ(rp2040->V, false);
}

// should execute `add sp, r8` instruction and not update the flags
TEST(execute_add_instruction_3, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->setSP(0x20030000);
  rp2040->Z = true;
  rp2040->flash16[0] = opcodeADDreg(SP, R8);
  rp2040->registers[R8] = 0x11;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->getSP(), 0x20030011);
  EXPECT_EQ(rp2040->Z, true); // assert it didn't update the flags
}

// should execute `add pc, r8` instruction
TEST(execute_add_instruction_4, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeADDreg(PC, R8);
  rp2040->registers[R8] = 0x11;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->getPC(), 0x10000014);
}

// should execute a `add r1, sp, #4` instruction
TEST(execute_add_instruction_5, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->setSP(0x55);
  rp2040->flash16[0] = opcodeADDspPlusImm(R1, 0x10);
  rp2040->registers[R1] = 0;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->getSP(), 0x55);
  EXPECT_EQ(rp2040->registers[R1], 0x65);
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
  rp2040->flash16[0] = opcodeADDSreg(R1, R2, R7);
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

// should execute an `asrs r3, r2, #31` instruction
TEST(execute_asrs_instruction, executeInstruction) {
  RP2040 *rp2040 = new RP2040("");
  rp2040->setPC(0x10000000);
  rp2040->flash16[0] = opcodeASRS(R3, R2, 31);
  rp2040->registers[R2] = 0x80000000;
  rp2040->executeInstruction();
  EXPECT_EQ(rp2040->registers[R3], 0xffffffff);
  EXPECT_EQ(rp2040->getPC(), 0x10000002);
  EXPECT_EQ(rp2040->N, true);
  EXPECT_EQ(rp2040->Z, false);
  EXPECT_EQ(rp2040->C, false);
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