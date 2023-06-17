#ifndef __INTEL_HEX_H__
#define __INTEL_HEX_H__

#include <cstdint>
#include <string>
#include <vector>

using namespace std;

void loadHex(string source, uint8_t target[], uint64_t baseAddress);
vector<string> split(string str, char delimiter);

#endif