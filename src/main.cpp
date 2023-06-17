#include "bootrom.h"
#include "intelhex.h"
#include "rp2040.h"
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

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cerr << "Please input HexFile!" << endl;
    cerr << "[Usage] $ ./rp2040-emulator ./examples/hello_uart.hex" << endl;
    return EXIT_FAILURE;
  }
  string filename(argv[1]);
  string hexFile = readHexFile(filename);
  RP2040 *mcu = new RP2040();
  mcu->loadBootrom(bootromB1, BOOT_ROM_B1_SIZE);
  loadHex(hexFile, mcu->flash);

  mcu->uart[0]->onByte = [](number value) -> void {
    cout << "UART sent: " << (char)(value) << endl;
  };

  mcu->setPC(0x10000000);
  mcu->execute();

  return EXIT_SUCCESS;
}