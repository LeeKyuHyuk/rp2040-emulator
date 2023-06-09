#include "rp2040.h"
#include "uart.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

string readHexFile(const string &path) {
  ifstream input_file(path);
  if (!input_file.is_open()) {
    cerr << "Could not open the file - '" << path << "'" << endl;
    exit(EXIT_FAILURE);
  }
  return string((std::istreambuf_iterator<char>(input_file)),
                std::istreambuf_iterator<char>());
}

int main(void) {
  string filename("../hello_uart.hex");
  string hexFile = readHexFile(filename);
  RP2040 *mcu = new RP2040(hexFile);

  RPUART *uart = new RPUART(mcu, UART0_BASE);

  uart->onByte = [](uint32_t value) -> void {
    cout << "UART sent: " << (char)(value) << endl;
  };

  // To start from boot_stage2:
  // load 256 bytes from flash to the end of SRAM
  // mcu->setLR(0x0);
  // const uint16_t BOOT2_SIZE = 256;
  // memcpy(&mcu->sram, &mcu->flash, BOOT2_SIZE);
  // mcu->setPC(RAM_START_ADDRESS + SRAM_SIZE - BOOT2_SIZE);

  mcu->setPC(0x10000000);
  for (uint32_t i = 0; i < 10000; i++) {
    if (mcu->getPC() >= 0x10000100) {
      cout << "PC: 0x" << hex << mcu->getPC() << endl;
    }
    mcu->executeInstruction();
  }
  return EXIT_SUCCESS;
}