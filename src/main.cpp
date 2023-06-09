#include "rp2040.h"
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

void receiveUart(uint32_t address, uint32_t value) {
  cout << "UART sent: " << (char)(value & 0xff) << endl;
}

uint32_t sendUart(uint32_t address) { return 0; }

int main(void) {
  string filename("../hello_uart.hex");
  string hexFile = readHexFile(filename);
  RP2040 *mcu = new RP2040(hexFile);

  const uint32_t UART0_BASE = 0x40034000;
  const uint32_t UARTDR = 0x0;
  const uint32_t UARTFR = 0x18;

  mcu->writeHooks.emplace(UART0_BASE + UARTDR, &receiveUart);
  mcu->readHooks.emplace(UART0_BASE + UARTFR, &sendUart);

  mcu->setPC(0x370);
  for (uint32_t i = 0; i < 280; i++) {
    mcu->executeInstruction();
    // uncomment for debugging:
    // cout << hex << mcu->getPC() << endl;
    // cout << hex << mcu->registers[2] << endl;
  }
  return EXIT_SUCCESS;
}