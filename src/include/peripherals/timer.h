#ifndef __TIMER_H__
#define __TIMER_H__

#include "peripheral.h"
#include <chrono>

typedef uint64_t number;

using namespace std;
using namespace std::chrono;

class RP2040;

const number TIMEHR = 0x08;
const number TIMELR = 0x0c;
const number TIMERAWH = 0x24;
const number TIMERAWL = 0x28;

const number ALARM_0 = 1 << 0;
const number ALARM_1 = 1 << 1;
const number ALARM_2 = 1 << 2;
const number ALARM_3 = 1 << 3;

class RPTimer : public LoggingPeripheral {
private:
  number latchedTimeHigh = 0;

public:
  RPTimer(RP2040 *rp2040, string name) : LoggingPeripheral(rp2040, name) {}

  number readUint32(number offset);
  void writeUint32(number offset, number value);
};

#endif