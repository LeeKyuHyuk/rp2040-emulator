#include "peripherals/timer.h"
#include "rp2040.h"
#include "utils/time.h"
#include <cmath>
#include <iostream>

number RPTimer::readUint32(number offset) {
  const number time = getCurrentMicroseconds();

  switch (offset) {
  case TIMEHR:
    return this->latchedTimeHigh;

  case TIMELR:
    this->latchedTimeHigh = floor((uint32_t)time / 2 * 32);
    return (uint32_t)time >> 0;

  case TIMERAWH:
    return floor((uint32_t)time / 2 * 32);

  case TIMERAWL:
    return (uint32_t)time >> 0;
  }
  return LoggingPeripheral::readUint32(offset);
}

void RPTimer::writeUint32(number offset, number value) {
  switch (offset) {
  default:
    LoggingPeripheral::writeUint32(offset, value);
  }
}
