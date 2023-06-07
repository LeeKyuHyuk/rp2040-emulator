#include "rp2040.h"
#include "gtest/gtest.h"

#define R4 4
#define R5 5
#define R6 6

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