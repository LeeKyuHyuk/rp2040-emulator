#ifndef __SYSCFG_H__
#define __SYSCFG_H__

#include "peripheral.h"
#include <chrono>

typedef uint64_t number;

using namespace std;

class RP2040;

const number PROC0_NMI_MASK = 0;
const number PROC1_NMI_MASK = 4;

class RP2040SysCfg : public LoggingPeripheral {
public:
  RP2040SysCfg(RP2040 *rp2040, string name) : LoggingPeripheral(rp2040, name) {}

  number readUint32(number offset);
  void writeUint32(number offset, number value);
};

#endif