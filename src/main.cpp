#include "rp2040.h"
#include "uart.h"
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

  mcu->setPC(0x10000370);
  for (uint32_t i = 0; i < 280; i++) {
    mcu->executeInstruction();
    // uncomment for debugging:
    // cout << hex << mcu->getPC() << endl;
    // cout << hex << mcu->registers[2] << endl;
  }
  return EXIT_SUCCESS;
}