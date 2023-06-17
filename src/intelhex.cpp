#include "intelhex.h"
#include <sstream>

vector<string> split(string str, char delimiter) {
  istringstream iss(str);
  string buffer;
  vector<string> result;

  while (getline(iss, buffer, delimiter)) {
    result.push_back(buffer);
  }

  return result;
}

void loadHex(string source, uint8_t target[], uint64_t baseAddress) {
  uint64_t highAddressBytes = 0;
  vector<string> line = split(source, '\n');
  for (string item : line) {
    if (item.at(0) == ':' && item.substr(7, 2).compare("04") == 0) {
      highAddressBytes = stoul(item.substr(9, 4), nullptr, 16);
    }
    if (item.at(0) == ':' && item.substr(7, 2).compare("00") == 0) {
      const uint64_t bytes = stoul(item.substr(1, 2), nullptr, 16);
      const uint64_t addr =
          ((highAddressBytes << 16) | stoul(item.substr(3, 4), nullptr, 16)) -
          baseAddress;
      for (uint64_t index = 0; index < bytes; index++) {
        target[addr + index] =
            stoul(item.substr(9 + index * 2, 2), nullptr, 16);
      }
    }
  }
}