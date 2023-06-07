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

int main(void) {
  string filename("../blink.hex");
  string hex = readHexFile(filename);
  RP2040 *mcu = new RP2040(hex);
  mcu->setPC(0x370);
  for (uint8_t i = 0; i < 60; i++) {
    mcu->executeInstruction();
  }
  return EXIT_SUCCESS;
}